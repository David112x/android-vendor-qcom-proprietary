/*!
 * @file vpp_pipeline.h
 *
 * @cr
 * Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.

 * @services
 */
#ifndef _VPP_PIPELINE_H_
#define _VPP_PIPELINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vpp_queue.h"
#include "vpp_reg.h"
#include "vpp_ip.h"

enum {
    eVppPl_WorkerStarted,
    eVppPl_Closing,
    eVppPl_InputFlushPending,
    eVppPl_OutputFlushPending,
    eVppPl_ReconfigurePending,
    eVppPl_ReconfigureCallback,
    eVppPl_ReconfigToBypass,
    eVppPl_Max = 32,
} t_EVppFlagPos;

#define PL_WORKER_STARTED               (1 << eVppPl_WorkerStarted)
#define PL_CLOSE_PENDING                (1 << eVppPl_Closing)
#define PL_INPUT_FLUSH_PENDING          (1 << eVppPl_InputFlushPending)
#define PL_OUTPUT_FLUSH_PENDING         (1 << eVppPl_OutputFlushPending)
#define PL_RECONFIGURE_PENDING          (1 << eVppPl_ReconfigurePending)
#define PL_RECONFIGURE_CALLBACK         (1 << eVppPl_ReconfigureCallback)
#define PL_RECONFIGURE_TO_BYPASS        (1 << eVppPl_ReconfigToBypass)

#define PL_CMD_Q_SZ                     30

typedef enum {
    VPP_PLSTATE_NULL,
    VPP_PLSTATE_INITED,
    VPP_PLSTATE_ACTIVE,
} t_EVppPipelineState;

typedef enum {
    VPP_PL_CMD_THREAD_EXIT = 1,
    VPP_PL_CMD_OPEN,
    VPP_PL_CMD_CLOSE,
    VPP_PL_CMD_FLUSH,
    VPP_PL_CMD_DRAIN_REQUEST,
    VPP_PL_CMD_DRAIN_COMPLETE,
    VPP_PL_CMD_STOP,
    VPP_PL_CMD_MAX,
} t_EVppPipelineCmd;

typedef struct {
    t_EVppPipelineCmd eCmd;
    union {
        struct {
            enum vpp_port ePort;
        } flush;
    };
} t_StVppPipelineCmd;

typedef struct {
    t_StVppCtx *pstCtx;
    uint32_t u32IpPos;
} t_StVppPipelineCbCookie;

typedef struct StVppPipelineCustCtrlNode {
    struct StVppPipelineCustCtrlNode *pNext;
    struct hqv_ctrl_custom ctrl;
} t_StVppPipelineCustCtrlNode;

typedef struct StVppPipelineCustCtrlList {
    uint32_t u32Cnt;
    struct StVppPipelineCustCtrlNode *pHead;
} t_StVppPipelineCustCtrlList;

typedef struct StVppPipelineCb {

    pthread_t thread;

    pthread_cond_t cond_to_worker;
    pthread_cond_t cond_from_worker;
    pthread_mutex_t mutex;

    t_StVppQueue stCmdQ;
    t_StVppPipelineCmd astCmdNode[PL_CMD_Q_SZ];

    uint32_t au32PendingFlags[VPP_IP_BLOCK_MAX];
    uint32_t u32PipelineFlags;

    t_StVppParam *pstParam;

    t_EVppPipelineState eState;
    uint32_t bBypass;

    t_StVppUsecase *pstUcOpen;      // Current usecase that pipeline is Opened with

    t_StVppPipelineCustCtrlList stCustCtrlList;

    uint32_t u32IpCnt;

    uint32_t u32ReconfigErr;

    // array of contexts returned by an IP block's init function
    void *avpIpCtx[VPP_IP_BLOCK_MAX];
    t_StVppPipelineCbCookie astCbCookie[VPP_IP_BLOCK_MAX];
    t_StVppCallback stCallbacks;

    t_StVppBufPool stInputQ;
    t_StVppBufPool stOutputQ;

    t_StVppCtx *pstCtx;
    void *pvStatsHdl;

    struct {
        STAT_DECL(RequestFlushInput);
        STAT_DECL(RequestFlushOutput);

        STAT_DECL(FlushedInputBuf);
        STAT_DECL(FlushedOutputBuf);
        STAT_DECL(RequestReconfigure);
        STAT_DECL(DrainDoneCb);

        STAT_DECL(IssueIBD);
        STAT_DECL(IssueOBD);
    } VPP_STAT_CB_NM;
} t_StVppPipelineCb;

/***************************************************************************
 * Function Prototypes
 ***************************************************************************/

uint32_t u32VppPipeline_Init(t_StVppCtx *pstCb, t_StVppCallback stCbs);
uint32_t u32VppPipeline_Term(t_StVppCtx *pstCb);
uint32_t u32VppPipeline_Open(t_StVppCtx *pstCb, t_StVppUsecase *pstUc,
                             t_StVppParam *pstParam, t_StVppHqvCtrl *pstCtrl);
uint32_t u32VppPipeline_Close(t_StVppCtx *pstCb);

uint32_t u32VppPipeline_SetHqvCtrl(t_StVppCtx *pstCb, struct hqv_control ctrl);
uint32_t u32VppPipeline_Reconfigure(t_StVppCtx *pstCb, t_StVppUsecase *pstUc,
                                    t_StVppParam *pstParam, t_StVppHqvCtrl *pstCtrl);

uint32_t u32VppPipeline_QueueBuf(t_StVppCtx *pstCb, enum vpp_port ePort,
                                 t_StVppBuf *pstBuf);
uint32_t u32VppPipeline_RequestFlush(t_StVppCtx *pstCb, enum vpp_port ePort);
uint32_t u32VppPipeline_RequestReconfigure(t_StVppCtx *pstCb);

#ifdef __cplusplus
 }
#endif

#endif /* _VPP_PIPELINE_H_ */
