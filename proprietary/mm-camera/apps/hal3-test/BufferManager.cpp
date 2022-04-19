////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  BufferManager.cpp
/// @brief Buffer Manager
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "BufferManager.h"
#include <stdlib.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <gralloc_priv.h>
#include <unistd.h>
#include <linux/dma-buf.h>
#include <linux/msm_ion.h>
#if TARGET_ION_ABI_VERSION >= 2
#include <ion/ion.h>
#endif


#undef LOG_TAG
#define LOG_TAG "CameraTest"
#include <log/log.h>
const char* CP_ION_LIB  = "libcpion.so";

BufferManager::BufferManager():\
mNumBuffers(0),
mBufferStride(0){
    Initialize();
#ifndef USE_GRALLOC1
    mIonFd = -1;
#endif
}

/************************************************************************
* name : ~BufferManager
* function: destruct object
************************************************************************/
BufferManager::~BufferManager()
{
    Destroy();
}

/************************************************************************
* name : Destory
* function: release all resource
************************************************************************/
void BufferManager::Destroy()
{
    FreeAllBuffers();
#ifdef USE_GRALLOC1
    gralloc1_close(mGralloc1Device);
#else
    close(mIonFd);
#endif
}

/************************************************************************
* name : Initialize
* function: Inint Gralloc interface
************************************************************************/
int BufferManager::Initialize()
{
    int result = 0;
#ifdef USE_GRALLOC1
    result = SetupGralloc1Interface();
#endif
    return result;
}

#ifdef USE_GRALLOC1
/************************************************************************
* name : SetupGralloc1Interface
* function: dlsym all symboles which used to alloc buffers
************************************************************************/
int BufferManager::SetupGralloc1Interface()
{
    int result = 0;

    hw_get_module(GRALLOC_HARDWARE_MODULE_ID, const_cast<const hw_module_t**>(&mHwModule));

    if (NULL != mHwModule)
    {
        gralloc1_open(mHwModule, &mGralloc1Device);
    }
    else
    {
        ALOGE("Can not get Gralloc hardware module\n");
        result = -1;
    }

    if (NULL != mGralloc1Device)
    {
        mGrallocInterface.CreateDescriptor  = reinterpret_cast<GRALLOC1_PFN_CREATE_DESCRIPTOR>(
            mGralloc1Device->getFunction(mGralloc1Device,
                GRALLOC1_FUNCTION_CREATE_DESCRIPTOR));
        mGrallocInterface.DestroyDescriptor = reinterpret_cast<GRALLOC1_PFN_DESTROY_DESCRIPTOR>(
            mGralloc1Device->getFunction(mGralloc1Device,
                GRALLOC1_FUNCTION_DESTROY_DESCRIPTOR));
        mGrallocInterface.SetDimensions     = reinterpret_cast<GRALLOC1_PFN_SET_DIMENSIONS>(
            mGralloc1Device->getFunction(mGralloc1Device,
                GRALLOC1_FUNCTION_SET_DIMENSIONS));
        mGrallocInterface.SetFormat         = reinterpret_cast<GRALLOC1_PFN_SET_FORMAT>(
            mGralloc1Device->getFunction(mGralloc1Device,
                GRALLOC1_FUNCTION_SET_FORMAT));
        mGrallocInterface.SetProducerUsage  = reinterpret_cast<GRALLOC1_PFN_SET_PRODUCER_USAGE>(
            mGralloc1Device->getFunction(mGralloc1Device,
                GRALLOC1_FUNCTION_SET_PRODUCER_USAGE));
        mGrallocInterface.SetConsumerUsage  = reinterpret_cast<GRALLOC1_PFN_SET_CONSUMER_USAGE>(
            mGralloc1Device->getFunction(mGralloc1Device,
                GRALLOC1_FUNCTION_SET_CONSUMER_USAGE));
        mGrallocInterface.Allocate          = reinterpret_cast<GRALLOC1_PFN_ALLOCATE>(
            mGralloc1Device->getFunction(mGralloc1Device,
                GRALLOC1_FUNCTION_ALLOCATE));
        mGrallocInterface.GetStride         = reinterpret_cast<GRALLOC1_PFN_GET_STRIDE>(
            mGralloc1Device->getFunction(mGralloc1Device,
                GRALLOC1_FUNCTION_GET_STRIDE));
        mGrallocInterface.Release           = reinterpret_cast<GRALLOC1_PFN_RELEASE>(
            mGralloc1Device->getFunction(mGralloc1Device,
                GRALLOC1_FUNCTION_RELEASE));
        mGrallocInterface.Lock              = reinterpret_cast<GRALLOC1_PFN_LOCK>(
            mGralloc1Device->getFunction(mGralloc1Device,
                GRALLOC1_FUNCTION_LOCK));
    }
    else
    {
        ALOGE("gralloc1_open failed\n");
        result = -1;
    }

    return result;
}
#endif

/************************************************************************
* name : AllocateBuffers
* function: Alloc buffrer based on input paramaters
************************************************************************/
int BufferManager::AllocateBuffers(
    uint32_t numBuffers,
    uint32_t width,
    uint32_t height,
    uint32_t format,
    uint64_t producerFlags,
    uint64_t consumerFlags,
    uint32_t isVideoMeta,
    uint32_t isUBWC)
{
    int result = 0;

    ALOGI("AllocateBuffers, Enter");
    mIsMetaBuf = isVideoMeta;
    mIsUBWC = isUBWC;

    for (uint32_t i = 0; i < numBuffers; i++)
    {
        AllocateOneBuffer(width, height, format, producerFlags, consumerFlags, &mBuffers[i], &mBufferStride,i);
        mNumBuffers++;
        mBuffersFree.push_back(&mBuffers[i]);
    }
#ifdef ENABLE_SECURE_MEM_DUMP
    AllocateOneNonSecureBuffer(width, height, format, producerFlags & (~GRALLOC_USAGE_PROTECTED),
                                consumerFlags & (~GRALLOC_USAGE_PROTECTED), &mNonSecBuffer, &mNonSecBufferStride);
#endif
    ALOGI("AllocateBuffers, EXIT");
    return result;
}

#ifdef ENABLE_SECURE_MEM_DUMP
int BufferManager::AllocateOneNonSecureBuffer(
    uint32_t           width,
    uint32_t           height,
    uint32_t           format,
    uint64_t           producerUsageFlags,
    uint64_t           consumerUsageFlags,
    buffer_handle_t*   pAllocatedBuffer,
    uint32_t*          pStride)
{
    int32_t result =  0;
#if TARGET_ION_ABI_VERSION >= 2
        struct ion_allocation_data alloc;
        struct ion_fd_data ion_info_fd;
        struct ion_fd_data data2;
        native_handle_t* nh = nullptr;
        int rc;
        size_t buf_size;
        int ion_fd;
        struct dma_buf_sync buf_sync;

        ion_fd = ion_open();
        if (ion_fd < 0)
        {
            ALOGE("Opening ION device for allocation failed, errno = %x ", errno);
            return -1;
        }

        memset(&alloc, 0, sizeof(alloc));
        if (height == 1) {
            // Blob
            buf_size = (size_t)width;
        } else {
            buf_size = (size_t)(width * height * 2);
        }

        if ((format == HAL_PIXEL_FORMAT_RAW16) || (format == HAL_PIXEL_FORMAT_RAW10))
        {
            buf_size = (size_t)(width * height * 2)/4;
        }
        alloc.len = (size_t)(buf_size);
        alloc.len = (alloc.len + 4095U) & (~4095U);
        alloc.align = 4096;
        alloc.flags = 0;
        alloc.heap_id_mask = ION_HEAP(ION_QSECOM_HEAP_ID);
        memset(&ion_info_fd, 0, sizeof(ion_info_fd));

        rc= ion_alloc_fd(ion_fd, alloc.len, alloc.align, alloc.heap_id_mask, alloc.flags, &(ion_info_fd.fd));
        if (rc < 0)
        {
            ALOGE("Error: ion_alloc_fd failed, ret = %d, errno = %x, len = %zu", rc, errno, alloc.len);
            ion_info_fd.fd = -1;
            return -1;
        }

        mNonSecBufferInfo.fd = ion_info_fd.fd;
        mNonSecBufferInfo.size = alloc.len;
        mNonSecBufferInfo.vaddr = (uint8_t*)mmap(NULL, alloc.len, PROT_READ | PROT_WRITE, MAP_SHARED, mNonSecBufferInfo.fd, 0);
        if (mNonSecBufferInfo.vaddr == MAP_FAILED)
        {
            ALOGE("Error::ION MMAP failed: len %zu, errno = %d", alloc.len, errno);
            mNonSecBufferInfo.vaddr = NULL;
            return -1;
        }
        buf_sync.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
        rc = ioctl(mNonSecBufferInfo.fd, DMA_BUF_IOCTL_SYNC, &buf_sync);
        if (rc)
        {
            ALOGE("Error:: DMA_BUF_IOCTL_SYNC start failed, ret = %d, errno = %d", rc, errno);
            munmap(mNonSecBufferInfo.vaddr, mNonSecBufferInfo.size);
            mNonSecBufferInfo.vaddr = NULL;
            return -1;
        }

        ALOGD("%s ION FD %d len %d\n", __func__, mNonSecBufferInfo.fd, alloc.len);
        if (!mIsMetaBuf) {
            *pAllocatedBuffer = native_handle_create(2, 4);
            nh = (native_handle_t*)(*pAllocatedBuffer);
            (nh)->data[0] = mNonSecBufferInfo.fd;
            (nh)->data[1] = 0;
            (nh)->data[2] = 0;
            (nh)->data[3] = 0;
            (nh)->data[4] = alloc.len;
            (nh)->data[5] = 0;
        } else {
            /*alloc private handle_t */

            /*(buffer_handler_t **)*/
            if (!mIsUBWC) {
                *pAllocatedBuffer = native_handle_create(1, 2);
                nh = (native_handle_t*)(*pAllocatedBuffer);
                (nh)->data[0] = mNonSecBufferInfo.fd;
                (nh)->data[1] = 0;
                (nh)->data[2] = alloc.len;
            } else {
                /*UBWC Mode*/
            #if 0
                private_handle_t *pnh = new private_handle_t(ion_info_fd.fd,
                    alloc.len,
                    0,
                    0,
                    0,
                    width,
                    height);
                *pAllocatedBuffer = (native_handle_t *)pnh;
                pnh->flags |= private_handle_t::PRIV_FLAGS_UBWC_ALIGNED;
                pnh->width = width;
                pnh->height = height;
                pnh->size = alloc.len;
                pnh->unaligned_width = width;
                pnh->unaligned_height = height;
                pnh->fd = ion_info_fd.fd;
                pnh->format = format;
            #endif
            }
        }
#endif
    return result;
}

void BufferManager::FreeNonSecureBuffers()
{
    if (NULL != mNonSecBufferInfo.vaddr)
    {
#if TARGET_ION_ABI_VERSION >= 2
        struct dma_buf_sync buf_sync;
        int rc;
        if (mNonSecBufferInfo.vaddr == NULL || mNonSecBufferInfo.fd < 0)
        {
            ALOGE("%s - Error invalid parameters vaddr = %p, fd = %d", __FUNCTION__, mNonSecBufferInfo.vaddr, 
                mNonSecBufferInfo.fd);
            return;
        }
        rc = munmap(mNonSecBufferInfo.vaddr, mNonSecBufferInfo.size);
        if (rc)
        {
            ALOGE("Error: ion munmap failed, ret = %d, errno = %x", rc, errno);
            return;
        }
        buf_sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;
        rc = ioctl(mNonSecBufferInfo.fd, DMA_BUF_IOCTL_SYNC, &buf_sync);
        if (rc)
        {
            ALOGE("Error:: DMA_BUF_IOCTL_SYNC start failed, ret = %d, errno = %d\n", rc, errno);
            return;
        }
        if(mNonSecBufferInfo.fd >= 0)
        {
            rc= close(mNonSecBufferInfo.fd);
            if (rc != 0)
            {
                ALOGE("Error: cp_ion_free failed %s , ret = %d, errno = %x , ion_fd_data->fd = %d", __FUNCTION__, rc, errno, mNonSecBufferInfo.fd);
                return;
            }
            mNonSecBufferInfo.fd = -1;
        }
        mNonSecBuffer = NULL;
#endif
    }
}
#endif
/************************************************************************
* name : AllocateOneBuffer
* function: subcase to alloc buf
************************************************************************/
int BufferManager::AllocateOneBuffer(
    uint32_t           width,
    uint32_t           height,
    uint32_t           format,
    uint64_t           producerUsageFlags,
    uint64_t           consumerUsageFlags,
    buffer_handle_t*   pAllocatedBuffer,
    uint32_t*          pStride,
    uint32_t           index)
{
    int32_t result =  0;
#ifdef USE_GRALLOC1
    result = GRALLOC1_ERROR_NONE;
    gralloc1_buffer_descriptor_t gralloc1BufferDescriptor;

    result = mGrallocInterface.CreateDescriptor(mGralloc1Device, &gralloc1BufferDescriptor);

    if (GRALLOC1_ERROR_NONE == result)
    {
        result = mGrallocInterface.SetDimensions(mGralloc1Device, gralloc1BufferDescriptor, width, height);
    }

    if (GRALLOC1_ERROR_NONE == result)
    {
        result = mGrallocInterface.SetFormat(mGralloc1Device, gralloc1BufferDescriptor, format);
    }

    if (GRALLOC1_ERROR_NONE == result)
    {
        result = mGrallocInterface.SetProducerUsage(mGralloc1Device, gralloc1BufferDescriptor, producerUsageFlags);
    }

    if (GRALLOC1_ERROR_NONE == result)
    {
        result = mGrallocInterface.SetConsumerUsage(mGralloc1Device, gralloc1BufferDescriptor, consumerUsageFlags);
    }

    if (GRALLOC1_ERROR_NONE == result)
    {
        result = mGrallocInterface.Allocate(mGralloc1Device, 1, &gralloc1BufferDescriptor, &pAllocatedBuffer[0]);
    }

    if (GRALLOC1_ERROR_NONE == result)
    {
        result = mGrallocInterface.GetStride(mGralloc1Device, *pAllocatedBuffer, pStride);
    }

    if (GRALLOC1_ERROR_NONE != result)
    {
        ALOGE("allocate buffer failed\n");
        return result;
    }

    private_handle_t *hnl = ((private_handle_t *)(*pAllocatedBuffer));
    mBufferinfo[index].vaddr= mmap(NULL, hnl->size, PROT_READ  | PROT_WRITE, MAP_SHARED, hnl->fd, 0);
    mBufferinfo[index].size = hnl->size;
    mBufferinfo[index].fd  = ((private_handle_t *)(*pAllocatedBuffer))->fd;

    mGrallocInterface.DestroyDescriptor(mGralloc1Device, gralloc1BufferDescriptor);
#else
    struct ion_allocation_data alloc;
    struct ion_fd_data ion_info_fd;
    struct ion_fd_data data2;
    native_handle_t* nh = nullptr;
    int rc;
    size_t buf_size;
    if (mIonFd <= 0) {
        mIonFd = open("/dev/ion", O_RDONLY);
    }
    if (mIonFd <= 0) {
        ALOGE("Ion dev open failed %s\n", strerror(errno));
        return NULL;
    }
    memset(&alloc, 0, sizeof(alloc));
    if (height == 1) {
        // Blob
        buf_size = (size_t)width;
    } else {
        buf_size = (size_t)(width * height * 2);
    }

    alloc.len = (size_t)(buf_size);
    alloc.len = (alloc.len + 4095U) & (~4095U);
    alloc.align = 4096;
    alloc.flags = ION_FLAG_CACHED;
    alloc.heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
    rc = ioctl(mIonFd, ION_IOC_ALLOC, &alloc);
    if (rc < 0) {
        ALOGE("ION allocation failed %s with rc = %d fd:%d\n", strerror(errno), rc, mIonFd);
        return NULL;
    }
    memset(&ion_info_fd, 0, sizeof(ion_info_fd));
    ion_info_fd.handle = alloc.handle;
    rc = ioctl(mIonFd, ION_IOC_SHARE, &ion_info_fd);
    if (rc < 0) {
        ALOGE("ION map failed %s\n", strerror(errno));
        return NULL;
    }
    ALOGD("%s ION FD %d len %d\n", __func__, ion_info_fd.fd, alloc.len);
    if (!mIsMetaBuf) {
        *pAllocatedBuffer = native_handle_create(2, 4);
        nh = (native_handle_t*)(*pAllocatedBuffer);
        (nh)->data[0] = ion_info_fd.fd;
        (nh)->data[1] = 0;
        (nh)->data[2] = 0;
        (nh)->data[3] = 0;
        (nh)->data[4] = alloc.len;
        (nh)->data[5] = 0;
    } else {
        /*alloc private handle_t */

        /*(buffer_handler_t **)*/
        if (!mIsUBWC) {
            *pAllocatedBuffer = native_handle_create(1, 2);
            nh = (native_handle_t*)(*pAllocatedBuffer);
            (nh)->data[0] = ion_info_fd.fd;
            (nh)->data[1] = 0;
            (nh)->data[2] = alloc.len;
        } else {
            /*UBWC Mode*/
            #if 0
            private_handle_t *pnh = new private_handle_t(ion_info_fd.fd,
                alloc.len,
                0,
                0,
                0,
                width,
                height);
            *pAllocatedBuffer = (native_handle_t *)pnh;
            pnh->flags |= private_handle_t::PRIV_FLAGS_UBWC_ALIGNED;
            pnh->width = width;
            pnh->height = height;
            pnh->size = alloc.len;
            pnh->unaligned_width = width;
            pnh->unaligned_height = height;
            pnh->fd = ion_info_fd.fd;
            pnh->format = format;
            #endif
        }
    }

    data2.handle = ion_info_fd.handle;
    rc = ioctl(mIonFd, ION_IOC_MAP, &data2);
    if (rc) {
        ALOGE("ION MAP failed %s\n", strerror(errno));
        return NULL;
    }
    mBufferinfo[index].vaddr = mmap(NULL, alloc.len, PROT_READ  | PROT_WRITE, MAP_SHARED, data2.fd, 0);
    ALOGI("Alloc buffer fd:%d len:%d vaddr:%p\n",data2.fd,alloc.len,mBufferinfo[index].vaddr);
    mBufferinfo[index].size = alloc.len;
    mBufferinfo[index].allocData = alloc;
    mBufferinfo[index].fd = ion_info_fd.fd;
#endif
    return result;
}

/************************************************************************
* name : FreeAllBuffers
* function: free buffers
************************************************************************/
void BufferManager::FreeAllBuffers()
{
    for (uint32_t i = 0; i < mNumBuffers; i++)
    {
        if (NULL != mBuffers[i])
        {
            munmap(mBufferinfo[i].vaddr, mBufferinfo[i].size);
#ifdef USE_GRALLOC1
            mGrallocInterface.Release(mGralloc1Device, mBuffers[i]);
#else
            ioctl(mIonFd, ION_IOC_FREE,mBufferinfo[i].allocData.handle);
            close(mBufferinfo[i].fd);
            native_handle_close((native_handle_t *)mBuffers[i]);
            native_handle_delete((native_handle_t *)mBuffers[i]);
#endif
            mBuffers[i] = NULL;
        }
    }
#ifdef ENABLE_SECURE_MEM_DUMP
    FreeNonSecureBuffers();
#endif
}


