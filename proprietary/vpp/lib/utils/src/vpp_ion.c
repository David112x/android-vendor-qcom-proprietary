/*!
 * @file dvp_ion.c
 *
 * @cr
 * Copyright (c) 2015, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @services
 *
 * @description
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <pthread.h>
#include <ion/ion.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define VPP_LOG_TAG     VPP_LOG_MODULE_ION_TAG
#define VPP_LOG_MODULE  VPP_LOG_MODULE_ION
#include "vpp_dbg.h"

#include "vpp.h"
#include "vpp_def.h"
#include "vpp_ctx.h"

#include "vpp_ion.h"

/************************************************************************
 * Local definitions
 ***********************************************************************/
#define ION_BUF_ALIGN 4096
#define ION_BUF_ALIGN_SECURE ION_BUF_ALIGN

#define VPP_ION_SESSION_MAX 16
/************************************************************************
 * Local static variables
 ***********************************************************************/
typedef struct {
    int fd_dev_ion;
    pthread_mutex_t mutex;
    t_StVppCtx *apstCtx[VPP_ION_SESSION_MAX];
} t_StVppIonCb;

static t_StVppIonCb sStIonCb = {
    .fd_dev_ion = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

/************************************************************************
 * Forward Declarations
 ***********************************************************************/
/************************************************************************
 * Local functions
 ***********************************************************************/
static uint32_t u32VppIon_IsSessionInitialized(t_StVppCtx *pstCtx)
{
    uint32_t i;
    for (i = 0; i < VPP_ION_SESSION_MAX; i++)
    {
        if (sStIonCb.apstCtx[i] == pstCtx)
            return VPP_TRUE;
    }

    return VPP_FALSE;
}

static void vVppIon_ClearIonBuf(t_StVppIonBuf *pstBuf)
{
    VPP_RET_VOID_IF_NULL(pstBuf);

    memset(pstBuf, 0, sizeof(*pstBuf));
    pstBuf->fd_ion_mem = -1;
    pstBuf->buf = MAP_FAILED;
}

/************************************************************************
 * Global functions
 ***********************************************************************/
uint32_t u32VppIon_Init(t_StVppCtx *pstCtx)
{
    uint32_t i, u32 = VPP_OK;
    uint32_t u32IdxFree = VPP_ION_SESSION_MAX;
    int fd = -1;

    if (!pstCtx)
        return VPP_ERR_PARAM;

    if (u32VppIon_IsSessionInitialized(pstCtx))
    {
        LOGE("%s(), trying to reinitialize ion for ctx=%p", __func__, pstCtx);
    }

    pthread_mutex_lock(&sStIonCb.mutex);

    if (sStIonCb.fd_dev_ion < 0)
    {
        fd = ion_open();
        if (fd < 0)
        {
            LOGE("ion_open failed, err=%s", strerror(errno));
            u32 = VPP_ERR_RESOURCES;
            goto vpp_ion_init_exit;
        }

        LOGI("ion_open succeeded, fd=%d", fd);
        sStIonCb.fd_dev_ion = fd;
    }

    for (i = 0; i < VPP_ION_SESSION_MAX; i++)
    {
        if (sStIonCb.apstCtx[i] == NULL)
        {
            u32IdxFree = i;
            continue;
        }

        if (sStIonCb.apstCtx[i] == pstCtx)
        {
            LOGE("%s(), found existing context=%p in slot=%u", __func__, pstCtx, i);
            goto vpp_ion_init_exit;
        }
    }

    if (i == VPP_ION_SESSION_MAX)
    {
        if (u32IdxFree < VPP_ION_SESSION_MAX)
        {
            sStIonCb.apstCtx[u32IdxFree] = pstCtx;
            LOGI("%s(), ctx=%p stored in slot=%u", __func__, pstCtx, u32IdxFree);
        }
        else
        {
            LOGE("%s(), ion dev opened, but out of context slots", __func__);
            u32 = VPP_ERR_RESOURCES;
        }
    }

vpp_ion_init_exit:
    pthread_mutex_unlock(&sStIonCb.mutex);
    return u32;
}

uint32_t u32VppIon_Term(t_StVppCtx *pstCtx)
{
    int rc = 0;
    uint32_t i, u32Cnt = 0;

    if (!pstCtx)
        return VPP_ERR_PARAM;

    if (!u32VppIon_IsSessionInitialized(pstCtx))
    {
        LOGE("%s(), trying to term uninitialized session, pstCtx=%p", __func__,
             pstCtx);
        return VPP_ERR_PARAM;
    }

    pthread_mutex_lock(&sStIonCb.mutex);

    for (i = 0; i < VPP_ION_SESSION_MAX; i++)
    {
        if (sStIonCb.apstCtx[i] != NULL)
        {
            if (sStIonCb.apstCtx[i] != pstCtx)
                u32Cnt += 1;
            else
            {
                LOGI("%s(), purging ctx=%p stored in slot=%u", __func__, pstCtx, i);
                sStIonCb.apstCtx[i] = NULL;
            }
        }
    }


    if (u32Cnt == 0)
    {
        LOGI("closing ion dev node, fd=%d", sStIonCb.fd_dev_ion);
        rc = ion_close(sStIonCb.fd_dev_ion);
        LOGE_IF(rc < 0, "ion_close failed, rc=%d", rc);
        sStIonCb.fd_dev_ion = -1;
    }

    pthread_mutex_unlock(&sStIonCb.mutex);
    return VPP_OK;
}

uint32_t u32VppIon_Alloc(t_StVppCtx *pstCtx, uint32_t u32Len, uint32_t u32Secure,
                         t_StVppIonBuf *buf)
{
    int rc = 0;
    uint32_t u32Flags = 0;
    uint32_t u32HeapMask = 0;
    uint32_t u32Align = 0;

    if (!pstCtx)
        return VPP_ERR_PARAM;

    if (!buf)
        return VPP_ERR_PARAM;

    if (!u32VppIon_IsSessionInitialized(pstCtx))
    {
        LOGE("%s() called with uninitialized session", __func__);
        return VPP_ERR_STATE;
    }

    LOG_ENTER_ARGS("pstCtx=%p, fd_dev_ion=%d, u32Len=%u, bSecure=%u, buf=%p",
                   pstCtx, sStIonCb.fd_dev_ion, u32Len, u32Secure, buf);

    vVppIon_ClearIonBuf(buf);


    if (u32Secure)
    {
        u32Flags = ION_FLAG_SECURE | ION_FLAG_CP_PIXEL;
        u32HeapMask = ION_HEAP(ION_SECURE_HEAP_ID);
        u32Align = ION_BUF_ALIGN_SECURE;
    }
    else
    {
        u32HeapMask = ION_HEAP(ION_SYSTEM_HEAP_ID);
        u32Align = ION_BUF_ALIGN;
    }

    // Starting in kernel 4.14, alignment is not passed to ion, however most of
    // our clients will still have some memory alignment requirement. Therefore
    // we will explicitly align it.
    u32Len += u32Align - 1;
    u32Len &= ~(u32Align - 1);

    // Allocate ion memory
    rc = ion_alloc_fd(sStIonCb.fd_dev_ion, u32Len, u32Align,
                      u32HeapMask, u32Flags, &buf->fd_ion_mem);
    if (rc < 0)
    {
        LOGE("ion_alloc_fd failed, rc=%d", rc);
        buf->fd_ion_mem = -1;
        goto ION_ALLOC_FAILED;
    }

    buf->buf = MAP_FAILED;
    if (!u32Secure)
    {
        // Map the ion buffer into virtual address space
        int prot_flags = PROT_READ | PROT_WRITE;
        buf->buf = mmap(NULL, u32Len, prot_flags, MAP_SHARED, buf->fd_ion_mem, 0);
        if (buf->buf == MAP_FAILED)
        {
            LOGE("mmap failed: %s, errno=%d, fd=%d, len=%u",
                 strerror(errno), errno, buf->fd_ion_mem, u32Len);
            goto ION_MAP_FAILED;
        }
    }

    buf->len = u32Len;
    buf->bSecure = u32Secure;

    return VPP_OK;

ION_MAP_FAILED:
    close(buf->fd_ion_mem);
    vVppIon_ClearIonBuf(buf);

ION_ALLOC_FAILED:
    return VPP_ERR;

}

uint32_t u32VppIon_Free(t_StVppCtx *pstCtx, t_StVppIonBuf *pstBuf)
{
    int rc = 0;

    if (!pstCtx)
        return VPP_ERR_PARAM;

    if (!pstBuf)
        return VPP_ERR_PARAM;

    if (!u32VppIon_IsSessionInitialized(pstCtx))
    {
        LOGE("%s() called with uninitialized session", __func__);
        return VPP_ERR_STATE;
    }

    if (!pstBuf->bSecure)
    {
        rc = munmap(pstBuf->buf, pstBuf->len);
        if (rc < 0)
        {
            LOGE("munmap: error, rc=%d", rc);
        }
    }

    if (pstBuf->fd_ion_mem)
    {
        close(pstBuf->fd_ion_mem);
    }

    vVppIon_ClearIonBuf(pstBuf);
    return VPP_OK;
}

uint32_t u32VppIon_GetHeapId(t_StVppCtx *pstCtx, uint32_t bSecure)
{
    VPP_UNUSED(pstCtx);
    if (bSecure)
        return ION_SECURE_HEAP_ID;
    else
        return ION_SYSTEM_HEAP_ID;
}
