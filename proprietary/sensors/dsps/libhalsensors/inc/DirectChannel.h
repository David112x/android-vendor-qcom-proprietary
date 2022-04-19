/*============================================================================
  @file DirectChannel.h

  @brief
  DirectChannel class definition.

  Copyright (c) 2017,2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/

#pragma once


#include <cstdlib>
#include <cutils/native_handle.h>
#include <hardware/gralloc1.h>

/*============================================================================
 * Class DirectChannel
 *=============================================================================*/
class DirectChannel {
public:
    DirectChannel(const struct native_handle *mem_handle,
        const size_t mem_size);
    DirectChannel(const DirectChannel &copy);
    ~DirectChannel();
    int getLowLatHandle();
    int getClientChannelHandle();
    int getBufferFd();
    bool is_same_memory(const struct native_handle *mem_handle);
    int init();
    int deinit();
private:
    struct native_handle* mem_native_handle;
    void* buffer_ptr;
    size_t buffer_size;
    // int buffer_fd; // This is derivable from mem_native_handle->data[0]
    int sns_low_lat_handle;

    // Assigned by the constructor. If this field is 0, then the object is invalid
    int client_channel_handle;

    // Note: valid range for Android channel handles is (>0, <INT32_MAX)
    static int client_channel_handle_counter;

    // Gralloc1 device singleton
    static gralloc1_device_t* gralloc1_device;
    static GRALLOC1_PFN_RETAIN gralloc1_retain;
    static GRALLOC1_PFN_RELEASE gralloc1_release;
    static GRALLOC1_PFN_LOCK gralloc1_lock;
    static GRALLOC1_PFN_UNLOCK gralloc1_unlock;
    static GRALLOC1_PFN_GET_BACKING_STORE gralloc1_GetBackingStore;

    int initGralloc1Dev(void);
};
