/*!
 * @file vpp_buf.c
 *
 * @cr
 * Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

 * @services
 *
 *
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#ifndef VPP_GRALLOC_DOES_NOT_EXIST
#include <qdMetaData.h>
#endif

#include <media/msm_vidc.h>
#ifndef OMX_EXTRADATA_DOES_NOT_EXIST
#include "OMX_Core.h"
#include "OMX_QCOMExtns.h"
#endif

#define VPP_LOG_TAG     VPP_LOG_MODULE_BUF_TAG
#define VPP_LOG_MODULE  VPP_LOG_MODULE_BUF
#include "vpp_dbg.h"

#include "vpp_def.h"
#include "vpp.h"
#include "vpp_core.h"
#include "vpp_buf.h"
#include "vpp_stats.h"
#include "vpp_uc.h"
#include "vpp_utils.h"

/************************************************************************
 * Local definitions
 ***********************************************************************/
#define GRALLOC_HDL(_pb) ((struct private_handle_t *)_pb->pBuf->pvGralloc)

// #define VPP_BUF_DBG_POOL

#define VPP_BUF_MAP_BUF_PX 0x1
#define VPP_BUF_MAP_BUF_EX 0x2
#define VPP_BUF_MAP_BUF_ALL (VPP_BUF_MAP_BUF_PX | VPP_BUF_MAP_BUF_EX)

#define PAGE_SZ (sysconf(_SC_PAGE_SIZE))

#define VPP_DBG_BUF_STR(_which) #_which ":{fd=%d, u32AllocLen=%u, u32FilledLen=%u, u32Offset=%u, u32ValidLen=%u}"
#define VPP_DBG_BUF_VAL(_val) _val.fd, _val.u32AllocLen, _val.u32FilledLen, _val.u32Offset, _val.u32ValidLen
#define VPP_DBG_MEM_BUF(_lvl, _pmb, _msg) \
    LOG##_lvl("%s --> fd=%d, u32AllocLen=%u, u32FilledLen=%u, pvBase=%p, u32Offset=%u, " \
              "u32MappedLen=%u, u32ValidLen=%u, pvPa=%p, u32PaOff=%u, u32PaMappedLen=%u", \
              _msg, \
              (_pmb)->fd, \
              (_pmb)->u32AllocLen, \
              (_pmb)->u32FilledLen, \
              (_pmb)->pvBase, \
              (_pmb)->u32Offset, \
              (_pmb)->u32MappedLen, \
              (_pmb)->u32ValidLen, \
              (_pmb)->priv.pvPa, \
              (_pmb)->priv.u32PaOffset, \
              (_pmb)->priv.u32PaMappedLen)

#define VPP_BUF_MAP(_var, _this, _that) \
    case _this: _var = _that; break

/************************************************************************
 * Local static variables
 ***********************************************************************/

/************************************************************************
 * Forward Declarations
 ************************************************************************/

/************************************************************************
 * Local Functions
 ***********************************************************************/
#ifdef VPP_CACHE_MMAPPED_PX_BUF
#define CACHE_NODE_STR "fd=%d, pv=%p"
#define CACHE_NODE_PR(n) n->fd, n->pv

extern void remote_register_buf(void* buf, int size, int fd);
#pragma weak remote_register_buf

static void *vpVppBuf_CacheBuffer(t_StVppBufCb *pstBufCb,
                                  struct vpp_mem_buffer *pstMemBuf)
{
    uint32_t i;
    int32_t s32FreeIdx = -1;

    t_StVppBufMapCacheNode *pstCacheNode;

    for (i = 0; i < VPP_INTERNAL_BUF_MAX; i++)
    {
        pstCacheNode = &pstBufCb->astCacheNode[i];
        if (pstCacheNode->fd == 0 && s32FreeIdx < 0)
        {
            s32FreeIdx = i;
        }
        else if (pstCacheNode->fd == pstMemBuf->fd)
        {
            LOGD("cache: found cached node at idx=%u, " CACHE_NODE_STR, i,
                 CACHE_NODE_PR(pstCacheNode));
            return pstCacheNode->pv;
        }
    }

    if (i == VPP_INTERNAL_BUF_MAX && s32FreeIdx >= 0)
    {
        pstCacheNode = &pstBufCb->astCacheNode[s32FreeIdx];

        pstCacheNode->pv = mmap(NULL, pstMemBuf->alloc_len,
                                PROT_READ | PROT_WRITE, MAP_SHARED,
                                pstMemBuf->fd, pstMemBuf->offset);

        if (pstCacheNode->pv != MAP_FAILED)
        {
            pstCacheNode->fd = pstMemBuf->fd;
            pstCacheNode->u32MapLen = pstMemBuf->alloc_len;
            pstBufCb->u32CacheCnt += 1;

            LOGD("cache: cached at idx=%d, " CACHE_NODE_STR " u32CacheCnt=%u",
                 s32FreeIdx, CACHE_NODE_PR(pstCacheNode), pstBufCb->u32CacheCnt);

            if (remote_register_buf)
            {
                remote_register_buf(pstCacheNode->pv,
                                    pstCacheNode->u32MapLen,
                                    pstCacheNode->fd);
                LOGD("cache: registering buffer len=%d, " CACHE_NODE_STR,
                     pstMemBuf->alloc_len, CACHE_NODE_PR(pstCacheNode));


            }

            return pstCacheNode->pv;
        }
        else
        {
            LOGE("cache: failed to cache at idx=%d, " CACHE_NODE_STR, s32FreeIdx,
                 CACHE_NODE_PR(pstCacheNode));

            pstCacheNode->fd = 0;
            pstCacheNode->pv = NULL;

            return MAP_FAILED;
        }
    }
    LOGE("unable to find free node to cache, fd=%d", pstMemBuf->fd);
    return NULL;
}

static void vVppBuf_FreeBufferCache(t_StVppBufCb *pstBufCb)
{
    uint32_t i;
    t_StVppBufMapCacheNode *pstCacheNode;
    for (i = 0; i < VPP_INTERNAL_BUF_MAX; i++)
    {

        pstCacheNode = &pstBufCb->astCacheNode[i];
        if (pstCacheNode->fd != 0 && pstCacheNode->pv)
        {
            if (remote_register_buf)
            {
                remote_register_buf(pstCacheNode->pv,
                                    pstCacheNode->u32MapLen,
                                    -1);
                LOGD("cache: unregistered buffer, " CACHE_NODE_STR
                     " idx=%u", CACHE_NODE_PR(pstCacheNode), i);
            }
            memset(pstCacheNode, 0, sizeof(t_StVppBufMapCacheNode));
        }
    }
}
#endif

static void vVppBuf_SetMemBuf(t_StVppMemBuf *pstBuf,
                              void *pv, uint32_t u32MappedLen,
                              void *pvPa, uint32_t u32PaOffset,
                              uint32_t u32PaMappedLen, t_EVppBufMapping eMap)
{
    if (!pstBuf)
        return;

    pstBuf->pvBase = pv;
    pstBuf->u32MappedLen = u32MappedLen;

    pstBuf->priv.pvPa = pvPa;
    pstBuf->priv.u32PaOffset = u32PaOffset;
    pstBuf->priv.u32PaMappedLen = u32PaMappedLen;

    pstBuf->eMapping = eMap;
}

static uint32_t u32VppBuf_MapMemBuf(t_StVppMemBuf *pstMemBuf)
{
    uint32_t u32PaOff, u32PaMapLen;
    void *pvPa = MAP_FAILED;

    if (!pstMemBuf || !pstMemBuf->fd)
        return VPP_ERR_PARAM;

    u32PaOff = pstMemBuf->u32Offset & ~(PAGE_SZ - 1);
    u32PaMapLen = pstMemBuf->u32ValidLen + pstMemBuf->u32Offset - u32PaOff;

    if (u32PaMapLen)
    {
        pvPa = mmap(NULL, u32PaMapLen, PROT_READ | PROT_WRITE, MAP_SHARED,
                    pstMemBuf->fd, u32PaOff);
    }

    if (pvPa != MAP_FAILED)
    {
        LOGI("u32PaOff=%u, u32PaMapLen=%u, pvPa=%p",
             u32PaOff, u32PaMapLen, pvPa);

        vVppBuf_SetMemBuf(pstMemBuf,
                          ((char *)pvPa) + u32PaOff - pstMemBuf->u32Offset,
                          pstMemBuf->u32ValidLen,
                          pvPa,
                          u32PaOff,
                          u32PaMapLen,
                          eVppBuf_MappedInternal);

        return VPP_OK;
    }
    else
    {
        LOGE("mmap separate failed, fd=%d, u32PaMapLen=%u, u32PaOff=%u, err=%s",
             pstMemBuf->fd, u32PaMapLen, u32PaOff, strerror(errno));
    }

    return VPP_ERR;
}

static void vVppBuf_UnmapMemBuf(t_StVppMemBuf *pstMemBuf)
{
    int rc;

    if (!pstMemBuf)
    {
        LOGE("ERROR: unable to munmap, pstMemBuf=%p", pstMemBuf);
        return;
    }

    if (!pstMemBuf->priv.pvPa ||
        !pstMemBuf->priv.u32PaMappedLen ||
        pstMemBuf->priv.pvPa == VPP_BUF_UNMAPPED_BUF_VAL)
    {
        LOGE("ERROR: unable to munmap: pvPa=%p, u32PaMappedLen=%u",
             pstMemBuf->priv.pvPa, pstMemBuf->priv.u32PaMappedLen);
        return;
    }

    rc = munmap(pstMemBuf->priv.pvPa, pstMemBuf->priv.u32PaMappedLen);
    if (rc)
        LOGE("ERROR: unable to munmap, pvBase=%p, u32PaMappedLen=%u, err=%s",
             pstMemBuf->priv.pvPa, pstMemBuf->priv.u32PaMappedLen, strerror(errno));
    else
        LOGI("unmapping: pvPa=%p, u32PaMappedLen=%u",
             pstMemBuf->priv.pvPa, pstMemBuf->priv.u32PaMappedLen);

    vVppBuf_SetMemBuf(pstMemBuf, NULL, 0, NULL, 0, 0, eVppBuf_Unmapped);
}

static void vVppBuf_TranslateBufExtToInt(t_StVppBufCb *pstBufCb, t_StVppBuf *pstBuf)
{
    if (!pstBufCb || !pstBuf)
        return;

    // Pixel
    pstBuf->stPixel.fd = pstBuf->pBuf->pixel.fd;
    pstBuf->stPixel.u32AllocLen = pstBuf->pBuf->pixel.alloc_len;
    pstBuf->stPixel.u32FilledLen = pstBuf->pBuf->pixel.filled_len;
    pstBuf->stPixel.u32Offset = pstBuf->pBuf->pixel.offset;
    pstBuf->stPixel.u32ValidLen = pstBuf->pBuf->pixel.valid_data_len;

    // Extradata
    pstBuf->stExtra.fd = pstBuf->pBuf->extradata.fd;
    pstBuf->stExtra.u32AllocLen = pstBuf->pBuf->extradata.alloc_len;
    pstBuf->stExtra.u32FilledLen = pstBuf->pBuf->extradata.filled_len;
    pstBuf->stExtra.u32Offset = pstBuf->pBuf->extradata.offset;
    pstBuf->stExtra.u32ValidLen = pstBuf->pBuf->extradata.valid_data_len;

    if (pstBuf->pBuf->extradata.pvMapped)
    {
        uint32_t u32Offset = pstBuf->stExtra.u32Offset;
        vVppBuf_SetMemBuf(&pstBuf->stExtra,
                          ((char *)pstBuf->pBuf->extradata.pvMapped) + u32Offset,
                          pstBuf->stExtra.u32ValidLen,
                          pstBuf->pBuf->extradata.pvMapped,
                          0,
                          pstBuf->stExtra.u32AllocLen,
                          eVppBuf_MappedExternal);
    }
    else if (!pstBufCb->bIsSecure && pstBuf->eQueuedPort == VPP_PORT_OUTPUT)
    {
        if (pstBuf->stExtra.fd == pstBuf->stPixel.fd &&
            pstBuf->stExtra.u32Offset == pstBuf->stPixel.u32Offset)
        {
            pstBuf->stExtra.u32Offset += pstBufCb->u32OutExOff;
            pstBuf->stPixel.u32ValidLen = pstBufCb->u32OutExOff;
            pstBuf->stExtra.u32ValidLen = pstBuf->stPixel.u32AllocLen -
                pstBuf->stPixel.u32ValidLen - pstBuf->stPixel.u32Offset;
        }
    }
}

static void vVppBuf_TranslateBufIntToExt(t_StVppBufCb *pstBufCb, t_StVppBuf *pstBuf)
{
    if (!pstBufCb || !pstBuf)
        return;

    // Pixel
    pstBuf->pBuf->pixel.fd = pstBuf->stPixel.fd;
    pstBuf->pBuf->pixel.alloc_len = pstBuf->stPixel.u32AllocLen;
    pstBuf->pBuf->pixel.filled_len = pstBuf->stPixel.u32FilledLen;
    pstBuf->pBuf->pixel.offset = pstBuf->stPixel.u32Offset;
    pstBuf->pBuf->pixel.valid_data_len = pstBuf->stPixel.u32ValidLen;

    // Extradata
    pstBuf->pBuf->extradata.fd = pstBuf->stExtra.fd;
    pstBuf->pBuf->extradata.alloc_len = pstBuf->stExtra.u32AllocLen;
    pstBuf->pBuf->extradata.filled_len = pstBuf->stExtra.u32FilledLen;
    pstBuf->pBuf->extradata.offset = pstBuf->stExtra.u32Offset;
    pstBuf->pBuf->extradata.valid_data_len = pstBuf->stExtra.u32ValidLen;
}

static void vVppBuf_MapIntBuf(t_StVppBufCb *pstBufCb, t_StVppBuf *pstBuf,
                              uint32_t u32MapFlags)
{
    uint32_t u32Px = VPP_OK, u32Ex = VPP_OK, bMt = VPP_FALSE;
    uint32_t u32MapLen = 0, u32PaOff = 0, u32PaMapLen = 0, u32ExpOff = 0;
    void *pvPa = MAP_FAILED, *pvPx, *pvEx;
    t_StVppMemBuf *pstIntPx, *pstIntEx;

    if (!pstBufCb || !pstBuf)
        return;

    pstIntPx = &pstBuf->stPixel;
    pstIntEx = &pstBuf->stExtra;

    u32ExpOff = pstBuf->eQueuedPort == VPP_PORT_INPUT ?
        pstBufCb->u32InExOff : pstBufCb->u32OutExOff;

    if (pstBuf->stPixel.fd == pstBuf->stExtra.fd &&
        pstBuf->stExtra.u32Offset > pstBuf->stPixel.u32Offset &&
        ((pstBuf->stExtra.u32Offset - pstBuf->stPixel.u32Offset) == u32ExpOff))
        bMt = VPP_TRUE;

    LOGI("port=%u, mt=%d, " VPP_DBG_BUF_STR(px) " " VPP_DBG_BUF_STR(ex),
         pstBuf->eQueuedPort, bMt,
         VPP_DBG_BUF_VAL(pstBuf->stPixel),
         VPP_DBG_BUF_VAL(pstBuf->stExtra));

    if (pstBufCb->bIsSecure)
    {
        VPP_FLAG_CLR(u32MapFlags, VPP_BUF_MAP_BUF_PX);
    }

    if (u32VppBuf_IsEmptyEos(pstBuf))
    {
        LOGI("found empty eos, not mapping extradata");
        VPP_FLAG_CLR(u32MapFlags, VPP_BUF_MAP_BUF_EX);
    }

    if (pstIntEx->eMapping == eVppBuf_MappedExternal)
    {
        LOGI("found externally mapped extradata buffer, not mapping extradata");
        VPP_FLAG_CLR(u32MapFlags, VPP_BUF_MAP_BUF_EX);
    }

    if (u32MapFlags && !(u32MapFlags ^ VPP_BUF_MAP_BUF_ALL) && bMt && pstIntPx->fd >= 0)
    {
        // Can map pixel and extradata together
        u32PaOff = pstIntPx->u32Offset & ~(PAGE_SZ - 1);

        u32PaMapLen = pstIntPx->u32ValidLen + pstIntEx->u32ValidLen;
        u32PaMapLen += pstIntPx->u32Offset - u32PaOff;

        if (u32PaMapLen)
            pvPa = mmap(NULL, u32PaMapLen, PROT_READ | PROT_WRITE, MAP_SHARED,
                        pstIntPx->fd, u32PaOff);

        if (pvPa != MAP_FAILED)
        {
            pvPx = ((char *)pvPa) + u32PaOff;
            pvEx = ((char *)pvPa) + u32PaOff + pstIntEx->u32Offset;

            vVppBuf_SetMemBuf(pstIntPx,
                              pvPx, pstIntPx->u32ValidLen,
                              pvPa, u32PaOff, u32PaMapLen,
                              eVppBuf_MappedInternal);

            vVppBuf_SetMemBuf(pstIntEx,
                              pvEx, pstIntEx->u32ValidLen,
                              pvPa, u32PaOff, u32PaMapLen,
                              eVppBuf_MappedInternal);

            pstBuf->eMapType = eVppMapType_PxExTogether;

            VPP_FLAG_CLR(u32MapFlags, VPP_BUF_MAP_BUF_ALL);
        }
        else
        {
            LOGE("map together failed, fd=%d, u32PaMapLen=%u, u32PaOff=%u, err=%s",
                 pstIntPx->fd, u32PaMapLen, u32PaOff, strerror(errno));
        }
    }

    if (VPP_FLAG_IS_SET(u32MapFlags, VPP_BUF_MAP_BUF_PX) && pstIntPx->fd >= 0)
    {
        pstBuf->eMapType = eVppMapType_PxExSeparate;
        u32Px = u32VppBuf_MapMemBuf(pstIntPx);
        VPP_FLAG_CLR(u32MapFlags, VPP_BUF_MAP_BUF_PX);
    }

    if (VPP_FLAG_IS_SET(u32MapFlags, VPP_BUF_MAP_BUF_EX) && pstIntEx->fd >= 0)
    {
        pstBuf->eMapType = eVppMapType_PxExSeparate;
        u32Ex = u32VppBuf_MapMemBuf(pstIntEx);
        VPP_FLAG_CLR(u32MapFlags, VPP_BUF_MAP_BUF_EX);
    }

    VPP_DBG_MEM_BUF(I, pstIntPx, (u32Px ? "px-error" : "px"));
    VPP_DBG_MEM_BUF(I, pstIntEx, (u32Ex ? "ex-error" : "ex"));
}

static void vVppBuf_UnmapIntBuf(t_StVppBuf *pstBuf)
{
    if (!pstBuf)
        return;

    if (pstBuf->eMapType == eVppMapType_PxExTogether)
    {
        vVppBuf_UnmapMemBuf(&pstBuf->stPixel);
    }
    else if (pstBuf->eMapType == eVppMapType_PxExSeparate)
    {
        if (pstBuf->stPixel.eMapping == eVppBuf_MappedInternal)
            vVppBuf_UnmapMemBuf(&pstBuf->stPixel);

        if (pstBuf->stExtra.eMapping == eVppBuf_MappedInternal)
            vVppBuf_UnmapMemBuf(&pstBuf->stExtra);
    }
}

#define MSG_MAX 512
static void vVppBuf_PrintLog(t_StVppBufCb *pstBufCb, t_StVppBuf *pstBuf)
{
    char cBuf[MSG_MAX] = {0};
    int offset = 0, rc;
    uint32_t i;
    struct timeval *pstTv;

    if (!pstBuf || !pstBufCb || !pstBufCb->u32LogMask)
        return;

    offset += snprintf(cBuf, MSG_MAX, "0x%x", pstBufCb->u32LogMask);
    offset += snprintf(cBuf + offset, MSG_MAX - offset, ", %u", pstBuf->u32BufNum);
    offset += snprintf(cBuf + offset, MSG_MAX - offset, ", %s, %s",
                       pstBuf->eQueuedPort == VPP_PORT_INPUT ? "I" : "O",
                       pstBuf->eReturnPort == VPP_PORT_INPUT ? "I" : "O");

    for (i = 0; i < eVppLogId_Max; i++)
    {
        pstTv = &pstBuf->astLogTime[i];

        if (pstBufCb->u32LogMask & (1 << i))
        {
            offset += snprintf(cBuf + offset, MSG_MAX - offset, ", %ld.%06ld",
                               pstTv->tv_sec, pstTv->tv_usec);
        }
        else
        {
            offset += snprintf(cBuf + offset, MSG_MAX - offset, ", -");
        }
    }

    LOGE("%s", cBuf);
}

static t_EVppBufType eVppBuf_GetInternalFrameTypeFromMsm(enum msm_vidc_extradata_type eType)
{
    t_EVppBufType eRet = eVppBufType_Progressive;
    switch(eType)
    {
        VPP_BUF_MAP(eRet, MSM_VIDC_INTERLACE_INTERLEAVE_FRAME_TOPFIELDFIRST, eVppBufType_Interleaved_TFF);
        VPP_BUF_MAP(eRet, MSM_VIDC_INTERLACE_INTERLEAVE_FRAME_BOTTOMFIELDFIRST, eVppBufType_Interleaved_BFF);
        VPP_BUF_MAP(eRet, MSM_VIDC_INTERLACE_FRAME_TOPFIELDFIRST, eVppBufType_Frame_TFF);
        VPP_BUF_MAP(eRet, MSM_VIDC_INTERLACE_FRAME_BOTTOMFIELDFIRST, eVppBufType_Frame_BFF);
        VPP_BUF_MAP(eRet, MSM_VIDC_INTERLACE_FRAME_PROGRESSIVE, eVppBufType_Progressive);
        default:
            eRet = eVppBufType_Progressive;
    }

    return eRet;
}

static enum msm_vidc_interlace_type eVppBuf_GetMsmFrameTypeFromInternal(t_EVppBufType eType)
{
    enum msm_vidc_interlace_type eRet = MSM_VIDC_INTERLACE_FRAME_PROGRESSIVE;

    switch (eType)
    {
        VPP_BUF_MAP(eRet, eVppBufType_Interleaved_TFF, MSM_VIDC_INTERLACE_INTERLEAVE_FRAME_TOPFIELDFIRST);
        VPP_BUF_MAP(eRet, eVppBufType_Interleaved_BFF, MSM_VIDC_INTERLACE_INTERLEAVE_FRAME_BOTTOMFIELDFIRST);
        VPP_BUF_MAP(eRet, eVppBufType_Frame_TFF, MSM_VIDC_INTERLACE_FRAME_TOPFIELDFIRST);
        VPP_BUF_MAP(eRet, eVppBufType_Frame_BFF, MSM_VIDC_INTERLACE_FRAME_BOTTOMFIELDFIRST);
        VPP_BUF_MAP(eRet, eVppBufType_Progressive, MSM_VIDC_INTERLACE_FRAME_PROGRESSIVE);
        default:
            eRet = MSM_VIDC_INTERLACE_FRAME_PROGRESSIVE;
    }

    return eRet;
}
static void *vpVppBuf_FindExtradataHeaderMsm(t_StVppBuf *pstBufSrc,
                                             uint32_t u32ExType)
{
    uint32_t u32AvailSrc, u32CurLen = 0;
    struct msm_vidc_extradata_header *pExDataSrc;

    if (!pstBufSrc)
        return NULL;

    if (!pstBufSrc->stExtra.u32MappedLen || !pstBufSrc->stExtra.pvBase)
    {
        LOGI("%s(), unable to find extra data -- src.u32MappedLen=%u, pvBase=%p",
             __func__, pstBufSrc->stExtra.u32MappedLen, pstBufSrc->stExtra.pvBase);
        return NULL;
    }

    pExDataSrc = (struct msm_vidc_extradata_header *)pstBufSrc->stExtra.pvBase;

    LOGI("%s, src: pvBase=%p, alloc_len=%u, filled_len=%u", __func__,
         pstBufSrc->stExtra.pvBase, pstBufSrc->stExtra.u32AllocLen,
         pstBufSrc->stExtra.u32FilledLen);

    while (u32CurLen < pstBufSrc->stExtra.u32MappedLen)
    {
        if (pExDataSrc->type == MSM_VIDC_EXTRADATA_NONE)
        {
            LOGI("found packet type == NONE");
            break;
        }

        if ((u32CurLen + pExDataSrc->size) >= pstBufSrc->stExtra.u32MappedLen)
        {
            LOGE("packet size > available size in dst, u32CurLen=%u, sz=%u, "
                 "u32MappedLen=%u", u32CurLen, pExDataSrc->size,
                 pstBufSrc->stExtra.u32MappedLen);
            break;
        }

        LOGI("type=%u, pExDataSrc->size=%u", pExDataSrc->type, pExDataSrc->size);

        if (!pExDataSrc->size)
        {
            LOGE("ERROR: extradata size is zero, type=%u", pExDataSrc->type);
            break;
        }

        if (pExDataSrc->type == u32ExType)
            return (void *)pExDataSrc;

        u32CurLen += pExDataSrc->size;

        pExDataSrc = (struct msm_vidc_extradata_header *)
            ((char *)pExDataSrc + pExDataSrc->size);
    }

    return NULL;
}

static void * vpVppBuf_FindExtradataMsm(t_StVppBuf *pstBufSrc, uint32_t u32ExType)
{
    void *pv;
    struct msm_vidc_extradata_header *pExDataSrc;

    pv = vpVppBuf_FindExtradataHeaderMsm(pstBufSrc, u32ExType);
    if (pv)
    {
        pExDataSrc = (struct msm_vidc_extradata_header *)pv;
        return (void *)pExDataSrc->data;
    }

    return NULL;
}

static uint32_t u32VppBuf_GetFrameTypeMsm(t_StVppBuf *pstBuf,
                                          t_EVppBufType *peBufType)
{
    struct msm_vidc_interlace_payload *pl;

    LOG_ENTER();

    if (!pstBuf || !peBufType)
    {
        LOGE("pstBuf was null");
        return VPP_ERR_PARAM;
    }

    pl = (struct msm_vidc_interlace_payload *)
        vpVppBuf_FindExtradataMsm(pstBuf, MSM_VIDC_EXTRADATA_INTERLACE_VIDEO);

    if (pl)
    {
        LOGI("found MSM_VIDC_EXTRADATA_INTERLACE_VIDEO, format=%u", pl->format);
        *peBufType = eVppBuf_GetInternalFrameTypeFromMsm(pl->format);
        return VPP_OK;
    }

    return VPP_ERR;
}


static uint32_t u32VppBuf_CopyExtradataMsm(t_StVppBuf *pstBufSrc, t_StVppBuf *pstBufDst,
                                           t_EVppBufType eDstBufType)
{
    uint32_t u32AvailDst, u32AvailSrc, u32CurLen = 0;
    t_StVppMemBuf *pstMemBufSrc, *pstMemBufDst;
    struct msm_vidc_extradata_header *pExDataSrc, *pExDataDst;
    struct msm_vidc_interlace_payload *pl;

    if (!pstBufSrc || !pstBufDst)
        return VPP_ERR_PARAM;

    if (!pstBufSrc->stExtra.u32MappedLen || !pstBufDst->stExtra.u32MappedLen)
    {
        LOGI("%s(), src.u32MappedLen=%u, dst.u32MappedLen=%u, not copying",
             __func__, pstBufSrc->stExtra.u32MappedLen,
             pstBufDst->stExtra.u32MappedLen);
        return VPP_ERR_PARAM;
    }

    pstMemBufDst = &pstBufDst->stExtra;
    pstMemBufSrc = &pstBufSrc->stExtra;

    u32AvailDst = pstBufDst->stExtra.u32MappedLen;
    u32AvailSrc = pstBufSrc->stExtra.u32MappedLen;

    pExDataSrc = (struct msm_vidc_extradata_header *)pstBufSrc->stExtra.pvBase;
    pExDataDst = (struct msm_vidc_extradata_header *)pstBufDst->stExtra.pvBase;

    LOGI("%s, src: pvBase=%p, offset=%d, alloc_len=%u, filled_len=%u",
         __func__, pstBufSrc->stExtra.pvBase, pstMemBufSrc->u32Offset,
         pstMemBufSrc->u32AllocLen, pstMemBufSrc->u32FilledLen);
    LOGI("%s, dst: pvBase=%p, offset=%d, alloc_len=%u, filled_len=%u",
         __func__, pstBufDst->stExtra.pvBase, pstMemBufDst->u32Offset,
         pstMemBufDst->u32AllocLen, pstMemBufDst->u32FilledLen);

    while (u32CurLen < pstMemBufSrc->u32MappedLen)
    {
        if(pExDataSrc->type == MSM_VIDC_EXTRADATA_NONE)
        {
            LOGI("copy-msm: found packet type == NONE");
            break;
        }

        if (pExDataSrc->size > (pstMemBufDst->u32MappedLen - u32CurLen))
        {
            LOGE("copy-msm: packet size > available size in dst");
            LOGE("sz: %u, u32MappedLen=%u, u32CurLen=%u, delta=%u",
                 pExDataSrc->size, pstMemBufDst->u32MappedLen, u32CurLen,
                 pstMemBufDst->u32MappedLen - u32CurLen);
            break;
        }

        LOGI("copy-msm: type=%u, pExDataSrc->size=%u",
             pExDataSrc->type, pExDataSrc->size);

        if (!pExDataSrc->size)
        {
            LOGE("ERROR: extradata size is zero, type=%u", pExDataSrc->type);
            break;
        }

        memcpy(pExDataDst, pExDataSrc, pExDataSrc->size);

        if (pExDataDst->type == MSM_VIDC_EXTRADATA_INTERLACE_VIDEO)
        {
            pl = (struct msm_vidc_interlace_payload *)pExDataDst->data;
            pl->format = eVppBuf_GetMsmFrameTypeFromInternal(eDstBufType);
        }

        u32CurLen += pExDataDst->size;

        pExDataSrc = (struct msm_vidc_extradata_header *)
            ((char *)pExDataSrc + pExDataSrc->size);
        pExDataDst = (struct msm_vidc_extradata_header *)
            ((char *)pExDataDst + pExDataDst->size);
    }

    pstMemBufDst->u32FilledLen = u32CurLen;
    return VPP_OK;
}

#ifndef OMX_EXTRADATA_DOES_NOT_EXIST

static t_EVppBufType eVppBuf_GetInternalFrameTypeFromOmx(enum OMX_INTERLACETYPE eType)
{
    t_EVppBufType eRet = eVppBufType_Progressive;
    switch(eType)
    {
        VPP_BUF_MAP(eRet, OMX_InterlaceInterleaveFrameTopFieldFirst, eVppBufType_Interleaved_TFF);
        VPP_BUF_MAP(eRet, OMX_InterlaceInterleaveFrameBottomFieldFirst, eVppBufType_Interleaved_BFF);
        VPP_BUF_MAP(eRet, OMX_InterlaceFrameTopFieldFirst, eVppBufType_Frame_TFF);
        VPP_BUF_MAP(eRet, OMX_InterlaceFrameBottomFieldFirst, eVppBufType_Frame_BFF);
        VPP_BUF_MAP(eRet, OMX_InterlaceFrameProgressive, eVppBufType_Progressive);
        default:
            eRet = eVppBufType_Progressive;
    }

    return eRet;
}

static enum OMX_INTERLACETYPE eVppBuf_GetOmxFrameTypeFromInternal(t_EVppBufType eType)
{
    enum OMX_INTERLACETYPE eRet = OMX_InterlaceFrameProgressive;
    switch (eType)
    {
        VPP_BUF_MAP(eRet, eVppBufType_Interleaved_TFF, OMX_InterlaceInterleaveFrameTopFieldFirst);
        VPP_BUF_MAP(eRet, eVppBufType_Interleaved_BFF, OMX_InterlaceInterleaveFrameBottomFieldFirst);
        VPP_BUF_MAP(eRet, eVppBufType_Frame_TFF, OMX_InterlaceFrameTopFieldFirst);
        VPP_BUF_MAP(eRet, eVppBufType_Frame_BFF, OMX_InterlaceFrameBottomFieldFirst);
        VPP_BUF_MAP(eRet, eVppBufType_Progressive, OMX_InterlaceFrameProgressive);
        default:
            eRet= OMX_InterlaceFrameProgressive;
    }

    return eRet;
}

static void * vpVppBuf_FindExtradataHeaderOmx(t_StVppBuf *pstBufSrc,
                                              uint32_t u32ExType)
{
    uint32_t u32AvailSrc, u32CurLen = 0;
    OMX_OTHER_EXTRADATATYPE *pExDataSrc;

    if (!pstBufSrc)
        return NULL;

    if (!pstBufSrc->stExtra.u32MappedLen || !pstBufSrc->stExtra.pvBase)
    {
        LOGI("%s(), unable to find extra data -- src.u32MappedLen=%u, pvBase=%p",
             __func__, pstBufSrc->stExtra.u32MappedLen, pstBufSrc->stExtra.pvBase);
        return NULL;
    }

    pExDataSrc = (OMX_OTHER_EXTRADATATYPE *)pstBufSrc->stExtra.pvBase;

    LOGI("%s, src: pvBase=%p, alloc_len=%u, filled_len=%u", __func__,
         pstBufSrc->stExtra.pvBase, pstBufSrc->stExtra.u32AllocLen,
         pstBufSrc->stExtra.u32FilledLen);

    while (u32CurLen < pstBufSrc->stExtra.u32MappedLen)
    {
        if(pExDataSrc->eType == OMX_ExtraDataNone)
        {
            LOGI("find-omx: found packet type == NONE");
            break;
        }

        if ((u32CurLen + pExDataSrc->nSize) >= pstBufSrc->stExtra.u32MappedLen)
        {
            LOGE("find-omx: packet size > available size in dst, u32CurLen=%u, sz=%u, "
                 "u32MappedLen=%u", u32CurLen, pExDataSrc->nSize,
                 pstBufSrc->stExtra.u32MappedLen);
            break;
        }

        LOGI("type=0x%x, pExDataSrc->size=%u", pExDataSrc->eType, pExDataSrc->nSize);

        if (!pExDataSrc->nSize)
        {
            LOGE("ERROR: extradata size is zero, type=%u", pExDataSrc->eType);
            break;
        }

        if (pExDataSrc->eType == u32ExType)
            return (void *)pExDataSrc;

        u32CurLen += pExDataSrc->nSize;

        pExDataSrc = (OMX_OTHER_EXTRADATATYPE *)
            ((char *)pExDataSrc + pExDataSrc->nSize);
    }

    return NULL;
}

static void * vpVppBuf_FindExtradataOmx(t_StVppBuf *pstBufSrc, uint32_t u32ExType)
{
    void *pv;
    OMX_OTHER_EXTRADATATYPE *pExDataSrc;

    pv = vpVppBuf_FindExtradataHeaderOmx(pstBufSrc, u32ExType);
    if (pv)
    {
        pExDataSrc = (OMX_OTHER_EXTRADATATYPE *)pv;
        return (void *)pExDataSrc->data;
    }

    return NULL;
}

static uint32_t u32VppBuf_GetFrameTypeOmx(t_StVppBuf *pstBuf,
                                          t_EVppBufType *peBufType)
{
    OMX_STREAMINTERLACEFORMAT *pl;

    LOG_ENTER();

    if (!pstBuf || !peBufType)
    {
        LOGE("pstBuf was null");
        return VPP_ERR_PARAM;
    }

    pl = (OMX_STREAMINTERLACEFORMAT *)
        vpVppBuf_FindExtradataOmx(pstBuf, OMX_ExtraDataInterlaceFormat);

    if (pl)
    {
        LOGI("found OMX_ExtraDataInterlaceFormat, format=%u", pl->nInterlaceFormats);
        *peBufType = eVppBuf_GetInternalFrameTypeFromOmx(pl->nInterlaceFormats);
        return VPP_OK;
    }

    return VPP_ERR;
}


static uint32_t u32VppBuf_CopyExtradataOmx(t_StVppBuf *pstBufSrc, t_StVppBuf *pstBufDst,
                                           t_EVppBufType eDstBufType)
{
    uint32_t u32AvailDst, u32AvailSrc, u32CurLen = 0;
    t_StVppMemBuf *pstMemBufSrc, *pstMemBufDst;
    OMX_OTHER_EXTRADATATYPE *pExDataSrc, *pExDataDst;
    OMX_STREAMINTERLACEFORMAT *pl;

    if (!pstBufSrc || !pstBufDst)
        return VPP_ERR_PARAM;

    if (!pstBufSrc->stExtra.u32MappedLen || !pstBufDst->stExtra.u32MappedLen)
    {
        LOGI("%s(), src.u32MappedLen=%u, dst.u32MappedLen=%u, not copying",
             __func__, pstBufSrc->stExtra.u32MappedLen,
             pstBufDst->stExtra.u32MappedLen);
        return VPP_ERR_PARAM;
    }

    pstMemBufDst = &pstBufDst->stExtra;
    pstMemBufSrc = &pstBufSrc->stExtra;

    u32AvailDst = pstBufDst->stExtra.u32MappedLen;
    u32AvailSrc = pstBufSrc->stExtra.u32MappedLen;

    pExDataSrc = (OMX_OTHER_EXTRADATATYPE *)pstMemBufSrc->pvBase;
    pExDataDst = (OMX_OTHER_EXTRADATATYPE *)pstMemBufDst->pvBase;

    LOGI("%s, src: pvBase=%p, offset=%d, alloc_len=%u, filled_len=%u",
         __func__, pstMemBufSrc->pvBase, pstMemBufSrc->u32Offset,
         pstMemBufSrc->u32AllocLen, pstMemBufSrc->u32FilledLen);
    LOGI("%s, dst: pvBase=%p, offset=%d, alloc_len=%u, filled_len=%u",
         __func__, pstMemBufDst->pvBase, pstMemBufDst->u32Offset,
         pstMemBufDst->u32AllocLen, pstMemBufDst->u32FilledLen);

    while (u32CurLen < pstMemBufSrc->u32MappedLen)
    {
        if(pExDataSrc->eType == OMX_ExtraDataNone)
        {
            LOGI("copy-omx: found packet type == NONE");
            break;
        }

        if (pExDataSrc->nSize > (pstMemBufDst->u32MappedLen - u32CurLen))
        {
            LOGE("copy-omx: packet size > available size in dst");
            LOGE("sz: %u, u32MappedLen=%u, u32CurLen=%u, delta=%u",
                 pExDataSrc->nSize, pstMemBufDst->u32MappedLen, u32CurLen,
                 pstMemBufDst->u32MappedLen - u32CurLen);
            break;
        }

        LOGI("copy-omx: type=0x%x, pExDataSrc->size=%u",
             pExDataSrc->eType, pExDataSrc->nSize);

        if (!pExDataSrc->nSize)
        {
            LOGE("ERROR: extradata size is zero, type=%u", pExDataSrc->eType);
            break;
        }

        memcpy(pExDataDst, pExDataSrc, pExDataSrc->nSize);

        if (pExDataDst->eType == (OMX_EXTRADATATYPE)OMX_ExtraDataInterlaceFormat)
        {
            pl = (OMX_STREAMINTERLACEFORMAT *)pExDataDst->data;
            pl->nInterlaceFormats = eVppBuf_GetOmxFrameTypeFromInternal(eDstBufType);
        }

        u32CurLen += pExDataDst->nSize;

        pExDataSrc = (OMX_OTHER_EXTRADATATYPE *)
            ((char *)pExDataSrc + pExDataSrc->nSize);
        pExDataDst = (OMX_OTHER_EXTRADATATYPE *)
            ((char *)pExDataDst + pExDataDst->nSize);
    }

    pstMemBufDst->u32FilledLen = u32CurLen;
    return VPP_OK;
}

#endif
/************************************************************************
 * Global Functions
 ***********************************************************************/
uint32_t u32VppBufPool_Init(t_StVppBufPool *pstPool)
{
    if (!pstPool)
        return VPP_ERR_PARAM;

    memset(pstPool, 0, sizeof(t_StVppBufPool));

    return VPP_OK;
}

uint32_t u32VppBufPool_Term(t_StVppBufPool *pstPool)
{
    if (!pstPool)
        return VPP_OK;

    if (pstPool->u32Cnt)
        LOGE("destroying queue with non zero count, cnt=%u", pstPool->u32Cnt);

    memset(pstPool, 0, sizeof(t_StVppBufPool));

    return VPP_OK;
}

t_StVppBuf *pstVppBufPool_Get(t_StVppBufPool *pstPool)
{
    t_StVppBuf *pstTmp = NULL;

    if (!pstPool || !pstPool->pstHead)
        return NULL;

    pstTmp = pstPool->pstHead;
    pstPool->pstHead = pstPool->pstHead->pNext;
    pstPool->u32Cnt -= 1;

    if (!pstPool->u32Cnt)
        pstPool->pstTail = NULL;

#ifdef VPP_BUF_DBG_POOL
    LOGI("%s: pool: %p, nodes remaining: %u, returning: %p, head: %p, tail: %p",
         __func__, pstPool, pstPool->u32Cnt, pstTmp, pstPool->pstHead,
         pstPool->pstTail);
#endif

    return pstTmp;
}

t_StVppBuf *pstVppBufPool_Peek(t_StVppBufPool *pstPool, uint32_t index)
{
    // return ptr to the element but will not remove it from the list if
    // index==0, return pstPool->pstHead;if index==1, return
    // pstPool->pstHead->next; and so on

    t_StVppBuf *pstTmp = NULL;
    uint32_t cnt = 0 ;

    if (!pstPool || !pstPool->pstHead || index >= pstPool->u32Cnt)
        return NULL;

    pstTmp = pstPool->pstHead;
    while(pstTmp && cnt < index)
    {
        pstTmp = pstTmp->pNext;
        cnt += 1;
    }

#ifdef VPP_BUF_DBG_POOL
    LOGI("%s: pool: %p, nodes remaining: %u, returning: %p, head: %p, tail: %p, cnt: %d, index:%d",
         __func__, pstPool, pstPool->u32Cnt, pstTmp, pstPool->pstHead,
         pstPool->pstTail, cnt, index);
#endif

    return pstTmp;
}

uint32_t u32VppBufPool_Put(t_StVppBufPool *pstPool, t_StVppBuf *pstBuf)
{
    if (!pstPool || !pstBuf)
        return VPP_ERR_PARAM;

    pstBuf->pNext = NULL;

    if (!pstPool->u32Cnt)
    {
        pstPool->pstHead = pstBuf;
        pstPool->pstTail = pstBuf;
    }
    else
    {
        pstPool->pstTail->pNext = pstBuf;
        pstPool->pstTail = pstBuf;
    }

    pstPool->u32Cnt += 1;

#ifdef VPP_BUF_DBG_POOL
    LOGI("%s: pool: %p, nodes count: %u, buf: %p, head: %p, tail: %p",
         __func__, pstPool, pstPool->u32Cnt, pstBuf, pstPool->pstHead,
         pstPool->pstTail);
#endif

    return VPP_OK;
}

uint32_t u32VppBufPool_Cnt(t_StVppBufPool *pstPool)
{
    if (!pstPool)
        return 0;

    return pstPool->u32Cnt;
}

uint32_t u32VppBuf_Init(t_StVppCtx *pstCtx, uint32_t bIsSecure, uint32_t u32LogMask)
{
    int rc = 0;
    uint32_t i;
    uint32_t u32Ret = VPP_OK;
    t_StVppBufCb *pstBufCb;

    if (!pstCtx)
        return VPP_ERR_PARAM;

    pstBufCb = calloc(1, sizeof(t_StVppBufCb));
    if (!pstBufCb)
    {
        LOGE("unable to malloc buf control block");
        return VPP_ERR_NO_MEM;
    }

    rc = pthread_mutex_init(&pstBufCb->mutex, NULL);
    if (rc)
    {
        LOGE("unable to initialize mutex");
        goto ERR_MUTEX_INIT;
    }

    u32Ret = u32VppBufPool_Init(&pstBufCb->stPool);
    if (u32Ret != VPP_OK)
    {
        LOGE("unable to intiailize buffer pool structure, u32Ret=%u", u32Ret);
        goto ERR_POOL_INIT;
    }

    for (i = 0; i < VPP_INTERNAL_BUF_MAX; i ++)
    {
        u32Ret = u32VppBufPool_Put(&pstBufCb->stPool, &pstBufCb->astBufNodes[i]);
        if (u32Ret != VPP_OK)
        {
            LOGE("unable to queue node at index: %u, u32Ret=%u", i, u32Ret);
            goto ERR_POOL_QUEUE;
        }
    }

    pstBufCb->u32LogMask = u32LogMask;
    pstBufCb->bIsSecure = bIsSecure;
    pstCtx->pstBufCb = pstBufCb;

    return VPP_OK;

ERR_POOL_QUEUE:
    while (pstVppBufPool_Get(&pstBufCb->stPool));
    u32VppBufPool_Term(&pstBufCb->stPool);

ERR_POOL_INIT:
    pthread_mutex_destroy(&pstBufCb->mutex);

ERR_MUTEX_INIT:
    free(pstBufCb);
    return VPP_ERR;

}

uint32_t u32VppBuf_Term(t_StVppCtx *pstCtx)
{
    t_StVppBufCb *pstBufCb;
    uint32_t u32Cnt;
    uint32_t u32Ret = VPP_OK;

    if (!pstCtx)
        return VPP_ERR_PARAM;

    pstBufCb = pstCtx->pstBufCb;

    u32Cnt = u32VppBufPool_Cnt(&pstBufCb->stPool);
    if (u32Cnt != VPP_INTERNAL_BUF_MAX)
    {
        LOGE("term called when buffer pool does not hold all buffers: expected=%u, actual=%u",
             VPP_INTERNAL_BUF_MAX, u32Cnt);
        u32Ret = VPP_ERR;
    }

    while (pstVppBufPool_Get(&pstBufCb->stPool));

    u32VppBufPool_Term(&pstBufCb->stPool);

#ifdef VPP_CACHE_MMAPPED_PX_BUF
    pthread_mutex_lock(&pstBufCb->mutex);
    vVppBuf_FreeBufferCache(pstBufCb);
    pthread_mutex_unlock(&pstBufCb->mutex);
#endif

    pthread_mutex_destroy(&pstBufCb->mutex);

    free(pstBufCb);

    return u32Ret;
}

#ifdef VPP_CACHE_MMAPPED_PX_BUF
uint32_t u32VppBuf_FlushBufferCache(t_StVppCtx *pstCtx)
{
    t_StVppBufCb *pstBufCb;

    if (!pstCtx)
        return VPP_ERR_PARAM;

    pstBufCb = pstCtx->pstBufCb;

    pthread_mutex_lock(&pstBufCb->mutex);
    vVppBuf_FreeBufferCache(pstBufCb);
    pthread_mutex_unlock(&pstBufCb->mutex);

    return VPP_OK;
}
#endif

uint32_t u32VppBuf_SetCurrentUsecase(t_StVppCtx *pstCtx, struct StVppUsecase *pstUc)
{
    uint32_t u32;
    t_StVppBufCb *pstBufCb;

    if (!pstCtx || !pstCtx->pstBufCb)
        return VPP_ERR_PARAM;

    pstBufCb = pstCtx->pstBufCb;

    u32 = u32VppUsecase_IsMappedPxBufRequired(pstUc);
    pstBufCb->bRequiresMappedPxBuf = u32;

    LOGI("current usecase %s mapped pixel buffers",
         u32 ? "requires" : "doesn't require");

    return VPP_OK;
}

#define VPP_BUF_IS_CTX_VALID(_p) ((_p) && (_p)->pstBufCb)
uint32_t u32VppBuf_SetPortParams(t_StVppCtx *pstCtx,
                                 struct vpp_port_param stInput,
                                 struct vpp_port_param stOutput)
{
    t_StVppBufCb *pstBufCb;

    if (!VPP_BUF_IS_CTX_VALID(pstCtx))
        return VPP_ERR_PARAM;

    pstBufCb = pstCtx->pstBufCb;

    pstBufCb->u32InExOff = u32VppUtils_GetPxBufferSize(&stInput);
    pstBufCb->u32OutExOff = u32VppUtils_GetPxBufferSize(&stOutput);

    LOGI("ex_off: in=%u, out=%u", pstBufCb->u32InExOff, pstBufCb->u32OutExOff);

    return VPP_OK;
}

uint32_t u32VppBuf_InternalGet(t_StVppCtx *pstCtx,
                               struct vpp_buffer *pstExtBuf,
                               enum vpp_port ePort,
                               t_StVppBuf **o_ppstIntBuf)
{
    int rc;
    uint32_t u32 = VPP_OK;
    uint32_t u32MapFlags = 0;
    t_StVppBuf *pstIntBuf;
    t_StVppBufCb *pstBufCb;
    uint32_t u32BufDumpEn = VPP_FALSE;

    if (!pstCtx || !pstExtBuf || !o_ppstIntBuf || ePort >= VPP_PORT_MAX)
        return VPP_ERR_PARAM;

    pstBufCb = pstCtx->pstBufCb;

    *o_ppstIntBuf = NULL;

    rc = pthread_mutex_lock(&pstBufCb->mutex);
    if (rc)
    {
        LOGE("unable to lock mutex, rc=%d, err=%s", rc, strerror(rc));
        return VPP_ERR;
    }

    pstIntBuf = pstVppBufPool_Get(&pstBufCb->stPool);

    rc = pthread_mutex_unlock(&pstBufCb->mutex);
    if (rc)
    {
        u32VppBufPool_Put(&pstBufCb->stPool, pstIntBuf);
        LOGE("unable to unlock mutex, rc=%d, err=%s", rc, strerror(rc));
        return VPP_ERR;
    }

    if (!pstIntBuf)
    {
        LOGE("no more buffers in buffer pool!");
        return VPP_ERR;
    }

    memset(pstIntBuf, 0, sizeof(t_StVppBuf));

    pstIntBuf->pBuf = pstExtBuf;
    pstIntBuf->eQueuedPort = ePort;

    VPP_LOG_BUF(pstCtx, pstIntBuf, eVppLogId_InternalGet);
    VPP_STATS(pstBufCb, InternalGet);

    u32MapFlags = VPP_BUF_MAP_BUF_EX;
    if (pstBufCb->bRequiresMappedPxBuf)
        u32MapFlags |= VPP_BUF_MAP_BUF_PX;

    u32 = u32VppUtils_GetBufDumpEnable(pstCtx, &u32BufDumpEn);
    if (u32 == VPP_OK && !pstBufCb->bIsSecure && u32BufDumpEn)
        u32MapFlags |= VPP_BUF_MAP_BUF_PX;

    vVppBuf_SetMemBuf(&pstIntBuf->stPixel, NULL, 0, NULL, 0, 0, eVppBuf_Unmapped);
    vVppBuf_SetMemBuf(&pstIntBuf->stExtra, NULL, 0, NULL, 0, 0, eVppBuf_Unmapped);

    vVppBuf_TranslateBufExtToInt(pstBufCb, pstIntBuf);
    vVppBuf_MapIntBuf(pstBufCb, pstIntBuf, u32MapFlags);

    *o_ppstIntBuf = pstIntBuf;

    return VPP_OK;
}

uint32_t u32VppBuf_InternalPut(t_StVppCtx *pstCtx, t_StVppBuf *pstIntBuf, struct vpp_buffer **o_ppstExtBuf)
{
    int rc;
    uint32_t u32Ret;
    t_StVppBufCb *pstBufCb;

    if (!pstCtx || !pstIntBuf || !o_ppstExtBuf)
        return VPP_ERR_PARAM;

    pstBufCb = pstCtx->pstBufCb;

    VPP_STATS(pstBufCb, InternalPut);

    vVppBuf_TranslateBufIntToExt(pstBufCb, pstIntBuf);
    vVppBuf_UnmapIntBuf(pstIntBuf);

    *o_ppstExtBuf = pstIntBuf->pBuf;

    rc = pthread_mutex_lock(&pstBufCb->mutex);
    LOGE_IF(rc, "unable to lock mutex, rc=%d, err=%s", rc, strerror(rc));

    VPP_LOG_BUF(pstCtx, pstIntBuf, eVppLogId_InternalPut);

    vVppBuf_PrintLog(pstBufCb, pstIntBuf);

    u32Ret = u32VppBufPool_Put(&pstBufCb->stPool, pstIntBuf);
    rc = pthread_mutex_unlock(&pstBufCb->mutex);
    LOGE_IF(rc, "unable to lock mutex, rc=%d, err=%s", rc, strerror(rc));

    if (u32Ret != VPP_OK)
    {
        LOGE("unable to return buffer to pool: pstIntBuf=%p", pstIntBuf);
    }

    return VPP_OK;
}

uint32_t u32VppBuf_IsEos(t_StVppBuf *pstVppBuf)
{
    if (!pstVppBuf || !pstVppBuf->pBuf)
        return VPP_FALSE;

    return VPP_FLAG_IS_SET(pstVppBuf->pBuf->flags, VPP_BUFFER_FLAG_EOS);
}

uint32_t u32VppBuf_IsEmptyEos(t_StVppBuf *pstVppBuf)
{
    if (!pstVppBuf || !pstVppBuf->pBuf)
        return VPP_FALSE;

    return (u32VppBuf_IsEos(pstVppBuf) && pstVppBuf->stPixel.u32FilledLen == 0);
}

void vVppBuf_Log(t_StVppCtx *pstCtx, t_StVppBuf *pstBuf, uint32_t u32Id)
{
    t_StVppBufCb *pstBufCb;

    if (!pstCtx || !pstBuf || u32Id >= eVppLogId_Max)
    {
        LOGE("buf log error, pstCtx=%p, pstBuf=%p, u32Id=%u",
             pstCtx, pstBuf, u32Id);
        return;
    }

    pstBufCb = pstCtx->pstBufCb;
    if (pstBufCb && pstBufCb->u32LogMask & (1 << u32Id))
        gettimeofday(&pstBuf->astLogTime[u32Id], NULL);
}

void vVppBuf_LogInfo(t_StVppCtx *pstCtx, t_StVppBuf *pstBuf)
{
    if (!pstCtx || !pstBuf)
    {
        LOGE("buf log info error, pstCtx=%p, pstBuf=%p", pstCtx, pstBuf);
        return;
    }

    LOGI("BufNum=%d, Flags=0x%x, BufType=%d, BufPxType=%d\n",
         pstBuf->u32BufNum, pstBuf->u32InternalFlags, pstBuf->eBufType,
         pstBuf->eBufPxType);
    LOGI("Pixel: Base=%p, MappedLen=%d, flags=0x%x, timestamp=0x%llx\n",
         pstBuf->stPixel.pvBase, pstBuf->stPixel.u32MappedLen,
         pstBuf->pBuf->flags, (long long unsigned int)pstBuf->pBuf->timestamp);

    if (pstBuf->pBuf != NULL)
    {
        LOGI("Pixel: fd=%d, offset=0x%x, alloc_len=%d, filled_len=%d\n",
             pstBuf->pBuf->pixel.fd, pstBuf->pBuf->pixel.offset,
             pstBuf->pBuf->pixel.alloc_len, pstBuf->pBuf->pixel.filled_len);
    }
    LOGI("Extra: Base=%p, MappedLen=%d\n",
         pstBuf->stExtra.pvBase, pstBuf->stExtra.u32MappedLen);
    if (pstBuf->pBuf != NULL)
    {
        LOGI("Extra: fd=%d, offset=0x%x, alloc_len=%d, filled_len=%d\n",
             pstBuf->pBuf->extradata.fd,pstBuf->pBuf->extradata.offset,
             pstBuf->pBuf->extradata.alloc_len,
             pstBuf->pBuf->extradata.filled_len);
    }
}

void * vpVppBuf_FindExtradata(t_StVppBuf *pstBufSrc, uint32_t u32ExType,
                              t_EExtradataBufferFormat eExBufFormat)
{
    if(eExBufFormat == EXTRADATA_BUFFER_FORMAT_MSM)
        return vpVppBuf_FindExtradataMsm(pstBufSrc, u32ExType);
#ifndef OMX_EXTRADATA_DOES_NOT_EXIST
    else if(eExBufFormat == EXTRADATA_BUFFER_FORMAT_OMX)
        return vpVppBuf_FindExtradataOmx(pstBufSrc, u32ExType);
#endif
    else
        return NULL;
}

void * vpVppBuf_FindExtradataHeader(t_StVppBuf *pstBufSrc, uint32_t u32ExType,
                                    t_EExtradataBufferFormat eExBufFormat)
{
    if(eExBufFormat == EXTRADATA_BUFFER_FORMAT_MSM)
        return vpVppBuf_FindExtradataHeaderMsm(pstBufSrc, u32ExType);
#ifndef OMX_EXTRADATA_DOES_NOT_EXIST
    else if(eExBufFormat == EXTRADATA_BUFFER_FORMAT_OMX)
        return vpVppBuf_FindExtradataHeaderOmx(pstBufSrc, u32ExType);
#endif
    else
        return NULL;
}

uint32_t u32VppBuf_GetFrameTypeExtradata(t_StVppBuf *pstBuf,
                                         t_EExtradataBufferFormat eExBufFormat,
                                         t_EVppBufType *peBufType)
{
    if (!pstBuf || !peBufType)
        return VPP_ERR_PARAM;

    if(eExBufFormat == EXTRADATA_BUFFER_FORMAT_MSM)
        return u32VppBuf_GetFrameTypeMsm(pstBuf, peBufType);
#ifndef OMX_EXTRADATA_DOES_NOT_EXIST
    else if(eExBufFormat == EXTRADATA_BUFFER_FORMAT_OMX)
        return u32VppBuf_GetFrameTypeOmx(pstBuf, peBufType);
#endif
    else
        return eVppBufType_Progressive;
}


uint32_t u32VppBuf_CopyExtradata(t_StVppBuf *pstBufSrc, t_StVppBuf *pstBufDst,
                                 t_EVppBufType eDstBufType, t_EExtradataBufferFormat eExBufFormat)
{
    if(eExBufFormat == EXTRADATA_BUFFER_FORMAT_MSM)
        return u32VppBuf_CopyExtradataMsm(pstBufSrc, pstBufDst, eDstBufType);
#ifndef OMX_EXTRADATA_DOES_NOT_EXIST
    else if(eExBufFormat == EXTRADATA_BUFFER_FORMAT_OMX)
        return u32VppBuf_CopyExtradataOmx(pstBufSrc, pstBufDst, eDstBufType);
#endif
    else
        return VPP_ERR_PARAM;
}

uint32_t u32VppBuf_GrallocMetadataCopy(t_StVppBuf *pstBufSrc, t_StVppBuf *pstBufDst)
{
#ifdef VPP_GRALLOC_DOES_NOT_EXIST
    VPP_UNUSED(pstBufSrc);
    VPP_UNUSED(pstBufDst);
    return VPP_ERR_PARAM;
#else
    int s32;

    if (pstBufSrc == pstBufDst ||
        !pstBufSrc || !pstBufDst ||
        !pstBufSrc->pBuf || !pstBufDst->pBuf ||
        !pstBufSrc->pBuf->pvGralloc || !pstBufDst->pBuf->pvGralloc)
        return VPP_ERR_PARAM;

    LOGI("copying gralloc from %p to %p", pstBufSrc, pstBufDst);

    s32 = copyMetaData(GRALLOC_HDL(pstBufSrc), GRALLOC_HDL(pstBufDst));
    if (s32)
    {
        LOGE("gralloc copyMetaData returned err, s32=%d", s32);
        return VPP_ERR;
    }

    return VPP_OK;
#endif
}

uint32_t u32VppBuf_GrallocFramerateMultiply(t_StVppBuf *pstBuf, uint32_t u32Factor)
{
#ifdef VPP_GRALLOC_DOES_NOT_EXIST
    VPP_UNUSED(pstBuf);
    VPP_UNUSED(u32Factor);
    return VPP_ERR_PARAM;
#else
    int s32;
    float fFps = 0.0, fFpsNew = 0.0;

    if (!pstBuf || !pstBuf->pBuf || !pstBuf->pBuf->pvGralloc)
        return VPP_ERR_PARAM;

    s32 = getMetaData(GRALLOC_HDL(pstBuf), GET_REFRESH_RATE, (void *)&fFps);
    if (s32)
    {
        LOGE("getMetaData returned err, s32=%d", s32);
        return VPP_ERR;
    }

    if (!fFps)
    {
        LOGI("got refresh rate == 0 for buf=%p, not updating.", pstBuf);
        return VPP_OK;
    }

    fFpsNew = fFps * u32Factor;


    s32 = setMetaData(GRALLOC_HDL(pstBuf), UPDATE_REFRESH_RATE, (void *)&fFpsNew);
    if (s32)
    {
        LOGE("gralloc setMetaData returned err, s32=%d", s32);
        return VPP_ERR;
    }

    LOGI("multiplying gralloc fps from %f to %f for buffer %p",
         fFps, fFpsNew, pstBuf);

    return VPP_OK;
#endif
}

uint32_t u32VppBuf_GrallocFrameTypeSet(t_StVppBuf *pstBuf, t_EVppBufType eBuf)
{
#ifdef VPP_GRALLOC_DOES_NOT_EXIST
    VPP_UNUSED(pstBuf);
    VPP_UNUSED(eBuf);
    return VPP_ERR_PARAM;
#else
    int s32, s32Enable;

    if (!pstBuf || !pstBuf->pBuf || !pstBuf->pBuf->pvGralloc)
        return VPP_ERR_PARAM;

#ifndef OMX_EXTRADATA_DOES_NOT_EXIST
    s32Enable = eVppBuf_GetOmxFrameTypeFromInternal(eBuf);
#else
    s32Enable = (eBuf == eVppBufType_Progressive) ? 0 : 1;
#endif
    s32 = setMetaData(GRALLOC_HDL(pstBuf), PP_PARAM_INTERLACED,
                      (void *) &s32Enable);

    if (s32)
    {
        LOGE("gralloc setMetaData returned err, s32=%d", s32);
        return VPP_ERR;
    }

    LOGI("setting gralloc frametype interlaced=%d for buffer %p\n", s32Enable,
         pstBuf);

    return VPP_OK;
#endif
}

uint32_t u32VppBuf_GrallocFrameTypeGet(t_StVppBuf *pstBuf,
                                       t_EVppBufType *peBufType)
{
#ifdef VPP_GRALLOC_DOES_NOT_EXIST
    VPP_UNUSED(pstBuf);
    VPP_UNUSED(peBufType);
    return VPP_ERR_PARAM;
#else
    int s32Ret, s32 = 0;

    if (!pstBuf || !pstBuf->pBuf || !pstBuf->pBuf->pvGralloc)
        return VPP_ERR_PARAM;

    s32Ret = getMetaData(GRALLOC_HDL(pstBuf), GET_PP_PARAM_INTERLACED, &s32);
    if (!s32Ret)
    {
#ifndef OMX_EXTRADATA_DOES_NOT_EXIST
        *peBufType = eVppBuf_GetInternalFrameTypeFromOmx((enum OMX_INTERLACETYPE)s32);
#else
        *peBufType = eVppBufType_Progressive;
        if (s32)
            *peBufType = eVppBufType_Interleaved_TFF;
#endif
    }

    LOGI("getting gralloc frametype, s32Ret=%d, s32=%d, peBufType=0x%x",
         s32Ret, s32, *peBufType);

    return s32Ret ? VPP_ERR : VPP_OK;
#endif
}

t_EVppBufType eVppBuf_GetFrameType(t_StVppBuf *pstBuf)
{
    uint32_t u32 = VPP_OK;
    t_EVppBufType eRet = eVppBufType_Progressive;

    if (!pstBuf)
        return eRet;

    u32 = u32VppBuf_GrallocFrameTypeGet(pstBuf, &eRet);
    if (u32 == VPP_OK)
        return eRet;

    u32 = u32VppBuf_GetFrameTypeExtradata(pstBuf, VPP_EXTERNAL_EXTRADATA_TYPE, &eRet);
    if (u32 == VPP_OK)
        return eRet;

    return eVppBufType_Progressive;
}

void vVppBuf_SetDumpParams(t_StVppCtx *pstCtx, char *pcPath)
{
    t_StVppBufCb *pstBufCb;

    if (!pstCtx)
    {
        LOGE("%s called with null ctx", __func__);
        return;
    }

    pstBufCb = pstCtx->pstBufCb;

    if (pcPath)
    {
        strlcpy(pstBufCb->cBufDumpPath, pcPath, sizeof(pstBufCb->cBufDumpPath));
    }
    else
    {
        strlcpy(pstBufCb->cBufDumpPath, VPP_DEFAULT_DUMP_BUF_PATH,
                sizeof(pstBufCb->cBufDumpPath));
    }
    LOGI("setting buffer dump path: %s", pstBufCb->cBufDumpPath);
}

#define VPP_DUMP_BUF_LEN 256

static void vVppBuf_GetDumpPath(t_StVppBufCb *pstBufCb,
                                char *pcPath,
                                uint32_t u32PathLen,
                                const char *pcFn)
{
    size_t len;

    VPP_RET_VOID_IF_NULL(pstBufCb);

    mkdir(pstBufCb->cBufDumpPath, S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH);

    len = strlen(pstBufCb->cBufDumpPath);
    strlcpy(pcPath, pstBufCb->cBufDumpPath, u32PathLen);
    if (len > 0 && len < u32PathLen && pcPath[len - 1] != '/')
    {
        pcPath[len++] = '/';
        pcPath[len] = '\0';
    }
    strlcat(pcPath, pcFn, u32PathLen);
}

uint32_t u32VppBuf_Dump(t_StVppCtx *pstCtx, t_StVppBuf *pstBuf,
                        const char *pcFn, uint32_t u32W, uint32_t u32H,
                        uint32_t u32Str, uint32_t u32UvOff)
{
    char cPath[VPP_DUMP_BUF_LEN];
    FILE *fd;
    int offset;
    char *base, *virt;
    uint32_t u32YSz, u32UVSz, i;
    uint32_t u32Ret;
    t_StVppBufCb *pstBufCb;

    VPP_RET_IF_NULL(pstCtx, VPP_ERR_PARAM);
    pstBufCb = pstCtx->pstBufCb;
    VPP_RET_IF_NULL(pstBufCb, VPP_ERR_PARAM);

    if (!pstBuf)
    {
        LOGE("unable to dump buffer, pstCtx=%p, pstBuf=%p, fp=%s, fn=%s",
             pstCtx, pstBuf, pstBufCb->cBufDumpPath, pcFn);
        return VPP_ERR_PARAM;
    }

    memset(cPath, 0, sizeof(cPath));

    vVppBuf_GetDumpPath(pstBufCb, cPath, VPP_DUMP_BUF_LEN, pcFn);

    fd = fopen(cPath, "wb");
    if (!fd)
    {
        LOGE("Unable to open: %s", cPath);
        return VPP_ERR;
    }

    offset = pstBuf->pBuf->pixel.offset;
    base = pstBuf->stPixel.pvBase;
    virt = base + offset;

    u32YSz = u32W * u32H;
    u32UVSz = u32YSz / 2;

    LOGD("dumping: %s: offset=%d, base=%p, virt=%p, u32YSz=%u, fd=%p", cPath, offset,
         base, virt, u32YSz, fd);

    if(u32W == u32Str)
    {
        u32Ret = fwrite(virt, sizeof(char), u32YSz, fd);
        if(u32Ret != u32YSz)
        {
            LOGE("File write Y failed.");
            goto ERR_WRITE;
        }

        // Venus has alignment restrictions
        virt = virt + u32UvOff;
        u32Ret = fwrite(virt, sizeof(char), u32UVSz, fd);
        if(u32Ret != u32UVSz)
        {
            LOGE("File write UV failed.");
            goto ERR_WRITE;
        }
    }
    else
    {
        LOGI("Removing stride alignment of %d on file write\n", u32Str);
        for(i = 0; i < u32H; i++)
        {
            u32Ret = fwrite(virt, sizeof(char), u32W, fd);
            if(u32Ret != u32W)
            {
                LOGE("File write Y stride line failed.");
                goto ERR_WRITE;
            }
            virt += u32Str;
        }
        virt = base + offset + u32UvOff;
        for(i = 0; i < (u32H >> 1); i++)
        {
            u32Ret = fwrite(virt, sizeof(char), u32W, fd);
            if(u32Ret != u32W)
            {
                LOGE("File write UV stride line failed.");
                goto ERR_WRITE;
            }
            virt += u32Str;
        }
    }

ERR_WRITE:
    fclose(fd);

    return VPP_OK;
}

uint32_t u32VppBuf_DumpExtraData(t_StVppCtx *pstCtx,
                                 t_StVppBuf *pstBuf,
                                 const char *pcFn)
{
    char cPath[VPP_DUMP_BUF_LEN];
    size_t len;
    FILE *fd;
    char *base;
    uint32_t bytes, i, u32Ret;
    t_StVppBufCb *pstBufCb;

    VPP_RET_IF_NULL(pstCtx, VPP_ERR_PARAM);
    pstBufCb = pstCtx->pstBufCb;
    VPP_RET_IF_NULL(pstBufCb, VPP_ERR_PARAM);

    if (!pstBuf)
    {
        LOGE("unable to dump buffer, pstCtx=%p, pstBuf=%p, fp=%s, fn=%s",
             pstCtx, pstBuf, pstBufCb->cBufDumpPath, pcFn);
        return VPP_ERR_PARAM;
    }

    memset(cPath, 0, sizeof(cPath));
    vVppBuf_GetDumpPath(pstBufCb, cPath, VPP_DUMP_BUF_LEN, pcFn);

    fd = fopen(cPath, "wb");
    if (!fd)
    {
        LOGE("Unable to open: %s", cPath);
        return VPP_ERR;
    }
    base = pstBuf->stExtra.pvBase;
    bytes = pstBuf->stExtra.u32FilledLen;

    LOGD("dumping extradata: %s: base=%p, bytes=%u, fd=%p",
         cPath, base, bytes, fd);

    u32Ret = fwrite(base, sizeof(char), bytes, fd);
    if(u32Ret != bytes)
    {
        LOGE("File write Y failed.");
        goto ERR_WRITE;
    }

ERR_WRITE:
    fclose(fd);

    return VPP_OK;
}

uint32_t u32Vpp_CrcBuffer(t_StVppBuf *pstBuf, t_EVppTypeBuf eBuf, uint32_t start_offset,
                          uint32_t len, uint32_t idx, char* pmsg)
{
    uint32_t* p = NULL;
    uint32_t size = 0;
    uint32_t crc = 0;
    char *base = NULL;
    char *virt = NULL;
    t_StVppMemBuf* pstmem_buf;
    LOGI("%s\n", __func__);
    VPP_RET_IF_NULL(pstBuf,0);
    VPP_RET_IF_NULL(pmsg,0);

    switch (eBuf)
    {
        case eVppBuf_Pixel:
            pstmem_buf=&(pstBuf->stPixel);
            break;
        case eVppBuf_Extra:
            pstmem_buf=&(pstBuf->stExtra);
            break;
        default:
            LOGE("%s Error: Wrong eBuf=%d", __func__, eBuf);
            return(0);
    }

    base = pstmem_buf->pvBase;
    if (!base)
    {
        LOGE("%s Error: base is NULL", __func__);
        return 0;
    }
    if (pstmem_buf->u32MappedLen < (start_offset+len))
    {
        LOGE("%s Error: MappedLen is to small %d", __func__,pstmem_buf->u32MappedLen);
        return 0;
    }
    virt = base + start_offset;
    p = (uint32_t*)virt;
    size = len / 4;

    //CRC generation
    uint32_t i;
    for (i = 0; i < size; i++)
    {
       crc += (uint32_t) * (p + i);
    }

    LOGI("%s idx_vpp=%d, eBuf=%d, crc=0x%x, pstBuf=%p, flags=0x%x, start_offset=0x%x, len=%d",
          pmsg, idx, eBuf, crc, pstBuf, pstBuf->pBuf->flags, start_offset, len);
    LOG_EXIT_RET(crc);
}
