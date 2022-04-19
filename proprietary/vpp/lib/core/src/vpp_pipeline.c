/*!
 * @file vpp_pipeline.c
 *
 * @cr
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

 * @services    Implements the pipeline, which coordinates buffer flow to the
 *              various IP blocks within the VPP.
 */

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include "vpp.h"
#define VPP_LOG_TAG     VPP_LOG_MODULE_PIPELINE_TAG
#define VPP_LOG_MODULE  VPP_LOG_MODULE_PIPELINE
#include "vpp_dbg.h"
#include "vpp_core.h"
#include "vpp_ctx.h"
#include "vpp_ip.h"
#include "vpp_uc.h"
#include "vpp_buf.h"
#include "vpp_stats.h"
#include "vpp_utils.h"

#include "vpp_pipeline.h"

/************************************************************************
 * Local definitions
 ***********************************************************************/

#define VPP_PL_CB_GET(ctx) ((t_StVppPipelineCb *)ctx->pstPlCb)
#define VPP_PL_STATE_EQUAL(plcb, state) ((uint32_t)(plcb->eState == state))

#define IP_CTX_GET(pstPlCb, i)                  pstPlCb->avpIpCtx[i]
#define IP_CTX_SET(pstPlCb, i, val)             pstPlCb->avpIpCtx[i] = val

#define IP_GET(pPl, pIdx)                       pPl->pstUcOpen->composition[pIdx]

#define IP_FUNC(pPl, pIdx)                      IP_GET(pPl, pIdx).ip->fnc

#define IP_INIT(pPlCb, pCtx, pIdx, flags, cbs)  IP_FUNC(pPlCb, pIdx).init(pCtx, flags, cbs)
#define IP_TERM(pPlCb, pIdx)                    IP_FUNC(pPlCb, pIdx).term(IP_CTX_GET(pPlCb, pIdx))
#define IP_OPEN(pPlCb, pIdx)                    IP_FUNC(pPlCb, pIdx).open(IP_CTX_GET(pPlCb, pIdx))
#define IP_CLOSE(pPlCb, pIdx)                   IP_FUNC(pPlCb, pIdx).close(IP_CTX_GET(pPlCb, pIdx))
#define IP_SET_PARAM(pPlCb, pIdx, port, param)  IP_FUNC(pPlCb, pIdx).set_param(IP_CTX_GET(pPlCb, pIdx), port, param)
#define IP_SET_CTRL(pPlCb, pIdx, ctrl)          IP_FUNC(pPlCb, pIdx).set_ctrl(IP_CTX_GET(pPlCb, pIdx), ctrl)
#define IP_GET_BUF_REQ(pPlCb, pIdx, in, out)    IP_FUNC(pPlCb, pIdx).get_buffer_requirements(IP_CTX_GET(pPlCb, pIdx), in, out)
#define IP_QUEUE_BUF(pPlCb, pIdx, port, buf)    IP_FUNC(pPlCb, pIdx).queue_buf(IP_CTX_GET(pPlCb, pIdx), port, buf)
#define IP_FLUSH(pPlCb, pIdx, port)             IP_FUNC(pPlCb, pIdx).flush(IP_CTX_GET(pPlCb, pIdx), port)
#define IP_DRAIN(pPlCb, pIdx)                   IP_FUNC(pPlCb, pIdx).drain(IP_CTX_GET(pPlCb, pIdx))
#define IP_RCFG(pPlCb, pIdx, in, out)           IP_FUNC(pPlCb, pIdx).reconfigure(IP_CTX_GET(pPlCb, pIdx), in, out)


#define IP_LOG(LVL, pPlCb, idx, msg, ...) LOG##LVL("UC=%s, IP[%u]=%s: " msg, pPlCb->pstUcOpen->name, idx, IP_GET(pPlCb, idx).ip->name, ##__VA_ARGS__)

#define LOG_PL_CMD vVppPipeline_LogCmd

#define PRINT_UC(uc) uc, uc ? uc->name : "NULL"

enum {
    /*! The time taken to open the pipeline, including all submodules */
    PL_STAT_OPEN,
    /*! The time taken to close the pipeline, including all submodules */
    PL_STAT_CLOSE,
    /*! The time taken from when the first reconfigure request comes in to the
     * pipeline to the time that reconfigure done is issued. */
    PL_STAT_RCFG_LATENCY,
    /*! The time taken from when the drain done event comes from the last block
     * in the pipeline to the time that the reconfigure done is issued from
     * pipeline. */
    PL_STAT_RCFG_RAW,
};
/************************************************************************
 * Local static variables
 ***********************************************************************/

static const t_StVppStatsConfig astPlStatsCfg[] = {
    VPP_PROF_DECL(PL_STAT_OPEN, 1, 1),
    VPP_PROF_DECL(PL_STAT_CLOSE, 1, 1),
    VPP_PROF_DECL(PL_STAT_RCFG_LATENCY, 1, 1),
    VPP_PROF_DECL(PL_STAT_RCFG_RAW, 1, 1),
};

static const uint32_t u32PlStatCnt = VPP_STATS_CNT(astPlStatsCfg);

void dump_param(t_StVppParam *pstParam)
{
    LOGD("pstParam: flags=0x%x, input:{w=%u, h=%u, fmt=%u}, "
         "output:{w=%u, h=%u, fmt=%u}", pstParam->u32SessionFlags,
         pstParam->input_port.width, pstParam->input_port.height,
         pstParam->input_port.fmt, pstParam->output_port.width,
         pstParam->output_port.height, pstParam->output_port.fmt);
}

void dump_controls(t_StVppHqvCtrl *pstCtrl)
{
    LOGD("pstCtrl: mode=%u, ", pstCtrl->mode);
    LOG_CADE(D, &pstCtrl->cade);
    LOG_TNR(D, &pstCtrl->tnr);
    LOG_CNR(D, &pstCtrl->cnr);
    LOG_AIE(D, &pstCtrl->aie);
    LOG_DI(D, &pstCtrl->di);
}

void vVppPipeline_LogCmd(const char *pc, t_EVppPipelineCmd eCmd)
{

    switch (eCmd)
    {
        LOG_CASE(pc, VPP_PL_CMD_THREAD_EXIT);
        LOG_CASE(pc, VPP_PL_CMD_OPEN)
        LOG_CASE(pc, VPP_PL_CMD_CLOSE);
        LOG_CASE(pc, VPP_PL_CMD_FLUSH);
        LOG_CASE(pc, VPP_PL_CMD_DRAIN_REQUEST);
        LOG_CASE(pc, VPP_PL_CMD_DRAIN_COMPLETE);
        LOG_CASE(pc, VPP_PL_CMD_STOP);
        LOG_CASE(pc, VPP_PL_CMD_MAX);
    }
}

/************************************************************************
 * Forward Declarations
 ************************************************************************/

/************************************************************************
 * Local Functions
 ***********************************************************************/
// TODO: refactor these two functions (CmdGet/CmdPut) between this and
// GPU so that code gets reused...
static uint32_t u32VppPipeline_CmdGet(t_StVppPipelineCb *pstPlCb,
                                      t_StVppPipelineCmd *pstCmd)
{
    LOGI("%s()", __func__);
    int32_t idx;
    idx = vpp_queue_dequeue(&pstPlCb->stCmdQ);

    if (idx < 0)
    {
        return VPP_ERR;
    }
    else
    {
        *pstCmd = pstPlCb->astCmdNode[idx];
        LOG_PL_CMD("PL:GetCmd", pstCmd->eCmd);
    }

    return VPP_OK;
}

static uint32_t u32VppPipeline_CmdPut(t_StVppPipelineCb *pstPlCb,
                                      t_StVppPipelineCmd stCmd)
{
    int32_t idx;
    uint32_t u32Ret = VPP_OK;

    pthread_mutex_lock(&pstPlCb->mutex);

    LOG_PL_CMD("PL:InsertCmd", stCmd.eCmd);
    idx = vpp_queue_enqueue(&pstPlCb->stCmdQ);
    if (idx < 0)
    {
        u32Ret = VPP_ERR;
    } else
    {
        pstPlCb->astCmdNode[idx] = stCmd;
        pthread_cond_signal(&pstPlCb->cond_to_worker);
    }

    pthread_mutex_unlock(&pstPlCb->mutex);

    return u32Ret;
}

static uint32_t u32VppPipeline_HandleDrainDone(t_StVppCtx *pstCtx,
                                               t_StVppPipelineCb *pstPlCb,
                                               uint32_t u32IpPos)
{
    // This is issued from an IP block's callback. We need to put this onto the
    // Pipeline worker thread, because when the VPP interface layer calls into
    // here, it may cause for IP blocks to get torn down (if the new UC has
    // different composition from the current one, for example).

    VPP_UNUSED(pstCtx);

    t_StVppPipelineCmd stCmd;

    memset(&stCmd, 0, sizeof(t_StVppPipelineCmd));
    stCmd.eCmd = VPP_PL_CMD_DRAIN_COMPLETE;

    VPP_STATS(pstPlCb, DrainDoneCb);

    if (pstPlCb->u32IpCnt == 1 && u32IpPos == 0)
    {
        u32VppPipeline_CmdPut(pstPlCb, stCmd);
    }
    else
    {
        VPP_NOT_IMPLEMENTED();
    }

    return VPP_OK;
}

void vVppPipeline_InputBufferDoneCb(void *pv, t_StVppBuf *pstBuf)
{
    t_StVppPipelineCbCookie *pstCookie = (t_StVppPipelineCbCookie *)pv;
    t_StVppPipelineCb *pstCb = pstCookie->pstCtx->pstPlCb;
    uint32_t u32Pos = pstCookie->u32IpPos;

    LOGI("input buffer done from block: %u", pstCookie->u32IpPos);

    if (pstCb->u32IpCnt == 1)
    {
        if (pstBuf->u32InternalFlags & VPP_BUF_FLAG_FLUSHED &&
            pstCb->u32PipelineFlags & (PL_CLOSE_PENDING | PL_RECONFIGURE_TO_BYPASS))
        {
            LOGI("storing input buffer");
            pthread_mutex_lock(&pstCb->mutex);
            u32VppBufPool_Put(&pstCb->stInputQ, pstBuf);
            pthread_mutex_unlock(&pstCb->mutex);
        }
        else
        {
            VPP_STATS(pstCb, IssueIBD);
            VPP_LOG_BUF(pstCookie->pstCtx, pstBuf, eVppLogId_PlBufDone);
            u32VppIp_CbBufDone(&pstCb->stCallbacks, VPP_PORT_INPUT, pstBuf);
        }
    }
}

void vVppPipeline_OutputBufferDoneCb(void *pv, t_StVppBuf *pstBuf)
{
    t_StVppPipelineCbCookie *pstCookie = (t_StVppPipelineCbCookie *)pv;
    t_StVppPipelineCb *pstCb = pstCookie->pstCtx->pstPlCb;
    uint32_t u32Pos = pstCookie->u32IpPos;

    LOGI("output buffer done from block: %u", pstCookie->u32IpPos);

    if (pstCb->u32IpCnt == 1)
    {
        if (pstBuf->u32InternalFlags & VPP_BUF_FLAG_FLUSHED &&
            pstBuf->stPixel.u32FilledLen)
        {
            IP_LOG(E, pstCb, u32Pos, "received flushed buffer with non-zero "
                   "fill_len (len=%u)... setting to zero",
                   pstBuf->stPixel.u32FilledLen);
            pstBuf->stPixel.u32FilledLen = 0;
        }

        if (pstBuf->u32InternalFlags & VPP_BUF_FLAG_FLUSHED &&
            pstCb->u32PipelineFlags & (PL_CLOSE_PENDING | PL_RECONFIGURE_TO_BYPASS))
        {
            LOGI("storing output buffer");
            pthread_mutex_lock(&pstCb->mutex);
            u32VppBufPool_Put(&pstCb->stOutputQ, pstBuf);
            pthread_mutex_unlock(&pstCb->mutex);
        }
        else
        {
            VPP_STATS(pstCb, IssueOBD);
            VPP_LOG_BUF(pstCookie->pstCtx, pstBuf, eVppLogId_PlBufDone);
            u32VppIp_CbBufDone(&pstCb->stCallbacks, VPP_PORT_OUTPUT, pstBuf);
        }
    }
}

void vVppPipeline_EventCb(void *pv, t_StVppEvt stEvt)
{
    t_StVppPipelineCbCookie *pstCookie = (t_StVppPipelineCbCookie *)pv;
    t_StVppCtx *pstCtx = pstCookie->pstCtx;
    t_StVppPipelineCb *pstPlCb = pstCookie->pstCtx->pstPlCb;
    uint32_t u32Pos = pstCookie->u32IpPos;

    LOGI("event from block: %u", u32Pos);

    if (stEvt.eType == VPP_EVT_FLUSH_DONE)
    {
        enum vpp_port ePort = stEvt.flush.ePort;

        if (VPP_FLAG_IS_SET(pstPlCb->u32PipelineFlags, PL_CLOSE_PENDING) ||
            VPP_FLAG_IS_SET(pstPlCb->u32PipelineFlags, PL_RECONFIGURE_TO_BYPASS))
        {
            pthread_mutex_lock(&pstPlCb->mutex);

            if (ePort == VPP_PORT_INPUT)
                pstPlCb->au32PendingFlags[u32Pos] &= ~PL_INPUT_FLUSH_PENDING;
            else if (ePort == VPP_PORT_OUTPUT)
                pstPlCb->au32PendingFlags[u32Pos] &= ~PL_OUTPUT_FLUSH_PENDING;

            pthread_mutex_unlock(&pstPlCb->mutex);

            LOGI("flush event from block: %u, port=%u", u32Pos, ePort);

            pthread_cond_signal(&pstPlCb->cond_from_worker);
        }
        else
        {
            if (pstPlCb->u32IpCnt == 1)
            {
                u32VppIpCbEvent(&pstPlCb->stCallbacks, stEvt);
            }
            else
            {
                VPP_NOT_IMPLEMENTED();
            }
        }
    }
    else if (stEvt.eType == VPP_EVT_DRAIN_DONE)
    {
        u32VppPipeline_HandleDrainDone(pstCtx, pstPlCb, u32Pos);
    }
    else
    {
        LOGE("unrecognized event from block: %u", u32Pos);
    }
}

void vVppPipeline_LogCb(void *pv, t_StVppBuf *pstBuf, t_EVppBufIpLogId eId)
{
    if (!pv || !pstBuf || eId >= eVppLogId_IpMax)
        return;

    t_StVppPipelineCbCookie *pstCookie = (t_StVppPipelineCbCookie *)pv;
    t_StVppCtx *pstCtx = pstCookie->pstCtx;
    uint32_t u32Pos = pstCookie->u32IpPos;
    uint32_t u32Id = eVppLogId_IpStart + (eVppLogId_IpMax * u32Pos) + eId;
    VPP_LOG_BUF(pstCtx, pstBuf, u32Id);
}

static uint32_t u32VppPipeline_IsCustCtrlForIp(t_StVppPipelineCb *pstPlCb,
                                              uint32_t u32Idx,
                                              struct hqv_ctrl_custom *pstCust)
{
    if (pstCust->id == IP_GET(pstPlCb, u32Idx).ip->eIp)
        return VPP_TRUE;
    return VPP_FALSE;
}

static void vVppPipeline_CustomCtrlPut(t_StVppPipelineCb *pstPlCb,
                                       struct hqv_ctrl_custom *pstCustCtrl)
{
    t_StVppPipelineCustCtrlNode *pstNode;

    if (!pstPlCb || !pstCustCtrl)
        return;

    pstNode = (t_StVppPipelineCustCtrlNode *)
        malloc(sizeof(t_StVppPipelineCustCtrlNode));

    if (!pstNode)
    {
        LOGE("Unable to allocate memory for custom control node");
        return;
    }

    memcpy(&pstNode->ctrl, pstCustCtrl, sizeof(struct hqv_ctrl_custom));

    pstNode->pNext = pstPlCb->stCustCtrlList.pHead;
    pstPlCb->stCustCtrlList.pHead = pstNode;
    pstPlCb->stCustCtrlList.u32Cnt += 1;

    LOGI("stored custom control, cnt=%u, pHead=%p, ctrl: {id=%u, len=%u, data[0]=0x%x}",
         pstPlCb->stCustCtrlList.u32Cnt,
         pstPlCb->stCustCtrlList.pHead,
         pstNode->ctrl.id,
         pstNode->ctrl.len,
         ((uint32_t *)pstNode->ctrl.data)[0]);
}

static uint32_t u32VppPipeline_CustomCtrlGet(t_StVppPipelineCb *pstPlCb,
                                             uint32_t bUseCurrentUc,
                                             struct hqv_ctrl_custom *pstCustCtrl)
{
    uint32_t i, bFound = VPP_FALSE;
    uint32_t u32DbgCnt = 0;
    t_StVppPipelineCustCtrlNode *pstNode = NULL, *pstNodePrev = NULL;

    LOG_ENTER();

    if (!pstPlCb || !pstCustCtrl)
        LOG_EXIT_RET(VPP_ERR_PARAM);

    if (!pstPlCb->stCustCtrlList.u32Cnt)
        LOG_EXIT_RET(VPP_ERR);

    if (bUseCurrentUc)
    {
        pstNode = pstPlCb->stCustCtrlList.pHead;
        while (pstNode != NULL)
        {
            for (i = 0; i < pstPlCb->u32IpCnt; i++)
            {
                if (u32VppPipeline_IsCustCtrlForIp(pstPlCb, i, &pstNode->ctrl))
                {
                    if (pstNodePrev == NULL)
                        pstPlCb->stCustCtrlList.pHead = pstNode->pNext;
                    else
                        pstNodePrev->pNext = pstNode->pNext;

                    pstPlCb->stCustCtrlList.u32Cnt -= 1;
                    bFound = VPP_TRUE;
                    LOGI("found custom control at position %u for IP=%u",
                         u32DbgCnt, i);
                    break;
                }
            }

            if (bFound)
                break;
            else
            {
                u32DbgCnt += 1;
                pstNodePrev = pstNode;
                pstNode = pstNode->pNext;
            }
        }
    }
    else
    {
        LOGI("removing first custom control");
        pstNode = pstPlCb->stCustCtrlList.pHead;
        pstPlCb->stCustCtrlList.pHead = pstNode->pNext;
        pstPlCb->stCustCtrlList.u32Cnt -= 1;
    }

    if (pstNode)
    {
        memcpy(pstCustCtrl, &pstNode->ctrl, sizeof(struct hqv_ctrl_custom));
        free(pstNode);
        LOG_EXIT_RET(VPP_OK);
    }

    LOG_EXIT_RET(VPP_ERR);
}

static uint32_t u32VppPipeline_SubmodInit(t_StVppCtx *pstCtx,
                                          t_StVppPipelineCb *pstPlCb,
                                          t_StVppUsecase *pstUc)
{
    uint32_t i, err = 0;
    void *tmp;
    t_StVppCallback cbs;

    memset(&cbs, 0, sizeof(t_StVppCallback));

    cbs.input_buffer_done = vVppPipeline_InputBufferDoneCb;
    cbs.output_buffer_done = vVppPipeline_OutputBufferDoneCb;
    cbs.event = vVppPipeline_EventCb;
    cbs.log = vVppPipeline_LogCb;

    for (i = 0; i < VPP_IP_BLOCK_MAX; i++)
    {
        if (!pstUc->composition[i].ip)
            break;

        pstPlCb->astCbCookie[i].pstCtx = pstCtx;
        pstPlCb->astCbCookie[i].u32IpPos = i;
        cbs.pv = &pstPlCb->astCbCookie[i];

        IP_LOG(I, pstPlCb, i, ">> initializing");
        tmp = IP_INIT(pstPlCb, pstCtx, i, pstPlCb->pstParam->u32SessionFlags, cbs);
        IP_LOG(I, pstPlCb, i, "<< initialized, ctx=%p", tmp);
        if (!tmp)
        {
            err = 1;
            break;
        }
        IP_CTX_SET(pstPlCb, i, tmp);
        pstPlCb->u32IpCnt += 1;
    }

    if (err)
    {
        for (; i > 0; i--)
        {
            IP_TERM(pstPlCb, i - 1);
            IP_CTX_SET(pstPlCb, i - 1, NULL);
            pstPlCb->u32IpCnt -= 1;
        }
        return VPP_ERR;
    }

    LOGI("Total initialized ip: %u", pstPlCb->u32IpCnt);
    return VPP_OK;
}


static uint32_t u32VppPipeline_SubmodTerm(t_StVppPipelineCb *pstPlCb)
{
    uint32_t i;

    for (i = 0; i < pstPlCb->u32IpCnt; i++)
    {
        IP_LOG(I, pstPlCb, i, "terminating");
        IP_TERM(pstPlCb, i);
        IP_CTX_SET(pstPlCb, i, NULL);
    }

    pstPlCb->u32IpCnt = 0;

    return VPP_OK;
}

static uint32_t u32VppPipeline_SubmodOpen(t_StVppPipelineCb *pstPlCb)
{
    uint32_t u32Ret = VPP_ERR_INVALID_CFG;
    uint32_t i;

    for (i = 0; i < pstPlCb->u32IpCnt; i++)
    {
        u32Ret = IP_OPEN(pstPlCb, i);

        if (u32Ret != VPP_OK)
        {
            IP_LOG(E, pstPlCb, i, "error opening submodule");
            break;
        }
    }

    if (u32Ret != VPP_OK)
    {
        for (; i > 0; i--)
        {
            LOGE("closing index: %u", i);
            IP_CLOSE(pstPlCb, i);
        }
    }

    return u32Ret;
}

static uint32_t u32VppPipeline_SubmodClose(t_StVppPipelineCb *pstPlCb)
{
    uint32_t u32Ret, u32Error = VPP_OK, i;

    for (i = 0; i < pstPlCb->u32IpCnt; i++)
    {
        u32Ret = IP_CLOSE(pstPlCb, i);
        if (u32Ret != VPP_OK)
        {
            IP_LOG(E, pstPlCb, i, "error closing submodule, err=%u", u32Ret);
            u32Error = u32Ret;
        }
    }

    return u32Error;
}

static uint32_t u32VppPipeline_SubmodParamSet(t_StVppPipelineCb *pstPlCb)
{
    uint32_t u32Ret = VPP_OK, u32InRet, u32OutRet, i;

    for (i = 0; i < pstPlCb->u32IpCnt; i++)
    {
        // WARNING: this will be a problem for multiple blocks, when input
        // configuration is not the same as output configuration. Will need to
        // take scalar into account (but it is not currently supported). There
        // is also the requirement that input buffers can be sent to the output
        // port.
        u32InRet = IP_SET_PARAM(pstPlCb, i, VPP_PORT_INPUT, pstPlCb->pstParam->input_port);
        u32OutRet = IP_SET_PARAM(pstPlCb, i, VPP_PORT_OUTPUT, pstPlCb->pstParam->output_port);

        if (u32InRet != VPP_OK || u32OutRet != VPP_OK)
        {
            IP_LOG(E, pstPlCb, i, "error setting param, u32InRet=%u, u32OutRet=%u",
                   u32InRet, u32OutRet);
            u32Ret = VPP_ERR;
        }
    }

    return u32Ret;
}

static uint32_t u32VppPipeline_SubmodCtrlSetSingle(t_StVppPipelineCb *pstPlCb,
                                                   uint32_t u32IpIdx,
                                                   t_StVppHqvCtrl *pstCtrl,
                                                   enum hqv_control_type type)
{
    uint32_t u32Ret;
    struct hqv_control ctrl;
    memset(&ctrl, 0, sizeof(struct hqv_control));

    ctrl.mode = pstCtrl->mode;

    if ((ctrl.mode != HQV_MODE_AUTO) || (ctrl.ctrl_type >= HQV_CONTROL_GLOBAL_START))
    {
        ctrl.mode = pstCtrl->mode;
        ctrl.ctrl_type = type;

        switch (type)
        {
            case HQV_CONTROL_CADE:
                ctrl.cade = pstCtrl->cade;
                break;
            case HQV_CONTROL_DI:
                ctrl.di = pstCtrl->di;
                break;
            case HQV_CONTROL_TNR:
                ctrl.tnr = pstCtrl->tnr;
                break;
            case HQV_CONTROL_CNR:
                ctrl.cnr = pstCtrl->cnr;
                break;
            case HQV_CONTROL_AIE:
                ctrl.aie = pstCtrl->aie;
                break;
            case HQV_CONTROL_FRC:
                ctrl.frc = pstCtrl->frc;
                break;
            case HQV_CONTROL_GLOBAL_DEMO:
                ctrl.demo = pstCtrl->demo;
                break;
            case HQV_CONTROL_CUST:
                LOGE("HQV_CONTROL_CUST requested in SetSingle, but is invalid");
                break;
            case HQV_CONTROL_MAX:
                LOGE("HQV_CONTROL_MAX and mode not HQV_MODE_AUTO!");
            default:
                LOGE("Invalid type passed to SetSingle");
                break;
        }
    }

    IP_LOG(D, pstPlCb, u32IpIdx, "setting ctrl=%u", type);

    u32Ret = IP_SET_CTRL(pstPlCb, u32IpIdx, ctrl);

    if (u32Ret != VPP_OK)
        IP_LOG(E, pstPlCb, u32IpIdx, "setting ctrl=%u failed, u32Ret=%u",
               type, u32Ret);

    return u32Ret;
}

static uint32_t u32VppPipeline_SubmodCtrlDisableSingle(t_StVppPipelineCb *pstPlCb,
                                                       uint32_t u32IpIdx,
                                                       t_StVppHqvCtrl *pstCtrl,
                                                       enum hqv_control_type type)
{
    uint32_t u32Ret;
    struct hqv_control ctrl;
    memset(&ctrl, 0, sizeof(struct hqv_control));

    if (pstCtrl->mode == HQV_MODE_AUTO)
    {
        ctrl.mode = HQV_MODE_OFF;
    }
    else
    {
        ctrl.mode = pstCtrl->mode;
        ctrl.ctrl_type = type;

        switch (type)
        {
            case HQV_CONTROL_CADE:
                ctrl.cade.mode = HQV_MODE_OFF;
                break;
            case HQV_CONTROL_DI:
                ctrl.di.mode = HQV_DI_MODE_OFF;
                break;
            case HQV_CONTROL_TNR:
                ctrl.tnr.mode = HQV_MODE_OFF;
                break;
            case HQV_CONTROL_CNR:
                ctrl.cnr.mode = HQV_MODE_OFF;
                break;
            case HQV_CONTROL_AIE:
                ctrl.aie.mode = HQV_MODE_OFF;
                break;
            case HQV_CONTROL_FRC:
                ctrl.frc.mode = HQV_FRC_MODE_OFF;
                break;
            case HQV_CONTROL_CUST:
                break;
            case HQV_CONTROL_MAX:
            default:
                break;
        }
    }
    IP_LOG(D, pstPlCb, u32IpIdx, "disabling ctrl=%u", type);

    u32Ret = IP_SET_CTRL(pstPlCb, u32IpIdx, ctrl);

    if (u32Ret != VPP_OK)
        IP_LOG(E, pstPlCb, u32IpIdx, "disabling ctrl=%u failed, u32Ret=%u",
               type, u32Ret);

    return u32Ret;
}

static uint32_t u32VppPipeline_SubmodCtrlSet(t_StVppPipelineCb *pstPlCb,
                                             t_StVppUsecase *pstUc,
                                             t_StVppHqvCtrl *pstCtrl,
                                             uint32_t bDisable)
{
    uint32_t idx_ip, idx_algo, u32Ret;
    enum hqv_control_type type;

    if (pstCtrl->mode == HQV_MODE_AUTO)
    {
        for (idx_ip = 0; idx_ip < pstPlCb->u32IpCnt; idx_ip++)
        {
            if (bDisable)
                u32VppPipeline_SubmodCtrlDisableSingle(pstPlCb, idx_ip, pstCtrl,
                                                       HQV_CONTROL_MAX);
            else
                u32VppPipeline_SubmodCtrlSetSingle(pstPlCb, idx_ip, pstCtrl,
                                                   HQV_CONTROL_MAX);
        }
    }
    else
    {
        // foreach algo in the use case
        for (idx_algo = 0; idx_algo < pstUc->algos.u32CtrlCnt; idx_algo++)
        {
            type = pstUc->algos.ctrls[idx_algo];

            // check if each ip block supports the algo
            for (idx_ip = 0; idx_ip < pstPlCb->u32IpCnt; idx_ip++)
            {
                if (u32VppIp_SupportsAlgo(IP_GET(pstPlCb, idx_ip).ip, type))
                {
                    if (bDisable)
                        u32VppPipeline_SubmodCtrlDisableSingle(pstPlCb, idx_ip,
                                                               pstCtrl, type);
                    else
                        u32VppPipeline_SubmodCtrlSetSingle(pstPlCb, idx_ip,
                                                           pstCtrl, type);
                }
            }
        }
    }

    return VPP_OK;
}

uint32_t u32VppPipeline_SubmodCustomCtrlSet(t_StVppPipelineCb *pstPlCb)
{
    uint32_t u32, i;
    struct hqv_control stCtrl;
    struct hqv_ctrl_custom *pstCustCtrl;

    LOG_ENTER();
    if (!pstPlCb)
        LOG_EXIT_RET(VPP_ERR_PARAM);

    memset(&stCtrl, 0, sizeof(struct hqv_control));
    stCtrl.mode = HQV_MODE_MANUAL;
    stCtrl.ctrl_type = HQV_CONTROL_CUST;
    pstCustCtrl = &stCtrl.custom;

    // For each custom control that is stored, pull it from the list and
    // send it to the IP block for which that custom control is destined.
    while (u32VppPipeline_CustomCtrlGet(pstPlCb, VPP_TRUE, pstCustCtrl) == VPP_OK)
    {
        for (i = 0; i < pstPlCb->u32IpCnt; i++)
        {
            if (u32VppPipeline_IsCustCtrlForIp(pstPlCb, i, pstCustCtrl))
            {
                IP_LOG(D, pstPlCb, i, "setting custom ctrl, id=%u",
                       pstCustCtrl->id);
                u32 = IP_SET_CTRL(pstPlCb, i, stCtrl);
                if (u32 != VPP_OK)
                    IP_LOG(E, pstPlCb, i, "setting custom ctrl, id=%u, err=%u",
                           pstCustCtrl->id, u32);
                break;
            }
        }

        if (i == pstPlCb->u32IpCnt)
        {
            // Should never hit this case, since CustomCtrlGet() should only
            // return custom controls based on the composition of the current
            // usecase.
            LOGE("found custom control for IP that is not in use");
        }
    }

    LOG_EXIT_RET(VPP_OK);
}

static uint32_t u32VppPipeline_SubmodGlobalCtrlSet(t_StVppPipelineCb *pstPlCb,
                                                   t_StVppHqvCtrl *pstCtrl)
{
    uint32_t idx_ip, idx_algo, u32Ret;
    enum hqv_control_type type;

    LOG_ENTER();

    for(type = HQV_CONTROL_GLOBAL_START; type < HQV_CONTROL_MAX; type++)
    {
        // check if each ip block supports the algo
        for (idx_ip = 0; idx_ip < pstPlCb->u32IpCnt; idx_ip++)
        {
            if (u32VppIp_SupportsAlgo(IP_GET(pstPlCb, idx_ip).ip, type))
            {
                u32VppPipeline_SubmodCtrlSetSingle(pstPlCb, idx_ip, pstCtrl, type);
            }
        }
    }
    LOG_EXIT_RET(VPP_OK);
}

uint32_t u32VppPipeline_SubmodGetBufReq(t_StVppPipelineCb *pstPlCb)
{
    VPP_UNUSED(pstPlCb);
    // TODO
    return VPP_OK;
}

uint32_t u32VppPipeline_SubmodImplementReconfigure(t_StVppPipelineCb *pstPlCb)
{
    uint32_t i;

    if (!pstPlCb)
        return VPP_FALSE;

    for (i = 0; i < pstPlCb->u32IpCnt; i++)
        if (!IP_FUNC(pstPlCb, i).reconfigure)
            return VPP_FALSE;

    return VPP_TRUE;
}

uint32_t u32VppPipeline_SubmodReconfigure(t_StVppPipelineCb *pstPlCb)
{
    uint32_t i, u32Ret = VPP_OK;

    if (!pstPlCb)
        return VPP_ERR_PARAM;

    for (i = 0; i < pstPlCb->u32IpCnt; i++)
    {
        u32Ret = IP_RCFG(pstPlCb, i, pstPlCb->pstParam->input_port,
                         pstPlCb->pstParam->output_port);
        if (u32Ret != VPP_OK)
        {
            IP_LOG(E, pstPlCb, i, "error reconfiguring submodule");
            break;
        }
    }

    return u32Ret;
}

uint32_t u32VppPipeline_FlushIpHoldBufs(t_StVppPipelineCb *pstPlCb,
                                        uint32_t u32Flag)
{
    uint32_t u32Ret = VPP_OK;

    if (pstPlCb->u32IpCnt == 1)
    {
        uint32_t u32 = 0;

        pthread_mutex_lock(&pstPlCb->mutex);
        pstPlCb->u32PipelineFlags |= u32Flag;
        pstPlCb->au32PendingFlags[u32] |= PL_INPUT_FLUSH_PENDING;
        pstPlCb->au32PendingFlags[u32] |= PL_OUTPUT_FLUSH_PENDING;
        pthread_mutex_unlock(&pstPlCb->mutex);

        u32Ret = IP_FLUSH(pstPlCb, u32, VPP_PORT_INPUT);
        if (u32Ret != VPP_OK)
        {
            LOGE("unable to flush input port");
            pthread_mutex_lock(&pstPlCb->mutex);
            pstPlCb->au32PendingFlags[u32] &= ~PL_INPUT_FLUSH_PENDING;
            pthread_mutex_unlock(&pstPlCb->mutex);
        }

        u32Ret = IP_FLUSH(pstPlCb, u32, VPP_PORT_OUTPUT);
        if (u32Ret != VPP_OK)
        {
            LOGE("unable to flush input port");
            pthread_mutex_lock(&pstPlCb->mutex);
            pstPlCb->au32PendingFlags[u32] &= ~PL_OUTPUT_FLUSH_PENDING;
            pthread_mutex_unlock(&pstPlCb->mutex);
        }

        LOGI("waiting for flags to be cleared");
        pthread_mutex_lock(&pstPlCb->mutex);
        while (pstPlCb->au32PendingFlags[u32] &
               (PL_OUTPUT_FLUSH_PENDING | PL_INPUT_FLUSH_PENDING))
            pthread_cond_wait(&pstPlCb->cond_from_worker, &pstPlCb->mutex);

        pstPlCb->u32PipelineFlags &= ~u32Flag;

        pthread_mutex_unlock(&pstPlCb->mutex);
        LOGI("flag cleared");
    }
    return u32Ret;
}

uint32_t u32VppPipeline_InternalOpen(t_StVppCtx *pstCb, t_StVppHqvCtrl *pstCtrl,
                                     t_StVppUsecase *pstUc)
{
    /*!
     * This function expects that all validation has already occurred on the
     * parameters
     */
    uint32_t u32;
    t_StVppPipelineCb *pstPlCb;

    pstPlCb = pstCb->pstPlCb;
    pstPlCb->pstUcOpen = pstUc;

    u32 = u32VppPipeline_SubmodInit(pstCb, pstPlCb, pstUc);
    if (u32 != VPP_OK)
    {
        LOGE("SubmodInit returned %u", u32);
        goto ERROR_SUBMOD_INIT;
    }

    u32 = u32VppPipeline_SubmodParamSet(pstPlCb);
    if (u32 != VPP_OK)
    {
        LOGE("SubmodParamSet returned %u", u32);
        goto ERROR_SUBMOD_PARAMSET;
    }

    u32 = u32VppPipeline_SubmodCtrlSet(pstPlCb, pstUc, pstCtrl, VPP_FALSE);
    LOGE_IF(u32 != VPP_OK, "SubmodCtrlSet returned %u", u32);

    u32 = u32VppPipeline_SubmodCustomCtrlSet(pstPlCb);
    LOGE_IF(u32 != VPP_OK, "SubmodCustomCtrlSet returned %u", u32);

    u32 = u32VppPipeline_SubmodGlobalCtrlSet(pstPlCb, pstCtrl);
    LOGE_IF(u32 != VPP_OK, "SubmodGlobalCtrlSet returned %u", u32);

    u32 = u32VppPipeline_SubmodGetBufReq(pstPlCb);
    LOGE_IF(u32 != VPP_OK, "SubmodGetBufReq returned %u", u32);

    u32 = u32VppPipeline_SubmodOpen(pstPlCb);
    if (u32 != VPP_OK)
    {
        LOGE("SubmodOpen returned %u", u32);
        goto ERROR_SUBMOD_OPEN;
    }

    return u32;

ERROR_SUBMOD_OPEN:
ERROR_SUBMOD_PARAMSET:
    u32VppPipeline_SubmodTerm(pstPlCb);

ERROR_SUBMOD_INIT:
    pstPlCb->pstUcOpen = NULL;
    return u32;
}


uint32_t u32VppPipeline_InternalClose(t_StVppCtx *pstCb)
{
    uint32_t u32Ret = VPP_OK;
    t_StVppPipelineCb *pstPlCb;

    pstPlCb = VPP_PL_CB_GET(pstCb);

    if (pstPlCb->pstUcOpen)
    {
        u32Ret = u32VppPipeline_FlushIpHoldBufs(pstPlCb, PL_CLOSE_PENDING);
        LOGE_IF(u32Ret != VPP_OK, "FlushIpHoldBufs failed");

        u32Ret = u32VppPipeline_SubmodClose(pstPlCb);
        LOGE_IF(u32Ret != VPP_OK, "submod close failed");

        u32Ret = u32VppPipeline_SubmodTerm(pstPlCb);
        LOGE_IF(u32Ret != VPP_OK, "submod term failed");
    }

    pstPlCb->pstUcOpen = NULL;

    return u32Ret;
}

static uint32_t u32VppPipeline_HandleReconfigureCompEqCloseOpen(t_StVppCtx *pstCb,
                                                                t_StVppPipelineCb *pstPlCb,
                                                                t_StVppUsecase *pstUc,
                                                                t_StVppHqvCtrl *pstCtrl)
{
    uint32_t u32;

    LOG_ENTER();

    u32 = u32VppPipeline_SubmodClose(pstPlCb);
    if (u32 != VPP_OK)
    {
        LOGE("SubmodClose returned %u", u32);
        goto ERROR_CLOSE;
    }

    u32 = u32VppPipeline_SubmodParamSet(pstPlCb);
    if (u32 != VPP_OK)
    {
        LOGE("SubmodParamSet returned %u", u32);
        goto ERROR_PARAMSET;
    }

    if (pstUc != pstPlCb->pstUcOpen)
    {
        // Possible that the composition is the same but number of
        // controls that can be enabled are different.
        LOGD("Reconfigure, different usecases, pstUcOld=%p, pstUcNew=%p",
             pstPlCb->pstUcOpen, pstUc);
        u32 = u32VppPipeline_SubmodCtrlSet(pstCb->pstPlCb, pstPlCb->pstUcOpen,
                                           pstCtrl, VPP_TRUE);
        LOGE_IF(u32 != VPP_OK, "SubmodCtrlSet (disable) returned %u",
                u32);

        u32 = u32VppPipeline_SubmodCtrlSet(pstCb->pstPlCb, pstUc, pstCtrl,
                                           VPP_FALSE);
        LOGE_IF(u32 != VPP_OK, "SubmodCtrlSet (enable) returned %u",
                u32);
    }

    u32 = u32VppPipeline_SubmodOpen(pstCb->pstPlCb);
    if (u32 != VPP_OK)
    {
        LOGE("SubmodOpen returned %u", u32);
        goto ERROR_OPEN;
    }

    pstPlCb->pstUcOpen = pstUc;
    LOG_EXIT_RET(u32);

ERROR_OPEN:
ERROR_PARAMSET:
ERROR_CLOSE:
    u32VppPipeline_SubmodTerm(pstCb->pstPlCb);
    pstPlCb->pstUcOpen = NULL;
    LOG_EXIT_RET(u32);
}

static uint32_t u32VppPipeline_HandleReconfigureCompEqRcfg(t_StVppCtx *pstCb,
                                                           t_StVppPipelineCb *pstPlCb,
                                                           t_StVppUsecase *pstUc,
                                                           t_StVppHqvCtrl *pstCtrl)
{
    uint32_t u32 = VPP_OK;

    VPP_UNUSED(pstCb);
    VPP_UNUSED(pstCtrl);

    LOG_ENTER();

    LOGD("optimized reconfigure of pipeline");
    u32 = u32VppPipeline_SubmodReconfigure(pstPlCb);
    if (u32 != VPP_OK)
    {
        LOGE("SubmodReconfigure returned %u", u32);
        goto ERROR_RECONFIGURE;
    }

    pstPlCb->pstUcOpen = pstUc;
    LOG_EXIT_RET(u32);

ERROR_RECONFIGURE:
    u32VppPipeline_SubmodTerm(pstCb->pstPlCb);
    pstPlCb->pstUcOpen = NULL;
    LOG_EXIT_RET(u32);
}

static uint32_t u32VppPipeline_HandleReconfigureCompEqual(t_StVppCtx *pstCb,
                                                          t_StVppPipelineCb *pstPlCb,
                                                          t_StVppUsecase *pstUc,
                                                          t_StVppHqvCtrl *pstCtrl)
{
    uint32_t u32 = VPP_OK;

    if (pstUc == pstPlCb->pstUcOpen &&
        u32VppPipeline_SubmodImplementReconfigure(pstPlCb))
    {
        u32 = u32VppPipeline_HandleReconfigureCompEqRcfg(pstCb, pstPlCb,
                                                         pstUc, pstCtrl);
    }
    else
    {
        u32 = u32VppPipeline_HandleReconfigureCompEqCloseOpen(pstCb, pstPlCb,
                                                              pstUc, pstCtrl);
    }

    return u32;
}

static uint32_t u32VppPipeline_HandleReconfigureCompDiff(t_StVppCtx *pstCb,
                                                         t_StVppPipelineCb *pstPlCb,
                                                         t_StVppUsecase *pstUc,
                                                         t_StVppHqvCtrl *pstCtrl)
{
    uint32_t u32;

    LOG_ENTER();

    if (!pstUc)
        return VPP_ERR_PARAM;

    // The usecase composition is different, thus there can either be a real
    // usecase that is opened, or bypass.
    if (pstPlCb->pstUcOpen)
    {
        u32 = u32VppPipeline_InternalClose(pstCb);
        if (u32 != VPP_OK)
        {
            LOGE("u32VppPipeline_InternalClose, u32=%u", u32);
            return VPP_ERR;
        }
    }

    u32 = u32VppPipeline_InternalOpen(pstCb, pstCtrl, pstUc);
    if (u32 != VPP_OK)
    {
        LOGE("u32VppPipeline_InternalOpen, u32=%u", u32);
        return VPP_ERR;
    }

    return u32;
}

static void vVppPipeline_SetState(t_StVppPipelineCb *pstCb,
                                  t_EVppPipelineState eState)
{
    LOGI("pipeline state transition: %u to %u", pstCb->eState, eState);
    pstCb->eState = eState;
}

static void vVppPipeline_FlushPort(t_StVppCtx *pstCtx,
                                   t_StVppPipelineCb *pstPlCb,
                                   enum vpp_port ePort)
{
    t_StVppBuf *pstBuf;
    t_StVppBufPool pstQ;

    if (ePort == VPP_PORT_INPUT)
    {
        while ((pstBuf = pstVppBufPool_Get(&pstPlCb->stInputQ)) != NULL)
        {
            pstBuf->stPixel.u32FilledLen = 0;
            VPP_STATS(pstPlCb, FlushedInputBuf);
            VPP_LOG_BUF(pstCtx, pstBuf, eVppLogId_PlBufDone);
            u32VppIp_CbBufDone(&pstPlCb->stCallbacks, ePort, pstBuf);
        }
    }
    else
    {
        while ((pstBuf = pstVppBufPool_Get(&pstPlCb->stOutputQ)) != NULL)
        {
            pstBuf->stPixel.u32FilledLen = 0;
            VPP_STATS(pstPlCb, FlushedOutputBuf);
            VPP_LOG_BUF(pstCtx, pstBuf, eVppLogId_PlBufDone);
            u32VppIp_CbBufDone(&pstPlCb->stCallbacks, ePort, pstBuf);
        }
    }
}


static uint32_t u32VppPipeline_ProcCmdFlush(t_StVppCtx *pstCtx,
                                            t_StVppPipelineCb *pstPlCb,
                                            t_StVppPipelineCmd *pstCmd)
{
    t_StVppEvt stEvt;
    enum vpp_port ePort;

    LOG_ENTER();

    if (pstCmd->eCmd != VPP_PL_CMD_FLUSH)
        return VPP_ERR_PARAM;

    ePort = pstCmd->flush.ePort;

    vVppPipeline_FlushPort(pstCtx, pstPlCb, ePort);

    stEvt.eType = VPP_EVT_FLUSH_DONE;
    stEvt.flush.ePort = ePort;
    u32VppIpCbEvent(&pstPlCb->stCallbacks, stEvt);

    LOG_EXIT();
    return VPP_OK;

}

static uint32_t u32VppPipeline_ProcCmdDrainRequest(t_StVppCtx *pstCtx,
                                                   t_StVppPipelineCb *pstPlCb,
                                                   t_StVppPipelineCmd *pstCmd)
{
    VPP_UNUSED(pstCmd);

    t_StVppBuf *pstBuf;
    t_StVppPipelineCmd stCmd;
    memset(&stCmd, 0, sizeof(t_StVppPipelineCmd));

    while ((pstBuf = pstVppBufPool_Get(&pstPlCb->stInputQ)) != NULL)
    {
        VPP_STATS(pstPlCb, IssueOBD);
        VPP_LOG_BUF(pstCtx, pstBuf, eVppLogId_PlBufDone);
        u32VppIp_CbBufDone(&pstPlCb->stCallbacks, VPP_PORT_OUTPUT, pstBuf);
    }

    stCmd.eCmd = VPP_PL_CMD_DRAIN_COMPLETE;
    u32VppPipeline_CmdPut(pstPlCb, stCmd);

    return VPP_OK;
}

static uint32_t u32VppPipeline_ProcCmdDrainComplete(t_StVppCtx *pstCtx,
                                                    t_StVppPipelineCb *pstPlCb,
                                                    t_StVppPipelineCmd *pstCmd)
{
    VPP_UNUSED(pstCtx);
    VPP_UNUSED(pstCmd);

    t_StVppEvt stEvt;

    if (VPP_FLAG_IS_SET(pstPlCb->u32PipelineFlags, PL_RECONFIGURE_PENDING))
    {
        u32VppStats_Start(pstPlCb->pstCtx, pstPlCb->pvStatsHdl, PL_STAT_RCFG_RAW);

        pthread_mutex_lock(&pstPlCb->mutex);
        VPP_FLAG_SET(pstPlCb->u32PipelineFlags, PL_RECONFIGURE_CALLBACK);
        pthread_mutex_unlock(&pstPlCb->mutex);

        stEvt.eType = VPP_EVT_RECONFIG_RDY;
        u32VppIpCbEvent(&pstPlCb->stCallbacks, stEvt);

        pthread_mutex_lock(&pstPlCb->mutex);
        VPP_FLAG_CLR(pstPlCb->u32PipelineFlags, PL_RECONFIGURE_CALLBACK);
        pthread_mutex_unlock(&pstPlCb->mutex);

        if (VPP_FLAG_IS_SET(pstPlCb->u32PipelineFlags, PL_RECONFIGURE_PENDING))
        {
            pstPlCb->u32ReconfigErr = VPP_ERR;
            VPP_FLAG_CLR(pstPlCb->u32PipelineFlags, PL_RECONFIGURE_PENDING);
            LOGE("Pipeline was not reconfigured during callback");
        }

        stEvt.eType = VPP_EVT_RECONFIG_DONE;
        stEvt.reconfig.u32Err = pstPlCb->u32ReconfigErr;
        if (pstPlCb->bBypass)
            stEvt.reconfig.pstUc = NULL;
        else
            stEvt.reconfig.pstUc = pstPlCb->pstUcOpen;

        u32VppStats_Stop(pstPlCb->pstCtx, pstPlCb->pvStatsHdl, PL_STAT_RCFG_RAW);
        u32VppStats_Stop(pstPlCb->pstCtx, pstPlCb->pvStatsHdl, PL_STAT_RCFG_LATENCY);
        u32VppIpCbEvent(&pstPlCb->stCallbacks, stEvt);
    }
    else
    {
        LOGE("%s() called with invalid state", __func__);
        return VPP_ERR_STATE;
    }

    return VPP_OK;
}

uint32_t u32VppPipeline_WorkerShouldSleep(t_StVppPipelineCb *pstPlCb)
{
    uint32_t u32Ret = VPP_TRUE;
    uint32_t u32CmdQSz, u32InQSz, u32OutQSz;

    u32CmdQSz = vpp_queue_count(&pstPlCb->stCmdQ);
    u32InQSz = u32VppBufPool_Cnt(&pstPlCb->stInputQ);
    u32OutQSz = u32VppBufPool_Cnt(&pstPlCb->stOutputQ);

    if (u32CmdQSz)
    {
        u32Ret = VPP_FALSE;
    }
    else if (VPP_PL_STATE_EQUAL(pstPlCb, VPP_PLSTATE_ACTIVE) &&
             pstPlCb->bBypass && u32InQSz)
    {
        u32Ret = VPP_FALSE;
    }

    LOGI("CHECK: shouldSleep=%u, u32CmdQSz=%u, eState=%u, bBypass=%u, "
         "u32InQSz=%u, u32OutQSz=%u", u32Ret, u32CmdQSz, pstPlCb->eState,
         pstPlCb->bBypass, u32InQSz, u32OutQSz);

    return u32Ret;
}

void *vpVppPipeline_Worker(void *pv)
{
    uint32_t u32Ret;
    t_StVppCtx *pstCtx;
    t_StVppPipelineCb *pstPlCb;
    t_StVppPipelineCmd stCmd;

    LOGD("%s started", __func__);

    pstCtx = (t_StVppCtx *)pv;
    pstPlCb = pstCtx->pstPlCb;

    pthread_mutex_lock(&pstPlCb->mutex);
    VPP_FLAG_SET(pstPlCb->u32PipelineFlags, PL_WORKER_STARTED);
    pthread_mutex_unlock(&pstPlCb->mutex);
    pthread_cond_signal(&pstPlCb->cond_from_worker);

    while (1)
    {
        pthread_mutex_lock(&pstPlCb->mutex);
        while (u32VppPipeline_WorkerShouldSleep(pstPlCb))
            pthread_cond_wait(&pstPlCb->cond_to_worker, &pstPlCb->mutex);

        u32Ret = u32VppPipeline_CmdGet(pstPlCb, &stCmd);
        if (u32Ret == VPP_OK)
        {
            pthread_mutex_unlock(&pstPlCb->mutex);

            // Process the command
            LOG_PL_CMD("PL:ProcessCmd", stCmd.eCmd);

            if (stCmd.eCmd == VPP_PL_CMD_THREAD_EXIT)
                break;

            else if (stCmd.eCmd == VPP_PL_CMD_FLUSH)
                u32VppPipeline_ProcCmdFlush(pstCtx, pstPlCb, &stCmd);

            else if (stCmd.eCmd == VPP_PL_CMD_DRAIN_REQUEST)
                u32VppPipeline_ProcCmdDrainRequest(pstCtx, pstPlCb, &stCmd);

            else if (stCmd.eCmd == VPP_PL_CMD_DRAIN_COMPLETE)
                u32VppPipeline_ProcCmdDrainComplete(pstCtx, pstPlCb, &stCmd);

            else
                LOGE("unknown command in queue");

            continue;
        }

        if (!VPP_PL_STATE_EQUAL(pstPlCb, VPP_PLSTATE_ACTIVE))
        {
            LOGD("got buffer, but state is not active");
            pthread_mutex_unlock(&pstPlCb->mutex);
            continue;
        }
        else
        {
            t_StVppBuf *pstBuf;
            pstBuf = pstVppBufPool_Get(&pstPlCb->stInputQ);
            pthread_mutex_unlock(&pstPlCb->mutex);
            if (!pstBuf)
            {
                LOGE("unable to dequeue buffer from pool");
                continue;
            }

            VPP_STATS(pstPlCb, IssueOBD);
            VPP_LOG_BUF(pstCtx, pstBuf, eVppLogId_PlBufDone);
            u32VppIp_CbBufDone(&pstPlCb->stCallbacks, VPP_PORT_OUTPUT, pstBuf);
        }
    }

    LOGD("%s exited", __func__);

    return NULL;
}

/************************************************************************
 * Global Functions
 ***********************************************************************/

uint32_t u32VppPipeline_Init(t_StVppCtx *pstCtx, t_StVppCallback stCbs)
{
    int rc = 0;
    uint32_t u32;
    void *pvStatsHdl = NULL;
    t_StVppPipelineCb *pstPlCb;

    if (!pstCtx)
        return VPP_ERR_PARAM;

    pstPlCb = calloc(1, sizeof(t_StVppPipelineCb));

    if (!pstPlCb)
        return VPP_ERR_NO_MEM;

    pstCtx->pstPlCb = pstPlCb;
    pstPlCb->pstCtx = pstCtx;
    pstPlCb->stCallbacks = stCbs;

    u32 = u32VppStats_Register(pstCtx, astPlStatsCfg, u32PlStatCnt, &pvStatsHdl);
    LOGE_IF(u32 != VPP_OK, "unable to register pipeline stats, u32=%u", u32);
    pstPlCb->pvStatsHdl = pvStatsHdl;

    u32VppBufPool_Init(&pstPlCb->stInputQ);
    u32VppBufPool_Init(&pstPlCb->stOutputQ);

    vpp_queue_init(&pstPlCb->stCmdQ, PL_CMD_Q_SZ);

    rc = pthread_mutex_init(&pstPlCb->mutex, NULL);
    if (rc)
        goto ERR_MUTEX_INIT;

    rc = pthread_cond_init(&pstPlCb->cond_to_worker, NULL);
    if (rc)
        goto ERR_COND_TW_INIT;

    rc = pthread_cond_init(&pstPlCb->cond_from_worker, NULL);
    if (rc)
        goto ERR_COND_FW_INIT;

    rc = pthread_create(&pstPlCb->thread, NULL, vpVppPipeline_Worker, pstCtx);
    if (rc)
        goto ERR_THREAD_CREATE;

    pthread_mutex_lock(&pstPlCb->mutex);
    while (!VPP_FLAG_IS_SET(pstPlCb->u32PipelineFlags, PL_WORKER_STARTED))
        pthread_cond_wait(&pstPlCb->cond_from_worker, &pstPlCb->mutex);
    pthread_mutex_unlock(&pstPlCb->mutex);

    vVppPipeline_SetState(pstPlCb, VPP_PLSTATE_INITED);

    return VPP_OK;

ERR_THREAD_CREATE:
    pthread_cond_destroy(&pstPlCb->cond_from_worker);

ERR_COND_FW_INIT:
    pthread_cond_destroy(&pstPlCb->cond_to_worker);

ERR_COND_TW_INIT:
    pthread_mutex_destroy(&pstPlCb->mutex);

ERR_MUTEX_INIT:
    u32 = u32VppStats_Unregister(pstCtx, pstPlCb->pvStatsHdl);
    LOGE_IF(u32 != VPP_OK, "ec: unable to unregister pipeline stats, u32=%u", u32);

    free(pstPlCb);
    pstCtx->pstPlCb = NULL;

    return VPP_ERR;
}

uint32_t u32VppPipeline_Term(t_StVppCtx *pstCb)
{
    uint32_t u32;
    t_StVppPipelineCb *pstPlCb;
    t_StVppBuf *pstBuf;
    t_StVppPipelineCmd stCmd;
    struct hqv_ctrl_custom stCustCtrlTmp;

    if (!pstCb || !pstCb->pstPlCb)
        return VPP_ERR_PARAM;

    pstPlCb = VPP_PL_CB_GET(pstCb);

    stCmd.eCmd = VPP_PL_CMD_THREAD_EXIT;
    u32VppPipeline_CmdPut(pstPlCb, stCmd);

    pthread_join(pstPlCb->thread, NULL);

    // Flush out input and output pool if they're not empty...
    vVppPipeline_FlushPort(pstCb, pstPlCb, VPP_PORT_INPUT);
    vVppPipeline_FlushPort(pstCb, pstPlCb, VPP_PORT_OUTPUT);

    vpp_queue_term(&pstPlCb->stCmdQ);

    u32VppBufPool_Term(&pstPlCb->stInputQ);
    u32VppBufPool_Term(&pstPlCb->stOutputQ);

    // Flush out custom controls if there are any
    while(u32VppPipeline_CustomCtrlGet(pstPlCb, VPP_FALSE, &stCustCtrlTmp) == VPP_OK);

    pthread_cond_destroy(&pstPlCb->cond_to_worker);
    pthread_cond_destroy(&pstPlCb->cond_from_worker);
    pthread_mutex_destroy(&pstPlCb->mutex);

    u32 = u32VppStats_Unregister(pstPlCb->pstCtx, pstPlCb->pvStatsHdl);
    LOGE_IF(u32 != VPP_OK, "unable to unregister pipeline stats, u32=%u", u32);

    free(pstPlCb);
    pstCb->pstPlCb = NULL;

    return VPP_OK;
}

uint32_t u32VppPipeline_Open(t_StVppCtx *pstCb, t_StVppUsecase *pstUc,
                             t_StVppParam *pstParam, t_StVppHqvCtrl *pstCtrl)
{
    uint32_t u32Ret = VPP_OK;
    uint32_t u32SubmodCnt;
    t_StVppPipelineCb *pstPlCb;

    if (!pstCb || !pstCb->pstPlCb || !pstCtrl)
        return VPP_ERR_PARAM;

    pstPlCb = VPP_PL_CB_GET(pstCb);

    if (!VPP_PL_STATE_EQUAL(pstPlCb, VPP_PLSTATE_INITED))
        return VPP_ERR_STATE;

    u32VppStats_Start(pstPlCb->pstCtx, pstPlCb->pvStatsHdl, PL_STAT_OPEN);

    pstPlCb->pstParam = pstParam;
    pstPlCb->bBypass = VPP_FALSE;

    LOGD("UC=%p (%s)", PRINT_UC(pstUc));

    dump_param(pstParam);
    dump_controls(pstCtrl);

    if (pstUc)
    {
        u32SubmodCnt = u32VppUsecase_SubmodCount(pstUc);
        if (u32SubmodCnt == 0 || u32SubmodCnt > 1)
        {
            // Bypass
            u32Ret = VPP_ERR_PARAM;
            LOGD("IP block exceeds max supported in pipeline, cnt=%u", u32SubmodCnt);
        }
        else
        {
            u32Ret = u32VppPipeline_InternalOpen(pstCb, pstCtrl, pstUc);
            if (u32Ret != VPP_OK)
            {
                LOGE("IP open failed, enabling bypass, u32Ret=%u", u32Ret);
            }
        }
    }

    if (!pstUc || u32Ret != VPP_OK)
    {
        LOGD("Enabling pipeline bypass");
        pstPlCb->bBypass = VPP_TRUE;
    }

    vVppPipeline_SetState(pstPlCb, VPP_PLSTATE_ACTIVE);

    u32VppStats_Stop(pstPlCb->pstCtx, pstPlCb->pvStatsHdl, PL_STAT_OPEN);

    return u32Ret;
}

uint32_t u32VppPipeline_Close(t_StVppCtx *pstCb)
{
    uint32_t u32Ret = VPP_OK;
    t_StVppPipelineCb *pstPlCb;

    LOGI("%s()", __func__);

    if (!pstCb || !pstCb->pstPlCb)
        return VPP_ERR_PARAM;

    pstPlCb = VPP_PL_CB_GET(pstCb);

    u32VppStats_Start(pstPlCb->pstCtx, pstPlCb->pvStatsHdl, PL_STAT_CLOSE);

    u32VppPipeline_InternalClose(pstCb);

    vVppPipeline_SetState(pstPlCb, VPP_PLSTATE_INITED);

    u32VppStats_Stop(pstPlCb->pstCtx, pstPlCb->pvStatsHdl, PL_STAT_CLOSE);

    return u32Ret;
}

uint32_t u32VppPipeline_SetHqvCtrl(t_StVppCtx *pstCb, struct hqv_control ctrl)
{
    uint32_t u32Ret = VPP_OK;
    uint32_t i;

    t_StVppPipelineCb *pstPlCb;

    if (!pstCb || !pstCb->pstPlCb)
        return VPP_ERR_PARAM;

    pstPlCb = VPP_PL_CB_GET(pstCb);

    if (pstPlCb->eState == VPP_PLSTATE_INITED)
    {
        if (ctrl.ctrl_type != HQV_CONTROL_CUST)
        {
            LOGE("Invalid set control in pipeline");
            return VPP_ERR_STATE;
        }

        LOGI("received custom control in inited state");
        vVppPipeline_CustomCtrlPut(pstPlCb, &ctrl.custom);
        return VPP_OK;
    }
    else if (pstPlCb->eState != VPP_PLSTATE_ACTIVE)
    {
        return VPP_ERR_STATE;
    }

    // This function ignores HQV mode in the top level control. It is expected
    // that the caller of this function will filter out any calls if the
    // HQV mode is invalid. This function just passes on the ctrl and params.
    for (i = 0; i < pstPlCb->u32IpCnt; i++)
    {
        if (ctrl.ctrl_type == HQV_CONTROL_CUST)
        {
            if (u32VppPipeline_IsCustCtrlForIp(pstPlCb, i, &ctrl.custom))
            {
                IP_LOG(D, pstPlCb, i, "setting custom ctrl=%u", ctrl.ctrl_type);
                u32Ret = IP_SET_CTRL(pstPlCb, i, ctrl);
            }
            else
            {
                vVppPipeline_CustomCtrlPut(pstPlCb, &ctrl.custom);
            }
        }
        else if (u32VppIp_SupportsAlgo(IP_GET(pstPlCb, i).ip, ctrl.ctrl_type))
        {
            IP_LOG(D, pstPlCb, i, "setting ctrl=%u", ctrl.ctrl_type);
            u32Ret = IP_SET_CTRL(pstPlCb, i, ctrl);
        }
    }

    return u32Ret;
}


uint32_t u32VppPipeline_Reconfigure(t_StVppCtx *pstCb, t_StVppUsecase *pstUc,
                                    t_StVppParam *pstParam, t_StVppHqvCtrl *pstCtrl)
{
    uint32_t u32Ret = VPP_OK;
    uint32_t bCompEq;
    t_StVppPipelineCb *pstPlCb;
    t_StVppBuf *pstBuf;

    if (!pstCb || !pstCb->pstPlCb || !pstCtrl || !pstParam)
        return VPP_ERR_PARAM;

    pstPlCb = VPP_PL_CB_GET(pstCb);

    LOG_ENTER();

    if (!VPP_FLAG_IS_SET(pstPlCb->u32PipelineFlags, PL_RECONFIGURE_CALLBACK))
    {
        LOGE("%s() called when reconfigure callback flag not set!", __func__);
        LOG_EXIT();
        return VPP_ERR_STATE;
    }

    dump_param(pstParam);
    dump_controls(pstCtrl);

    pstPlCb->pstParam = pstParam;

    LOGD("%s, bBypass=%u, uc_open:{%p, %s}, uc_new:{%p, %s}, ", __func__,
         pstPlCb->bBypass, PRINT_UC(pstPlCb->pstUcOpen), PRINT_UC(pstUc));

    if (pstUc)
    {
        bCompEq = u32VppUsecase_CompositionEqual(pstUc, pstPlCb->pstUcOpen);
        LOGD("%s, bCompEqual=%u", __func__, bCompEq);

        if (pstPlCb->bBypass)
        {
            LOGD("Reconfiguring out of bypass");
            pstPlCb->bBypass = VPP_FALSE;
        }

        if (bCompEq)
        {
            // Identical usecases implies equal composition as well
            u32Ret = u32VppPipeline_HandleReconfigureCompEqual(pstCb, pstPlCb,
                                                               pstUc, pstCtrl);
            LOGE_IF(u32Ret != VPP_OK, "HandleReconfigureCompEqual, u32=%u", u32Ret);
        }
        else
        {
            u32Ret = u32VppPipeline_HandleReconfigureCompDiff(pstCb, pstPlCb,
                                                              pstUc, pstCtrl);
            LOGE_IF(u32Ret != VPP_OK, "HandleReconfigureCompDiff, u32=%u", u32Ret);
        }

        if (u32Ret == VPP_OK)
        {

            uint32_t u32;
            while ((pstBuf = pstVppBufPool_Get(&pstPlCb->stInputQ)) != NULL)
            {
                u32 = IP_QUEUE_BUF(pstPlCb, 0, VPP_PORT_INPUT, pstBuf);
                LOGE_IF(u32 != VPP_OK, "queue buf to reconfigure'd IP "
                        "block failed, u32=%u", u32);
            }
            while ((pstBuf = pstVppBufPool_Get(&pstPlCb->stOutputQ)) != NULL)
            {
                u32 = IP_QUEUE_BUF(pstPlCb, 0, VPP_PORT_OUTPUT, pstBuf);
                LOGE_IF(u32 != VPP_OK, "queue buf to reconfigure'd IP "
                        "block failed, u32=%u", u32);
            }
        }

    }

    if (u32Ret != VPP_OK || !pstUc)
    {
        // pstUc being NULL signifies that we were requested to be reconfigured
        // into bypass mode. If u32Ret != VPP_OK, then there was an error
        // during reconfiguration and we have to put the pipeline into bypass.
        // In this case, we need to also report an error.
        //
        // If pstUc == NULL, we could have been reconfigured from a previous
        // bypass state to a new bypass state, or we are being reconfigured
        // from a normal running state to bypass state. If the latter is the
        // case, we need to make sure that we get all buffers out of the
        // pipeline.
        //
        // If we are the former, we need to make sure that we are reporting the
        // error so that we can tell the usecase / concurrency manager.

        LOGD("Reconfiguring into bypass mode, pstUc=%p, u32Ret=%u", pstUc, u32Ret);

        if (!pstPlCb->bBypass && !pstUc)
        {
            // Normal mode, going into bypass
            u32Ret = u32VppPipeline_FlushIpHoldBufs(pstPlCb, PL_RECONFIGURE_TO_BYPASS);
        }

        if (u32Ret != VPP_OK)
        {
            // Bypass or Normal mode, going into normal mode, error.
            pstPlCb->u32ReconfigErr = u32Ret;
        }

        pstPlCb->bBypass = VPP_TRUE;
    }

    VPP_FLAG_CLR(pstPlCb->u32PipelineFlags, PL_RECONFIGURE_PENDING);

    LOG_EXIT();

    return u32Ret;
}

uint32_t u32VppPipeline_QueueBuf(t_StVppCtx *pstCb, enum vpp_port ePort,
                                 t_StVppBuf *pstBuf)
{
    uint32_t u32Ret = VPP_OK;
    t_StVppPipelineCb *pstPlCb;

    if (!pstCb || !pstCb->pstPlCb || !pstBuf)
        return VPP_ERR_PARAM;

    if (ePort != VPP_PORT_INPUT && ePort != VPP_PORT_OUTPUT)
        return VPP_ERR_PARAM;

    pstPlCb = VPP_PL_CB_GET(pstCb);

    if (!VPP_PL_STATE_EQUAL(pstPlCb, VPP_PLSTATE_ACTIVE))
        return VPP_ERR_STATE;

    VPP_LOG_BUF(pstCb, pstBuf, eVppLogId_PlQueueBuf);

    pthread_mutex_lock(&pstPlCb->mutex);
    if (pstPlCb->bBypass == VPP_FALSE &&
        !VPP_FLAG_IS_SET(pstPlCb->u32PipelineFlags, PL_RECONFIGURE_CALLBACK))
    {
        pthread_mutex_unlock(&pstPlCb->mutex);
        if (pstPlCb->u32IpCnt == 1)
        {
            u32Ret = IP_QUEUE_BUF(pstPlCb, 0, ePort, pstBuf);
        }
        else
        {
            // TODO: multiblock pipeline is not supported at this time.
            LOGE("multiblock pipeline not supported");
            u32Ret = VPP_ERR;
        }
    }
    else
    {
        if (ePort == VPP_PORT_INPUT)
            u32VppBufPool_Put(&pstPlCb->stInputQ, pstBuf);
        else
            u32VppBufPool_Put(&pstPlCb->stOutputQ, pstBuf);

        pthread_mutex_unlock(&pstPlCb->mutex);
        pthread_cond_signal(&pstPlCb->cond_to_worker);
    }

    return u32Ret;
}

uint32_t u32VppPipeline_RequestFlush(t_StVppCtx *pstCb, enum vpp_port ePort)
{
    t_StVppPipelineCb *pstPlCb;
    t_StVppPipelineCmd stCmd;
    uint32_t u32Ret = VPP_OK;
    uint32_t bAsyncFlush = VPP_FALSE;

    if (!pstCb || !pstCb->pstPlCb)
        return VPP_ERR_PARAM;

    if (ePort != VPP_PORT_INPUT && ePort != VPP_PORT_OUTPUT)
        return VPP_ERR_PARAM;

    pstPlCb = VPP_PL_CB_GET(pstCb);

    if (ePort == VPP_PORT_INPUT)
        VPP_STATS(pstPlCb, RequestFlushInput);
    else
        VPP_STATS(pstPlCb, RequestFlushOutput);

    LOGI("%s(), state=%u, bypass=%u, blocks=%u, port=%u", __func__,
         pstPlCb->eState, pstPlCb->bBypass, pstPlCb->u32IpCnt, ePort);

    if (VPP_PL_STATE_EQUAL(pstPlCb, VPP_PLSTATE_INITED))
    {
        // When inited, there should be no buffers in flight; at most there
        // will be buffers in the holding ports.
        bAsyncFlush = VPP_TRUE;
    }
    else if (VPP_PL_STATE_EQUAL(pstPlCb, VPP_PLSTATE_ACTIVE))
    {
        // Blocks in the pipeline, or we're in bypass mode
        if (pstPlCb->bBypass)
        {
            bAsyncFlush = VPP_TRUE;
        }
        else if (pstPlCb->u32IpCnt == 1)
        {
            u32Ret = IP_FLUSH(pstPlCb, 0, ePort);
            if (u32Ret != VPP_OK)
            {
                IP_LOG(E, pstPlCb, 0, "IP_FLUSH returned u32Ret=%u", u32Ret);
                return u32Ret;
            }
        }
        else
        {
            if (ePort == VPP_PORT_INPUT)
            {
                // Input port flush. Need to finish processing anything in
                // flight on the first block and then drain the rest of the
                // pipeline. This translates into a flush_input on the first
                // block, and then drain on the rest of the IP blocks
                // downstream. IP blocks should trigger all OBDs before
                // triggering the event done callback so we can use that to
                // synchronize as to when to trigger the downstream blocks to
                // drain...
                VPP_NOT_IMPLEMENTED();
            }
            else
            {
                // Output port flush. This is tricky because we need to make
                // sure that there is nothing in flight. We need to drain the
                // pipeline, starting from the second block. This should make
                // sure that there are no more buffers on the input, so then
                // we can flush all of the output ports and return the buffers.
                VPP_NOT_IMPLEMENTED();
            }
        }
    }
    else
    {
        u32Ret = VPP_ERR;
    }

    if (bAsyncFlush)
    {
        stCmd.eCmd = VPP_PL_CMD_FLUSH;
        stCmd.flush.ePort = ePort;
        u32VppPipeline_CmdPut(pstPlCb, stCmd);
    }

    return u32Ret;
}

uint32_t u32VppPipeline_RequestReconfigure(t_StVppCtx *pstCb)
{
    t_StVppPipelineCb *pstPlCb;
    t_StVppPipelineCmd stCmd;
    uint32_t u32Ret = VPP_OK;
    uint32_t bAsyncDrain = VPP_FALSE;

    if (!pstCb || !pstCb->pstPlCb)
        return VPP_ERR_PARAM;

    pstPlCb = VPP_PL_CB_GET(pstCb);

    VPP_STATS(pstPlCb, RequestReconfigure);

    LOGI("%s(), state=%u, bypass=%u, blocks=%u", __func__, pstPlCb->eState,
         pstPlCb->bBypass, pstPlCb->u32IpCnt);

    if (VPP_FLAG_IS_SET(pstPlCb->u32PipelineFlags, PL_RECONFIGURE_PENDING))
    {
        LOGE("Multiple reconfigure requests received");
        return VPP_ERR_PARAM;
    }

    pstPlCb->u32ReconfigErr = VPP_OK;

    u32VppStats_Start(pstPlCb->pstCtx, pstPlCb->pvStatsHdl, PL_STAT_RCFG_LATENCY);

    if (VPP_PL_STATE_EQUAL(pstPlCb, VPP_PLSTATE_INITED))
    {
        // When inited, there should be no buffers being processed...
        // the worker thread should just be able to return immediately.
        bAsyncDrain = VPP_TRUE;
        LOGD("RequestReconfigure in VPP_PLSTATE_INITED");
    }
    else if (VPP_PL_STATE_EQUAL(pstPlCb, VPP_PLSTATE_ACTIVE))
    {
        // Blocks in the pipeline, or we're in bypass mode
        if (pstPlCb->bBypass)
        {
            bAsyncDrain = VPP_TRUE;
        }
        else if (pstPlCb->u32IpCnt == 1)
        {
            VPP_FLAG_SET(pstPlCb->u32PipelineFlags, PL_RECONFIGURE_PENDING);

            u32Ret = IP_DRAIN(pstPlCb, 0);
            if (u32Ret != VPP_OK)
            {
                IP_LOG(E, pstPlCb, 0, "IP_DRAIN returned u32Ret=%u", u32Ret);
                VPP_FLAG_CLR(pstPlCb->u32PipelineFlags, PL_RECONFIGURE_PENDING);
                return u32Ret;
            }
        }
        else
        {
            // There are multiple blocks in the pipeline. Need to drain each
            // block prior to draining the downstream block. Once the last
            // block has been drained, then we can issue the drain callback.
            VPP_NOT_IMPLEMENTED();
        }
    }
    else
    {
        u32Ret = VPP_ERR;
    }

    if (bAsyncDrain)
    {
        VPP_FLAG_SET(pstPlCb->u32PipelineFlags, PL_RECONFIGURE_PENDING);
        stCmd.eCmd = VPP_PL_CMD_DRAIN_REQUEST;
        u32VppPipeline_CmdPut(pstPlCb, stCmd);
    }

    return u32Ret;
}
