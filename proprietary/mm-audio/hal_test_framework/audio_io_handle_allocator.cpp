/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "audio_io_handle_allocator.h"

namespace qti_hal_test
{
audio_io_handle_t AudioIOHandleAllocator::audio_io_handle_ = 0x999;

std::mutex AudioIOHandleAllocator::allocator_lock_;

AudioIOHandleAllocator::AudioIOHandleAllocator()
{

}

AudioIOHandleAllocator::~AudioIOHandleAllocator()
{

}

audio_io_handle_t AudioIOHandleAllocator::GetHandle()
{
    std::lock_guard<std::mutex> guard(allocator_lock_);
    if (audio_io_handle_ > 0)
    {
        return audio_io_handle_--;
    }
    else
    {
        throw std::runtime_error{ "Cannot allocate any additional audio_io_handle_t handles" };
    }
}
}