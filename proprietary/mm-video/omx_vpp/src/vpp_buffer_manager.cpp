/* =========================================================================
Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

Copyright (c) 2009, The Linux Foundation. All rights reserved.
--------------------------------------------------------------------------*/
/*============================================================================
                            O p e n M A X   w r a p p e r s
                O p e n  M A X   C o m p o n e n t  I n t e r f a c e

*//** @file vpp_buffer_manager.cpp
  This module contains the abstract interface for the OpenMAX components.

*//*========================================================================*/

//////////////////////////////////////////////////////////////////////////////
//                             Include Files
//////////////////////////////////////////////////////////////////////////////
#include <dlfcn.h>
#include <sys/prctl.h>
#include <gralloc_priv.h>
#include <media/hardware/HardwareAPI.h>
#include <cmath>
#include <qdMetaData.h>
#include <algorithm>
#include <sys/mman.h>
#include "OMX_QCOMExtns.h"

#define _ANDROID_
#include "vidc_debug.h"
#include "omx_vdec.h"
#undef _ANDROID_

#ifdef _TARGET_KERNEL_VERSION_44_
#include "vpp_buffer_manager.h"
#endif

#include "vpp_omx_component.h"

#define DISPLAY_BUF_COUNT 4
#define ARG_TOUCH(x) (void)x
#define MAX_NUM_VPP_BUFFERS 18

VppBufferManager::VppBufferManager():
    mInFrc(false),
    mInBypass(false),
    mDisableDynamicBufMode(false),
    mOmxMinBufferCount(0),
    mOmxVppTotalBufferCount(0),
    mOmxFtbCount(0),
    mVppFtbCount(0),
    mMinVppOutCount(1),
    mExtraDataFd(-1),
    mExtraDataTotalSize(0),
    mExtraDataSize(0),
    pExtraDataVirBaseAdrr(NULL),
    mOMXBufferID(0)
{
    DEBUG_PRINT_LOW(" Constructed the buffer manager. ");
}

VppBufferManager:: ~VppBufferManager()
{
    //clean up Client ExtraData buffer
    if (pExtraDataVirBaseAdrr) {
        munmap(pExtraDataVirBaseAdrr, mExtraDataTotalSize);
        pExtraDataVirBaseAdrr = NULL;
    }
    if (mExtraDataFd != -1) {
        close(mExtraDataFd);
        mExtraDataFd = -1;
    }

    DEBUG_PRINT_INFO("Destructed VppBufferManager");
}

bool VppBufferManager:: ResetBufferRequirements(OMX_PARAM_PORTDEFINITIONTYPE* portDef) {
    portDef->nBufferCountMin = mOmxMinBufferCount;
    portDef->nBufferCountActual = mOmxBufferCount;
    return true;
}

bool VppBufferManager:: UpdateBufferRequirements(struct vpp_requirements &vppreq,
                                                    OMX_PARAM_PORTDEFINITIONTYPE* portDef)
{
    if(portDef->nBufferCountMin == mOmxMinBufferCount) {
        portDef->nBufferCountMin = mOmxVppTotalBufferCount;
        DEBUG_PRINT_INFO("No need to change buffer requirements. ");
        return false;
    }
    OMX_VIDEO_PORTDEFINITIONTYPE *videoDef = &(portDef->format.video);
    uint32_t u32VppRes;
    if ((videoDef->nFrameWidth <= RES_SD_MAX_W) && (videoDef->nFrameHeight <= RES_SD_MAX_H))
        u32VppRes = VPP_RESOLUTION_SD;
    else if ((videoDef->nFrameWidth <= RES_HD_MAX_W) && (videoDef->nFrameHeight <= RES_HD_MAX_H))
        u32VppRes = VPP_RESOLUTION_HD;
    else if ((videoDef->nFrameWidth <= RES_FHD_MAX_W) && (videoDef->nFrameHeight <= RES_FHD_MAX_H))
        u32VppRes = VPP_RESOLUTION_FHD;
    else
        u32VppRes = VPP_RESOLUTION_UHD;

    mOmxBufferCount = portDef->nBufferCountActual;
    mOmxMinBufferCount = portDef->nBufferCountMin;

    mVppFtbCount = mOmxBufferCount*vppreq.out_factor[u32VppRes].mul + vppreq.out_factor[u32VppRes].add;
    mVppFtbCount = std::min((uint32_t)MAX_NUM_VPP_BUFFERS, mVppFtbCount);
    mOmxFtbCount = mOmxBufferCount;
    mMinVppOutCount = 2 * vppreq.out_factor[u32VppRes].mul + vppreq.out_factor[u32VppRes].add;
    mOmxVppTotalBufferCount = mOmxBufferCount + mVppFtbCount;

    // Max supported buffers in firmware is 32. Hence clamping it to 32
    if(mOmxVppTotalBufferCount > 32) {
        mVppFtbCount = 32 - mOmxFtbCount;
        mOmxVppTotalBufferCount = 32;
    }

    portDef->nBufferCountActual = mOmxVppTotalBufferCount;
    portDef->nBufferCountMin = mOmxVppTotalBufferCount;
    DEBUG_PRINT_LOW("get_parameter override bufcount from %d to %d",
             mOmxBufferCount, mOmxVppTotalBufferCount);

    DEBUG_PRINT_LOW(" min %d, vdec %d, vpp %d, total %d", mOmxMinBufferCount, mOmxFtbCount,
                                    mVppFtbCount, mOmxVppTotalBufferCount);
    return true;
}

bool VppBufferManager:: RegisterClientBuffer(OMX_BUFFERHEADERTYPE *buffer, OMX_U8* handle)
{

    DEBUG_PRINT_INFO("In registerClientBuffer.. buffer %p",(void*) buffer);
    OmxVppBuffer omxBuffer;
    omxBuffer.omxHeader = buffer;
    omxBuffer.owner = OMX_CLIENT;
    omxBuffer.handle = handle;
    omxBuffer.bufferID = mOMXBufferID++;
    mBuffers.push_back(omxBuffer);
    return true;
}

bool VppBufferManager:: DeRegisterClientBuffer(OMX_BUFFERHEADERTYPE *buffer)
{
    AutoMutex autoLock(mVppBufferLock);
    for (unsigned i=0; i<mBuffers.size(); i++) {
        if (mBuffers[i].omxHeader == buffer) {
            mBuffers.erase(mBuffers.begin()+i);
            return true;
        }
    }
    return false;
}

bool VppBufferManager:: UpdateBufferOwnership(OMX_BUFFERHEADERTYPE* buffer, OmxBufferOwner buff_owner)
{

    for (OmxVppBuffer &info : mBuffers) {
        if (info.omxHeader == buffer) {
            info.owner = buff_owner;
            return true;
        }
    }
    DEBUG_PRINT_INFO("could not find OmxVppBuffer for %p", buffer);
    return false;
}

VppBufferManager::OmxVppBuffer& VppBufferManager:: GetOmxVppBuffer(OMX_BUFFERHEADERTYPE* buffer)
{

    for (OmxVppBuffer &info : mBuffers) {
        if (info.omxHeader == buffer) {
            return info;
        }
    }
    DEBUG_PRINT_INFO("could not find OmxVppBuffer for %p", buffer);
    // There is a memory leak here. But happens in case of a grave error
    return *(new OmxVppBuffer());
}

VppBufferManager::OmxBufferOwner VppBufferManager:: GetFtbDestination(
        OMX_BUFFERHEADERTYPE* buffer,
        struct vpp_buffer **vppBuffer,
        bool inReconfig,
        bool eosReached,
        uint8_t passExtradata)
{
    OmxBufferOwner bufferOwner = UNKNOWN;
    AutoMutex autoLock(mVppBufferLock);
    OmxVppBuffer& omxVppBuffer = GetOmxVppBuffer(buffer); //Lock acquired inside
    private_handle_t *handle = NULL;

    DEBUG_PRINT_LOW("vdec and vpp requirements: [%d,%d]",mOmxFtbCount,mVppFtbCount);

    if (!buffer || !buffer->pBuffer) {
        DEBUG_PRINT_INFO("%s: invalid params: %p", __FUNCTION__, buffer);
        return bufferOwner;
    } else if (!omxVppBuffer.omxHeader) {
        return bufferOwner;
    }

    if(mDisableDynamicBufMode) {
        handle = (private_handle_t *)omxVppBuffer.handle;
    }    else {
        struct VideoDecoderOutputMetaData *meta;
        meta = (struct VideoDecoderOutputMetaData *)buffer->pBuffer;
        handle = (private_handle_t *)meta->pHandle;
    }

    if (handle == NULL) {
        DEBUG_PRINT_INFO("%s: invalid handle: NULL", __FUNCTION__);
        return bufferOwner;
    }

    unsigned vdecOutCount, vppInCount, vppOutCount, clientCount;
    GetBufferCounts(vdecOutCount, vppInCount, vppOutCount, clientCount);
    DEBUG_PRINT_LOW("Number of buffers owned: Vdec[%u], VppIn[%u], VppOut[%u], Client[%u]",
            vdecOutCount,vppInCount,vppOutCount,clientCount);

    bool vppInBypass = !mVppFtbCount || (mInBypass && vppOutCount);
    bool vdecFull = (vdecOutCount >= mOmxFtbCount);
    bool vppFull = (vppOutCount >= mVppFtbCount);
    bool queueToVppOnReconfig = inReconfig && (vppOutCount < vppInCount) & !vppInBypass;

    bool isRoBuf = buffer->nFlags & OMX_BUFFERFLAG_READONLY;

    if (isRoBuf || vppInBypass || (vppOutCount && !queueToVppOnReconfig && !eosReached &&
        ((!vdecFull && vppInCount == 0) || (vppOutCount >= mMinVppOutCount)))) {
        buffer->nAllocLen = mDisableDynamicBufMode? buffer->nAllocLen:sizeof(struct VideoDecoderOutputMetaData);
        buffer->nFilledLen = 0;
        buffer->nTimeStamp = 0;
        DEBUG_PRINT_LOW("To Vdec: bufHdr ,FD,offset, allocLen, nFilledLen, nTimestamp = %p,%d,%d,%d,%d,%lld",
                buffer,handle->fd, buffer->nOffset, buffer->nAllocLen, buffer->nFilledLen, buffer->nTimeStamp);
        omxVppBuffer.owner = OMX_VDEC;
        bufferOwner = OMX_VDEC;
    } else {
        struct vpp_buffer *vppBuffer = &omxVppBuffer.vppBuffer;
        memset(vppBuffer,0,sizeof(struct vpp_buffer));
        if (passExtradata & EXTRADATA_EXTERNAL) {
            vppBuffer->extradata.pvMapped = pExtraDataVirBaseAdrr;
            vppBuffer->extradata.fd = mExtraDataFd;
            vppBuffer->extradata.offset = (omxVppBuffer.bufferID * mExtraDataSize);
            vppBuffer->extradata.alloc_len = mExtraDataTotalSize;
            vppBuffer->extradata.filled_len = 0;
            vppBuffer->extradata.valid_data_len = mExtraDataSize;
            DEBUG_PRINT_LOW("Extradata: Index %d, Base Address %p",
                         omxVppBuffer.bufferID, pExtraDataVirBaseAdrr);
        } else {
            vppBuffer->extradata.pvMapped = NULL;
            vppBuffer->extradata.fd = -1;
            DEBUG_PRINT_LOW("Extradata: Index %d, Internal Extradata or No Extradata, set to NULL",
                         omxVppBuffer.bufferID);
        }
        vppBuffer->pixel.fd = handle->fd;
        vppBuffer->pixel.offset = buffer->nOffset;
        vppBuffer->pixel.alloc_len = handle->size;
        vppBuffer->pixel.filled_len = buffer->nFilledLen = 0;
        vppBuffer->pixel.valid_data_len = handle->size;
        vppBuffer->pixel.pvMapped = NULL;
        vppBuffer->timestamp = buffer->nTimeStamp = 0;
        buffer->nFlags = vppBuffer->flags = 0; //clear flags here
        vppBuffer->cookie_in_to_out = NULL;
        vppBuffer->cookie = buffer;
        vppBuffer->pvGralloc = handle;
        DEBUG_PRINT_LOW("To Vpp OUT: allocLen %d, offset %d, fd %d, bufferHeader %p)",
                vppBuffer->pixel.alloc_len, buffer->nOffset,
                vppBuffer->pixel.fd, (void *)buffer);
        omxVppBuffer.owner = VPP_OUT;
        bufferOwner = VPP_OUT;
    }
    *vppBuffer = &omxVppBuffer.vppBuffer;
    return bufferOwner;
}

struct vpp_buffer* VppBufferManager:: GetBufferForVppIn(
        OMX_BUFFERHEADERTYPE* buffer)
{

    DEBUG_PRINT_LOW("Vdec Fbd to Vpp Input, buffer %p",(void*)buffer);
    OmxVppBuffer& omxVppBuffer = GetOmxVppBuffer(buffer);
    private_handle_t *handle = NULL;

    if(omxVppBuffer.omxHeader) {
        if (!buffer->nFilledLen && !(buffer->nFlags & OMX_BUFFERFLAG_EOS)) {
            return nullptr;
        }

        omxVppBuffer.owner = VPP_IN;
        struct vdec_output_frameinfo *bufInfo = (struct vdec_output_frameinfo*)buffer->pOutputPortPrivate;

        if(mDisableDynamicBufMode) {
            handle = (private_handle_t *)omxVppBuffer.handle;
        } else {
            struct VideoDecoderOutputMetaData *meta;
            meta = (struct VideoDecoderOutputMetaData *)buffer->pBuffer;
            handle = (private_handle_t *)meta->pHandle;
        }

        if (handle == NULL) {
            DEBUG_PRINT_INFO("%s: invalid handle: NULL", __FUNCTION__);
            return NULL;
        }

        struct vpp_buffer* vppBuffer = &omxVppBuffer.vppBuffer;
        memset(vppBuffer,0,sizeof(struct vpp_buffer));

        vppBuffer->pixel.fd = handle->fd;
        vppBuffer->pixel.offset = buffer->nOffset;
        vppBuffer->pixel.alloc_len = handle->size;
        vppBuffer->pixel.filled_len = bufInfo->len;
        vppBuffer->pixel.valid_data_len = bufInfo->len;
        vppBuffer->pixel.pvMapped = NULL;
        if (buffer->nFlags & OMX_BUFFERFLAG_EXTRADATA) {
            vppBuffer->extradata.pvMapped = pExtraDataVirBaseAdrr;
            vppBuffer->extradata.fd = mExtraDataFd;
            vppBuffer->extradata.offset = (omxVppBuffer.bufferID * mExtraDataSize);
            vppBuffer->extradata.alloc_len = mExtraDataTotalSize;
            vppBuffer->extradata.filled_len = mExtraDataSize;
            vppBuffer->extradata.valid_data_len = mExtraDataSize;
            DEBUG_PRINT_LOW("Extradata: Index %d,  Base Address %p",
                         omxVppBuffer.bufferID, pExtraDataVirBaseAdrr);
        } else {
            vppBuffer->extradata.pvMapped = NULL;
            vppBuffer->extradata.fd = -1;
            DEBUG_PRINT_LOW("Extradata: Index %d, 0-length buffer, set NULL eExtradata",
                         omxVppBuffer.bufferID);
        }
        ConvertOmxVppFlags(buffer->nFlags, vppBuffer->flags, true);
        vppBuffer->timestamp = buffer->nTimeStamp;
        vppBuffer->cookie_in_to_out = NULL;
        vppBuffer->cookie = buffer;
        vppBuffer->pvGralloc = handle;
        ConvertOmxVppFlags(buffer->nFlags, vppBuffer->flags, true);

        DEBUG_PRINT_LOW("ToVppIn: handle %d, offset %d, AllocLen %d, FilledLen %u,\
                     timestamp %llu, buffer %p",
                     handle->fd, buffer->nOffset, handle->size, (unsigned)bufInfo->len,
                     (long long unsigned)buffer->nTimeStamp, (void*)buffer);
        return vppBuffer;
    }

    DEBUG_PRINT_INFO("Error: OmxVppBuffer not found of omxHeader %p",(void*)buffer);
    return NULL;
}

void VppBufferManager:: PrintBufferStatistics()
{
    int vdecOut(0),vppIn(0),vppOut(0),client(0);

    for(OmxVppBuffer &info : mBuffers) {
        if(info.owner == OMX_VDEC)
            vdecOut++;
        else if(info.owner == VPP_IN)
            vppIn++;
        else if(info.owner == VPP_OUT)
            vppOut++;
    }
    client = mOmxVppTotalBufferCount - vppIn - vppOut - vdecOut;
    DEBUG_PRINT_LOW("Buffers: Client [%d], VDEC [%d], VPP_IN [%d], VPP_OUT [%d]",
            client,vdecOut,vppIn,vppOut);
}

// Should only be called from vpp_output_buffer done
bool VppBufferManager:: HandleBypass(OMX_BUFFERHEADERTYPE *omxHeader)
{
    if(mInFrc) {
        mInBypass = false;
        return false;
    }

    for(OmxVppBuffer &info : mBuffers) {
        if(info.omxHeader == omxHeader) {
            if(info.owner == VPP_IN) {
                mInBypass = true;
                return true;
            }
            mInBypass = false;
            return false;
        }
    }
    return false;
}

void VppBufferManager:: GetBufferCounts(unsigned& vdecCount, unsigned& vppInCount,
                                                           unsigned& vppOutCount,unsigned& clientCount)
{

    vdecCount = vppInCount = vppOutCount = clientCount = 0;

    for(OmxVppBuffer &info : mBuffers) {
        if(info.owner == OMX_VDEC)
            vdecCount++;
        else if(info.owner == VPP_IN)
            vppInCount++;
        else if(info.owner == VPP_OUT)
            vppOutCount++;
    }
    clientCount = mOmxVppTotalBufferCount - vppInCount -vppOutCount - vdecCount;
    DEBUG_PRINT_LOW("Buffers: Client [%d], VDEC [%d], VPP_IN [%d], VPP_OUT [%d]",
            clientCount,vdecCount,vppInCount,vppOutCount);

}

void VppBufferManager:: ClearBuffers() {
    mBuffers.clear();
    mOMXBufferID = 0;
}

bool VppBufferManager:: GetBufferMode() {
    return mDisableDynamicBufMode;
}

void VppBufferManager:: SetBufferMode(bool bDisableDynamicBufMode) {
    mDisableDynamicBufMode = bDisableDynamicBufMode;
}

void VppBufferManager:: UpdateBufferGeometry(OMX_BUFFERHEADERTYPE *buffer,
                                                            struct vpp_port_param *vppParams) {

    BufferDim_t dim;
    private_handle_t *private_handle = NULL;

    OmxVppBuffer& omxVppBuffer = GetOmxVppBuffer(buffer);

    dim.sliceWidth = vppParams->width;
    dim.sliceHeight = vppParams->height;

    private_handle = (private_handle_t*)omxVppBuffer.handle;

    if(private_handle) {
        DEBUG_PRINT_LOW("set metadata VPP: update buf-geometry with stride %d slice %d",
            dim.sliceWidth, dim.sliceHeight);

        setMetaData(private_handle, UPDATE_BUFFER_GEOMETRY, (void*)&dim);

    }
}

//Helper function to convert from OMX flags to VPP flags
void VppBufferManager:: ConvertOmxVppFlags(uint32_t inflags, uint32_t& outflags,bool fromOmxToVpp) {
    uint32_t ctzOmxEosFlag = __builtin_ctz(OMX_BUFFERFLAG_EOS);
    uint32_t ctzOmxDCFlag = __builtin_ctz(OMX_BUFFERFLAG_DATACORRUPT);
    uint32_t ctzOmxSyncFlag = __builtin_ctz(OMX_BUFFERFLAG_SYNCFRAME);
    uint32_t ctzOmxROFlag = __builtin_ctz(OMX_BUFFERFLAG_READONLY);

    uint32_t ctzVppEosFlag = __builtin_ctz(VPP_BUFFER_FLAG_EOS);
    uint32_t ctzVppDcFlag = __builtin_ctz(VPP_BUFFER_FLAG_DATACORRUPT);
    uint32_t ctzVppSyncFlag = __builtin_ctz(VPP_BUFFER_FLAG_SYNCFRAME);
    uint32_t ctzVppROFlag = __builtin_ctz(VPP_BUFFER_FLAG_READONLY);

    DEBUG_PRINT_LOW(" Input Flags: %s and value is 0x%x",
        fromOmxToVpp ? "OMX" : "Vpp",
        inflags);

    if (fromOmxToVpp) {
        outflags = (((inflags & OMX_BUFFERFLAG_EOS) >> ctzOmxEosFlag) << ctzVppEosFlag ) | \
                (((inflags & OMX_BUFFERFLAG_DATACORRUPT) >> ctzOmxDCFlag) << ctzVppDcFlag ) | \
                (((inflags & OMX_BUFFERFLAG_SYNCFRAME) >> ctzOmxSyncFlag) << ctzVppSyncFlag) | \
                (((inflags & OMX_BUFFERFLAG_READONLY) >> ctzOmxROFlag) << ctzVppROFlag);
    } else {
        outflags = (((inflags & VPP_BUFFER_FLAG_EOS) >> ctzVppEosFlag) << ctzOmxEosFlag) | \
                (((inflags & VPP_BUFFER_FLAG_DATACORRUPT) >> ctzVppDcFlag) << ctzOmxDCFlag) | \
                (((inflags & VPP_BUFFER_FLAG_SYNCFRAME) >> ctzVppSyncFlag) << ctzOmxSyncFlag) | \
                (((inflags & VPP_BUFFER_FLAG_READONLY) >> ctzVppROFlag) << ctzOmxROFlag);
    }

    DEBUG_PRINT_LOW(" Output Flags: %s and value is 0x%x",
        fromOmxToVpp ? "Vpp" : "OMX",
        outflags);
}

void VppBufferManager:: UpdateMetaData(OMX_BUFFERHEADERTYPE *buffer,
                                                            uint32_t colorFormat) {
    OmxVppBuffer& omxVppBuffer = GetOmxVppBuffer(buffer);
    private_handle_t *handle = NULL;

    if (omxVppBuffer.omxHeader) {
        if (mDisableDynamicBufMode) {
            handle = (private_handle_t *)omxVppBuffer.handle;
        } else {
            struct VideoDecoderOutputMetaData *meta;
            meta = (struct VideoDecoderOutputMetaData *)buffer->pBuffer;
            handle = (private_handle_t *)meta->pHandle;
        }

        if (!mInBypass && colorFormat == QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m) {
            setMetaData(handle, LINEAR_FORMAT, (void*)&colorFormat);
        }
    }
}

bool VppBufferManager::SetClientExtraDataBuffer(int32_t fd, uint32_t totalSize, uint32_t size) {
    //clean up old Client ExtraData buffer
    if (pExtraDataVirBaseAdrr) {
        munmap(pExtraDataVirBaseAdrr, mExtraDataTotalSize);
        pExtraDataVirBaseAdrr = NULL;
    }
    if (mExtraDataFd != -1) {
        close(mExtraDataFd);
        mExtraDataFd = -1;
    }
    mExtraDataSize = 0;
    mExtraDataTotalSize = 0;

    //set new Client ExtraData buffer
    if (fd < 0 || !totalSize) {
        return false;
    }

    mExtraDataFd = fd;
    mExtraDataTotalSize= totalSize;
    mExtraDataSize= size;
    pExtraDataVirBaseAdrr = (OMX_U8*)mmap(0, totalSize,
            PROT_READ|PROT_WRITE, MAP_SHARED, mExtraDataFd, 0);
    if (pExtraDataVirBaseAdrr == MAP_FAILED) {
        pExtraDataVirBaseAdrr = NULL;
        if (mExtraDataFd != -1) {
            close(mExtraDataFd);
            mExtraDataFd = -1;
        }
        mExtraDataSize = 0;
        mExtraDataTotalSize = 0;
        DEBUG_PRINT_INFO("SetClientExtraDataBuffer: mmap failed");
        return false;
    }
    DEBUG_PRINT_LOW("SetClientExtraDataBuffer: Success.");
    return true;
}
