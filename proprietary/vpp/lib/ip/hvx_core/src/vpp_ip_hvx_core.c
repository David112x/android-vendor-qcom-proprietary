/*!
 * @file vpp_ip_hvx_core.c
 *
 * @cr
 * Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

 * @services
 */

#include <sys/types.h>
#include <sys/mman.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "AEEStdErr.h"

#define VPP_LOG_TAG     VPP_LOG_MODULE_HVX_CORE_TAG
#define VPP_LOG_MODULE  VPP_LOG_MODULE_HVX_CORE
#include "vpp_dbg.h"

#include "vpp.h"
#include "vpp_ion.h"
#include "vpp_reg.h"
#include "vpp_ip.h"
#include "vpp_utils.h"
#include "vpp_stats.h"
#include "vpp_ip_hvx_core.h"


/************************************************************************
 * Local definitions
 ***********************************************************************/
#define HVX_MIGRATE_CNT_MAX                 2
#define HVX_VERSION_LEN                     128
#define HVX_LIB_NAME_LEN                    64
#define HVX_PROPERTY_LOGFLAGS               "vendor.media.vpp.hvx.logflags"
#define HVX_PROPERTY_STATSFLAGS             "vendor.media.vpp.hvx.statsflags"
#define HVX_PROPERTY_STATSPERIOD            "vendor.media.vpp.hvx.statsperiod"

enum {
    HVX_CORE_STAT_REGBUF,
    HVX_CORE_STAT_UNREGBUF,

    HVX_CORE_STAT_SVC_INIT,
    HVX_CORE_STAT_SVC_DEINIT,
    HVX_CORE_STAT_SVC_PREPARE_CTX,
    HVX_CORE_STAT_SVC_GET_CTX,
    HVX_CORE_STAT_SVC_GET_DIAG_CTX,

    HVX_CORE_STAT_MIGRATE,
    HVX_CORE_STAT_ALLOC_CTX,
    HVX_CORE_STAT_FREE_CTX,
    HVX_CORE_STAT_CORE_OPEN,

    HVX_CORE_STAT_SET_DBG_LVL,
    HVX_CORE_STAT_GET_VER_INFO,
};

/************************************************************************
 * Local static variables
 ***********************************************************************/
static const t_StVppStatsConfig astHvxCoreStatsCfg[] = {
    VPP_PROF_DECL(HVX_CORE_STAT_REGBUF, 100, 0),
    VPP_PROF_DECL(HVX_CORE_STAT_UNREGBUF, 100, 0),

    VPP_PROF_DECL(HVX_CORE_STAT_SVC_INIT, 1, 1),
    VPP_PROF_DECL(HVX_CORE_STAT_SVC_DEINIT, 1, 1),
    VPP_PROF_DECL(HVX_CORE_STAT_SVC_PREPARE_CTX, 1, 1),
    VPP_PROF_DECL(HVX_CORE_STAT_SVC_GET_CTX, 1, 1),
    VPP_PROF_DECL(HVX_CORE_STAT_SVC_GET_DIAG_CTX, 1, 1),

    VPP_PROF_DECL(HVX_CORE_STAT_MIGRATE, 1, 1),
    VPP_PROF_DECL(HVX_CORE_STAT_ALLOC_CTX, 1, 1),
    VPP_PROF_DECL(HVX_CORE_STAT_FREE_CTX, 1, 1),
    VPP_PROF_DECL(HVX_CORE_STAT_CORE_OPEN, 1, 1),

    VPP_PROF_DECL(HVX_CORE_STAT_SET_DBG_LVL, 1, 1),
    VPP_PROF_DECL(HVX_CORE_STAT_GET_VER_INFO, 1, 1),
};

static const uint32_t u32HvxCoreStatCnt = VPP_STATS_CNT(astHvxCoreStatsCfg);

/************************************************************************
 * Forward Declarations
 ************************************************************************/
extern void *remote_register_fd(int fd, int size);
#pragma weak remote_register_fd

extern void remote_register_buf(void* buf, int size, int fd);
#pragma weak remote_register_buf

/************************************************************************
 * Local Functions
 ***********************************************************************/

static void vVppIpHvxCore_SetDebugLevels(t_StVppIpHvxCoreCb *pstCb)
{
    int32_t s32Ret;
    vpp_svc_statlogs_cfg_t stDebugCfg;

    stDebugCfg.log_flag = pstCb->debug_cfg.u32LogFlags;
    stDebugCfg.stats_flag = pstCb->debug_cfg.u32StatsFlags;
    stDebugCfg.stats_period = pstCb->debug_cfg.u32StatsPeriod;

    s32Ret = vpp_svc_set_config(CFG_TYPE_STATSLOGS, (unsigned char*)&stDebugCfg,
                                sizeof(vpp_svc_statlogs_cfg_t));
    if (s32Ret != 0)
    {
        LOGE("unable to set stats config on firmware, s32Ret=%d", s32Ret);
    }
}

static void vVppIpHvxCore_ReadProperties(t_StVppIpHvxCoreCb *pstCb)
{
    VPP_RET_VOID_IF_NULL(pstCb);

    u32VppUtils_ReadPropertyU32(HVX_PROPERTY_LOGFLAGS,
                                &pstCb->debug_cfg.u32LogFlags, "0");
    u32VppUtils_ReadPropertyU32(HVX_PROPERTY_STATSFLAGS,
                                &pstCb->debug_cfg.u32StatsFlags, "0");
    u32VppUtils_ReadPropertyU32(HVX_PROPERTY_STATSPERIOD,
                                &pstCb->debug_cfg.u32StatsPeriod, "0");

    LOGI("HVX debug levels: log_flag=0x%x, stats_flag=0x%x, stats_period=0x%x",
         pstCb->debug_cfg.u32LogFlags, pstCb->debug_cfg.u32StatsFlags,
         pstCb->debug_cfg.u32StatsPeriod);
}

static void vVppIpHvxCore_GetVersionInfo(t_StVppIpHvxCoreCb *pstCb)
{
    int32_t s32Ret, i;
    vpp_svc_cap_resource_list_t* pstCapRes;

    VPP_RET_VOID_IF_NULL(pstCb);

    if (pstCb->once.u32TsRead)
        return;
    pstCb->once.u32TsRead = VPP_TRUE;

    pstCapRes = pstCb->pstCapabilityResources;

    for (i = 0; i < pstCapRes->resourceLen; i++)
    {
        s32Ret = vpp_svc_get_buildts_id(pstCapRes->resource[i].vpp_func_id,
                                        (unsigned char *)pstCapRes->resource[i].build_ts,
                                        MAX_TS_LEN);
        if (s32Ret == AEE_SUCCESS)
        {
            LOGI("HVX func_id[%d] version: %s",
                 pstCapRes->resource[i].vpp_func_id,
                 pstCapRes->resource[i].build_ts);
        }
        else if (s32Ret == AEE_EREADONLY)
            LOGI("HVX in protected mode. Unable to get HVX version for func_id[%d]", i);
        else
            LOGE("Unable to get HVX version for func_id[%d], s32Ret=%d", i, s32Ret);
    }
}

static uint32_t u32VppIpHvxCore_HandleSessionMigration(t_StVppIpHvxCoreCb *pstCb)
{
    int rc;
    uint32_t u32Heap, i;
    uint32_t u32Ret = VPP_ERR_HW;

    VPP_RET_IF_NULL(pstCb, VPP_ERR_PARAM);

    VPP_IP_PROF_START(&pstCb->stBase, HVX_CORE_STAT_MIGRATE);

    if (!pstCb->stBase.bSecureSession)
        return VPP_OK;

    u32Heap = u32VppIon_GetHeapId(pstCb->stBase.pstCtx,
                                  pstCb->stBase.bSecureSession);

    for (i = 0; i < HVX_MIGRATE_CNT_MAX; i++)
    {
        rc = vpp_svc_migrate_pd(u32Heap);
        if (rc == AEE_SUCCESS)
        {
            u32Ret = VPP_OK;
            LOGI("vpp_svc_migrate_pd(), i=%u, rc=%d", i, rc);
            break;
        }
        else if (rc != AEE_EINTERRUPTED)
        {
            LOGE("ERROR: vpp_svc_migrate_pd(), rc=%d", rc);
            break;
        }
    }

    VPP_IP_PROF_STOP(&pstCb->stBase, HVX_CORE_STAT_MIGRATE);
    return u32Ret;
}

static void vVppIpHvxCore_RegisterIon(t_StVppIonBuf *pstIon, void **ppv)
{
    if (!ppv)
    {
        LOGE("%s: ppv null", __func__);
        return;
    }

    *ppv = MAP_FAILED;
    if (!pstIon)
    {
        LOGE("%s: pstIon is null", __func__);
        return;
    }

    if (pstIon->bSecure)
    {
        if (remote_register_fd)
        {
            *ppv = remote_register_fd(pstIon->fd_ion_mem, pstIon->len);
            if (*ppv == (void *)-1 || *ppv == NULL)
            {
                LOGE("unable to remote_register_fd, dummy_ptr=%p", *ppv);
                *ppv = MAP_FAILED;
            }
            LOGI("registering secure buffer, fd=%d, sz=%u, new_ptr=%p",
                 pstIon->fd_ion_mem, pstIon->len, *ppv);
        }
        else
            LOGE("unable to register secure buffer, fd=%d, unregistering "
                 "ptr=%p, sz=%u", pstIon->fd_ion_mem, pstIon->buf, pstIon->len);
    }
    else
    {
        if (remote_register_buf)
        {
            remote_register_buf(pstIon->buf, pstIon->len, pstIon->fd_ion_mem);
            *ppv = pstIon->buf;
            LOGI("registering buffer, fd=%d, ptr=%p, sz=%u",
                 pstIon->fd_ion_mem, pstIon->buf, pstIon->len);
        }
        else
            LOGE("unable to register buffer, fd=%d, ptr=%p, sz=%u",
                 pstIon->fd_ion_mem, pstIon->buf, pstIon->len);
    }
}

static void vVppIpHvxCore_UnregisterIon(t_StVppIonBuf *pstIon, void *pv)
{
    void *pvLocal;
    if (!pstIon)
    {
        LOGE("%s called with null ptr", __func__);
        return;
    }

    if (pstIon->bSecure)
        pvLocal = pv;
    else
        pvLocal = pstIon->buf;

    if (remote_register_buf)
    {
        LOGI("unregistering buffer, secure=%u, fd=%d, ptr=%p, sz=%u",
             pstIon->bSecure, pstIon->fd_ion_mem, pvLocal, pstIon->len);
        remote_register_buf(pvLocal, pstIon->len, -1);
    }
    else
    {
        LOGI("unable to unregister buffer, fd=%d, ptr=%p, sz=%u",
             pstIon->fd_ion_mem, pstIon->buf, pstIon->len);
    }
}

static uint32_t u32VppIpHvxCore_AllocateContext(t_StVppIpHvxCoreCb *pstHvxCore,
                                                uint32_t u32ReqLen)
{
    uint32_t u32Ret = VPP_OK;
    vpp_svc_context_t* pstContext;
    t_StVppIonBuf *pstIonBuf;

    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstContext = pstHvxCore->pstContext;
    pstIonBuf = &pstHvxCore->ctx.stIonCtxBuf;

    LOGD("allocating context buffer, sz=%u", u32ReqLen);

#ifdef USE_MALLOC
    if (0 == (pstContext->user_data =
              (unsigned char*)memalign(128, u32ReqLen)))
    {
        LOGE("Error: pstContext->user_data alloc failed\n");
        u32Ret = VPP_ERR_NO_MEM;
        goto ERR_ALLOC;
    }
#else
    uint32_t u32;
    VPP_IP_PROF_START(&pstHvxCore->stBase, HVX_CORE_STAT_ALLOC_CTX);
    u32 = u32VppIon_Alloc(pstHvxCore->stBase.pstCtx, u32ReqLen,
                          pstHvxCore->stBase.bSecureSession, pstIonBuf);
    VPP_IP_PROF_STOP(&pstHvxCore->stBase, HVX_CORE_STAT_ALLOC_CTX);
    if (u32 != VPP_OK)
    {
        LOGE("Error: context ion alloc failed, secure=%u, fd=%d, addr=%p, sz=%u",
             pstHvxCore->stBase.bSecureSession, pstIonBuf->fd_ion_mem,
             pstIonBuf->buf, u32ReqLen);
        u32Ret = VPP_ERR_NO_MEM;
        goto ERR_ALLOC;
    }
    else
    {
        pstHvxCore->ctx.bAllocated = VPP_TRUE;
        pstHvxCore->ctx.u32AllocatedSize = u32ReqLen;
    }

    LOGD("allocated ion mem fd=%d, addr=%p, sz=%u", pstIonBuf->fd_ion_mem,
         pstIonBuf->buf, u32ReqLen);

    vVppIpHvxCore_RegisterIon(pstIonBuf, (void *)&pstContext->user_data);
#endif

ERR_ALLOC:
    pstContext->user_dataLen = u32Ret == VPP_OK ? u32ReqLen : 0;

    return u32Ret;

}

static uint32_t u32VppIpHvxCore_FreeContext(t_StVppIpHvxCoreCb *pstHvxCore)
{
    t_StVppIonBuf *pstIonBuf;

    if (!pstHvxCore)
        return VPP_ERR_PARAM;

    if (!pstHvxCore->ctx.bAllocated)
        return VPP_OK;

    pstIonBuf = &pstHvxCore->ctx.stIonCtxBuf;

    LOGD("freeing context buffer");

#ifdef USE_MALLOC
    if(pstHvxCore->pstContext->user_data)
        free(pstHvxCore->pstContext->user_data);
#else
    vVppIpHvxCore_UnregisterIon(pstIonBuf, (void *)&pstHvxCore->pstContext->user_data);

    VPP_IP_PROF_START(&pstHvxCore->stBase, HVX_CORE_STAT_FREE_CTX);
    u32VppIon_Free(pstHvxCore->stBase.pstCtx, pstIonBuf);
    VPP_IP_PROF_STOP(&pstHvxCore->stBase, HVX_CORE_STAT_FREE_CTX);
#endif

    pstHvxCore->ctx.bAllocated = VPP_FALSE;
    pstHvxCore->ctx.u32AllocatedSize = 0;

    return VPP_OK;
}

/************************************************************************
 * Global Functions
 ***********************************************************************/

vpp_svc_field_fmt_t eVppIpHvxCore_SvcFieldFormatGet(t_EVppBufType eBufType)
{
    vpp_svc_field_fmt_t eFmt;
    switch (eBufType)
    {
        case eVppBufType_Interleaved_TFF:
            eFmt = FIELD_FMT_INTERLEAVED_TFF;
            break;

        case eVppBufType_Interleaved_BFF:
            eFmt = FIELD_FMT_INTERLEAVED_BFF;
            break;
        case eVppBufType_Frame_TFF:
            eFmt = FIELD_FMT_TFF;
            break;

        case eVppBufType_Frame_BFF:
            eFmt = FIELD_FMT_BFF;
            break;

        case eVppBufType_Progressive:
            eFmt = FIELD_FMT_PROGRESSIVE;
            break;

        case eVppBufType_Max:
        default:
            LOGE("Requested invalid format: eBufType=%d", eBufType);
            eFmt = FIELD_FMT_PROGRESSIVE;
            break;
    }
    return eFmt;
}

vpp_svc_pixel_fmt_t eVppIpHvxCore_SvcPixelFormatGet(enum vpp_color_format fmt)
{
    if (fmt == VPP_COLOR_FORMAT_NV12_VENUS)
        return PIXEL_FMT_NV12;

    else if (fmt == VPP_COLOR_FORMAT_NV21_VENUS)
        return PIXEL_FMT_NV21;

    LOGE("unsupported color format: %u, returning default", fmt);
    return PIXEL_FMT_NV12;
}

inline void *hvx_alloc_int(uint32_t sz, const char *str)
{
    LOGI("-- allocating %d bytes for: %s\n", sz, str);
    return memalign(128, sz);
}

void vVppIpHvxCore_RegisterBuffer(t_StVppIpHvxCoreCb *pstHvxCore, t_StVppMemBuf *pstIntMemBuf)
{
    if (!pstHvxCore || !pstHvxCore->stBase.pstCtx || !pstIntMemBuf)
        return;

#ifndef HVX_DO_NOT_REGISTER_BUFFER
    if (VPP_FLAG_IS_SET(pstIntMemBuf->u32IntBufFlags, VPP_BUF_FLAG_HVX_REGISTERED))
        return;

    // In a secure session, buffer management logic can not map this into
    // virtual address space for us, but the interface to HVX requires that the
    // buffer have a virtual address. Thus, we need to ask the adsp driver to
    // give us a virtual address to use. We will replace the vaddr here in the
    // buffer, and when we do the unregister call, we will replace it with
    // NULL. Must take care here not to modify the u32MappedLen or anything
    // like that.
    if (pstIntMemBuf->eMapping == eVppBuf_Unmapped)
    {
        void *pv = NULL;
        if (remote_register_fd)
        {
            VPP_IP_PROF_START(&pstHvxCore->stBase, HVX_CORE_STAT_REGBUF);
            pv = remote_register_fd(pstIntMemBuf->fd, pstIntMemBuf->u32AllocLen);
            VPP_IP_PROF_STOP(&pstHvxCore->stBase, HVX_CORE_STAT_REGBUF);

            if (pv == (void *)-1 || pv == NULL)
            {
                LOGE("unable to remote_register_fd, fd=%d, alloc_len=%u, "
                     "dummy_ptr=%p", pstIntMemBuf->fd, pstIntMemBuf->u32AllocLen, pv);
            }
            else
            {
                pstIntMemBuf->pvBase = pv;
                VPP_FLAG_SET(pstIntMemBuf->u32IntBufFlags, VPP_BUF_FLAG_HVX_REGISTERED);
                LOGI("remote_register_fd, fd=%d, sz=%u, new_ptr=%p",
                     pstIntMemBuf->fd, pstIntMemBuf->u32AllocLen, pv);
            }
        }
        else
            LOGE("unable to remote_register_fd, fd=%d, ptr=%p, sz=%u",
                 pstIntMemBuf->fd, pv, pstIntMemBuf->u32AllocLen);

    }
    else
    {
        if (remote_register_buf)
        {
            VPP_IP_PROF_START(&pstHvxCore->stBase, HVX_CORE_STAT_REGBUF);
            remote_register_buf(pstIntMemBuf->pvBase, pstIntMemBuf->u32MappedLen,
                                pstIntMemBuf->fd);
            VPP_IP_PROF_STOP(&pstHvxCore->stBase, HVX_CORE_STAT_REGBUF);

            VPP_FLAG_SET(pstIntMemBuf->u32IntBufFlags, VPP_BUF_FLAG_HVX_REGISTERED);

            LOGI("remote_register_buf, fd=%d, ptr=%p, sz=%u", pstIntMemBuf->fd,
                 pstIntMemBuf->pvBase, pstIntMemBuf->u32MappedLen);
        }
        else
            LOGE("unable to remote_register_buf, fd=%d, ptr=%p, sz=%u",
                 pstIntMemBuf->fd, pstIntMemBuf->pvBase, pstIntMemBuf->u32MappedLen);
    }
#else
    VPP_UNUSED(pstIntMemBuf);
    VPP_UNUSED(pstHvxCore);
#endif
}

void vVppIpHvxCore_UnregisterBuffer(t_StVppIpHvxCoreCb *pstHvxCore, t_StVppMemBuf *pstIntMemBuf)
{
    if (!pstHvxCore || !pstHvxCore->stBase.pstCtx || !pstIntMemBuf)
        return;

#ifndef HVX_DO_NOT_REGISTER_BUFFER
    if (!VPP_FLAG_IS_SET(pstIntMemBuf->u32IntBufFlags, VPP_BUF_FLAG_HVX_REGISTERED))
        return;

    if (remote_register_buf)
    {
        uint32_t u32MappedLen = pstIntMemBuf->u32MappedLen;
        if (pstIntMemBuf->eMapping == eVppBuf_Unmapped)
        {
            u32MappedLen = pstIntMemBuf->u32AllocLen;
        }

        VPP_IP_PROF_START(&pstHvxCore->stBase, HVX_CORE_STAT_UNREGBUF);
        remote_register_buf(pstIntMemBuf->pvBase, u32MappedLen, -1);
        VPP_IP_PROF_STOP(&pstHvxCore->stBase, HVX_CORE_STAT_UNREGBUF);

        if (pstIntMemBuf->eMapping == eVppBuf_Unmapped)
        {
            pstIntMemBuf->pvBase = VPP_BUF_UNMAPPED_BUF_VAL;
        }

        VPP_FLAG_CLR(pstIntMemBuf->u32IntBufFlags, VPP_BUF_FLAG_HVX_REGISTERED);

        LOGI("unregistering buffer, fd=%d, ptr=%p, sz=%u", pstIntMemBuf->fd,
             pstIntMemBuf->pvBase, u32MappedLen);
    }
#else
    VPP_UNUSED(pstIntMemBuf);
    VPP_UNUSED(pstHvxCore);
#endif
}

uint32_t bVppIpHvxCore_IsSecure(t_StVppIpHvxCoreCb *pstHvxCore)
{
    if(!pstHvxCore)
        return VPP_FALSE;

    return pstHvxCore->stBase.bSecureSession;
}

uint32_t u32VppIpHvxCore_SvcParamSetROI(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t bEnable,
                                        uint32_t bCompute, uint32_t u32XStart, uint32_t u32YStart,
                                        uint32_t u32XEnd, uint32_t u32YEnd, uint32_t u32LineWidth,
                                        uint32_t u32LineY, uint32_t u32LineCr, uint32_t u32LineCb)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->pstParams->gen_params.roi_en = bEnable;
    if (bCompute) {
        pstHvxCore->pstParams->gen_params.roi_x_tl = u32XStart;
        pstHvxCore->pstParams->gen_params.roi_y_tl = u32YStart;
        pstHvxCore->pstParams->gen_params.roi_x_br = u32XEnd;
        pstHvxCore->pstParams->gen_params.roi_y_br = u32YEnd;
        pstHvxCore->pstParams->gen_params.transit_reg_wid = u32LineWidth;
        pstHvxCore->pstParams->gen_params.transit_reg_yval = u32LineY;
        pstHvxCore->pstParams->gen_params.transit_reg_c1val = u32LineCr;
        pstHvxCore->pstParams->gen_params.transit_reg_c2val = u32LineCb;
    }
    return VPP_OK;
}

uint32_t u32VppIpHvxCore_SvcParamSetHeaderIdxAlgo(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32Index,
                                                  uint32_t u32Algo)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    (pstHvxCore->pstParams->header + u32Index)->vpp_func_id = u32Algo;
    (pstHvxCore->pstParams->header + u32Index)->process_flags = 1;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_SvcParamSetDataSize(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32AlgoCnt,
                                             uint32_t u32DataLength)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->pstParams->headerLen = u32AlgoCnt;
    pstHvxCore->pstParams->user_dataLen = u32DataLength;

    return VPP_OK;
}

vpp_svc_config_hdr_t* u32pVppIpHvxCore_SvcParamGetHeaderIdxAddr(t_StVppIpHvxCoreCb *pstHvxCore,
                                                                uint32_t u32Index)
{
    if(!pstHvxCore)
        return NULL;

    return (vpp_svc_config_hdr_t*)(pstHvxCore->pstParams->header + u32Index);
}

void* vpVppIpHvxCore_SvcParamGetDataOffsetAddr(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32Offset)
{
    if(!pstHvxCore)
        return NULL;

    return (void *)(pstHvxCore->pstParams->user_data + u32Offset);
}

uint32_t u32VppIpHvxCore_BufParamSetSize(t_StVppIpHvxCoreCb *pstHvxCore,
                                         uint32_t u32Width, uint32_t u32Height)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->stHvxBufParams.u32InWidth = u32Width;
    pstHvxCore->stHvxBufParams.u32InHeight = u32Height;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufParamGetSize(t_StVppIpHvxCoreCb *pstHvxCore,
                                         uint32_t *u32pWidth, uint32_t *u32pHeight)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    *u32pWidth = pstHvxCore->stHvxBufParams.u32InWidth;
    *u32pHeight = pstHvxCore->stHvxBufParams.u32InHeight;

    return VPP_OK;

}

uint32_t u32VppIpHvxCore_BufParamSetPixFmt(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32Format)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->stHvxBufParams.ePixFmt = u32Format;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufParamGetPixFmt(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t *u32pFormat)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    *u32pFormat = pstHvxCore->stHvxBufParams.ePixFmt;

    return VPP_OK;

}

uint32_t u32VppIpHvxCore_BufParamSetFldFmt(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32Format)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->stHvxBufParams.eFieldFmt = u32Format;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufParamGetFldFmt(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t *u32pFormat)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    *u32pFormat = pstHvxCore->stHvxBufParams.eFieldFmt;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufParamSetPlaneSize(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32Pl0,
                                              uint32_t u32Pl1, uint32_t u32Pl2)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->stHvxBufParams.u32Pl0SizeBytes = u32Pl0;
    pstHvxCore->stHvxBufParams.u32Pl1SizeBytes = u32Pl1;
    pstHvxCore->stHvxBufParams.u32Pl2SizeBytes = u32Pl2;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufParamGetPlaneSize(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t *u32pPl0,
                                              uint32_t *u32pPl1, uint32_t *u32pPl2)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    *u32pPl0 = pstHvxCore->stHvxBufParams.u32Pl0SizeBytes;
    *u32pPl1 = pstHvxCore->stHvxBufParams.u32Pl1SizeBytes;
    *u32pPl2 = pstHvxCore->stHvxBufParams.u32Pl2SizeBytes;

    return VPP_OK;

}

uint32_t u32VppIpHvxCore_BufParamSetPlaneStride(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32Pl0,
                                                uint32_t u32Pl1, uint32_t u32Pl2)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->stHvxBufParams.u32Pl0Stride = u32Pl0;
    pstHvxCore->stHvxBufParams.u32Pl1Stride = u32Pl1;
    pstHvxCore->stHvxBufParams.u32Pl2Stride = u32Pl2;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufParamGetPlaneStride(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t *u32pPl0,
                                                uint32_t *u32pPl1, uint32_t *u32pPl2)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    *u32pPl0 = pstHvxCore->stHvxBufParams.u32Pl0Stride;
    *u32pPl1 = pstHvxCore->stHvxBufParams.u32Pl1Stride;
    *u32pPl2 = pstHvxCore->stHvxBufParams.u32Pl2Stride;

    return VPP_OK;

}

uint32_t u32VppIpHvxCore_BuffInInit(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32NumBuffers)
{
    uint32_t u32PixDataAllocSize;
    uint32_t i;
    vpp_svc_frame_group_descriptor_t* pstBufferdataIn;

    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstBufferdataIn = pstHvxCore->pstBufferdataIn;
    u32PixDataAllocSize = sizeof(vpp_svc_raw_buffer_t) * u32NumBuffers;

    memset((void*)pstBufferdataIn, 0, sizeof(vpp_svc_frame_group_descriptor_t));

    pstBufferdataIn->pixel_data = (vpp_svc_raw_buffer_t*)HVX_ALLOC(u32PixDataAllocSize);
    if(!pstBufferdataIn->pixel_data)
        return VPP_ERR_NO_MEM;

    pstBufferdataIn->numbuffers = u32NumBuffers;
    pstBufferdataIn->pixel_dataLen = u32NumBuffers;

    for (i = 0; i < pstBufferdataIn->numbuffers; i++)
    {
        pstBufferdataIn->bufferattributes[i].index = i;
    }

    return VPP_OK;
}

void vVppIpHvxCore_BuffInTerm(t_StVppIpHvxCoreCb *pstHvxCore)
{
    if ((pstHvxCore) && (pstHvxCore->pstBufferdataIn))
    {
        if (pstHvxCore->pstBufferdataIn->pixel_data)
            free(pstHvxCore->pstBufferdataIn->pixel_data);

        pstHvxCore->pstBufferdataIn->pixel_data = NULL;
    }
}

uint32_t u32VppIpHvxCore_BuffInCompute(t_StVppIpHvxCoreCb *pstHvxCore)
{
    t_StHvxCoreBufParams *pstBufParams;
    vpp_svc_frame_group_descriptor_t* pstBufferdataIn;
    uint32_t u32FrameSize;
    uint32_t u32Stride0, u32Stride1, u32Stride2, u32Size0, u32Size1, u32Size2;
    uint32_t i;

    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstBufParams = &pstHvxCore->stHvxBufParams;
    pstBufferdataIn = pstHvxCore->pstBufferdataIn;
    u32VppIpHvxCore_BufParamGetPlaneSize(pstHvxCore, &u32Size0, &u32Size1, &u32Size2);
    u32VppIpHvxCore_BufParamGetPlaneStride(pstHvxCore, &u32Stride0, &u32Stride1, &u32Stride2);
    u32FrameSize = u32Size0 + u32Size1 + u32Size2;

    pstBufferdataIn->width = pstBufParams->u32InWidth;
    pstBufferdataIn->height = pstBufParams->u32InHeight;
    pstBufferdataIn->pixelformat = pstBufParams->ePixFmt;
    pstBufferdataIn->fieldformat = pstBufParams->eFieldFmt;
    for (i = 0; i < pstBufferdataIn->numbuffers; i++)
    {
        u32VppIpHvxCore_BufInSetAttrSize(pstHvxCore, i, u32Size0, u32Size1, u32Size2);
        u32VppIpHvxCore_BufInSetAttrStride(pstHvxCore, i, u32Stride0, u32Stride1, u32Stride2);
        u32VppIpHvxCore_BufInSetUserDataLen(pstHvxCore, i, u32FrameSize);

        LOGD("pstBufferdataIn[%d] len=%d", i, u32FrameSize);
        LOGD("u32Pl0Stride=%d u32Pl0SizeBytes=%d", u32Stride0, u32Size0);
        LOGD("u32Pl1Stride=%d u32Pl1SizeBytes=%d", u32Stride1, u32Size1);
        LOGD("u32Pl2Stride=%d u32Pl2SizeBytes=%d", u32Stride2, u32Size2);
    }

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufInSetSize(t_StVppIpHvxCoreCb *pstHvxCore,
                                         uint32_t u32Width, uint32_t u32Height)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->pstBufferdataIn->width = u32Width;
    pstHvxCore->pstBufferdataIn->height = u32Height;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufInSetPixFmt(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32Format)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->pstBufferdataIn->pixelformat = u32Format;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufInSetFldFmt(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32Format)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->pstBufferdataIn->fieldformat = u32Format;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufInSetNumBuffers(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32NumBuffers)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->pstBufferdataIn->numbuffers = u32NumBuffers;
    pstHvxCore->pstBufferdataIn->pixel_dataLen = u32NumBuffers;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufInSetAttrSize(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32Idx,
                                          uint32_t u32Pl0, uint32_t u32Pl1, uint32_t u32Pl2)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->pstBufferdataIn->bufferattributes[u32Idx].plane_sizebytes[PLANE_ID_Y] = u32Pl0;
    pstHvxCore->pstBufferdataIn->bufferattributes[u32Idx].plane_sizebytes[PLANE_ID_U] = u32Pl1;
    pstHvxCore->pstBufferdataIn->bufferattributes[u32Idx].plane_sizebytes[PLANE_ID_V] = u32Pl2;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufInSetAttrStride(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32Idx,
                                            uint32_t u32Pl0, uint32_t u32Pl1, uint32_t u32Pl2)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->pstBufferdataIn->bufferattributes[u32Idx].plane_stride[PLANE_ID_Y] = u32Pl0;
    pstHvxCore->pstBufferdataIn->bufferattributes[u32Idx].plane_stride[PLANE_ID_U] = u32Pl1;
    pstHvxCore->pstBufferdataIn->bufferattributes[u32Idx].plane_stride[PLANE_ID_V] = u32Pl2;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufInSetUserDataLen(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32Idx,
                                             uint32_t u32Size)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->pstBufferdataIn->pixel_data[u32Idx].user_dataLen = u32Size;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufInSetUserDataAddr(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32Idx,
                                              void* vpAddr)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->pstBufferdataIn->pixel_data[u32Idx].user_data = vpAddr;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BuffOutInit(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32NumBuffers)
{
    uint32_t u32PixDataAllocSize;
    vpp_svc_frame_group_descriptor_t* pstBufferdataOut;

    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    u32PixDataAllocSize = sizeof(vpp_svc_raw_buffer_t) * u32NumBuffers;
    pstBufferdataOut = pstHvxCore->pstBufferdataOut;

    memset((void*)pstBufferdataOut, 0, sizeof(vpp_svc_frame_group_descriptor_t));

    pstBufferdataOut->pixel_data = (vpp_svc_raw_buffer_t*)HVX_ALLOC(u32PixDataAllocSize);
    if(!pstBufferdataOut->pixel_data)
        return VPP_ERR_NO_MEM;

    pstBufferdataOut->numbuffers = u32NumBuffers;
    pstBufferdataOut->pixel_dataLen = u32NumBuffers;

    return VPP_OK;
}

void vVppIpHvxCore_BuffOutTerm(t_StVppIpHvxCoreCb *pstHvxCore)
{
    if ((pstHvxCore) && (pstHvxCore->pstBufferdataOut))
    {
        if (pstHvxCore->pstBufferdataOut->pixel_data)
            free(pstHvxCore->pstBufferdataOut->pixel_data);

        pstHvxCore->pstBufferdataOut->pixel_data = NULL;
    }
}

uint32_t u32VppIpHvxCore_BuffOutCompute(t_StVppIpHvxCoreCb *pstHvxCore)
{
    t_StHvxCoreBufParams *pstBufParams;
    vpp_svc_frame_group_descriptor_t* pstBufferdataOut;
    uint32_t u32FrameSize;
    uint32_t u32Size0, u32Size1, u32Size2;
    uint32_t i;

    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstBufParams = &pstHvxCore->stHvxBufParams;
    pstBufferdataOut = pstHvxCore->pstBufferdataOut;
    u32VppIpHvxCore_BufParamGetPlaneSize(pstHvxCore, &u32Size0, &u32Size1, &u32Size2);
    u32FrameSize = u32Size0 + u32Size1 + u32Size2;

    for (i = 0; i < pstBufferdataOut->numbuffers; i++)
    {
        u32VppIpHvxCore_BufOutSetUserDataLen(pstHvxCore, i, u32FrameSize);
        LOGD("pstBufferdataOut[%d] len=%d", i, u32FrameSize);
    }

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufOutSetNumBuffers(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32NumBuffers)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->pstBufferdataOut->numbuffers = u32NumBuffers;
    pstHvxCore->pstBufferdataOut->pixel_dataLen = u32NumBuffers;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufOutSetUserDataLen(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32Idx,
                                              uint32_t u32Size)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->pstBufferdataOut->pixel_data[u32Idx].user_dataLen = u32Size;

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_BufOutSetUserDataAddr(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32Idx,
                                               void* vpAddr)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstHvxCore->pstBufferdataOut->pixel_data[u32Idx].user_data = vpAddr;

    return VPP_OK;
}

int VppIpHvxCore_Process(t_StVppIpHvxCoreCb *pstHvxCore)
{
    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    return vpp_svc_process(pstHvxCore->pstParams, pstHvxCore->pstContext->user_data,
                           pstHvxCore->pstContext->user_dataLen,
                           (const vpp_svc_frame_group_descriptor_t*)pstHvxCore->pstBufferdataIn,
                           pstHvxCore->pstBufferdataOut);
}

uint32_t u32VppIpHvxCore_AlgoInit(t_StVppIpHvxCoreCb *pstHvxCore, uint32_t u32Index,
                                  uint32_t u32Algo, const char *cpAlgoLibName)
{
    vpp_svc_cap_resource_list_t* pstCapabilityResources;

    VPP_RET_IF_NULL(pstHvxCore, VPP_ERR_PARAM);

    pstCapabilityResources = pstHvxCore->pstCapabilityResources;
    pstCapabilityResources->resource[u32Index].vpp_func_id = u32Algo;
    pstCapabilityResources->resource[u32Index].feature_flags = 1;
    if(cpAlgoLibName)
        strlcpy(pstCapabilityResources->resource[u32Index].name_loc, cpAlgoLibName,
                HVX_LIB_NAME_LEN);

    return VPP_OK;
}

uint32_t u32VppIpHvxCore_PrepareCtx(t_StVppIpHvxCoreCb *pstHvxCore)
{
    int rc = 0;

    VPP_IP_PROF_START(&pstHvxCore->stBase, HVX_CORE_STAT_SVC_PREPARE_CTX);

    rc = vpp_svc_prepare_ctx((unsigned char*)pstHvxCore->pstContext->user_data,
                             pstHvxCore->ctx.u32Length,
                             pstHvxCore->ctx.u32FrameSizeBytes,
                             pstHvxCore->ctx.u32ProcessingFlags,
                             pstHvxCore->ctx.u32CtxSz,
                             pstHvxCore->ctx.u32DiagCtxSz);

    VPP_IP_PROF_STOP(&pstHvxCore->stBase, HVX_CORE_STAT_SVC_PREPARE_CTX);

    if (rc)
    {
        LOGE("Error: context config failed, rc=%d", rc);
        return VPP_ERR_HW;
    }

    return VPP_OK;
}


uint32_t u32VppIpHvxCore_Open(t_StVppIpHvxCoreCb *pstHvxCore,
                              uint32_t vpp_processing_flags,
                              uint32_t framesize_bytes)
{
    int rc = 0;
    uint32_t u32Ret = VPP_OK;
    uint32_t u32;
    uint32_t u32RequiredLength;
    uint32_t i;
    int32_t vpp_ctx_size = 0;
    vpp_svc_cap_resource_list_t* pstCapabilityResources;
    vpp_svc_context_t* pstContext;

    if(!pstHvxCore || !pstHvxCore->stBase.pstCtx)
        return VPP_ERR_PARAM;

    VPP_IP_PROF_START(&pstHvxCore->stBase, HVX_CORE_STAT_CORE_OPEN);

    VPP_IP_PROF_START(&pstHvxCore->stBase, HVX_CORE_STAT_SET_DBG_LVL);
    vVppIpHvxCore_SetDebugLevels(pstHvxCore);
    VPP_IP_PROF_STOP(&pstHvxCore->stBase, HVX_CORE_STAT_SET_DBG_LVL);

    LOGD("flags=0x%x", vpp_processing_flags);

    pstCapabilityResources = pstHvxCore->pstCapabilityResources;
    pstContext = pstHvxCore->pstContext;

    LOGI("calling vpp_svc_init");

    VPP_IP_PROF_START(&pstHvxCore->stBase, HVX_CORE_STAT_SVC_INIT);
    rc = vpp_svc_init(pstCapabilityResources);
    VPP_IP_PROF_STOP(&pstHvxCore->stBase, HVX_CORE_STAT_SVC_INIT);
    if (rc != 0)
    {
        u32Ret = VPP_ERR_HW;
        LOGE("vpp_svc_init() error, rc=%d", rc);
        goto ERR_VPP_SVC_INIT;
    }
    LOGI("vpp_svc_init() successful");

    // Get the version information from the firmware. Must be called after vpp_svc_init()!
    VPP_IP_PROF_START(&pstHvxCore->stBase, HVX_CORE_STAT_GET_VER_INFO);
    vVppIpHvxCore_GetVersionInfo(pstHvxCore);
    VPP_IP_PROF_STOP(&pstHvxCore->stBase, HVX_CORE_STAT_GET_VER_INFO);

    u32Ret = u32VppIpHvxCore_HandleSessionMigration(pstHvxCore);
    if (u32Ret != VPP_OK)
    {
        LOGE("session migration failed, u32Ret=%u", u32Ret);
        goto ERR_VPP_MIGRATE_SESSION;
    }

    // Calculate context size and allocate context
    VPP_IP_PROF_START(&pstHvxCore->stBase, HVX_CORE_STAT_SVC_GET_CTX);
    vpp_ctx_size = vpp_svc_get_ctx_size(vpp_processing_flags, framesize_bytes);
    VPP_IP_PROF_STOP(&pstHvxCore->stBase, HVX_CORE_STAT_SVC_GET_CTX);

    if (vpp_ctx_size < 0)
    {
        LOGE("get_ctx_sz returned invalid, ctx_sz=%d", vpp_ctx_size);
        u32Ret = VPP_ERR_HW;
        goto ERR_GET_CTX_SZ;
    }

    u32RequiredLength = (vpp_ctx_size + 127) & (~127);

    LOGD("framesize_bytes=%d, vpp_ctx_size=%d, u32RequiredLength=%u",
         framesize_bytes, vpp_ctx_size, u32RequiredLength);

    // if needs to allocate ion buffer
    if (pstHvxCore->ctx.bAllocated &&
        pstHvxCore->ctx.u32AllocatedSize < u32RequiredLength)
    {
        LOGD("freeing previously allocated ctx (sz=%u) for new ctx (sz=%u)",
             pstHvxCore->ctx.u32AllocatedSize, u32RequiredLength);
        u32VppIpHvxCore_FreeContext(pstHvxCore);
    }

    if (!pstHvxCore->ctx.bAllocated)
    {
        u32 = u32VppIpHvxCore_AllocateContext(pstHvxCore, u32RequiredLength);
        if (u32 != VPP_OK)
        {
            LOGE("ERROR: AllocateContext failed, u32=%u", u32);
            goto ERR_ALLOC;
        }
    }

    pstHvxCore->ctx.u32Length = u32RequiredLength;
    pstHvxCore->ctx.u32FrameSizeBytes = framesize_bytes;
    pstHvxCore->ctx.u32ProcessingFlags = vpp_processing_flags;
    pstHvxCore->ctx.u32CtxSz = vpp_ctx_size;
    pstHvxCore->ctx.u32DiagCtxSz = 0;

    //Prepare context for use
    u32 = u32VppIpHvxCore_PrepareCtx(pstHvxCore);
    if(u32 != VPP_OK)
    {
        LOGE("Error: context config failed, rc=%d", rc);
        u32Ret = VPP_ERR_HW;
        goto ERR_PREPARE_CTX;
    }

    VPP_IP_PROF_STOP(&pstHvxCore->stBase, HVX_CORE_STAT_CORE_OPEN);
    return VPP_OK;

ERR_PREPARE_CTX:
    u32VppIpHvxCore_FreeContext(pstHvxCore);

ERR_ALLOC:
ERR_GET_CTX_SZ:
ERR_VPP_MIGRATE_SESSION:
    VPP_IP_PROF_START(&pstHvxCore->stBase, HVX_CORE_STAT_SVC_DEINIT);
    rc = vpp_svc_deinit(pstCapabilityResources);
    VPP_IP_PROF_STOP(&pstHvxCore->stBase, HVX_CORE_STAT_SVC_DEINIT);
    if (rc != 0)
        LOGE("vpp_svc_deinit() failed, rc=%d", rc);

ERR_VPP_SVC_INIT:
    VPP_IP_PROF_STOP(&pstHvxCore->stBase, HVX_CORE_STAT_CORE_OPEN);
    return u32Ret;
}

uint32_t u32VppIpHvxCore_Close(t_StVppIpHvxCoreCb *pstHvxCore)
{
    uint32_t u32Ret = VPP_OK;
    int rc = 0;
    vpp_svc_cap_resource_list_t* pstCapabilityResources;

    LOGD("%s()", __func__);

    if (!pstHvxCore || ! pstHvxCore->stBase.pstCtx)
        return VPP_ERR_PARAM;

    pstCapabilityResources = pstHvxCore->pstCapabilityResources;

    rc = vpp_svc_deinit(pstCapabilityResources);
    if(rc)
    {
        LOGE("Error: HVX close failed rc=%d", rc);
        u32Ret = VPP_ERR_HW;
    }

    return u32Ret;
}

t_StVppIpHvxCoreCb *u32pVppIpHvxCore_Init(t_StVppCtx *pstCtx, uint32_t u32Flags,
                                          uint32_t u32CtrlCnt, uint32_t u32UserDataSize)
{
    LOGI("%s", __func__);

    uint32_t u32;
    t_StVppCallback cbs;
    t_StVppIpHvxCoreCb *pstHvxCore = NULL;
    uint32_t u32Length;

    memset(&cbs, 0, sizeof(t_StVppCallback));

    pstHvxCore = calloc(sizeof(t_StVppIpHvxCoreCb), 1);
    if (!pstHvxCore)
    {
        LOGE("calloc failed for hvx core");
        return NULL;
    }

    u32VppIp_SetBase(pstCtx, u32Flags, cbs, &pstHvxCore->stBase);

    u32 = VPP_IP_PROF_REGISTER(&pstHvxCore->stBase, astHvxCoreStatsCfg, u32HvxCoreStatCnt);
    LOGE_IF(u32 != VPP_OK, "ERROR: unable to register stats, u32=%u", u32);

    vVppIpHvxCore_ReadProperties(pstHvxCore);

    pstHvxCore->pstCapabilityResources = (vpp_svc_cap_resource_list_t *)
        HVX_ALLOC(sizeof(vpp_svc_cap_resource_list_t));
    if (pstHvxCore->pstCapabilityResources == NULL)
    {
        LOGE("Error: pstCapabilityResources alloc failed\n");
        goto ERROR_CAPABILITY;
    }

    pstHvxCore->pstCapabilityResources->resourceLen = u32CtrlCnt;
    u32Length = sizeof(vpp_svc_cap_resource_t) * u32CtrlCnt;
    pstHvxCore->pstCapabilityResources->resource =
        (vpp_svc_cap_resource_t*)HVX_ALLOC(u32Length);
    if (pstHvxCore->pstCapabilityResources->resource == NULL)
    {
        LOGE("Error: pstCapabilityResources->resource alloc faild\n");
        goto ERROR_CAPABILITY_RES;
    }

    pstHvxCore->pstBufferdataIn = (vpp_svc_frame_group_descriptor_t*)
        HVX_ALLOC(sizeof(vpp_svc_frame_group_descriptor_t));
    if (pstHvxCore->pstBufferdataIn == NULL)
    {
        LOGE("Error: pstBufferdataIn alloc failed\n");
        goto ERROR_BUFFER_IN;
    }

    pstHvxCore->pstBufferdataOut = (vpp_svc_frame_group_descriptor_t*)
        HVX_ALLOC(sizeof(vpp_svc_frame_group_descriptor_t));
    if (pstHvxCore->pstBufferdataOut == NULL)
    {
        LOGE("Error: pstBufferdataOut alloc failed\n");
        goto ERROR_BUFFER_OUT;
    }

    pstHvxCore->pstContext = (vpp_svc_context_t*)HVX_ALLOC(sizeof(vpp_svc_context_t));
    if (pstHvxCore->pstContext == NULL)
    {
        LOGE("Error: pstHvxCore->pstContext alloc failed\n");
        goto ERROR_CONTEXT;
    }

    pstHvxCore->pstParams = (vpp_svc_params_t*)HVX_ALLOC(sizeof(vpp_svc_params_t));
    if (pstHvxCore->pstParams == NULL)
    {
        LOGE("Error: pstHvxCore->pstParams alloc failed\n");
        goto ERROR_PARAMS;
    }

    u32Length = sizeof(vpp_svc_config_hdr_t) * u32CtrlCnt;
    pstHvxCore->pstParams->header = (vpp_svc_config_hdr_t*)HVX_ALLOC(u32Length);
    if (pstHvxCore->pstParams->header == NULL)
    {
        LOGE("Error: pstParams->header alloc failed\n");
        goto ERROR_HEADER;
    }

    pstHvxCore->pstParams->user_data = (unsigned char*)HVX_ALLOC(u32UserDataSize);
    if (pstHvxCore->pstParams->user_data == NULL)
    {
        LOGE("Error: alloc pstParams->user_data failed\n");
        goto ERROR_USERDATA;
    }

    return pstHvxCore;

ERROR_USERDATA:
    if(pstHvxCore->pstParams->header)
        free(pstHvxCore->pstParams->header);

ERROR_HEADER:
    if(pstHvxCore->pstParams)
        free(pstHvxCore->pstParams);

ERROR_PARAMS:
    if(pstHvxCore->pstContext)
        free(pstHvxCore->pstContext);

ERROR_CONTEXT:
    if(pstHvxCore->pstBufferdataOut)
        free(pstHvxCore->pstBufferdataOut);

ERROR_BUFFER_OUT:
    if(pstHvxCore->pstBufferdataIn)
        free(pstHvxCore->pstBufferdataIn);

ERROR_BUFFER_IN:
    if(pstHvxCore->pstCapabilityResources->resource)
        free(pstHvxCore->pstCapabilityResources->resource);

ERROR_CAPABILITY_RES:
    if(pstHvxCore->pstCapabilityResources)
        free(pstHvxCore->pstCapabilityResources);

ERROR_CAPABILITY:
    u32 = VPP_IP_PROF_UNREGISTER(&pstHvxCore->stBase);
    LOGE_IF(u32 != VPP_OK, "ERROR: unable to unregister stats, u32=%u", u32);

    if (pstHvxCore)
        free(pstHvxCore);

    return NULL;
}

void vVppIpHvxCore_Term(t_StVppIpHvxCoreCb *pstHvxCore)
{
    uint32_t u32;

    LOGI("%s\n", __func__);

    VPP_RET_VOID_IF_NULL(pstHvxCore);

    u32VppIpHvxCore_FreeContext(pstHvxCore);

    u32 = VPP_IP_PROF_UNREGISTER(&pstHvxCore->stBase);
    LOGE_IF(u32 != VPP_OK, "ERROR: unable to unregister stats, u32=%u", u32);

    if (pstHvxCore->pstBufferdataOut)
    {
        free(pstHvxCore->pstBufferdataOut);
        pstHvxCore->pstBufferdataOut = NULL;
    }

    if (pstHvxCore->pstBufferdataIn)
    {
        free(pstHvxCore->pstBufferdataIn);
        pstHvxCore->pstBufferdataIn = NULL;
    }

    if (pstHvxCore->pstParams)
    {
        if (pstHvxCore->pstParams->user_data)
            free(pstHvxCore->pstParams->user_data);

        if (pstHvxCore->pstParams->header)
            free(pstHvxCore->pstParams->header);

        free(pstHvxCore->pstParams);
        pstHvxCore->pstParams = NULL;
    }

    if (pstHvxCore->pstContext)
    {
        free(pstHvxCore->pstContext);
        pstHvxCore->pstContext = NULL;
    }

    if (pstHvxCore->pstCapabilityResources)
    {
        if (pstHvxCore->pstCapabilityResources->resource)
            free(pstHvxCore->pstCapabilityResources->resource);
        free(pstHvxCore->pstCapabilityResources);
        pstHvxCore->pstCapabilityResources = NULL;
    }

    free(pstHvxCore);
}
