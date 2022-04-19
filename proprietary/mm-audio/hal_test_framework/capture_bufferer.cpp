/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "capture_bufferer.h"

namespace qti_hal_test
{
CaptureBufferer::CaptureBufferer(int stream_id,
                                 qahw_stream_handle_t * const in_stream_handle,
                                 std::shared_ptr<Sink> sink,
                                 uint32_t duration,
                                 std::chrono::seconds condition_timeout,
                                 std::shared_ptr<StreamLogger> stream_logger) :
    stream_id_{ stream_id },
    in_stream_handle_{ in_stream_handle },
    sink_{ sink },
    duration_{ duration },
    buffer_{ nullptr },
    buffer_size_{ 0 },
    condition_timeout_{ condition_timeout },
    stream_logger_{ stream_logger }
{
    LOG_STREAM_ENTRY(stream_id_);
}

CaptureBufferer::~CaptureBufferer()
{
    LOG_STREAM_ENTRY(stream_id_);
}

CaptureBufferer::CaptureBufferer(CaptureBufferer && rhs) noexcept :
    stream_id_{ rhs.stream_id_ },
    in_stream_handle_{ rhs.in_stream_handle_ },
    sink_{ std::move(rhs.sink_) },
    duration_{ rhs.duration_ },
    buffer_{ std::move(rhs.buffer_) },
    buffer_size_{ rhs.buffer_size_ },
    condition_timeout_ {rhs.condition_timeout_},
    stream_logger_{ std::move(rhs.stream_logger_) } // std::move is faster
{
    LOG_STREAM_ENTRY(stream_id_);

    rhs.stream_id_ = -1;

    rhs.in_stream_handle_ = nullptr;

    rhs.sink_ = nullptr;  // Since we have to copy the Sink pointer (which is shared at this point) do not null here.

    rhs.duration_ = 0;

    rhs.buffer_ = nullptr;
    rhs.buffer_size_ = 0;

    rhs.condition_timeout_ = std::chrono::seconds{ 0 };

    rhs.stream_logger_ = nullptr;
}

void CaptureBufferer::operator()(std::exception_ptr & stream_exception)
{
    LOG_STREAM_ENTRY(stream_id_);

    try
    {
        buffer_size_ = GetBufferSize();
        buffer_ = std::make_unique<char[]>(buffer_size_);

        BufferSamples();
    }
    catch (std::exception const &) // TO DO: Confirm the exception specifier is correct.  Also, note that the variable name is elided here. What is best practice?
    {
        stream_exception = std::current_exception();
    }
}

void CaptureBufferer::BufferSamples()
{
    LOG_STREAM_ENTRY(stream_id_);

    assert(in_stream_handle_);
    assert(sink_);
    assert(buffer_.get());
    assert(buffer_size_ > 0);

    //LOG_STREAM_API(stream_id_, "qahw_in_set_parameters");
    //qahw_in_set_parameters(input_stream_handle_, param);  // TO DO: Implement

    qahw_in_buffer_t in_buffer;
    memset(&in_buffer, 0, sizeof(qahw_in_buffer_t));

    std::chrono::duration<double> capture_duration(static_cast<double>(duration_));

    std::chrono::steady_clock::time_point begin_capture = std::chrono::steady_clock::now();

    while (std::chrono::steady_clock::now() < begin_capture + capture_duration)   // TO DO: Abstract to Fill-buffer Drain-buffer
    {

        in_buffer.buffer = buffer_.get();
        in_buffer.bytes = buffer_size_;

        stream_logger_->API("qahw_in_read");
        ssize_t status = qahw_in_read(in_stream_handle_, &in_buffer);
        if (status < 0)
        {
            std::ostringstream error_msg;
            error_msg << "qahw_in_read failed, returned " << status;
            stream_logger_->Error(error_msg.str());
            throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
        }

        sink_->Write(static_cast<const char * const>(in_buffer.buffer), in_buffer.bytes);  // TO DO: Why not buffer_ and buffer_size_?
    }

    std::chrono::steady_clock::time_point end_capture = std::chrono::steady_clock::now();

    std::ostringstream capture_duration_msg;
    capture_duration_msg << "Capture duration: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end_capture - begin_capture).count()
              << " milliseconds" <<std::endl;

    stream_logger_->FrameworkInfo(capture_duration_msg.str());
}

size_t CaptureBufferer::GetBufferSize() const
{
    LOG_STREAM_ENTRY(stream_id_);

    stream_logger_->API("qahw_in_get_buffer_size");
    size_t buffer_size = qahw_in_get_buffer_size(in_stream_handle_);

    if (buffer_size == 0)
    {
        std::ostringstream error_msg;
        error_msg << "qahw_in_get_buffer_size failed, returned buffer size " << buffer_size;
        stream_logger_->Error(error_msg.str());
        throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
    }

    return buffer_size;
}
}

