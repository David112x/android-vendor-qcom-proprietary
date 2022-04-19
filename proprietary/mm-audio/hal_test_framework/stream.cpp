/**st
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "stream.h"

namespace qti_hal_test
{
Stream::Stream(int stream_id,
               std::unique_ptr<HALStream> hal_stream) :
    stream_id_ { stream_id },
    hal_stream_{ std::move(hal_stream) }
{
    LOG_ENTRY;
}

Stream::~Stream()
{
    // stream_id_ should now be set to indicate invalid over on the main thread's side.
    // All other objects were moved.  Do the destructors for hal_stream_factory_
    // and up_fcs_ get called in both threads???  Maybe during the move???
    LOG_STREAM_ENTRY(stream_id_);
}

Stream::Stream(Stream && rhs) noexcept :
stream_id_{ rhs.stream_id_ },
hal_stream_{ std::move(rhs.hal_stream_) } // THIS IS NOT NECESSARY!  unique_ptrs are move
                                          // only and the move is done implicitly.Moving
{                                         // the(unique) pointer, NOT the
    LOG_STREAM_ENTRY(stream_id_);         // object => FileCaptureStream's move constructor
                                          // will not be called.
    rhs.stream_id_ = -1;
    // TO DO: null out the smart pointer rhs.hal_stream_ after moving.  Is this necessary?
    assert(rhs.hal_stream_.get() == nullptr);
}

void Stream::operator()(std::exception_ptr & stream_exception) // TO DO: call Start
{                                                              // directly as thread funcs
    LOG_STREAM_ENTRY(stream_id_);                              //  callable?

    try
    {
        Start();
    }
    catch (std::exception const &) // TO DO: Confirm the exception specifier is correct.
                                   // Also, note that the variable name is elided here.
    {                              // What is best practice?
        stream_exception = std::current_exception();
    }

    // Exit the thread here
}

void Stream::Start()
{
    LOG_STREAM_ENTRY(stream_id_);

    assert(hal_stream_);
    hal_stream_->Start();
}
}