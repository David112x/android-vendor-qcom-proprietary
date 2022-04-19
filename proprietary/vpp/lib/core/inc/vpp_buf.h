/*!
 * @file vpp_buf.h
 *
 * @cr
 * Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

 * @services
 */
#ifndef _VPP_BUF_H_
#define _VPP_BUF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include "vpp_def.h"
#include "vpp.h"
#include "vpp_ctx.h"
#include "vpp_stats.h"

#ifndef VPP_INTERNAL_BUF_MAX
#define VPP_INTERNAL_BUF_MAX 128
#endif


#define VPP_BUF_UNMAPPED_BUF_VAL    NULL

struct StVppUsecase;

typedef enum {
    eVppBufFlag_Flushed,
    eVppBufFlag_Bypass,
    eVppBufFlag_Dump,
    /*!
     * Flags between positions 24 and 31 are owned by IP blocks and are only
     * valid while the buffer is owned by the IP block. These flags' meanings
     * are only valid while set and unset by the IP block.
     */
    eVppBufFlag_IpStart = 24,
    eVppBufFlag_IpMax = 31,
} t_EVppBufferFlags;

typedef enum {
    /*!
     * Flags between positions 24 and 31 are owned by IP blocks and are only
     * valid while the buffer is owned by the IP block. These flags' meanings
     * are only valid while set and unset by the IP block.
     */
    eVppIntBufFlag_IpStart = 24,
    eVppIntBufFlag_IpMax = 31,
} t_EVppIntBufferFlags;

#define VPP_BUF_FLAG_FLUSHED        (1 << eVppBufFlag_Flushed)
#define VPP_BUF_FLAG_BYPASS         (1 << eVppBufFlag_Bypass)
#define VPP_BUF_FLAG_DUMP           (1 << eVppBufFlag_Dump)
#define VPP_BUF_FLAG_IP_START       (1 << eVppBufFlag_IpStart)
#define VPP_BUF_FLAG_IP_MAX         (1 << eVppBufFlag_IpMax)

typedef enum {
    eVppLogId_IpQueueBuf,
    eVppLogId_IpBufDone,
    eVppLogId_IpProcStart,
    eVppLogId_IpProcDone,
    eVppLogId_IpMax,
} t_EVppBufIpLogId;

typedef enum {
    eVppLogId_InternalGet,      // Top level queue buf, after InternalGet but before MapBuf
    eVppLogId_InternalPut,      // Top level IBD/OBD, after InternalPut, but before UnmapBuf
    eVppLogId_MapExtraStart,    // Start to call of MapBuf, for extradata
    eVppLogId_MapExtraDone,     // After call to MapBuf completed, for extradata
    eVppLogId_MapPixelStart,    // Start to call of MapBuf, for pixel
    eVppLogId_MapPixelDone,     // After call to UnmapBuf, for pixel
    eVppLogId_UnmapExtraStart,  // Start to call of UnmapBuf, for extradata
    eVppLogId_UnmapExtraDone,   // After call to UnmapBuf completed, for extradata
    eVppLogId_UnmapPixelStart,  // Start to call of UnmapBuf, for pixel
    eVppLogId_UnmapPixelDone,   // After call to UnmapBuf, for pixel
    eVppLogId_PlQueueBuf,       // Pipeline received queue buf
    eVppLogId_PlBufDone,        // Pipeline received IBD/OBD
    eVppLogId_IpStart,          // Start of IP block specific logs
    eVppLogId_Max = eVppLogId_IpStart + (eVppLogId_IpMax * VPP_IP_MAX),
} t_EVppBufLogId;

#define VPP_LOG_BUF(ctx, buf, id) vVppBuf_Log(ctx, buf, id)

typedef enum {
    eVppBufPxDataType_Raw,
    eVppBufPxDataType_Compressed,
    eVppBufPxDataType_Max,
} t_EVppBufPxDataType;

typedef enum {
    eVppBufType_Progressive,
    eVppBufType_Interleaved_TFF,
    eVppBufType_Interleaved_BFF,
    eVppBufType_Frame_TFF,
    eVppBufType_Frame_BFF,
    eVppBufType_Max,
} t_EVppBufType;

typedef enum {
    eVppBuf_Unmapped,
    eVppBuf_MappedInternal,
    eVppBuf_MappedExternal,
} t_EVppBufMapping;

typedef struct {
    t_EVppBufMapping eMapping;

    int fd;                         // TranslateBuf
    uint32_t u32AllocLen;           // TranslateBuf
    uint32_t u32FilledLen;          // TranslateBuf
    uint32_t u32Offset;             // TranslateBuf
    uint32_t u32ValidLen;           // TranslateBuf

    // Desired offset
    void *pvBase;                   // MapIntBuf
                                    // pv at the start of valid memory. pv will point
                                    // to pvPa + (u32Offset + u32PaOffset)

    uint32_t u32MappedLen;          // MapIntBuf
                                    // Number of bytes that are mmap'd into userspace
                                    // starting from pv.

    struct {
        void *pvPa;                 // MapIntBuf
                                    // address to page aligned vaddr that was mmap'd

        uint32_t u32PaOffset;       // MapIntBuf
                                    // page aligned offset of u32Offset. This is what is
                                    // passed to mmap.

        uint32_t u32PaMappedLen;    // MapIntBuf
                                    // Number of bytes that were passed to mmap. This
                                    // is the number of bytes that were mapped into
                                    // userspace, starting from pvPa
    } priv;
    uint32_t u32IntBufFlags;        // Flags used for internal memory buffers.
                                    // For bit mappings refer to t_EVppIntBufferFlags enum.
} t_StVppMemBuf;

typedef enum {
    eVppMapType_Unmapped,
    eVppMapType_PxExSeparate,
    eVppMapType_PxExTogether,
} t_EVppMapType;

typedef struct StVppBuf {
    struct StVppBuf *pNext;
    struct vpp_buffer *pBuf;
    t_StVppMemBuf stPixel;
    t_StVppMemBuf stExtra;
    t_EVppMapType eMapType;

    uint32_t u32InternalFlags;
    uint32_t u32FrameRate;

    // Performance metrics
    struct timeval astLogTime[eVppLogId_Max];
    enum vpp_port eQueuedPort;
    enum vpp_port eReturnPort;
    uint32_t u32BufNum;
    uint32_t u32Idx;

    // File output
    char cBufPath[256];

    t_EVppBufType eBufType;
    t_EVppBufPxDataType eBufPxType;
} t_StVppBuf;

typedef struct {
    t_StVppBuf *pstHead;
    t_StVppBuf *pstTail;
    uint32_t u32Cnt;
} t_StVppBufPool;

typedef struct {
    uint32_t u32BufDumpEnable;
} t_StVppBufDump;

#ifdef VPP_CACHE_MMAPPED_PX_BUF
typedef struct {
    int fd;
    void *pv;
    uint32_t u32MapLen;
} t_StVppBufMapCacheNode;
#endif

typedef struct StVppBufCb {
    t_StVppBuf astBufNodes[VPP_INTERNAL_BUF_MAX];
    t_StVppBufPool stPool;
    pthread_mutex_t mutex;
    uint32_t bIsSecure;
    uint32_t u32LogMask;
    t_StVppBufDump stBufDump;
    char cBufDumpPath[128];
#ifdef VPP_CACHE_MMAPPED_PX_BUF
    t_StVppBufMapCacheNode astCacheNode[VPP_INTERNAL_BUF_MAX];
    uint32_t u32CacheCnt;
#endif

    uint32_t bRequiresMappedPxBuf;

    struct {
        // The values stored here should already be calculated to align to
        // extradata alignment requirements
        uint32_t u32InExOff;
        uint32_t u32OutExOff;
    };

    struct {
        STAT_DECL(InternalGet);
        STAT_DECL(InternalPut);
        STAT_DECL(InternalGetFail);
    } VPP_STAT_CB_NM;
} t_StVppBufCb;

typedef enum
{
    EXTRADATA_BUFFER_FORMAT_MSM,
#ifndef OMX_EXTRADATA_DOES_NOT_EXIST
    EXTRADATA_BUFFER_FORMAT_OMX,
#endif
    EXTRADATA_BUFFER_FORMAT_MAX,
} t_EExtradataBufferFormat;

#ifndef VPP_EXTERNAL_EXTRADATA_TYPE
    #define VPP_EXTERNAL_EXTRADATA_TYPE EXTRADATA_BUFFER_FORMAT_MSM
#endif

typedef enum {
    eVppBuf_Pixel,
    eVppBuf_Extra
} t_EVppTypeBuf;

/***************************************************************************
 * Function Prototypes
 ***************************************************************************/

uint32_t u32VppBufPool_Init(t_StVppBufPool *pstPool);
uint32_t u32VppBufPool_Term(t_StVppBufPool *pstPool);
t_StVppBuf *pstVppBufPool_Get(t_StVppBufPool *pstPool);
uint32_t u32VppBufPool_Put(t_StVppBufPool *pstPool, t_StVppBuf *pstBuf);
uint32_t u32VppBufPool_Cnt(t_StVppBufPool *pstPool);
t_StVppBuf *pstVppBufPool_Peek(t_StVppBufPool *pstPool, uint32_t index);

uint32_t u32VppBuf_Init(t_StVppCtx *pstCtx, uint32_t bIsSecure, uint32_t u32LogMask);
uint32_t u32VppBuf_Term(t_StVppCtx *pstCtx);
#ifdef VPP_CACHE_MMAPPED_PX_BUF
uint32_t u32VppBuf_FlushBufferCache(t_StVppCtx *pstCtx);
#endif
uint32_t u32VppBuf_SetCurrentUsecase(t_StVppCtx *pstCtx, struct StVppUsecase *pstUc);
uint32_t u32VppBuf_SetPortParams(t_StVppCtx *pstCtx,
                                 struct vpp_port_param stInput,
                                 struct vpp_port_param stOutput);
uint32_t u32VppBuf_InternalGet(t_StVppCtx *pstCtx,
                               struct vpp_buffer *pstExtBuf,
                               enum vpp_port ePort,
                               t_StVppBuf **o_ppstIntBuf);
uint32_t u32VppBuf_InternalPut(t_StVppCtx *pstCtx, t_StVppBuf *pstIntBuf, struct vpp_buffer **o_ppstExtBuf);

uint32_t u32VppBuf_IsEos(t_StVppBuf *pstVppBuf);
uint32_t u32VppBuf_IsEmptyEos(t_StVppBuf *pstVppBuf);

void vVppBuf_Log(t_StVppCtx *pstCtx, t_StVppBuf *pstBuf, uint32_t u32Id);
void vVppBuf_LogInfo(t_StVppCtx *pstCtx, t_StVppBuf *pstBuf);
void * vpVppBuf_FindExtradata(t_StVppBuf *pstBufSrc, uint32_t u32ExType,
                              t_EExtradataBufferFormat eExBufFormat);
void * vpVppBuf_FindExtradataHeader(t_StVppBuf *pstBufSrc, uint32_t u32ExType,
                                    t_EExtradataBufferFormat eExBufFormat);
uint32_t u32VppBuf_GetFrameTypeExtradata(t_StVppBuf *pstBuf,
                                         t_EExtradataBufferFormat eExBufFormat,
                                         t_EVppBufType *peBufType);

uint32_t u32VppBuf_CopyExtradata(t_StVppBuf *pstBufSrc, t_StVppBuf *pstBufDst,
                                 t_EVppBufType eDstBufType, t_EExtradataBufferFormat eExBufFormat);

uint32_t u32VppBuf_GrallocMetadataCopy(t_StVppBuf *pstBufSrc, t_StVppBuf *pstBufDst);
uint32_t u32VppBuf_GrallocFramerateMultiply(t_StVppBuf *pstBuf, uint32_t u32Factor);
uint32_t u32VppBuf_GrallocFrameTypeSet(t_StVppBuf *pstBuf, t_EVppBufType eBuf);
uint32_t u32VppBuf_GrallocFrameTypeGet(t_StVppBuf *pstBuf, t_EVppBufType *peBufType);

t_EVppBufType eVppBuf_GetFrameType(t_StVppBuf *pstBuf);

void vVppBuf_SetDumpParams(t_StVppCtx *pstCtx, char *pcPath);
uint32_t u32VppBuf_Dump(t_StVppCtx *pstCtx, t_StVppBuf *pstBuf,
                        const char *pcFn, uint32_t u32W, uint32_t u32H,
                        uint32_t u32Str, uint32_t u32UvOff);
uint32_t u32VppBuf_DumpExtraData(t_StVppCtx *pstCtx,
                                 t_StVppBuf *pstBuf,
                                 const char *pcFn);
uint32_t u32Vpp_CrcBuffer(t_StVppBuf *pstBuf, t_EVppTypeBuf eBuf, uint32_t start_offset,
                          uint32_t len, uint32_t idx, char* pmsg);


#ifdef __cplusplus
 }
#endif

#endif /* _VPP_BUF_H_ */
