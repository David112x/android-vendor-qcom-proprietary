/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef  AUDIO_IO_HANDLE_ALLOCATOR_H_
#define  AUDIO_IO_HANDLE_ALLOCATOR_H_

#include <cassert>
#include <iostream>
#include <mutex>
#include <stdexcept>

#include "platform_dependencies.h"

namespace qti_hal_test
{
class AudioIOHandleAllocator
{
public:
    AudioIOHandleAllocator();
    ~AudioIOHandleAllocator();

    AudioIOHandleAllocator(AudioIOHandleAllocator & rhs) = delete;
    void operator=(AudioIOHandleAllocator & rhs) = delete;

    static audio_io_handle_t GetHandle();

private:
    static audio_io_handle_t audio_io_handle_;

    static std::mutex allocator_lock_;
};
}
#endif // AUDIO_IO_HANDLE_ALLOCATOR_H_