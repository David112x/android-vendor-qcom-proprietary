/*!
 * @file vpp_ip_frc_me.c
 *
 * @cr
 * Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @services: Interface to Motion Estimation from Venus Encoder
 */


/*----------------------------------------------------------------------
* Include Files
* ----------------------------------------------------------------------*/
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//Activation of the Logging Service
#define VPP_LOG_TAG     VPP_LOG_MODULE_FRC_ME_TAG
#define VPP_LOG_MODULE  VPP_LOG_MODULE_FRC_ME

#include <media/msm_media_info.h>
#include <media/msm_vidc.h>
#include "vpp_dbg.h"
#include "vpp.h"
#include "vpp_buf.h"
#include "vpp_ion.h"
#include "vpp_buf.h"
#include "vpp_callback.h"
#include "vpp_reg.h"
#include "vpp_queue.h"
#include "vpp_ip.h"
#include "vpp_utils.h"
#include "vpp_stats.h"
#include "vpp_ip_frc_me.h"

//ME specific includes
#include <linux/videodev2.h>
#include <poll.h>
#include <media/msm_vidc.h>
#include <linux/msm_vidc_enc.h>
#include <sys/eventfd.h>


/*-------------------------------------------------------------------------
* Preprocessor Definitions and Constants
* ----------------------------------------------------------------------
*/

//Debug options
#define FRC_ME_MBI_TEST  1      //Test 1: MBI App test, 0: Encoder App test
#define DEBUG_READ_BACK  0      //Activate/Deactivate  Debug Read back registers
#define FRC_ME_INC_PRI   0      //Increase Receiver thread priority
#define VPP_FRC_ME_DUMP_BUF_ENABLE  0 //Enable file DUMP
#define VPP_FRC_ME_CRC_BUF_ENABLE   0 //Enable file CRC
#define VPP_FRC_ME_ZERO_MBI_ENABLE  0 //Enable zero overwrite MBI output
#define VPP_FRC_ME_LOG_IOCTL_ENABLE 0 //Enable IOCTL log
#define VPP_FRC_ME_LOGIS            0 //Enable LOGIS - secondary, less important
#if VPP_FRC_ME_LOGIS
#define LOGIS  LOGI
#else
#define LOGIS(fmt, args...)     //Disable least important logs
#endif

//ME constraints
#define FRC_ME_PORT_BUF_Q_SZ VPP_INTERNAL_BUF_MAX    //Buffer queues size
#define FRC_ME_CMD_Q_SZ 128                          //Command queue size
#define FRC_ME_CB_GET(ctx) (ctx ? (t_StVppIpFrcMeCb *)ctx : NULL)
#define SZ_4K 0x1000                      //Buffer property alignment
#define SZ_1M 0x100000
#define FRC_ME_POLL_TIMEOUT   1000
#define FRC_ME_MAX_RESOLUTION_W 4096
#define FRC_ME_MAX_RESOLUTION_H 2160
#define FRC_ME_NO_PLANES 2
#define FRC_ME_V4L2_SECURE_UNMAPPED_BUF_VAL (unsigned long)1

//ME Flags
#define FRC_ME_RECEIVER_THREAD_STARTED 0x01
#define FRC_ME_RELEASED_INPUT_EOS_DETECTED 0x02
#define FRC_ME_RELEASED_OUTPUT_EOS_DETECTED 0x04
#define FRC_ME_RELEASED_INPUT_EOS_DRAIN_DETECTED 0x08
#define FRC_ME_RELEASED_OUTPUT_EOS_DRAIN_DETECTED 0x10
#define FRC_ME_FLUSH_DRAIN_STARTED 0x20

#define FRC_ME_START_IN_BUFS  (1)        //This constant MUST be 1 if EOS can come in the first buffer!
#define FRC_ME_MIN_IN_BUFS  (3)
#define FRC_ME_MIN_OUT_BUFS (4)
#define FRC_ME_MAX_IN_BUFS  (31)         //30 input buffers and 1 for EOS Drain buffer
#define FRC_ME_MAX_OUT_BUFS (30)
#define FRC_ME_DEFAULT_OUT_BUFS (12)
#define FRC_ME_MAX_INDEX    (0xffffffff)
#define FRC_ME_DEFAULT_DELAY (33*1000)   //ms

#define FRC_ME_STATS(cb, stat)                 (cb)->stats.u32##stat++

#if VPP_FRC_ME_LOG_IOCTL_ENABLE
inline int ioctlLog(int fd, int cmd, void* par);
#define IOCTL(fd, cmd, par) ioctlLog(fd, cmd, par)
#else
#define IOCTL(fd, cmd, par) ioctl(fd, cmd, par)
#endif

//ME Status
enum
{
    VPP_FRC_ME_TERMED,
    VPP_FRC_ME_INITED,
    VPP_FRC_ME_OPENED,
    VPP_FRC_ME_RUNNING,
    VPP_FRC_ME_RUNNING_FLUSHED,
    VPP_FRC_ME_RUNNING_DRAINED,
    VPP_FRC_ME_MAX
};

//ME Stage
enum
{
    VPP_FRC_ME_STAGE_START,
    VPP_FRC_ME_STAGE_FEED,
    VPP_FRC_ME_STAGE_FLUSH,
    VPP_FRC_ME_STAGE_BYPASS,
    VPP_FRC_ME_STAGE_MAX
};

enum
{
    FRC_ME_STAT_ENC_1F,
    FRC_ME_STAT_WORKER,
    FRC_ME_STAT_WORKER_SLEEP,
    FRC_ME_STAT_REC_WORKER,
    FRC_ME_STAT_REC_WORKER_SLEEP,
};

typedef enum {
    VPP_IP_FRC_BUF_ME_YUV_IN,
    VPP_IP_FRC_BUF_ME_YUV_OUT,   //Do not use, it doesn't contain valid YUV data
    VPP_IP_FRC_BUF_ME_MBI_OUT,
} t_EVppIpFrcMeDumpBuf;

/*----------------------------------------------------------------------
* Type Definition
* ----------------------------------------------------------------------*/

/*!
 * @brief      timestamp definition struct
 *
 * @description
 *
 */
typedef struct {
    uint64_t timestamp;
    uint32_t flags;
} t_StVppIpFrcMeTimestamp;

/*!
 * @brief      timestamp circular list
 *
 * @description
 *
 */
typedef struct {
    t_StVppIpFrcMeTimestamp arr[FRC_ME_MAX_IN_BUFS+1];
    uint32_t read_idx;   //index to be read
    uint32_t write_idx;  //index to be writen
} t_StVppIpFrcMeTimestampList;

/*!
 * @brief      ME port definition struct
 *
 * @description
 *
 */
typedef struct {
    struct vpp_port_param stParam;
    t_StVppBufPool stQ;
} t_StVppIpFrcMePort;

/*!
 * @brief      ME context struct
 *
 * @description
 *
 */
typedef struct {
    t_StVppIpBase stBase;

    struct hqv_control ctrl;

    uint32_t u32InternalFlags;

    pthread_t thread;                //ME working thread
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    uint32_t u32WorkerErr;

    t_StVppIpFrcMePort stInput;      //ME ports
    t_StVppIpFrcMePort stOutput;

    t_StVppQueue stCmdQ;                       //ME command queue
    t_StVppIpCmd astCmdNode[FRC_ME_CMD_Q_SZ];

    uint32_t eState;

    //Profiling counters
    struct {
        STAT_DECL(InQCnt);          // Number of frames queued to input
        STAT_DECL(OutQCnt);         // Number of frames queued to output
        STAT_DECL(InProcCnt);       // Number of input frames processed
        STAT_DECL(OutProcCnt);      // Number of output frames processed into
        STAT_DECL(IBDCnt);          // Number of IBD issued
        STAT_DECL(OBDCnt);          // Number of OBD issued
        STAT_DECL(BypassBuf);       // Bypass buffer due to flag
        STAT_DECL(BypassIntBuf);    // Bypass buffer due to interlaced
    } stats;

    //ME
    pthread_t MeThread;
    pthread_cond_t MeCond;
    unsigned int MeFd;
    int MePollNotification;
    uint32_t u32MeStatus;
    uint32_t u32MeFlags;
    uint32_t u32MeFoundBypass;
    uint32_t u32MeFoundEos;
    uint32_t u32MeWorkerErr;
    uint32_t u32MeEvent;
    uint32_t u32MeCmd;
    uint32_t u32MeStage;
    uint32_t aMeStream[2];
    uint32_t aMePrepareOutput[FRC_ME_MAX_OUT_BUFS];
    uint32_t u32MeProcInQCnt;
    uint32_t u32MeProcOutQCnt;
    char* sMeDeviceName;
    int MeFpsNum;
    int MeFpsDen;
    int MeTargetBitRate;
    int MeInputFormat;
    int MeCodecType;
    int MeCodecProfile;
    int MeProfileLevel;
    int MeSessionGpRangeMinQp;
    int MeSessionGpRangeMaxQp;
    int MeIsThuliumV1;
    struct v4l2_capability           stMeCapabilities;
    struct v4l2_fmtdesc              stMeFormat;
    struct v4l2_requestbuffers       stMeBufReq;
    struct venc_allocatorproperty    stMeInputBuffProperty;
    struct venc_allocatorproperty    stMeOutputBuffProperty;
    struct v4l2_frmsizeenum          stMeFrameSize;
    struct v4l2_format               stMeFormat_Input;
    struct v4l2_format               stMeFormat_Output;

    //ME processing status counters
    int InputBufsMEGot;
    int OutputBufsMEGot;
    int InputBufsMEQueued;
    int OutputBufsMEQueued;
    int InputBufsMEInside;
    int OutputBufsMEInside;
    int InputBufsMEDone;
    int OutputBufsMEDone;
    int InputBufsNeed;
    int OutputBufsNeed;
    int InputBufsInternalNeed;
    int OutputBufsInternalNeed;
    int OutputBufsInternalPool;
    int InputEosCnt;
    int InputGotEosCnt;

    t_StVppBuf *pstBufIn[FRC_ME_MAX_IN_BUFS];          //input buffers indexes array
    t_StVppBuf *pstBufOut[FRC_ME_MAX_OUT_BUFS];        //output buffers indexes array
    t_StVppBuf stBufEos;                               //buffer Drain EOS pointer
    struct vpp_buffer BufEos;                          //buffer Drain EOS parameters
    t_StVppIonBuf stIonBuf;
    uint64_t MeLastTimestamp;
    t_StVppIpFrcMeTimestampList InputTimestampList;    //timestamp circular list
} t_StVppIpFrcMeCb;

/************************************************************************
 * Local static variables
 ************************************************************************/
static const int aMeEventType[] =
{
    V4L2_EVENT_MSM_VIDC_FLUSH_DONE,
    V4L2_EVENT_MSM_VIDC_SYS_ERROR,
};

int aMeEventType_size = sizeof(aMeEventType)/sizeof(int);

static const t_StVppStatsConfig astFrcMeStatsCfg[] = {
    VPP_PROF_DECL(FRC_ME_STAT_ENC_1F, 100, 1),
    VPP_PROF_DECL(FRC_ME_STAT_WORKER, 100, 0),
    VPP_PROF_DECL(FRC_ME_STAT_WORKER_SLEEP, 100, 0),
    VPP_PROF_DECL(FRC_ME_STAT_REC_WORKER, 100, 0),
    VPP_PROF_DECL(FRC_ME_STAT_REC_WORKER_SLEEP, 100, 0),
};

static const uint32_t u32FrcMeStatCnt = VPP_STATS_CNT(astFrcMeStatsCfg);

/************************************************************************
 * Forward Declarations
 *************************************************************************/
static uint32_t u32VppIpFrcMe_LogBufferInfo(t_StVppBuf *pstBuf);
static uint32_t u32VppIpFrcMe_MeStreamOn(t_StVppIpFrcMeCb *pstCb, enum vpp_port ePort);
static uint32_t u32VppIpFrcMe_MeStreamOff(t_StVppIpFrcMeCb *pstCb, enum vpp_port ePort);
static void *vpVppIpFrcMe_MeReceiver(void *pv);
static void vVppIpFrcMe_MeWaitReceiverStart(t_StVppIpFrcMeCb *pstCb);
static void vVppIpFrcMe_SignalWorkerStart(t_StVppIpFrcMeCb *pstCb);
static void vVppIpFrcMe_HandleStartPendingFlushDrain(t_StVppIpFrcMeCb *pstCb);

#if VPP_FRC_ME_DUMP_BUF_ENABLE
#define VPP_FRC_DUMP_NM_LEN             256
#define VPP_FRC_DUMP_BUF_ME_IN_YUV_NM   "frc_%p_me_yuv_in_%06d.yuv"
#define VPP_FRC_DUMP_BUF_ME_OUT_YUV_NM  "frc_%p_me_yuv_out_%06d.yuv"
#define VPP_FRC_DUMP_BUF_ME_OUT_MBI_NM  "frc_%p_me_mbi_out_%06d.mbi"

void vVppIpFrcMe_DumpBuffer(t_StVppIpFrcMeCb *pstCb,
                            t_StVppBuf *pstBuf,
                            t_EVppIpFrcMeDumpBuf eBuf,
                            uint32_t idx);

#define VPP_FRC_ME_DUMP_BUF(_pstCb,_pBuf,_eBuf,_idx) vVppIpFrcMe_DumpBuffer(_pstCb,_pBuf,_eBuf,_idx)
#else
#define VPP_FRC_ME_DUMP_BUF(_pstCb,_pBuf,_eBuf,_idx)
#endif

#if VPP_FRC_ME_CRC_BUF_ENABLE
#define VPP_FRC_ME_CRC_BUF(_pstCb,_pBuf, _idx) u32Vpp_CrcBuffer(_pBuf, eVppBuf_Pixel,\
                                 (_pBuf->stPixel.u32FilledLen / 2),\
                                 (_pBuf->stPixel.u32FilledLen / 8),\
                                 _idx, "VPP_FRC_ME_CRC_BUF")
#define VPP_FRC_ME_CRC_MBI_BUF(_pstCb,_pBuf, _idx) u32Vpp_CrcBuffer(_pBuf, eVppBuf_Extra,\
                                 (_pBuf->stExtra.u32FilledLen / 2),\
                                 (_pBuf->stExtra.u32FilledLen / 8),\
                                 _idx, "VPP_FRC_ME_CRC_BUF")
#else
#define VPP_FRC_ME_CRC_BUF(_pstCb,_pBuf,_idx)
#define VPP_FRC_ME_CRC_MBI_BUF(_pstCb,_pBuf, idx)
#endif

#if VPP_FRC_ME_ZERO_MBI_ENABLE
uint32_t u32VppIpFrcMe_ZeroBuffer(t_StVppIpFrcMeCb *pstCb,
                                          t_StVppBuf *pstBuf,
                                          t_EVppIpFrcMeDumpBuf eBuf,
                                          uint32_t idx);

#define VPP_FRC_ME_ZERO_BUF(_pstCb,_pBuf,_eBuf,_idx) u32VppIpFrcMe_ZeroBuffer(_pstCb,_pBuf,_eBuf,_idx)
#else
#define VPP_FRC_ME_ZERO_BUF(_pstCb,_pBuf,_eBuf,_idx)
#endif

#if VPP_FRC_ME_LOG_IOCTL_ENABLE
char* getIoctlString(int cmd)
{
    switch (cmd)
    {
       case VIDIOC_ENCODER_CMD:       return("VIDIOC_ENCODER_CMD_FLUSH");
       case VIDIOC_REQBUFS:           return("VIDIOC_REQBUFS");
       case VIDIOC_UNSUBSCRIBE_EVENT: return("VIDIOC_UNSUBSCRIBE_EVENT");
       case VIDIOC_G_CTRL:            return("VIDIOC_G_CTRL");
       case VIDIOC_G_EXT_CTRLS:       return("VIDIOC_G_EXT_CTRLS");
       case VIDIOC_SUBSCRIBE_EVENT:   return("VIDIOC_SUBSCRIBE_EVENT");
       case VIDIOC_QUERYCAP:          return("VIDIOC_QUERYCAP");
       case VIDIOC_S_FMT:             return("VIDIOC_S_FMT");
       case VIDIOC_S_CTRL:            return("VIDIOC_S_CTRL");
       case VIDIOC_ENUM_FRAMESIZES:   return("VIDIOC_ENUM_FRAMESIZES");
       case VIDIOC_S_PARM:            return("VIDIOC_S_PARM");
       case VIDIOC_G_FMT:             return("VIDIOC_G_FMT");
       case VIDIOC_PREPARE_BUF:       return("VIDIOC_PREPARE_BUF");
       case VIDIOC_STREAMON:          return("VIDIOC_STREAMON");
       case VIDIOC_STREAMOFF:         return("VIDIOC_STREAMOFF");
       case VIDIOC_QBUF:              return("VIDIOC_QBUF");
       case VIDIOC_DQBUF:             return("VIDIOC_DQBUF");
       case VIDIOC_DQEVENT:           return("VIDIOC_DQEVENT");
       default:                       return("unknown");
    }
}

inline int ioctlLog(int fd, int cmd, void* par)
{
    static unsigned long idx=0;
    int i=cmd & 0xff;
    LOGI("%lu %s(%d, %s)",idx, __func__, fd, getIoctlString(cmd));
    idx++;
    return ioctl(fd, cmd, par);
}
#endif

/************************************************************************
 * Local Functions
 ************************************************************************/


/*!
 * @brief       Log Me Status.
 *
 * @description
 *
 * @input       pstCb    ME Context
 *                  u32InQSz size of the input buffers queue
 *                  u32OutQSz size of the output buffers queue
 *
 * @return     VPP_OK, VPP_ERR_PARAM for error
 *
 */
static uint32_t u32VppIpFrcMe_LogStatus(t_StVppIpFrcMeCb *pstCb, uint32_t u32InQSz,
                                        uint32_t u32OutQSz)
{
    uint32_t u32Ret = VPP_OK;

    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);
    LOGI("u32InQSz=%u, u32OutQSz=%u, u32MeStatus=%d, u32MeStage=%d, "
         "u32InternalFlags=0x%x, u32MeFlags=0x%x, pstCb->u32MeFoundBypass=%d, "
         "InputEosCnt=%d, InputGotEosCnt=%d",u32InQSz, u32OutQSz, pstCb->u32MeStatus,
         pstCb->u32MeStage, pstCb->u32InternalFlags, pstCb->u32MeFlags,
         pstCb->u32MeFoundBypass, pstCb->InputEosCnt, pstCb->InputGotEosCnt);
    LOGI("InNeed=%d, OutNeed=%d, InInternalNeed=%d, OutInternalNeed=%d, "
         "InGot=%d, OutGot=%d, InQueued=%d, OutQueued=%d, InInside=%d, "
         "OutInside=%d, InDone=%d, OutDone=%d \n",
         pstCb->InputBufsNeed, pstCb->OutputBufsNeed,
         pstCb->InputBufsInternalNeed, pstCb->OutputBufsInternalNeed,
         pstCb->InputBufsMEGot, pstCb->OutputBufsMEGot,
         pstCb->InputBufsMEQueued, pstCb->OutputBufsMEQueued,
         pstCb->InputBufsMEInside, pstCb->OutputBufsMEInside,
         pstCb->InputBufsMEDone, pstCb->OutputBufsMEDone);
    return (u32Ret);
}

/*!
 * @brief       Flush timestamp circular list.
 *
 * @description The caller must lock the mutex.
 *
 * @input       plist    timestamp list pointer
 *
 * @return     VPP_OK, VPP_ERR_PARAM for error
 *
 */
static uint32_t u32VppIpFrcMe_TimestampListFlush(t_StVppIpFrcMeTimestampList* plist)
{
    uint32_t u32Ret = VPP_OK;

    VPP_RET_IF_NULL(plist, VPP_ERR_PARAM);
    memset((void*)plist, 0, sizeof(t_StVppIpFrcMeTimestampList));
    return (u32Ret);
}

/*!
 * @brief       Read timestamp circular list.
 *
 * @description The caller must lock the mutex.
 *
 * @input       plist    timestamp list pointer
 * @output    ptimestamp    timestamp dest pointer
 *
 * @return     VPP_OK valid timestamp, VPP_ERR empty list or error
 *
 */
static uint32_t u32VppIpFrcMe_TimestampListRead(t_StVppIpFrcMeTimestampList* plist,
                                                              uint64_t* ptimestamp)
{
    uint32_t u32Ret = VPP_OK;

    VPP_RET_IF_NULL(plist, VPP_ERR_PARAM);
    VPP_RET_IF_NULL(ptimestamp, VPP_ERR_PARAM);
    if (plist->read_idx != plist->write_idx)
    {
        *ptimestamp=plist->arr[plist->read_idx].timestamp;
        plist->read_idx++;
        if (plist->read_idx >= (FRC_ME_MAX_IN_BUFS + 1))
            plist->read_idx = 0;
    }
    else
    {
        //LOGE("%s ERROR: Timestamp list is empty and cannot be read!",  __func__);
        u32Ret=VPP_ERR;
    }
    LOGIS("%s ret=%d, timestamp=0x%llx",  __func__, u32Ret, (long long unsigned int)*ptimestamp);
    return (u32Ret);
}

/*!
 * @brief       Read timestamp circular list.
 *
 * @description The caller must lock the mutex.
 *
 * @input       plist    timestamp list pointer
 * @input       timestamp    to be written
 *
 * @return     VPP_OK , VPP_ERR full list or error
 *
 */
static uint32_t u32VppIpFrcMe_TimestampListWrite(t_StVppIpFrcMeTimestampList* plist,
                                                              uint64_t timestamp)
{
    uint32_t u32Ret = VPP_OK;

    VPP_RET_IF_NULL(plist, VPP_ERR_PARAM);

    uint32_t u32w = plist->write_idx;
    u32w++;
    if (u32w >= (FRC_ME_MAX_IN_BUFS + 1))
        u32w = 0;

    if (plist->read_idx != u32w)
    {
        plist->arr[plist->write_idx].timestamp = timestamp;
        plist->write_idx = u32w;
    }
    else
    {
        LOGE("%s ERROR: Timestamp list is full and Timestamp=0x%llx cannot be written!",  __func__,
             (long long unsigned int)timestamp);
        u32Ret = VPP_ERR;
    }
    LOGIS("%s ret=%d, timestamp=0x%llx",  __func__, u32Ret, (long long unsigned int)timestamp);
    return (u32Ret);
}

/*!
 * @brief       Get cmd from the command queue.
 *
 * @description Command queue is protected by mutex.
 *
 * @input       pstCb    ME Context
 * @output    pstCmd Command struct pointer
 *
 * @return     VPP_OK for new command, VPP_ERR for empty queue or error
 *
 */
static uint32_t u32VppIpFrcMe_CmdGet(t_StVppIpFrcMeCb *pstCb, t_StVppIpCmd *pstCmd)
{
    int32_t idx;
    uint32_t u32Ret = VPP_OK;

    LOGIS("%s()", __func__);
    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);
    idx = vpp_queue_dequeue(&pstCb->stCmdQ);

    if (idx < 0)
    {
        u32Ret = VPP_ERR;
    }
    else
    {
        *pstCmd = pstCb->astCmdNode[idx];
        LOG_CMD("GetCmd", pstCmd->eCmd);
    }

    LOG_EXITI_RET(u32Ret);
}

/*!
 * @brief       Put cmd into the command queue.
 *
 * @description Command queue is protected by mutex.
 *
 * @input       pstCb    ME Context
 * @input       pstCmd Command struct pointer
 *
 * @return     VPP_OK for success, VPP_ERR for full queue or error
 *
 */
static uint32_t u32VppIpFrcMe_CmdPut(t_StVppIpFrcMeCb *pstCb, t_StVppIpCmd* stpCmd)
{
    int32_t idx;
    uint32_t u32Ret = VPP_OK;

    LOGIS("%s\n", __func__);
    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);
    pthread_mutex_lock(&pstCb->mutex);

    LOG_CMD("InsertCmd in FrcMe Worker", stpCmd->eCmd);
    idx = vpp_queue_enqueue(&pstCb->stCmdQ);
    if (idx < 0)
    {
        u32Ret = VPP_ERR;
    }
    else
    {
        pstCb->astCmdNode[idx] = *stpCmd;
        pthread_cond_signal(&pstCb->cond);
    }
    pthread_mutex_unlock(&pstCb->mutex);

    LOG_EXITI_RET(u32Ret);
}

/*!
 * @brief       ME Initialization.
 *
 * @description Open ME driver and create fd for notification.
 *
 * @input       pstCb    ME Context
 *
 * @return     VPP_OK for success, VPP_ERR for error
 *
 */
static uint32_t u32VppIpFrcMe_MeInit(t_StVppIpFrcMeCb *pstCb)
{
    t_StVppIpCmd stCmd;

    LOGI("%s\n", __func__);
    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);

    if (pstCb->u32MeStatus != VPP_FRC_ME_TERMED)
    {
        LOGE("Error: ME is already Inited!\n");
        LOG_EXITI_RET(VPP_ERR_STATE);
    }

    //Open ME driver
    pstCb->sMeDeviceName = (char*)"/dev/video33";
    pstCb->MeFd=-1;
    pstCb->MeFd = open (pstCb->sMeDeviceName, O_RDWR);

    if (pstCb->MeFd == 0)
    {
        LOGE("Error: Got MeFd as 0 for msm_vidc_enc, Opening again!\n");
        pstCb->MeFd = open (pstCb->sMeDeviceName, O_RDWR);
    }

    if ((int)pstCb->MeFd < 0)
    {
        LOGE("Error: MeFd=%d <0. Open() filed! \n", (int)pstCb->MeFd);
        LOG_EXITI_RET(VPP_ERR_STATE);
    }
    LOGI("MeFd=%d success\n", pstCb->MeFd);

    //Create fd for event notification
    pstCb->MePollNotification = eventfd(0, 0);
    if (pstCb->MePollNotification < 0)
    {
        LOGE("Error: MePollNotification=%d <0. Open() filed!\n", pstCb->MePollNotification);
        close(pstCb->MeFd);
        pstCb->MeFd = -1;
        LOG_EXITI_RET(VPP_ERR_STATE);
    }
    LOGI("MePollNotification=%d success\n", pstCb->MePollNotification);

    pstCb->u32MeStatus = VPP_FRC_ME_INITED;
    LOG_EXITI_RET(VPP_OK);
}

/*!
 * @brief       ME Termination.
 *
 * @description Flush, Stop, Unsubscribe and Close ME.
 *
 * @input       pstCb    ME Context
 *
 * @return     VPP_OK for success, VPP_ERR for error
 *
 */
static uint32_t u32VppIpFrcMe_MeTerm(t_StVppIpFrcMeCb *pstCb)
{
    t_StVppIpCmd stCmd;

    LOGI("%s\n", __func__);
    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);

    if (pstCb->u32MeStatus == VPP_FRC_ME_TERMED)
        LOG_EXITI_RET(VPP_OK);

    //Wait for the current Fush if any
    while ((pstCb->u32InternalFlags & IP_FLUSH_PENDING)||
           (pstCb->u32InternalFlags & IP_DRAIN_PENDING)||
           (pstCb->u32InternalFlags & IP_EOS_PENDING))
    {
        sched_yield();
    }
    pstCb->u32MeStatus = VPP_FRC_ME_TERMED;

    //Close ME
    struct v4l2_encoder_cmd enc;

    if ((int)pstCb->MeFd >= 0)
    {
        struct v4l2_requestbuffers bufreq;
        int ret = 0;

        //Flush ME
        pstCb->u32MeStage = VPP_FRC_ME_STAGE_FLUSH;
        pstCb->u32InternalFlags |= IP_FLUSH_PENDING;
        pstCb->u32MeEvent = 0;
        enc.cmd = V4L2_ENC_QCOM_CMD_FLUSH;
        enc.flags = V4L2_QCOM_CMD_FLUSH_OUTPUT | V4L2_QCOM_CMD_FLUSH_CAPTURE;

        ret = IOCTL(pstCb->MeFd, VIDIOC_ENCODER_CMD, &enc);
        if (ret)
        {
            LOGE("Error: Flush ME Failed, ret=%d", ret);
        }

        LOGI("%s ME Flush Command\n", __func__);
        while (pstCb->u32MeEvent != V4L2_EVENT_MSM_VIDC_FLUSH_DONE)
            sched_yield();
        pstCb->u32MeEvent = 0;
        LOGI("%s ME Flushed.\n", __func__);

        u32VppIpFrcMe_MeStreamOff(pstCb,VPP_PORT_INPUT);
        LOGI("%s ME INPUT Stream OFF.\n", __func__);

        LOGI("Releasing registered buffers from driver on o/p port");
        bufreq.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        bufreq.memory = V4L2_MEMORY_USERPTR;
        bufreq.count = 0;
        ret = IOCTL(pstCb->MeFd, VIDIOC_REQBUFS, &bufreq);
        if (ret)
        {
            LOGE("ERROR: VIDIOC_REQBUFS INPUT MPLANE Failed, ret=%d", ret);
        }
        LOGI("%s ME INPUT REQBUFS done.\n", __func__);

        u32VppIpFrcMe_MeStreamOff(pstCb,VPP_PORT_OUTPUT);
        LOGI("%s ME OUTPUT Stream OFF.\n", __func__);

        LOGI("Releasing registered buffers from driver on o/p port");
        bufreq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        bufreq.memory = V4L2_MEMORY_USERPTR;
        bufreq.count = 0;
        ret = IOCTL(pstCb->MeFd, VIDIOC_REQBUFS, &bufreq);
        if (ret)
        {
            LOGE("ERROR: VIDIOC_REQBUFS OUTPUT MPLANE Failed, ret=%d", ret);
        }
        LOGI("%s ME OUTPUT REQBUFS done.\n", __func__);

        //Stop ME
        if(!eventfd_write(pstCb->MePollNotification, 1))
        {
            if (pstCb->MeThread !=  0)
            {
                ret = pthread_join(pstCb->MeThread, NULL);
                if (ret)
                    LOGE("Error: Receiver pthread_join failed: %d --> %s", ret, strerror(ret));
            }
        }
        pstCb->u32MeEvent = 0;
        LOGI("%s ME Stopped.\n", __func__);
        LOGI("%s Receiver thread terminated.\n", __func__);
        pstCb->MeThread = 0;

        //Unsubscribe events
        LOGI("ME Unsubscribe events");
        struct v4l2_event_subscription sub;
        int array_sz = sizeof(aMeEventType)/sizeof(int);
        int i,rc;

        for (i = 0; i < array_sz; ++i) {
            memset(&sub, 0, sizeof(sub));
            sub.type = aMeEventType[i];
            rc = IOCTL(pstCb->MeFd, VIDIOC_UNSUBSCRIBE_EVENT, &sub);

            if (rc) {
                LOGE("Error: Failed to unsubscribe event: 0x%x", sub.type);
            }
        }

        ret = pthread_cond_destroy(&pstCb->MeCond);
        if (ret)
            LOGE("Error: MeCond pthread_cond_destroy failed: %d --> %s", ret, strerror(ret));

        LOGI("InputBufsMEGot=%d, OutputBufsMEGot=%d, InputEosCnt=%d\n",pstCb->InputBufsMEGot,
              pstCb->OutputBufsMEGot,pstCb->InputEosCnt);
        LOGI("InputBufsMEQueued=%d, OutputBufsMEQueued=%d\n",pstCb->InputBufsMEQueued,
              pstCb->OutputBufsMEQueued);
        LOGI("InputBufsMEInside=%d, OutputBufsMEInside=%d\n",pstCb->InputBufsMEInside,
              pstCb->OutputBufsMEInside);
        LOGI("InputBufsMEDone=%d, OutputBufsMEDone=%d\n",pstCb->InputBufsMEDone,
              pstCb->OutputBufsMEDone);
        memset(&pstCb->MeCond,0,sizeof(pthread_cond_t));
        ret = u32VppIon_Free(pstCb->stBase.pstCtx, &pstCb->stIonBuf);
        if (ret)
            LOGE("Error: Unable to Free EOS Drain input buffer ret=%d", ret);
        pstCb->u32InternalFlags &= ~IP_FLUSH_PENDING;
    }

    close(pstCb->MePollNotification);
    pstCb->MePollNotification = -1;
    close(pstCb->MeFd);
    pstCb->MeFd = -1;

    LOG_EXITI_RET(VPP_OK);
}

/*!
 * @brief       ME ReadBack.
 *
 * @description Read back ME parameters.
 *
 * @input       pstCb    ME Context
 *
 * @return     VPP_OK
 *
 */
static uint32_t u32VppIpFrcMe_MeReadBack(t_StVppIpFrcMeCb *pstCb)
{
    int32_t idx;
    uint32_t u32Ret = VPP_OK;
    int ret;
    struct v4l2_control control;
    struct v4l2_ext_control acontrol[4];
    struct v4l2_ext_controls controls;
    struct v4l2_streamparm parm;

    LOGI("%s\n", __func__);
    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);

    memset((void*)&control, 0, sizeof(control));
    memset((void*)&acontrol, 0, sizeof(acontrol));
    memset((void*)&controls, 0, sizeof(controls));
    memset((void*)&parm, 0, sizeof(parm));

    control.id = V4L2_CID_MPEG_VIDC_VIDEO_SECURE;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL,&control);
    LOGI("V4L2_CID_MPEG_VIDC_VIDEO_SECURE: ret=%d, val=%d\n",ret, control.value);

    parm.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_PARM, &parm);
    LOGI("Frame Rate: ret=%d, numerator=%d, denominator=%d\n",ret,
          parm.parm.output.timeperframe.numerator, parm.parm.output.timeperframe.denominator);

    control.id = V4L2_CID_MPEG_VIDEO_BITRATE;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDEO_BITRATE: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDC_VIDEO_EXTRADATA;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDC_VIDEO_EXTRADATA: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDEO_HEADER_MODE;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDEO_HEADER_MODE: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDC_VIDEO_REQUEST_SEQ_HEADER;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDC_VIDEO_REQUEST_SEQ_HEADER: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDEO_H264_PROFILE;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDEO_H264_PROFILE: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDEO_H264_LEVEL;
    ret=IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDEO_H264_LEVEL: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDC_VIDEO_IDR_PERIOD;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDC_VIDEO_IDR_PERIOD: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDC_VIDEO_NUM_P_FRAMES;
    ret=IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDC_VIDEO_NUM_P_FRAMES: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDC_VIDEO_NUM_B_FRAMES;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDC_VIDEO_NUM_B_FRAMES: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP;
    ret=IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_MODE;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_MODE: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDEO_H264_MIN_QP;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDEO_H264_MIN_QP: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDEO_H264_MAX_QP;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDEO_H264_MAX_QP: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDC_VIDEO_HIER_P_NUM_LAYERS;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDC_VIDEO_HIER_P_NUM_LAYERS: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDC_VIDEO_H264_NAL_SVC;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDC_VIDEO_H264_NAL_SVC: ret=%d, val=%d\n",ret, control.value);

    controls.count = 2;
    controls.ctrl_class = V4L2_CTRL_CLASS_MPEG;
    controls.controls = acontrol;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_EXT_CTRLS, &controls);
    LOGI("V4L2_CTRL_CLASS_MPEG: ret=%d, LTR_MODE=%d, LTRCNT=%d\n",ret,
          acontrol[0].value, acontrol[1].value);

    control.id = V4L2_CID_MPEG_VIDC_VIDEO_HYBRID_HIERP_MODE;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDC_VIDEO_HYBRID_HIERP_MODE: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDC_VIDEO_H264_NAL_SVC;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDC_VIDEO_H264_NAL_SVC: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDC_VIDEO_ROTATION;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDC_VIDEO_ROTATION: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_TIMESTAMP_MODE;
    ret=IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_TIMESTAMP_MODE: ret=%d, val=%d\n",
          ret, control.value);

    control.id = V4L2_CID_MPEG_VIDC_VIDEO_AU_DELIMITER;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDC_VIDEO_AU_DELIMITER: ret=%d, val=%d\n",ret, control.value);

    control.id = V4L2_CID_MPEG_VIDC_VIDEO_H264_VUI_TIMING_INFO;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDC_VIDEO_H264_VUI_TIMING_INFO: ret=%d, val=%d\n",ret, control.value);

    acontrol[0].id = V4L2_CID_MPEG_VIDC_VIDEO_I_FRAME_QP;
    acontrol[0].value = 1;  //AA?
    acontrol[1].id = V4L2_CID_MPEG_VIDC_VIDEO_P_FRAME_QP;
    acontrol[1].value = 1;  //AA?
    acontrol[2].id = V4L2_CID_MPEG_VIDC_VIDEO_B_FRAME_QP;
    acontrol[2].value = 1;  //AA?
    acontrol[3].id = V4L2_CID_MPEG_VIDC_VIDEO_ENABLE_INITIAL_QP;
    acontrol[3].value = 1;  //AA?

    controls.count = 4;
    controls.ctrl_class = V4L2_CTRL_CLASS_MPEG;
    controls.controls = acontrol;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_EXT_CTRLS, &controls);
    LOGI("V4L2_CTRL_CLASS_MPEG: ret=%d, I_FRAME=%d, P_FRAME=%d, B_FRAME=%d, ENABLE_INIT=%d\n",
          ret, acontrol[0].value, acontrol[1].value, acontrol[2].value, acontrol[3].value);

    control.id = V4L2_CID_MPEG_VIDEO_BITRATE_PEAK;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_CTRL, &control);
    LOGI("V4L2_CID_MPEG_VIDEO_BITRATE_PEAK: ret=%d, val=%d\n",ret, control.value);
    LOG_EXITI_RET(VPP_OK);
}


/*!
 * @brief       ME Open.
 *
 * @description Open ME and configure it as HEVC.
 *
 * @input       pstCb    ME Context
 *
 * @return     VPP_OK for success, VPP_ERR for error
 *
 */
static uint32_t u32VppIpFrcMe_MeOpen_HEVC(t_StVppIpFrcMeCb *pstCb)
{
    int32_t idx;
    uint32_t u32Ret = VPP_OK;
    int ret;
    struct v4l2_control control;
    struct v4l2_ext_control acontrol[4];
    struct v4l2_ext_controls controls;
    struct v4l2_streamparm parm;
    char buffer[10];

    LOGI("%s\n", __func__);
    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);

    memset((void*)&control, 0, sizeof(control));
    memset((void*)&acontrol, 0, sizeof(acontrol));
    memset((void*)&controls, 0, sizeof(controls));
    memset((void*)&parm, 0, sizeof(parm));

    memset((void*)pstCb->pstBufIn, 0, sizeof(pstCb->pstBufIn));
    memset((void*)pstCb->pstBufOut, 0, sizeof(pstCb->pstBufOut));
    memset((void*)&pstCb->aMeStream, 0, sizeof(pstCb->aMeStream));
    memset((void*)&pstCb->aMePrepareOutput, 0, sizeof(pstCb->aMePrepareOutput));
    memset((void*)&pstCb->stBufEos, 0, sizeof(pstCb->stBufEos));
    memset((void*)&pstCb->BufEos, 0, sizeof(pstCb->BufEos));
    memset((void*)&pstCb->stIonBuf, 0, sizeof(pstCb->stIonBuf));

    pstCb->stBufEos.pBuf = &(pstCb->BufEos);
    pstCb->u32MeFlags = 0;
    pstCb->u32MeFoundBypass = 0;
    pstCb->u32MeWorkerErr = 0;
    pstCb->u32MeEvent = 0;
    pstCb->u32MeCmd = 0;
    pstCb->u32MeStage = VPP_FRC_ME_STAGE_START;
    pstCb->u32MeProcInQCnt = 0;
    pstCb->u32MeProcOutQCnt = 0;
    pstCb->MeFpsNum = 0;
    pstCb->MeFpsDen = 0;
    pstCb->MeTargetBitRate = 0;
    pstCb->MeInputFormat = 0;
    pstCb->MeCodecType = 0;
    pstCb->MeCodecProfile = 0;
    pstCb->MeProfileLevel = 0;
    pstCb->MeSessionGpRangeMinQp = 0;
    pstCb->MeSessionGpRangeMaxQp = 0;
    pstCb->MeIsThuliumV1 = 0;
    pstCb->MeLastTimestamp = 0;

    memset((void*)&pstCb->stMeCapabilities, 0, sizeof(pstCb->stMeCapabilities));
    memset((void*)&pstCb->stMeFormat, 0, sizeof(pstCb->stMeFormat));
    memset((void*)&pstCb->stMeBufReq, 0, sizeof(pstCb->stMeBufReq));
    memset((void*)&pstCb->stMeInputBuffProperty, 0, sizeof(pstCb->stMeInputBuffProperty));
    memset((void*)&pstCb->stMeOutputBuffProperty, 0, sizeof(pstCb->stMeOutputBuffProperty));
    memset((void*)&pstCb->stMeFrameSize, 0, sizeof(pstCb->stMeFrameSize));
    memset((void*)&pstCb->stMeFormat_Input, 0, sizeof(pstCb->stMeFormat_Input));
    memset((void*)&pstCb->stMeFormat_Output, 0, sizeof(pstCb->stMeFormat_Output));
    memset((void*)&pstCb->InputTimestampList, 0, sizeof(pstCb->InputTimestampList));

    pstCb->InputEosCnt = 0;
    pstCb->InputGotEosCnt = 0;
    pstCb->InputBufsMEGot = 0;
    pstCb->OutputBufsMEGot = 0;
    pstCb->InputBufsMEQueued = 0;
    pstCb->OutputBufsMEQueued = 0;
    pstCb->InputBufsMEInside = 0;
    pstCb->OutputBufsMEInside = 0;
    pstCb->InputBufsMEDone = 0;
    pstCb->OutputBufsMEDone = 0;
    pstCb->OutputBufsInternalPool = FRC_ME_DEFAULT_OUT_BUFS;
    pstCb->InputBufsNeed = FRC_ME_MAX_IN_BUFS - 1;            //One buffer reserved for EOS Drain buffer
    pstCb->OutputBufsNeed = pstCb->OutputBufsInternalPool;

    pstCb->InputBufsInternalNeed = FRC_ME_MAX_IN_BUFS - 1;    //One buffer reserved for EOS Drain buffer
    pstCb->OutputBufsInternalNeed = pstCb->OutputBufsInternalPool;

    if (pstCb->u32MeStatus != VPP_FRC_ME_INITED)
    {
        LOGE("Error: ME cannot be Opened in this state = %d\n", pstCb->u32MeStatus);
        LOG_EXITI_RET(VPP_ERR_STATE);
    }

    //Set Frame Rate
    pstCb->MeFpsNum = 1;
    pstCb->MeFpsDen = 60;

    //Set Encoder format
    pstCb->MeCodecType = V4L2_PIX_FMT_HEVC;
    pstCb->MeInputFormat = V4L2_PIX_FMT_NV12;

    //Subscribe to the events
    struct v4l2_event_subscription stMeSub;
    int i;
    for (i = 0; i < aMeEventType_size; i++)
    {
        memset(&stMeSub,0,sizeof(stMeSub));
        stMeSub.type=aMeEventType[i];
        ret = IOCTL(pstCb->MeFd, VIDIOC_SUBSCRIBE_EVENT, &stMeSub);
        if (ret)
        {
            LOGE("Error: Me failed to subscribe to the event 0x%x",stMeSub.type);
            LOG_EXITI_RET(VPP_ERR_STATE);
        }
    }
    LOGI("ME Events subscription done.");

    //Request ME capabilities
    ret = IOCTL(pstCb->MeFd, VIDIOC_QUERYCAP, &pstCb->stMeCapabilities);
    if (ret)
    {
        LOGE("Error: Me failure to request capabilities, ret=%d", ret);
        LOG_EXITI_RET(VPP_ERR_STATE);
    }
    else
    {
        LOGI("ME capabilities: name=%s, card=%s, bus=%s, ver=%d, cap=0x%x",
              pstCb->stMeCapabilities.driver, pstCb->stMeCapabilities.card,
              pstCb->stMeCapabilities.bus_info,
              pstCb->stMeCapabilities.version, pstCb->stMeCapabilities.capabilities);
    }

    pstCb->stMeOutputBuffProperty.alignment = SZ_4K;
    pstCb->stMeInputBuffProperty.alignment = SZ_4K;

    struct v4l2_format stMeFormat;

    //Set Output format
    memset(&stMeFormat, 0, sizeof(stMeFormat));
    stMeFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    stMeFormat.fmt.pix_mp.height = pstCb->stOutput.stParam.height;
    stMeFormat.fmt.pix_mp.width = pstCb->stOutput.stParam.width;
    stMeFormat.fmt.pix_mp.pixelformat = pstCb->MeCodecType;
    ret = IOCTL(pstCb->MeFd, VIDIOC_S_FMT, &stMeFormat);
    if (ret)
    {
        LOGE("Error: Failed to set the output format, ret=%d", ret);
        LOG_EXITI_RET(VPP_ERR_STATE);
    }
    pstCb->stMeOutputBuffProperty.datasize=stMeFormat.fmt.pix_mp.plane_fmt[0].sizeimage;
    LOGI("Output image size = %d", stMeFormat.fmt.pix_mp.plane_fmt[0].sizeimage);

    //Set input port format
    memset(&stMeFormat, 0, sizeof(stMeFormat));
    stMeFormat.type=V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    stMeFormat.fmt.pix_mp.height = pstCb->stInput.stParam.height;
    stMeFormat.fmt.pix_mp.width = pstCb->stInput.stParam.width;
    stMeFormat.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12;
    stMeFormat.fmt.pix_mp.plane_fmt[0].bytesperline = VENUS_Y_STRIDE(COLOR_FMT_NV12,
                                                                   pstCb->stInput.stParam.width);
    stMeFormat.fmt.pix_mp.plane_fmt[0].reserved[0] = VENUS_Y_SCANLINES(COLOR_FMT_NV12,
                                                                     pstCb->stInput.stParam.height);
    ret = IOCTL(pstCb->MeFd, VIDIOC_S_FMT, &stMeFormat);
    if (ret)
    {
        LOGE("Error: Failed to set the input format, ret=%d", ret);
        LOG_EXITI_RET(VPP_ERR_STATE);
    }
    pstCb->stMeInputBuffProperty.datasize = stMeFormat.fmt.pix_mp.plane_fmt[0].sizeimage;
    LOGI("Input image size = %d", stMeFormat.fmt.pix_mp.plane_fmt[0].sizeimage);

#if FRC_ME_MBI_TEST
    //Enable MBI extradata
    control.id = V4L2_CID_MPEG_VIDC_VIDEO_EXTRADATA;
    control.value = V4L2_MPEG_VIDC_EXTRADATA_METADATA_MBI;
    ret = IOCTL(pstCb->MeFd, VIDIOC_S_CTRL, &control);
    if (ret)
    {
        LOGE("Error: Failed to enable MBI, ret=%d", ret);
        LOG_EXITI_RET(VPP_ERR_STATE);
    }

    //Set MBI extradata
    control.id = V4L2_CID_MPEG_VIDC_VIDEO_MBI_STATISTICS_MODE;
    control.value = V4L2_CID_MPEG_VIDC_VIDEO_MBI_MODE_3;
    ret = IOCTL(pstCb->MeFd, VIDIOC_S_CTRL, &control);
    if (ret)
    {
        LOGE("Error: Failed to set MBI mode 3, ret=%d", ret);
        LOG_EXITI_RET(VPP_ERR_STATE);
    }

#endif

    //Request number of input buffers
    pstCb->stMeBufReq.memory = V4L2_MEMORY_USERPTR;
    pstCb->stMeBufReq.count = FRC_ME_MAX_IN_BUFS - 1;  //One buffer is reserved for EOS Drain buffer
    pstCb->stMeBufReq.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    ret = IOCTL(pstCb->MeFd, VIDIOC_REQBUFS, &pstCb->stMeBufReq);
    if (ret)
    {
        LOGE("Error: Failed input VIDIOC_REQBUFS, ret=%d", ret);
        LOG_EXITI_RET(VPP_ERR_STATE);
    }
    pstCb->stMeInputBuffProperty.mincount = pstCb->stMeInputBuffProperty.actualcount =
                                                                           pstCb->stMeBufReq.count;
    LOGI("Input properties: mincount=%ld, datasize=%ld",pstCb->stMeInputBuffProperty.mincount,
          pstCb->stMeInputBuffProperty.datasize);

    //Request number of output buffers
    pstCb->stMeBufReq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    pstCb->stMeBufReq.count = pstCb->OutputBufsInternalPool;     //Min is 2 buffers
    ret = IOCTL(pstCb->MeFd, VIDIOC_REQBUFS, &pstCb->stMeBufReq);
    if (ret)
    {
        LOGE("Error: Failed Output VIDIOC_REQBUFS, ret=%d", ret);
        LOG_EXITI_RET(VPP_ERR_STATE);
    }
    pstCb->stMeOutputBuffProperty.mincount=pstCb->stMeOutputBuffProperty.actualcount =
                                                                           pstCb->stMeBufReq.count;
    LOGI("Output properties: mincount=%ld, datasize=%ld",pstCb->stMeOutputBuffProperty.mincount,
          pstCb->stMeOutputBuffProperty.datasize);

    //Open secure device
    if (pstCb->stBase.bSecureSession)
    {
        control.id = V4L2_CID_MPEG_VIDC_VIDEO_SECURE;
        control.value = 1;
        LOGI("%s Open Secure device",__func__);
        ret = IOCTL(pstCb->MeFd, VIDIOC_S_CTRL, &control);
        if (ret)
        {
            LOGE("Error: Open secure device failed, ret=%d", ret);
            LOG_EXITI_RET(VPP_ERR_STATE);
        }
    }

    //Get HW capabilities
    memset(&pstCb->stMeFrameSize,0,sizeof(pstCb->stMeFrameSize));
    pstCb->stMeFrameSize.index = 0;
    pstCb->stMeFrameSize.pixel_format = pstCb->MeCodecType;
    ret = IOCTL(pstCb->MeFd, VIDIOC_ENUM_FRAMESIZES, &pstCb->stMeFrameSize);
    if (ret || (pstCb->stMeFrameSize.type!=V4L2_FRMSIZE_TYPE_STEPWISE))
    {
        LOGE("Error: Failed to get frame size, ret=%d", ret);
        LOG_EXITI_RET(VPP_ERR_STATE);
    }
    LOGI("VIDIOC_ENUM_FRAMESIZES : idx=%d, format=%d, type=%d", pstCb->stMeFrameSize.index,
          pstCb->stMeFrameSize.pixel_format, pstCb->stMeFrameSize.type);

    //Set frame rate
    parm.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    parm.parm.output.timeperframe.numerator = pstCb->MeFpsNum;
    parm.parm.output.timeperframe.denominator = pstCb->MeFpsDen;
    ret = IOCTL(pstCb->MeFd, VIDIOC_S_PARM, &parm);
    if (ret)
    {
        LOGE("Error: Failed to set frame rate, ret=%d", ret);
        LOG_EXITI_RET(VPP_ERR_STATE);
    }

    //Set P frames
    control.id = V4L2_CID_MPEG_VIDC_VIDEO_NUM_P_FRAMES;
#if FRC_ME_MBI_TEST
    control.value = INT_MAX;  //set infinite intra-period
#else
    control.value = 9;
#endif
    ret = IOCTL(pstCb->MeFd, VIDIOC_S_CTRL, &control);
    if (ret)
    {
        LOGE("Error: Failed to set V4L2_CID_MPEG_VIDC_VIDEO_NUM_P_FRAMES, ret=%d", ret);
        LOG_EXITI_RET(VPP_ERR_STATE);
    }

    //Set B frames
    control.id = V4L2_CID_MPEG_VIDC_VIDEO_NUM_B_FRAMES;
#if FRC_ME_MBI_TEST
    control.value = 0;  //disabled
#else
    control.value = 18;
#endif
    ret = IOCTL(pstCb->MeFd, VIDIOC_S_CTRL, &control);
    if (ret)
    {
        LOGE("Error: Failed to set V4L2_CID_MPEG_VIDC_VIDEO_NUM_B_FRAMES, ret=%d", ret);
        LOG_EXITI_RET(VPP_ERR_STATE);
    }

    //Set rate control
    control.id = V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL;
#if FRC_ME_MBI_TEST
    control.value = V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_OFF;
#else
    control.value = 2;
#endif
    ret = IOCTL(pstCb->MeFd, VIDIOC_S_CTRL, &control);
    if (ret)
    {
        LOGE("Error: Failed to set V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL, ret=%d", ret);
        LOG_EXITI_RET(VPP_ERR_STATE);
    }

    //-----------------------------------------------------------------------
    //Read back parameters
    //-----------------------------------------------------------------------

    LOGI("Read back contol parameters:\n");

    pstCb->stMeFormat_Output.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    pstCb->stMeFormat_Output.fmt.pix_mp.height = pstCb->stOutput.stParam.height;
    pstCb->stMeFormat_Output.fmt.pix_mp.width = pstCb->stOutput.stParam.width;
    pstCb->stMeFormat_Output.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_FMT, &pstCb->stMeFormat_Output);
    if (ret)
        LOGE("Error: Get Output Port Requirements Failed rc=%d", ret);
    else
    {
        LOGI("V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE: ret=%d, type=%d, w=%d, h=%d, format=%d, csc=%d,"
             "num_planes=%d, bytesperline=%d, scanlines=%d, sizeimage=%d, flags=%d\n",
              ret, pstCb->stMeFormat_Output.type, pstCb->stMeFormat_Output.fmt.pix_mp.width,
              pstCb->stMeFormat_Output.fmt.pix_mp.height,
              pstCb->stMeFormat_Output.fmt.pix_mp.pixelformat,
              pstCb->stMeFormat_Output.fmt.pix_mp.colorspace,
              pstCb->stMeFormat_Output.fmt.pix_mp.num_planes,
              pstCb->stMeFormat_Output.fmt.pix_mp.plane_fmt[0].bytesperline,
              pstCb->stMeFormat_Output.fmt.pix_mp.plane_fmt[0].reserved[0],
              pstCb->stMeFormat_Output.fmt.pix_mp.plane_fmt[0].sizeimage,
              pstCb->stMeFormat_Output.fmt.pix_mp.flags);
        LOGI("Output Data Size =%d\n", pstCb->stMeFormat_Output.fmt.pix_mp.plane_fmt[0].sizeimage);

#if FRC_ME_MBI_TEST
        if (pstCb->stMeFormat_Output.fmt.pix_mp.num_planes != FRC_ME_NO_PLANES)
        {
             LOGE("Error: Output ME no of planes should be %d and it is %d", FRC_ME_NO_PLANES,
                  pstCb->stMeFormat_Output.fmt.pix_mp.num_planes);
             LOG_EXITI_RET(VPP_ERR_INVALID_CFG);
        }
#endif

        if (pstCb->stMeFormat_Output.fmt.pix_mp.num_planes > 1)
        {
             unsigned int extra_idx = pstCb->stMeFormat_Output.fmt.pix_mp.num_planes - 1;
             LOGI("Output ExtraData Size =%d\n",
                  pstCb->stMeFormat_Output.fmt.pix_mp.plane_fmt[extra_idx].sizeimage);
        }
    }

    pstCb->stMeFormat_Input.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    pstCb->stMeFormat_Input.fmt.pix_mp.height = pstCb->stInput.stParam.height;
    pstCb->stMeFormat_Input.fmt.pix_mp.width = pstCb->stInput.stParam.width;
    pstCb->stMeFormat_Input.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12;
    ret = IOCTL(pstCb->MeFd, VIDIOC_G_FMT, &pstCb->stMeFormat_Input);
    if (ret)
        LOGE("Error: Get Input Port Requirements Failed rc=%d", ret);
    else
    {
        LOGI("V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE: ret=%d, type=%d, w=%d, h=%d, format=%d, csc=%d,"
             "num_planes=%d, bytesperline=%d, scanlines=%d, sizeimage=%d, flags=%d\n",
              ret, pstCb->stMeFormat_Input.type, pstCb->stMeFormat_Input.fmt.pix_mp.width,
              pstCb->stMeFormat_Input.fmt.pix_mp.height,
              pstCb->stMeFormat_Input.fmt.pix_mp.pixelformat,
              pstCb->stMeFormat_Input.fmt.pix_mp.colorspace,
              pstCb->stMeFormat_Input.fmt.pix_mp.num_planes,
              pstCb->stMeFormat_Input.fmt.pix_mp.plane_fmt[0].bytesperline,
              pstCb->stMeFormat_Input.fmt.pix_mp.plane_fmt[0].reserved[0],
              pstCb->stMeFormat_Input.fmt.pix_mp.plane_fmt[0].sizeimage,
              pstCb->stMeFormat_Input.fmt.pix_mp.flags);
        LOGI("Input Data Size =%d\n", pstCb->stMeFormat_Input.fmt.pix_mp.plane_fmt[0].sizeimage);

        if (pstCb->stMeFormat_Input.fmt.pix_mp.num_planes > 1)
        {
             unsigned int extra_idx = pstCb->stMeFormat_Input.fmt.pix_mp.num_planes - 1;
             LOGI("Input ExtraData Size =%d\n",
                   pstCb->stMeFormat_Input.fmt.pix_mp.plane_fmt[extra_idx].sizeimage);
        }
    }

#if DEBUG_READ_BACK
    u32VppIpFrcMe_MeReadBack(pstCb);
#endif

    //Alloc EOS Drain input buffer
    uint32_t u32Sz = u32VppUtils_GetBufferSize(&pstCb->stInput.stParam);

    ret = u32VppIon_Alloc(pstCb->stBase.pstCtx, u32Sz, pstCb->stBase.bSecureSession,&pstCb->stIonBuf);
    if (ret || (pstCb->stIonBuf.buf == NULL))
    {
        LOGE("Error: Unable to allocate EOS Drain input buffer: pstCtx=%p, "
             "secure=%d, size=%d, ret=%d",
              pstCb->stBase.pstCtx, pstCb->stBase.bSecureSession, u32Sz, ret);
        LOG_EXITI_RET(VPP_ERR_RESOURCES);
    }
    pstCb->stBufEos.stPixel.fd = pstCb->stIonBuf.fd_ion_mem;
    pstCb->stBufEos.stPixel.u32Offset = 0;
    pstCb->stBufEos.stPixel.u32AllocLen = u32Sz;
    pstCb->stBufEos.stPixel.u32FilledLen = u32Sz;
    pstCb->stBufEos.pBuf->flags = VPP_BUFFER_FLAG_EOS;
    if (!pstCb->stBase.bSecureSession)
    {
        pstCb->stBufEos.stPixel.u32MappedLen = u32Sz;
        pstCb->stBufEos.stPixel.pvBase = pstCb->stIonBuf.buf;
        memset(pstCb->stBufEos.stPixel.pvBase, 0, u32Sz);
    }
    else
    {
        pstCb->stBufEos.stPixel.u32MappedLen = 0;
        pstCb->stBufEos.stPixel.pvBase = VPP_BUF_UNMAPPED_BUF_VAL;
        LOGI("Allocated secure EOS drain buffer");
    }

    pstCb->stBufEos.u32InternalFlags = VPP_BUF_FLAG_ME_INTERN_EOS;
    pstCb->stBufEos.eBufType = eVppBufType_Progressive;
    pstCb->stBufEos.eBufPxType = eVppBufPxDataType_Raw;
    LOGI("Buffer EOS Drain buffer size %d is allocated.", u32Sz);

    //Allocate OS objects
    ret = pthread_cond_init(&pstCb->MeCond, NULL);
    if (ret)
    {
        LOGE("Error: Unable to init MeCond condition variable");
        LOG_EXITI_RET(VPP_ERR_RESOURCES);
    }

    ret = pthread_create(&pstCb->MeThread, NULL, vpVppIpFrcMe_MeReceiver, pstCb);
    if (ret)
    {
        LOGE("Error: Unable to spawn MeThread worker thread");
        ret = pthread_cond_destroy(&pstCb->MeCond);
        if (ret)
            LOGE("Error: MeCond pthread_cond_destroy failed: %d --> %s", ret, strerror(ret));
        LOG_EXITI_RET(VPP_ERR_RESOURCES);
    }

    //Increase the priority of the Receiver
#if FRC_ME_INC_PRI
    {
        int policy = SCHED_FIFO;
        int priority_max = 0;
        struct sched_param sched;

        priority_max = sched_get_priority_max(SCHED_FIFO);

        if (priority_max < 0)
           LOGE("Error: sched_get_priority_max failed");
        else
           sched.sched_priority = priority_max;

        ret = pthread_setschedparam(pstCb->thread, policy, &sched);
        if (ret != 0)
           LOGE("Error: pthread_setschedparam failed, ret=%d : %s",ret, strerror(ret));

        LOGI("Policy=%d, Priority=%d", policy, sched.sched_priority);
    }
#endif

    // Wait for the thread to launch before returning
    vVppIpFrcMe_MeWaitReceiverStart(pstCb);
    pstCb->u32MeStatus = VPP_FRC_ME_OPENED;
    LOG_EXITI_RET(VPP_OK);
}

/*!
 * @brief       Get Free index.
 *
 * @description Returns the first available free index. The caller must lock the mutex.
 *
 * @input       pstCb   ME Context
 * @input       ePort    ME Port
 *
 * @return     Free available index for success, FRC_ME_MAX_INDEX for error
 *
 */
static uint32_t u32VppIpFrcMe_MeGetFreeIndex(t_StVppIpFrcMeCb *pstCb,
                                                         enum vpp_port ePort)
{
    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);
    LOGIS("%s port=%d\n", __func__, ePort);

    if (ePort == VPP_PORT_OUTPUT)
    {
        //Output buffer
        int i = 0;
        for (i = 0; i < FRC_ME_MAX_OUT_BUFS; i++)
            if (pstCb->pstBufOut[i] == NULL)
                LOG_EXITI_RET(i);

        LOG_EXITI_RET(FRC_ME_MAX_INDEX);
    }
    else
    {
        //Input buffer
        int i = 0;
        for (i = 0; i < FRC_ME_MAX_IN_BUFS; i++)
            if (pstCb->pstBufIn[i] == NULL)
                LOG_EXITI_RET(i);

        LOG_EXITI_RET(FRC_ME_MAX_INDEX);
    }
}

/*!
 * @brief       Prepare buffer to be used by ME.
 *
 * @description Because we do not call STREAMOFF in FLUSH and DRAIN we cannot re-PREPARE output buffers.
 *
 * @input       pstCb   ME Context
 * @input       pstBuf  Buffer pointer
 * @input       ePort    ME Port
 *
 * @return     VPP_OK for success, VPP_ERR for error
 *
 */
static uint32_t u32VppIpFrcMe_MePrepareBuffer(t_StVppIpFrcMeCb *pstCb,
                                                           t_StVppBuf *pstBuf, enum vpp_port ePort)
{
    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);
    VPP_RET_IF_NULL(pstBuf, VPP_ERR_PARAM);
    LOGI("%s port=%d, buf_ixd=%d\n", __func__, ePort, pstBuf->u32Idx);

    u32VppIpFrcMe_LogBufferInfo(pstBuf);
    if (ePort == VPP_PORT_OUTPUT)
    {
        //Output buffer
        if (pstCb->aMePrepareOutput[pstBuf->u32Idx]==0)
        {
            int rc;
            struct v4l2_buffer buf;
            struct v4l2_plane plane[VIDEO_MAX_PLANES];

            memset(&buf, 0, sizeof(buf));
            memset(&plane, 0, sizeof(plane));

            buf.index = pstBuf->u32Idx;
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
            buf.memory = V4L2_MEMORY_USERPTR;
            plane[0].length      =  pstBuf->stPixel.u32AllocLen;
            plane[0].bytesused   =  0;
            if (!pstCb->stBase.bSecureSession)
                plane[0].m.userptr   = (unsigned long)((char*)pstBuf->stPixel.pvBase);
            else
                plane[0].m.userptr   = FRC_ME_V4L2_SECURE_UNMAPPED_BUF_VAL;
            plane[0].data_offset =  pstBuf->stPixel.u32Offset;
            plane[0].reserved[0] =  pstBuf->stPixel.fd;
            plane[0].reserved[1] =  0;
            buf.m.planes = plane;
            buf.flags = 0;
    #if FRC_ME_MBI_TEST
            buf.length = FRC_ME_NO_PLANES;  //num_planes;
            plane[1].length      =  pstBuf->stExtra.u32AllocLen;
            plane[1].bytesused   =  0;
            plane[1].m.userptr   = (unsigned long)((char*)pstBuf->stExtra.pvBase);
            plane[1].data_offset =  pstBuf->stExtra.u32Offset;
            plane[1].reserved[0] =  pstBuf->stExtra.fd;
            plane[1].reserved[1] =  0;
    #else
            buf.length = 1;  //num_planes;
    #endif
            rc = IOCTL(pstCb->MeFd, VIDIOC_PREPARE_BUF, &buf);
            if (rc)
            {
                LOGE("Error: OUTPUT PREPARE_BUF Failed index=%d, rc=%d", buf.index, rc);
                LOG_EXITI_RET(VPP_ERR);
            }
            pstCb->aMePrepareOutput[pstBuf->u32Idx]=1;
        }
    }
    else
    {
        //Input buffer
        int rc;
        struct v4l2_buffer buf;
        struct v4l2_plane plane[VIDEO_MAX_PLANES];

        memset(&buf, 0, sizeof(buf));
        memset(&plane, 0, sizeof(plane));

        buf.index = pstBuf->u32Idx;
        buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        buf.memory = V4L2_MEMORY_USERPTR;
        plane[0].length      = pstBuf->stPixel.u32AllocLen;
        plane[0].bytesused   =  0;
        if (!pstCb->stBase.bSecureSession)
            plane[0].m.userptr   = (unsigned long)((char*)pstBuf->stPixel.pvBase);
        else
            plane[0].m.userptr   = FRC_ME_V4L2_SECURE_UNMAPPED_BUF_VAL;
        plane[0].data_offset = pstBuf->stPixel.u32Offset;
        plane[0].reserved[0] = pstBuf->stPixel.fd;
        plane[0].reserved[1] = 0;
        buf.m.planes = plane;
        buf.length = 1;

        rc = IOCTL(pstCb->MeFd, VIDIOC_PREPARE_BUF, &buf);
        if (rc)
        {
            LOGE("Error: INPUT PREPARE_BUF Failed index=%d, rc=%d", buf.index, rc);
            LOG_EXITI_RET(VPP_ERR);
        }
    }

    LOG_EXITI_RET(VPP_OK);
}

/*!
 * @brief       ME Stream ON.
 *
 * @description It doesn't do STREAMON if already ON
 *
 * @input       pstCb   ME Context
 * @input       ePort    ME Port
 *
 * @return     VPP_OK for success, VPP_ERR for error
 *
 */
static uint32_t u32VppIpFrcMe_MeStreamOn(t_StVppIpFrcMeCb *pstCb, enum vpp_port ePort)
{
    t_StVppIpCmd stCmd;
    uint32_t u32;

    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);
    LOGI("%s port=%d Stream ON\n", __func__, ePort);

    if (ePort == VPP_PORT_OUTPUT)
    {
        //Output Port
        u32 = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    }
    else
    {
        //Input Port
        u32 = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    }

    if (pstCb->aMeStream[ePort] == 0)
    {
        int rc;

        rc = IOCTL(pstCb->MeFd, VIDIOC_STREAMON, &u32);
        if (rc)
        {
            LOGE("Error: VIDIOC_STREAMON (%d) Failed rc = %d", ePort, rc);
            LOG_EXITI_RET(VPP_ERR);
        }
        pstCb->aMeStream[ePort] = 1;
    }

    LOG_EXITI_RET(VPP_OK);
}

/*!
 * @brief       ME Stream OFF.
 *
 * @description
 *
 * @input       pstCb   ME Context
 * @input       ePort    ME Port
 *
 * @return     VPP_OK for success, VPP_ERR for error
 *
 */
static uint32_t u32VppIpFrcMe_MeStreamOff(t_StVppIpFrcMeCb *pstCb, enum vpp_port ePort)
{
    t_StVppIpCmd stCmd;

    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);
    LOGI("%s port=%d Stream OFF\n", __func__, ePort);

    if (ePort==VPP_PORT_OUTPUT)
    {
        //Output buffer
        int rc;
        uint32_t u32 = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

        rc = IOCTL(pstCb->MeFd, VIDIOC_STREAMOFF,&u32);
        if (rc)
        {
            LOGE("Error: VIDIOC_STREAMOFF (Output) Failed rc=%d", rc);
            LOG_EXITI_RET(VPP_ERR);
        }
        pstCb->aMeStream[VPP_PORT_OUTPUT]=0;
    }
    else
    {
        //Input buffer
        int rc;
        uint32_t u32 = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;

        rc = IOCTL(pstCb->MeFd, VIDIOC_STREAMOFF,&u32);
        if (rc) {
            LOGE("Error: VIDIOC_STREAMOFF (Input) Failed rc=%d", rc);
            LOG_EXITI_RET(VPP_ERR);
        }
        pstCb->aMeStream[VPP_PORT_INPUT]=0;
    }

    LOG_EXITI_RET(VPP_OK);
}

/*!
 * @brief       Queue buffer into ME.
 *
 * @description
 *
 * @input       pstCb   ME Context
 * @input       pstBuf  Buffer pointer
 * @input       ePort    ME Port
 *
 * @return     VPP_OK for success, VPP_ERR for error
 *
 */
static uint32_t u32VppIpFrcMe_MeQueueBuffer(t_StVppIpFrcMeCb *pstCb,t_StVppBuf *pstBuf,
                                                         enum vpp_port ePort)
{
    t_StVppIpCmd stCmd;

    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);
    VPP_RET_IF_NULL(pstBuf, VPP_ERR_PARAM);
    LOGI("%s port=%d, buf_idx=%d\n", __func__, ePort, pstBuf->u32Idx);

    u32VppIpFrcMe_LogBufferInfo(pstBuf);
    if (ePort==VPP_PORT_OUTPUT)
    {
        //Output buffer
        int rc;
        struct v4l2_buffer buf;
        struct v4l2_plane plane[VIDEO_MAX_PLANES];

        memset(&buf, 0, sizeof(buf));
        memset(&plane, 0, sizeof(plane));

        buf.index = pstBuf->u32Idx;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf.memory = V4L2_MEMORY_USERPTR;
        plane[0].length      =  pstBuf->stPixel.u32AllocLen;
        plane[0].bytesused   =  0;
        if (!pstCb->stBase.bSecureSession)
            plane[0].m.userptr   = (unsigned long)((char*)pstBuf->stPixel.pvBase);
        else
            plane[0].m.userptr   = FRC_ME_V4L2_SECURE_UNMAPPED_BUF_VAL;
        plane[0].data_offset =  pstBuf->stPixel.u32Offset;
        plane[0].reserved[0] =  pstBuf->stPixel.fd;
        plane[0].reserved[1] =  0;
        buf.m.planes = plane;
        buf.flags = 0;

#if FRC_ME_MBI_TEST
        buf.length = FRC_ME_NO_PLANES;  //num_planes;
        plane[1].length      =  pstBuf->stExtra.u32AllocLen;
        plane[1].bytesused   =  0;
        plane[1].m.userptr   = (unsigned long)((char*)pstBuf->stExtra.pvBase);
        plane[1].data_offset =  pstBuf->stExtra.u32Offset;
        plane[1].reserved[0] =  pstBuf->stExtra.fd;
        plane[1].reserved[1] =  0;
#else
        buf.length = 1;  //num_planes;
#endif

        LOGIS("buf.index=%d, buf.id=%d", buf.index, plane[0].reserved[0]);
        rc = IOCTL(pstCb->MeFd, VIDIOC_QBUF, &buf);
        if (rc) {
            LOGE("Error: VIDIOC_QBUF (Output idx=%d) Failed rc=%d", buf.index, rc);
            LOG_EXITI_RET(VPP_ERR);
        }
    }
    else
    {
        //Input buffer
        int rc;
        struct v4l2_buffer buf;
        struct v4l2_plane plane[VIDEO_MAX_PLANES];

        memset(&buf, 0, sizeof(buf));
        memset(&plane, 0, sizeof(plane));

        buf.index = pstBuf->u32Idx;
        buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        buf.memory = V4L2_MEMORY_USERPTR;
        plane[0].length      = pstBuf->stPixel.u32AllocLen;
        plane[0].bytesused   = pstBuf->stPixel.u32FilledLen;
        if (!pstCb->stBase.bSecureSession)
            plane[0].m.userptr   = (unsigned long)((char*)pstBuf->stPixel.pvBase);
        else
            plane[0].m.userptr   = FRC_ME_V4L2_SECURE_UNMAPPED_BUF_VAL;
        plane[0].data_offset = pstBuf->stPixel.u32Offset;
        plane[0].reserved[0] = pstBuf->stPixel.fd;
        plane[0].reserved[1] = 0;
        buf.m.planes = plane;
        buf.length = 1;
        buf.flags = 0;

        pthread_mutex_lock(&pstCb->mutex);
        if (pstBuf->pBuf->flags & VPP_BUFFER_FLAG_EOS)
        {
            pstCb->InputGotEosCnt++;
            buf.flags |= V4L2_QCOM_BUF_FLAG_EOS;
            if (pstBuf->u32InternalFlags & VPP_BUF_FLAG_ME_INTERN_EOS)
                LOGI("VIDIOC_QBUF Input EOS DRAIN detected! Flags=0x%x", pstBuf->pBuf->flags);
            else
                LOGI("VIDIOC_QBUF Input EOS detected! Flags=0x%x", pstBuf->pBuf->flags);
        }

        buf.timestamp.tv_sec = (pstBuf->pBuf->timestamp/ 1000000);
        buf.timestamp.tv_usec = (pstBuf->pBuf->timestamp% 1000000);

        u32VppIpFrcMe_TimestampListWrite(&(pstCb->InputTimestampList), pstBuf->pBuf->timestamp);
        pthread_mutex_unlock(&pstCb->mutex);

        LOGIS("buf.index=%d, buf.id=%d", buf.index, plane[0].reserved[0]);
        rc = IOCTL(pstCb->MeFd, VIDIOC_QBUF, &buf);
        if (rc)
        {
            LOGE("Error: VIDIOC_QBUF (Input idx=%d) Failed rc=%d", buf.index, rc);
            LOG_EXITI_RET(VPP_ERR);
        }
        VPP_IP_PROF_START(&pstCb->stBase, FRC_ME_STAT_ENC_1F);
    }

    LOG_EXITI_RET(VPP_OK);
}

/*!
 * @brief       Set Receiver thread started flag
 *
 * @description It sets the flag and signals the condition
 *
 * @input       pstCb   ME Context
 *
 * @return
 *
 */
static void vVppIpFrcMe_MeSignalReceiverStart(t_StVppIpFrcMeCb *pstCb)
{
    LOGI("%s\n", __func__);
    VPP_RET_VOID_IF_NULL(pstCb);
    pthread_mutex_lock(&pstCb->mutex);

    pstCb->u32MeFlags |= FRC_ME_RECEIVER_THREAD_STARTED;

    pthread_cond_signal(&pstCb->MeCond);

    pthread_mutex_unlock(&pstCb->mutex);
    LOGI("%s exit\n", __func__);
}

/*!
 * @brief       Wait Receiver thread started flag
 *
 * @description It gets the mutex and reads the flag when condition is signaled.
 *
 * @input       pstCb   ME Context
 *
 * @return
 *
 */
static void vVppIpFrcMe_MeWaitReceiverStart(t_StVppIpFrcMeCb *pstCb)
{
    LOGI("%s\n", __func__);
    VPP_RET_VOID_IF_NULL(pstCb);
    pthread_mutex_lock(&pstCb->mutex);

    while (!(pstCb->u32MeFlags & FRC_ME_RECEIVER_THREAD_STARTED))
        pthread_cond_wait(&pstCb->MeCond, &pstCb->mutex);

    pthread_mutex_unlock(&pstCb->mutex);
    LOGI("%s exit\n", __func__);
}

/*!
 * @brief       Sets Worker thread started flag
 *
 * @description Set a condition in order to wake up the waiting thread.
 *
 * @input       pstCb   ME Context
 *
 * @return
 *
 */
static void vVppIpFrcMe_SignalWorkerStart(t_StVppIpFrcMeCb *pstCb)
{
    LOGI("%s\n", __func__);
    VPP_RET_VOID_IF_NULL(pstCb);
    pthread_mutex_lock(&pstCb->mutex);

    pstCb->u32InternalFlags |= IP_WORKER_STARTED;

    pthread_cond_signal(&pstCb->cond);

    pthread_mutex_unlock(&pstCb->mutex);
}

/*!
 * @brief       Wait Worker thread started flag
 *
 * @description Get the mutex and check the flag when condition is signaled.
 *
 * @input       pstCb   ME Context
 *
 * @return
 *
 */
static void vVppIpFrcMe_WaitWorkerStart(t_StVppIpFrcMeCb *pstCb)
{
    LOGI("%s\n", __func__);
    VPP_RET_VOID_IF_NULL(pstCb);
    pthread_mutex_lock(&pstCb->mutex);

    while (!(pstCb->u32InternalFlags & IP_WORKER_STARTED))
        pthread_cond_wait(&pstCb->cond, &pstCb->mutex);

    pthread_mutex_unlock(&pstCb->mutex);
}

/*!
 * @brief  Log the queues status when the Worker Thread is closing
 *
 * @description The caller must lock the mutex
 *
 * @input       pstCb   ME Context
 *
 * @return     VPP_OK
 *
 */
static uint32_t u32VppIpFrcMe_ProcCmdExit(t_StVppIpFrcMeCb *pstCb)
{
    uint32_t u32InQSz, u32OutQSz;

    LOGI("%s\n", __func__);
    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);
    u32InQSz = u32VppBufPool_Cnt(&pstCb->stInput.stQ);
    u32OutQSz = u32VppBufPool_Cnt(&pstCb->stOutput.stQ);

    LOGD("Worker Thread closed: u32InQSz=%u, u32OutQSz=%u", u32InQSz, u32OutQSz);

    LOG_EXITI_RET(VPP_OK);
}

/*!
 * @brief       Get buffer from the PORT's queue
 *
 * @description The caller is supposed to lock the mutex which protects the queue
 *
 * @input       pstPort    pointer to the Port struct
 * @output     ppstBuf   destination address where the buffer pointer will be stored.
 *
 * @return    VPP_OK if a valid buffer is gotten,  VPP_ERR is error or no buffer available
 *
 */
static uint32_t u32VppIpFrcMe_PortBufGet(t_StVppIpFrcMePort *pstPort,
                                       t_StVppBuf **ppstBuf)
{
    int32_t idx;
    uint32_t u32Ret = VPP_OK;
    LOGIS("%s\n", __func__);
    VPP_RET_IF_NULL(pstPort, VPP_ERR_PARAM);

    *ppstBuf =  pstVppBufPool_Get(&pstPort->stQ);
    if((*ppstBuf) == NULL)
    {
        u32Ret=VPP_ERR;
    }

    LOG_EXITI_RET(u32Ret);
}

/*!
 * @brief       Put buffer into the PORT's queue
 *
 * @description The caller is supposed to lock the mutex which protects the queue
 *
 * @input       pstCb      ME Context
 * @input       pstPort    pointer to the Port struct
 * @input       pBuf        buffer pointer.
 *
 * @return    VPP_OK,  VPP_ERR if queue is full or error
 *
 */
static uint32_t u32VppIpFrcMe_PortBufPut(t_StVppIpFrcMeCb *pstCb,
                                       t_StVppIpFrcMePort *pstPort,
                                       t_StVppBuf *pBuf)
{
    int32_t idx;
    uint32_t u32Ret = VPP_OK;

    LOGIS("%s\n", __func__);
    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);
    VPP_RET_IF_NULL(pstPort, VPP_ERR_PARAM);

    u32Ret = u32VppBufPool_Put(&pstPort->stQ, pBuf);
    if (u32Ret != VPP_OK)
    {
        LOGE("Error: u32VppBufPool_Put() failed, ret=%d\n", u32Ret);
        u32Ret = VPP_ERR;
    }
    else
    {
        LOGIS("u32VppBufPool_Put() OK, ret=%d\n", u32Ret);
        u32Ret = VPP_OK;
        pthread_cond_signal(&pstCb->cond);
    }

    LOG_EXITI_RET(u32Ret);
}

/*!
 * @brief       Flush Port's queue
 *
 * @description All buffers found in the PORT's queue are released and the flush flag is set.
 *
 * @input       pstCb      ME Context
 * @input       pstPort    pointer to the Port struct
 *
 * @return    VPP_OK,  VPP_ERR or error
 *
 */
static uint32_t u32VppIpFrcMe_FlushPort(t_StVppIpFrcMeCb *pstCb, enum vpp_port ePort)
{
    t_StVppBuf *pBuf;
    t_StVppIpFrcMePort *pstPort;
    uint32_t u32Ret = VPP_OK;

    LOGI("%s(%d) Started.\n", __func__, (int)ePort);
    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);

    if (ePort == VPP_PORT_INPUT)
    {
        pstPort = &pstCb->stInput;
    }
    else if (ePort == VPP_PORT_OUTPUT)
    {
        pstPort = &pstCb->stOutput;
    }
    else
    {
        LOG_EXITI_RET(VPP_ERR_PARAM);
    }

    while (1)
    {
        pthread_mutex_lock(&pstCb->mutex);
        u32Ret=u32VppIpFrcMe_PortBufGet(pstPort, &pBuf);
        pthread_mutex_unlock(&pstCb->mutex);
        if (u32Ret == VPP_OK)
        {
            LOGI("%s : u32VppIp_CbBufDone()\n", __func__);
            VPP_FLAG_SET(pBuf->u32InternalFlags, VPP_BUF_FLAG_FLUSHED);
            pBuf->stPixel.u32FilledLen = 0;
            vVppIpCbLog(&pstCb->stBase.stCb, pBuf, eVppLogId_IpBufDone);
            u32VppIp_CbBufDone(&pstCb->stBase.stCb, ePort, pBuf);
        }
        else
            break;
    }

    LOGI("%s(%d) Done.\n", __func__, (int)ePort);
    LOG_EXITI_RET(VPP_OK);
}

/*!
 * @brief       ME Flush Command
 *
 * @description ME is flushed internally. The port specified by the cmd is flushed. Both streams are OFF.
 *                     ME is set in the START stage. Durring Flush no other cmd or buffer is accepted.
 *                     The caller must lock the mutex
 * @input       pstCb      ME Context
 * @input       pstCmd    pointer Command struct
 *
 * @return    VPP_OK,  VPP_ERR or error
 *
 */
static uint32_t u32VppIpFrcMe_ProcCmdFlush(t_StVppIpFrcMeCb *pstCb,
                                         t_StVppIpCmd *pstCmd)
{
    uint32_t u32;
    t_StVppEvt stEvt;
    int ret=0;

    LOGI("%s\n", __func__);
    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);
    VPP_RET_IF_NULL(pstCmd, VPP_ERR_PARAM);

    //Flush ME
    LOGI("ME Worker Blocking");
    while ((pstCb->u32InternalFlags & IP_FLUSH_PENDING)||
           (pstCb->u32InternalFlags & IP_DRAIN_PENDING)||
           (pstCb->u32InternalFlags & IP_EOS_PENDING))
    {
        pthread_cond_wait(&pstCb->cond, &pstCb->mutex);
    }
    pstCb->u32MeStage = VPP_FRC_ME_STAGE_FLUSH;
    pstCb->u32InternalFlags |= IP_FLUSH_PENDING;
    LOGI("ME Worker Running");
    if ((pstCb->u32MeStatus != VPP_FRC_ME_RUNNING_FLUSHED) &&
        (pstCb->u32MeStatus != VPP_FRC_ME_RUNNING_DRAINED))
    {
        struct v4l2_encoder_cmd enc;

        pthread_mutex_unlock(&pstCb->mutex);
        enc.cmd = V4L2_ENC_QCOM_CMD_FLUSH;
        //Venus Encoder requires FLUSH on both ports
        enc.flags = V4L2_QCOM_CMD_FLUSH_OUTPUT | V4L2_QCOM_CMD_FLUSH_CAPTURE;
        ret = IOCTL(pstCb->MeFd, VIDIOC_ENCODER_CMD, &enc);
        if (ret)
        {
            LOGE("Error: Flush ME Failed, port=%d, ret=%d", pstCmd->flush.ePort, ret);
        }
        LOGI("%s ME Internal Buffers Flush Command, port=%d\n", __func__, pstCmd->flush.ePort);
        LOGI("ME Worker Blocking");
        pthread_mutex_lock(&pstCb->mutex);
        while (pstCb->u32MeEvent != V4L2_EVENT_MSM_VIDC_FLUSH_DONE)
            pthread_cond_wait(&pstCb->cond, &pstCb->mutex);
        pstCb->u32MeEvent = 0;
        pthread_mutex_unlock(&pstCb->mutex);
        LOGI("ME Worker Running");
        LOGI("%s ME Internal Buffers port=%d flushed.\n", __func__, pstCmd->flush.ePort);
    }
    else
        pthread_mutex_unlock(&pstCb->mutex);

    //Flush port.
    u32 = u32VppIpFrcMe_FlushPort(pstCb, pstCmd->flush.ePort);
    if (u32 == VPP_OK)
    {
        stEvt.eType = VPP_EVT_FLUSH_DONE;
        stEvt.flush.ePort = pstCmd->flush.ePort;
        u32VppIpCbEvent(&pstCb->stBase.stCb, stEvt);
    }

    //This is to force resume processing buffers already in queues
    pthread_mutex_lock(&pstCb->mutex);
    pstCb->u32InternalFlags &= ~IP_FLUSH_PENDING;
    pstCb->u32MeStatus = VPP_FRC_ME_RUNNING_FLUSHED;
    pstCb->u32MeStage = VPP_FRC_ME_STAGE_START;
    pthread_cond_signal(&pstCb->cond);
    LOG_EXITI_RET(u32);
}

/*!
 * @brief       ME Drain Command
 *
 * @description One internal Drain EOS input buffer is injected into the input queue. The Drain pending flag is set.
 *                     No other cmd or input buffer is accepted at the input during Drain.
 *                     The caller must lock the mutex
 * @input       pstCb      ME Context
 *
 * @return    VPP_OK,  VPP_ERR or error
 *
 */
static uint32_t u32VppIpFrcMe_ProcCmdDrain(t_StVppIpFrcMeCb *pstCb)
{
    t_StVppEvt stEvt;

    LOGI("%s\n", __func__);
    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);
    uint32_t u32Ret;

    pstCb->u32InternalFlags |= IP_DRAIN_PENDING;
    pstCb->stBufEos.pBuf->timestamp = pstCb->MeLastTimestamp + (FRC_ME_DEFAULT_DELAY / 2);
    u32Ret = u32VppIpFrcMe_PortBufPut(pstCb, &pstCb->stInput, &pstCb->stBufEos);
    FRC_ME_STATS(pstCb, InQCnt);

    LOG_EXITI_RET(VPP_OK);
}

/*!
 * @brief       Check the buffer requirements
 *
 * @description Checks if the available buffers are enough for processing in the current stage.
 *                     This function requires that the caller has already locked the mutex which guards these two queues.
 * @input       pstCb      ME Context
 *
 * @return    VPP_TRUE if the available buffers are enough for processing,  VPP_FALSE if buffers are not available
 *
 */
static uint32_t u32ProcBufReqMet(t_StVppIpFrcMeCb *pstCb)
{
    // Determine if the buffers in the ports satisfy the requirements to trigger processing
    t_StVppBuf *pstBuf;
    uint32_t ret=0;
    uint32_t u32BypassFlags;

    pstCb->u32MeFoundBypass = 0;
    pstCb->u32MeFoundEos = 0;
    pstCb->u32MeProcInQCnt = u32VppBufPool_Cnt(&pstCb->stInput.stQ);
    pstCb->u32MeProcOutQCnt = u32VppBufPool_Cnt(&pstCb->stOutput.stQ);
    if (pstCb->u32MeProcInQCnt)
    {
        t_StVppBuf * bufc = pstVppBufPool_Peek(&pstCb->stInput.stQ, 0);
        if (bufc != NULL)
        {
            u32BypassFlags = VPP_BUF_FLAG_BYPASS | VPP_BUF_FLAG_ME_INTERN_BYPASS;
            if ((VPP_FLAG_IS_SET(bufc->u32InternalFlags, u32BypassFlags)) ||
                (bufc->eBufType != eVppBufType_Progressive))
                pstCb->u32MeFoundBypass = 1;

            if (VPP_FLAG_IS_SET(bufc->pBuf->flags, VPP_BUFFER_FLAG_EOS))
            {
                if (bufc->stPixel.u32FilledLen)
                    pstCb->u32MeFoundEos = 1;
            }
            if ((!pstCb->u32MeFoundBypass) && (pstCb->u32MeStage == VPP_FRC_ME_STAGE_BYPASS) &&
                (!(VPP_FLAG_IS_SET(pstCb->u32InternalFlags, IP_DRAIN_PENDING))))
                pstCb->u32MeStage = VPP_FRC_ME_STAGE_START;
        }
    }

    if (pstCb->u32MeStatus == VPP_FRC_ME_TERMED)
        ret = VPP_FALSE;
    else if ((pstCb->u32MeFoundBypass) && ((pstCb->u32MeStage == VPP_FRC_ME_STAGE_START) ||
              (pstCb->u32MeStage == VPP_FRC_ME_STAGE_FEED)))
    {
        if (pstCb->InputBufsInternalNeed > 0)
        {
            pstCb->u32MeStage = VPP_FRC_ME_STAGE_BYPASS;
            ret = VPP_TRUE;
        }
        else
            ret = VPP_FALSE;
    }
    else if ((pstCb->u32MeStage == VPP_FRC_ME_STAGE_BYPASS) &&
             (!(VPP_FLAG_IS_SET(pstCb->u32InternalFlags, IP_FLUSH_PENDING))))
    {
        if (((pstCb->u32MeProcInQCnt) && (pstCb->InputBufsInternalNeed > 0) &&
            (!(VPP_FLAG_IS_SET(pstCb->u32InternalFlags, IP_DRAIN_PENDING)))) ||
            ((pstCb->u32MeProcOutQCnt) && (pstCb->OutputBufsInternalNeed > 0) &&
            (VPP_FLAG_IS_SET(pstCb->u32InternalFlags, IP_DRAIN_PENDING))))
            ret = VPP_TRUE;
        else
            ret = VPP_FALSE;
    }
    else if (pstCb->u32MeStage == VPP_FRC_ME_STAGE_START)
    {
        if ((pstCb->u32MeProcInQCnt >= FRC_ME_START_IN_BUFS) && (!(pstCb->InputGotEosCnt)) &&
            ((int)pstCb->u32MeProcOutQCnt >= pstCb->OutputBufsInternalPool))
            ret = VPP_TRUE;
        else
            ret = VPP_FALSE;
    }
    else if (pstCb->u32MeStage == VPP_FRC_ME_STAGE_FEED)
    {
        if (((pstCb->u32MeProcInQCnt) && (pstCb->InputBufsInternalNeed > 0) &&
            (!(pstCb->InputGotEosCnt))) || ((pstCb->u32MeProcOutQCnt) &&
            (pstCb->OutputBufsInternalNeed > 0)))
            ret = VPP_TRUE;
        else
            ret = VPP_FALSE;
    }
    else
        ret = VPP_FALSE;
    u32VppIpFrcMe_LogStatus(pstCb, pstCb->u32MeProcInQCnt, pstCb->u32MeProcOutQCnt);
    LOG_EXITI_RET(ret);
}

/*!
 * @brief       Check if the Working thread needs to sleep
 *
 * @description This function requires that the caller has already locked the mutex which guards the queues.
 *
 * @input       pstCb      ME Context
 *
 * @return    VPP_TRUE if the Working thread needs to sleep,  VPP_FALSE  if the Working thread needs to process
 *
 */
static uint32_t u32WorkerThreadShouldSleep(t_StVppIpFrcMeCb *pstCb)
{
    uint32_t u32Ret = VPP_TRUE;
    uint32_t u32CmdQSz, u32ProcMet;

    LOGIS("%s\n", __func__);
    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);

    // This is a predicate function used for determining if the function worker
    // thread should sleep or not. Worker thread uses a condition variable to
    // wake itself and the mutex which is used is the same as that which guards
    // these functions. Therefore, there is no need to lock a mutex prior to
    // checking the command queues within this context.

    vVppIpFrcMe_HandleStartPendingFlushDrain(pstCb);
    u32CmdQSz = vpp_queue_count(&pstCb->stCmdQ);
    u32ProcMet = u32ProcBufReqMet(pstCb);

    if ((u32CmdQSz)&&
        (!(pstCb->u32InternalFlags & IP_FLUSH_PENDING)) &&
        (!(pstCb->u32InternalFlags & IP_DRAIN_PENDING)) &&
        (!(pstCb->u32InternalFlags & IP_EOS_PENDING)) && (pstCb->InputEosCnt <= 0))
    {
        u32Ret = VPP_FALSE;
    }
    else if (pstCb->eState == VPP_IP_STATE_ACTIVE && u32ProcMet)
    {
        u32Ret = VPP_FALSE;
    }

    LOGI("CHECK: shouldSleep=%u, u32CmdQSz=%u, u32ProcMet=%u", u32Ret,
         u32CmdQSz, u32ProcMet);

    LOG_EXITI_RET(u32Ret);
}

/*!
 * @brief       Print buffer parameters
 *
 * @description
 *
 * @input       pstBuf      buffer pointer
 *
 * @return    VPP_OK
 *
 */
static uint32_t u32VppIpFrcMe_LogBufferInfo(t_StVppBuf *pstBuf)
{
    VPP_RET_IF_NULL(pstBuf, VPP_ERR_PARAM);
    LOGI("Idx=%d, pstBuf=%p, pBuf=%p, Flags=0x%x, BufType=%d, BufPxType=%d\n", pstBuf->u32Idx,
          pstBuf, pstBuf->pBuf, pstBuf->u32InternalFlags, pstBuf->eBufType, pstBuf->eBufPxType);
    LOGI("Pixel: Base=%p, MappedLen=%d, flags=0x%x, timestamp=0x%llx\n", pstBuf->stPixel.pvBase,
          pstBuf->stPixel.u32MappedLen, pstBuf->pBuf->flags,
          (long long unsigned int)pstBuf->pBuf->timestamp);
    if (pstBuf->pBuf != NULL)
    {
        LOGIS("Pixel: fd=%d, offset=0x%x, u32AllocLen=%d, u32FilledLen=%d\n", pstBuf->pBuf->pixel.fd,
               pstBuf->pBuf->pixel.offset, pstBuf->stPixel.u32AllocLen,
               pstBuf->stPixel.u32FilledLen);
    }
    LOGIS("Extra: Base=%p, MappedLen=%d\n", pstBuf->stExtra.pvBase, pstBuf->stExtra.u32MappedLen);
    if (pstBuf->pBuf != NULL)
    {
        LOGIS("Extra: fd=%d, offset=0x%x, u32AllocLen=%d, u32FilledLen=%d\n",
               pstBuf->stExtra.fd, pstBuf->stExtra.u32Offset,
               pstBuf->stExtra.u32AllocLen, pstBuf->stExtra.u32FilledLen);
    }

    return(VPP_OK);
}

#if VPP_FRC_ME_DUMP_BUF_ENABLE
void vVppIpFrcMe_DumpBuffer(t_StVppIpFrcMeCb *pstCb,
                            t_StVppBuf *pstBuf,
                            t_EVppIpFrcMeDumpBuf eBuf,
                            uint32_t idx)
{
    char cPath[VPP_FRC_DUMP_NM_LEN];
    struct vpp_port_param *pstParam;
    uint32_t u32Stride, u32Width, u32Height, u32UvOffset;

    VPP_RET_VOID_IF_NULL(pstCb);
    VPP_RET_VOID_IF_NULL(pstBuf);

    if (pstCb->stBase.bSecureSession)
        return;

    if (((eBuf==VPP_IP_FRC_BUF_ME_YUV_IN) &&
         (VPP_FLAG_IS_SET(pstBuf->u32InternalFlags, VPP_BUF_FLAG_FLUSHED)))||
         ((eBuf==VPP_IP_FRC_BUF_ME_MBI_OUT) && (pstBuf->stExtra.u32FilledLen==0)))
        return;

    LOG_ENTER_ARGS("eBuf=%u", eBuf);

    switch (eBuf)
    {
        case VPP_IP_FRC_BUF_ME_YUV_IN:
            snprintf(cPath, VPP_FRC_DUMP_NM_LEN, VPP_FRC_DUMP_BUF_ME_IN_YUV_NM,
                     pstCb, idx);
            pstParam = &pstCb->stInput.stParam;
            break;
        case VPP_IP_FRC_BUF_ME_YUV_OUT:
            snprintf(cPath, VPP_FRC_DUMP_NM_LEN, VPP_FRC_DUMP_BUF_ME_OUT_YUV_NM,
                     pstCb, idx);
            pstParam = &pstCb->stOutput.stParam;
            break;
        case VPP_IP_FRC_BUF_ME_MBI_OUT:
            snprintf(cPath, VPP_FRC_DUMP_NM_LEN, VPP_FRC_DUMP_BUF_ME_OUT_MBI_NM,
                     pstCb, idx);
            break;
        default:
            LOGE("%s Error: Wrong eBuf=%d", __func__, eBuf);
            return;
    }

    if (eBuf == VPP_IP_FRC_BUF_ME_MBI_OUT)
    {
        u32VppBuf_DumpExtraData(pstCb->stBase.pstCtx, pstBuf, cPath);
    }
    else
    {
        u32Stride = u32VppUtils_GetStride(pstParam);
        u32Width = pstParam->width;
        u32Height = pstParam->height;
        u32UvOffset = u32VppUtils_GetUVOffset(pstParam);

        LOGI("w=%d, h=%d, stride=%d, UVoffset=%d", u32Width, u32Height, u32Stride, u32UvOffset);
        u32VppBuf_Dump(pstCb->stBase.pstCtx, pstBuf, cPath, u32Width, u32Height,
                       u32Stride, u32UvOffset);
    }

    LOG_EXIT();
}
#endif

#if VPP_FRC_ME_ZERO_MBI_ENABLE
uint32_t u32VppIpFrcMe_ZeroBuffer(t_StVppIpFrcMeCb *pstCb,
                                 t_StVppBuf *pstBuf,
                                 t_EVppIpFrcMeDumpBuf eBuf,
                                 uint32_t idx)
{
    uint32_t size=0;
    char *base=NULL;
    char *virt=NULL;

    VPP_RET_IF_NULL(pstCb,0);
    VPP_RET_IF_NULL(pstBuf,0);

    if (pstCb->stBase.bSecureSession)
        return(0);

    LOG_ENTER_ARGS("eBuf=%u", eBuf);

    switch (eBuf)
    {
        case VPP_IP_FRC_BUF_ME_YUV_IN:
            virt = pstBuf->stPixel.pvBase;
            size = pstBuf->stPixel.u32FilledLen;
            break;
        case VPP_IP_FRC_BUF_ME_YUV_OUT:
            virt = pstBuf->stPixel.pvBase;
            size = pstBuf->stPixel.u32FilledLen;
            break;
        case VPP_IP_FRC_BUF_ME_MBI_OUT:
            virt = (char *)pstBuf->stExtra.pvBase + 0x40;
            size = pstBuf->stExtra.u32FilledLen;
            if (size >= 0x40)
                size -= 0x40;
            break;
        default:
            LOGE("%s Error: Wrong eBuf=%d", __func__, eBuf);
            return(0);
    }

    if ((base!=NULL) && (virt!=NULL) && (size>0))
        memset((void*)virt, 0, size);

    LOGI("idx=%d, eBuf=%d, pstBuf=%p",idx, eBuf, pstBuf);
    LOG_EXIT_RET(0);
}
#endif

/*!
 * @brief       Process buffers
 *
 * @description It implements all the ME stages.
 *
 * @input       pstCb     ME context
 *
 * @return    VPP_OK is processing ok, VPP_ERR for error
 *
 */
static uint32_t u32VppIpFrcMe_ProcessBuffer(t_StVppIpFrcMeCb *pstCb)
{
    uint32_t u32BufSz, u32AvailSz, u32OutMin;
    struct vpp_mem_buffer *pstMemBuf;
    uint32_t ret = VPP_OK;
    int i,j;

    LOG_ENTER();
    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);

    switch (pstCb->u32MeStage)
    {
       case VPP_FRC_ME_STAGE_START:
              LOGI("%s VPP_FRC_ME_STAGE_START", __func__);
              //Force I field at the beginning
              struct v4l2_control control;
              control.id = V4L2_CID_MPEG_VIDC_VIDEO_REQUEST_IFRAME;
              control.value = 1;
              ret=IOCTL(pstCb->MeFd, VIDIOC_S_CTRL, &control);
              if (ret)
              {
                 LOGE("%s Error: V4L2_CID_MPEG_VIDC_VIDEO_REQUEST_IFRAME faild, ret=%d", __func__, ret);
              }
              pthread_mutex_lock(&pstCb->mutex);
              memset((void*)pstCb->pstBufIn, 0, sizeof(pstCb->pstBufIn));
              memset((void*)pstCb->pstBufOut, 0, sizeof(pstCb->pstBufOut));
              pstCb->u32MeStatus = VPP_FRC_ME_RUNNING;

              //Get input buffer
              for (i = 0; i < FRC_ME_START_IN_BUFS; i++)
              {
                  u32VppIpFrcMe_PortBufGet(&pstCb->stInput, &pstCb->pstBufIn[i]);
                  pstCb->pstBufIn[i]->u32Idx=i;
                  pstCb->InputBufsMEGot++;
              }

              //Get output buffer
              for (i = 0; i < pstCb->OutputBufsInternalPool; i++)
              {
                  u32VppIpFrcMe_PortBufGet(&pstCb->stOutput, &pstCb->pstBufOut[i]);
                  pstCb->pstBufOut[i]->u32Idx=i;
                  pstCb->OutputBufsMEGot++;
              }

              pthread_mutex_unlock(&pstCb->mutex);
              LOGIS("OutputBufsInternalPool=%d",pstCb->OutputBufsInternalPool);

              // Normal processing
              FRC_ME_STATS(pstCb, InProcCnt);
              FRC_ME_STATS(pstCb, OutProcCnt);

              //Stream Output
              if (u32VppIpFrcMe_MeStreamOn(pstCb, VPP_PORT_OUTPUT) != VPP_OK)
              {
                  LOGE("Error: u32VppIpFrcMe_MeStreamOn(Output) failed!");
                  //Release buffers
                  for (j = 0; j < FRC_ME_START_IN_BUFS; j++)
                  {
                      LOGI("%s : u32VppIp_CbBufDone()\n", __func__);
                      VPP_FLAG_SET(pstCb->pstBufIn[j]->u32InternalFlags, VPP_BUF_FLAG_FLUSHED);
                      pstCb->pstBufIn[j]->stPixel.u32FilledLen = 0;
                      vVppIpCbLog(&pstCb->stBase.stCb, pstCb->pstBufIn[j], eVppLogId_IpBufDone);
                      u32VppIp_CbBufDone(&pstCb->stBase.stCb,VPP_PORT_INPUT,pstCb->pstBufIn[j]);
                      pthread_mutex_lock(&pstCb->mutex);
                      pstCb->InputBufsMEDone++;
                      pstCb->InputBufsNeed++;
                      pstCb->InputBufsInternalNeed++;
                      pstCb->pstBufIn[j]=NULL;
                      pthread_mutex_unlock(&pstCb->mutex);
                  }
                  for (j = 0; j<pstCb->OutputBufsInternalPool; j++)
                  {
                      LOGI("%s : u32VppIp_CbBufDone()\n", __func__);
                      VPP_FLAG_SET(pstCb->pstBufOut[j]->u32InternalFlags, VPP_BUF_FLAG_FLUSHED);
                      pstCb->pstBufOut[j]->stPixel.u32FilledLen = 0;
                      vVppIpCbLog(&pstCb->stBase.stCb, pstCb->pstBufOut[j], eVppLogId_IpBufDone);
                      u32VppIp_CbBufDone(&pstCb->stBase.stCb,VPP_PORT_OUTPUT,pstCb->pstBufOut[j]);
                      pthread_mutex_lock(&pstCb->mutex);
                      pstCb->OutputBufsMEDone++;
                      pstCb->OutputBufsNeed++;
                      pstCb->OutputBufsInternalNeed++;
                      pstCb->pstBufOut[j] = NULL;
                      pthread_mutex_unlock(&pstCb->mutex);
                  }
                  ret = VPP_ERR;
                  goto adr_exit;
              }

              //Prepare Output Buffer
              for (i = 0; i < pstCb->OutputBufsInternalPool; i++)
              {
                if(u32VppIpFrcMe_MePrepareBuffer(pstCb,pstCb->pstBufOut[i],VPP_PORT_OUTPUT)!=VPP_OK)
                  {
                      LOGE("Error: u32VppIpFrcMe_MePrepareBuffer(Output %d) failed!", i);
                      //Release buffers
                      for (j = 0; j < FRC_ME_START_IN_BUFS; j++)
                      {
                          LOGI("%s : u32VppIp_CbBufDone()\n", __func__);
                          VPP_FLAG_SET(pstCb->pstBufIn[j]->u32InternalFlags, VPP_BUF_FLAG_FLUSHED);
                          pstCb->pstBufIn[j]->stPixel.u32FilledLen = 0;
                          vVppIpCbLog(&pstCb->stBase.stCb, pstCb->pstBufIn[j], eVppLogId_IpBufDone);
                          u32VppIp_CbBufDone(&pstCb->stBase.stCb,VPP_PORT_INPUT,pstCb->pstBufIn[j]);
                          pthread_mutex_lock(&pstCb->mutex);
                          pstCb->InputBufsMEDone++;
                          pstCb->InputBufsNeed++;
                          pstCb->InputBufsInternalNeed++;
                          pstCb->pstBufIn[j] = NULL;
                          pthread_mutex_unlock(&pstCb->mutex);
                      }
                      for (j = 0; j < pstCb->OutputBufsInternalPool; j++)
                      {
                          LOGI("%s : u32VppIp_CbBufDone()\n", __func__);
                          VPP_FLAG_SET(pstCb->pstBufOut[j]->u32InternalFlags, VPP_BUF_FLAG_FLUSHED);
                          pstCb->pstBufOut[j]->stPixel.u32FilledLen = 0;
                        vVppIpCbLog(&pstCb->stBase.stCb, pstCb->pstBufOut[j], eVppLogId_IpBufDone);
                        u32VppIp_CbBufDone(&pstCb->stBase.stCb,VPP_PORT_OUTPUT,pstCb->pstBufOut[j]);
                          pthread_mutex_lock(&pstCb->mutex);
                          pstCb->OutputBufsMEDone++;
                          pstCb->OutputBufsNeed++;
                          pstCb->OutputBufsInternalNeed++;
                          pstCb->pstBufOut[j] = NULL;
                          pthread_mutex_unlock(&pstCb->mutex);
                      }
                      ret = VPP_ERR;
                      goto adr_exit;
                  }
              }

              //Queue Output Buffer
              for (i = 0; i < pstCb->OutputBufsInternalPool; i++)
              {
                  if(u32VppIpFrcMe_MeQueueBuffer(pstCb,pstCb->pstBufOut[i],VPP_PORT_OUTPUT)!=VPP_OK)
                  {
                      LOGE("Error: u32VppIpFrcMe_MeQueueBuffer(output %d) failed!", i);
                      //Release buffers
                      for (j = 0; j < FRC_ME_START_IN_BUFS; j++)
                      {
                          LOGI("%s : u32VppIp_CbBufDone()\n", __func__);
                          VPP_FLAG_SET(pstCb->pstBufIn[j]->u32InternalFlags, VPP_BUF_FLAG_FLUSHED);
                          pstCb->pstBufIn[j]->stPixel.u32FilledLen = 0;
                          vVppIpCbLog(&pstCb->stBase.stCb, pstCb->pstBufIn[j], eVppLogId_IpBufDone);
                          u32VppIp_CbBufDone(&pstCb->stBase.stCb,VPP_PORT_INPUT,pstCb->pstBufIn[j]);
                          pthread_mutex_lock(&pstCb->mutex);
                          pstCb->InputBufsMEDone++;
                          pstCb->InputBufsNeed++;
                          pstCb->InputBufsInternalNeed++;
                          pstCb->pstBufIn[j] = NULL;
                          pthread_mutex_unlock(&pstCb->mutex);
                      }
                      for (j = i; j < pstCb->OutputBufsInternalPool; j++)
                      {
                          LOGI("%s : u32VppIp_CbBufDone()\n", __func__);
                          VPP_FLAG_SET(pstCb->pstBufOut[j]->u32InternalFlags, VPP_BUF_FLAG_FLUSHED);
                          pstCb->pstBufOut[j]->stPixel.u32FilledLen = 0;
                        vVppIpCbLog(&pstCb->stBase.stCb, pstCb->pstBufOut[j], eVppLogId_IpBufDone);
                        u32VppIp_CbBufDone(&pstCb->stBase.stCb,VPP_PORT_OUTPUT,pstCb->pstBufOut[j]);
                          pthread_mutex_lock(&pstCb->mutex);
                          pstCb->OutputBufsMEDone++;
                          pstCb->OutputBufsNeed++;
                          pstCb->OutputBufsInternalNeed++;
                          pstCb->pstBufOut[j] = NULL;
                          pthread_mutex_unlock(&pstCb->mutex);
                      }
                      ret = VPP_ERR;
                      goto adr_exit;
                  }
                  pthread_mutex_lock(&pstCb->mutex);
                  pstCb->OutputBufsMEQueued++;
                  pstCb->OutputBufsMEInside++;
                  pstCb->OutputBufsInternalNeed--;
                  pthread_mutex_unlock(&pstCb->mutex);
              }

              //Queue Input Buffer
              for (i = 0; i < FRC_ME_START_IN_BUFS; i++)
              {
                  if(u32VppIpFrcMe_MeQueueBuffer(pstCb,pstCb->pstBufIn[i],VPP_PORT_INPUT)!=VPP_OK)
                  {
                      LOGE("Error: u32VppIpFrcMe_MeQueueBuffer(Input %d) failed!", i);
                      //Release buffers
                      for (j = i; j < FRC_ME_START_IN_BUFS; j++)
                      {
                          LOGI("%s : u32VppIp_CbBufDone()\n", __func__);
                          VPP_FLAG_SET(pstCb->pstBufIn[j]->u32InternalFlags, VPP_BUF_FLAG_FLUSHED);
                          pstCb->pstBufIn[j]->stPixel.u32FilledLen = 0;
                          vVppIpCbLog(&pstCb->stBase.stCb, pstCb->pstBufIn[j], eVppLogId_IpBufDone);
                          u32VppIp_CbBufDone(&pstCb->stBase.stCb,VPP_PORT_INPUT,pstCb->pstBufIn[j]);
                          pthread_mutex_lock(&pstCb->mutex);
                          pstCb->InputBufsMEDone++;
                          pstCb->InputBufsNeed++;
                          pstCb->InputBufsInternalNeed++;
                          pstCb->pstBufIn[j] = NULL;
                          pthread_mutex_unlock(&pstCb->mutex);
                      }
                      ret = VPP_ERR;
                      goto adr_exit;
                  }
                  pthread_mutex_lock(&pstCb->mutex);
                  pstCb->InputBufsMEQueued++;
                  pstCb->InputBufsMEInside++;
                  pstCb->InputBufsInternalNeed--;
                  pthread_mutex_unlock(&pstCb->mutex);
              }

              //Stream Input
              if (u32VppIpFrcMe_MeStreamOn(pstCb, VPP_PORT_INPUT) != VPP_OK)
              {
                  LOGE("Error: u32VppIpFrcMe_MeStreamOn(Input) failed!");
                  ret = VPP_ERR;
                  goto adr_exit;
              }
              pstCb->u32MeStage = VPP_FRC_ME_STAGE_FEED;
          break;

       case VPP_FRC_ME_STAGE_FEED:
              LOGI("%s VPP_FRC_ME_STAGE_FEED", __func__);
              t_StVppBuf *pstInBuf_tmp = NULL;
              t_StVppBuf *pstOutBuf_tmp = NULL;
              uint32_t u32InputFreeIndex = 0;
              uint32_t u32OutputFreeIndex = 0;

              pthread_mutex_lock(&pstCb->mutex);
              pstCb->u32MeStatus = VPP_FRC_ME_RUNNING;
              //Get output buffer
              if ((pstCb->u32MeProcOutQCnt)&&(pstCb->OutputBufsInternalNeed > 0))
              {
                  u32OutputFreeIndex = u32VppIpFrcMe_MeGetFreeIndex(pstCb, VPP_PORT_OUTPUT);
                  if (u32OutputFreeIndex == FRC_ME_MAX_INDEX)
                  {
                      pthread_mutex_unlock(&pstCb->mutex);
                      LOGE("Error: No free Outnput Index was found!\n");
                      u32OutputFreeIndex=0;
                  }
                  else
                  {
                      u32VppIpFrcMe_PortBufGet(&pstCb->stOutput, &pstOutBuf_tmp);
                      pstCb->pstBufOut[u32OutputFreeIndex] = pstOutBuf_tmp;
                      pstCb->pstBufOut[u32OutputFreeIndex]->u32Idx = u32OutputFreeIndex;
                      pstCb->OutputBufsMEGot++;
                      pthread_mutex_unlock(&pstCb->mutex);

                      //Queue output buffer
                      if (u32VppIpFrcMe_MeQueueBuffer(pstCb, pstCb->pstBufOut[u32OutputFreeIndex],
                                                      VPP_PORT_OUTPUT) != VPP_OK)
                      {
                          LOGE("Error: u32VppIpFrcMe_MeQueueBuffer(output index=%d) failed!",
                                u32OutputFreeIndex);
                          //Release buffers
                          LOGI("%s : u32VppIp_CbBufDone()\n", __func__);
                          VPP_FLAG_SET(pstCb->pstBufOut[u32OutputFreeIndex]->u32InternalFlags,
                                                        VPP_BUF_FLAG_FLUSHED);
                          pstCb->pstBufOut[u32OutputFreeIndex]->stPixel.u32FilledLen = 0;
                          vVppIpCbLog(&pstCb->stBase.stCb, pstCb->pstBufOut[u32OutputFreeIndex],
                                      eVppLogId_IpBufDone);
                          u32VppIp_CbBufDone(&pstCb->stBase.stCb,VPP_PORT_OUTPUT,
                                              pstCb->pstBufOut[u32OutputFreeIndex]);
                          pthread_mutex_lock(&pstCb->mutex);
                          pstCb->OutputBufsMEDone++;
                          pstCb->OutputBufsNeed++;
                          pstCb->OutputBufsInternalNeed++;
                          pstCb->pstBufOut[u32OutputFreeIndex] = NULL;
                          pthread_mutex_unlock(&pstCb->mutex);
                          ret = VPP_ERR;
                          goto adr_exit;
                      }
                      pthread_mutex_lock(&pstCb->mutex);
                      pstCb->OutputBufsMEQueued++;
                      pstCb->OutputBufsMEInside++;
                      pstCb->OutputBufsInternalNeed--;
                      pthread_mutex_unlock(&pstCb->mutex);
                  }
              }
              else
                  pthread_mutex_unlock(&pstCb->mutex);

              pthread_mutex_lock(&pstCb->mutex);
              //Get input buffer
              if ((pstCb->u32MeProcInQCnt) && (pstCb->InputBufsInternalNeed > 0) &&
                  (!(pstCb->InputGotEosCnt)))
              {
                  u32InputFreeIndex = u32VppIpFrcMe_MeGetFreeIndex(pstCb, VPP_PORT_INPUT);
                  if (u32InputFreeIndex == FRC_ME_MAX_INDEX)
                  {
                      pthread_mutex_unlock(&pstCb->mutex);
                      LOGE("Error: No free Input Index was found!\n");
                      u32InputFreeIndex=0;
                  }
                  else
                  {
                      u32VppIpFrcMe_PortBufGet(&pstCb->stInput, &pstInBuf_tmp);
                      pstCb->pstBufIn[u32InputFreeIndex] = pstInBuf_tmp;
                      pstCb->pstBufIn[u32InputFreeIndex]->u32Idx = u32InputFreeIndex;
                      pstCb->InputBufsMEGot++;
                      pthread_mutex_unlock(&pstCb->mutex);

                      //Queue input buffer
                      if (u32VppIpFrcMe_MeQueueBuffer(pstCb, pstCb->pstBufIn[u32InputFreeIndex],
                                                      VPP_PORT_INPUT) != VPP_OK)
                      {
                          LOGE("Error: u32VppIpFrcMe_MeQueueBuffer(Input index=%d) failed!",
                                u32InputFreeIndex);
                          //Release buffers
                          LOGI("%s : u32VppIp_CbBufDone()\n", __func__);
                          VPP_FLAG_SET(pstCb->pstBufIn[u32InputFreeIndex]->u32InternalFlags,
                                       VPP_BUF_FLAG_FLUSHED);
                          pstCb->pstBufIn[u32InputFreeIndex]->stPixel.u32FilledLen = 0;
                          vVppIpCbLog(&pstCb->stBase.stCb, pstCb->pstBufIn[u32InputFreeIndex],
                                      eVppLogId_IpBufDone);
                          u32VppIp_CbBufDone(&pstCb->stBase.stCb,VPP_PORT_INPUT,
                                              pstCb->pstBufIn[u32InputFreeIndex]);
                          pthread_mutex_lock(&pstCb->mutex);
                          pstCb->InputBufsMEDone++;
                          pstCb->InputBufsNeed++;
                          pstCb->InputBufsInternalNeed++;
                          pstCb->pstBufIn[u32InputFreeIndex] = NULL;
                          pthread_mutex_unlock(&pstCb->mutex);
                          ret = VPP_ERR;
                          goto adr_exit;
                      }
                      pthread_mutex_lock(&pstCb->mutex);
                      pstCb->InputBufsMEQueued++;
                      pstCb->InputBufsMEInside++;
                      pstCb->InputBufsInternalNeed--;
                      pthread_mutex_unlock(&pstCb->mutex);
                  }
              }
              else
                  pthread_mutex_unlock(&pstCb->mutex);

              // Normal processing
              FRC_ME_STATS(pstCb, InProcCnt);
              FRC_ME_STATS(pstCb, OutProcCnt);

           break;

       case VPP_FRC_ME_STAGE_FLUSH:
              LOGI("%s VPP_FRC_ME_STAGE_FLUSH", __func__);
           break;

       case VPP_FRC_ME_STAGE_BYPASS:
              LOGI("%s VPP_FRC_ME_STAGE_BYPASS", __func__);
              pthread_mutex_lock(&pstCb->mutex);

              if ((pstCb->u32MeProcInQCnt) && ((pstCb->u32MeStatus == VPP_FRC_ME_RUNNING_FLUSHED) ||
                   (pstCb->u32MeStatus == VPP_FRC_ME_RUNNING_DRAINED) ||
                   (pstCb->u32MeStatus == VPP_FRC_ME_OPENED)))
              {
                  //ME doesn't need Drain, return BYPASS input buffer
                  t_StVppBuf *pstInBuf_tmp = NULL;
                  ret = u32VppIpFrcMe_PortBufGet(&pstCb->stInput, &pstInBuf_tmp);
                  if ((ret != VPP_OK) || (pstInBuf_tmp == NULL))
                  {
                      LOGE("Error: u32VppIpFrcMe_PortBufGet failed! pstPort %p, pstBuf %p",
                           &pstCb->stInput, pstInBuf_tmp);
                      pthread_mutex_unlock(&pstCb->mutex);
                      goto adr_exit;
                  }
                  pstCb->InputBufsMEGot++;
                  pstCb->InputBufsMEDone++;
                  if (VPP_FLAG_IS_SET(pstInBuf_tmp->u32InternalFlags, VPP_BUF_FLAG_ME_INTERN_BYPASS))
                  {
                      VPP_FLAG_CLR(pstInBuf_tmp->u32InternalFlags, VPP_BUF_FLAG_ME_INTERN_BYPASS);
                      LOGI("Removed Internal bypass flag from input buffer");
                  }
                  pthread_mutex_unlock(&pstCb->mutex);
                  u32VppIp_CbBufDone(&pstCb->stBase.stCb, VPP_PORT_INPUT, pstInBuf_tmp);
                  FRC_ME_STATS(pstCb, InProcCnt);
              }
              else if (!(pstCb->u32InternalFlags & IP_DRAIN_PENDING))
              {
                  //A new input buffer came which requires ME Drain
                  pstCb->u32InternalFlags |= IP_DRAIN_PENDING;
                  pstCb->u32MeStatus = VPP_FRC_ME_RUNNING;
                  pstCb->stBufEos.pBuf->timestamp = pstCb->MeLastTimestamp+(FRC_ME_DEFAULT_DELAY/2);
                  //This condition cannot fail because we check in u32ProcBufReqMet()
                  if ((pstCb->u32MeProcInQCnt) && (pstCb->InputBufsInternalNeed > 0))
                  {
                      u32InputFreeIndex = u32VppIpFrcMe_MeGetFreeIndex(pstCb, VPP_PORT_INPUT);
                      if (u32InputFreeIndex == FRC_ME_MAX_INDEX)
                      {
                          pthread_mutex_unlock(&pstCb->mutex);
                          LOGE("Error: No free Input Index was found!\n");
                          u32InputFreeIndex = 0;
                      }
                      else
                      {
                          pstCb->pstBufIn[u32InputFreeIndex] = &pstCb->stBufEos;
                          pstCb->pstBufIn[u32InputFreeIndex]->u32Idx = u32InputFreeIndex;
                          pstCb->InputBufsMEGot++;
                          pthread_mutex_unlock(&pstCb->mutex);

                          //Queue input buffer
                          if (u32VppIpFrcMe_MeQueueBuffer(pstCb, pstCb->pstBufIn[u32InputFreeIndex],
                              VPP_PORT_INPUT) != VPP_OK)
                          {
                              LOGE("Error: u32VppIpFrcMe_MeQueueBuffer(Input index=%d) failed!",
                                    u32InputFreeIndex);
                              pthread_mutex_lock(&pstCb->mutex);
                              pstCb->InputBufsMEDone++;
                              pstCb->InputBufsInternalNeed++;
                              pstCb->pstBufIn[u32InputFreeIndex] = NULL;
                              pthread_mutex_unlock(&pstCb->mutex);
                              ret = VPP_ERR;
                              goto adr_exit;
                          }
                          pthread_mutex_lock(&pstCb->mutex);
                          pstCb->InputBufsMEQueued++;
                          pstCb->InputBufsMEInside++;
                          pstCb->InputBufsInternalNeed--;
                          pthread_mutex_unlock(&pstCb->mutex);
                      }
                  }
                  else
                  {
                      LOGE("%s Error: It is something wrong if we hit this place!",__func__);
                      pthread_mutex_unlock(&pstCb->mutex);
                  }
              }
              else if ((pstCb->u32MeProcOutQCnt) &&
                       (!(VPP_FLAG_IS_SET(pstCb->u32InternalFlags, IP_FLUSH_PENDING))) &&
                       (pstCb->u32InternalFlags & IP_DRAIN_PENDING))
              {
                  //Feed outputs in ME to do Drain
                  t_StVppBuf *pstOutBuf_tmp = NULL;
                  uint32_t u32OutputFreeIndex = 0;

                  pstCb->u32MeStatus = VPP_FRC_ME_RUNNING;
                  //Get output buffer
                  if ((pstCb->u32MeProcOutQCnt) && (pstCb->OutputBufsInternalNeed > 0))
                  {
                      u32OutputFreeIndex = u32VppIpFrcMe_MeGetFreeIndex(pstCb, VPP_PORT_OUTPUT);
                      if (u32OutputFreeIndex == FRC_ME_MAX_INDEX)
                      {
                          pthread_mutex_unlock(&pstCb->mutex);
                          LOGE("Error: No free Outnput Index was found!\n");
                          u32OutputFreeIndex=0;
                      }
                      else
                      {
                          u32VppIpFrcMe_PortBufGet(&pstCb->stOutput, &pstOutBuf_tmp);
                          pstCb->pstBufOut[u32OutputFreeIndex] = pstOutBuf_tmp;
                          pstCb->pstBufOut[u32OutputFreeIndex]->u32Idx = u32OutputFreeIndex;
                          pstCb->OutputBufsMEGot++;
                          pthread_mutex_unlock(&pstCb->mutex);

                          //Queue output buffer
                          if(u32VppIpFrcMe_MeQueueBuffer(pstCb,pstCb->pstBufOut[u32OutputFreeIndex],
                             VPP_PORT_OUTPUT) != VPP_OK)
                          {
                              LOGE("Error: u32VppIpFrcMe_MeQueueBuffer(output index=%d) failed!",
                                    u32OutputFreeIndex);
                              //Release buffers
                              LOGI("%s : u32VppIp_CbBufDone()\n", __func__);
                              VPP_FLAG_SET(pstCb->pstBufOut[u32OutputFreeIndex]->u32InternalFlags,
                                                            VPP_BUF_FLAG_FLUSHED);
                              pstCb->pstBufOut[u32OutputFreeIndex]->stPixel.u32FilledLen = 0;
                              vVppIpCbLog(&pstCb->stBase.stCb, pstCb->pstBufOut[u32OutputFreeIndex],
                                          eVppLogId_IpBufDone);
                              u32VppIp_CbBufDone(&pstCb->stBase.stCb,VPP_PORT_OUTPUT,
                                                  pstCb->pstBufOut[u32OutputFreeIndex]);
                              pthread_mutex_lock(&pstCb->mutex);
                              pstCb->OutputBufsMEDone++;
                              pstCb->OutputBufsNeed++;
                              pstCb->OutputBufsInternalNeed++;
                              pstCb->pstBufOut[u32OutputFreeIndex] = NULL;
                              pthread_mutex_unlock(&pstCb->mutex);
                              ret = VPP_ERR;
                              goto adr_exit;
                          }
                          pthread_mutex_lock(&pstCb->mutex);
                          pstCb->OutputBufsMEQueued++;
                          pstCb->OutputBufsMEInside++;
                          pstCb->OutputBufsInternalNeed--;
                          pthread_mutex_unlock(&pstCb->mutex);
                      }
                  }
                  else
                      pthread_mutex_unlock(&pstCb->mutex);
              }
              else
                  pthread_mutex_unlock(&pstCb->mutex);

            break;
       default:
              LOGE("Error ME in invalid stage!\n");
    }
adr_exit:
    LOG_EXITI_RET(ret);
}

/*!
 * @brief       Start Pending FlushDrain process
 *
 * @description It flushes ME internally if the EOS buffer was completly processed.
 *                     The caller must hold the mutex.
 * @input       pstCb     ME context
 *
 * @return
 *
 */
static void vVppIpFrcMe_HandleStartPendingFlushDrain(t_StVppIpFrcMeCb *pstCb)
{
    t_StVppEvt stEvt;
    uint32_t ret = VPP_OK;

    LOGI("%s", __func__);
    VPP_RET_VOID_IF_NULL(pstCb);

    if ((!(pstCb->u32MeFlags & FRC_ME_FLUSH_DRAIN_STARTED))&&
         (((pstCb->u32InternalFlags & IP_DRAIN_PENDING) &&
          (pstCb->u32MeFlags & FRC_ME_RELEASED_INPUT_EOS_DRAIN_DETECTED) &&
          (pstCb->u32MeFlags & FRC_ME_RELEASED_OUTPUT_EOS_DRAIN_DETECTED)) ||
         ((pstCb->u32InternalFlags & IP_EOS_PENDING) &&
           (!(pstCb->u32InternalFlags & IP_FLUSH_PENDING)) &&
          (pstCb->u32MeFlags & FRC_ME_RELEASED_INPUT_EOS_DETECTED) &&
          (pstCb->u32MeFlags & FRC_ME_RELEASED_OUTPUT_EOS_DETECTED))))
    {
        //Flush ME
        if (pstCb->u32MeStage != VPP_FRC_ME_STAGE_BYPASS)
            pstCb->u32MeStage = VPP_FRC_ME_STAGE_FLUSH;

        pstCb->u32InternalFlags |= IP_FLUSH_PENDING;
        pstCb->u32MeFlags |= FRC_ME_FLUSH_DRAIN_STARTED;

        struct v4l2_encoder_cmd enc;

        enc.cmd = V4L2_ENC_QCOM_CMD_FLUSH;
        //Venus Encoder requires FLUSH on both ports
        enc.flags = V4L2_QCOM_CMD_FLUSH_OUTPUT | V4L2_QCOM_CMD_FLUSH_CAPTURE;

        ret = IOCTL(pstCb->MeFd, VIDIOC_ENCODER_CMD, &enc);
        if (ret)
        {
            LOGE("Error: Flush ME Failed, ret=%d", ret);
        }

        LOGI("%s ME Internal Flush Command\n", __func__);
    }
}

/*!
 * @brief       Stop Pending FlushDrain process
 *
 * @description Clear flags, STREAM OFF ports, sensd VPP_EVT_DRAIN_DONE event. Set START stage.
 *
 * @input       pstCb     ME context
 *
 * @return
 *
 */
static void vVppIpFrcMe_HandleEndPendingFlushDrain(t_StVppIpFrcMeCb *pstCb)
{
    t_StVppEvt stEvt;
    uint32_t ret=VPP_OK;

    LOGI("%s Started\n", __func__);
    VPP_RET_VOID_IF_NULL(pstCb);

    pthread_mutex_lock(&pstCb->mutex);
    if ((pstCb->u32InternalFlags & IP_FLUSH_PENDING) &&
        (pstCb->u32MeFlags & FRC_ME_FLUSH_DRAIN_STARTED))
    {
        //Clear flags
        int drain_pending = ((pstCb->u32InternalFlags & IP_DRAIN_PENDING) &&
                             (!(pstCb->u32InternalFlags & IP_EOS_PENDING)));
        pstCb->u32InternalFlags &= ~IP_EOS_PENDING;
        pstCb->u32InternalFlags &= ~IP_FLUSH_PENDING;
        pstCb->u32MeFlags &= ~FRC_ME_RELEASED_INPUT_EOS_DETECTED;
        pstCb->u32MeFlags &= ~FRC_ME_RELEASED_OUTPUT_EOS_DETECTED;
        pstCb->u32MeFlags &= ~FRC_ME_FLUSH_DRAIN_STARTED;
        pstCb->u32MeStatus = VPP_FRC_ME_RUNNING_FLUSHED;

        if (!(pstCb->u32InternalFlags & IP_DRAIN_PENDING))
        {
            if (pstCb->InputEosCnt > 0)
                pstCb->InputEosCnt--;
        }

        if (pstCb->InputGotEosCnt > 0)
            pstCb->InputGotEosCnt--;

        if (drain_pending)
        {
            pstCb->u32InternalFlags &= ~IP_DRAIN_PENDING;
            pstCb->u32MeFlags &= ~FRC_ME_RELEASED_INPUT_EOS_DRAIN_DETECTED;
            pstCb->u32MeFlags &= ~FRC_ME_RELEASED_OUTPUT_EOS_DRAIN_DETECTED;
            pstCb->u32MeStatus = VPP_FRC_ME_RUNNING_DRAINED;
        }

        //Send DRAIN DONE event
        if (pstCb->u32MeStage != VPP_FRC_ME_STAGE_BYPASS)
        {
            pstCb->u32MeStage = VPP_FRC_ME_STAGE_START;
            if (drain_pending)
            {
                stEvt.eType = VPP_EVT_DRAIN_DONE;
                pthread_mutex_unlock(&pstCb->mutex);
                u32VppIpCbEvent(&pstCb->stBase.stCb, stEvt);
                LOGI("%s ME Drain Done.\n", __func__);
                pthread_mutex_lock(&pstCb->mutex);
            }
        }
    }

    u32VppIpFrcMe_TimestampListFlush(&(pstCb->InputTimestampList));
    //This is to force resume processing buffers already in queues
    pstCb->u32MeEvent = V4L2_EVENT_MSM_VIDC_FLUSH_DONE;
    pthread_cond_signal(&pstCb->cond);
    pthread_mutex_unlock(&pstCb->mutex);
}

/*!
 * @brief       ME worker thread
 *
 * @description Main loop which process commands and buffers.
 *                     It sleeps if no buffers or commands to be processed.
 *
 * @input       pv     ME context
 *
 * @return
 *
 */
static void *vpVppIpFrcMe_Worker(void *pv)
{
    LOGD("%s started", __func__);

    t_StVppIpFrcMeCb *pstCb = (t_StVppIpFrcMeCb *)pv;

    // Signal back to main thread that we've launched and are ready to go
    vVppIpFrcMe_SignalWorkerStart(pstCb);

    if (pstCb->u32WorkerErr != VPP_OK)
        goto FRC_ME_WORKER_EXIT;

    while (1)
    {
        LOGI("ME Worker Blocking");
        pthread_mutex_lock(&pstCb->mutex);
        while (u32WorkerThreadShouldSleep(pstCb))
        {
            VPP_IP_PROF_START(&pstCb->stBase, FRC_ME_STAT_WORKER_SLEEP);
            pthread_cond_wait(&pstCb->cond, &pstCb->mutex);
            VPP_IP_PROF_STOP(&pstCb->stBase, FRC_ME_STAT_WORKER_SLEEP);
        }

        VPP_IP_PROF_START(&pstCb->stBase, FRC_ME_STAT_WORKER);
        uint32_t u32Ret;
        t_StVppIpCmd stCmd;
        if ((!(pstCb->u32InternalFlags & IP_FLUSH_PENDING))&&
            (!(pstCb->u32InternalFlags & IP_DRAIN_PENDING))&&
            (!(pstCb->u32InternalFlags & IP_EOS_PENDING)) && (pstCb->InputEosCnt <= 0))
        {
            //Get command from queue
            u32Ret = u32VppIpFrcMe_CmdGet(pstCb, &stCmd);
            if (u32Ret == VPP_OK)
            {
                // Process the command
                LOG_CMD("ProcessCmd", stCmd.eCmd);

                if (stCmd.eCmd == VPP_IP_CMD_THREAD_EXIT)
                {
                    u32VppIpFrcMe_ProcCmdExit(pstCb);
                    VPP_IP_PROF_STOP(&pstCb->stBase, FRC_ME_STAT_WORKER);
                    pthread_mutex_unlock(&pstCb->mutex);
                    break;
                }

                else if (stCmd.eCmd == VPP_IP_CMD_FLUSH)
                    u32VppIpFrcMe_ProcCmdFlush(pstCb, &stCmd);

                else if (stCmd.eCmd == VPP_IP_CMD_DRAIN)
                    u32VppIpFrcMe_ProcCmdDrain(pstCb);

                else
                    LOGE("Error: Unknown command in queue");

                VPP_IP_PROF_STOP(&pstCb->stBase, FRC_ME_STAT_WORKER);
                pthread_mutex_unlock(&pstCb->mutex);
                continue;
            }
        }
        pthread_mutex_unlock(&pstCb->mutex);

        //Procerss buffers
        if (pstCb->eState == VPP_IP_STATE_ACTIVE)
        {
            LOGI("%s Process buffer\n", __func__);
            u32VppIpFrcMe_ProcessBuffer(pstCb);

            VPP_IP_PROF_STOP(&pstCb->stBase, FRC_ME_STAT_WORKER);
            continue;
        }

        LOGE("%s Error: woke up, but did no work", __func__);
        VPP_IP_PROF_STOP(&pstCb->stBase, FRC_ME_STAT_WORKER);
    }

FRC_ME_WORKER_EXIT:
    LOGD("%s exited", __func__);

    return NULL;
}

/*!
 * @brief       ME Receiver thread
 *
 * @description Receiver thread in which ME is polled for buffers done and events.
 *                     It sleeps if no driver buffers or events available.
 *
 * @input       pv     ME context
 *
 * @return
 *
 */
static void *vpVppIpFrcMe_MeReceiver(void *pv)
{
    LOGD("%s started", __func__);

    t_StVppIpFrcMeCb *pstCb = (t_StVppIpFrcMeCb *)pv;

    // Signal back to main thread that we've launched and are ready to go
    vVppIpFrcMe_MeSignalReceiverStart(pstCb);
    pstCb->u32MeWorkerErr = VPP_OK;

    struct v4l2_plane plane[VIDEO_MAX_PLANES];
    struct pollfd pfdesc[2];
    struct v4l2_buffer v4l2_buf;
    struct v4l2_event event;
    int ret=0;
    int rc=0;
    pfdesc[0].events = POLLIN | POLLRDNORM | POLLOUT | POLLWRNORM | POLLRDBAND | POLLPRI;
    pfdesc[1].events = POLLIN | POLLERR;
    pfdesc[0].fd = pstCb->MeFd;
    pfdesc[1].fd = pstCb->MePollNotification;
    int mbi_size=0;

    while (1)
    {
        //Receiver exit command
        if (pstCb->u32MeCmd == VPP_IP_CMD_THREAD_EXIT)
        {
            pstCb->u32MeCmd = 0;
            break;
        }

        {
            VPP_IP_PROF_START(&pstCb->stBase, FRC_ME_STAT_REC_WORKER_SLEEP);
            rc = poll(pfdesc, 2, FRC_ME_POLL_TIMEOUT);
            VPP_IP_PROF_STOP(&pstCb->stBase, FRC_ME_STAT_REC_WORKER_SLEEP);

            VPP_IP_PROF_START(&pstCb->stBase, FRC_ME_STAT_REC_WORKER);
            if (!rc)
            {
                //timeout
                pthread_mutex_lock(&pstCb->mutex);
                uint32_t u32InQSz = u32VppBufPool_Cnt(&pstCb->stInput.stQ);
                uint32_t u32OutQSz = u32VppBufPool_Cnt(&pstCb->stOutput.stQ);
                pthread_mutex_unlock(&pstCb->mutex);
                LOGE("%s Error: ME polling timedout !" ,__func__);
                u32VppIpFrcMe_LogStatus(pstCb, u32InQSz, u32OutQSz);
                VPP_IP_PROF_STOP(&pstCb->stBase, FRC_ME_STAT_REC_WORKER);
                continue;
            }
            else if (rc < 0)
            {
                //error
                pthread_mutex_lock(&pstCb->mutex);
                uint32_t u32InQSz = u32VppBufPool_Cnt(&pstCb->stInput.stQ);
                uint32_t u32OutQSz = u32VppBufPool_Cnt(&pstCb->stOutput.stQ);
                pthread_mutex_unlock(&pstCb->mutex);
                LOGE("%s Error while ME polling: %d", __func__, rc);
                u32VppIpFrcMe_LogStatus(pstCb, u32InQSz, u32OutQSz);
                ret = VPP_ERR;
                VPP_IP_PROF_STOP(&pstCb->stBase, FRC_ME_STAT_REC_WORKER);
                break;
            }

            //Receiver exit event
            if ((pfdesc[1].revents & POLLIN) || (pfdesc[1].revents & POLLERR))
            {
                 LOGI("MeReceiver thread interrupted to be exited");
                 VPP_IP_PROF_STOP(&pstCb->stBase, FRC_ME_STAT_REC_WORKER);
                 break;
            }

            //Read Input buffers returned my the driver
            if ((pfdesc[0].revents & POLLOUT) || (pfdesc[0].revents & POLLWRNORM))
            {
                memset(&plane, 0, sizeof(plane));
                memset(&v4l2_buf, 0, sizeof(v4l2_buf));

                v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
                v4l2_buf.memory = V4L2_MEMORY_USERPTR;
                v4l2_buf.m.planes = plane;
                v4l2_buf.length = 1;

                while (!IOCTL(pfdesc[0].fd, VIDIOC_DQBUF, &v4l2_buf))
                {
                    LOGI("%s DeQueue Input buffer %d", __func__, v4l2_buf.index);
                    VPP_IP_PROF_STOP(&pstCb->stBase, FRC_ME_STAT_ENC_1F);
                    pthread_mutex_lock(&pstCb->mutex);

                    //Check index
                    if (pstCb->pstBufIn[v4l2_buf.index] == NULL)
                    {
                        LOGE("Error: Dequeued Input buffer index = %d belongs to an empty location "
                             "in list! (Critical Error)", v4l2_buf.index);
                        pthread_mutex_unlock(&pstCb->mutex);
                        continue;
                    }
                    else if (pstCb->pstBufIn[v4l2_buf.index]->u32Idx != v4l2_buf.index)
                    {
                        LOGE("Error: Dequeue Input buffer index = %d is different than the "
                             "index = %d of the buffer from the list! (Critical Error)",
                              v4l2_buf.index, pstCb->pstBufIn[v4l2_buf.index]->u32Idx);
                    }
                    u32VppIpFrcMe_LogBufferInfo(pstCb->pstBufIn[v4l2_buf.index]);

                    if (pstCb->pstBufIn[v4l2_buf.index]->u32InternalFlags &
                        VPP_BUF_FLAG_ME_INTERN_EOS)
                    {
                        pstCb->u32MeFlags |= FRC_ME_RELEASED_INPUT_EOS_DRAIN_DETECTED;
                        LOGI("Detected EOS Drain input buffer released!");
                        //This is internal ME buffer and it is not sent out.
                        if (!(pstCb->u32InternalFlags & IP_DRAIN_PENDING))
                            LOGE("Error: u32InternalFlags is not IP_DRAIN_PENDING, "
                                 "u32InternalFlags=0x%x",pstCb->u32InternalFlags);
                        pstCb->InputBufsMEDone++;
                        pstCb->InputBufsMEInside--;
                        pstCb->InputBufsNeed++;
                        pstCb->InputBufsInternalNeed++;
                        pstCb->pstBufIn[v4l2_buf.index]=NULL;
                        pthread_mutex_unlock(&pstCb->mutex);
                    }
                    else
                    {
                        if (pstCb->u32InternalFlags & IP_FLUSH_PENDING)
                            VPP_FLAG_SET(pstCb->pstBufIn[v4l2_buf.index]->u32InternalFlags,
                                         VPP_BUF_FLAG_FLUSHED);

                        if (pstCb->pstBufIn[v4l2_buf.index]->pBuf->flags & VPP_BUFFER_FLAG_EOS)
                        {
                            pstCb->u32InternalFlags |= IP_EOS_PENDING;
                            pstCb->u32MeFlags |= FRC_ME_RELEASED_INPUT_EOS_DETECTED;
                            LOGI("Detected EOS input buffer released!");
                        }
                        t_StVppBuf * p=pstCb->pstBufIn[v4l2_buf.index];
                        pstCb->InputBufsMEDone++;
                        pstCb->InputBufsMEInside--;
                        pstCb->InputBufsNeed++;
                        pstCb->InputBufsInternalNeed++;
                        pstCb->pstBufIn[v4l2_buf.index]=NULL;
                        pthread_mutex_unlock(&pstCb->mutex);

                        // Return the Input buffers
                        VPP_FRC_ME_CRC_BUF(pstCb, p, pstCb->InputBufsMEDone);
                        VPP_FRC_ME_DUMP_BUF(pstCb, p, VPP_IP_FRC_BUF_ME_YUV_IN,
                                            pstCb->InputBufsMEDone);
                        FRC_ME_STATS(pstCb, IBDCnt);
                        vVppIpCbLog(&pstCb->stBase.stCb, p, eVppLogId_IpBufDone);
                        u32VppIp_CbBufDone(&pstCb->stBase.stCb, VPP_PORT_INPUT,p);
                    }
                    pthread_cond_signal(&pstCb->cond);
                }
            }

            //Read Output buffers returned my the driver
            if ((pfdesc[0].revents & POLLIN) || (pfdesc[0].revents & POLLRDNORM))
            {
                memset(&plane, 0, sizeof(plane));
                memset(&v4l2_buf, 0, sizeof(v4l2_buf));

                v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
                v4l2_buf.memory = V4L2_MEMORY_USERPTR;
                v4l2_buf.m.planes = plane;
#if FRC_ME_MBI_TEST
                v4l2_buf.length = FRC_ME_NO_PLANES;
#else
                v4l2_buf.length = 1;
#endif

                while (!IOCTL(pfdesc[0].fd, VIDIOC_DQBUF, &v4l2_buf))
                {
                    LOGI("%s DeQueue Output buffer %d", __func__, v4l2_buf.index);
                    //Check index
                    if (pstCb->pstBufOut[v4l2_buf.index] == NULL)
                    {
                        LOGE("Error: Dequeued Output buffer index = %d belongs to an empty location"
                             "in list! (Critical Error)", v4l2_buf.index);
                        continue;
                    }
                    else if (pstCb->pstBufOut[v4l2_buf.index]->u32Idx != v4l2_buf.index)
                    {
                        LOGE("Error: Dequeue Output buffer index = %d is different than the "
                             "index = %d of the buffer from the list! (Critical Error)",
                              v4l2_buf.index, pstCb->pstBufOut[v4l2_buf.index]->u32Idx);
                    }

                    //Copy info from v4l2 buffer
                    pstCb->pstBufOut[v4l2_buf.index]->eBufPxType = eVppBufPxDataType_Compressed;
                    pstCb->pstBufOut[v4l2_buf.index]->stPixel.u32FilledLen =
                                                                    v4l2_buf.m.planes[0].bytesused;
                    pstCb->pstBufOut[v4l2_buf.index]->stPixel.u32Offset =
                                                                  v4l2_buf.m.planes[0].data_offset;
                    if (v4l2_buf.length > 1)
                    {
                        //MBI info
                        if ((v4l2_buf.m.planes[0].bytesused == 0)&&
                            (!(v4l2_buf.flags & V4L2_QCOM_BUF_FLAG_EOS)))
                        {
                            //Supplimentary working output buffers released after EOS output buffer.
                            pstCb->pstBufOut[v4l2_buf.index]->stExtra.u32FilledLen = 0;
                        }
                        else
                        {
                            //Output buffer which contain a valid MBI
                            struct msm_vidc_extradata_header *pv = NULL;
                            pstCb->pstBufOut[v4l2_buf.index]->stExtra.u32FilledLen =
                                                                     v4l2_buf.m.planes[1].bytesused;
                            pv=(struct msm_vidc_extradata_header*)vpVppBuf_FindExtradataHeader(
                                                                   pstCb->pstBufOut[v4l2_buf.index],
                                                                   MSM_VIDC_EXTRADATA_METADATA_MBI,
                                                                   EXTRADATA_BUFFER_FORMAT_MSM);
                            LOGIS("MBI size=%d, MBI data_size=%d",pv->size,pv->data_size);
                            if (pv != NULL)
                            {
                                 if (pv->data_size == 0)
                                 {
                                   pstCb->pstBufOut[v4l2_buf.index]->stExtra.u32FilledLen = 0;
                                 }
                            }
                        }
                        LOGIS("EXTRADATA: bytesused=%d, length=%d, data_offset=0x%x, "
                              "userptr=0x%lx, fd=%d, offset=0x%x",
                             v4l2_buf.m.planes[1].bytesused,v4l2_buf.m.planes[1].length,
                             v4l2_buf.m.planes[1].data_offset,
                             v4l2_buf.m.planes[1].m.userptr,v4l2_buf.m.planes[1].reserved[0],
                             v4l2_buf.m.planes[1].reserved[1]);
                    }

                    //Copy from input to output:
                    pthread_mutex_lock(&pstCb->mutex);
                    pstCb->pstBufOut[v4l2_buf.index]->pBuf->timestamp = 0;
                    if (v4l2_buf.flags & V4L2_QCOM_BUF_FLAG_EOS)
                    {
                        pstCb->pstBufOut[v4l2_buf.index]->pBuf->flags |=  VPP_BUFFER_FLAG_EOS;
                        if ((pstCb->u32InternalFlags & IP_DRAIN_PENDING)&&
                            (!(pstCb->u32InternalFlags & IP_EOS_PENDING)))
                        {
                            pstCb->pstBufOut[v4l2_buf.index]->stPixel.u32FilledLen = 0;
                            //The EOS Drain Output buffer is not needed and has invalid data.
                            pstCb->pstBufOut[v4l2_buf.index]->stExtra.u32FilledLen = 0;
                            pstCb->u32MeFlags |= FRC_ME_RELEASED_OUTPUT_EOS_DRAIN_DETECTED;
                            LOGI("Output EOS Drain detected! Flags=0x%x",
                                  pstCb->pstBufOut[v4l2_buf.index]->pBuf->flags);
                        }
                        else
                        {
                            pstCb->u32InternalFlags |= IP_EOS_PENDING;
                            pstCb->u32MeFlags |= FRC_ME_RELEASED_OUTPUT_EOS_DETECTED;
                            LOGI("Output EOS detected! Flags=0x%x",
                                  pstCb->pstBufOut[v4l2_buf.index]->pBuf->flags);
                        }
                    }
                    pstCb->pstBufOut[v4l2_buf.index]->pBuf->cookie_in_to_out = 0;
                    //pstCb->pstBufIn[0]->pBuf->cookie_in_to_out;

                    uint64_t timestamp = 0;

                    if ((v4l2_buf.length > 1) &&
                        ((pstCb->pstBufOut[v4l2_buf.index]->stExtra.u32FilledLen) ||
                         (pstCb->pstBufOut[v4l2_buf.index]->pBuf->flags & VPP_BUFFER_FLAG_EOS)))
                        if (VPP_OK!=u32VppIpFrcMe_TimestampListRead(&(pstCb->InputTimestampList),
                                                                    &timestamp))
                            timestamp = 0;

                    pstCb->pstBufOut[v4l2_buf.index]->pBuf->timestamp=timestamp;
                    VPP_FLAG_CLR(pstCb->pstBufOut[v4l2_buf.index]->u32InternalFlags,
                                 VPP_BUF_FLAG_FLUSHED);
                    LOGI("%s ME Output file information: ", __func__);
                    u32VppIpFrcMe_LogBufferInfo(pstCb->pstBufOut[v4l2_buf.index]);

                    t_StVppBuf * p=pstCb->pstBufOut[v4l2_buf.index];
                    pstCb->OutputBufsMEDone++;
                    pstCb->OutputBufsMEInside--;
                    pstCb->OutputBufsNeed++;
                    pstCb->OutputBufsInternalNeed++;
                    pstCb->pstBufOut[v4l2_buf.index] = NULL;
                    pthread_mutex_unlock(&pstCb->mutex);

                    // Return the Output buffers
                    if((p->stExtra.u32FilledLen==0) &&
                        (!(p->pBuf->flags &  VPP_BUFFER_FLAG_EOS))&&
                        (!(pstCb->u32InternalFlags & IP_FLUSH_PENDING)))
                    {
                        LOGE("Warning: Output buffer u32FilledLen=0! v4l2_mbi_size=%d", mbi_size);
                    }
                    VPP_FRC_ME_ZERO_BUF(pstCb, p,VPP_IP_FRC_BUF_ME_MBI_OUT,pstCb->OutputBufsMEDone);
                    VPP_FRC_ME_CRC_MBI_BUF(pstCb, p,pstCb->OutputBufsMEDone);
                    VPP_FRC_ME_DUMP_BUF(pstCb, p,VPP_IP_FRC_BUF_ME_MBI_OUT,pstCb->OutputBufsMEDone);
                    FRC_ME_STATS(pstCb, OBDCnt);
                    vVppIpCbLog(&pstCb->stBase.stCb, p, eVppLogId_IpBufDone);
                    u32VppIp_CbBufDone(&pstCb->stBase.stCb, VPP_PORT_OUTPUT, p);
                    pthread_cond_signal(&pstCb->cond);
                }
            }

            //Read events returned by the driver
            if (pfdesc[0].revents & POLLPRI)
            {
                rc = IOCTL(pfdesc[0].fd, VIDIOC_DQEVENT, &event);

                if (event.type == V4L2_EVENT_MSM_VIDC_FLUSH_DONE)
                {
                    LOGI("%s V4L2_EVENT_MSM_VIDC_FLUSH_DONE ", __func__);
                    vVppIpFrcMe_HandleEndPendingFlushDrain(pstCb);
                }
                else if (event.type == V4L2_EVENT_MSM_VIDC_HW_OVERLOAD)
                {
                    pstCb->u32MeEvent = V4L2_EVENT_MSM_VIDC_HW_OVERLOAD;
                    pthread_mutex_lock(&pstCb->mutex);
                    uint32_t u32InQSz = u32VppBufPool_Cnt(&pstCb->stInput.stQ);
                    uint32_t u32OutQSz = u32VppBufPool_Cnt(&pstCb->stOutput.stQ);
                    pthread_mutex_unlock(&pstCb->mutex);
                    LOGE("%s Error: V4L2_EVENT_MSM_VIDC_HW_OVERLOAD ", __func__);
                    u32VppIpFrcMe_LogStatus(pstCb, u32InQSz, u32OutQSz);
                    t_StVppEvt stEvt;
                    stEvt.eType = VPP_EVT_ERR_HW_OVERLOAD;
                    u32VppIpCbEvent(&pstCb->stBase.stCb, stEvt);
                }
                else if (event.type == V4L2_EVENT_MSM_VIDC_SYS_ERROR)
                {
                    pstCb->u32MeEvent = V4L2_EVENT_MSM_VIDC_SYS_ERROR;
                    pthread_mutex_lock(&pstCb->mutex);
                    uint32_t u32InQSz = u32VppBufPool_Cnt(&pstCb->stInput.stQ);
                    uint32_t u32OutQSz = u32VppBufPool_Cnt(&pstCb->stOutput.stQ);
                    pthread_mutex_unlock(&pstCb->mutex);
                    LOGE("%s Error: V4L2_EVENT_MSM_VIDC_SYS_ERROR ", __func__);
                    u32VppIpFrcMe_LogStatus(pstCb, u32InQSz, u32OutQSz);
                    t_StVppEvt stEvt;
                    stEvt.eType = VPP_EVT_ERR_SYSTEM;
                    u32VppIpCbEvent(&pstCb->stBase.stCb, stEvt);
                }
                else if (event.type == V4L2_EVENT_MSM_VIDC_MAX_CLIENTS)
                {
                    pstCb->u32MeEvent = V4L2_EVENT_MSM_VIDC_MAX_CLIENTS;
                    pthread_mutex_lock(&pstCb->mutex);
                    uint32_t u32InQSz = u32VppBufPool_Cnt(&pstCb->stInput.stQ);
                    uint32_t u32OutQSz = u32VppBufPool_Cnt(&pstCb->stOutput.stQ);
                    pthread_mutex_unlock(&pstCb->mutex);
                    LOGE("%s Error: V4L2_EVENT_MSM_VIDC_MAX_CLIENTS ", __func__);
                    u32VppIpFrcMe_LogStatus(pstCb, u32InQSz, u32OutQSz);
                    t_StVppEvt stEvt;
                    stEvt.eType = VPP_EVT_ERR_MAX_CLIENTS;
                    u32VppIpCbEvent(&pstCb->stBase.stCb, stEvt);
                }
                else if (event.type == V4L2_EVENT_MSM_VIDC_HW_UNSUPPORTED)
                {
                    pstCb->u32MeEvent = V4L2_EVENT_MSM_VIDC_HW_UNSUPPORTED;
                    pthread_mutex_lock(&pstCb->mutex);
                    uint32_t u32InQSz = u32VppBufPool_Cnt(&pstCb->stInput.stQ);
                    uint32_t u32OutQSz = u32VppBufPool_Cnt(&pstCb->stOutput.stQ);
                    pthread_mutex_unlock(&pstCb->mutex);
                    LOGE("%s Error: V4L2_EVENT_MSM_VIDC_HW_UNSUPPORTED ", __func__);
                    u32VppIpFrcMe_LogStatus(pstCb, u32InQSz, u32OutQSz);
                    t_StVppEvt stEvt;
                    stEvt.eType = VPP_EVT_ERR_HW_UNSUPPORTED;
                    u32VppIpCbEvent(&pstCb->stBase.stCb, stEvt);
                }
            }
            VPP_IP_PROF_STOP(&pstCb->stBase, FRC_ME_STAT_REC_WORKER);
        }
    }

addr_exit:
    LOGD("%s exited", __func__);

    return NULL;
}

/*!
 * @brief      Validate Configuration
 *
 * @description Checks the resolutions and the video format.
 *
 *
 * @input       pstCb      ME context
 *
 * @return     VPP_OK or VPP_ERR
 *
 */
static uint32_t u32VppIpFrcMe_ValidateConfig(t_StVppIpFrcMeCb *pstCb)
{
    struct vpp_port_param *pInput, *pOutput;

    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);
    pInput = &pstCb->stInput.stParam;
    pOutput = &pstCb->stOutput.stParam;

    if ((pInput->height >FRC_ME_MAX_RESOLUTION_H) || (pInput->height == 0))
    {
        LOGE("Error: Validation failed: invalid input height\n");
        LOG_EXITI_RET(VPP_ERR);
    }

    if ((pInput->width >FRC_ME_MAX_RESOLUTION_W) || (pInput->width== 0))
    {
        LOGE("Error: Validation failed: invalid input width\n");
        LOG_EXITI_RET(VPP_ERR);
    }

    if ((pInput->height != pOutput->height) && ((pInput->height/2) != pOutput->height))
    {
        LOGE("Error: Validation failed: height, input: %u, output: %u",
             pInput->height, pOutput->height);
        LOG_EXITI_RET(VPP_ERR);
    }

    if ((pInput->width != pOutput->width)&&((pInput->width / 2) != pOutput->width))
    {

        LOGE("Error: Validation failed: width, input: %u, output: %u",
             pInput->width, pOutput->width);
        LOG_EXITI_RET(VPP_ERR);
    }

    if (pInput->fmt != pOutput->fmt)
    {
        LOGE("Error: Validation failed: fmt, input: %u, output: %u",
             pInput->fmt, pOutput->fmt);
        LOG_EXITI_RET(VPP_ERR);
    }

    LOG_EXITI_RET(VPP_OK);
}


/************************************************************************
 * Global Interface Functions
 ************************************************************************/


/*!
 * @brief      FrcME Initialization
 *
 * @description Allocate ME context, create OS objects and start the ME working thread.
 *
 *
 * @input       pstCtx         VPP context
 * @input       u32Flags      flags
 * @input       cbs              CallBack functions structure
 *
 * @return     pointer to the new ME context structure or NULL
 *
 */
void *vpVppIpFrcMe_Init(t_StVppCtx *pstCtx, uint32_t u32Flags, t_StVppCallback cbs)
{
    LOGI("%s", __func__);

    int rc;
    uint32_t u32;
    t_StVppIpFrcMeCb *pstCb;


    //Allocate ME context
    pstCb = calloc(sizeof(t_StVppIpFrcMeCb), 1);
    if (!pstCb)
    {
        LOGE("Error: calloc failed for FrcMe context");
        goto ERROR_MALLOC;
    }

    memset((void*)pstCb, 0, sizeof(t_StVppIpFrcMeCb));

    u32VppIp_SetBase(pstCtx, u32Flags, cbs, &pstCb->stBase);

    u32 = VPP_IP_PROF_REGISTER(&pstCb->stBase, astFrcMeStatsCfg, u32FrcMeStatCnt);
    LOGE_IF(u32 != VPP_OK, "ERROR: unable to register stats, u32=%u", u32);
    //Queues initialization
    LOGI("%s : FRC_ME_PORT_BUF_Q_SZ=%d", __func__, FRC_ME_PORT_BUF_Q_SZ);
    u32VppBufPool_Init(&pstCb->stInput.stQ);
    u32VppBufPool_Init(&pstCb->stOutput.stQ);
    vpp_queue_init(&pstCb->stCmdQ, FRC_ME_CMD_Q_SZ);

    //OS ojects initialization
    rc = pthread_mutex_init(&pstCb->mutex, NULL);
    if (rc)
    {
        LOGE("Error: Unable to initialize FrcMe mutex");
        goto ERROR_MUTEX_INIT;
    }

    rc = pthread_cond_init(&pstCb->cond, NULL);
    if (rc)
    {
        LOGE("Error: Unable to init FrcMe condition variable");
        goto ERROR_COND_INIT;
    }

    rc = pthread_create(&pstCb->thread, NULL, vpVppIpFrcMe_Worker, pstCb);
    if (rc)
    {
        LOGE("Error: Unable to spawn FrcMe worker thread");
        goto ERROR_THREAD_CREATE;
    }

    //Set priority of the Worker less than of the Receiver
#if FRC_ME_INC_PRI
    {
        int policy = SCHED_FIFO;
        int priority_min = 0;
        struct sched_param sched;

        priority_min = sched_get_priority_min(SCHED_FIFO);

        if (priority_min<0)
           LOGE("Error: sched_get_priority_min failed");
        else
           sched.sched_priority = priority_min;

        rc = pthread_setschedparam(pstCb->thread, policy, &sched);
        if (rc != 0)
           LOGE("Error: pthread_setschedparam failed, ret=%d : %s",rc, strerror(rc));

        LOGI("Policy=%d, Priority=%d", policy, sched.sched_priority);
    }
#endif

    // Wait for the thread to launch before returning
    vVppIpFrcMe_WaitWorkerStart(pstCb);

    if (pstCb->u32WorkerErr != VPP_OK)
    {
        LOGE("Error: FrcMe Worker initialization error, err=%u", pstCb->u32WorkerErr);
        goto ERROR_WORKER_START;
    }

    VPP_IP_STATE_SET(pstCb, VPP_IP_STATE_INITED);
    return pstCb;

ERROR_WORKER_START:
    rc = pthread_join(pstCb->thread, NULL);
    if (rc)
        LOGE("Error: FrcMe pthread_join failed: %d --> %s", rc, strerror(rc));

ERROR_THREAD_CREATE:
    LOGI("destroying FrcMe condition variable");
    pthread_cond_destroy(&pstCb->cond);

ERROR_COND_INIT:
    LOGI("destroying FrcMe mutex");
    pthread_mutex_destroy(&pstCb->mutex);

ERROR_MUTEX_INIT:
    LOGI("destroying FrcMe pstCb");

    u32 = VPP_IP_PROF_UNREGISTER(&pstCb->stBase);
    LOGE_IF(u32 != VPP_OK, "ERROR: unable to unregister stats, u32=%u", u32);

    free(pstCb);

ERROR_MALLOC:
    return NULL;
}

/*!
 * @brief      FrcME Termination
 *
 * @description Terminate Working thread, flush ports, distroy OS objects, dealocate ME context .
 *
 * @input       ctx              ME context
 *
 * @return
 *
 */
void vVppIpFrcMe_Term(void *ctx)
{
    int rc;
    uint32_t u32;
    t_StVppIpFrcMeCb *pstCb;
    t_StVppIpCmd stCmd;

    LOGI("%s Started.\n", __func__);

    VPP_RET_VOID_IF_NULL(ctx);
    pstCb = FRC_ME_CB_GET(ctx);

    //Terminate Working thread
    stCmd.eCmd = VPP_IP_CMD_THREAD_EXIT;
    u32VppIpFrcMe_CmdPut(pstCb, &stCmd);

    rc = pthread_join(pstCb->thread, NULL);
    if (rc)
        LOGE("Error: pthread_join failed: %d --> %s", rc, strerror(rc));

    //Flush ports
    LOGI("%s FrcMe working thread terminated.\n", __func__);
    u32VppIpFrcMe_FlushPort(pstCb, VPP_PORT_INPUT);
    u32VppIpFrcMe_FlushPort(pstCb, VPP_PORT_OUTPUT);

    //Destroy OS objects
    rc = pthread_cond_destroy(&pstCb->cond);
    if (rc)
        LOGE("Error: pthread_cond_destroy failed: %d --> %s", rc, strerror(rc));

    rc = pthread_mutex_destroy(&pstCb->mutex);
    if (rc)
        LOGE("Error: pthread_mutex_destroy failed: %d --> %s", rc, strerror(rc));

    //Clean queues
    vpp_queue_term(&pstCb->stCmdQ);
    u32VppBufPool_Term(&pstCb->stInput.stQ);
    u32VppBufPool_Term(&pstCb->stOutput.stQ);

    u32 = VPP_IP_PROF_UNREGISTER(&pstCb->stBase);
    LOGE_IF(u32 != VPP_OK, "ERROR: unable to unregister stats, u32=%u", u32);

    //Dealocate ME context
    free(pstCb);
    LOGI("%s Done.\n", __func__);
}

/*!
 * @brief      FrcME Open
 *
 * @description Validate configuration, Init ME, Open ME.
 *
 * @input       ctx              ME context
 *
 * @return    VPP_OK or error
 *
 */
uint32_t u32VppIpFrcMe_Open(void *ctx)
{
    LOGI("%s\n", __func__);

    t_StVppIpFrcMeCb *pstCb;
    t_StVppIpCmd stCmd;

    VPP_RET_IF_NULL(ctx, VPP_ERR_PARAM);
    pstCb = FRC_ME_CB_GET(ctx);

    if (!VPP_IP_STATE_EQUAL(pstCb, VPP_IP_STATE_INITED))
        LOG_EXITI_RET(VPP_ERR_STATE);

    // Validate that the port configuration is valid
    if (u32VppIpFrcMe_ValidateConfig(pstCb) != VPP_OK)
        LOG_EXITI_RET(VPP_ERR_PARAM);

    //Init ME
    if (u32VppIpFrcMe_MeInit(pstCb) != VPP_OK)
    {
        LOGE("Error: u32VppIpFrcMe_MeInit() failed!");
        LOG_EXITI_RET(VPP_ERR_PARAM);
    }

    //Open ME
    if (u32VppIpFrcMe_MeOpen_HEVC(pstCb) != VPP_OK)
    {
        LOGE("Error: u32VppIpFrcMe_MeOpen() failed!");
        close(pstCb->MePollNotification);
        pstCb->MePollNotification = -1;
        close(pstCb->MeFd);
        pstCb->MeFd = -1;
        LOG_EXITI_RET(VPP_ERR_PARAM);
    }

    VPP_IP_STATE_SET(pstCb, VPP_IP_STATE_ACTIVE);
    LOG_EXITI_RET(VPP_OK);
}

/*!
 * @brief      FrcME Close
 *
 * @description Terminate ME.
 *
 * @input       ctx    ME context
 *
 * @return    VPP_OK or error
 *
 */
uint32_t u32VppIpFrcMe_Close(void *ctx)
{
    LOGI("%s\n", __func__);

    t_StVppIpFrcMeCb *pstCb;
    t_StVppIpCmd stCmd;

    VPP_RET_IF_NULL(ctx, VPP_ERR_PARAM);
    pstCb = FRC_ME_CB_GET(ctx);

    if (!VPP_IP_STATE_EQUAL(pstCb, VPP_IP_STATE_ACTIVE))
        LOG_EXITI_RET(VPP_ERR_STATE);

    //Close Venus Encoder
    if (u32VppIpFrcMe_MeTerm(pstCb) != VPP_OK)
        LOG_EXITI_RET(VPP_ERR_PARAM);

    VPP_IP_STATE_SET(pstCb, VPP_IP_STATE_INITED);

    LOG_EXITI_RET(VPP_OK);
}

/*!
 * @brief      FrcME Set port parameters
 *
 * @description Terminate ME.
 *
 * @input       ctx      ME context
 * @input       port     ME port
 * @input       param  ME parameters
 *
 * @return    VPP_OK or error
 *
 */
uint32_t u32VppIpFrcMe_SetParam(void *ctx,enum vpp_port port, struct vpp_port_param param)
{
    LOGI("%s(), port=%u", __func__, port);

    uint32_t u32Ret = VPP_OK;
    t_StVppIpFrcMeCb *pstCb;

    VPP_RET_IF_NULL(ctx, VPP_ERR_PARAM);
    pstCb = FRC_ME_CB_GET(ctx);

    if (!VPP_IP_STATE_EQUAL(pstCb, VPP_IP_STATE_INITED))
        LOG_EXITI_RET(VPP_ERR_STATE);

    pthread_mutex_lock(&pstCb->mutex);

    if (port == VPP_PORT_INPUT)
        pstCb->stInput.stParam = param;
    else if (port == VPP_PORT_OUTPUT)
    {
#if FRC_ME_MBI_TEST
        //This is specific for MBI application
        param.height /= 2;
        param.width /= 2;
#endif
        pstCb->stOutput.stParam = param;
    }
    else
        u32Ret = VPP_ERR_PARAM;

    LOG_PARAM_PTR(I, &pstCb->stInput.stParam, &pstCb->stOutput.stParam);

    pthread_mutex_unlock(&pstCb->mutex);

    LOG_EXITI_RET(u32Ret);
}

/*!
 * @brief      FrcME Set HQV  controls
 *
 * @description Not used
 *
 * @input       ctx      ME context
 * @input       ctrl      hqv controls
 *
 * @return    VPP_OK or error
 *
 */
uint32_t u32VppIpFrcMe_SetCtrl(void *ctx, struct hqv_control ctrl)
{
    LOGI("%s\n", __func__);

    uint32_t u32Ret = VPP_OK;
    t_StVppIpFrcMeCb *pstCb;

    VPP_RET_IF_NULL(ctx, VPP_ERR_PARAM);
    pstCb = FRC_ME_CB_GET(ctx);
    pstCb->ctrl=ctrl;

    LOG_EXITI_RET(u32Ret);
}

/*!
 * @brief     Get ME minimal buffer requirement
 *
 * @description Get the minimal number of input and output buffers necessary for processing
 *
 * @input          ctx      ME context
 * @output       pstInputBufReq      pointer destination for the requred number of input buffers
 * @output       pstOutputBufReq   pointer destination for the requred number of input buffers
 *
 * @return    VPP_OK or error
 *
 */
uint32_t u32VppIpFrcMe_GetBufferRequirements(void *ctx,
                                t_StVppIpBufReq *pstInputBufReq, t_StVppIpBufReq *pstOutputBufReq)
{
    LOGI("%s\n", __func__);

    t_StVppIpFrcMeCb *pstCb;

    VPP_RET_IF_NULL(ctx, VPP_ERR_PARAM);
    VPP_RET_IF_NULL(pstInputBufReq, VPP_ERR_PARAM);
    VPP_RET_IF_NULL(pstOutputBufReq, VPP_ERR_PARAM);
    pstCb = FRC_ME_CB_GET(ctx);

    pstInputBufReq->u32MinCnt=  FRC_ME_MIN_IN_BUFS;
    pstInputBufReq->u32PxSz = pstCb->stMeFormat_Input.fmt.pix_mp.plane_fmt[0].sizeimage;
    if (pstCb->stMeFormat_Input.fmt.pix_mp.num_planes > 1)
    {
        unsigned int extra_idx = pstCb->stMeFormat_Input.fmt.pix_mp.num_planes - 1;
        pstInputBufReq->u32ExSz = pstCb->stMeFormat_Input.fmt.pix_mp.plane_fmt[extra_idx].sizeimage;
    }
    else
    {
        pstInputBufReq->u32ExSz = 0;
    }

    pstOutputBufReq->u32MinCnt = pstCb->OutputBufsInternalPool;
    pstOutputBufReq->u32PxSz = pstCb->stMeFormat_Output.fmt.pix_mp.plane_fmt[0].sizeimage;
    if (pstCb->stMeFormat_Output.fmt.pix_mp.num_planes > 1)
    {
        unsigned int extra_idx = pstCb->stMeFormat_Output.fmt.pix_mp.num_planes - 1;
        pstOutputBufReq->u32ExSz=pstCb->stMeFormat_Output.fmt.pix_mp.plane_fmt[extra_idx].sizeimage;
    }
    else
    {
        pstOutputBufReq->u32ExSz = 0;
    }

    LOG_EXITI_RET(VPP_OK);
}

/*!
 * @brief     Get ME necessary buffers to be queued
 *
 * @description Get the current the number of input and output buffers necessary for processing
 *
 * @input          ctx           ME context
 * @output       pu32In      pointer destination for the necessary number of input buffers
 * @output       pu32Out   pointer destination for the  necessary number of input buffers
 *
 * @return    VPP_OK or error
 *
 */
uint32_t u32VppIpFrcMe_GetBufferNeed(void *ctx, uint32_t *pu32In, uint32_t *pu32Out)
{
    t_StVppIpFrcMeCb *pstCb;

    VPP_RET_IF_NULL(ctx, VPP_ERR_PARAM);
    VPP_RET_IF_NULL(pu32In, VPP_ERR_PARAM);
    VPP_RET_IF_NULL(pu32Out, VPP_ERR_PARAM);
    pstCb = FRC_ME_CB_GET(ctx);

    *pu32In  = pstCb->InputBufsNeed;
    *pu32Out = pstCb->OutputBufsNeed;
    return (VPP_OK);
}

/*!
 * @brief     Queue buffer into FrcME queue
 *
 * @description
 *
 * @input          ctx           ME context
 * @output       ePort        port
 * @output       pBuf         pointer to the buffer to be queued
 *
 * @return    VPP_OK or error
 *
 */
uint32_t u32VppIpFrcMe_QueueBuf(void *ctx, enum vpp_port ePort, t_StVppBuf *pBuf)
{
    LOGI("%s() port=%u\n", __func__, (uint32_t)ePort);

    uint32_t u32Ret = VPP_OK;
    uint32_t u32BypassFlags;
    t_StVppIpFrcMeCb *pstCb;
    t_EVppBufType eBufType;

    VPP_RET_IF_NULL(ctx, VPP_ERR_PARAM);
    VPP_RET_IF_NULL(pBuf, VPP_ERR_PARAM);
    pstCb = FRC_ME_CB_GET(ctx);

    vVppIpCbLog(&pstCb->stBase.stCb, pBuf, eVppLogId_IpQueueBuf);

    pthread_mutex_lock(&pstCb->mutex);
    if (ePort == VPP_PORT_INPUT)
    {
        if (VPP_FLAG_IS_SET(pBuf->u32InternalFlags, VPP_BUF_FLAG_FLUSHED))
        {
            LOGE("Error: Input buffer %p came with VPP_BUF_FLAG_FLUSHED flag set!", pBuf);
            VPP_FLAG_CLR(pBuf->u32InternalFlags, VPP_BUF_FLAG_FLUSHED);
        }

        if ((VPP_FLAG_IS_SET(pBuf->pBuf->flags, VPP_BUFFER_FLAG_EOS)) &&
            !(pBuf->stPixel.u32FilledLen))
        {
            //Workaround for the zero timestamp bug
            //Keep the current input timestamp
        }
        else
            pstCb->MeLastTimestamp=pBuf->pBuf->timestamp;   //Store the current input timestamp

        if (!(pBuf->stPixel.u32FilledLen) && (pBuf->eBufType == eVppBufType_Progressive))
        {
            VPP_FLAG_SET(pBuf->u32InternalFlags, VPP_BUF_FLAG_ME_INTERN_BYPASS);
            LOGI("Input with u32FilledLen=0 detected! Setting internal bypass flag");
        }

        u32Ret = u32VppIpFrcMe_PortBufPut(pstCb, &pstCb->stInput, pBuf);
        if (u32Ret == VPP_OK)
        {
            u32BypassFlags = VPP_BUF_FLAG_ME_INTERN_BYPASS | VPP_BUF_FLAG_BYPASS;
            if ((VPP_FLAG_IS_SET(pBuf->pBuf->flags, VPP_BUFFER_FLAG_EOS)) &&
                !(VPP_FLAG_IS_SET(pBuf->u32InternalFlags, u32BypassFlags)) &&
                (pBuf->eBufType == eVppBufType_Progressive))
            {
                pstCb->InputEosCnt++;
            }

            pstCb->InputBufsNeed--;
            FRC_ME_STATS(pstCb, InQCnt);
        }
        //VPP_FRC_ME_CRC_BUF(pstCb, pBuf, pstCb->stats.u32InQCnt);

    }
    else if (ePort == VPP_PORT_OUTPUT)
    {
        pBuf->pBuf->timestamp = 0;   //Clean output buffer timestamp
        u32Ret = u32VppIpFrcMe_PortBufPut(pstCb, &pstCb->stOutput, pBuf);
        if (u32Ret == VPP_OK)
        {
            pstCb->OutputBufsNeed--;
            FRC_ME_STATS(pstCb, OutQCnt);
        }
    }

    pthread_mutex_unlock(&pstCb->mutex);
    LOG_EXITI_RET(u32Ret);
}

/*!
 * @brief     Queue Flush command in the command queue
 *
 * @description
 *
 * @input          ctx           ME context
 * @output       ePort        port
 *
 * @return    VPP_OK or error
 *
 */
uint32_t u32VppIpFrcMe_Flush(void *ctx, enum vpp_port ePort)
{
    LOGI("%s port=%d\n", __func__, ePort);

    uint32_t u32Ret = VPP_OK;
    t_StVppIpFrcMeCb *pstCb;
    t_StVppIpCmd stCmd;

    VPP_RET_IF_NULL(ctx, VPP_ERR_PARAM);
    pstCb = FRC_ME_CB_GET(ctx);

    stCmd.eCmd = VPP_IP_CMD_FLUSH;
    stCmd.flush.ePort = ePort;
    u32Ret=u32VppIpFrcMe_CmdPut(pstCb, &stCmd);

    LOG_EXITI_RET(u32Ret);
}

/*!
 * @brief     Queue Drain command in the command queue
 *
 * @description
 *
 * @input          ctx           ME context
 * @output       ePort        port
 *
 * @return    VPP_OK or error
 *
 */
uint32_t u32VppIpFrcMe_Drain(void *ctx)
{
    LOGI("%s\n", __func__);

    uint32_t u32Ret = VPP_OK;
    t_StVppIpFrcMeCb *pstCb;
    t_StVppIpCmd stCmd;

    VPP_RET_IF_NULL(ctx, VPP_ERR_PARAM);
    pstCb = FRC_ME_CB_GET(ctx);

    stCmd.eCmd = VPP_IP_CMD_DRAIN;
    u32Ret=u32VppIpFrcMe_CmdPut(pstCb, &stCmd);

    LOG_EXITI_RET(u32Ret);
}


