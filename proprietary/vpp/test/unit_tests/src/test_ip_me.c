/*!
 * @file test_ip_me.c
 *
 * @cr
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @services    Implements tests for the me (Motion Estimation) submodule
 */

#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <time.h>


#include "buf_pool.h"
#include "test_utils.h"

#include "dvpTest.h"
#include "dvpTest_tb.h"

#define VPP_LOG_TAG     VPP_LOG_UT_ME_TAG
#define VPP_LOG_MODULE  VPP_LOG_UT_ME
#include "vpp_dbg.h"
#include "vpp.h"
// #include "vpp_core.h"
// #include "vpp_ctx.h"
#include "vpp_reg.h"
#include "vpp_ip.h"
#include "vpp_ip_frc_me.h"


// #include "vpp_uc.h"

/************************************************************************
 * Local definitions
 ***********************************************************************/
#define FRC_ME_CHECK_FILE       0
#define INPUT_FLUSH_DONE        (1<<0)
#define OUTPUT_FLUSH_DONE       (1<<1)
#define DRAIN_DONE              (1<<2)
#define OUTPUT_EOS              (1<<3)
#define TIME_DELAY              (33*1000)

struct me_test_ctx {
    void *pme_ctx;          // context returned from me init
    uint32_t u32ExpExtraLen;
    t_StVppCtx stCtx;       //This is necessary because inside ME we allocate EOS Drain input buffer
};

/************************************************************************
 * Local static variables
 ***********************************************************************/
static struct test_ctx tctx;
static struct me_test_ctx stCtxMe;
/************************************************************************
 * Forward Declarations
 ************************************************************************/

/************************************************************************
 * Local Functions
 ***********************************************************************/
void test_me_input_buffer_done(void *pv, t_StVppBuf *pBuf)
{
    LOGI("%s() with buffer=0x%x", __func__, (unsigned int)pBuf);

    struct test_ctx *pCtx = (struct test_ctx *)pv;
    struct bufnode *pNode = pBuf->pBuf->cookie;

    pthread_mutex_lock(&pCtx->mutex);  //Mutex to protect the pools
    DVP_ASSERT_TRUE(pNode->owner == BUF_OWNER_LIBRARY);
    pNode->owner = BUF_OWNER_CLIENT;

    put_buf(pCtx->buf_pool, pNode);
    tctx.u32InRxCnt++;
    pthread_mutex_unlock(&pCtx->mutex);
    pthread_cond_signal(&pCtx->cond);
}

void test_me_output_buffer_done(void *pv, t_StVppBuf *pBuf)
{
    LOGI("%s() with buffer=0x%x", __func__, (unsigned int)pBuf);

    struct test_ctx *pCtx = (struct test_ctx *)pv;
    struct bufnode *pNode = pBuf->pBuf->cookie;
    t_EVppBufType eType;

    pthread_mutex_lock(&pCtx->mutex);  //Mutex to protect the pools
    DVP_ASSERT_TRUE(pNode->owner == BUF_OWNER_LIBRARY);
    pNode->owner = BUF_OWNER_CLIENT;

    eType = eVppBuf_GetFrameType(pBuf);
    DVP_ASSERT_EQUAL(eType, eVppBufType_Progressive);

    if (pNode->pIntBuf->pBuf->flags & VPP_BUFFER_FLAG_EOS)
    {
       pCtx->u32Flags |= OUTPUT_EOS;
    }

    if (pNode->pIntBuf->stPixel.u32FilledLen)
    {
        dump_buf(pNode);
//AA?        DVP_ASSERT_EQUAL(pNode->pIntBuf->stExtra.u32FilledLen, pMeCtx->u32ExpExtraLen);
//        LOGI("extradata: expFillLen=%u, act_fill_len=%u", pMeCtx->u32ExpExtraLen,
//             pNode->pIntBuf->stExtra.u32FilledLen);
//        validate_extradata_integrity(pNode);
    }

    put_buf(pCtx->buf_pool_ext, pNode);
    tctx.u32OutRxCnt++;
    pthread_mutex_unlock(&pCtx->mutex);
    pthread_cond_signal(&pCtx->cond);
}

void test_me_event(void *pv, t_StVppEvt stEvt)
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

/************************************************************************
 * Utility Functions
 ***********************************************************************/
#if FRC_ME_CHECK_FILE
static int exists(const char *fname)
{
    FILE *file = fopen(fname, "r");
    if (file !=NULL)
    {
        fclose(file);
        return 1;
    }
    return 0;
}
#endif

/************************************************************************
 * Test Functions
 ***********************************************************************/


TEST_SUITE_INIT(IpMeSuiteInit)
{
}

TEST_SUITE_TERM(IpMeSuiteTerm)
{
}

TEST_SETUP(IpMeTestInit)
{
    uint32_t u32Ret=VPP_OK;
    memset((void*)&tctx,0,sizeof(tctx));
    memset((void*)&stCtxMe,0,sizeof(stCtxMe));
    //This is necessary because inside ME we allocate EOS Drain input buffer
    u32Ret = u32VppIon_Init(&stCtxMe.stCtx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    if (u32Ret)
        LOGE("Error: u32VppIon_Init failed! ret=%d",u32Ret);
    u32Ret=tctx_common_init((struct test_ctx*)&tctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
}

TEST_CLEANUP(IpMeTestTerm)
{
    uint32_t u32Ret=VPP_OK;
    u32Ret=tctx_common_destroy(&tctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    //This is necessary because inside ME we allocate EOS Drain input buffer
    u32Ret = u32VppIon_Term(&stCtxMe.stCtx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    if (u32Ret)
        LOGE("Error: u32VppIon_Term failed! ret=%d",u32Ret);
}

TEST(__unused IpMe_EncodeBuffer_Null)
{
    uint32_t u32Ret=VPP_OK;
    uint32_t i;

    LOGI("%s() IpMe_EncodeBuffer_Null", __func__);

    for (i = 0; i < 10; i++)
    {
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, NULL);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, NULL);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    sleep(1);
}

//#define TEST_MBI_ALL_NEW_BUFFERS
TEST(__unused IpMe_EncodeBuffer)
{
    uint32_t u32Ret=VPP_OK;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32InFrames, u32InFrames_done;
    t_StVppIpBufReq stInputBufReq;
    t_StVppIpBufReq stOutputBufReq;

    struct vpp_port_param port_param_output;
    char s[MAX_FILE_LEN];

    LOGI("%s() IpMe_EncodeBuffer", __func__);

    tctx.cb.input_buffer_done = test_me_input_buffer_done;
    tctx.cb.output_buffer_done = test_me_output_buffer_done;
    tctx.cb.event = test_me_event;
    tctx.cb.pv = &tctx;

    tctx.pPrivateCtx = &stCtxMe;
    tctx.u32FramesOut = 10;

    uint32_t frame_width=1920;  //720;   //1920;
    uint32_t frame_height=1080;  //480;  //1080;

    tctx_set_port_params(&tctx, frame_width, frame_height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width=frame_width;
    tctx.params.u32Height=frame_height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cInputPath,(char*)"/data/test/input/", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_480p.yuv",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"testmotion_nv12_1080p.yuv",MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cInputName,(char*)"yuvgtmotion_nv12_1080p.yuv", MAX_FILE_SEG_LEN);
    strlcpy((char*)s,(char*)tctx.params.cInputPath, MAX_FILE_SEG_LEN);
    strlcat((char*)s,(char*)tctx.params.cInputName, MAX_FILE_SEG_LEN);
#if FRC_ME_CHECK_FILE
    if (!exists((char*)s))
    {
        DVP_ASSERT_PTR_NNULL(NULL);
        LOGE("Error: %s file does't exist!", (char*)tctx.params.cInputName);
        return;
    }
#endif

    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cOutputPath,(char*)"/data/test/output/",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_480p.bin", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"testmotion_nv12_1080p.bin", MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cOutputName,(char*)"yuvgtmotion_nv12_1080p.bin", MAX_FILE_SEG_LEN);

    port_param_output.width=frame_width;
    port_param_output.height=frame_height;
    port_param_output.fmt=VPP_COLOR_FORMAT_NV12_VENUS;

    stCtxMe.pme_ctx = vpVppIpFrcMe_Init(&stCtxMe.stCtx, 0, tctx.cb);
    DVP_ASSERT_PTR_NNULL(stCtxMe.pme_ctx);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, port_param_output);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32VppIpFrcMe_GetBufferRequirements(stCtxMe.pme_ctx, &stInputBufReq, &stOutputBufReq);
    u32InFrames =tctx.u32FramesOut;
    u32InFrames_done =0;

    LOGI("%s() init_buf_pool(Input)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params, u32InFrames, VPP_TRUE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

#ifdef TEST_MBI_ALL_NEW_BUFFERS
    struct bufnode * InputNodes[tctx.u32FramesOut];
    for (i=0; i<u32InFrames; i++)
        InputNodes[i]=get_buf(tctx.buf_pool);
#endif

    LOGI("%s() init_buf_pool(Output)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params, stOutputBufReq.u32MinCnt, VPP_FALSE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    // Queue to input pool
    LOGI("%s() Push buffers loop", __func__);
    LOGI("%s Init Stage", __func__);
    //Init Stage ----------------------------------
    //Feed Internal FRC Output Buffers
    for (i = 0; i < stOutputBufReq.u32MinCnt; i++)
    {
        pthread_mutex_lock(&tctx.mutex);
        pNode=NULL;
        while (pNode==NULL)
            pNode = get_buf(tctx.buf_pool_ext);
        pthread_mutex_unlock(&tctx.mutex);
        DVP_ASSERT_PTR_NNULL(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;

        //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

        vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc); //For debugging purposes
        pNode->pIntBuf->pBuf->timestamp=0;
        pNode->pIntBuf->pBuf->flags=0;
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    //Feed Stage --------------------------------
    //Feed input and output buffers as they become available.
    LOGI("%s Feed Stage", __func__);
    while ((u32InFrames_done<u32InFrames)||(tctx.u32InRxCnt < u32InFrames)||(tctx.u32OutRxCnt < u32InFrames))
    {
        int buffer_found=0;

        if ((tctx.buf_pool->u32ListSz)&&(u32InFrames_done<u32InFrames))
        {
            pthread_mutex_lock(&tctx.mutex);
#ifdef TEST_MBI_ALL_NEW_BUFFERS
            pNode=InputNodes[u32InFrames_done];
#else
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool);
#endif
            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            fill_buf(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            fill_extra_data(pNode, eVppBufType_Progressive, 0);
            stCtxMe.u32ExpExtraLen = pNode->pIntBuf->stExtra.u32FilledLen;

            pNode->pIntBuf->pBuf->timestamp=u32InFrames_done*TIME_DELAY;  //Inc with 33ms every input buffer
            pNode->pIntBuf->pBuf->flags=0;
            if (u32InFrames_done==(u32InFrames-1))
                pNode->pIntBuf->pBuf->flags|=VPP_BUFFER_FLAG_EOS;
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            u32InFrames_done++;
            buffer_found=1;
        }
        if ((tctx.buf_pool_ext->u32ListSz)&&(tctx.u32OutRxCnt < u32InFrames))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool_ext);

            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

            pNode->pIntBuf->pBuf->flags=0;
            vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc);  //For debugging purposes
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            buffer_found=1;
        }
        if (!buffer_found)
           sched_yield();
    }

    u32Ret=u32VppIpFrcMe_Close(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    vVppIpFrcMe_Term(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    LOGI("freeing buffer pools");
    free_buf_pool(tctx.buf_pool_ext, VPP_FALSE);
    free_buf_pool(tctx.buf_pool, VPP_TRUE);
}

//10 inputs -> 9 MBIs
TEST(IpMe_EncodeBuffer_MBI_1080p_Downscale)
{
    uint32_t u32Ret=VPP_OK;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32InFrames, u32InFrames_done;
    t_StVppIpBufReq stInputBufReq;
    t_StVppIpBufReq stOutputBufReq;
    struct vpp_port_param port_param_output;
    char s[MAX_FILE_LEN];

    LOGI("%s() IpMe_EncodeBuffer_MBI", __func__);

    tctx.cb.input_buffer_done = test_me_input_buffer_done;
    tctx.cb.output_buffer_done = test_me_output_buffer_done;
    tctx.cb.event = test_me_event;
    tctx.cb.pv = &tctx;

    tctx.pPrivateCtx = &stCtxMe;
    tctx.u32FramesOut = 10;

    uint32_t frame_width=1920;
    uint32_t frame_height=1080;

    tctx_set_port_params(&tctx, frame_width, frame_height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width=frame_width;
    tctx.params.u32Height=frame_height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cInputPath,(char*)"/data/test/input/", MAX_FILE_SEG_LEN);
    //strcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_1080p.yuv");
    //strcpy((char*)tctx.params.cInputName,(char*)"testmotion_nv12_1080p.yuv");
    strlcpy((char*)tctx.params.cInputName,(char*)"yuvgtmotion_nv12_1080p.yuv", MAX_FILE_SEG_LEN);
    strlcpy((char*)s,(char*)tctx.params.cInputPath, MAX_FILE_SEG_LEN);
    strlcat((char*)s,(char*)tctx.params.cInputName, MAX_FILE_SEG_LEN);
#if FRC_ME_CHECK_FILE
    if (!exists((char*)s))
    {
        DVP_ASSERT_PTR_NNULL(NULL);
        LOGE("Error: %s file does't exist!", (char*)tctx.params.cInputName);
        return;
    }
#endif

    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cOutputPath,(char*)"/data/test/output/", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_1080p.bin, MAX_FILE_SEG_LEN");
    //strlcpy((char*)tctx.params.cOutputName,(char*)"testmotion_nv12_1080p_downscale.bin, MAX_FILE_SEG_LEN");
    strlcpy((char*)tctx.params.cOutputName,(char*)"yuvgtmotion_nv12_1080p_downscale.bin", MAX_FILE_SEG_LEN);

    port_param_output.width=frame_width;
    port_param_output.height=frame_height;
    port_param_output.fmt=VPP_COLOR_FORMAT_NV12_VENUS;

    stCtxMe.pme_ctx = vpVppIpFrcMe_Init(&stCtxMe.stCtx, 0, tctx.cb);
    DVP_ASSERT_PTR_NNULL(stCtxMe.pme_ctx);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, port_param_output);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32VppIpFrcMe_GetBufferRequirements(stCtxMe.pme_ctx, &stInputBufReq, &stOutputBufReq);
    u32InFrames =tctx.u32FramesOut;

    LOGI("%s() init_buf_pool(Input)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params, u32InFrames, VPP_TRUE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

#ifdef TEST_MBI_ALL_NEW_BUFFERS
    struct bufnode * InputNodes[tctx.u32FramesOut];
    for (i=0; i<u32InFrames; i++)
        InputNodes[i]=get_buf(tctx.buf_pool);
#endif

    LOGI("%s() init_buf_pool(Output)", __func__);
    tctx.params.u32Width /= 2;
    tctx.params.u32Height/= 2;
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params, stOutputBufReq.u32MinCnt, VPP_FALSE);
    tctx.params.u32Width *= 2;
    tctx.params.u32Height*= 2;
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    // Queue to input pool
    LOGI("%s() Push buffers loop", __func__);
    LOGI("%s Init Stage. %d input frames, %d output buffers", __func__, u32InFrames, stOutputBufReq.u32MinCnt);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);

    u32InFrames_done =0;
    tctx.u32Flags = 0;
    tctx.buf_pool->u32RdIdx=0;
    tctx.buf_pool->u32WrIdx=0;
    tctx.buf_pool_ext->u32RdIdx=0;
    tctx.buf_pool_ext->u32WrIdx=0;
    tctx.u32InRxCnt = 0;
    tctx.u32OutRxCnt = 0;
    tctx.u32InPutCnt = 0;
    tctx.u32OutPutCnt = 0;
    tctx.u32InHoldCnt = 0;
    tctx.u32OutHoldCnt = 0;
    //Init Stage ----------------------------------
    //Feed Internal FRC Output Buffers
    for (i = 0; i < stOutputBufReq.u32MinCnt; i++)
    {
        pthread_mutex_lock(&tctx.mutex);
        pNode=NULL;
        while (pNode==NULL)
            pNode = get_buf(tctx.buf_pool_ext);
        pthread_mutex_unlock(&tctx.mutex);
        DVP_ASSERT_PTR_NNULL(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;

        //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

        vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc); //For debugging purposes
        pNode->pIntBuf->pBuf->timestamp=0;
        pNode->pIntBuf->pBuf->flags=0;
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    //Feed Stage --------------------------------
    //Feed input and output buffers as they become available.
    LOGI("%s Feed Stage", __func__);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    while ((u32InFrames_done<u32InFrames)||(tctx.u32InRxCnt < u32InFrames)||(tctx.u32OutRxCnt < (u32InFrames)))
    {
        int buffer_found=0;

#ifdef TEST_MBI_ALL_NEW_BUFFERS
//Uncomment this for TEST_MBI_ALL_NEW_BUFFERS
//        if ((u32InFrames_done<u32InFrames))
//        {
//            pthread_mutex_lock(&tctx.mutex);
//            pNode=InputNodes[u32InFrames_done];
#else
        if ((tctx.buf_pool->u32ListSz)&&(u32InFrames_done<u32InFrames))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool);
#endif
            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            fill_buf(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            fill_extra_data(pNode, eVppBufType_Progressive, 0);
            stCtxMe.u32ExpExtraLen = pNode->pIntBuf->stExtra.u32FilledLen;

            pNode->pIntBuf->pBuf->timestamp=u32InFrames_done*TIME_DELAY;  //Inc with 33ms every input buffer
            pNode->pIntBuf->pBuf->flags=0;
            if (u32InFrames_done==(u32InFrames-1))
                pNode->pIntBuf->pBuf->flags|=VPP_BUFFER_FLAG_EOS;
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            u32InFrames_done++;
            buffer_found=1;
        }
        if ((tctx.buf_pool_ext->u32ListSz)&&(tctx.u32OutRxCnt < (u32InFrames)))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool_ext);

            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

            pNode->pIntBuf->pBuf->flags=0;
            vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc);  //For debugging purposes
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            buffer_found=1;
        }
        if (!buffer_found)
           sched_yield();
    }

    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_OUTPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    while (!((tctx.u32Flags & INPUT_FLUSH_DONE)&&(tctx.u32Flags & OUTPUT_FLUSH_DONE)))
        sched_yield();

    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32Ret=u32VppIpFrcMe_Close(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    vVppIpFrcMe_Term(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    LOGI("freeing buffer pools");
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    free_buf_pool(tctx.buf_pool_ext, VPP_FALSE);
    free_buf_pool(tctx.buf_pool, VPP_TRUE);
}

TEST(IpMe_EncodeBuffer_MBI_1080p_Downscale_MultiFlushInput)
{
    uint32_t u32Ret=VPP_OK;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32InFrames, u32InFrames_done;
    t_StVppIpBufReq stInputBufReq;
    t_StVppIpBufReq stOutputBufReq;
    struct vpp_port_param port_param_output;
    char s[MAX_FILE_LEN];

    LOGI("%s() IpMe_EncodeBuffer_MBI", __func__);

    tctx.cb.input_buffer_done = test_me_input_buffer_done;
    tctx.cb.output_buffer_done = test_me_output_buffer_done;
    tctx.cb.event = test_me_event;
    tctx.cb.pv = &tctx;

    tctx.pPrivateCtx = &stCtxMe;
    tctx.u32FramesOut = 20;

    uint32_t frame_width=1920;
    uint32_t frame_height=1080;

    tctx_set_port_params(&tctx, frame_width, frame_height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width=frame_width;
    tctx.params.u32Height=frame_height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cInputPath,(char*)"/data/test/input/", MAX_FILE_SEG_LEN);
    //strcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_1080p.yuv");
    //strcpy((char*)tctx.params.cInputName,(char*)"testmotion_nv12_1080p.yuv");
    strlcpy((char*)tctx.params.cInputName,(char*)"yuvgtmotion_nv12_1080p.yuv", MAX_FILE_SEG_LEN);
    strlcpy((char*)s,(char*)tctx.params.cInputPath, MAX_FILE_SEG_LEN);
    strlcat((char*)s,(char*)tctx.params.cInputName, MAX_FILE_SEG_LEN);
#if FRC_ME_CHECK_FILE
    if (!exists((char*)s))
    {
        DVP_ASSERT_PTR_NNULL(NULL);
        LOGE("Error: %s file does't exist!", (char*)tctx.params.cInputName);
        return;
    }
#endif

    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cOutputPath,(char*)"/data/test/output/", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_1080p.bin, MAX_FILE_SEG_LEN");
    //strlcpy((char*)tctx.params.cOutputName,(char*)"testmotion_nv12_1080p_downscale.bin, MAX_FILE_SEG_LEN");
    strlcpy((char*)tctx.params.cOutputName,(char*)"yuvgtmotion_nv12_1080p_downscale_MultiFlushInput.bin", MAX_FILE_SEG_LEN);

    port_param_output.width=frame_width;
    port_param_output.height=frame_height;
    port_param_output.fmt=VPP_COLOR_FORMAT_NV12_VENUS;

    stCtxMe.pme_ctx = vpVppIpFrcMe_Init(&stCtxMe.stCtx, 0, tctx.cb);
    DVP_ASSERT_PTR_NNULL(stCtxMe.pme_ctx);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, port_param_output);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32VppIpFrcMe_GetBufferRequirements(stCtxMe.pme_ctx, &stInputBufReq, &stOutputBufReq);
    u32InFrames =tctx.u32FramesOut;

    LOGI("%s() init_buf_pool(Input)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params, u32InFrames, VPP_TRUE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

#ifdef TEST_MBI_ALL_NEW_BUFFERS
    struct bufnode * InputNodes[tctx.u32FramesOut];
    for (i=0; i<u32InFrames; i++)
        InputNodes[i]=get_buf(tctx.buf_pool);
#endif

    LOGI("%s() init_buf_pool(Output)", __func__);
    tctx.params.u32Width /= 2;
    tctx.params.u32Height/= 2;
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params, stOutputBufReq.u32MinCnt, VPP_FALSE);
    tctx.params.u32Width *= 2;
    tctx.params.u32Height*= 2;
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    // Queue to input pool
    LOGI("%s() Push buffers loop", __func__);
    LOGI("%s Init Stage. %d input frames, %d output buffers", __func__, u32InFrames, stOutputBufReq.u32MinCnt);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);

    u32InFrames_done =0;
    tctx.u32Flags = 0;
    tctx.buf_pool->u32RdIdx=0;
    tctx.buf_pool->u32WrIdx=0;
    tctx.buf_pool_ext->u32RdIdx=0;
    tctx.buf_pool_ext->u32WrIdx=0;
    tctx.u32InRxCnt = 0;
    tctx.u32OutRxCnt = 0;
    tctx.u32InPutCnt = 0;
    tctx.u32OutPutCnt = 0;
    tctx.u32InHoldCnt = 0;
    tctx.u32OutHoldCnt = 0;
    //Init Stage ----------------------------------
    //Feed Internal FRC Output Buffers
    for (i = 0; i < stOutputBufReq.u32MinCnt; i++)
    {
        pthread_mutex_lock(&tctx.mutex);
        pNode=NULL;
        while (pNode==NULL)
            pNode = get_buf(tctx.buf_pool_ext);
        pthread_mutex_unlock(&tctx.mutex);
        DVP_ASSERT_PTR_NNULL(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;

        //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

        vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc); //For debugging purposes
        pNode->pIntBuf->pBuf->timestamp=0;
        pNode->pIntBuf->pBuf->flags=0;
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    //Feed Stage --------------------------------
    //Feed input and output buffers as they become available.
    LOGI("%s Feed Stage", __func__);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    while ((u32InFrames_done<u32InFrames)||(!(tctx.u32Flags & OUTPUT_EOS)))
    {
        int buffer_found=0;

#ifdef TEST_MBI_ALL_NEW_BUFFERS
//Uncomment this for TEST_MBI_ALL_NEW_BUFFERS
//        if ((u32InFrames_done<u32InFrames))
//        {
//            pthread_mutex_lock(&tctx.mutex);
//            pNode=InputNodes[u32InFrames_done];
#else
        if ((tctx.buf_pool->u32ListSz)&&(u32InFrames_done<u32InFrames))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool);
#endif
            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            fill_buf(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            fill_extra_data(pNode, eVppBufType_Progressive, 0);
            stCtxMe.u32ExpExtraLen = pNode->pIntBuf->stExtra.u32FilledLen;

            pNode->pIntBuf->pBuf->timestamp=u32InFrames_done*TIME_DELAY;  //Inc with 33ms every input buffer
            pNode->pIntBuf->pBuf->flags=0;
            if (u32InFrames_done==(u32InFrames-1))
                pNode->pIntBuf->pBuf->flags|=VPP_BUFFER_FLAG_EOS;
            else if (u32InFrames_done==8)
            {
                u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
                DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
                //u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_OUTPUT);
                //DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
                while (!(tctx.u32Flags & INPUT_FLUSH_DONE))
                    sched_yield();
                tctx.u32Flags &= ~INPUT_FLUSH_DONE;
                tctx.u32Flags &= ~OUTPUT_FLUSH_DONE;
            }

            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            u32InFrames_done++;
            buffer_found=1;
        }
        if ((tctx.buf_pool_ext->u32ListSz))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool_ext);

            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

            pNode->pIntBuf->pBuf->flags=0;
            vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc);  //For debugging purposes
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            buffer_found=1;
        }
        if (!buffer_found)
           sched_yield();
    }

    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_OUTPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    while (!((tctx.u32Flags & INPUT_FLUSH_DONE)&&(tctx.u32Flags & OUTPUT_FLUSH_DONE)))
        sched_yield();

    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32Ret=u32VppIpFrcMe_Close(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    vVppIpFrcMe_Term(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    LOGI("freeing buffer pools");
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    free_buf_pool(tctx.buf_pool_ext, VPP_FALSE);
    free_buf_pool(tctx.buf_pool, VPP_TRUE);
}


TEST(IpMe_EncodeBuffer_MBI_1080p_Downscale_MultiFlushInputAndOutput)
{
    uint32_t u32Ret=VPP_OK;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32InFrames, u32InFrames_done;
    t_StVppIpBufReq stInputBufReq;
    t_StVppIpBufReq stOutputBufReq;
    struct vpp_port_param port_param_output;
    char s[MAX_FILE_LEN];

    LOGI("%s() IpMe_EncodeBuffer_MBI", __func__);

    tctx.cb.input_buffer_done = test_me_input_buffer_done;
    tctx.cb.output_buffer_done = test_me_output_buffer_done;
    tctx.cb.event = test_me_event;
    tctx.cb.pv = &tctx;

    tctx.pPrivateCtx = &stCtxMe;
    tctx.u32FramesOut = 20;

    uint32_t frame_width=1920;
    uint32_t frame_height=1080;

    tctx_set_port_params(&tctx, frame_width, frame_height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width=frame_width;
    tctx.params.u32Height=frame_height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cInputPath,(char*)"/data/test/input/", MAX_FILE_SEG_LEN);
    //strcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_1080p.yuv");
    //strcpy((char*)tctx.params.cInputName,(char*)"testmotion_nv12_1080p.yuv");
    strlcpy((char*)tctx.params.cInputName,(char*)"yuvgtmotion_nv12_1080p.yuv", MAX_FILE_SEG_LEN);
    strlcpy((char*)s,(char*)tctx.params.cInputPath, MAX_FILE_SEG_LEN);
    strlcat((char*)s,(char*)tctx.params.cInputName, MAX_FILE_SEG_LEN);
#if FRC_ME_CHECK_FILE
    if (!exists((char*)s))
    {
        DVP_ASSERT_PTR_NNULL(NULL);
        LOGE("Error: %s file does't exist!", (char*)tctx.params.cInputName);
        return;
    }
#endif

    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cOutputPath,(char*)"/data/test/output/", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_1080p.bin, MAX_FILE_SEG_LEN");
    //strlcpy((char*)tctx.params.cOutputName,(char*)"testmotion_nv12_1080p_downscale.bin, MAX_FILE_SEG_LEN");
    strlcpy((char*)tctx.params.cOutputName,(char*)"yuvgtmotion_nv12_1080p_downscale_MultiFlushInputAndOutput.bin", MAX_FILE_SEG_LEN);

    port_param_output.width=frame_width;
    port_param_output.height=frame_height;
    port_param_output.fmt=VPP_COLOR_FORMAT_NV12_VENUS;

    stCtxMe.pme_ctx = vpVppIpFrcMe_Init(&stCtxMe.stCtx, 0, tctx.cb);
    DVP_ASSERT_PTR_NNULL(stCtxMe.pme_ctx);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, port_param_output);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32VppIpFrcMe_GetBufferRequirements(stCtxMe.pme_ctx, &stInputBufReq, &stOutputBufReq);
    u32InFrames =tctx.u32FramesOut;

    LOGI("%s() init_buf_pool(Input)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params, u32InFrames, VPP_TRUE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

#ifdef TEST_MBI_ALL_NEW_BUFFERS
    struct bufnode * InputNodes[tctx.u32FramesOut];
    for (i=0; i<u32InFrames; i++)
        InputNodes[i]=get_buf(tctx.buf_pool);
#endif

    LOGI("%s() init_buf_pool(Output)", __func__);
    tctx.params.u32Width /= 2;
    tctx.params.u32Height/= 2;
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params, stOutputBufReq.u32MinCnt, VPP_FALSE);
    tctx.params.u32Width *= 2;
    tctx.params.u32Height*= 2;
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    // Queue to input pool
    LOGI("%s() Push buffers loop", __func__);
    LOGI("%s Init Stage. %d input frames, %d output buffers", __func__, u32InFrames, stOutputBufReq.u32MinCnt);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);

    u32InFrames_done =0;
    tctx.u32Flags = 0;
    tctx.buf_pool->u32RdIdx=0;
    tctx.buf_pool->u32WrIdx=0;
    tctx.buf_pool_ext->u32RdIdx=0;
    tctx.buf_pool_ext->u32WrIdx=0;
    tctx.u32InRxCnt = 0;
    tctx.u32OutRxCnt = 0;
    tctx.u32InPutCnt = 0;
    tctx.u32OutPutCnt = 0;
    tctx.u32InHoldCnt = 0;
    tctx.u32OutHoldCnt = 0;
    //Init Stage ----------------------------------
    //Feed Internal FRC Output Buffers
    for (i = 0; i < stOutputBufReq.u32MinCnt; i++)
    {
        pthread_mutex_lock(&tctx.mutex);
        pNode=NULL;
        while (pNode==NULL)
            pNode = get_buf(tctx.buf_pool_ext);
        pthread_mutex_unlock(&tctx.mutex);
        DVP_ASSERT_PTR_NNULL(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;

        //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

        vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc); //For debugging purposes
        pNode->pIntBuf->pBuf->timestamp=0;
        pNode->pIntBuf->pBuf->flags=0;
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    //Feed Stage --------------------------------
    //Feed input and output buffers as they become available.
    LOGI("%s Feed Stage", __func__);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    while ((u32InFrames_done<u32InFrames)||(!(tctx.u32Flags & OUTPUT_EOS)))
    {
        int buffer_found=0;

#ifdef TEST_MBI_ALL_NEW_BUFFERS
//Uncomment this for TEST_MBI_ALL_NEW_BUFFERS
//        if ((u32InFrames_done<u32InFrames))
//        {
//            pthread_mutex_lock(&tctx.mutex);
//            pNode=InputNodes[u32InFrames_done];
#else
        if ((tctx.buf_pool->u32ListSz)&&(u32InFrames_done<u32InFrames))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool);
#endif
            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            fill_buf(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            fill_extra_data(pNode, eVppBufType_Progressive, 0);
            stCtxMe.u32ExpExtraLen = pNode->pIntBuf->stExtra.u32FilledLen;

            pNode->pIntBuf->pBuf->timestamp=u32InFrames_done*TIME_DELAY;  //Inc with 33ms every input buffer
            pNode->pIntBuf->pBuf->flags=0;
            if (u32InFrames_done==(u32InFrames-1))
                pNode->pIntBuf->pBuf->flags|=VPP_BUFFER_FLAG_EOS;
            else if (u32InFrames_done==8)
            {
                u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
                DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
                u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_OUTPUT);
                DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
                while (!((tctx.u32Flags & INPUT_FLUSH_DONE)&&(tctx.u32Flags & OUTPUT_FLUSH_DONE)))
                    sched_yield();
                tctx.u32Flags &= ~INPUT_FLUSH_DONE;
                tctx.u32Flags &= ~OUTPUT_FLUSH_DONE;
            }

            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            u32InFrames_done++;
            buffer_found=1;
        }
        if ((tctx.buf_pool_ext->u32ListSz))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool_ext);

            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

            pNode->pIntBuf->pBuf->flags=0;
            vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc);  //For debugging purposes
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            buffer_found=1;
        }
        if (!buffer_found)
           sched_yield();
    }

    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_OUTPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    while (!((tctx.u32Flags & INPUT_FLUSH_DONE)&&(tctx.u32Flags & OUTPUT_FLUSH_DONE)))
        sched_yield();

    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32Ret=u32VppIpFrcMe_Close(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    vVppIpFrcMe_Term(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    LOGI("freeing buffer pools");
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    free_buf_pool(tctx.buf_pool_ext, VPP_FALSE);
    free_buf_pool(tctx.buf_pool, VPP_TRUE);
}

//20 inputs -> 10 MBIs
TEST(IpMe_EncodeBuffer_MBI_1080p_Downscale_123)
{
    uint32_t u32Ret=VPP_OK;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32InFrames, u32InFrames_done;
    t_StVppIpBufReq stInputBufReq;
    t_StVppIpBufReq stOutputBufReq;
    struct vpp_port_param port_param_output;
    char s[MAX_FILE_LEN];
    int no_loops=1;

    LOGI("%s() IpMe_EncodeBuffer_MBI", __func__);

    tctx.cb.input_buffer_done = test_me_input_buffer_done;
    tctx.cb.output_buffer_done = test_me_output_buffer_done;
    tctx.cb.event = test_me_event;
    tctx.cb.pv = &tctx;

    tctx.pPrivateCtx = &stCtxMe;
    tctx.u32FramesOut = 20;

    uint32_t frame_width=1920;
    uint32_t frame_height=1080;

    tctx_set_port_params(&tctx, frame_width, frame_height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width=frame_width;
    tctx.params.u32Height=frame_height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cInputPath,(char*)"/data/test/input/", MAX_FILE_SEG_LEN);
    //strcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_1080p.yuv");
    //strcpy((char*)tctx.params.cInputName,(char*)"testmotion_nv12_1080p.yuv");
    strlcpy((char*)tctx.params.cInputName,(char*)"yuvgtmotion_nv12_1080p.yuv", MAX_FILE_SEG_LEN);
    strlcpy((char*)s,(char*)tctx.params.cInputPath, MAX_FILE_SEG_LEN);
    strlcat((char*)s,(char*)tctx.params.cInputName, MAX_FILE_SEG_LEN);
#if FRC_ME_CHECK_FILE
    if (!exists((char*)s))
    {
        DVP_ASSERT_PTR_NNULL(NULL);
        LOGE("Error: %s file does't exist!", (char*)tctx.params.cInputName);
        return;
    }
#endif

    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cOutputPath,(char*)"/data/test/output/", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_1080p.bin, MAX_FILE_SEG_LEN");
    //strlcpy((char*)tctx.params.cOutputName,(char*)"testmotion_nv12_1080p_downscale.bin, MAX_FILE_SEG_LEN");
    strlcpy((char*)tctx.params.cOutputName,(char*)"yuvgtmotion_nv12_1080p_downscale_123.bin",MAX_FILE_SEG_LEN);

    port_param_output.width=frame_width;
    port_param_output.height=frame_height;
    port_param_output.fmt=VPP_COLOR_FORMAT_NV12_VENUS;

    stCtxMe.pme_ctx = vpVppIpFrcMe_Init(&stCtxMe.stCtx, 0, tctx.cb);
    DVP_ASSERT_PTR_NNULL(stCtxMe.pme_ctx);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, port_param_output);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32VppIpFrcMe_GetBufferRequirements(stCtxMe.pme_ctx, &stInputBufReq, &stOutputBufReq);
    u32InFrames =tctx.u32FramesOut;

    LOGI("%s() init_buf_pool(Input)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params, u32InFrames, VPP_TRUE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

#ifdef TEST_MBI_ALL_NEW_BUFFERS
    struct bufnode * InputNodes[tctx.u32FramesOut];
    for (i=0; i<u32InFrames; i++)
        InputNodes[i]=get_buf(tctx.buf_pool);
#endif

    LOGI("%s() init_buf_pool(Output)", __func__);
    tctx.params.u32Width /= 2;
    tctx.params.u32Height/= 2;
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params, stOutputBufReq.u32MinCnt, VPP_FALSE);
    tctx.params.u32Width *= 2;
    tctx.params.u32Height*= 2;
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

adr_loop:
    // Queue to input pool
    LOGI("Loop #%d",no_loops);
    LOGI("%s() Push buffers loop", __func__);
    LOGI("%s Init Stage. %d input frames, %d output buffers", __func__, u32InFrames, stOutputBufReq.u32MinCnt);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);

    u32InFrames_done =0;
    tctx.u32Flags = 0;
    tctx.buf_pool->u32RdIdx=0;
    tctx.buf_pool->u32WrIdx=0;
    tctx.buf_pool_ext->u32RdIdx=0;
    tctx.buf_pool_ext->u32WrIdx=0;
    tctx.u32InRxCnt = 0;
    tctx.u32OutRxCnt = 0;
    tctx.u32InPutCnt = 0;
    tctx.u32OutPutCnt = 0;
    tctx.u32InHoldCnt = 0;
    tctx.u32OutHoldCnt = 0;
    //Init Stage ----------------------------------
    //Feed Internal FRC Output Buffers
    for (i = 0; i < stOutputBufReq.u32MinCnt; i++)
    {
        pthread_mutex_lock(&tctx.mutex);
        pNode=NULL;
        while (pNode==NULL)
            pNode = get_buf(tctx.buf_pool_ext);
        pthread_mutex_unlock(&tctx.mutex);
        DVP_ASSERT_PTR_NNULL(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;

        //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

        vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc); //For debugging purposes
        pNode->pIntBuf->pBuf->timestamp=0;
        pNode->pIntBuf->pBuf->flags=0;
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    //Feed Stage --------------------------------
    //Feed input and output buffers as they become available.
    LOGI("%s Feed Stage", __func__);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    while ((u32InFrames_done<u32InFrames)||(tctx.u32InRxCnt < u32InFrames))
    {
        int buffer_found=0;

#ifdef TEST_MBI_ALL_NEW_BUFFERS
//Uncomment this for TEST_MBI_ALL_NEW_BUFFERS
//        if ((u32InFrames_done<u32InFrames))
//        {
//            pthread_mutex_lock(&tctx.mutex);
//            pNode=InputNodes[u32InFrames_done];
#else
        if ((tctx.buf_pool->u32ListSz)&&(u32InFrames_done<u32InFrames))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool);
#endif
            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            fill_buf(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            fill_extra_data(pNode, eVppBufType_Progressive, 0);
            stCtxMe.u32ExpExtraLen = pNode->pIntBuf->stExtra.u32FilledLen;

            pNode->pIntBuf->pBuf->timestamp=u32InFrames_done*TIME_DELAY;  //Inc with 33ms every input buffer
            pNode->pIntBuf->pBuf->flags=0;
            if ((u32InFrames_done==(u32InFrames-1))||
                (u32InFrames_done==0)||(u32InFrames_done==1)||(u32InFrames_done==3)||
                (u32InFrames_done==4)||(u32InFrames_done==7)||(u32InFrames_done==8)||
                (u32InFrames_done==9)||(u32InFrames_done==11)||(u32InFrames_done==13))
                pNode->pIntBuf->pBuf->flags|=VPP_BUFFER_FLAG_EOS;
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            u32InFrames_done++;
            buffer_found=1;
        }
        if ((tctx.buf_pool_ext->u32ListSz))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool_ext);

            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

            pNode->pIntBuf->pBuf->flags=0;
            vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc);  //For debugging purposes
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            buffer_found=1;
        }
        if (!buffer_found)
           sched_yield();
    }

    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_OUTPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    while (!((tctx.u32Flags & INPUT_FLUSH_DONE)&&(tctx.u32Flags & OUTPUT_FLUSH_DONE)))
        sched_yield();

    no_loops--;
    if (no_loops)
        goto adr_loop;

    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32Ret=u32VppIpFrcMe_Close(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    vVppIpFrcMe_Term(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    LOGI("freeing buffer pools");
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    free_buf_pool(tctx.buf_pool_ext, VPP_FALSE);
    free_buf_pool(tctx.buf_pool, VPP_TRUE);
}

TEST(__unused IpMe_EncodeBuffer_MBI_1080p_Downscale_123_EOS_len_zero)
{
    uint32_t u32Ret=VPP_OK;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32InFrames, u32InFrames_done;
    t_StVppIpBufReq stInputBufReq;
    t_StVppIpBufReq stOutputBufReq;
    struct vpp_port_param port_param_output;
    char s[MAX_FILE_LEN];
    int no_loops=1;

    LOGI("%s() IpMe_EncodeBuffer_MBI", __func__);

    tctx.cb.input_buffer_done = test_me_input_buffer_done;
    tctx.cb.output_buffer_done = test_me_output_buffer_done;
    tctx.cb.event = test_me_event;
    tctx.cb.pv = &tctx;

    tctx.pPrivateCtx = &stCtxMe;
    tctx.u32FramesOut = 20;

    uint32_t frame_width=1920;
    uint32_t frame_height=1080;

    tctx_set_port_params(&tctx, frame_width, frame_height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width=frame_width;
    tctx.params.u32Height=frame_height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cInputPath,(char*)"/data/test/input/", MAX_FILE_SEG_LEN);
    //strcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_1080p.yuv");
    //strcpy((char*)tctx.params.cInputName,(char*)"testmotion_nv12_1080p.yuv");
    strlcpy((char*)tctx.params.cInputName,(char*)"yuvgtmotion_nv12_1080p.yuv", MAX_FILE_SEG_LEN);
    strlcpy((char*)s,(char*)tctx.params.cInputPath, MAX_FILE_SEG_LEN);
    strlcat((char*)s,(char*)tctx.params.cInputName, MAX_FILE_SEG_LEN);
#if FRC_ME_CHECK_FILE
    if (!exists((char*)s))
    {
        DVP_ASSERT_PTR_NNULL(NULL);
        LOGE("Error: %s file does't exist!", (char*)tctx.params.cInputName);
        return;
    }
#endif

    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cOutputPath,(char*)"/data/test/output/", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_1080p.bin, MAX_FILE_SEG_LEN");
    //strlcpy((char*)tctx.params.cOutputName,(char*)"testmotion_nv12_1080p_downscale.bin, MAX_FILE_SEG_LEN");
    strlcpy((char*)tctx.params.cOutputName,(char*)"yuvgtmotion_nv12_1080p_downscale_123_EOS_len_zero.bin",MAX_FILE_SEG_LEN);

    port_param_output.width=frame_width;
    port_param_output.height=frame_height;
    port_param_output.fmt=VPP_COLOR_FORMAT_NV12_VENUS;

    stCtxMe.pme_ctx = vpVppIpFrcMe_Init(&stCtxMe.stCtx, 0, tctx.cb);
    DVP_ASSERT_PTR_NNULL(stCtxMe.pme_ctx);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, port_param_output);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32VppIpFrcMe_GetBufferRequirements(stCtxMe.pme_ctx, &stInputBufReq, &stOutputBufReq);
    u32InFrames =tctx.u32FramesOut;

    LOGI("%s() init_buf_pool(Input)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params, u32InFrames, VPP_TRUE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

#ifdef TEST_MBI_ALL_NEW_BUFFERS
    struct bufnode * InputNodes[tctx.u32FramesOut];
    for (i=0; i<u32InFrames; i++)
        InputNodes[i]=get_buf(tctx.buf_pool);
#endif

    LOGI("%s() init_buf_pool(Output)", __func__);
    tctx.params.u32Width /= 2;
    tctx.params.u32Height/= 2;
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params, stOutputBufReq.u32MinCnt, VPP_FALSE);
    tctx.params.u32Width *= 2;
    tctx.params.u32Height*= 2;
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

adr_loop:
    // Queue to input pool
    LOGI("Loop #%d",no_loops);
    LOGI("%s() Push buffers loop", __func__);
    LOGI("%s Init Stage. %d input frames, %d output buffers", __func__, u32InFrames, stOutputBufReq.u32MinCnt);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);

    u32InFrames_done =0;
    tctx.u32Flags = 0;
    tctx.buf_pool->u32RdIdx=0;
    tctx.buf_pool->u32WrIdx=0;
    tctx.buf_pool_ext->u32RdIdx=0;
    tctx.buf_pool_ext->u32WrIdx=0;
    tctx.u32InRxCnt = 0;
    tctx.u32OutRxCnt = 0;
    tctx.u32InPutCnt = 0;
    tctx.u32OutPutCnt = 0;
    tctx.u32InHoldCnt = 0;
    tctx.u32OutHoldCnt = 0;
    //Init Stage ----------------------------------
    //Feed Internal FRC Output Buffers
    for (i = 0; i < stOutputBufReq.u32MinCnt; i++)
    {
        pthread_mutex_lock(&tctx.mutex);
        pNode=NULL;
        while (pNode==NULL)
            pNode = get_buf(tctx.buf_pool_ext);
        pthread_mutex_unlock(&tctx.mutex);
        DVP_ASSERT_PTR_NNULL(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;

        //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

        vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc); //For debugging purposes
        pNode->pIntBuf->pBuf->timestamp=0;
        pNode->pIntBuf->pBuf->flags=0;
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    //Feed Stage --------------------------------
    //Feed input and output buffers as they become available.
    LOGI("%s Feed Stage", __func__);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    while ((u32InFrames_done<u32InFrames)||(tctx.u32InRxCnt < u32InFrames))
    {
        int buffer_found=0;

#ifdef TEST_MBI_ALL_NEW_BUFFERS
//Uncomment this for TEST_MBI_ALL_NEW_BUFFERS
//        if ((u32InFrames_done<u32InFrames))
//        {
//            pthread_mutex_lock(&tctx.mutex);
//            pNode=InputNodes[u32InFrames_done];
#else
        if ((tctx.buf_pool->u32ListSz)&&(u32InFrames_done<u32InFrames))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool);
#endif
            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            fill_buf(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            fill_extra_data(pNode, eVppBufType_Progressive, 0);
            stCtxMe.u32ExpExtraLen = pNode->pIntBuf->stExtra.u32FilledLen;

            pNode->pIntBuf->pBuf->timestamp=u32InFrames_done*TIME_DELAY;  //Inc with 33ms every input buffer
            pNode->pIntBuf->pBuf->flags=0;
            if ((u32InFrames_done==(u32InFrames-1))||
                (u32InFrames_done==0)||(u32InFrames_done==1)||(u32InFrames_done==3)||
                (u32InFrames_done==4)||(u32InFrames_done==7)||(u32InFrames_done==8)||
                (u32InFrames_done==9)||(u32InFrames_done==11)||(u32InFrames_done==13))
            {
                pNode->pIntBuf->pBuf->flags|=VPP_BUFFER_FLAG_EOS;
                pNode->pIntBuf->stPixel.u32FilledLen = 0;
            }

            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            u32InFrames_done++;
            buffer_found=1;
        }
        if ((tctx.buf_pool_ext->u32ListSz))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool_ext);

            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

            pNode->pIntBuf->pBuf->flags=0;
            vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc);  //For debugging purposes
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            buffer_found=1;
        }
        if (!buffer_found)
           sched_yield();
    }

    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_OUTPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    while (!((tctx.u32Flags & INPUT_FLUSH_DONE)&&(tctx.u32Flags & OUTPUT_FLUSH_DONE)))
        sched_yield();

    no_loops--;
    if (no_loops)
        goto adr_loop;

    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32Ret=u32VppIpFrcMe_Close(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    vVppIpFrcMe_Term(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    LOGI("freeing buffer pools");
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    free_buf_pool(tctx.buf_pool_ext, VPP_FALSE);
    free_buf_pool(tctx.buf_pool, VPP_TRUE);
}

TEST(IpMe_EncodeBuffer_MBI_1080p_Downscale_MultiOpen)
{
    uint32_t u32Ret=VPP_OK;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32InFrames, u32InFrames_done;
    t_StVppIpBufReq stInputBufReq;
    t_StVppIpBufReq stOutputBufReq;
    struct vpp_port_param port_param_output;
    char s[MAX_FILE_LEN];
    int MultiOpen=10;

    LOGI("%s() IpMe_EncodeBuffer_MBI", __func__);

    tctx.cb.input_buffer_done = test_me_input_buffer_done;
    tctx.cb.output_buffer_done = test_me_output_buffer_done;
    tctx.cb.event = test_me_event;
    tctx.cb.pv = &tctx;

    tctx.pPrivateCtx = &stCtxMe;
    tctx.u32FramesOut = 10;

    uint32_t frame_width=1920;
    uint32_t frame_height=1080;

    tctx_set_port_params(&tctx, frame_width, frame_height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width=frame_width;
    tctx.params.u32Height=frame_height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cInputPath,(char*)"/data/test/input/", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_1080p.yuv", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"testmotion_nv12_1080p.yuv", MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cInputName,(char*)"yuvgtmotion_nv12_1080p.yuv", MAX_FILE_SEG_LEN);
    strlcpy((char*)s,(char*)tctx.params.cInputPath, MAX_FILE_SEG_LEN);
    strlcat((char*)s,(char*)tctx.params.cInputName, MAX_FILE_SEG_LEN);
#if FRC_ME_CHECK_FILE
    if (!exists((char*)s))
    {
        DVP_ASSERT_PTR_NNULL(NULL);
        LOGE("Error: %s file does't exist!", (char*)tctx.params.cInputName);
        return;
    }
#endif
    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cOutputPath,(char*)"/data/test/output/", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_1080p.bin", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"testmotion_nv12_1080p_downscale.bin", MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cOutputName,(char*)"yuvgtmotion_nv12_1080p_downscale_MultiOpen.bin", MAX_FILE_SEG_LEN);

    port_param_output.width=frame_width;
    port_param_output.height=frame_height;
    port_param_output.fmt=VPP_COLOR_FORMAT_NV12_VENUS;

    stCtxMe.pme_ctx = vpVppIpFrcMe_Init(&stCtxMe.stCtx, 0, tctx.cb);
    DVP_ASSERT_PTR_NNULL(stCtxMe.pme_ctx);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, port_param_output);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32VppIpFrcMe_GetBufferRequirements(stCtxMe.pme_ctx, &stInputBufReq, &stOutputBufReq);
    u32InFrames =tctx.u32FramesOut;

    LOGI("%s() init_buf_pool(Input)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params, u32InFrames, VPP_TRUE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

#ifdef TEST_MBI_ALL_NEW_BUFFERS
    struct bufnode * InputNodes[tctx.u32FramesOut];
    for (i=0; i<u32InFrames; i++)
        InputNodes[i]=get_buf(tctx.buf_pool);
#endif

    LOGI("%s() init_buf_pool(Output)", __func__);
    tctx.params.u32Width /= 2;
    tctx.params.u32Height/= 2;
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params, stOutputBufReq.u32MinCnt, VPP_FALSE);
    tctx.params.u32Width *= 2;
    tctx.params.u32Height*= 2;
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

adr_multiopen:
    // Queue to input pool
    LOGI("%s() Push buffers loop", __func__);
    LOGI("%s Init Stage. %d input frames, %d output buffers", __func__, u32InFrames, stOutputBufReq.u32MinCnt);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32InFrames_done =0;
    tctx.u32Flags = 0;
    tctx.buf_pool->u32RdIdx=0;
    tctx.buf_pool->u32WrIdx=0;
    tctx.buf_pool_ext->u32RdIdx=0;
    tctx.buf_pool_ext->u32WrIdx=0;
    tctx.u32InRxCnt = 0;
    tctx.u32OutRxCnt = 0;
    tctx.u32InPutCnt = 0;
    tctx.u32OutPutCnt = 0;
    tctx.u32InHoldCnt = 0;
    tctx.u32OutHoldCnt = 0;

    //Init Stage ----------------------------------
    //Feed Internal FRC Output Buffers
    for (i = 0; i < stOutputBufReq.u32MinCnt; i++)
    {
        pthread_mutex_lock(&tctx.mutex);
        pNode=NULL;
        while (pNode==NULL)
            pNode = get_buf(tctx.buf_pool_ext);
        pthread_mutex_unlock(&tctx.mutex);
        DVP_ASSERT_PTR_NNULL(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;

        //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

        vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc); //For debugging purposes
        pNode->pIntBuf->pBuf->timestamp=0;
        pNode->pIntBuf->pBuf->flags=0;
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    //Feed Stage --------------------------------
    //Feed input and output buffers as they become available.
    LOGI("%s Feed Stage", __func__);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    while ((u32InFrames_done<u32InFrames)||(tctx.u32InRxCnt < u32InFrames)||(tctx.u32OutRxCnt < (u32InFrames)))
    {
        int buffer_found=0;

#ifdef TEST_MBI_ALL_NEW_BUFFERS
//Uncomment this for TEST_MBI_ALL_NEW_BUFFERS
//        if ((u32InFrames_done<u32InFrames))
//        {
//            pthread_mutex_lock(&tctx.mutex);
//            pNode=InputNodes[u32InFrames_done];
#else
        //LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
        if ((tctx.buf_pool->u32ListSz)&&(u32InFrames_done<u32InFrames))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool);
#endif
            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            fill_buf(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            fill_extra_data(pNode, eVppBufType_Progressive, 0);
            stCtxMe.u32ExpExtraLen = pNode->pIntBuf->stExtra.u32FilledLen;

            pNode->pIntBuf->pBuf->timestamp=u32InFrames_done*TIME_DELAY;  //Inc with 33ms every input buffer
            pNode->pIntBuf->pBuf->flags=0;
            if (u32InFrames_done==(u32InFrames-1))
                pNode->pIntBuf->pBuf->flags|=VPP_BUFFER_FLAG_EOS;
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            u32InFrames_done++;
            buffer_found=1;
        }
        if ((tctx.buf_pool_ext->u32ListSz)&&(tctx.u32OutRxCnt < (u32InFrames)))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool_ext);

            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

            pNode->pIntBuf->pBuf->flags=0;
            vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc);  //For debugging purposes
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            buffer_found=1;
        }
        if (!buffer_found)
           sched_yield();
    }

    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_OUTPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    while (!((tctx.u32Flags & INPUT_FLUSH_DONE)&&(tctx.u32Flags & OUTPUT_FLUSH_DONE)))
        sched_yield();

    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32Ret=u32VppIpFrcMe_Close(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    if (MultiOpen)
    {
        MultiOpen--;
        u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
        goto adr_multiopen;
    }

    vVppIpFrcMe_Term(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    LOGI("freeing buffer pools");
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    free_buf_pool(tctx.buf_pool_ext, VPP_FALSE);
    free_buf_pool(tctx.buf_pool, VPP_TRUE);
}

TEST(IpMe_EncodeBuffer_MBI_1080p_Downscale_Reprogram)
{
    uint32_t u32Ret=VPP_OK;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32InFrames, u32InFrames_done;
    t_StVppIpBufReq stInputBufReq;
    t_StVppIpBufReq stOutputBufReq;
    struct vpp_port_param port_param_output;
    char s[MAX_FILE_LEN];

    LOGI("%s() IpMe_EncodeBuffer_MBI", __func__);

    tctx.cb.input_buffer_done = test_me_input_buffer_done;
    tctx.cb.output_buffer_done = test_me_output_buffer_done;
    tctx.cb.event = test_me_event;
    tctx.cb.pv = &tctx;

    tctx.pPrivateCtx = &stCtxMe;
    tctx.u32FramesOut = 10;
    tctx.u32Flags = 0;

    stCtxMe.pme_ctx = vpVppIpFrcMe_Init(&stCtxMe.stCtx, 0, tctx.cb);
    DVP_ASSERT_PTR_NNULL(stCtxMe.pme_ctx);

    uint32_t frame_width=1920;
    uint32_t frame_height=1080;

    tctx_set_port_params(&tctx, frame_width, frame_height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width=frame_width;
    tctx.params.u32Height=frame_height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cInputPath,(char*)"/data/test/input/", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_1080p.yuv", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"testmotion_nv12_1080p.yuv", MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cInputName,(char*)"yuvgtmotion_nv12_1080p.yuv", MAX_FILE_SEG_LEN);
    strlcpy((char*)s,(char*)tctx.params.cInputPath, MAX_FILE_SEG_LEN);
    strlcat((char*)s,(char*)tctx.params.cInputName, MAX_FILE_SEG_LEN);
#if FRC_ME_CHECK_FILE
    if (!exists((char*)s))
    {
        DVP_ASSERT_PTR_NNULL(NULL);
        LOGE("Error: %s file does't exist!", (char*)tctx.params.cInputName);
        return;
    }
#endif
    LOGI("Input File: %s",s);
    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cOutputPath,(char*)"/data/test/output/", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_1080p.bin", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"testmotion_nv12_1080p_downscale.bin", MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cOutputName,(char*)"yuvgtmotion_nv12_1080p_downscale_Reprogram.bin", MAX_FILE_SEG_LEN);

    port_param_output.width=frame_width;
    port_param_output.height=frame_height;
    port_param_output.fmt=VPP_COLOR_FORMAT_NV12_VENUS;

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, port_param_output);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32VppIpFrcMe_GetBufferRequirements(stCtxMe.pme_ctx, &stInputBufReq, &stOutputBufReq);
    u32InFrames =tctx.u32FramesOut;

    LOGI("%s() init_buf_pool(Input)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params, u32InFrames, VPP_TRUE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

#ifdef TEST_MBI_ALL_NEW_BUFFERS
    struct bufnode * InputNodes[tctx.u32FramesOut];
    for (i=0; i<u32InFrames; i++)
        InputNodes[i]=get_buf(tctx.buf_pool);
#endif

    LOGI("%s() init_buf_pool(Output)", __func__);
    tctx.params.u32Width /= 2;
    tctx.params.u32Height/= 2;
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params, stOutputBufReq.u32MinCnt, VPP_FALSE);
    tctx.params.u32Width *= 2;
    tctx.params.u32Height*= 2;
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    // Queue to input pool
    LOGI("%s() Push buffers loop", __func__);
    LOGI("%s Init Stage. %d input frames, %d output buffers", __func__, u32InFrames, stOutputBufReq.u32MinCnt);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32InFrames_done =0;
    tctx.u32Flags = 0;
    tctx.buf_pool->u32RdIdx=0;
    tctx.buf_pool->u32WrIdx=0;
    tctx.buf_pool_ext->u32RdIdx=0;
    tctx.buf_pool_ext->u32WrIdx=0;
    tctx.u32InRxCnt = 0;
    tctx.u32OutRxCnt = 0;
    tctx.u32InPutCnt = 0;
    tctx.u32OutPutCnt = 0;
    tctx.u32InHoldCnt = 0;
    tctx.u32OutHoldCnt = 0;

    //Init Stage ----------------------------------
    //Feed Internal FRC Output Buffers
    for (i = 0; i < stOutputBufReq.u32MinCnt; i++)
    {
        pthread_mutex_lock(&tctx.mutex);
        pNode=NULL;
        while (pNode==NULL)
            pNode = get_buf(tctx.buf_pool_ext);
        pthread_mutex_unlock(&tctx.mutex);
        DVP_ASSERT_PTR_NNULL(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;

        //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

        vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc); //For debugging purposes
        pNode->pIntBuf->pBuf->timestamp=0;
        pNode->pIntBuf->pBuf->flags=0;
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    //Feed Stage --------------------------------
    //Feed input and output buffers as they become available.
    LOGI("%s Feed Stage", __func__);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    while ((u32InFrames_done<u32InFrames)||(tctx.u32InRxCnt < u32InFrames)||(tctx.u32OutRxCnt < (u32InFrames)))
    {
        int buffer_found=0;

#ifdef TEST_MBI_ALL_NEW_BUFFERS
//Uncomment this for TEST_MBI_ALL_NEW_BUFFERS
//        if ((u32InFrames_done<u32InFrames))
//        {
//            pthread_mutex_lock(&tctx.mutex);
//            pNode=InputNodes[u32InFrames_done];
#else
        //LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
        if ((tctx.buf_pool->u32ListSz)&&(u32InFrames_done<u32InFrames))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool);
#endif
            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            fill_buf(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            fill_extra_data(pNode, eVppBufType_Progressive, 0);
            stCtxMe.u32ExpExtraLen = pNode->pIntBuf->stExtra.u32FilledLen;

            pNode->pIntBuf->pBuf->timestamp=u32InFrames_done*TIME_DELAY;  //Inc with 33ms every input buffer
            pNode->pIntBuf->pBuf->flags=0;
            if (u32InFrames_done==(u32InFrames-1))
                pNode->pIntBuf->pBuf->flags|=VPP_BUFFER_FLAG_EOS;
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            u32InFrames_done++;
            buffer_found=1;
        }
        if ((tctx.buf_pool_ext->u32ListSz)&&(tctx.u32OutRxCnt < (u32InFrames)))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool_ext);

            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

            pNode->pIntBuf->pBuf->flags=0;
            vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc);  //For debugging purposes
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            buffer_found=1;
        }
        if (!buffer_found)
           sched_yield();
    }

    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_OUTPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    while (!((tctx.u32Flags & INPUT_FLUSH_DONE)&&(tctx.u32Flags & OUTPUT_FLUSH_DONE)))
        sched_yield();

    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32Ret=u32VppIpFrcMe_Close(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    LOGI("freeing buffer pools");
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    free_buf_pool(tctx.buf_pool_ext, VPP_FALSE);
    free_buf_pool(tctx.buf_pool, VPP_TRUE);

    //---------------------------------
    //Reprogram
    //---------------------------------

    tctx.u32Flags = 0;
    frame_width=720;
    frame_height=480;

    tctx_set_port_params(&tctx, frame_width, frame_height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width=frame_width;
    tctx.params.u32Height=frame_height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cInputPath,(char*)"/data/test/input/", MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_480p.yuv", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_1080p.yuv", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"testmotion_nv12_1080p.yuv", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"yuvgtmotion_nv12_1080p.yuv", MAX_FILE_SEG_LEN);
    strlcpy((char*)s,(char*)tctx.params.cInputPath, MAX_FILE_SEG_LEN);
    strlcat((char*)s,(char*)tctx.params.cInputName, MAX_FILE_SEG_LEN);
#if FRC_ME_CHECK_FILE
    if (!exists((char*)s))
    {
        DVP_ASSERT_PTR_NNULL(NULL);
        LOGE("Error: %s file does't exist!", (char*)tctx.params.cInputName);
        return;
    }
#endif
    LOGI("Input File: %s",s);
    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cOutputPath,(char*)"/data/test/output/", MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_480p_Reprogram.bin", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_1080p.bin", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"testmotion_nv12_1080p_downscale.bin", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"yuvgtmotion_nv12_1080p_downscale.bin", MAX_FILE_SEG_LEN);

    port_param_output.width=frame_width;
    port_param_output.height=frame_height;
    port_param_output.fmt=VPP_COLOR_FORMAT_NV12_VENUS;

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, port_param_output);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32VppIpFrcMe_GetBufferRequirements(stCtxMe.pme_ctx, &stInputBufReq, &stOutputBufReq);
    u32InFrames =tctx.u32FramesOut;

    LOGI("%s() init_buf_pool(Input)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params, u32InFrames, VPP_TRUE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

#ifdef TEST_MBI_ALL_NEW_BUFFERS
    struct bufnode * InputNodes[tctx.u32FramesOut];
    for (i=0; i<u32InFrames; i++)
        InputNodes[i]=get_buf(tctx.buf_pool);
#endif

    LOGI("%s() init_buf_pool(Output)", __func__);
    tctx.params.u32Width /= 2;
    tctx.params.u32Height/= 2;
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params, stOutputBufReq.u32MinCnt, VPP_FALSE);
    tctx.params.u32Width *= 2;
    tctx.params.u32Height*= 2;
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    // Queue to input pool
    LOGI("%s() Push buffers loop", __func__);
    LOGI("%s Init Stage. %d input frames, %d output buffers", __func__, u32InFrames, stOutputBufReq.u32MinCnt);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);

    u32InFrames_done =0;
    tctx.u32Flags = 0;
    tctx.buf_pool->u32RdIdx=0;
    tctx.buf_pool->u32WrIdx=0;
    tctx.buf_pool_ext->u32RdIdx=0;
    tctx.buf_pool_ext->u32WrIdx=0;
    tctx.u32InRxCnt = 0;
    tctx.u32OutRxCnt = 0;
    tctx.u32InPutCnt = 0;
    tctx.u32OutPutCnt = 0;
    tctx.u32InHoldCnt = 0;
    tctx.u32OutHoldCnt = 0;

    //Init Stage ----------------------------------
    //Feed Internal FRC Output Buffers
    for (i = 0; i < stOutputBufReq.u32MinCnt; i++)
    {
        pthread_mutex_lock(&tctx.mutex);
        pNode=NULL;
        while (pNode==NULL)
            pNode = get_buf(tctx.buf_pool_ext);
        pthread_mutex_unlock(&tctx.mutex);
        DVP_ASSERT_PTR_NNULL(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;

        //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

        vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc); //For debugging purposes
        pNode->pIntBuf->pBuf->timestamp=0;
        pNode->pIntBuf->pBuf->flags=0;
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    //Feed Stage --------------------------------
    //Feed input and output buffers as they become available.
    LOGI("%s Feed Stage", __func__);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    while ((u32InFrames_done<u32InFrames)||(tctx.u32InRxCnt < u32InFrames)||(tctx.u32OutRxCnt < (u32InFrames)))
    {
        int buffer_found=0;

#ifdef TEST_MBI_ALL_NEW_BUFFERS
//Uncomment this for TEST_MBI_ALL_NEW_BUFFERS
//        if ((u32InFrames_done<u32InFrames))
//        {
//            pthread_mutex_lock(&tctx.mutex);
//            pNode=InputNodes[u32InFrames_done];
#else
        //LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
        if ((tctx.buf_pool->u32ListSz)&&(u32InFrames_done<u32InFrames))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool);
#endif
            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            fill_buf(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            fill_extra_data(pNode, eVppBufType_Progressive, 0);
            stCtxMe.u32ExpExtraLen = pNode->pIntBuf->stExtra.u32FilledLen;

            pNode->pIntBuf->pBuf->timestamp=u32InFrames_done*TIME_DELAY;  //Inc with 33ms every input buffer
            pNode->pIntBuf->pBuf->flags=0;
            if (u32InFrames_done==(u32InFrames-1))
                pNode->pIntBuf->pBuf->flags|=VPP_BUFFER_FLAG_EOS;
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            u32InFrames_done++;
            buffer_found=1;
        }
        if ((tctx.buf_pool_ext->u32ListSz)&&(tctx.u32OutRxCnt < (u32InFrames)))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool_ext);

            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

            pNode->pIntBuf->pBuf->flags=0;
            vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc);  //For debugging purposes
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            buffer_found=1;
        }
        if (!buffer_found)
           sched_yield();
    }

    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_OUTPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    while (!((tctx.u32Flags & INPUT_FLUSH_DONE)&&(tctx.u32Flags & OUTPUT_FLUSH_DONE)))
        sched_yield();

    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32Ret=u32VppIpFrcMe_Close(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    LOGI("freeing buffer pools");
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    free_buf_pool(tctx.buf_pool_ext, VPP_FALSE);
    free_buf_pool(tctx.buf_pool, VPP_TRUE);
}

//10 inputs -> 8 MBIs
TEST(IpMe_EncodeBuffer_MBI_1080p_Downscale_Drain)
{
    uint32_t u32Ret=VPP_OK;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32InFrames, u32InFrames_done;
    t_StVppIpBufReq stInputBufReq;
    t_StVppIpBufReq stOutputBufReq;
    struct vpp_port_param port_param_output;
    char s[MAX_FILE_LEN];

    LOGI("%s() IpMe_EncodeBuffer_MBI", __func__);

    tctx.cb.input_buffer_done = test_me_input_buffer_done;
    tctx.cb.output_buffer_done = test_me_output_buffer_done;
    tctx.cb.event = test_me_event;
    tctx.cb.pv = &tctx;

    tctx.pPrivateCtx = &stCtxMe;
    tctx.u32FramesOut = 10;

    uint32_t frame_width=1920;
    uint32_t frame_height=1080;

    tctx_set_port_params(&tctx, frame_width, frame_height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width=frame_width;
    tctx.params.u32Height=frame_height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cInputPath,(char*)"/data/test/input/", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_1080p.yuv", MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"testmotion_nv12_1080p.yuv", MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cInputName,(char*)"yuvgtmotion_nv12_1080p.yuv", MAX_FILE_SEG_LEN);
    strlcpy((char*)s,(char*)tctx.params.cInputPath, MAX_FILE_SEG_LEN);
    strlcat((char*)s,(char*)tctx.params.cInputName, MAX_FILE_SEG_LEN);
#if FRC_ME_CHECK_FILE
    if (!exists((char*)s))
    {
        DVP_ASSERT_PTR_NNULL(NULL);
        LOGE("Error: %s file does't exist!", (char*)tctx.params.cInputName);
        return;
    }
#endif
    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cOutputPath,(char*)"/data/test/output/",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_1080p.bin",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"testmotion_nv12_1080p_downscale.bin",MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cOutputName,(char*)"yuvgtmotion_nv12_1080p_downscale_Drain.bin",MAX_FILE_SEG_LEN);

    port_param_output.width=frame_width;
    port_param_output.height=frame_height;
    port_param_output.fmt=VPP_COLOR_FORMAT_NV12_VENUS;

    stCtxMe.pme_ctx = vpVppIpFrcMe_Init(&stCtxMe.stCtx, 0, tctx.cb);
    DVP_ASSERT_PTR_NNULL(stCtxMe.pme_ctx);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, port_param_output);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32VppIpFrcMe_GetBufferRequirements(stCtxMe.pme_ctx, &stInputBufReq, &stOutputBufReq);
    u32InFrames =tctx.u32FramesOut;

    LOGI("%s() init_buf_pool(Input)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params, u32InFrames, VPP_TRUE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

#ifdef TEST_MBI_ALL_NEW_BUFFERS
    struct bufnode * InputNodes[tctx.u32FramesOut];
    for (i=0; i<u32InFrames; i++)
        InputNodes[i]=get_buf(tctx.buf_pool);
#endif

    LOGI("%s() init_buf_pool(Output)", __func__);
    tctx.params.u32Width /= 2;
    tctx.params.u32Height/= 2;
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params, stOutputBufReq.u32MinCnt, VPP_FALSE);
    tctx.params.u32Width *= 2;
    tctx.params.u32Height*= 2;
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    // Queue to input pool
    LOGI("%s() Push buffers loop", __func__);
    LOGI("%s Init Stage. %d input frames, %d output buffers", __func__, u32InFrames, stOutputBufReq.u32MinCnt);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);

    u32InFrames_done=0;
    tctx.u32Flags=0;
    tctx.buf_pool->u32RdIdx=0;
    tctx.buf_pool->u32WrIdx=0;
    tctx.buf_pool_ext->u32RdIdx=0;
    tctx.buf_pool_ext->u32WrIdx=0;
    tctx.u32InRxCnt=0;
    tctx.u32OutRxCnt=0;
    tctx.u32InPutCnt=0;
    tctx.u32OutPutCnt=0;
    tctx.u32InHoldCnt=0;
    tctx.u32OutHoldCnt=0;
    int DrainStage=0;
    uint64_t timestamp=0;

    //Init Stage ----------------------------------
    //Feed Internal FRC Output Buffers
    for (i = 0; i < stOutputBufReq.u32MinCnt; i++)
    {
        pthread_mutex_lock(&tctx.mutex);
        pNode=NULL;
        while (pNode==NULL)
            pNode = get_buf(tctx.buf_pool_ext);
        pthread_mutex_unlock(&tctx.mutex);
        DVP_ASSERT_PTR_NNULL(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;

        //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

        vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc); //For debugging purposes
        pNode->pIntBuf->pBuf->flags=0;
        pNode->pIntBuf->pBuf->timestamp=0;
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    //Feed Stage --------------------------------
    //Feed input and output buffers as they become available.
    LOGI("%s Feed Stage", __func__);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    while ((u32InFrames_done<u32InFrames)||(tctx.u32InRxCnt < u32InFrames)||(tctx.u32OutRxCnt < (u32InFrames)))
    {
        int buffer_found=0;

#ifdef TEST_MBI_ALL_NEW_BUFFERS
//Uncomment this for TEST_MBI_ALL_NEW_BUFFERS
//        if ((u32InFrames_done<u32InFrames))
//        {
//            pthread_mutex_lock(&tctx.mutex);
//            pNode=InputNodes[u32InFrames_done];
#else
        if ((tctx.buf_pool->u32ListSz)&&(u32InFrames_done<u32InFrames)&&(!DrainStage))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool);
#endif
            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            fill_buf(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            fill_extra_data(pNode, eVppBufType_Progressive, 0);
            stCtxMe.u32ExpExtraLen = pNode->pIntBuf->stExtra.u32FilledLen;

            pNode->pIntBuf->pBuf->timestamp=timestamp;
            timestamp+=TIME_DELAY;  //Inc with 33ms every input buffer
            pNode->pIntBuf->pBuf->flags=0;
            if (u32InFrames_done==(u32InFrames-1))
                pNode->pIntBuf->pBuf->flags|=VPP_BUFFER_FLAG_EOS;
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            u32InFrames_done++;
            buffer_found=1;
        }

        if ((tctx.buf_pool_ext->u32ListSz))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool_ext);

            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

            pNode->pIntBuf->pBuf->flags=0;
            vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc);  //For debugging purposes
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            buffer_found=1;
        }

        if (DrainStage)
        {
            if (tctx.u32Flags & DRAIN_DONE)
            {
                tctx.u32Flags &= ~DRAIN_DONE;
                DrainStage=0;
            }
        }
        else if (u32InFrames_done==5)
        {
            u32VppIpFrcMe_Drain(stCtxMe.pme_ctx);
            DrainStage=1;
        }

        if (!buffer_found)
           sched_yield();
    }

    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_OUTPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    while (!((tctx.u32Flags & INPUT_FLUSH_DONE)&&(tctx.u32Flags & OUTPUT_FLUSH_DONE)))
        sched_yield();

    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32Ret=u32VppIpFrcMe_Close(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    vVppIpFrcMe_Term(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    LOGI("freeing buffer pools");
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    free_buf_pool(tctx.buf_pool_ext, VPP_FALSE);
    free_buf_pool(tctx.buf_pool, VPP_TRUE);
}

//10 inputs -> 7 MBIs
TEST(IpMe_EncodeBuffer_MBI_1080p_Downscale_Drain1)
{
    uint32_t u32Ret=VPP_OK;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32InFrames, u32InFrames_done;
    t_StVppIpBufReq stInputBufReq;
    t_StVppIpBufReq stOutputBufReq;
    struct vpp_port_param port_param_output;
    char s[MAX_FILE_LEN];

    LOGI("%s() IpMe_EncodeBuffer_MBI", __func__);

    tctx.cb.input_buffer_done = test_me_input_buffer_done;
    tctx.cb.output_buffer_done = test_me_output_buffer_done;
    tctx.cb.event = test_me_event;
    tctx.cb.pv = &tctx;

    tctx.pPrivateCtx = &stCtxMe;
    tctx.u32FramesOut = 10;

    uint32_t frame_width=1920;
    uint32_t frame_height=1080;

    tctx_set_port_params(&tctx, frame_width, frame_height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width=frame_width;
    tctx.params.u32Height=frame_height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cInputPath,(char*)"/data/test/input/",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_1080p.yuv",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"testmotion_nv12_1080p.yuv",MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cInputName,(char*)"yuvgtmotion_nv12_1080p.yuv",MAX_FILE_SEG_LEN);
    strlcpy((char*)s,(char*)tctx.params.cInputPath, MAX_FILE_SEG_LEN);
    strlcat((char*)s,(char*)tctx.params.cInputName, MAX_FILE_SEG_LEN);
#if FRC_ME_CHECK_FILE
    if (!exists((char*)s))
    {
        DVP_ASSERT_PTR_NNULL(NULL);
        LOGE("Error: %s file does't exist!", (char*)tctx.params.cInputName);
        return;
    }
#endif
    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cOutputPath,(char*)"/data/test/output/",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_1080p.bin",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"testmotion_nv12_1080p_downscale.bin",MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cOutputName,(char*)"yuvgtmotion_nv12_1080p_downscale_Drain1.bin",MAX_FILE_SEG_LEN);

    port_param_output.width=frame_width;
    port_param_output.height=frame_height;
    port_param_output.fmt=VPP_COLOR_FORMAT_NV12_VENUS;

    stCtxMe.pme_ctx = vpVppIpFrcMe_Init(&stCtxMe.stCtx, 0, tctx.cb);
    DVP_ASSERT_PTR_NNULL(stCtxMe.pme_ctx);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, port_param_output);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32VppIpFrcMe_GetBufferRequirements(stCtxMe.pme_ctx, &stInputBufReq, &stOutputBufReq);
    u32InFrames =tctx.u32FramesOut;

    LOGI("%s() init_buf_pool(Input)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params, u32InFrames, VPP_TRUE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

#ifdef TEST_MBI_ALL_NEW_BUFFERS
    struct bufnode * InputNodes[tctx.u32FramesOut];
    for (i=0; i<u32InFrames; i++)
        InputNodes[i]=get_buf(tctx.buf_pool);
#endif

    LOGI("%s() init_buf_pool(Output)", __func__);
    tctx.params.u32Width /= 2;
    tctx.params.u32Height/= 2;
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params, stOutputBufReq.u32MinCnt, VPP_FALSE);
    tctx.params.u32Width *= 2;
    tctx.params.u32Height*= 2;
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    // Queue to input pool
    LOGI("%s() Push buffers loop", __func__);
    LOGI("%s Init Stage. %d input frames, %d output buffers", __func__, u32InFrames, stOutputBufReq.u32MinCnt);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);

    u32InFrames_done=0;
    tctx.u32Flags=0;
    tctx.buf_pool->u32RdIdx=0;
    tctx.buf_pool->u32WrIdx=0;
    tctx.buf_pool_ext->u32RdIdx=0;
    tctx.buf_pool_ext->u32WrIdx=0;
    tctx.u32InRxCnt=0;
    tctx.u32OutRxCnt=0;
    tctx.u32InPutCnt=0;
    tctx.u32OutPutCnt=0;
    tctx.u32InHoldCnt=0;
    tctx.u32OutHoldCnt=0;
    int DrainStage=0;
    uint64_t timestamp=0;

    //Init Stage ----------------------------------
    //Feed Internal FRC Output Buffers
    for (i = 0; i < stOutputBufReq.u32MinCnt; i++)
    {
        pthread_mutex_lock(&tctx.mutex);
        pNode=NULL;
        while (pNode==NULL)
            pNode = get_buf(tctx.buf_pool_ext);
        pthread_mutex_unlock(&tctx.mutex);
        DVP_ASSERT_PTR_NNULL(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;

        //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

        vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc); //For debugging purposes
        pNode->pIntBuf->pBuf->flags=0;
        pNode->pIntBuf->pBuf->timestamp=0;
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    //Feed Stage --------------------------------
    //Feed input and output buffers as they become available.
    LOGI("%s Feed Stage", __func__);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    while ((u32InFrames_done<u32InFrames)||(tctx.u32InRxCnt < u32InFrames)||(tctx.u32OutRxCnt < (u32InFrames)))
    {
        int buffer_found=0;

#ifdef TEST_MBI_ALL_NEW_BUFFERS
//Uncomment this for TEST_MBI_ALL_NEW_BUFFERS
//        if ((u32InFrames_done<u32InFrames))
//        {
//            pthread_mutex_lock(&tctx.mutex);
//            pNode=InputNodes[u32InFrames_done];
#else
        if ((tctx.buf_pool->u32ListSz)&&(u32InFrames_done<u32InFrames)&&(!DrainStage))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool);
#endif
            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            fill_buf(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            fill_extra_data(pNode, eVppBufType_Progressive, 0);
            stCtxMe.u32ExpExtraLen = pNode->pIntBuf->stExtra.u32FilledLen;

            pNode->pIntBuf->pBuf->timestamp=timestamp;
            timestamp+=TIME_DELAY;  //Inc with 33ms every input buffer
            pNode->pIntBuf->pBuf->flags=0;
            if ((u32InFrames_done==(u32InFrames-1))||(u32InFrames_done==4))
                pNode->pIntBuf->pBuf->flags|=VPP_BUFFER_FLAG_EOS;
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            u32InFrames_done++;
            buffer_found=1;
        }

        if ((tctx.buf_pool_ext->u32ListSz))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool_ext);

            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

            pNode->pIntBuf->pBuf->flags=0;
            vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc);  //For debugging purposes
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            buffer_found=1;
        }

        if (DrainStage)
        {
            if (tctx.u32Flags & DRAIN_DONE)
            {
                tctx.u32Flags &= ~DRAIN_DONE;
                DrainStage=0;
            }
        }
        else if ((u32InFrames_done==5)||(u32InFrames_done==8))
        {
            u32VppIpFrcMe_Drain(stCtxMe.pme_ctx);
            DrainStage=1;
        }

        if (!buffer_found)
           sched_yield();
    }

    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_OUTPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    while (!((tctx.u32Flags & INPUT_FLUSH_DONE)&&(tctx.u32Flags & OUTPUT_FLUSH_DONE)))
        sched_yield();

    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32Ret=u32VppIpFrcMe_Close(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    vVppIpFrcMe_Term(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    LOGI("freeing buffer pools");
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    free_buf_pool(tctx.buf_pool_ext, VPP_FALSE);
    free_buf_pool(tctx.buf_pool, VPP_TRUE);
}

//10 input buffers -> 3 MBIs
TEST(IpMe_EncodeBuffer_MBI_1080p_Downscale_Bypass)
{
    uint32_t u32Ret=VPP_OK;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32InFrames, u32InFrames_done;
    t_StVppIpBufReq stInputBufReq;
    t_StVppIpBufReq stOutputBufReq;
    struct vpp_port_param port_param_output;
    char s[MAX_FILE_LEN];

    LOGI("%s() IpMe_EncodeBuffer_MBI", __func__);

    tctx.cb.input_buffer_done = test_me_input_buffer_done;
    tctx.cb.output_buffer_done = test_me_output_buffer_done;
    tctx.cb.event = test_me_event;
    tctx.cb.pv = &tctx;

    tctx.pPrivateCtx = &stCtxMe;
    tctx.u32FramesOut = 10;

    uint32_t frame_width=1920;
    uint32_t frame_height=1080;

    tctx_set_port_params(&tctx, frame_width, frame_height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width=frame_width;
    tctx.params.u32Height=frame_height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cInputPath,(char*)"/data/test/input/",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_1080p.yuv",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"testmotion_nv12_1080p.yuv",MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cInputName,(char*)"yuvgtmotion_nv12_1080p.yuv",MAX_FILE_SEG_LEN);
    strlcpy((char*)s,(char*)tctx.params.cInputPath, MAX_FILE_SEG_LEN);
    strlcat((char*)s,(char*)tctx.params.cInputName, MAX_FILE_SEG_LEN);
#if FRC_ME_CHECK_FILE
    if (!exists((char*)s))
    {
        DVP_ASSERT_PTR_NNULL(NULL);
        LOGE("Error: %s file does't exist!", (char*)tctx.params.cInputName);
        return;
    }
#endif
    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cOutputPath,(char*)"/data/test/output/",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_1080p.bin",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"testmotion_nv12_1080p_downscale.bin",MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cOutputName,(char*)"yuvgtmotion_nv12_1080p_downscale_Bypass.bin",MAX_FILE_SEG_LEN);

    port_param_output.width=frame_width;
    port_param_output.height=frame_height;
    port_param_output.fmt=VPP_COLOR_FORMAT_NV12_VENUS;

    stCtxMe.pme_ctx = vpVppIpFrcMe_Init(&stCtxMe.stCtx, 0, tctx.cb);
    DVP_ASSERT_PTR_NNULL(stCtxMe.pme_ctx);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, port_param_output);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32VppIpFrcMe_GetBufferRequirements(stCtxMe.pme_ctx, &stInputBufReq, &stOutputBufReq);
    u32InFrames =tctx.u32FramesOut;

    LOGI("%s() init_buf_pool(Input)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params, u32InFrames, VPP_TRUE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

#ifdef TEST_MBI_ALL_NEW_BUFFERS
    struct bufnode * InputNodes[tctx.u32FramesOut];
    for (i=0; i<u32InFrames; i++)
        InputNodes[i]=get_buf(tctx.buf_pool);
#endif

    LOGI("%s() init_buf_pool(Output)", __func__);
    tctx.params.u32Width /= 2;
    tctx.params.u32Height/= 2;
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params, stOutputBufReq.u32MinCnt, VPP_FALSE);
    tctx.params.u32Width *= 2;
    tctx.params.u32Height*= 2;
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    // Queue to input pool
    LOGI("%s() Push buffers loop", __func__);
    LOGI("%s Init Stage. %d input frames, %d output buffers", __func__, u32InFrames, stOutputBufReq.u32MinCnt);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);

    u32InFrames_done=0;
    tctx.u32Flags=0;
    tctx.buf_pool->u32RdIdx=0;
    tctx.buf_pool->u32WrIdx=0;
    tctx.buf_pool_ext->u32RdIdx=0;
    tctx.buf_pool_ext->u32WrIdx=0;
    tctx.u32InRxCnt=0;
    tctx.u32OutRxCnt=0;
    tctx.u32InPutCnt=0;
    tctx.u32OutPutCnt=0;
    tctx.u32InHoldCnt=0;
    tctx.u32OutHoldCnt=0;
    uint64_t timestamp=0;

    //Init Stage ----------------------------------
    //Feed Internal FRC Output Buffers
    for (i = 0; i < stOutputBufReq.u32MinCnt; i++)
    {
        pthread_mutex_lock(&tctx.mutex);
        pNode=NULL;
        while (pNode==NULL)
            pNode = get_buf(tctx.buf_pool_ext);
        pthread_mutex_unlock(&tctx.mutex);
        DVP_ASSERT_PTR_NNULL(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;

        //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

        vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc); //For debugging purposes
        pNode->pIntBuf->pBuf->flags=0;
        pNode->pIntBuf->pBuf->timestamp=0;
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    //Feed Stage --------------------------------
    //Feed input and output buffers as they become available.
    LOGI("%s Feed Stage", __func__);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    while ((u32InFrames_done<u32InFrames)||(tctx.u32InRxCnt < u32InFrames)||(tctx.u32OutRxCnt < u32InFrames))
    {
        int buffer_found=0;

#ifdef TEST_MBI_ALL_NEW_BUFFERS
//Uncomment this for TEST_MBI_ALL_NEW_BUFFERS
//        if ((u32InFrames_done<u32InFrames))
//        {
//            pthread_mutex_lock(&tctx.mutex);
//            pNode=InputNodes[u32InFrames_done];
#else
        if ((tctx.buf_pool->u32ListSz)&&(u32InFrames_done<u32InFrames))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool);
#endif
            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            fill_buf(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            fill_extra_data(pNode, eVppBufType_Progressive, 0);
            stCtxMe.u32ExpExtraLen = pNode->pIntBuf->stExtra.u32FilledLen;

            pNode->pIntBuf->pBuf->flags=0;
            pNode->pIntBuf->pBuf->timestamp=timestamp;
            timestamp+=TIME_DELAY;  //Inc with 33ms every input buffer
            if ((u32InFrames_done<=1)||((u32InFrames_done>=3)&&(u32InFrames_done<=4))||(u32InFrames_done==9))
            {
                pNode->pIntBuf->u32InternalFlags |= VPP_BUF_FLAG_BYPASS;
            }
            else
            {
                pNode->pIntBuf->u32InternalFlags = 0;
            }

            if (u32InFrames_done==(u32InFrames-1))
                pNode->pIntBuf->pBuf->flags|=VPP_BUFFER_FLAG_EOS;

            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            u32InFrames_done++;
            buffer_found=1;
        }

        if ((tctx.buf_pool_ext->u32ListSz))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool_ext);

            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

            pNode->pIntBuf->pBuf->flags=0;
            vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc);  //For debugging purposes
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            buffer_found=1;
        }

        if (!buffer_found)
           sched_yield();
    }

    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_OUTPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    while (!((tctx.u32Flags & INPUT_FLUSH_DONE)&&(tctx.u32Flags & OUTPUT_FLUSH_DONE)))
        sched_yield();

    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32Ret=u32VppIpFrcMe_Close(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    vVppIpFrcMe_Term(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    LOGI("freeing buffer pools");
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    free_buf_pool(tctx.buf_pool_ext, VPP_FALSE);
    free_buf_pool(tctx.buf_pool, VPP_TRUE);
}

//10 input buffers -> 3 MBIs
TEST(IpMe_EncodeBuffer_MBI_1080p_Downscale_Inter)
{
    uint32_t u32Ret=VPP_OK;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32InFrames, u32InFrames_done;
    t_StVppIpBufReq stInputBufReq;
    t_StVppIpBufReq stOutputBufReq;
    struct vpp_port_param port_param_output;
    char s[MAX_FILE_LEN];

    LOGI("%s() IpMe_EncodeBuffer_MBI", __func__);

    tctx.cb.input_buffer_done = test_me_input_buffer_done;
    tctx.cb.output_buffer_done = test_me_output_buffer_done;
    tctx.cb.event = test_me_event;
    tctx.cb.pv = &tctx;

    tctx.pPrivateCtx = &stCtxMe;
    tctx.u32FramesOut = 10;

    uint32_t frame_width=1920;
    uint32_t frame_height=1080;

    tctx_set_port_params(&tctx, frame_width, frame_height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width=frame_width;
    tctx.params.u32Height=frame_height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cInputPath,(char*)"/data/test/input/",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_1080p.yuv",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"testmotion_nv12_1080p.yuv",MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cInputName,(char*)"yuvgtmotion_nv12_1080p.yuv",MAX_FILE_SEG_LEN);
    strlcpy((char*)s,(char*)tctx.params.cInputPath, MAX_FILE_SEG_LEN);
    strlcat((char*)s,(char*)tctx.params.cInputName, MAX_FILE_SEG_LEN);
#if FRC_ME_CHECK_FILE
    if (!exists((char*)s))
    {
        DVP_ASSERT_PTR_NNULL(NULL);
        LOGE("Error: %s file does't exist!", (char*)tctx.params.cInputName);
        return;
    }
#endif
    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cOutputPath,(char*)"/data/test/output/",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_1080p.bin",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"testmotion_nv12_1080p_downscale.bin",MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cOutputName,(char*)"yuvgtmotion_nv12_1080p_downscale_Inter.bin",MAX_FILE_SEG_LEN);

    port_param_output.width=frame_width;
    port_param_output.height=frame_height;
    port_param_output.fmt=VPP_COLOR_FORMAT_NV12_VENUS;

    stCtxMe.pme_ctx = vpVppIpFrcMe_Init(&stCtxMe.stCtx, 0, tctx.cb);
    DVP_ASSERT_PTR_NNULL(stCtxMe.pme_ctx);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, port_param_output);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32VppIpFrcMe_GetBufferRequirements(stCtxMe.pme_ctx, &stInputBufReq, &stOutputBufReq);
    u32InFrames =tctx.u32FramesOut;

    LOGI("%s() init_buf_pool(Input)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params, u32InFrames, VPP_TRUE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

#ifdef TEST_MBI_ALL_NEW_BUFFERS
    struct bufnode * InputNodes[tctx.u32FramesOut];
    for (i=0; i<u32InFrames; i++)
        InputNodes[i]=get_buf(tctx.buf_pool);
#endif

    LOGI("%s() init_buf_pool(Output)", __func__);
    tctx.params.u32Width /= 2;
    tctx.params.u32Height/= 2;
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params, stOutputBufReq.u32MinCnt, VPP_FALSE);
    tctx.params.u32Width *= 2;
    tctx.params.u32Height*= 2;
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    // Queue to input pool
    LOGI("%s() Push buffers loop", __func__);
    LOGI("%s Init Stage. %d input frames, %d output buffers", __func__, u32InFrames, stOutputBufReq.u32MinCnt);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);

    u32InFrames_done=0;
    tctx.u32Flags=0;
    tctx.buf_pool->u32RdIdx=0;
    tctx.buf_pool->u32WrIdx=0;
    tctx.buf_pool_ext->u32RdIdx=0;
    tctx.buf_pool_ext->u32WrIdx=0;
    tctx.u32InRxCnt=0;
    tctx.u32OutRxCnt=0;
    tctx.u32InPutCnt=0;
    tctx.u32OutPutCnt=0;
    tctx.u32InHoldCnt=0;
    tctx.u32OutHoldCnt=0;
    uint64_t timestamp=0;

    //Init Stage ----------------------------------
    //Feed Internal FRC Output Buffers
    for (i = 0; i < stOutputBufReq.u32MinCnt; i++)
    {
        pthread_mutex_lock(&tctx.mutex);
        pNode=NULL;
        while (pNode==NULL)
            pNode = get_buf(tctx.buf_pool_ext);
        pthread_mutex_unlock(&tctx.mutex);
        DVP_ASSERT_PTR_NNULL(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;

        //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

        vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc); //For debugging purposes
        pNode->pIntBuf->pBuf->flags=0;
        pNode->pIntBuf->pBuf->timestamp=0;
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    //Feed Stage --------------------------------
    //Feed input and output buffers as they become available.
    LOGI("%s Feed Stage", __func__);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    while ((u32InFrames_done<u32InFrames)||(tctx.u32InRxCnt < u32InFrames)||(tctx.u32OutRxCnt < u32InFrames))
    {
        int buffer_found=0;

#ifdef TEST_MBI_ALL_NEW_BUFFERS
//Uncomment this for TEST_MBI_ALL_NEW_BUFFERS
//        if ((u32InFrames_done<u32InFrames))
//        {
//            pthread_mutex_lock(&tctx.mutex);
//            pNode=InputNodes[u32InFrames_done];
#else
        if ((tctx.buf_pool->u32ListSz)&&(u32InFrames_done<u32InFrames))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool);
#endif
            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            fill_buf(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            fill_extra_data(pNode, eVppBufType_Progressive, 0);
            stCtxMe.u32ExpExtraLen = pNode->pIntBuf->stExtra.u32FilledLen;

            pNode->pIntBuf->pBuf->flags=0;
            pNode->pIntBuf->pBuf->timestamp=timestamp;
            timestamp+=TIME_DELAY;  //Inc with 33ms every input buffer
            if ((u32InFrames_done<=1)||((u32InFrames_done>=3)&&(u32InFrames_done<=4))||(u32InFrames_done==9))
            {
                pNode->pIntBuf->eBufType = eVppBufType_Interleaved_TFF;
            }
            else
            {
                pNode->pIntBuf->eBufType = eVppBufType_Progressive;
            }

            if (u32InFrames_done==(u32InFrames-1))
                pNode->pIntBuf->pBuf->flags|=VPP_BUFFER_FLAG_EOS;

            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            u32InFrames_done++;
            buffer_found=1;
        }

        if ((tctx.buf_pool_ext->u32ListSz))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool_ext);

            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

            pNode->pIntBuf->pBuf->flags=0;
            vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc);  //For debugging purposes
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            buffer_found=1;
        }

        if (!buffer_found)
           sched_yield();
    }

    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_OUTPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    while (!((tctx.u32Flags & INPUT_FLUSH_DONE)&&(tctx.u32Flags & OUTPUT_FLUSH_DONE)))
        sched_yield();

    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32Ret=u32VppIpFrcMe_Close(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    vVppIpFrcMe_Term(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    LOGI("freeing buffer pools");
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    free_buf_pool(tctx.buf_pool_ext, VPP_FALSE);
    free_buf_pool(tctx.buf_pool, VPP_TRUE);
}

TEST(IpMe_EncodeBuffer_MBI_1080p_Downscale_Flush_after_EOS)
{
    uint32_t u32Ret=VPP_OK;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32InFrames, u32InFrames_done;
    t_StVppIpBufReq stInputBufReq;
    t_StVppIpBufReq stOutputBufReq;
    struct vpp_port_param port_param_output;
    char s[MAX_FILE_LEN];

    LOGI("%s() IpMe_EncodeBuffer_MBI", __func__);

    tctx.cb.input_buffer_done = test_me_input_buffer_done;
    tctx.cb.output_buffer_done = test_me_output_buffer_done;
    tctx.cb.event = test_me_event;
    tctx.cb.pv = &tctx;

    tctx.pPrivateCtx = &stCtxMe;
    tctx.u32FramesOut = 10;

    uint32_t frame_width=1920;
    uint32_t frame_height=1080;

    tctx_set_port_params(&tctx, frame_width, frame_height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width=frame_width;
    tctx.params.u32Height=frame_height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cInputPath,(char*)"/data/test/input/",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_1080p.yuv",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"testmotion_nv12_1080p.yuv",MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cInputName,(char*)"yuvgtmotion_nv12_1080p.yuv",MAX_FILE_SEG_LEN);
    strlcpy((char*)s,(char*)tctx.params.cInputPath, MAX_FILE_SEG_LEN);
    strlcat((char*)s,(char*)tctx.params.cInputName, MAX_FILE_SEG_LEN);
#if FRC_ME_CHECK_FILE
    if (!exists((char*)s))
    {
        DVP_ASSERT_PTR_NNULL(NULL);
        LOGE("Error: %s file does't exist!", (char*)tctx.params.cInputName);
        return;
    }
#endif

    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cOutputPath,(char*)"/data/test/output/",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_1080p.bin",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"testmotion_nv12_1080p_downscale.bin",MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cOutputName,(char*)"yuvgtmotion_nv12_1080p_downscale_after_EOS.bin",MAX_FILE_SEG_LEN);

    port_param_output.width=frame_width;
    port_param_output.height=frame_height;
    port_param_output.fmt=VPP_COLOR_FORMAT_NV12_VENUS;

    stCtxMe.pme_ctx = vpVppIpFrcMe_Init(&stCtxMe.stCtx, 0, tctx.cb);
    DVP_ASSERT_PTR_NNULL(stCtxMe.pme_ctx);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, port_param_output);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32VppIpFrcMe_GetBufferRequirements(stCtxMe.pme_ctx, &stInputBufReq, &stOutputBufReq);
    u32InFrames =tctx.u32FramesOut;

    LOGI("%s() init_buf_pool(Input)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params, u32InFrames, VPP_TRUE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

#ifdef TEST_MBI_ALL_NEW_BUFFERS
    struct bufnode * InputNodes[tctx.u32FramesOut];
    for (i=0; i<u32InFrames; i++)
        InputNodes[i]=get_buf(tctx.buf_pool);
#endif

    LOGI("%s() init_buf_pool(Output)", __func__);
    tctx.params.u32Width /= 2;
    tctx.params.u32Height/= 2;
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params, stOutputBufReq.u32MinCnt, VPP_FALSE);
    tctx.params.u32Width *= 2;
    tctx.params.u32Height*= 2;
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    // Queue to input pool
    LOGI("%s() Push buffers loop", __func__);
    LOGI("%s Init Stage. %d input frames, %d output buffers", __func__, u32InFrames, stOutputBufReq.u32MinCnt);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);

    u32InFrames_done =0;
    tctx.u32Flags = 0;
    tctx.buf_pool->u32RdIdx=0;
    tctx.buf_pool->u32WrIdx=0;
    tctx.buf_pool_ext->u32RdIdx=0;
    tctx.buf_pool_ext->u32WrIdx=0;
    tctx.u32InRxCnt = 0;
    tctx.u32OutRxCnt = 0;
    tctx.u32InPutCnt = 0;
    tctx.u32OutPutCnt = 0;
    tctx.u32InHoldCnt = 0;
    tctx.u32OutHoldCnt = 0;
    //Init Stage ----------------------------------
    //Feed Internal FRC Output Buffers
    for (i = 0; i < stOutputBufReq.u32MinCnt; i++)
    {
        pthread_mutex_lock(&tctx.mutex);
        pNode=NULL;
        while (pNode==NULL)
            pNode = get_buf(tctx.buf_pool_ext);
        pthread_mutex_unlock(&tctx.mutex);
        DVP_ASSERT_PTR_NNULL(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;

        //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

        vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc); //For debugging purposes
        pNode->pIntBuf->pBuf->timestamp=0;
        pNode->pIntBuf->pBuf->flags=0;
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    //Feed Stage --------------------------------
    //Feed input and output buffers as they become available.
    LOGI("%s Feed Stage", __func__);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    for (i=0; i<u32InFrames; i++)
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool);
            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            fill_buf(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            fill_extra_data(pNode, eVppBufType_Progressive, 0);
            stCtxMe.u32ExpExtraLen = pNode->pIntBuf->stExtra.u32FilledLen;

            pNode->pIntBuf->pBuf->timestamp=u32InFrames_done*TIME_DELAY;  //Inc with 33ms every input buffer
            pNode->pIntBuf->pBuf->flags=0;
            if (u32InFrames_done==(u32InFrames-1))
                pNode->pIntBuf->pBuf->flags|=VPP_BUFFER_FLAG_EOS;
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            u32InFrames_done++;
        }

    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    while (!((tctx.u32Flags & INPUT_FLUSH_DONE)))
        sched_yield();

    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32Ret=u32VppIpFrcMe_Close(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    vVppIpFrcMe_Term(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    LOGI("freeing buffer pools");
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    free_buf_pool(tctx.buf_pool_ext, VPP_FALSE);
    free_buf_pool(tctx.buf_pool, VPP_TRUE);
}

TEST(IpMe_EncodeBuffer_MBI_1080p_Downscale_Flush_no_EOS)
{
    uint32_t u32Ret=VPP_OK;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32InFrames, u32InFrames_done;
    t_StVppIpBufReq stInputBufReq;
    t_StVppIpBufReq stOutputBufReq;
    struct vpp_port_param port_param_output;
    char s[MAX_FILE_LEN];

    LOGI("%s() IpMe_EncodeBuffer_MBI", __func__);

    tctx.cb.input_buffer_done = test_me_input_buffer_done;
    tctx.cb.output_buffer_done = test_me_output_buffer_done;
    tctx.cb.event = test_me_event;
    tctx.cb.pv = &tctx;

    tctx.pPrivateCtx = &stCtxMe;
    tctx.u32FramesOut = 10;

    uint32_t frame_width=1920;
    uint32_t frame_height=1080;

    tctx_set_port_params(&tctx, frame_width, frame_height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width=frame_width;
    tctx.params.u32Height=frame_height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cInputPath,(char*)"/data/test/input/",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_1080p.yuv",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"testmotion_nv12_1080p.yuv",MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cInputName,(char*)"yuvgtmotion_nv12_1080p.yuv",MAX_FILE_SEG_LEN);
    strlcpy((char*)s,(char*)tctx.params.cInputPath, MAX_FILE_SEG_LEN);
    strlcat((char*)s,(char*)tctx.params.cInputName, MAX_FILE_SEG_LEN);
#if FRC_ME_CHECK_FILE
    if (!exists((char*)s))
    {
        DVP_ASSERT_PTR_NNULL(NULL);
        LOGE("Error: %s file does't exist!", (char*)tctx.params.cInputName);
        return;
    }
#endif

    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cOutputPath,(char*)"/data/test/output/",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_1080p.bin",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"testmotion_nv12_1080p_downscale.bin",MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cOutputName,(char*)"yuvgtmotion_nv12_1080p_downscale_no_EOS.bin",MAX_FILE_SEG_LEN);

    port_param_output.width=frame_width;
    port_param_output.height=frame_height;
    port_param_output.fmt=VPP_COLOR_FORMAT_NV12_VENUS;

    stCtxMe.pme_ctx = vpVppIpFrcMe_Init(&stCtxMe.stCtx, 0, tctx.cb);
    DVP_ASSERT_PTR_NNULL(stCtxMe.pme_ctx);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, port_param_output);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32VppIpFrcMe_GetBufferRequirements(stCtxMe.pme_ctx, &stInputBufReq, &stOutputBufReq);
    u32InFrames =tctx.u32FramesOut;

    LOGI("%s() init_buf_pool(Input)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params, u32InFrames, VPP_TRUE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

#ifdef TEST_MBI_ALL_NEW_BUFFERS
    struct bufnode * InputNodes[tctx.u32FramesOut];
    for (i=0; i<u32InFrames; i++)
        InputNodes[i]=get_buf(tctx.buf_pool);
#endif

    LOGI("%s() init_buf_pool(Output)", __func__);
    tctx.params.u32Width /= 2;
    tctx.params.u32Height/= 2;
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params, stOutputBufReq.u32MinCnt, VPP_FALSE);
    tctx.params.u32Width *= 2;
    tctx.params.u32Height*= 2;
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    // Queue to input pool
    LOGI("%s() Push buffers loop", __func__);
    LOGI("%s Init Stage. %d input frames, %d output buffers", __func__, u32InFrames, stOutputBufReq.u32MinCnt);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);

    u32InFrames_done =0;
    tctx.u32Flags = 0;
    tctx.buf_pool->u32RdIdx=0;
    tctx.buf_pool->u32WrIdx=0;
    tctx.buf_pool_ext->u32RdIdx=0;
    tctx.buf_pool_ext->u32WrIdx=0;
    tctx.u32InRxCnt = 0;
    tctx.u32OutRxCnt = 0;
    tctx.u32InPutCnt = 0;
    tctx.u32OutPutCnt = 0;
    tctx.u32InHoldCnt = 0;
    tctx.u32OutHoldCnt = 0;
    //Init Stage ----------------------------------
    //Feed Internal FRC Output Buffers
    for (i = 0; i < stOutputBufReq.u32MinCnt; i++)
    {
        pthread_mutex_lock(&tctx.mutex);
        pNode=NULL;
        while (pNode==NULL)
            pNode = get_buf(tctx.buf_pool_ext);
        pthread_mutex_unlock(&tctx.mutex);
        DVP_ASSERT_PTR_NNULL(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;

        //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

        vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc); //For debugging purposes
        pNode->pIntBuf->pBuf->timestamp=0;
        pNode->pIntBuf->pBuf->flags=0;
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    //Feed Stage --------------------------------
    //Feed input and output buffers as they become available.
    LOGI("%s Feed Stage", __func__);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    for (i=0; i<u32InFrames; i++)
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool);
            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            fill_buf(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            fill_extra_data(pNode, eVppBufType_Progressive, 0);
            stCtxMe.u32ExpExtraLen = pNode->pIntBuf->stExtra.u32FilledLen;

            pNode->pIntBuf->pBuf->timestamp=u32InFrames_done*TIME_DELAY;  //Inc with 33ms every input buffer
            pNode->pIntBuf->pBuf->flags=0;
            //if (u32InFrames_done==(u32InFrames-1))
            //    pNode->pIntBuf->pBuf->flags|=VPP_BUFFER_FLAG_EOS;
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            u32InFrames_done++;
        }

    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    while (!((tctx.u32Flags & INPUT_FLUSH_DONE)))
        sched_yield();

    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32Ret=u32VppIpFrcMe_Close(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    vVppIpFrcMe_Term(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    LOGI("freeing buffer pools");
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    free_buf_pool(tctx.buf_pool_ext, VPP_FALSE);
    free_buf_pool(tctx.buf_pool, VPP_TRUE);
}

TEST(__unused IpMe_EncodeBuffer_MBI_1080p_Downscale_Seed)
{
    uint32_t u32Ret=VPP_OK;
    uint32_t i;
    struct bufnode *pNode;
    uint32_t u32InFrames, u32InFrames_done;
    t_StVppIpBufReq stInputBufReq;
    t_StVppIpBufReq stOutputBufReq;
    struct vpp_port_param port_param_output;
    char s[MAX_FILE_LEN];
    LOGI("%s() IpMe_EncodeBuffer_MBI", __func__);

    int seed = time(NULL);
    LOGI("Seed=%d\n",seed);

    tctx.cb.input_buffer_done = test_me_input_buffer_done;
    tctx.cb.output_buffer_done = test_me_output_buffer_done;
    tctx.cb.event = test_me_event;
    tctx.cb.pv = &tctx;

    tctx.pPrivateCtx = &stCtxMe;
    tctx.u32FramesOut = 100;

    uint32_t frame_width=1920;
    uint32_t frame_height=1080;

    tctx_set_port_params(&tctx, frame_width, frame_height, VPP_COLOR_FORMAT_NV12_VENUS);
    tctx.params.u32Width=frame_width;
    tctx.params.u32Height=frame_height;
    tctx.params.eBufferType=eVppBufType_Progressive;

    tctx.params.eInputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eInputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eInputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cInputPath,(char*)"/data/test/input/",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"rollercoaster_compression_1080p.yuv",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cInputName,(char*)"testmotion_nv12_1080p.yuv",MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cInputName,(char*)"yuvgtmotion_nv12_1080p.yuv",MAX_FILE_SEG_LEN);
    strlcpy((char*)s,(char*)tctx.params.cInputPath, MAX_FILE_SEG_LEN);
    strlcat((char*)s,(char*)tctx.params.cInputName, MAX_FILE_SEG_LEN);
#if FRC_ME_CHECK_FILE
    if (!exists((char*)s))
    {
        DVP_ASSERT_PTR_NNULL(NULL);
        LOGE("Error: %s file does't exist!", (char*)tctx.params.cInputName);
        return;
    }
#endif
    tctx.params.eOutputFileFormat=FILE_FORMAT_NV12;
    tctx.params.eOutputFileType=FILE_TYPE_MULTI_FRAMES;
    tctx.params.eOutputBufFmt=VPP_COLOR_FORMAT_NV12_VENUS;
    strlcpy((char*)tctx.params.cOutputPath,(char*)"/data/test/output/",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"rollercoaster_compression_1080p.bin",MAX_FILE_SEG_LEN);
    //strlcpy((char*)tctx.params.cOutputName,(char*)"testmotion_nv12_1080p_downscale.bin",MAX_FILE_SEG_LEN);
    strlcpy((char*)tctx.params.cOutputName,(char*)"yuvgtmotion_nv12_1080p_downscale_Seed.bin",MAX_FILE_SEG_LEN);

    port_param_output.width=frame_width;
    port_param_output.height=frame_height;
    port_param_output.fmt=VPP_COLOR_FORMAT_NV12_VENUS;

    stCtxMe.pme_ctx = vpVppIpFrcMe_Init(&stCtxMe.stCtx, 0, tctx.cb);
    DVP_ASSERT_PTR_NNULL(stCtxMe.pme_ctx);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_INPUT, tctx.port_param);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_SetParam(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, port_param_output);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32Ret = u32VppIpFrcMe_Open(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    u32VppIpFrcMe_GetBufferRequirements(stCtxMe.pme_ctx, &stInputBufReq, &stOutputBufReq);
    u32InFrames =tctx.u32FramesOut;

    LOGI("%s() init_buf_pool(Input)", __func__);
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool, &tctx.params, u32InFrames, VPP_TRUE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

#ifdef TEST_MBI_ALL_NEW_BUFFERS
    struct bufnode * InputNodes[tctx.u32FramesOut];
    for (i=0; i<u32InFrames; i++)
        InputNodes[i]=get_buf(tctx.buf_pool);
#endif

    LOGI("%s() init_buf_pool(Output)", __func__);
    tctx.params.u32Width /= 2;
    tctx.params.u32Height/= 2;
    u32Ret = init_buf_pool(&tctx.stVppCtx, &tctx.buf_pool_ext, &tctx.params, stOutputBufReq.u32MinCnt, VPP_FALSE);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    tctx.params.u32Width *= 2;
    tctx.params.u32Height*= 2;

    // Queue to input pool
    LOGI("%s() Push buffers loop", __func__);
    LOGI("%s Init Stage. %d input frames, %d output buffers", __func__, u32InFrames, stOutputBufReq.u32MinCnt);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);

    u32InFrames_done=0;
    tctx.u32Flags=0;
    tctx.buf_pool->u32RdIdx=0;
    tctx.buf_pool->u32WrIdx=0;
    tctx.buf_pool_ext->u32RdIdx=0;
    tctx.buf_pool_ext->u32WrIdx=0;
    tctx.u32InRxCnt=0;
    tctx.u32OutRxCnt=0;
    tctx.u32InPutCnt=0;
    tctx.u32OutPutCnt=0;
    tctx.u32InHoldCnt=0;
    tctx.u32OutHoldCnt=0;
    uint64_t timestamp=0;

    //Init Stage ----------------------------------
    //Feed Internal FRC Output Buffers
    for (i = 0; i < stOutputBufReq.u32MinCnt; i++)
    {
        pthread_mutex_lock(&tctx.mutex);
        pNode=NULL;
        while (pNode==NULL)
            pNode = get_buf(tctx.buf_pool_ext);
        pthread_mutex_unlock(&tctx.mutex);
        DVP_ASSERT_PTR_NNULL(pNode);

        pNode->owner = BUF_OWNER_LIBRARY;

        //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

        vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc); //For debugging purposes
        pNode->pIntBuf->pBuf->flags=0;
        pNode->pIntBuf->pBuf->timestamp=0;
        u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
        DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    }

    //Feed Stage --------------------------------
    //Feed input and output buffers as they become available.
    LOGI("%s Feed Stage", __func__);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    while ((u32InFrames_done<u32InFrames)||(tctx.u32InRxCnt < u32InFrames)||(tctx.u32OutRxCnt < u32InFrames))
    {
        int buffer_found=0;
        int random=rand();

#ifdef TEST_MBI_ALL_NEW_BUFFERS
//Uncomment this for TEST_MBI_ALL_NEW_BUFFERS
//        if ((u32InFrames_done<u32InFrames))
//        {
//            pthread_mutex_lock(&tctx.mutex);
//            pNode=InputNodes[u32InFrames_done];
#else
        if ((tctx.buf_pool->u32ListSz)&&(u32InFrames_done<u32InFrames))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool);
#endif
            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            fill_buf(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            fill_extra_data(pNode, eVppBufType_Progressive, 0);
            stCtxMe.u32ExpExtraLen = pNode->pIntBuf->stExtra.u32FilledLen;

            pNode->pIntBuf->pBuf->flags=0;
            pNode->pIntBuf->pBuf->timestamp=timestamp;
            timestamp+=TIME_DELAY;  //Inc with 33ms every input buffer
            pNode->pIntBuf->eBufType = eVppBufType_Progressive;
            pNode->pIntBuf->u32InternalFlags=0;

            if ((!(random%3))||(!(random%4)))
            {
                pNode->pIntBuf->u32InternalFlags |= VPP_BUF_FLAG_BYPASS;
            }
            else if ((!(random%5))||((!(random%7))||(!(random%11))))
            {
                pNode->pIntBuf->eBufType = eVppBufType_Interleaved_TFF;
            }

            if (u32InFrames_done==(u32InFrames-1))
                pNode->pIntBuf->pBuf->flags|=VPP_BUFFER_FLAG_EOS;

            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_INPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            u32InFrames_done++;
            buffer_found=1;
        }

        if ((tctx.buf_pool_ext->u32ListSz))
        {
            pthread_mutex_lock(&tctx.mutex);
            pNode=NULL;
            while (pNode==NULL)
               pNode = get_buf(tctx.buf_pool_ext);

            pthread_mutex_unlock(&tctx.mutex);
            DVP_ASSERT_PTR_NNULL(pNode);

            pNode->owner = BUF_OWNER_LIBRARY;

            //DVP_ASSERT_FALSE(tctx.u32Flags & DRAIN_DONE);

            pNode->pIntBuf->pBuf->flags=0;
            vVppBuf_CleanExtradata(pNode->pIntBuf, 0xcc);  //For debugging purposes
            u32Ret = u32VppIpFrcMe_QueueBuf(stCtxMe.pme_ctx, VPP_PORT_OUTPUT, pNode->pIntBuf);
            DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
            buffer_found=1;
        }

        if (!buffer_found)
           sched_yield();
    }

    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_INPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    u32Ret=u32VppIpFrcMe_Flush(stCtxMe.pme_ctx, VPP_PORT_OUTPUT);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    while (!((tctx.u32Flags & INPUT_FLUSH_DONE)&&(tctx.u32Flags & OUTPUT_FLUSH_DONE)))
        sched_yield();

    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    u32Ret=u32VppIpFrcMe_Close(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    vVppIpFrcMe_Term(stCtxMe.pme_ctx);
    DVP_ASSERT_EQUAL(u32Ret, VPP_OK);

    LOGI("freeing buffer pools");
    LOGI("buf_pool_size=%d, buf_pool_ext_size=%d",tctx.buf_pool->u32ListSz, tctx.buf_pool_ext->u32ListSz);
    free_buf_pool(tctx.buf_pool_ext, VPP_FALSE);
    free_buf_pool(tctx.buf_pool, VPP_TRUE);
}



/************************************************************************
 * Global Functions
 ***********************************************************************/
TEST_CASES IpMeTests[] = {
//    TEST_CASE(IpMe_EncodeBuffer_Null),
//    TEST_CASE(IpMe_EncodeBuffer),
    TEST_CASE(IpMe_EncodeBuffer_MBI_1080p_Downscale),
    TEST_CASE(IpMe_EncodeBuffer_MBI_1080p_Downscale_MultiFlushInput),
    TEST_CASE(IpMe_EncodeBuffer_MBI_1080p_Downscale_MultiFlushInputAndOutput),
    TEST_CASE(IpMe_EncodeBuffer_MBI_1080p_Downscale_Flush_after_EOS),
    TEST_CASE(IpMe_EncodeBuffer_MBI_1080p_Downscale_Flush_no_EOS),
    TEST_CASE(IpMe_EncodeBuffer_MBI_1080p_Downscale_123),
//    TEST_CASE(IpMe_EncodeBuffer_MBI_1080p_Downscale_123_EOS_len_zero),
    TEST_CASE(IpMe_EncodeBuffer_MBI_1080p_Downscale_MultiOpen),
    TEST_CASE(IpMe_EncodeBuffer_MBI_1080p_Downscale_Reprogram),
    TEST_CASE(IpMe_EncodeBuffer_MBI_1080p_Downscale_Drain),
    TEST_CASE(IpMe_EncodeBuffer_MBI_1080p_Downscale_Drain1),
    TEST_CASE(IpMe_EncodeBuffer_MBI_1080p_Downscale_Bypass),
    TEST_CASE(IpMe_EncodeBuffer_MBI_1080p_Downscale_Inter),
//    TEST_CASE(IpMe_EncodeBuffer_MBI_1080p_Downscale_Seed),
    TEST_CASE_NULL(),
};

TEST_SUITE(IpMeSuite,
        "IpMe Tests",
        IpMeSuiteInit,
        IpMeSuiteTerm,
        IpMeTestInit,
        IpMeTestTerm,
        IpMeTests);

