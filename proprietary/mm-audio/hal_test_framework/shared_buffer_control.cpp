/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "shared_buffer_control.h"

namespace qti_hal_test
{
SharedBufferControl::SharedBufferControl() :
    write_ready_{false},
    drain_ready_{false}
{
    LOG_ENTRY;
}

SharedBufferControl::~SharedBufferControl()
{
    LOG_ENTRY;
}

bool SharedBufferControl::WriteReady() const
{
    //LOG_ENTRY;
    return write_ready_;
}

void SharedBufferControl::SetWriteReady()
{
    //LOG_ENTRY;
    write_ready_ = true;
}

void SharedBufferControl::ResetWriteReady()
{
    //LOG_ENTRY;
    write_ready_ = false;
}

bool SharedBufferControl::DrainReady() const
{
    //LOG_ENTRY;
    return drain_ready_;
}

void SharedBufferControl::SetDrainReady()
{
    //LOG_ENTRY;
    drain_ready_ = true;
}

void SharedBufferControl::ResetDrainReady()
{
    //LOG_ENTRY;
    drain_ready_ = false;
}
}