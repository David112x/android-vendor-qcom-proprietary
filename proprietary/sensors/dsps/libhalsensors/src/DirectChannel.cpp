/*============================================================================
  @file DirectChannel.cpp

  @brief
  DirectChannel class implementation.

  Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/

#include "DirectChannel.h"

#include <string>
#include <cutils/native_handle.h>
#include <hardware/hardware.h>
#include <hardware/gralloc1.h>

#include "sensors_hal.h"
#include "rpcmem.h"
#include "sns_low_lat_stream.h"

// Initialize the android_handle_counter
int DirectChannel::client_channel_handle_counter = 0;

// Initialize the gralloc1 device and functions
gralloc1_device_t* DirectChannel::gralloc1_device = nullptr;
GRALLOC1_PFN_RETAIN DirectChannel::gralloc1_retain = nullptr;
GRALLOC1_PFN_RELEASE DirectChannel::gralloc1_release = nullptr;
GRALLOC1_PFN_LOCK DirectChannel::gralloc1_lock = nullptr;
GRALLOC1_PFN_UNLOCK DirectChannel::gralloc1_unlock = nullptr;
GRALLOC1_PFN_GET_BACKING_STORE DirectChannel::gralloc1_GetBackingStore = nullptr;

DirectChannel::DirectChannel(const struct native_handle *mem_handle,
    const size_t mem_size)
    :sns_low_lat_handle(-1),
     client_channel_handle(0)
{
    int ret = 0;
    // If the gralloc1 module singleton hasn't been started, bring it up
    if (NULL == gralloc1_device)
    {
      if (initGralloc1Dev())
      {
        // Kick out if gralloc1_device initialization fails
        HAL_LOG_ERROR("%s: initGralloc1Dev failed!", __FUNCTION__);
        return;
      }
    }

    // Check the native_handle for validity
    if (mem_handle->numFds < 1)
    {
      HAL_LOG_ERROR("%s: Unexpected numFds. Expected at least 1. Received %d.", __FUNCTION__, mem_handle->numFds);
      return;
    }

    // Copy the native_handle
    mem_native_handle = native_handle_clone(mem_handle);

    buffer_size = mem_size;

    // Register the buffer
    ret = gralloc1_retain(gralloc1_device, mem_native_handle);
    if (ret != 0)
    {
      HAL_LOG_ERROR("%s: gralloc1_retain FAILED ret %d", __FUNCTION__, ret);
      return;
    }

    // Lock the shared memory to get the virtual address
    gralloc1_rect_t accessRegion = {
        .left = 0,
        .top = 0,
        .width = static_cast<int32_t>(buffer_size),
        .height = 1
    };
    ret = gralloc1_lock(gralloc1_device, mem_native_handle,
        GRALLOC1_PRODUCER_USAGE_SENSOR_DIRECT_DATA,
        GRALLOC1_CONSUMER_USAGE_CPU_READ_OFTEN, &accessRegion, &buffer_ptr, -1);
    if (ret != 0)
    {
      HAL_LOG_ERROR("%s: gralloc1_lock FAILED ret %d", __FUNCTION__, ret);
      return;
    }

    // Increment the Android handle counter and check it for validity. Then
    // assign the resulting Android handle to this Direct Channel
    ++client_channel_handle_counter;
    if (client_channel_handle_counter <= 0)
    {
      client_channel_handle_counter = 1;
    }
    client_channel_handle = client_channel_handle_counter;

    // Clear out the buffer
    memset(buffer_ptr, 0, buffer_size);
}

// Copy constructor
DirectChannel::DirectChannel(const DirectChannel &copy)
{
    mem_native_handle = native_handle_clone(copy.mem_native_handle);
    buffer_ptr = copy.buffer_ptr;
    buffer_size = copy.buffer_size;
    sns_low_lat_handle = copy.sns_low_lat_handle;
    client_channel_handle = copy.client_channel_handle;
}

DirectChannel::~DirectChannel()
{
    int ret = 0;
    int32_t dummy_var; // Not really used

    // Make sure the object is valid by checking the android_handle
    if (0 != client_channel_handle)
    {
      ret = sns_low_lat_stream_deinit(sns_low_lat_handle);

      gralloc1_unlock(gralloc1_device, mem_native_handle, &dummy_var);
      gralloc1_release(gralloc1_device, mem_native_handle);

      ret = native_handle_close(mem_native_handle);
    }
}

int DirectChannel::getLowLatHandle()
{
    return sns_low_lat_handle;
}

int DirectChannel::getClientChannelHandle()
{
    return client_channel_handle;
}

int DirectChannel::getBufferFd()
{
    if (mem_native_handle)
    {
        return mem_native_handle->data[0];
    }
    else
    {
        return -1;
    }
}
bool DirectChannel::is_same_memory(const struct native_handle *mem_handle)
{
  gralloc1_backing_store_t s1, s2;

  if ((gralloc1_GetBackingStore(gralloc1_device, mem_native_handle, &s1) != GRALLOC1_ERROR_NONE) ||
    (gralloc1_GetBackingStore(gralloc1_device, mem_handle, &s2) != GRALLOC1_ERROR_NONE)) {
    HAL_LOG_ERROR("%s: get backing store FAILED, h1=%p, h2=%p", __FUNCTION__,
        mem_native_handle, mem_handle);
    return false;
  }

  HAL_LOG_VERBOSE("Get backing store, s1=%p, s2=%p", (void *)s1, (void *)s2);

  return (s1 == s2);
}

int DirectChannel::init()
{
    int ret = 0;

    // Check the health of the DirectChannel object
    if(0 == client_channel_handle)
    {
      HAL_LOG_ERROR("%s: init FAILED. DirectChannel object is in a bad state.", __FUNCTION__);
      return -1;
    }

    // Map the shared memory to a fastRPC addressable file descriptor
    register_buf(buffer_ptr, buffer_size, this->getBufferFd());

    // Initialize the buffer with the Low Latency library
    ret = sns_low_lat_stream_init(this->getBufferFd(), buffer_size, 0, &sns_low_lat_handle);

    return ret;
}


int DirectChannel::deinit()
{
    int ret = 0;

    // Check the health of the DirectChannel object
    if(0 == client_channel_handle)
    {
      HAL_LOG_ERROR("%s: deinit FAILED. DirectChannel object is in a bad state.", __FUNCTION__);
      return -1;
    }

    ret = sns_low_lat_stream_deinit(sns_low_lat_handle);

    return ret;
}


int DirectChannel::initGralloc1Dev(void)
{
  int ret = 0;

  const hw_module_t* hw_mod = NULL;
  ret = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &hw_mod);
  if (ret != 0)
  {
    HAL_LOG_ERROR("%s: hw_get_module FAILED ret %d", __FUNCTION__, ret);
    return -1;
  }

  // Check the gralloc API version for gralloc1
  if (((hw_mod->module_api_version >> 8) & 0xFF) != 1)
  {
    HAL_LOG_ERROR("%s: unsupported gralloc version 0x%X", __FUNCTION__,
        hw_mod->module_api_version);
    return -2;
  }
  else
  {
    // Open the gralloc1 module
    ret = gralloc1_open(hw_mod, &gralloc1_device);
    if (ret != 0)
    {
      HAL_LOG_ERROR("%s: gralloc1_open FAILED ret %d", __FUNCTION__, ret);
      return -3;
    }
    else
    {
      // Get the necessary function pointers
      gralloc1_retain = (GRALLOC1_PFN_RETAIN)gralloc1_device->getFunction(gralloc1_device, GRALLOC1_FUNCTION_RETAIN);
      gralloc1_release = (GRALLOC1_PFN_RELEASE)gralloc1_device->getFunction(gralloc1_device, GRALLOC1_FUNCTION_RELEASE);
      gralloc1_lock = (GRALLOC1_PFN_LOCK)gralloc1_device->getFunction(gralloc1_device, GRALLOC1_FUNCTION_LOCK);
      gralloc1_unlock = (GRALLOC1_PFN_UNLOCK)gralloc1_device->getFunction(gralloc1_device, GRALLOC1_FUNCTION_UNLOCK);
      gralloc1_GetBackingStore = (GRALLOC1_PFN_GET_BACKING_STORE)(gralloc1_device->getFunction(gralloc1_device,GRALLOC1_FUNCTION_GET_BACKING_STORE));

      if (gralloc1_retain == NULL || gralloc1_release == NULL
       || gralloc1_lock == NULL || gralloc1_unlock == NULL
       || gralloc1_GetBackingStore == NULL)
      {
        HAL_LOG_ERROR("Fail to get gralloc1 function pointer: retain=%p, release=%p, lock=%p, unlock=%p, getBackingStore=%p",
            gralloc1_retain, gralloc1_release, gralloc1_lock, gralloc1_unlock, gralloc1_GetBackingStore);
        return -4;
      }
    }
  }

  return 0;
}
