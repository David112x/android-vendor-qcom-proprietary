/*!
 * @file test_ip_mc.c
 *
 * @cr
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

 * @services    Implements tests for the mc (Motion Compensation) submodule
 */

#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
//#define __STDC_FORMAT_MACROS
//#include <inttypes.h>

#include "buf_pool.h"
#include "test_utils.h"

#include "dvpTest.h"
#include "dvpTest_tb.h"

#define VPP_LOG_TAG     VPP_LOG_UT_MC_TAG
#define VPP_LOG_MODULE  VPP_LOG_UT_MC
#include "vpp_dbg.h"
#include "vpp.h"
// #include "vpp_core.h"
// #include "vpp_ctx.h"
#include "vpp_reg.h"
#include "vpp_ip.h"
#include "vpp_ip_frc_mc.h"
#include "vpp_utils.h"

// #include "vpp_uc.h"

/************************************************************************
 * Local definitions
 ***********************************************************************/
#define INPUT_FLUSH_DONE        (1<<0)
#define OUTPUT_FLUSH_DONE       (1<<1)
#define DRAIN_DONE              (1<<2)

#define TIMESTAMP_MS            33000

struct mc_test_ctx {
    void *mc_ctx; // context returned from mc init
    t_StVppCtx *pstCtx;
    uint32_t u32ExpExtraLen;
};

/************************************************************************
 * Local static variables
 ***********************************************************************/
static struct test_ctx tctx;
static struct mc_test_ctx stCtxMc;
const uint32_t u32InBufCnt = 6, u32OutBufCnt = 6;
uint32_t u32MbiBufCnt = 6;
uint32_t u32FlagCorrupt = 0;
uint32_t u32Inited = 0;

static uint32_t u32TestTimeStamp = 0;//do not test time stamp as default
/************************************************************************
 * Forward Declarations
 ************************************************************************/

/************************************************************************
 * Local Functions
 ***********************************************************************/
void test_mc_input_buffer_done(void *pv, t_StVppBuf *pBuf)
{
    LOGI("%s() with buffer=%p", __func__, pBuf);
    struct test_ctx *pCtx = (struct test_ctx *)pv;
    struct bufnode *pNode = pBuf->pBuf->cookie;

    DVP_ASSERT_TRUE(pNode->owner == BUF_OWNER_LIBRARY);
    pNode->owner = BUF_OWNER_CLIENT;
    pthread_mutex_lock(&tctx.mutex);
#if 0 //debug timestamp.
    //Normally We should only get MBI buf (pBuf->eBufPxType==eVppBufPxDataType_Compressed)
    printf("in buf timestamp = %llu eBufPxType=%s\n",
           pBuf->pBuf->timestamp,
           pBuf->eBufPxType==eVppBufPxDataType_Raw? "eVppBufPxDataType_Raw":"eVppBufPxDataType_Compressed");
#endif

    //Note: When flush, input pixel buffer may be returned to input port.
    if (pBuf->eBufPxType == eVppBufPxDataType_Compressed)
        put_buf(pCtx->buf_pool_ext, pNode);
    else
    {
        put_buf(pCtx->buf_pool, pNode);
        tctx.u32InRxCnt++;
        tctx.u32InHoldCnt--;
    }
    pthread_mutex_unlock(&tctx.mutex);

    //DVP_ASSERT_TRUE(pBuf->eBufPxType == eVppBufPxDataType_Compressed);
    LOGD("pBuf->eBufPxType=%d", pBuf->eBufPxType);
    pthread_cond_signal(&pCtx->cond);
}

void test_mc_output_buffer_done(void *pv, t_StVppBuf *pBuf)
{
    struct test_ctx *pCtx = (struct test_ctx *)pv;
    struct bufnode *pNode = pBuf->pBuf->cookie;

    t_EVppBufType eType;

    LOGI("%s() with buffer=0x%p", __func__, pBuf);
    DVP_ASSERT_TRUE(pNode->owner == BUF_OWNER_LIBRARY);
    pNode->owner = BUF_OWNER_CLIENT;

    eType = eVppBuf_GetFrameType(pBuf);
    DVP_ASSERT_EQUAL(eType, eVppBufType_Progressive);

    pthread_mutex_lock(&tctx.mutex);
    // VPP_FLAG_SET(pstBufOut[0]->pBuf->flags, VPP_BUFFER_FLAG_DATACORRUPT);
    if (VPP_FLAG_IS_SET(pBuf->pBuf->flags,VPP_BUFFER_FLAG_DATACORRUPT))
        u32FlagCorrupt = 1;
    //test timestamp
    if (u32TestTimeStamp && ((tctx.u32OutRxCnt + tctx.u32InRxCnt) < (u32InBufCnt - 1)) &&
        pNode->port_owner == VPP_PORT_OUTPUT && pBuf->stPixel.u32FilledLen != 0 &&
        tctx.u32InRxCnt)
    {
        uint64_t u64OutTimestamp = (tctx.u32InRxCnt * TIMESTAMP_MS) - (TIMESTAMP_MS / 2);
#if 0 //debug timestamp
        printf("out Buf timestamp = %llu eBufPxType=%s port=%s\n",
                pBuf->pBuf->timestamp,
                pBuf->eBufPxType == eVppBufPxDataType_Raw? "eVppBufPxDataType_Raw":"eVppBufPxDataType_Compressed",
                pNode->port_owner == VPP_PORT_INPUT? "IN":"OUT");
        printf("pBuf->pBuf->timestamp = %llu u64OutTimestamp = %llu\n", pBuf->pBuf->timestamp, u64OutTimestamp);
#endif
        DVP_ASSERT_TRUE(pBuf->pBuf->timestamp == u64OutTimestamp);
    }
    //if (pNode->stPixel.u32FilledLen)
    //{
    //    dump_buf(pNode);
    //    DVP_ASSERT_EQUAL(pNode->pIntBuf->stExtra.u32FilledLen, pMcCtx->u32ExpExtraLen);
    //    LOGI("extradata: expFillLen=%u, act_fill_len=%u", pMcCtx->u32ExpExtraLen,
    //         pNode->pIntBuf->stExtra.u32FilledLen);
    //    validate_extradata_integrity(pNode);
    //}
    if (pNode->port_owner == VPP_PORT_INPUT)
    {
        tctx.u32InRxCnt++;
        tctx.u32InHoldCnt--;
    }
    else
    {
        tctx.u32OutRxCnt++;
        tctx.u32OutHoldCnt--;
    }

    put_buf(pCtx->buf_pool, pNode);

    pthread_mutex_unlock(&tctx.mutex);
    pthread_cond_signal(&pCtx->cond);

}

void test_mc_event(void *pv, t_StVppEvt stEvt)
{
    LOGI("%s() got event: %u", __func__, stEvt.eType);
    struct test_ctx *pCtx = (struct test_ctx *)pv;

    pthread_mutex_lock(&pCtx->mutex);

    if (stEvt.eType == VPP_EVT_FLUSH_DONE)
    {
        if (stEvt.flush.ePort == VPP_PORT_INPUT)
            pCtx->u32Flags |= INPUT_FLUSH_DONE;
        else if (stEvt.flush.ePort == VPP_PORT_OUTPUT)
            pCtx->u32Flags |= OUTPUT_FLUSH_DONE;
    }
    else if (stEvt.eType == VPP_EVT_DRAIN_DONE)
    {
        pCtx->u32Flags |= DRAIN_DONE;
    }

    pthread_mutex_unlock(&pCtx->mutex);

    pthread_cond_signal(&pCtx->cond);
}

static uint32_t u32CheckMem(char *mem1,char *mem2,uint32_t len)
{
    uint32_t i = 0 ;
    for (i=0;i<len;i++)
    {
        if(mem1[i] != mem2[i])
            return VPP_ERR;
    }
    return VPP_OK;
}

static int32_t s32ParamTest(char* cLocalPara, char *cGlobalPara, size_t ParamSize, char *cParamName)
{
    uint32_t u32;
    struct hqv_control ctrl;
    uint32_t u32ParamId=0;
    uint32_t u32ParamSubId = 0;

    uint32_t fGlobalLocal = 0;

    //Test both global (fGlobalLocal==0) and local (fGlobalLocal==1)
    for (fGlobalLocal = 0; fGlobalLocal < 2; fGlobalLocal++)
    {
        char *ptr;
        LOGD("%s(): Test %s parameter setting u32ParamId=%d u32ParamSubId=%d.",
                __func__, fGlobalLocal==0 ? "global" : "local", u32ParamId, u32ParamSubId);

        // Set to manual and custom
        ctrl.mode = HQV_MODE_MANUAL;
        ctrl.ctrl_type = HQV_CONTROL_CUST;
        ctrl.custom.id = VPP_IP_FRC;
        ctrl.custom.len = ParamSize + FRCMC_CUSTOM_HEADER_SIZE;//header length is 4
        char *cPtr  = (char*)ctrl.custom.data;
        *cPtr++ = ParamSize + FRCMC_CUSTOM_HEADER_SIZE; //lenth low 8-bit
        *cPtr++ = 0;// length hi 8-bit
        uint32_t tmp = fGlobalLocal;
        tmp |= u32ParamId << FRCMC_GLOBAL_LOCAL_BIT_LENGTH;
        tmp |= u32ParamSubId << (FRCMC_PARAMID_BIT_LENGTH + FRCMC_GLOBAL_LOCAL_BIT_LENGTH);
        *cPtr++ = tmp & 0xff;       //ID low 8-bit
        *cPtr++ = (tmp>>8) & 0xff;  //ID hi 8-bit
        //LOGD("ThisID=0x%x",tmp);

        memset(cPtr, 0x55 + fGlobalLocal, ParamSize);
        u32 = u32VppIpFrcMc_SetCtrl(stCtxMc.mc_ctx, ctrl);
        DVP_ASSERT_TRUE(u32 == VPP_OK);

        //check result
        if (fGlobalLocal == 0)
            ptr = cGlobalPara;
        else
            ptr = cLocalPara;

        //printf("cGlobalPara =%p cLocalPara=%p cPtr=%p\n",cGlobalPara, cLocalPara, cPtr);

        u32 = u32CheckMem((char*)cPtr, ptr, ParamSize);
        DVP_ASSERT_TRUE(u32 == VPP_OK);
        LOGD("%s() After setting u32VppIpFrcMc_SetCtrl(),pstCustomFrcMcParams=%p 0x%x %x %x %x",
                __func__,ptr, ptr[0],ptr[1],ptr[2],ptr[3]);

        if (fGlobalLocal == 0)//Global
        {
            //check global is the same as local
            char *ptr1 = cGlobalPara;
            LOGD("%s(),u32ParamSubId=%d After global parameters setting, local %s=%p 0x%x %x %x %x",
                    __func__, u32ParamSubId, cParamName, ptr1, ptr1[0],ptr1[1],ptr1[2],ptr1[3]);
            ptr = cLocalPara;
            LOGD("%s(),u32ParamSubId=%d After global parameters setting, global %s=%p 0x%x %x %x %x",
                    __func__, u32ParamSubId, cParamName, ptr, ptr[0],ptr[1],ptr[2],ptr[3]);
            u32 = u32CheckMem((char*)ptr1, ptr, ParamSize);
            DVP_ASSERT_TRUE(u32 == VPP_OK); //local parameters the same as global for session2
        }
        if (fGlobalLocal == 1)//local
        {
            //check global is not the same as local
            char *ptr1 = cLocalPara;
            LOGD("%s(),u32ParamSubId=%d After local parameters setting, local %s=%p 0x%x %x %x %x",
                    __func__, u32ParamSubId, cParamName, ptr1, ptr1[0],ptr1[1],ptr1[2],ptr1[3]);
            char *ptr = cGlobalPara;
            LOGD("%s(),u32ParamSubId=%d After local parameters setting, global %s=%p 0x%x %x %x %x",
                    __func__, u32ParamSubId, cParamName, ptr, ptr[0],ptr[1],ptr[2],ptr[3]);
            u32 = u32CheckMem((char*)ptr1, ptr, ParamSize);
            DVP_ASSERT_TRUE(u32 != VPP_OK);
        }
    }
    return 0;
}

#define FLAG_BYPASS_BUFFER 0x1
#define FLAG_BYPASS_NONPROGRESIVE_BUFFER 0x2
#define FLAG_BYPASS_ALGO_MC_DISABLE 0x4
#define FLAG_BUFFER_FLAG_DATACORRUPT 0x8
#define FLAG_BUFFER_FLAG_EOS 0x10
static uint32_t u32QueueInput(uint32_t u32Flags)
{
    uint32_t u32Ret;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32BufTotal;
    struct hqv_control ctrl;

    LOGI("%s()", __func__);

    u32BufTotal = u32InBufCnt + u32OutBufCnt;

    LOGI("%s() init_buf_pool()", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params,
                           u32BufTotal, VPP_TRUE);
    if (u32Ret != VPP_OK)
        return u32Ret;
    LOGI("%s() init_buf_pool() buf_pool_ext", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params,
                           u32InBufCnt, VPP_TRUE);
    if (u32Ret != VPP_OK)
        return u32Ret;

    t_StVppIpFrcMcCb *pstCb = (t_StVppIpFrcMcCb *)stCtxMc.mc_ctx;
    if(!(u32Flags & FLAG_BYPASS_ALGO_MC_DISABLE))
    {
        // Set to manual and frc
        ctrl.mode = HQV_MODE_MANUAL;
        ctrl.ctrl_type = HQV_CONTROL_FRC;
        ctrl.frc.mode = HQV_FRC_MODE_LOW; //or HQV_FRC_MODE_MED or HQV_FRC_MODE_HIGH
        u32Ret = u32VppIpFrcMc_SetCtrl(pstCb, ctrl);
        LOGD("u32VppIpFrcMc_SetCtrl ,return = %d ", u32Ret);
        if (u32Ret != VPP_OK)
            return u32Ret;
    }
    else //((u32Flags & FLAG_BYPASS_ALGO_MC_DISABLE))
    {
        ctrl.mode = HQV_MODE_OFF;
        u32Ret = u32VppIpFrcMc_SetCtrl(pstCb, ctrl);
        LOGD("u32VppIpFrcMc_SetCtrl ,return = %d ", u32Ret);
        if (u32Ret != VPP_OK)
            return u32Ret;
    }

    // Queue to input pool
    LOGI("%s() Push buffers loop", __func__);
    for (i = 0; i < u32MbiBufCnt; i++)
    {
        //2. MBI buf
        pNode = get_buf(tctx.buf_pool_ext);
        DVP_ASSERT_PTR_NNULL(pNode);

        if(pNode)
        {
            u32Ret = fill_mbi_buf(pNode);
            if (u32Ret)
                LOGD("Unable to fill MBI buffer.  May contain garbage data");
            pNode->owner = BUF_OWNER_LIBRARY;

            pNode->pIntBuf->eBufPxType = eVppBufPxDataType_Compressed;
            pNode->pIntBuf->pBuf->timestamp = i * TIMESTAMP_MS;  //Inc with 33ms every input buffer
            u32Ret = u32VppIpFrcMc_QueueBuf(stCtxMc.mc_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
        }
        else
        {
            DVP_ASSERT_FAIL();
            LOGE("%s(): pNode == NULL",__func__);
            return VPP_ERR;
        }

        //1. input pixel buf
        pNode = get_buf(tctx.buf_pool);

        if(pNode == NULL)
        {
            LOGE("%s(): pNode == NULL",__func__);
            return VPP_ERR;
        }

        fill_buf(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;
        pNode->port_owner = VPP_PORT_INPUT;

        if(u32Flags & FLAG_BYPASS_BUFFER)
        {
            VPP_FLAG_SET(pNode->pIntBuf->u32InternalFlags, VPP_BUF_FLAG_BYPASS);
        }
        if(u32Flags & FLAG_BYPASS_NONPROGRESIVE_BUFFER)
        {
            pNode->pIntBuf->eBufType = eVppBufType_Interleaved_TFF;//!= eVppBufType_Progressive
        }

        if(u32Flags & FLAG_BUFFER_FLAG_DATACORRUPT)
        {
            VPP_FLAG_SET(pNode->pIntBuf->pBuf->flags, VPP_BUFFER_FLAG_DATACORRUPT);
        }

        if((u32Flags & FLAG_BUFFER_FLAG_EOS) && (u32MbiBufCnt - 1 == i))
        {
            VPP_FLAG_SET(pNode->pIntBuf->pBuf->flags, VPP_BUFFER_FLAG_EOS);
        }

        pNode->pIntBuf->eBufPxType = eVppBufPxDataType_Raw;
        pNode->pIntBuf->pBuf->timestamp = i * TIMESTAMP_MS;  //Inc with 33ms every input buffer
        u32Ret = u32VppIpFrcMc_QueueBuf(stCtxMc.mc_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
        pthread_mutex_lock(&tctx.mutex);
        tctx.u32InPutCnt++;
        tctx.u32InHoldCnt++;
        pthread_mutex_unlock(&tctx.mutex);
    }
    return VPP_OK;
}

static uint32_t u32BypassTest(uint32_t u32Flags)
{
    uint32_t u32Ret;
    uint32_t i;
    struct bufnode *pNode;

    u32TestTimeStamp = 0;//do not test time stamp in this test
    uint32_t u32BufTotal;

    u32BufTotal = u32InBufCnt + u32OutBufCnt;

    u32MbiBufCnt = 5;

    u32Ret = u32QueueInput(u32Flags);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMc_Drain(stCtxMc.mc_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    for (i = 0; i < u32OutBufCnt; i++)
    {
        //3. Output pixel buf
        pNode = get_buf(tctx.buf_pool);
        DVP_ASSERT_PTR_NNULL(pNode);

        if(pNode)
        {
            pNode->owner = BUF_OWNER_LIBRARY;
            pNode->port_owner = VPP_PORT_OUTPUT;
            pNode->pIntBuf->eBufPxType = eVppBufPxDataType_Raw;
            u32Ret = u32VppIpFrcMc_QueueBuf(stCtxMc.mc_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            pthread_mutex_lock(&tctx.mutex);
            tctx.u32OutPutCnt++;
            tctx.u32OutHoldCnt++;
            pthread_mutex_unlock(&tctx.mutex);
        }
        else
        {
            DVP_ASSERT_FAIL();
            LOGE("%s(): pNode == NULL",__func__);
            return VPP_ERR;
        }
    }

    pthread_mutex_lock(&tctx.mutex);

    while (!(tctx.u32Flags & DRAIN_DONE))
        pthread_cond_wait(&tctx.cond, &tctx.mutex);
    //printf("Before while: tctx.buf_pool->u32ListSz=%d u32BufTotal=%d\n", tctx.buf_pool->u32ListSz, u32BufTotal);
    //while (tctx.buf_pool->u32ListSz < u32BufTotal - 2)

    //{
    //    pthread_cond_wait(&tctx.cond, &tctx.mutex);
    //    printf("tctx.buf_pool->u32ListSz=%d u32BufTotal=%d\n", tctx.buf_pool->u32ListSz, u32BufTotal);
    //}
    pthread_mutex_unlock(&tctx.mutex);

    if(u32Flags & FLAG_BUFFER_FLAG_DATACORRUPT)
        //DVP_ASSERT_TRUE(pstCb->u32Flags & IP_CORRUPT_BUFFERS);
        DVP_ASSERT_TRUE(u32FlagCorrupt);
    else
        DVP_ASSERT_FALSE(u32FlagCorrupt);
    //printf("%s(): pstCb = %p pstCb->u32Flags = 0x%x\n", __func__, pstCb, pstCb->u32Flags);

    DVP_ASSERT_TRUE(tctx.u32Flags & DRAIN_DONE);
    //DVP_ASSERT_EQUAL(tctx.buf_pool->u32ListSz, u32BufTotal);

    //LOGI("freeing buffer pools");
    //free_buf_pool(tctx.buf_pool, VPP_TRUE);
    return VPP_OK;
}

static void IpMcInit(uint32_t u32Width, uint32_t u32Height)
{
    uint32_t u32Ret;

    tctx_common_init(&tctx);

    tctx.cb.input_buffer_done = test_mc_input_buffer_done;
    tctx.cb.output_buffer_done = test_mc_output_buffer_done;
    tctx.cb.event = test_mc_event;
    tctx.cb.pv = &tctx;

    tctx.pPrivateCtx = &stCtxMc;

    tctx_set_port_params(&tctx, u32Width, u32Height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width = u32Width;
    tctx.params.u32Height = u32Height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    char cDir[] = "/data/test/input/";
    char cFileName[] = "03_rotterdam_skater_nv12_padded.yuv";
    strlcpy(tctx.params.cInputPath, cDir, sizeof(tctx.params.cInputPath));
    strlcpy(tctx.params.cInputName, cFileName, sizeof(tctx.params.cInputName));

    char cMbiFileName1[] = "03_rotterdam_skater_QHD_soft_pass1.mbi";
    char cMbiFileName2[] = "03_rotterdam_skater_QHD_soft_pass2.mbi";
    strlcpy(tctx.params.cInputNameMbi1, cMbiFileName1, sizeof(tctx.params.cInputNameMbi1));
    strlcpy(tctx.params.cInputNameMbi2, cMbiFileName2, sizeof(tctx.params.cInputNameMbi2));

    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    char cDirOut[] = "/data/test/output/";
    char cFileNameOut[] = "03_rotterdam_skater_nv12_padded.bin";
    strlcpy(tctx.params.cOutputPath, cDirOut, sizeof(tctx.params.cOutputPath));
    strlcpy(tctx.params.cOutputName, cFileNameOut, sizeof(tctx.params.cOutputName));

    struct vpp_callbacks cb;
    memset(&cb, 0, sizeof(cb));
    stCtxMc.pstCtx = vpp_init(0, cb);

    stCtxMc.mc_ctx = vpVppIpFrcMc_Init(stCtxMc.pstCtx, 0, tctx.cb);
    DVP_ASSERT_PTR_NNULL(stCtxMc.mc_ctx);

    u32Ret = u32VppIpFrcMc_SetParam(stCtxMc.mc_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMc_SetParam(stCtxMc.mc_ctx, VPP_PORT_OUTPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMc_Open(stCtxMc.mc_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    //buf_params_init_default(&tctx.params, &tctx.port_param);

    u32Inited = 1;
}

static void IpMc_Term(void)
{
    u32VppIpFrcMc_Flush(stCtxMc.mc_ctx, VPP_PORT_INPUT);
    u32VppIpFrcMc_Flush(stCtxMc.mc_ctx, VPP_PORT_OUTPUT);
    u32VppIpFrcMc_Close(stCtxMc.mc_ctx);

    usleep(200000);

    LOGD("freeing buffer pools");
    free_buf_pool(tctx.buf_pool, VPP_TRUE);

    usleep(200000);
    vVppIpFrcMc_Term(stCtxMc.mc_ctx);

    vpp_term(stCtxMc.pstCtx);
    stCtxMc.pstCtx = NULL;

    tctx_common_destroy(&tctx);
}

static void FrcMcAlgoTest(uint32_t u32Width, uint32_t u32Height)
{
    uint32_t u32Ret;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32BufTotal;
    u32BufTotal = u32InBufCnt + u32OutBufCnt;
    u32TestTimeStamp = 1;//test time stamp in this test

    if (u32Inited && ( tctx.params.u32Width != u32Width || tctx.params.u32Height != u32Height))
    {
        IpMc_Term();
        IpMcInit(u32Width, u32Height);
    }

    u32Ret = u32QueueInput(0);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    for (i = 0; i < u32OutBufCnt; i++)
    {
        //3. Output pixel buf
        pNode = get_buf(tctx.buf_pool);
        DVP_ASSERT_PTR_NNULL(pNode);

        if(pNode)
        {
            pNode->owner = BUF_OWNER_LIBRARY;
            pNode->port_owner = VPP_PORT_OUTPUT;
            pNode->pIntBuf->eBufPxType = eVppBufPxDataType_Raw;
            u32Ret = u32VppIpFrcMc_QueueBuf(stCtxMc.mc_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            pthread_mutex_lock(&tctx.mutex);
            tctx.u32OutPutCnt++;
            tctx.u32OutHoldCnt++;
            pthread_mutex_unlock(&tctx.mutex);
            LOGI("\nPush loop %d",i);
        }
        else
        {
            DVP_ASSERT_FAIL();
            LOGE("%s(): pNode == NULL",__func__);
            return;
        }
    }
    pthread_mutex_lock(&tctx.mutex);

    LOGI("\nentering wait loop");
    while (tctx.u32InHoldCnt > 1)
    {
        pthread_cond_wait(&tctx.cond, &tctx.mutex);
        LOGI("InPutCnt=%d OutPutCnt=%d InHoldCnt=%d OutHoldCnt=%d", tctx.u32InPutCnt,
             tctx.u32OutPutCnt,tctx.u32InHoldCnt, tctx.u32OutHoldCnt);
    }

    LOGI("breaking out of wait loop");

    pthread_mutex_unlock(&tctx.mutex);

    //LOGD("freeing buffer pools");
    //free_buf_pool(tctx.buf_pool, VPP_TRUE);

    usleep(100000);
}

/************************************************************************
 * Test Functions
 ***********************************************************************/
TEST_SUITE_INIT(IpMcSuiteInit)
{
}

TEST_SUITE_TERM(IpMcSuiteTerm)
{
}

TEST_SETUP(IpMcTestInit)
{
    IpMcInit(1920,1080);
}
TEST_CLEANUP(IpMcTestTerm)
{
    IpMc_Term();
}

TEST(IpMc_InputBuffer_FrcMcAlgo_SD)
{
    FrcMcAlgoTest(1920/2, 1080/2);
}

TEST(IpMc_InputBuffer_FrcMcAlgo_HD)
{
    FrcMcAlgoTest(1920, 1080);
}

TEST(IpMc_SetCustomCtrlBasic)
{
    //Test t_StCustomFrcMcParams
    t_StVppIpFrcMcCb *pstFrcMcCb = (t_StVppIpFrcMcCb *)stCtxMc.mc_ctx;
    char *cLocal = (char *)&pstFrcMcCb->stLocalFrcMcParams;
    char *cGlobal = (char *)pstFrcMcCb->pstGlobalFrcMcParams;
    size_t ParamSize =  sizeof(t_StCustomFrcMcParams);
    struct hqv_control ctrl;
    uint32_t fGlobalLocal = 0;  //Test both global (fGlobalLocal==0) and local (fGlobalLocal==1)
    uint32_t u32ParamSubId = 0;
    uint32_t u32ParamId=0;
    uint32_t u32=0;

    //save global para
    //char cLocalBuf[sizeof(t_StCustomFrcMcParams)];
    char cGblBuf[sizeof(t_StCustomFrcMcParams)];
    //memcpy(cLocalBuf, cLocal, sizeof(t_StCustomFrcMcParams));
    memcpy(cGblBuf, cGlobal, sizeof(t_StCustomFrcMcParams));

    s32ParamTest(cLocal, cGlobal, ParamSize, (char *)"CustomFrcMcParams") ;
    //IpMc_Term();
    //IpMcInit(1920, 1080);

    //memcpy(cLocal,  cLocalBuf, sizeof(t_StCustomFrcMcParams));
    //memcpy(cGlobal, cGblBuf,   sizeof(t_StCustomFrcMcParams));

    //recover global para by setting saved global para
    // Set to manual and custom
    ctrl.mode = HQV_MODE_MANUAL;
    ctrl.ctrl_type = HQV_CONTROL_CUST;
    ctrl.custom.id = VPP_IP_FRC;
    ctrl.custom.len = ParamSize + FRCMC_CUSTOM_HEADER_SIZE;//header length is 4
    char *cPtr  = (char*)ctrl.custom.data;
    *cPtr++ = ParamSize + FRCMC_CUSTOM_HEADER_SIZE; //length low 8-bit
    *cPtr++ = 0;// length hi 8-bit
    uint32_t tmp = fGlobalLocal;
    tmp |= u32ParamId << FRCMC_GLOBAL_LOCAL_BIT_LENGTH;
    tmp |= u32ParamSubId << (FRCMC_PARAMID_BIT_LENGTH + FRCMC_GLOBAL_LOCAL_BIT_LENGTH);
    *cPtr++ = tmp & 0xff;       //ID low 8-bit
    *cPtr++ = (tmp>>8) & 0xff;  //ID hi 8-bit
    //LOGD("ThisID=0x%x",tmp);

    //memset(cPtr, 0x55 + fGlobalLocal, ParamSize);
    memcpy(cPtr, cGblBuf,   sizeof(t_StCustomFrcMcParams));
    u32 = u32VppIpFrcMc_SetCtrl(stCtxMc.mc_ctx, ctrl);
    DVP_ASSERT_TRUE(u32 == VPP_OK);
}

//To test if global or local parameters changed for one session, it will affect or not affect that of another running sessions
//Only pstGlobalFrcMcParams->stCustomNrParams used for the test.
//It also tests pstFrcMcCb->stFrcMcParams changed or not changed
TEST(IpMc_SetCustomCtrlTwoSessions)
{
    uint32_t u32;
    struct hqv_control ctrl;

    t_StVppIpFrcMcCb *pstFrcMcCb = (t_StVppIpFrcMcCb *)stCtxMc.mc_ctx;
    char *cGlobal = (char *)pstFrcMcCb->pstGlobalFrcMcParams;

    //save global para
    char cGblBuf[sizeof(t_StCustomFrcMcParams)];
    memcpy(cGblBuf, cGlobal, sizeof(t_StCustomFrcMcParams));

    //one session already started
    //t_StVppIpFrcMcCb *pstFrcMcCb1 = (t_StVppIpFrcMcCb *)stCtxFrcMc.pstFrcMcCb;
    t_StVppIpFrcMcCb *pstFrcMcCb1 = (t_StVppIpFrcMcCb *)stCtxMc.mc_ctx;

    //Start another session (session2)

    t_StVppIpFrcMcCb *pstFrcMcCb2 = vpVppIpFrcMc_Init(stCtxMc.pstCtx, 0, tctx.cb);

    DVP_ASSERT_PTR_NNULL(pstFrcMcCb2);

    u32 = u32VppIpFrcMc_SetParam(pstFrcMcCb2, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32, VPP_OK);

    u32 = u32VppIpFrcMc_SetParam(pstFrcMcCb2, VPP_PORT_OUTPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32, VPP_OK);

    if (pstFrcMcCb1 ==NULL)
    {
        LOGE("pstFrcMcCb1 is NULL");
        return;
    }
    else
    {
        LOGD("pstFrcMcCb1=%p",pstFrcMcCb1);
    }
    if (pstFrcMcCb2 ==NULL)
    {
        LOGE("pstFrcMcCb2 is NULL");
        return;
    }
    else
    {
        LOGD("pstFrcMcCb2=%p",pstFrcMcCb2);
    }

    uint32_t fGlobalLocal = 0;//0,global;1:local
    t_StCustomFrcMcParams stCustomFrcMcParamsOld;

    //set new parameter with value
    t_StCustomFrcMcParams stCustomFrcMcParamsNew = {
        .mc_quality = 1,
        .RepeatMode_repeatPeriod = 7,
        .TH_MOTION = 40,
        .TH_MOTION_LOW = 30,
        .TH_MVOUTLIER_COUNT = 20,
        .TH_MVOUTLIER_COUNT_LOW = 15,
        .TH_OCCLUSION = 20,
        .TH_OCCLUSION_LOW = 30,
        .TH_MOTION00 = 43,
        .TH_MOTION00_LOW = 53,
        .TH_MVOUTLIER_VARIANCE_COUNT = 20,
        .TH_MVOUTLIER_VARIANCE_COUNT_LOW = 15,
        .TH_SCENECUT = 55,
        .TH_VARIANCE = 2,
        .TH_SAD_FR_RATIO = 7
    };

   //save session1 old local parameters
    memcpy((void*) &stCustomFrcMcParamsOld, (void*) &pstFrcMcCb1->stLocalFrcMcParams, sizeof(t_StCustomFrcMcParams));

#if 0 //debug
    t_StCustomFrcMcParams *pstCustomFrcMcParams = &stCustomFrcMcParamsOld;
    LOGD("pstCustomFrcMcParams->mc_quality                      = %d", pstCustomFrcMcParams->mc_quality                     );
    LOGD("pstCustomFrcMcParams->RepeatMode_repeatPeriod         = %d", pstCustomFrcMcParams->RepeatMode_repeatPeriod        );
    LOGD("pstCustomFrcMcParams->TH_MOTION                       = %d", pstCustomFrcMcParams->TH_MOTION                      );
    LOGD("pstCustomFrcMcParams->TH_MOTION_LOW                   = %d", pstCustomFrcMcParams->TH_MOTION_LOW                  );
    LOGD("pstCustomFrcMcParams->TH_MVOUTLIER_COUNT              = %d", pstCustomFrcMcParams->TH_MVOUTLIER_COUNT             );
    LOGD("pstCustomFrcMcParams->TH_MVOUTLIER_COUNT_LOW          = %d", pstCustomFrcMcParams->TH_MVOUTLIER_COUNT_LOW         );
    LOGD("pstCustomFrcMcParams->TH_OCCLUSION                    = %d", pstCustomFrcMcParams->TH_OCCLUSION                   );
    LOGD("pstCustomFrcMcParams->TH_OCCLUSION_LOW                = %d", pstCustomFrcMcParams->TH_OCCLUSION_LOW               );
    LOGD("pstCustomFrcMcParams->TH_MOTION00                     = %d", pstCustomFrcMcParams->TH_MOTION00                    );
    LOGD("pstCustomFrcMcParams->TH_MOTION00_LOW                 = %d", pstCustomFrcMcParams->TH_MOTION00_LOW                );
    LOGD("pstCustomFrcMcParams->TH_MVOUTLIER_VARIANCE_COUNT     = %d", pstCustomFrcMcParams->TH_MVOUTLIER_VARIANCE_COUNT    );
    LOGD("pstCustomFrcMcParams->TH_MVOUTLIER_VARIANCE_COUNT_LOW = %d", pstCustomFrcMcParams->TH_MVOUTLIER_VARIANCE_COUNT_LOW);
    LOGD("pstCustomFrcMcParams->TH_SCENECUT                     = %d", pstCustomFrcMcParams->TH_SCENECUT                    );
    LOGD("pstCustomFrcMcParams->TH_VARIANCE                     = %d", pstCustomFrcMcParams->TH_VARIANCE                    );
    LOGD("pstCustomFrcMcParams->TH_SAD_FR_RATIO                 = %d", pstCustomFrcMcParams->TH_SAD_FR_RATIO                );
#endif //debug

    //Test both global (fGlobalLocal==0) and local (fGlobalLocal==1)
    int cnt;
    for (cnt = 0; cnt < 2; cnt++)
    {
        //first do local (fGlobalLocal = 1) then do global (fGlobalLocal = 0)
        if (cnt == 0)
            fGlobalLocal = 1;
        else
            fGlobalLocal = 0 ;
        LOGD("%s(): Test %s parameter setting.",__func__,fGlobalLocal==0 ? "global" : "local");
        t_StCustomFrcMcParams *pstCustomFrcMcParams;
        if (fGlobalLocal)
            pstCustomFrcMcParams = &pstFrcMcCb2->stLocalFrcMcParams;
        else
            pstCustomFrcMcParams = pstFrcMcCb2->pstGlobalFrcMcParams;

        // Set to manual and custom
        ctrl.mode = HQV_MODE_MANUAL;
        ctrl.ctrl_type = HQV_CONTROL_CUST;
        ctrl.custom.id = VPP_IP_FRC;
        ctrl.custom.len = sizeof(t_StCustomFrcMcParams) + FRCMC_CUSTOM_HEADER_SIZE;//header length is 4
        char *cPtr  = (char*)ctrl.custom.data;
        *cPtr++ = sizeof(t_StCustomFrcMcParams) + FRCMC_CUSTOM_HEADER_SIZE; //lenth low 8-bit
        *cPtr++ = 0;// length hi 8-bit
        uint32_t tmp = fGlobalLocal;
        //tmp |= FRCMC_CUSTOM_SET_NR_PARAM << FRCMC_GLOBAL_LOCAL_BIT_LENGTH;
        //uint32_t u32ParamSubId = 0 ;
        //tmp |= u32ParamSubId << (FRCMC_PARAMID_BIT_LENGTH + FRCMC_GLOBAL_LOCAL_BIT_LENGTH);
        *cPtr++ = tmp & 0xff;       //ID low 8-bit
        *cPtr++ = (tmp>>8) & 0xff;  //ID hi 8-bit

        //memset(cPtr,0x55+fGlobalLocal,sizeof(t_StCustomFrcMcParams)); //global parameters set to 0x55 and local set to 0x56
        memcpy((void*) cPtr, (void*) &stCustomFrcMcParamsNew, sizeof(t_StCustomFrcMcParams));
        u32 = u32VppIpFrcMc_SetCtrl(pstFrcMcCb2, ctrl);
        LOGD("u32VppIpFrcMc_SetCtrl custom,return = %d pstCustomFrcMcParams=%p",u32,(char*)pstCustomFrcMcParams);
        DVP_ASSERT_TRUE(u32 == VPP_OK);
        u32 = u32CheckMem((char*) cPtr, (char*) pstCustomFrcMcParams, sizeof(t_StCustomFrcMcParams));
        DVP_ASSERT_TRUE(u32 == VPP_OK);

        char *ptr = (char*) pstCustomFrcMcParams;
        LOGD("%s() After setting u32VppIpFrcMc_SetCtrl(),pstCustomFrcMcParams=%p  0x%x %x %x %x",
                __func__,ptr, ptr[0],ptr[1],ptr[2],ptr[3]);

        //No matter Global or local, session2 local should be changed to new param
        vpp_svc_frc_params_t *pstFrcMcParams = &pstFrcMcCb2->stFrcMcParams.stFrcParams;
        DVP_ASSERT_TRUE(pstFrcMcParams->mc_quality                      == stCustomFrcMcParamsNew.mc_quality                     );
        DVP_ASSERT_TRUE(pstFrcMcParams->RepeatMode_repeatPeriod         == stCustomFrcMcParamsNew.RepeatMode_repeatPeriod        );
        DVP_ASSERT_TRUE(pstFrcMcParams->TH_MOTION                       == stCustomFrcMcParamsNew.TH_MOTION                      );
        DVP_ASSERT_TRUE(pstFrcMcParams->TH_MOTION_LOW                   == stCustomFrcMcParamsNew.TH_MOTION_LOW                  );
        DVP_ASSERT_TRUE(pstFrcMcParams->TH_MVOUTLIER_COUNT              == stCustomFrcMcParamsNew.TH_MVOUTLIER_COUNT             );
        DVP_ASSERT_TRUE(pstFrcMcParams->TH_MVOUTLIER_COUNT_LOW          == stCustomFrcMcParamsNew.TH_MVOUTLIER_COUNT_LOW         );
        DVP_ASSERT_TRUE(pstFrcMcParams->TH_OCCLUSION                    == stCustomFrcMcParamsNew.TH_OCCLUSION                   );
        DVP_ASSERT_TRUE(pstFrcMcParams->TH_OCCLUSION_LOW                == stCustomFrcMcParamsNew.TH_OCCLUSION_LOW               );
        DVP_ASSERT_TRUE(pstFrcMcParams->TH_MOTION00                     == stCustomFrcMcParamsNew.TH_MOTION00                    );
        DVP_ASSERT_TRUE(pstFrcMcParams->TH_MOTION00_LOW                 == stCustomFrcMcParamsNew.TH_MOTION00_LOW                );
        DVP_ASSERT_TRUE(pstFrcMcParams->TH_MVOUTLIER_VARIANCE_COUNT     == stCustomFrcMcParamsNew.TH_MVOUTLIER_VARIANCE_COUNT    );
        DVP_ASSERT_TRUE(pstFrcMcParams->TH_MVOUTLIER_VARIANCE_COUNT_LOW == stCustomFrcMcParamsNew.TH_MVOUTLIER_VARIANCE_COUNT_LOW);
        DVP_ASSERT_TRUE(pstFrcMcParams->TH_SCENECUT                     == stCustomFrcMcParamsNew.TH_SCENECUT                    );
        DVP_ASSERT_TRUE(pstFrcMcParams->TH_VARIANCE                     == stCustomFrcMcParamsNew.TH_VARIANCE                    );
        DVP_ASSERT_TRUE(pstFrcMcParams->TH_SAD_FR_RATIO                 == stCustomFrcMcParamsNew.TH_SAD_FR_RATIO                );

        if (fGlobalLocal == 0)//Global
        {
            //check session2 global same as local
            char *ptr1 = (char*) &pstFrcMcCb2->stLocalFrcMcParams;
            LOGD("%s(),After global parameters setting, local stCustomFrcMcParams=%p  0x%x %x %x %x",
                    __func__,ptr1, ptr1[0],ptr1[1],ptr1[2],ptr1[3]);
            char *ptr = (char*) pstFrcMcCb2->pstGlobalFrcMcParams;
            LOGD("%s(),After global parameters setting, global stCustomFrcMcParams=%p  0x%x %x %x %x",
                    __func__,ptr, ptr[0],ptr[1],ptr[2],ptr[3]);
            u32 = u32CheckMem((char*)ptr1, ptr, sizeof(t_StCustomFrcMcParams));
            DVP_ASSERT_TRUE(u32 == VPP_OK); //session2 local parameters the same as global for session2

            //Check if the other thread has changed the parameters because one thread has done global setting
            usleep(100000);

            char *cPtr1 = (char*) &pstFrcMcCb1->stLocalFrcMcParams;
            char *cPtr2 = (char*) pstFrcMcCb1->pstGlobalFrcMcParams;
            LOGD("%s(), After global parameters setting, the other thread local stCustomFrcMcParams=%p 0x%x %x %x %x",
                    __func__,cPtr1, cPtr1[0],cPtr1[1],cPtr1[2],cPtr1[3]);
            LOGD("%s(), After global parameters setting, the other thread global stCustomFrcMcParams=%p 0x%x %x %x %x",
                    __func__,cPtr2, cPtr2[0],cPtr2[1],cPtr2[2],cPtr2[3]);
            u32 = u32CheckMem((char*)cPtr1, ptr, sizeof(t_StCustomFrcMcParams));
            DVP_ASSERT_TRUE(u32 == VPP_OK); //session1 local parameters the same as global ones of session2
            u32 = u32CheckMem((char*)cPtr2, ptr, sizeof(t_StCustomFrcMcParams));
            DVP_ASSERT_TRUE(u32 == VPP_OK); //session1 global parameters the same as global ones of session2

            //If session2 global param changed, session1 local should be changed to new param
            pstFrcMcParams = &pstFrcMcCb1->stFrcMcParams.stFrcParams;
            DVP_ASSERT_TRUE(pstFrcMcParams->mc_quality                      == stCustomFrcMcParamsNew.mc_quality                     );
            DVP_ASSERT_TRUE(pstFrcMcParams->RepeatMode_repeatPeriod         == stCustomFrcMcParamsNew.RepeatMode_repeatPeriod        );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_MOTION                       == stCustomFrcMcParamsNew.TH_MOTION                      );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_MOTION_LOW                   == stCustomFrcMcParamsNew.TH_MOTION_LOW                  );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_MVOUTLIER_COUNT              == stCustomFrcMcParamsNew.TH_MVOUTLIER_COUNT             );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_MVOUTLIER_COUNT_LOW          == stCustomFrcMcParamsNew.TH_MVOUTLIER_COUNT_LOW         );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_OCCLUSION                    == stCustomFrcMcParamsNew.TH_OCCLUSION                   );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_OCCLUSION_LOW                == stCustomFrcMcParamsNew.TH_OCCLUSION_LOW               );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_MOTION00                     == stCustomFrcMcParamsNew.TH_MOTION00                    );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_MOTION00_LOW                 == stCustomFrcMcParamsNew.TH_MOTION00_LOW                );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_MVOUTLIER_VARIANCE_COUNT     == stCustomFrcMcParamsNew.TH_MVOUTLIER_VARIANCE_COUNT    );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_MVOUTLIER_VARIANCE_COUNT_LOW == stCustomFrcMcParamsNew.TH_MVOUTLIER_VARIANCE_COUNT_LOW);
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_SCENECUT                     == stCustomFrcMcParamsNew.TH_SCENECUT                    );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_VARIANCE                     == stCustomFrcMcParamsNew.TH_VARIANCE                    );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_SAD_FR_RATIO                 == stCustomFrcMcParamsNew.TH_SAD_FR_RATIO                );
        }

        if (fGlobalLocal == 1)//local
        {
            char *ptr1 = (char*) &pstFrcMcCb2->stLocalFrcMcParams;
            LOGD("%s(),After local parameters setting, local stCustomFrcMcParams=%p  0x%x %x %x %x",
                    __func__,ptr1, ptr1[0],ptr1[1],ptr1[2],ptr1[3]);
            char *ptr = (char*) pstFrcMcCb2->pstGlobalFrcMcParams;
            LOGD("%s(),After local parameters setting, global stCustomFrcMcParams=%p  0x%x %x %x %x",
                    __func__,ptr, ptr[0],ptr[1],ptr[2],ptr[3]);
            cPtr = (char*) &pstFrcMcCb2->stLocalFrcMcParams;
            u32 = u32CheckMem((char*)cPtr, ptr, sizeof(t_StCustomFrcMcParams));
            DVP_ASSERT_TRUE(u32 != VPP_OK); //session2 global parameters not the same as local, if we set local

            //Check if one session session2 changed local parameters the other session session1 will not change
            usleep(10000);

            char *cPtr1 = (char*) &pstFrcMcCb1->stLocalFrcMcParams;
            char *cPtr2 = (char*) pstFrcMcCb1->pstGlobalFrcMcParams;
            LOGD("%s(),After local parameters setting, the other thread local stCustomFrcMcParams=%p 0x%x %x %x %x",
                    __func__,cPtr1, cPtr1[0],cPtr1[1],cPtr1[2],cPtr1[3]);
            LOGD("%s(),After local parameters setting, the other thread global stCustomFrcMcParams=%p 0x%x %x %x %x",
                    __func__,cPtr2, cPtr2[0],cPtr2[1],cPtr2[2],cPtr2[3]);
            u32 = u32CheckMem((char*)cPtr1, ptr, sizeof(t_StCustomFrcMcParams));
            DVP_ASSERT_TRUE(u32 == VPP_OK); //session1 local parameters the same as global ones of session2, so not the same as locals of session2
            u32 = u32CheckMem((char*)cPtr2, ptr, sizeof(t_StCustomFrcMcParams));
            DVP_ASSERT_TRUE(u32 == VPP_OK); //session1 global parameters the same as global ones of session2, so not the same as locals of session2

            //If session2 local param changed, session1 local param should not change
            pstFrcMcParams = &pstFrcMcCb1->stFrcMcParams.stFrcParams;
            DVP_ASSERT_TRUE(pstFrcMcParams->mc_quality                      == stCustomFrcMcParamsOld.mc_quality                     );
            DVP_ASSERT_TRUE(pstFrcMcParams->RepeatMode_repeatPeriod         == stCustomFrcMcParamsOld.RepeatMode_repeatPeriod        );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_MOTION                       == stCustomFrcMcParamsOld.TH_MOTION                      );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_MOTION_LOW                   == stCustomFrcMcParamsOld.TH_MOTION_LOW                  );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_MVOUTLIER_COUNT              == stCustomFrcMcParamsOld.TH_MVOUTLIER_COUNT             );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_MVOUTLIER_COUNT_LOW          == stCustomFrcMcParamsOld.TH_MVOUTLIER_COUNT_LOW         );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_OCCLUSION                    == stCustomFrcMcParamsOld.TH_OCCLUSION                   );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_OCCLUSION_LOW                == stCustomFrcMcParamsOld.TH_OCCLUSION_LOW               );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_MOTION00                     == stCustomFrcMcParamsOld.TH_MOTION00                    );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_MOTION00_LOW                 == stCustomFrcMcParamsOld.TH_MOTION00_LOW                );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_MVOUTLIER_VARIANCE_COUNT     == stCustomFrcMcParamsOld.TH_MVOUTLIER_VARIANCE_COUNT    );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_MVOUTLIER_VARIANCE_COUNT_LOW == stCustomFrcMcParamsOld.TH_MVOUTLIER_VARIANCE_COUNT_LOW);
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_SCENECUT                     == stCustomFrcMcParamsOld.TH_SCENECUT                    );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_VARIANCE                     == stCustomFrcMcParamsOld.TH_VARIANCE                    );
            DVP_ASSERT_TRUE(pstFrcMcParams->TH_SAD_FR_RATIO                 == stCustomFrcMcParamsOld.TH_SAD_FR_RATIO                );
        }
    }

    usleep(10000);

    vVppIpFrcMc_Term(pstFrcMcCb2);
    //IpMc_Term();
    //IpMcInit(1920, 1080);

    //recover global para by setting saved global para
    size_t ParamSize =  sizeof(t_StCustomFrcMcParams);
    fGlobalLocal = 0;  //Test both global (fGlobalLocal==0) and local (fGlobalLocal==1)
    uint32_t u32ParamSubId = 0;
    uint32_t u32ParamId=0;
    u32=0;
    // Set to manual and custom
    ctrl.mode = HQV_MODE_MANUAL;
    ctrl.ctrl_type = HQV_CONTROL_CUST;
    ctrl.custom.id = VPP_IP_FRC;
    ctrl.custom.len = ParamSize + FRCMC_CUSTOM_HEADER_SIZE;//header length is 4
    char *cPtr  = (char*)ctrl.custom.data;
    *cPtr++ = ParamSize + FRCMC_CUSTOM_HEADER_SIZE; //length low 8-bit
    *cPtr++ = 0;// length hi 8-bit
    uint32_t tmp = fGlobalLocal;
    tmp |= u32ParamId << FRCMC_GLOBAL_LOCAL_BIT_LENGTH;
    tmp |= u32ParamSubId << (FRCMC_PARAMID_BIT_LENGTH + FRCMC_GLOBAL_LOCAL_BIT_LENGTH);
    *cPtr++ = tmp & 0xff;       //ID low 8-bit
    *cPtr++ = (tmp>>8) & 0xff;  //ID hi 8-bit
    //LOGD("ThisID=0x%x",tmp);

    //memset(cPtr, 0x55 + fGlobalLocal, ParamSize);
    memcpy(cPtr, cGblBuf,   sizeof(t_StCustomFrcMcParams));
    u32 = u32VppIpFrcMc_SetCtrl(stCtxMc.mc_ctx, ctrl);
    DVP_ASSERT_TRUE(u32 == VPP_OK);
}

/*!
 *  Make sure that if we queue an equal number of input and output buffers, we
 *  get a drain done callback once all of the buffers have been returned.
 */
TEST(IpMc_Drain)
{
    uint32_t u32Ret;
    uint32_t i;
    struct bufnode *pNode;

    u32TestTimeStamp = 0;//do not test time stamp in this test
    uint32_t u32BufTotal;

    u32BufTotal = u32InBufCnt + u32OutBufCnt;

    u32Ret = u32QueueInput(0);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMc_Drain(stCtxMc.mc_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    for (i = 0; i < u32OutBufCnt; i++)
    {
        //3. Output pixel buf
        pNode = get_buf(tctx.buf_pool);
        DVP_ASSERT_PTR_NNULL(pNode);

        if(pNode)
        {
            pNode->owner = BUF_OWNER_LIBRARY;
            pNode->port_owner = VPP_PORT_OUTPUT;
            pNode->pIntBuf->eBufPxType = eVppBufPxDataType_Raw;
            u32Ret = u32VppIpFrcMc_QueueBuf(stCtxMc.mc_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            pthread_mutex_lock(&tctx.mutex);
            tctx.u32OutPutCnt++;
            tctx.u32OutHoldCnt++;
            pthread_mutex_unlock(&tctx.mutex);
        }
        else
        {
            DVP_ASSERT_FAIL();
            LOGE("%s(): pNode == NULL",__func__);
            return;
        }
    }
    pthread_mutex_lock(&tctx.mutex);

    while (!(tctx.u32Flags & DRAIN_DONE))
        pthread_cond_wait(&tctx.cond, &tctx.mutex);
    pthread_mutex_unlock(&tctx.mutex);

    DVP_ASSERT_TRUE(tctx.u32Flags & DRAIN_DONE);
    //printf("tctx.buf_pool->u32ListSz=%d, u32BufTotal=%d\n", tctx.buf_pool->u32ListSz, u32BufTotal);
    //DVP_ASSERT_EQUAL(tctx.buf_pool->u32ListSz, u32BufTotal);

    //LOGI("freeing buffer pools");
    //free_buf_pool(tctx.buf_pool, VPP_TRUE);
}

/*!
 * Basic drain test case. Make sure that if we have never enqueued any buffers,
 * if we request a drain, we get back a drain done right away.
 */
TEST(IpMc_DrainNoBuffers)
{
    uint32_t u32Ret;

    u32Ret = u32VppIpFrcMc_Drain(stCtxMc.mc_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    while (!(tctx.u32Flags & DRAIN_DONE))
        pthread_cond_wait(&tctx.cond, &tctx.mutex);
    pthread_mutex_unlock(&tctx.mutex);

    DVP_ASSERT_TRUE(tctx.u32Flags & DRAIN_DONE);
}

/*!
 * Test bypass of non-progressive buffer
 */
TEST(IpMc_BypassNonProgressiveBuffer)
{
    uint32_t u32Ret = u32BypassTest(FLAG_BYPASS_NONPROGRESIVE_BUFFER);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
}
/*!
 * Test bypass of buffer with bypass flag
 */
TEST(IpMc_BypassBypassBuffer)
{
    uint32_t u32Ret = u32BypassTest(FLAG_BYPASS_BUFFER);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
}
/*!
 * Test bypass of buffer without set and thus frc mc disabled
 */
TEST(IpMc_BypassMcDisable)
{
    uint32_t u32Ret = u32BypassTest(FLAG_BYPASS_ALGO_MC_DISABLE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
}
/*!
 * Test FlagDataCorrupt
 */
TEST(IpMc_FlagDataCorrupt)
{
    u32FlagCorrupt = 0;
    uint32_t u32Ret = u32BypassTest(FLAG_BUFFER_FLAG_DATACORRUPT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
}
/*!
 * Test FlagDataCorrupt
 */
TEST(IpMc_FlagEos)
{
    u32FlagCorrupt = 0;
    uint32_t u32Ret = u32BypassTest(FLAG_BUFFER_FLAG_EOS);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
}
TEST(IpMc_Reconfig)
{
    u32FlagCorrupt = 0;

    LOGD("\n--------------\n1st mc_algo test\n---------------\n");
    uint32_t u32Ret = u32BypassTest(0);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    uint32_t i;
    struct bufnode *pNode;
    //struct vpp_port_param stParam = {1920, 1080, VPP_COLOR_FORMAT_NV12_VENUS};
    uint32_t u32Width = 1920;
    uint32_t u32Height = 1080;

    u32TestTimeStamp = 0;//do not test time stamp in this test
    uint32_t u32BufTotal;

    tctx.u32Flags &= ~DRAIN_DONE;

    LOGD("\n--------------\n2nd mc_algo test\n---------------\n");
    u32BufTotal = u32InBufCnt + u32OutBufCnt;

    u32VppIpFrcMc_Flush(stCtxMc.mc_ctx, VPP_PORT_INPUT);
    u32VppIpFrcMc_Flush(stCtxMc.mc_ctx, VPP_PORT_OUTPUT);

    u32Ret = u32VppIpFrcMc_Close(stCtxMc.mc_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    usleep(200000);

    //LOGD("\nIpMc_Reconfig freeing buffer pools");
    free_buf_pool(tctx.buf_pool, VPP_TRUE);
    usleep(100000);

    if (( tctx.params.u32Width != 1920/2 || tctx.params.u32Height != 1080/2))
    {
       //stParam = {1920/2, 1080/2, VPP_COLOR_FORMAT_NV12_VENUS};
       u32Width = 1920/2;
       u32Height = 1080/2;
    }

    tctx_set_port_params(&tctx, u32Width, u32Height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width = u32Width;
    tctx.params.u32Height = u32Height;
    tctx.params.eBufferType=eVppBufType_Progressive;
    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;

    u32Ret = u32VppIpFrcMc_SetParam(stCtxMc.mc_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    u32Ret = u32VppIpFrcMc_SetParam(stCtxMc.mc_ctx, VPP_PORT_OUTPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMc_Open(stCtxMc.mc_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

/*
    IpMc_Term();
    usleep(100000);
    if (( tctx.params.u32Width != 1920/2 || tctx.params.u32Height != 1080/2))
    {
        IpMcInit(1920/2, 1080/2);
    }
    else
        IpMcInit(1920, 1080);
*/
    u32Ret = u32QueueInput(0);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMc_Drain(stCtxMc.mc_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    for (i = 0; i < u32OutBufCnt; i++)
    {
        //3. Output pixel buf
        pNode = get_buf(tctx.buf_pool);
        DVP_ASSERT_PTR_NNULL(pNode);

        if(pNode)
        {
            pNode->owner = BUF_OWNER_LIBRARY;
            pNode->port_owner = VPP_PORT_OUTPUT;
            pNode->pIntBuf->eBufPxType = eVppBufPxDataType_Raw;
            u32Ret = u32VppIpFrcMc_QueueBuf(stCtxMc.mc_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            pthread_mutex_lock(&tctx.mutex);
            tctx.u32OutPutCnt++;
            tctx.u32OutHoldCnt++;
            pthread_mutex_unlock(&tctx.mutex);
        }
        else
        {
            DVP_ASSERT_FAIL();
            LOGE("%s(): pNode == NULL",__func__);
            return;
        }
    }

    pthread_mutex_lock(&tctx.mutex);

    while (!(tctx.u32Flags & DRAIN_DONE))
        pthread_cond_wait(&tctx.cond, &tctx.mutex);
    //printf("Before while: tctx.buf_pool->u32ListSz=%d u32BufTotal=%d\n", tctx.buf_pool->u32ListSz, u32BufTotal);
    //while (tctx.buf_pool->u32ListSz < u32BufTotal - 2)

    //{
    //    pthread_cond_wait(&tctx.cond, &tctx.mutex);
    //    printf("tctx.buf_pool->u32ListSz=%d u32BufTotal=%d\n", tctx.buf_pool->u32ListSz, u32BufTotal);
    //}
    pthread_mutex_unlock(&tctx.mutex);

    //LOGI("freeing buffer pools");
    //free_buf_pool(tctx.buf_pool, VPP_TRUE);
}
/************************************************************************
 * Global Functions
 ***********************************************************************/
TEST_CASES IpMcTests[] = {
    TEST_CASE(IpMc_SetCustomCtrlBasic),
    TEST_CASE(IpMc_SetCustomCtrlTwoSessions),
    TEST_CASE(IpMc_InputBuffer_FrcMcAlgo_HD),
    TEST_CASE(IpMc_InputBuffer_FrcMcAlgo_SD),
    TEST_CASE(IpMc_DrainNoBuffers),
    TEST_CASE(IpMc_Drain),
    TEST_CASE(IpMc_BypassBypassBuffer),
    TEST_CASE(IpMc_BypassNonProgressiveBuffer),
    TEST_CASE(IpMc_BypassMcDisable),
    TEST_CASE(IpMc_FlagDataCorrupt),
    TEST_CASE(IpMc_FlagEos),
    TEST_CASE(IpMc_Reconfig),
    TEST_CASE_NULL(),
};

TEST_SUITE(IpMcSuite,
           "IpMc Tests",
           IpMcSuiteInit,
           IpMcSuiteTerm,
           IpMcTestInit,
           IpMcTestTerm,
           IpMcTests);

