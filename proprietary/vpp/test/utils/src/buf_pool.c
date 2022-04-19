/*!
 * @file buf_pool.c
 *
 * @cr
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

 * @services
 *
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <media/msm_vidc.h>

#include "dvpTest.h"
#include "dvpTest_tb.h"

#define VPP_LOG_TAG     VPP_LOG_TEST_UTILS_POOL_TAG
#define VPP_LOG_MODULE  VPP_LOG_TEST_UTILS_POOL

#include "vpp_dbg.h"
#include "vpp_def.h"
#include "vpp_utils.h"

#include "vpp_ion.h"

#ifndef OMX_EXTRADATA_DOES_NOT_EXIST
#include "OMX_Core.h"
#include "OMX_QCOMExtns.h"
#endif

#include "buf_pool.h"


#define FILL_BUF
#define DUMP_BUF
//#define DUMP_PADDED

/************************************************************************
 * Local definitions
 ***********************************************************************/

// Add additional extradata types for use in the test suite

// Ensure the enumeration doesn't conflict with the vendor extensions
typedef enum OMX_VPP_TEST_EXTRADATATYPE {
    OMX_ExtraDataBufMarker = 0x7ff00000,
} OMX_VPP_TEST_EXTRADATATYPE;

typedef struct OMX_VPP_TEST_BUFMARKER {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVerion;
    OMX_U32 nMarker;
} OMX_VPP_TEST_BUFMARKER;

// Ensure the enumeration doesn't conflict with the msm extra data types
typedef enum MSM_VPP_TEST_EXTRADATATYPE {
    MSM_VPP_TEST_EXTRADATA_BUFMARKER = 0x7ff00000,
} MSM_VPP_TEST_EXTRADATATYPE;

typedef struct msm_vpp_test_bufmarker {
    uint32_t nMarker;
} msm_vpp_test_bufmarker;

/************************************************************************
 * Local static variables
 ***********************************************************************/
static struct {
    uint32_t u32AllocCnt;
    uint8_t au8FdAlloc[NUM_FD_MAX];
    uint8_t au8FdFree[NUM_FD_MAX];
    uint32_t u32TotalAllocated;
} stBufPoolStats;

/*!
 * Brook_NV12_420_720x480i_short.yuv
 * Flower_NV12_420_720x480i_short.yuv
 * Hair_NV12_420_720x480i_short.yuv
 * Stone_Bridge_NV12_420_720x480i_short.yuv
 * Wood_NV12_420_720x480i_short.yuv
 *
 * Brook_NV12_420_1920x1080i_short.yuv
 * Canyon_NV12_420_1920x1080i_short.yuv
 * Hair_NV12_420_1920x1080i_short.yuv
 * Wood_NV12_420_1920x1080i_short.yuv
 */
static const struct buf_pool_params clip_registry[CLIP_REG_MAX] = {
    [CLIP_CANYON_720x480] = {
        .u32Width = 720,
        .u32Height = 480,
        .u32MaxInputFrames = 20,

        .cInputPath = INPUT_FILEPATH_TEMPLATE,
        .cInputName = "Canyon_NV12_420_720x480i_short.yuv",
        .eInputFileFormat = FILE_FORMAT_NV12,
        .eInputFileType = FILE_TYPE_MULTI_FRAMES,
        .eInputBufFmt = VPP_COLOR_FORMAT_NV12_VENUS,
        .eBufferType = eVppBufType_Progressive,

        .cOutputPath = OUTPUT_FILEPATH_TEMPLATE,
        .cOutputName = "out_Canyon_NV12_420_720x480i_short.yuv",
        .eOutputFileFormat = FILE_FORMAT_NV12,
        .eOutputFileType = FILE_TYPE_MULTI_FRAMES,
        .eOutputBufFmt = VPP_COLOR_FORMAT_NV12_VENUS,
    },
    [CLIP_FLOWER_1920x1080] = {
        .u32Width = 1920,
        .u32Height = 1080,
        .u32MaxInputFrames = 20,

        .cInputPath = INPUT_FILEPATH_TEMPLATE,
        .cInputName = "Flower_NV12_420_1920x1080i_short.yuv",
        .eInputFileFormat = FILE_FORMAT_NV12,
        .eInputFileType = FILE_TYPE_MULTI_FRAMES,
        .eInputBufFmt = VPP_COLOR_FORMAT_NV12_VENUS,
        .eBufferType = eVppBufType_Progressive,

        .cOutputPath = OUTPUT_FILEPATH_TEMPLATE,
        .cOutputName = "out_Flower_NV12_420_1920x1080i_short.yuv",
        .eOutputFileFormat = FILE_FORMAT_NV12,
        .eOutputFileType = FILE_TYPE_MULTI_FRAMES,
        .eOutputBufFmt = VPP_COLOR_FORMAT_NV12_VENUS,
    },
    [CLIP_HAIR_1920x1080] = {
        .u32Width = 1920,
        .u32Height = 1080,
        .u32MaxInputFrames = 20,

        .cInputPath = INPUT_FILEPATH_TEMPLATE,
        .cInputName = "Hair_NV12_420_1920x1080i_short.yuv",
        .eInputFileFormat = FILE_FORMAT_NV12,
        .eInputFileType = FILE_TYPE_MULTI_FRAMES,
        .eInputBufFmt = VPP_COLOR_FORMAT_NV12_VENUS,
        .eBufferType = eVppBufType_Progressive,

        .cOutputPath = OUTPUT_FILEPATH_TEMPLATE,
        .cOutputName = "out_Hair_NV12_420_1920x1080i_short.yuv",
        .eOutputFileFormat = FILE_FORMAT_NV12,
        .eOutputFileType = FILE_TYPE_MULTI_FRAMES,
        .eOutputBufFmt = VPP_COLOR_FORMAT_NV12_VENUS,
    },
    [CLIP_FRC_1920x1080] = {
        .u32Width = 1920,
        .u32Height = 1080,
        .u32MaxInputFrames = 34,

        .cInputPath = INPUT_FILEPATH_TEMPLATE,
        .cInputName = "yuvgtmotion_nv12_1080p.yuv",
        .eInputFileFormat = FILE_FORMAT_NV12,
        .eInputFileType = FILE_TYPE_MULTI_FRAMES,
        .eInputBufFmt = VPP_COLOR_FORMAT_NV12_VENUS,
        .eBufferType = eVppBufType_Progressive,

        .cOutputPath = OUTPUT_FILEPATH_TEMPLATE,
        .cOutputName = "out_yuvgtmotion_nv12_1080p.yuv",
        .eOutputFileFormat = FILE_FORMAT_NV12,
        .eOutputFileType = FILE_TYPE_MULTI_FRAMES,
        .eOutputBufFmt = VPP_COLOR_FORMAT_NV12_VENUS,
    },
    [CLIP_NEW_YORK_3840x2160] = {
        .u32Width = 3840,
        .u32Height = 2160,
        .u32MaxInputFrames = 20,

        .cInputPath = INPUT_FILEPATH_TEMPLATE,
        .cInputName = "NewYork_nv12.yuv",
        .eInputFileFormat = FILE_FORMAT_NV12,
        .eInputFileType = FILE_TYPE_MULTI_FRAMES,
        .eInputBufFmt = VPP_COLOR_FORMAT_NV12_VENUS,
        .eBufferType = eVppBufType_Progressive,

        .cOutputPath = OUTPUT_FILEPATH_TEMPLATE,
        .cOutputName = "out_NewYork_nv12.yuv",
        .eOutputFileFormat = FILE_FORMAT_NV12,
        .eOutputFileType = FILE_TYPE_MULTI_FRAMES,
        .eOutputBufFmt = VPP_COLOR_FORMAT_NV12_VENUS,
    },
    [CLIP_BEACH_1280x720] = {
        .u32Width = 1280,
        .u32Height = 720,
        .u32MaxInputFrames = 20,

        .cInputPath = INPUT_FILEPATH_TEMPLATE,
        .cInputName = "Beach_NV12_1280x720.yuv",
        .eInputFileFormat = FILE_FORMAT_NV12,
        .eInputFileType = FILE_TYPE_MULTI_FRAMES,
        .eInputBufFmt = VPP_COLOR_FORMAT_NV12_VENUS,
        .eBufferType = eVppBufType_Progressive,

        .cOutputPath = OUTPUT_FILEPATH_TEMPLATE,
        .cOutputName = "out_Beach_NV12_1280x720.yuv",
        .eOutputFileFormat = FILE_FORMAT_NV12,
        .eOutputFileType = FILE_TYPE_MULTI_FRAMES,
        .eOutputBufFmt = VPP_COLOR_FORMAT_NV12_VENUS,
    },
};

static struct msm_vidc_s3d_frame_packing_payload stFramePackingRefMsm = {
    .fpa_id =                       0xbd637aca,
    .cancel_flag =                  0x1b372d8d,
    .fpa_type =                     0x46032815,
    .quin_cunx_flag =               0xdf33cf8a,
//    .content_interprtation_type =   0x564a8cf9,
    .spatial_flipping_flag =        0x7a2977c1,
    .frame0_flipped_flag =          0xcd2ef26f,
    .field_views_flag =             0xc0709bcb,
    .current_frame_is_frame0_flag = 0x7dc191f8,
    .frame0_self_contained_flag =   0xe5fb3050,
    .frame1_self_contained_flag =   0x7966d2da,
    .frame0_graid_pos_x =           0x5d811,
    .frame0_graid_pos_y =           0xb79ef499,
    .frame1_graid_pos_x =           0x0,
    .frame1_graid_pos_y =           0x2c8043ed,
    .fpa_reserved_byte =            0x2c3b56ba,
    .fpa_repetition_period =        0x13628917,
    .fpa_extension_flag =           0x5b327edf,
};

static struct msm_vidc_panscan_window stPanScanWindowMsm = {
    .panscan_height_offset =        0x4f3d3214,
    .panscan_width_offset =         0xd9f5b76b,
    .panscan_window_width =         0xb923bc61,
    .panscan_window_height =        0x8b944527,
};

#ifndef OMX_EXTRADATA_DOES_NOT_EXIST
static OMX_QCOM_FRAME_PACKINGFORMAT_TYPE stFramePackingRefOmx = {
    .nSize =                        0xbd637aca,
    .nVersion.nVersion =            0x1b372d8d,
    .nPortIndex =                   0x46032815,
    .nIndex =                       0xdf33cf8a,
    .eframePackingFormat =          0x7a2977c1,
};

static OMX_QCOM_PANSCAN stPanScanWindowOmx = {
    .numWindows =                   0x4f3d3214,
    .window[0] = {
        .x =                        0xd9f5b76b,
        .y =                        0xfa536421,
        .dx =                       0x542312fa,
        .dy =                       0x132cd432,
    },
    .window[1] = {
        .x =                        0xcd2ef26f,
        .y =                        0xc0709bcb,
        .dx =                       0x7dc191f8,
        .dy =                       0xe5fb3050,
    },
    .window[2] = {
        .x =                        0x7966d2da,
        .y =                        0x5d811,
        .dx =                       0xb79ef499,
        .dy =                       0x5,
    },
    .window[3] = {
        .x =                        0x2c8043ed,
        .y =                        0x2c3b56ba,
        .dx =                       0x13628917,
        .dy =                       0x5b327edf,
     },
};
#endif

#define MINIMUM_EXTRADATA_SIZE_MSM (sizeof(struct msm_vidc_extradata_header) + \
                                    sizeof(struct msm_vidc_s3d_frame_packing_payload) + \
                                    sizeof(struct msm_vidc_extradata_header) + \
                                    sizeof(struct msm_vidc_panscan_window) + \
                                    sizeof(struct msm_vidc_extradata_header) + \
                                    sizeof(struct msm_vidc_interlace_payload) + \
                                    sizeof(struct msm_vidc_extradata_header) + \
                                    sizeof(struct msm_vpp_test_bufmarker))

#ifndef OMX_EXTRADATA_DOES_NOT_EXIST
#define MINIMUM_EXTRADATA_SIZE_OMX (sizeof(OMX_OTHER_EXTRADATATYPE) + \
                                    sizeof(OMX_QCOM_FRAME_PACKINGFORMAT_TYPE) + \
                                    sizeof(OMX_OTHER_EXTRADATATYPE) + \
                                    sizeof(OMX_QCOM_PANSCAN) + \
                                    sizeof(OMX_OTHER_EXTRADATATYPE) + \
                                    sizeof(OMX_STREAMINTERLACEFORMAT) + \
                                    sizeof(OMX_OTHER_EXTRADATATYPE) + \
                                    sizeof(OMX_VPP_TEST_BUFMARKER))
#endif
/************************************************************************
 * Forward Declarations
 ************************************************************************/

/************************************************************************
 * Local Functions
 ***********************************************************************/

static void allocated_fd(int fd)
{
    if (fd < NUM_FD_MAX)
        stBufPoolStats.au8FdAlloc[fd]++;
}

static void freed_fd(int fd)
{
    if (fd < NUM_FD_MAX)
        stBufPoolStats.au8FdFree[fd]++;
}

static uint32_t buf_pool_alloc_ion(t_StVppCtx *pstCtx,
                                   enum buf_pool_protection eProt,
                                   uint32_t u32Sz,
                                   t_StVppIonBuf *pstIonBuf,
                                   struct vpp_mem_buffer *pstVppMemBuf,
                                   t_StVppMemBuf *pstIntMemBuf)
{
    uint32_t u32;
    uint32_t u32Secure;

    if (eProt >= PROTECTION_ZONE_MAX)
    {
        LOGE("Invalid params to alloc_ion for cpz. eProt=%u", eProt);
        return VPP_ERR;
    }

    u32Secure = eProt == PROTECTION_ZONE_SECURE ? VPP_TRUE : VPP_FALSE;
    u32 = u32VppIon_Alloc(pstCtx, u32Sz, u32Secure, pstIonBuf);
    if (u32 != VPP_OK)
        return u32;

    LOGI("allocating %s ion mem fd=%d addr=%p sz=%u",
         u32Secure ? "secure" : "non-secure",
         pstIonBuf->fd_ion_mem,
         pstIonBuf->buf, u32Sz);

    stBufPoolStats.u32TotalAllocated += u32Sz;
    allocated_fd(pstIonBuf->fd_ion_mem);

    pstVppMemBuf->fd = pstIonBuf->fd_ion_mem;
    pstVppMemBuf->offset = 0;
    pstVppMemBuf->alloc_len = u32Sz;
    pstVppMemBuf->valid_data_len = u32Sz;
    pstVppMemBuf->filled_len = 0;
#ifdef VPP_NO_MMAP_ON_QUEUE_BUF
    if (!u32Secure)
        pstVppMemBuf->pvMapped = pstIonBuf->buf;
#endif

    pstIntMemBuf->fd = pstIonBuf->fd_ion_mem;
    pstIntMemBuf->u32AllocLen = u32Sz;
    pstIntMemBuf->u32FilledLen = 0;
    pstIntMemBuf->u32Offset = 0;
    pstIntMemBuf->u32ValidLen = u32Sz;


    if (!u32Secure)
    {
        pstIntMemBuf->pvBase = pstIonBuf->buf;
        pstIntMemBuf->u32MappedLen = u32Sz;
        pstIntMemBuf->priv.pvPa = 0;
        pstIntMemBuf->priv.u32PaOffset = 0;
        pstIntMemBuf->priv.u32PaMappedLen = 0;

        memset(pstIntMemBuf->pvBase, 0, u32Sz);
    }
    else
    {
        pstIntMemBuf->pvBase = VPP_BUF_UNMAPPED_BUF_VAL;
        pstIntMemBuf->u32MappedLen = 0;
        pstIntMemBuf->priv.pvPa = VPP_BUF_UNMAPPED_BUF_VAL;
        pstIntMemBuf->priv.u32PaOffset = 0;
        pstIntMemBuf->priv.u32PaMappedLen = 0;
    }

    stBufPoolStats.u32AllocCnt++;

    return VPP_OK;
}
/************************************************************************
 * Global Functions
 ***********************************************************************/

void dump_buf_stats()
{
    uint32_t i;

    LOGD("--------------------------------");
    LOGD("- total allocated buffer count: %u", stBufPoolStats.u32AllocCnt);
    LOGD("- total allocated memory: %u", stBufPoolStats.u32TotalAllocated);

    for (i = 0; i < NUM_FD_MAX; i++)
    {
        if (stBufPoolStats.au8FdAlloc[i] || stBufPoolStats.au8FdFree[i])
        {
            if (stBufPoolStats.au8FdAlloc[i] != stBufPoolStats.au8FdFree[i])
                LOGE("- index[%03u] alloc=%03u free=%03u ** ", i,
                     stBufPoolStats.au8FdAlloc[i], stBufPoolStats.au8FdFree[i]);
            else
                LOGD("- index[%03u] alloc=%03u free=%03u", i,
                     stBufPoolStats.au8FdAlloc[i], stBufPoolStats.au8FdFree[i]);
        }
    }
    LOGD("--------------------------------");
}

uint32_t buf_params_init_default(struct buf_pool_params *params,
                                 struct vpp_port_param *port_param)
{
    memset(params, 0, sizeof(struct buf_pool_params));

    params->u32Width = port_param->width;
    params->u32Height = port_param->height;

    params->eInputBufFmt = port_param->fmt;
    params->eOutputBufFmt = port_param->fmt;

    if (port_param->fmt == VPP_COLOR_FORMAT_NV12_VENUS)
    {
        params->eInputFileFormat = FILE_FORMAT_NV12;
        params->eOutputFileFormat = FILE_FORMAT_NV12;
    }
    else
    {
        LOGE("Error: unsupported color format");
    }
    params->eProtection = PROTECTION_ZONE_NONSECURE;

    params->eInputFileType = FILE_TYPE_MULTI_FRAMES;
    params->eOutputFileType = FILE_TYPE_MULTI_FRAMES;
    params->eBufferType = eVppBufType_Progressive;


    params->u32MaxInputFrames = 10;

    strlcpy(params->cInputPath, INPUT_FILEPATH_TEMPLATE, MAX_FILE_SEG_LEN);
    strlcpy(params->cInputName, INPUT_FILENAME_TEMPLATE, MAX_FILE_SEG_LEN);
    strlcpy(params->cOutputPath, OUTPUT_FILEPATH_TEMPLATE, MAX_FILE_SEG_LEN);
    strlcpy(params->cOutputName, OUTPUT_FILENAME_TEMPLATE, MAX_FILE_SEG_LEN);

    return VPP_OK;
}

uint32_t populate_pool_params(enum clip_reg clip, struct buf_pool_params *params)
{
    if (clip >= CLIP_REG_MAX)
    {
        LOGE("Error: Specific clip outside range of registry.");
        return VPP_ERR;
    }

    memcpy(params, &clip_registry[clip], sizeof(*params));

    return VPP_OK;
}

uint32_t populate_port_params(enum clip_reg clip, enum vpp_port port, struct vpp_port_param *port_param)
{
    if (clip >= CLIP_REG_MAX)
    {
        LOGE("Error: Specific clip outside range of registry.");
        return VPP_ERR;
    }

    if (port != VPP_PORT_INPUT && port != VPP_PORT_OUTPUT)
    {
        LOGE("Error: Invalid port specified, port=%u", port);
        return VPP_ERR;
    }

    const struct buf_pool_params *clip_params = &clip_registry[clip];

    port_param->width = clip_params->u32Width;
    port_param->height = clip_params->u32Height;

    if (port == VPP_PORT_INPUT)
        port_param->fmt = clip_params->eInputBufFmt;
    else
        port_param->fmt = clip_params->eOutputBufFmt;

    port_param->stride = u32VppUtils_CalculateStride(port_param->width,
                                                     port_param->fmt);
    port_param->scanlines = u32VppUtils_CalculateScanlines(port_param->height,
                                                           port_param->fmt);

    return VPP_OK;
}

uint32_t get_res_for_clip(enum clip_reg clip, uint32_t *pu32Width,
                          uint32_t *pu32Height, enum vpp_color_format *peFmt)
{
    if (clip >= CLIP_REG_MAX)
        return VPP_ERR;

    if (pu32Width)
        *pu32Width = clip_registry[clip].u32Width;

    if (pu32Height)
        *pu32Height = clip_registry[clip].u32Height;

    if (peFmt)
        *peFmt = clip_registry[clip].eInputBufFmt;

    return VPP_OK;
}

uint32_t init_buf_pool(t_StVppCtx *pstCtx,
                       struct buf_pool **ppool,
                       struct buf_pool_params *params,
                       uint32_t u32Cnt,
                       uint32_t bInitIon)
{
    int rc;
    uint32_t i, u32;
    struct buf_pool *pool;
    DVP_ASSERT_PTR_NNULL(pstCtx);
    DVP_ASSERT_PTR_NNULL(params);
    DVP_ASSERT_PTR_NNULL(ppool);

    if (!params || !pstCtx || !ppool)
    {
        return VPP_ERR_PARAM;
    }

    pool = calloc(1, sizeof(struct buf_pool));
    if (!pool)
    {
        return VPP_ERR_NO_MEM;
    }

    rc = pthread_mutex_init(&pool->mutex, NULL);
    if (rc < 0)
    {
        free(pool);
        return VPP_ERR;
    }

    *ppool = pool;
    pool->pstCtx = pstCtx;

    pool->u32Cnt = u32Cnt;
    pool->u32ListSz = u32Cnt;
    pool->pstBuf = calloc(u32Cnt, sizeof(t_StVppBuf));
    pool->nodes = calloc(u32Cnt, sizeof(struct bufnode));
    pool->vpp_buffers = calloc(u32Cnt, sizeof(struct vpp_buffer));
    pool->pstIonMemPx = calloc(u32Cnt, sizeof(t_StVppIonBuf));
    pool->pstIonMemEx = calloc(u32Cnt, sizeof(t_StVppIonBuf));


    if (!pool->pstBuf || !pool->nodes || !pool->vpp_buffers ||
        !pool->pstIonMemPx || !pool->pstIonMemEx)
    {
        pthread_mutex_destroy(&pool->mutex);
        if (pool->pstBuf) free(pool->pstBuf);
        if (pool->nodes) free(pool->nodes);
        if (pool->vpp_buffers) free(pool->vpp_buffers);
        if (pool->pstIonMemPx) free(pool->pstIonMemPx);
        if (pool->pstIonMemEx) free(pool->pstIonMemEx);
        if (pool) free(pool);
        return VPP_ERR_NO_MEM;
    }

    memcpy(&pool->params, params, sizeof(*params));

    // Allocate ion buffers
    if (bInitIon)
    {
        u32 = u32VppIon_Init(pool->pstCtx);

        DVP_ASSERT_EQUAL(u32, VPP_OK);
        if (u32 != VPP_OK)
            LOGE("Error: unable to open ion device node");
    }

    // For the sake of convenience, store these
    pool->stInputPort.width = params->u32Width;
    pool->stInputPort.height = params->u32Height;
    pool->stInputPort.fmt = params->eInputBufFmt;
    pool->stInputPort.stride = u32VppUtils_CalculateStride(params->u32Width,
                                                           params->eInputBufFmt);
    pool->stInputPort.scanlines = u32VppUtils_CalculateScanlines(params->u32Height,
                                                                 params->eInputBufFmt);

    pool->stOutputPort.width = params->u32Width;
    pool->stOutputPort.height = params->u32Height;
    pool->stOutputPort.fmt = params->eOutputBufFmt;
    pool->stOutputPort.stride = u32VppUtils_CalculateStride(params->u32Width,
                                                            params->eOutputBufFmt);
    pool->stOutputPort.scanlines = u32VppUtils_CalculateScanlines(params->u32Height,
                                                                  params->eOutputBufFmt);

    uint32_t u32BufSz, u32ExBufSz, u32MbiBufSz;
    u32BufSz = u32VppUtils_GetBufferSize(&pool->stInputPort);
    u32ExBufSz = u32VppUtils_GetExtraDataBufferSize(&pool->stInputPort);
    u32MbiBufSz = (u32VppUtils_GetMbiSize(pool->stInputPort.width, pool->stInputPort.height)*2)+8192;
    if (u32MbiBufSz > u32ExBufSz)
        u32ExBufSz = u32MbiBufSz;

    LOGI("%s(): u32BufSz=%d u32ExBufSz = %d\n", __func__, u32BufSz, u32ExBufSz);


    // split up the ion buffer and assign it to the smaller ones

    for (i = 0; i < u32Cnt; i++)
    {
        u32 = buf_pool_alloc_ion(pool->pstCtx,
                                 pool->params.eProtection,
                                 u32BufSz,
                                 &pool->pstIonMemPx[i],
                                 &pool->vpp_buffers[i].pixel,
                                 &pool->pstBuf[i].stPixel);

        if (u32 != VPP_OK)
            LOGE("Error: unable to allocate px memory, i=%u, u32=%u", i, u32);

        u32 = buf_pool_alloc_ion(pool->pstCtx,
                                 PROTECTION_ZONE_NONSECURE,
                                 u32ExBufSz,
                                 &pool->pstIonMemEx[i],
                                 &pool->vpp_buffers[i].extradata,
                                 &pool->pstBuf[i].stExtra);

        if (u32 != VPP_OK)
            LOGE("Error: unable to allocate ex memory, i=%u, u32=%u", i, u32);
    }

    for (i = 0; i < u32Cnt; i++)
    {
        pool->vpp_buffers[i].cookie = &pool->nodes[i];

        pool->pstBuf[i].pBuf = &pool->vpp_buffers[i];

        pool->nodes[i].pPool = pool;
        pool->nodes[i].owner = BUF_OWNER_CLIENT;
        pool->nodes[i].pIntBuf = &pool->pstBuf[i];
        pool->nodes[i].pExtBuf = &pool->vpp_buffers[i];
        pool->nodes[i].pNext = pool->pHead;

        pool->pHead = &pool->nodes[i];
    }

    return VPP_OK;
}

void free_buf_pool(struct buf_pool *pool, uint32_t bTermIon)
{
    uint32_t i;

    if (!pool)
        return;

    DVP_ASSERT_EQUAL(pool->u32ListSz, pool->u32Cnt);
    if (pool->u32ListSz != pool->u32Cnt)
        LOGE("Error: pool->u32ListSz=%u, pool->u32Cnt=%u", pool->u32ListSz, pool->u32Cnt);

    // Free ion memory
    for (i = 0; i < pool->u32Cnt; i++)
    {
        LOGI("freeing ion mem fd=%d", pool->pstIonMemPx[i].fd_ion_mem);
        freed_fd(pool->pstIonMemPx[i].fd_ion_mem);
        u32VppIon_Free(pool->pstCtx, &pool->pstIonMemPx[i]);
        stBufPoolStats.u32AllocCnt--;

        LOGI("freeing ion mem fd=%d", pool->pstIonMemEx[i].fd_ion_mem);
        freed_fd(pool->pstIonMemEx[i].fd_ion_mem);
        u32VppIon_Free(pool->pstCtx, &pool->pstIonMemEx[i]);
        stBufPoolStats.u32AllocCnt--;
    }

    if (bTermIon)
        u32VppIon_Term(pool->pstCtx);

    pthread_mutex_destroy(&pool->mutex);
    if (pool->pstBuf) free(pool->pstBuf);
    if (pool->nodes) free(pool->nodes);
    if (pool->vpp_buffers) free(pool->vpp_buffers);
    if (pool->pstIonMemPx) free(pool->pstIonMemPx);
    if (pool->pstIonMemEx) free(pool->pstIonMemEx);
    if (pool) free(pool);
}

struct bufnode *get_buf(struct buf_pool *pool)
{
    struct bufnode *tmp = NULL;
    if (!pool || !pool->pHead || !pool->pstCtx)
        return NULL;

    pthread_mutex_lock(&pool->mutex);

    tmp = pool->pHead;
    pool->pHead = pool->pHead->pNext;
    pool->u32ListSz -= 1;

    pthread_mutex_unlock(&pool->mutex);

    tmp->pIntBuf->u32InternalFlags = 0;
    tmp->pExtBuf->flags = 0;

    LOGI("get_buf: pool: %p, node: %p, nodes remaining: %u",
         pool, tmp, pool->u32ListSz);

    return tmp;
}

void put_buf(struct buf_pool *pool, struct bufnode *node)
{
    if (!pool || !node)
        return;

    if (pool != node->pPool)
    {
        LOGE("Error: buffer node does not belong to this pool");
    }

    pthread_mutex_lock(&pool->mutex);

    node->pNext = pool->pHead;
    pool->pHead = node;
    pool->u32ListSz += 1;
    DVP_ASSERT_TRUE(pool->u32ListSz <= pool->u32Cnt);

    pthread_mutex_unlock(&pool->mutex);

    LOGI("put_buf: pool: %p, node: %p, nodes remaining: %u",
         pool, node, pool->u32ListSz);
}

uint32_t get_full_path(char *pcDst, uint32_t u32DstSz, char *pcPath,
                       char *pcTemplate, int32_t sIdx)
{
    int32_t s32RemLen;
    strlcpy(pcDst, pcPath, u32DstSz);
    s32RemLen = u32DstSz - strlen(pcDst);
    if (s32RemLen <= 0)
    {
        LOGE("%s: unable to copy, s32Max <= 0", __func__);
        return VPP_ERR;
    }

    snprintf(pcDst + strlen(pcDst), s32RemLen, pcTemplate, sIdx);

    return VPP_OK;
}

#define FN_BUF_SZ 256
uint32_t fill_buf(struct bufnode *node)
{
    uint32_t u32 = VPP_OK;
    FILE *fd;
    char cName[FN_BUF_SZ];
    struct buf_pool *pool = node->pPool;
    uint32_t u32Stride, i, u32FileSize, u32FileNumFrames, u32Ret;

    if (!pool)
    {
        LOGE("Received bufnode with no corresponding pool");
        return VPP_ERR;
    }


    LOGI("protection mode is: %d", pool->params.eProtection);

    if (pool->params.eProtection == PROTECTION_ZONE_SECURE)
    {
        LOGI("Received secure buffer... not filling buffer");
        u32 = VPP_OK;
        goto protected_buffer;
    }

#ifdef FILL_BUF
    // Currently only supporting NV12 to NV12.
    if (pool->params.eInputFileFormat != FILE_FORMAT_NV12 ||
        (pool->params.eInputBufFmt != VPP_COLOR_FORMAT_NV12_VENUS))
    {
        LOGE("Error: Reading invalid colour formats");
    }


    if(pool->params.eInputFileType == FILE_TYPE_SINGLE_FRAMES)
    {
        uint32_t u32Tmp;
        u32Tmp = get_full_path(cName, FN_BUF_SZ, pool->params.cInputPath,
                               pool->params.cInputName, (int32_t)pool->u32RdIdx);
        if (u32Tmp != VPP_OK)
        {
            LOGE("Error: unable to get full path name for fill buf");
            return VPP_ERR;
        }
    }
    else
    {
        strlcpy(cName, pool->params.cInputPath, FN_BUF_SZ);
        strlcat(cName, pool->params.cInputName, FN_BUF_SZ);
    }
    fd = fopen(cName, "rb");
    if (!fd)
    {
        if(pool->params.eInputFileType == FILE_TYPE_SINGLE_FRAMES)
        {
            uint32_t u32Tmp;
            LOGI("unable to open file: %s, resetting file counter", cName);
            pool->u32RdIdx = 0;
            u32Tmp = get_full_path(cName, FN_BUF_SZ, pool->params.cInputPath,
                                   pool->params.cInputName, pool->u32RdIdx);
            if (u32Tmp != VPP_OK)
            {
                LOGE("ERROR: unable to copy length < 0");
                return VPP_ERR;
            }
            fd = fopen(cName, "rb");
            if (!fd)
            {
                LOGE("Error: unable to open file: %s", cName);
                u32 = VPP_ERR;
                goto err_open_file;
            }
        }
        else
        {
            LOGE("Error: unable to open file: %s", cName);
            u32 = VPP_ERR;
            goto err_open_file;
        }

    }

    char *base = node->pIntBuf->stPixel.pvBase;
    char *virt = node->pIntBuf->stPixel.pvBase;
    u32Stride = u32VppUtils_GetStride(&pool->stInputPort);

    LOGI("reading: %s --> virt=%p", cName, virt);

    if (pool->params.eInputBufFmt == VPP_COLOR_FORMAT_NV12_VENUS &&
        pool->params.eInputFileFormat == FILE_FORMAT_NV12)
    {
        uint32_t u32YSz;
        uint32_t u32UVSz;

        u32YSz = pool->params.u32Width * pool->params.u32Height;
        u32UVSz = u32YSz / 2;

        if(pool->params.eInputFileType != FILE_TYPE_SINGLE_FRAMES)
        {
            fseek (fd, 0, SEEK_END);
            u32FileSize = ftell(fd);
            u32FileNumFrames = u32FileSize / (u32YSz + u32UVSz);

            if(pool->u32RdIdx >= u32FileNumFrames)
                pool->u32RdIdx = 0;

            u32Ret = fseek(fd, pool->u32RdIdx * (u32YSz + u32UVSz), SEEK_SET);
            if(u32Ret)
            {
                LOGE("Error: File seek failed. Might be end of file.");
                u32 = VPP_ERR;
                goto err_read_file;
            }
        }

        if(pool->params.u32Width == u32Stride)
        {
            u32Ret = fread(virt, sizeof(char), u32YSz, fd);
            if(u32Ret != u32YSz)
            {
                LOGE("Error: File read Y failed. Might be end of file.");
                u32 = VPP_ERR;
                goto err_read_file;
            }

            virt = virt + u32VppUtils_GetUVOffset(&pool->stInputPort);

            u32Ret = fread(virt, sizeof(char), u32UVSz, fd);
            if(u32Ret != u32UVSz)
            {
                LOGE("Error: File read UV failed. Might be end of file.");
                u32 = VPP_ERR;
                goto err_read_file;
            }
        }
        else // Stride align the buffer
        {
            LOGI("Aligning stride on file read to %d\n", u32Stride);
            for(i = 0; i < pool->params.u32Height; i++)
            {
                u32Ret = fread(virt, sizeof(char), pool->params.u32Width, fd);
                if(u32Ret != pool->params.u32Width)
                {
                    LOGE("Error: File read Y failed. Might be end of file.");
                    u32 = VPP_ERR;
                    goto err_read_file;
                }
                virt += u32Stride;
            }
            virt = base + u32VppUtils_GetUVOffset(&pool->stInputPort);
            for(i = 0; i < (pool->params.u32Height)>>1; i++)
            {
                u32Ret = fread(virt, sizeof(char), pool->params.u32Width, fd);
                if(u32Ret != pool->params.u32Width)
                {
                    LOGE("Error: File read UV failed. Might be end of file.");
                    u32 = VPP_ERR;
                    goto err_read_file;
                }
                virt += u32Stride;
            }
        }
    }

err_read_file:
    fclose(fd);

protected_buffer:
err_open_file:
#endif
    node->pIntBuf->pBuf->pixel.filled_len =
        u32VppUtils_GetPxBufferSize(&pool->stInputPort);

    node->pIntBuf->stPixel.u32FilledLen =
        u32VppUtils_GetPxBufferSize(&pool->stInputPort);

    pool->u32RdIdx += 1;
    if ((pool->u32RdIdx >= pool->params.u32MaxInputFrames) && (pool->params.u32MaxInputFrames != 0))
        pool->u32RdIdx = 0;

    return u32;
}

uint32_t fill_mbi_buf(struct bufnode *node)
{
    uint32_t u32 = VPP_OK;
    FILE *fd1, *fd2;
    char cName1[FN_BUF_SZ];
    char cName2[FN_BUF_SZ];
    struct buf_pool *pool = node->pPool;
    uint32_t u32MbiLen, u32FileSize, u32FileNumFrames, u32Ret;

    u32MbiLen = u32VppUtils_GetMbiSize(pool->params.u32Width, pool->params.u32Height);

#ifdef FILL_BUF
    strlcpy(cName1, pool->params.cInputPath, FN_BUF_SZ);
    strlcpy(cName2, pool->params.cInputPath, FN_BUF_SZ);
    strlcat(cName1, pool->params.cInputNameMbi1, FN_BUF_SZ);
    strlcat(cName2, pool->params.cInputNameMbi2, FN_BUF_SZ);

    fd1 = fopen(cName1, "rb");
    if (!fd1 )
    {
        LOGE("Error: unable to open file %s", cName1);
        u32 = VPP_ERR;
        goto err_open_file1;
    }

    fd2 = fopen(cName2, "rb");
    if (!fd2)
    {
        LOGE("Error: unable to open file %s", cName2);
        u32 = VPP_ERR;
        goto err_open_file2;
    }

    t_StVppBuf *pstBufSrc = node->pIntBuf;
    struct vpp_mem_buffer *pstMemBufSrc = &pstBufSrc->pBuf->extradata;
    struct msm_vidc_extradata_header *pExDataSrc;
    pExDataSrc = (struct msm_vidc_extradata_header *)pstBufSrc->stExtra.pvBase;
    pExDataSrc->type = MSM_VIDC_EXTRADATA_METADATA_MBI;
    pExDataSrc->data_size = u32MbiLen*2 + sizeof(struct msm_vidc_extradata_header);
    pstMemBufSrc->filled_len = u32MbiLen*2;
    pstBufSrc->stExtra.u32FilledLen = pstMemBufSrc->filled_len;

    char *base = (char*)pExDataSrc->data;

    LOGI("reading: %s --> node->pIntBuf->stExtra.pvBase=%p", cName1, base);

    fseek (fd1, 0, SEEK_END);
    u32FileSize = ftell(fd1);
    u32FileNumFrames = u32FileSize / u32MbiLen;

    if(pool->u32RdIdx >= u32FileNumFrames)
        pool->u32RdIdx = 0;

    u32Ret = fseek(fd1, pool->u32RdIdx * u32MbiLen, SEEK_SET);
    if(u32Ret)
    {
        LOGE("Error: File seek failed for fd1. Might be end of file.");
        u32 = VPP_ERR;
            goto err_read_file;
    }
    u32Ret = fseek(fd2, pool->u32RdIdx * u32MbiLen, SEEK_SET);
    if(u32Ret)
    {
        LOGE("Error: File seek failed for fd2. Might be end of file.");
        u32 = VPP_ERR;
            goto err_read_file;
    }

    u32Ret = fread(base, sizeof(char), u32MbiLen, fd1);
    if(u32Ret != u32MbiLen)
    {
        LOGE("Error: MBI File read fd1 failed. Might be end of file.");
        u32 = VPP_ERR;
        goto err_read_file;
    }

    u32Ret = fread(base + u32MbiLen, sizeof(char), u32MbiLen, fd2);
    if(u32Ret != u32MbiLen)
    {
        LOGE("Error: MBI File read fd2 failed. Might be end of file.");
        u32 = VPP_ERR;
        goto err_read_file;
    }

err_read_file:
    fclose(fd2);
err_open_file2:
    fclose(fd1);
err_open_file1:
#endif

    pool->u32RdIdx += 1;
    if ((pool->u32RdIdx >= pool->params.u32MaxInputFrames) && (pool->params.u32MaxInputFrames != 0))
        pool->u32RdIdx = 0;

    return u32;
}

uint32_t dump_buf(struct bufnode *node)
{
#ifdef DUMP_BUF
    FILE *fd=NULL;
    FILE *fd_mbi=NULL;
    char cName[FN_BUF_SZ];
    char cName_mbi[256];
    uint32_t u32Stride, i, u32;
#endif
    uint32_t u32Ret = VPP_OK;

    if (!node)
    {
        LOGE("Error: trying to dump null node");
        return VPP_ERR;
    }
    struct buf_pool *pool = node->pPool;

    if (!pool)
    {
        LOGE("received bufnode with no pool");
        return VPP_ERR;
    }


    if (pool->params.eProtection == PROTECTION_ZONE_SECURE)
    {
        LOGI("Received secure buffer... not dumping buffer");
        return VPP_OK;
    }

#ifdef DUMP_BUF
    if(pool->params.eOutputFileType != FILE_TYPE_NONE)
    {

        // Currently only supporting NV12 to NV12.
        if ((pool->params.eOutputFileFormat != FILE_FORMAT_NV12) ||
            (pool->params.eOutputBufFmt != VPP_COLOR_FORMAT_NV12_VENUS))
        {
            LOGE("Error: Invalid colour formats for write!");
        }

        // Make sure that output folder exists before trying to open the output
        // file.
        mkdir(pool->params.cOutputPath, S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH);


        if(pool->params.eOutputFileType == FILE_TYPE_SINGLE_FRAMES)
        {
            uint32_t u32Tmp;
            u32Tmp = get_full_path(cName, FN_BUF_SZ, pool->params.cOutputPath,
                                   pool->params.cOutputName, pool->u32WrIdx);
            if (u32Tmp != VPP_OK)
            {
                LOGE("Error: unable to get file name for multi frame output file");
                return VPP_ERR;
            }
            fd = fopen(cName, "wb");
            if (node->pIntBuf->eBufPxType==eVppBufPxDataType_Compressed)
            {
                strlcpy(cName_mbi, cName, FN_BUF_SZ);
                strlcat(cName_mbi, "_mbi", FN_BUF_SZ);
                fd_mbi = fopen(cName_mbi, "wb");
            }
        }
        else
        {
            strlcpy(cName, pool->params.cOutputPath, FN_BUF_SZ);
            strlcat(cName, pool->params.cOutputName, FN_BUF_SZ);
            if(pool->u32WrIdx == 0)
                fd = fopen(cName, "wb");
            else
                fd = fopen(cName, "ab");

            if (node->pIntBuf->eBufPxType==eVppBufPxDataType_Compressed)
            {
                strlcpy(cName_mbi, cName, FN_BUF_SZ);
                strlcat(cName_mbi, "_mbi", FN_BUF_SZ);
                if(pool->u32WrIdx == 0)
                    fd_mbi = fopen(cName_mbi, "wb");
                else
                    fd_mbi = fopen(cName_mbi, "ab");
            }
        }

        if (!fd)
        {
            LOGE("Error: unable to open file: %s", cName);
            return VPP_ERR;
        }

        if ((node->pIntBuf->eBufPxType==eVppBufPxDataType_Compressed) && (!fd_mbi))
        {
            LOGE("Error: unable to open MBI file: %s", cName_mbi);
            return VPP_ERR;
        }

        switch (node->pIntBuf->eBufPxType)
        {
            case eVppBufPxDataType_Raw:
            {
                char *base = node->pIntBuf->stPixel.pvBase;
                char *virt = node->pIntBuf->stPixel.pvBase;
                u32Stride = u32VppUtils_GetStride(&pool->stInputPort);

                LOGI("writing YUV frame: %s --> base=%p, virt=%p", cName, base, virt);

                if (pool->params.eInputBufFmt == VPP_COLOR_FORMAT_NV12_VENUS &&
                    pool->params.eInputFileFormat == FILE_FORMAT_NV12)
                {
                    uint32_t u32YSz;
                    uint32_t u32UVSz;

                    u32YSz = pool->params.u32Width * pool->params.u32Height;
                    u32UVSz = u32YSz / 2;

                    if(pool->params.u32Width == u32Stride)
                    {
                        u32 = fwrite(virt, sizeof(char), u32YSz, fd);
                        if(u32 != u32YSz)
                        {
                            LOGE("Error: File write Y failed.");
                            u32Ret = VPP_ERR;
                            goto err_write_file;
                        }

                        // Venus has alignment restrictions
                        virt = virt + u32VppUtils_GetUVOffset(&pool->stInputPort);
                        u32 = fwrite(virt, sizeof(char), u32UVSz, fd);
                        if(u32 != u32UVSz)
                        {
                            LOGE("Error: File write UV failed.");
                            u32Ret = VPP_ERR;
                            goto err_write_file;
                        }
                    }
                    else
#ifdef DUMP_PADDED
                    {
                        LOGI("Writing stride padded output \n");
                        for(i = 0; i < pool->params.u32Height; i++)
                        {
                            u32 = fwrite(virt, sizeof(char), u32Stride, fd);
                            if(u32 != u32Stride)
                            {
                                LOGE("Error: File write Y stride line failed.");
                                u32Ret = VPP_ERR;
                                goto err_write_file;
                            }
                            virt += u32Stride;
                        }
                        virt = base + u32VppUtils_GetUVOffset(&pool->stInputPort);
                        for(i = 0; i < (pool->params.u32Height)>>1; i++)
                        {
                            u32 = fwrite(virt, sizeof(char), u32Stride, fd);
                            if(u32 != u32Stride)
                            {
                                LOGE("Error: File write UV stride line failed.");
                                u32Ret = VPP_ERR;
                                goto err_write_file;
                            }
                            virt += u32Stride;
                        }
                    }
#else
                    {
                        LOGI("Removing stride alignment of %d on file write\n", u32Stride);
                        for(i = 0; i < pool->params.u32Height; i++)
                        {
                            u32 = fwrite(virt, sizeof(char), pool->params.u32Width, fd);
                            if(u32 != pool->params.u32Width)
                            {
                                LOGE("Error: File write Y stride line failed.");
                                u32Ret = VPP_ERR;
                                goto err_write_file;
                            }
                            virt += u32Stride;
                        }
                        virt = base + u32VppUtils_GetUVOffset(&pool->stInputPort);
                        for(i = 0; i < (pool->params.u32Height)>>1; i++)
                        {
                            u32 = fwrite(virt, sizeof(char), pool->params.u32Width, fd);
                            if(u32 != pool->params.u32Width)
                            {
                                LOGE("Error: File write UV stride line failed.");
                                u32Ret = VPP_ERR;
                                goto err_write_file;
                            }
                            virt += u32Stride;
                        }
                    }
#endif
                }
                break;
            }
            case eVppBufPxDataType_Compressed:
            {
                //Write comressed data
                char *base = node->pIntBuf->stPixel.pvBase;
                char *virt = node->pIntBuf->stPixel.pvBase;
                uint32_t buf_size = node->pIntBuf->stPixel.u32FilledLen;

                if (base==NULL)
                {
                   LOGE("Error: Error: pvBase is NULL!");
                   u32Ret = VPP_ERR;
                   goto err_write_file;
                }

                LOGI("writing compressed frame: %s --> base=%p, virt=%p, size=%d\n",
                     cName, base, virt, buf_size);
                u32 = fwrite(virt, sizeof(char), buf_size, fd);
                if(u32 != buf_size)
                {
                   LOGE("Error: File write compressed frame failed.");
                   u32Ret = VPP_ERR;
                }

                //Write MBI
                base = node->pIntBuf->stExtra.pvBase;
                virt = node->pIntBuf->stExtra.pvBase;
                buf_size = node->pIntBuf->stExtra.u32FilledLen;

                LOGI("writing MBI frame: %s --> base=%p, virt=%p, size=%d\n",
                     cName_mbi, base, virt, buf_size);
                u32 = fwrite(virt, sizeof(char), buf_size, fd_mbi);
                if(u32 != buf_size)
                {
                   LOGE("Error: File write MBI frame failed.");
                   u32Ret = VPP_ERR;
                }
                break;
            }
            default:
                LOGE("Error: Unsupported eBufPxType=%d\n",node->pIntBuf->eBufPxType);
        }

err_write_file:

        fclose(fd);
        if (node->pIntBuf->eBufPxType==eVppBufPxDataType_Compressed)
        {
            fclose(fd_mbi);
        }
    }
#endif

   pool->u32WrIdx += 1;

   return u32Ret;
}

uint32_t validate_equal(char *pSrc, char *pDst, uint32_t u32Sz)
{
    uint32_t bEq = VPP_TRUE;
    uint32_t i;

    if (!pSrc || !pDst)
        return VPP_FALSE;

    for (i = 0; i < u32Sz; i++)
    {
        if (pSrc[i] != pDst[i])
        {
            bEq = VPP_FALSE;
        }

        if (bEq != VPP_TRUE)
            LOGE("pSrc[%u]=%u, pDst[%u]=%u", i, pSrc[i], i, pDst[i]);
    }

    return bEq;
}

#ifndef OMX_EXTRADATA_DOES_NOT_EXIST

void validate_extradata_integrity_int_buf_omx(t_StVppBuf *pstIntBuf)
{
    OMX_QCOM_PANSCAN *pWin;
    OMX_QCOM_FRAME_PACKINGFORMAT_TYPE *pFp;

    pFp = (OMX_QCOM_FRAME_PACKINGFORMAT_TYPE *)
        vpVppBuf_FindExtradata(pstIntBuf, OMX_ExtraDataFramePackingArrangement,
                               EXTRADATA_BUFFER_FORMAT_OMX);
    DVP_ASSERT_PTR_NNULL(pFp);
    if (pFp)
        DVP_ASSERT_TRUE(validate_equal((char *)&stFramePackingRefOmx, (char *)pFp,
                        sizeof(OMX_QCOM_FRAME_PACKINGFORMAT_TYPE)));

    pWin = (OMX_QCOM_PANSCAN *)
        vpVppBuf_FindExtradata(pstIntBuf, OMX_ExtraDataFrameDimension,
                               EXTRADATA_BUFFER_FORMAT_OMX);
    DVP_ASSERT_PTR_NNULL(pWin);
    if (pWin)
        DVP_ASSERT_TRUE(validate_equal((char *)&stPanScanWindowOmx, (char *)pWin,
                        sizeof(OMX_QCOM_PANSCAN)));
}

uint32_t write_extradata_buf_omx(void *pv, uint32_t u32Ft, uint32_t u32Marker,
                                 uint32_t u32Sz)
{
    uint32_t u32FillLen = 0;
    OMX_OTHER_EXTRADATATYPE *pHdr;
    OMX_STREAMINTERLACEFORMAT *pl;
    OMX_VPP_TEST_BUFMARKER *pm;

    if (!pv)
        return 0;

    if (u32Sz < MINIMUM_EXTRADATA_SIZE_OMX)
        return 0;

    pHdr = (OMX_OTHER_EXTRADATATYPE *)pv;

    // if (bFramePackingPayload)
    {
        pHdr->nSize = sizeof(OMX_OTHER_EXTRADATATYPE) +
            sizeof(OMX_QCOM_FRAME_PACKINGFORMAT_TYPE);

        pHdr->eType = (OMX_EXTRADATATYPE)OMX_ExtraDataFramePackingArrangement;
        pHdr->nDataSize = sizeof(OMX_QCOM_FRAME_PACKINGFORMAT_TYPE);

        memcpy(pHdr->data, &stFramePackingRefOmx,
               sizeof(OMX_QCOM_FRAME_PACKINGFORMAT_TYPE));

        u32FillLen += pHdr->nSize;

        pHdr = (OMX_OTHER_EXTRADATATYPE *)((char *)pHdr + pHdr->nSize);
    }

    // if (bPanscanWindow)
    {
        pHdr->nSize = sizeof(OMX_OTHER_EXTRADATATYPE) +
            sizeof(OMX_QCOM_PANSCAN);

        pHdr->eType = (OMX_EXTRADATATYPE)OMX_ExtraDataFrameDimension;
        pHdr->nDataSize = sizeof(OMX_QCOM_PANSCAN);
        memcpy(pHdr->data, &stPanScanWindowOmx,
               sizeof(OMX_QCOM_PANSCAN));

        u32FillLen += pHdr->nSize;

        pHdr = (OMX_OTHER_EXTRADATATYPE *)((char *)pHdr + pHdr->nSize);
    }

    // Add buffer marker
    {
        pHdr->nSize = sizeof(OMX_OTHER_EXTRADATATYPE) +
            sizeof(OMX_VPP_TEST_BUFMARKER);

        pHdr->eType = (OMX_EXTRADATATYPE)OMX_ExtraDataBufMarker;
        pHdr->nDataSize = sizeof(OMX_VPP_TEST_BUFMARKER);
        pm = (OMX_VPP_TEST_BUFMARKER *)pHdr->data;
        pm->nMarker = u32Marker;

        u32FillLen += pHdr->nSize;

        pHdr = (OMX_OTHER_EXTRADATATYPE *)((char *)pHdr + pHdr->nSize);
    }

    pHdr->nSize = sizeof(OMX_OTHER_EXTRADATATYPE) +
        sizeof(OMX_STREAMINTERLACEFORMAT);
    pHdr->eType = (OMX_EXTRADATATYPE)OMX_ExtraDataInterlaceFormat;
    pHdr->nDataSize = sizeof(OMX_STREAMINTERLACEFORMAT);
    pl = (OMX_STREAMINTERLACEFORMAT *)pHdr->data;

    switch (u32Ft)
    {
        case eVppBufType_Interleaved_TFF:
            pl->nInterlaceFormats = OMX_InterlaceInterleaveFrameTopFieldFirst;
            break;
        case eVppBufType_Interleaved_BFF:
            pl->nInterlaceFormats = OMX_InterlaceInterleaveFrameBottomFieldFirst;
            break;
        case eVppBufType_Frame_TFF:
            pl->nInterlaceFormats = OMX_InterlaceFrameTopFieldFirst;
            break;
        case eVppBufType_Frame_BFF:
            pl->nInterlaceFormats = OMX_InterlaceFrameBottomFieldFirst;
            break;
        case eVppBufType_Progressive:
        case eVppBufType_Max:
            pl->nInterlaceFormats = OMX_InterlaceFrameProgressive;
            break;
    }
    u32FillLen += pHdr->nSize;

    return u32FillLen;
}

uint32_t fill_extra_data_int_buf_omx(t_StVppBuf *pstIntBuf, uint32_t u32Ft,
                                     uint32_t u32Marker)
{
    uint32_t u32AllocLen, u32FillLen = 0;
    uint32_t u32Valid;

    u32AllocLen = pstIntBuf->stExtra.u32AllocLen;
    u32Valid = pstIntBuf->stExtra.u32ValidLen;

    if (u32AllocLen < pstIntBuf->stExtra.u32ValidLen)
    {
        LOGE("ERROR: u32AllocLen < u32ValidLen, not filling...");
        return VPP_ERR;
    }

    if (u32Valid < MINIMUM_EXTRADATA_SIZE_OMX)
    {
        LOGE("ERROR: insufficient size remaining to write extra data to, req=%u, valid=%u",
             (unsigned)MINIMUM_EXTRADATA_SIZE_OMX, u32Valid);
        return VPP_ERR;
    }

    void *pvDst = (char *)pstIntBuf->stExtra.pvBase;
    u32FillLen = write_extradata_buf_omx(pvDst, u32Ft, u32Marker, u32Valid);

    LOGD("setting frame_type=%u, filledLen=%u", u32Ft, u32FillLen);
    pstIntBuf->pBuf->extradata.filled_len = u32FillLen;
    pstIntBuf->stExtra.u32FilledLen = u32FillLen;

    return VPP_OK;
}
#endif


void validate_extradata_integrity_int_buf_msm(t_StVppBuf *pstIntBuf)
{
    struct msm_vidc_panscan_window *pWin;
    struct msm_vidc_s3d_frame_packing_payload *pFp;

    pFp = (struct msm_vidc_s3d_frame_packing_payload *)
        vpVppBuf_FindExtradata(pstIntBuf, MSM_VIDC_EXTRADATA_S3D_FRAME_PACKING,
                               EXTRADATA_BUFFER_FORMAT_MSM);
    DVP_ASSERT_PTR_NNULL(pFp);
    if (pFp)
        DVP_ASSERT_TRUE(validate_equal((char *)&stFramePackingRefMsm, (char *)pFp,
                        sizeof(struct msm_vidc_s3d_frame_packing_payload)));

    pWin = (struct msm_vidc_panscan_window *)
        vpVppBuf_FindExtradata(pstIntBuf, MSM_VIDC_EXTRADATA_PANSCAN_WINDOW,
                               EXTRADATA_BUFFER_FORMAT_MSM);
    DVP_ASSERT_PTR_NNULL(pWin);
    if (pWin)
        DVP_ASSERT_TRUE(validate_equal((char *)&stPanScanWindowMsm, (char *)pWin,
                        sizeof(struct msm_vidc_panscan_window)));
}

// returns the number of bytes written
uint32_t write_extradata_buf_msm(void *pv, uint32_t u32Ft, uint32_t u32Marker,
                                 uint32_t u32Sz)
{
    uint32_t u32FillLen = 0;
    struct msm_vidc_extradata_header *pHdr;
    struct msm_vidc_interlace_payload *pl;
    struct msm_vpp_test_bufmarker *pm;

    if (!pv)
        return 0;

    if (u32Sz < MINIMUM_EXTRADATA_SIZE_MSM)
        return 0;

    pHdr = (struct msm_vidc_extradata_header *)pv;

    // if (bFramePackingPayload)
    {
        pHdr->size = sizeof(struct msm_vidc_extradata_header) +
            sizeof(struct msm_vidc_s3d_frame_packing_payload);

        pHdr->type = MSM_VIDC_EXTRADATA_S3D_FRAME_PACKING;
        pHdr->data_size = sizeof(struct msm_vidc_s3d_frame_packing_payload);

        memcpy(pHdr->data, &stFramePackingRefMsm,
               sizeof(struct msm_vidc_s3d_frame_packing_payload));

        u32FillLen += pHdr->size;

        pHdr = (struct msm_vidc_extradata_header *)((char *)pHdr + pHdr->size);
    }

    // if (bPanscanWindow)
    {
        pHdr->size = sizeof(struct msm_vidc_extradata_header) +
            sizeof(struct msm_vidc_panscan_window);

        pHdr->type = MSM_VIDC_EXTRADATA_PANSCAN_WINDOW;
        pHdr->data_size = sizeof(struct msm_vidc_panscan_window);
        memcpy(pHdr->data, &stPanScanWindowMsm,
               sizeof(struct msm_vidc_panscan_window));

        u32FillLen += pHdr->size;

        pHdr = (struct msm_vidc_extradata_header *)((char *)pHdr + pHdr->size);
    }

    {
        pHdr->size = sizeof(struct msm_vidc_extradata_header) +
            sizeof(struct msm_vpp_test_bufmarker);

        pHdr->type = MSM_VPP_TEST_EXTRADATA_BUFMARKER;

        pHdr->data_size = sizeof(struct msm_vpp_test_bufmarker);
        pm = (struct msm_vpp_test_bufmarker *)pHdr->data;
        pm->nMarker = u32Marker;

        u32FillLen += pHdr->size;

        pHdr = (struct msm_vidc_extradata_header *)((char *)pHdr + pHdr->size);
    }

    pHdr->size = sizeof(struct msm_vidc_extradata_header) +
        sizeof(struct msm_vidc_interlace_payload);
    pHdr->type = MSM_VIDC_EXTRADATA_INTERLACE_VIDEO;
    pHdr->data_size = sizeof(struct msm_vidc_interlace_payload);
    pl = (struct msm_vidc_interlace_payload *)pHdr->data;

    switch (u32Ft)
    {
        case eVppBufType_Interleaved_TFF:
            pl->format = MSM_VIDC_INTERLACE_INTERLEAVE_FRAME_TOPFIELDFIRST;
            break;
        case eVppBufType_Interleaved_BFF:
            pl->format = MSM_VIDC_INTERLACE_INTERLEAVE_FRAME_BOTTOMFIELDFIRST;
            break;
        case eVppBufType_Frame_TFF:
            pl->format = MSM_VIDC_INTERLACE_FRAME_TOPFIELDFIRST;
            break;
        case eVppBufType_Frame_BFF:
            pl->format = MSM_VIDC_INTERLACE_FRAME_BOTTOMFIELDFIRST;
            break;
        case eVppBufType_Progressive:
        case eVppBufType_Max:
            pl->format = MSM_VIDC_INTERLACE_FRAME_PROGRESSIVE;
            break;
    }

    u32FillLen += pHdr->size;

    return u32FillLen;
}

uint32_t fill_extra_data_int_buf_msm(t_StVppBuf *pstIntBuf, uint32_t u32Ft,
                                     uint32_t u32Marker)
{
    uint32_t u32AllocLen, u32FillLen = 0;
    uint32_t u32Valid;

    u32AllocLen = pstIntBuf->stExtra.u32AllocLen;
    u32Valid = pstIntBuf->stExtra.u32ValidLen;

    if (u32AllocLen < pstIntBuf->stExtra.u32ValidLen)
    {
        LOGE("ERROR: u32AllocLen < u32ValidLen, not filling...");
        return VPP_ERR;
    }

    if (u32Valid < MINIMUM_EXTRADATA_SIZE_MSM)
    {
        LOGE("ERROR: insufficient size remaining to write extra data to, req=%u, valid=%u",
             (unsigned)MINIMUM_EXTRADATA_SIZE_MSM, u32Valid);
        DVP_ASSERT_FAIL();
        return VPP_ERR;
    }

    void *pvDst = (char *)pstIntBuf->stExtra.pvBase;
    u32FillLen = write_extradata_buf_msm(pvDst, u32Ft, u32Marker, u32Valid);

    LOGD("setting frame_type=%u, filledLen=%u", u32Ft, u32FillLen);
    pstIntBuf->pBuf->extradata.filled_len = u32FillLen;
    pstIntBuf->stExtra.u32FilledLen = u32FillLen;

    return VPP_OK;

}

void validate_extradata_integrity_int_buf(t_StVppBuf *pstIntBuf)
{
    t_EExtradataBufferFormat eFormat = VPP_EXTERNAL_EXTRADATA_TYPE;

    if(eFormat == EXTRADATA_BUFFER_FORMAT_MSM)
        validate_extradata_integrity_int_buf_msm(pstIntBuf);
#ifndef OMX_EXTRADATA_DOES_NOT_EXIST
    else if(eFormat == EXTRADATA_BUFFER_FORMAT_OMX)
        validate_extradata_integrity_int_buf_omx(pstIntBuf);
#endif
    else
        DVP_ASSERT_FAIL();
}

void validate_extradata_integrity(struct bufnode *pNode)
{
    if (!pNode)
    {
        DVP_ASSERT_FAIL();
        return;
    }

    validate_extradata_integrity_int_buf(pNode->pIntBuf);
}

uint32_t fill_extra_data_int_buf(t_StVppBuf *pstIntBuf, uint32_t u32Ft,
                                 uint32_t u32Marker)
{
    t_EExtradataBufferFormat eFormat = VPP_EXTERNAL_EXTRADATA_TYPE;

    if(eFormat == EXTRADATA_BUFFER_FORMAT_MSM)
        return fill_extra_data_int_buf_msm(pstIntBuf, u32Ft, u32Marker);
#ifndef OMX_EXTRADATA_DOES_NOT_EXIST
    else if(eFormat == EXTRADATA_BUFFER_FORMAT_OMX)
        return fill_extra_data_int_buf_omx(pstIntBuf, u32Ft, u32Marker);
#endif
    else
        return VPP_ERR;
}


uint32_t fill_extra_data(struct bufnode *node, uint32_t u32Ft,
                         uint32_t u32Marker)
{
    if (!node)
        return VPP_ERR;

    return fill_extra_data_int_buf(node->pIntBuf, u32Ft, u32Marker);
}

uint32_t get_extra_data_marker_int_buf(t_StVppBuf *pstIntBuf,
                                       uint32_t* pu32Marker)
{
    t_EExtradataBufferFormat eFormat = VPP_EXTERNAL_EXTRADATA_TYPE;
    struct msm_vpp_test_bufmarker *pm;
    OMX_VPP_TEST_BUFMARKER *pl;
    if(eFormat == EXTRADATA_BUFFER_FORMAT_MSM)
    {
        pm = (struct msm_vpp_test_bufmarker*) vpVppBuf_FindExtradata(pstIntBuf,
               MSM_VPP_TEST_EXTRADATA_BUFMARKER, VPP_EXTERNAL_EXTRADATA_TYPE);
        if (pm == NULL)
            return VPP_ERR;
        else
        {
            *pu32Marker = pm->nMarker;
            return VPP_OK;
        }
    }
#ifndef OMX_EXTRADATA_DOES_NOT_EXIST
    else if(eFormat == EXTRADATA_BUFFER_FORMAT_OMX)
    {
        pl = (OMX_VPP_TEST_BUFMARKER*) vpVppBuf_FindExtradata(pstIntBuf,
               OMX_ExtraDataBufMarker, VPP_EXTERNAL_EXTRADATA_TYPE);
        if (pl == NULL)
            return VPP_ERR;
        else
        {
            *pu32Marker = pl->nMarker;
            return VPP_OK;
        }
    }
#endif
    else
        return VPP_ERR;
}

uint32_t write_extradata_buf(void *pv, uint32_t u32Ft, uint32_t u32Marker,
                             uint32_t u32AvailSz)
{
    t_EExtradataBufferFormat eFormat = VPP_EXTERNAL_EXTRADATA_TYPE;

    if(eFormat == EXTRADATA_BUFFER_FORMAT_MSM)
        return write_extradata_buf_msm(pv, u32Ft, u32Marker, u32AvailSz);
#ifndef OMX_EXTRADATA_DOES_NOT_EXIST
    else if(eFormat == EXTRADATA_BUFFER_FORMAT_OMX)
        return write_extradata_buf_omx(pv, u32Ft, u32Marker, u32AvailSz);
#endif
    else
        return 0;
}

uint32_t get_missing_node_count(struct buf_pool *pool)
{
    if (!pool)
        return 0;

    return pool->u32Cnt - pool->u32ListSz;
}

void vVppBuf_Clean(t_StVppBuf *pstBuf, char fill_value)
{
    if (pstBuf==NULL)
        return;

    if (pstBuf->stPixel.pvBase==NULL)
        return;

    char *base = pstBuf->stPixel.pvBase;
    uint32_t size = pstBuf->stPixel.u32ValidLen;
    memset((void*)base, fill_value, size);
}

void vVppBuf_CleanExtradata(t_StVppBuf *pstBuf, char fill_value)
{
    LOGI("%s pstBuf=%p, fill=%d\n", __func__, pstBuf, fill_value);
    if (pstBuf==NULL)
        return;

    if (pstBuf->stExtra.pvBase==NULL)
        return;

    char *base = pstBuf->stExtra.pvBase;
    uint32_t size = pstBuf->stExtra.u32ValidLen;
    memset((void*)base, fill_value, size);
}
