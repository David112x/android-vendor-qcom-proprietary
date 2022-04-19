
/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "playback_bufferer.h"

namespace qti_hal_test
{
PlaybackBufferer::PlaybackBufferer(int stream_id,
                                   qahw_stream_handle_t * const out_stream_handle,
                                   std::shared_ptr<Source> source,
                                   bool is_offload,
                                   std::chrono::seconds condition_timeout,
                                   std::shared_ptr<StreamLogger> stream_logger):
    stream_id_{ stream_id },
    out_stream_handle_{ out_stream_handle },
    source_{ source },
    is_offload_{ is_offload },
    buffer_{nullptr},
    buffer_size_{0},
    condition_timeout_{ condition_timeout },
    stream_logger_{ stream_logger }
{
    LOG_STREAM_ENTRY(stream_id_);

    assert(stream_logger_);
}

PlaybackBufferer::~PlaybackBufferer()
{
    LOG_STREAM_ENTRY(stream_id_);
}

PlaybackBufferer::PlaybackBufferer(PlaybackBufferer && rhs) noexcept :
    stream_id_{ rhs.stream_id_ },
    out_stream_handle_{ rhs.out_stream_handle_ },
    source_{ std::move(rhs.source_) }, // std::move is faster
    is_offload_{ rhs.is_offload_ },
    buffer_{ std::move(rhs.buffer_) },
    buffer_size_{ rhs.buffer_size_ },
    condition_timeout_{ rhs.condition_timeout_ },
    stream_logger_{ std::move(rhs.stream_logger_) } // std::move is faster
{
    rhs.stream_id_ = -1;
    rhs.out_stream_handle_ = nullptr;

    rhs.source_ = nullptr;

    rhs.is_offload_ = false;

    rhs.buffer_ = nullptr;
    rhs.buffer_size_ = 0;

    rhs.condition_timeout_ = std::chrono::seconds{ 0 };

    rhs.stream_logger_ = nullptr;
}

void PlaybackBufferer::operator()(std::exception_ptr & stream_exception,
                                  std::shared_ptr<SharedBufferControl> shared_buffer_control)
{
    LOG_STREAM_ENTRY(stream_id_);

    try
    {
        buffer_size_ = GetBufferSize();
        buffer_ = std::make_unique<char[]>(buffer_size_); // TO DO: Create custom deleter
                                                          // and use make_shared.
        BufferSamples(shared_buffer_control);
    }
    catch (std::exception const &) // TO DO: Confirm the exception specifier is correct.
    {                              // Also, note that the variable name is elided here.
                                   // What is best practice?
        stream_exception = std::current_exception();
    }
}

size_t PlaybackBufferer::GetBufferSize() const
{
    LOG_STREAM_ENTRY(stream_id_);

    stream_logger_->API("qahw_out_get_buffer_size");
    size_t buffer_size = qahw_out_get_buffer_size(out_stream_handle_);

    if (buffer_size == 0)
    {
        std::ostringstream error_msg;
        error_msg << "qahw_out_get_buffer_size failed, returned buffer size " << buffer_size;
        stream_logger_->Error(error_msg.str());
        throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
    }

    std::ostringstream buffer_size_msg;
    buffer_size_msg << "qahw_out_get_buffer_size returned buffer size " << buffer_size;
    stream_logger_->Info(buffer_size_msg.str());

    return buffer_size;
}

void PlaybackBufferer::BufferSamples(std::shared_ptr<SharedBufferControl> shared_buffer_control)
{
    LOG_STREAM_ENTRY(stream_id_);

    assert(out_stream_handle_);
    assert(source_);
    assert(buffer_.get());
    assert(buffer_size_ > 0);

    uint64_t offset{ 0 };
    uint64_t write_length{ 0 };
    uint64_t bytes_remaining{ 0 };
    uint64_t bytes_written{ 0 };
    uint64_t bytes_read{ 0 };

    uint64_t total_bytes_read{ 0 };
    uint64_t total_bytes_written{ 0 };

    uint64_t iterations{ 0 };
    uint64_t block_count{ 32 };

    bool done = false;

    while (!done)   // TO DO: Abstract to Fill-buffer Drain-buffer
    {
        if (bytes_remaining == 0)
        {
            bool eof = false;
            // TO DO: Does Read throw?
            source_->Read(buffer_.get(), // The way Read is designed and the way it interacts with
                          buffer_size_,  // the logic here causes it to, say, return 380 < 1280
                          bytes_read,    // if sees bytes_read > 0 it does not check eof on this
                          eof); // requested bytes, and set eof to true but since the nextiteration.
            if (bytes_read == 0)
            {
                if (eof)
                {
                    stream_logger_->Info("end of file");
                    if (is_offload_)
                    {
                        Drain(shared_buffer_control);
                    }
                }

                done = true;

                break;
            }

            total_bytes_read += bytes_read;

#define BLOCK_REPORTING
#ifdef BLOCK_REPORTING
            if (iterations % block_count == 0)
            {
#endif
                stream_logger_->Info("total_bytes_read: " + std::to_string(total_bytes_read));
#ifdef BLOCK_REPORTING
            }
#endif
            bytes_remaining = write_length = bytes_read;
        }

        offset = write_length - bytes_remaining;
        Write(shared_buffer_control,
              buffer_.get() + offset,
              bytes_remaining,
              bytes_written);

        total_bytes_written += bytes_written;

        bytes_remaining -= bytes_written;


#ifdef BLOCK_REPORTING
        if (iterations % block_count == 0)
        {
#endif
            stream_logger_->Info("total_bytes_written: " + std::to_string(total_bytes_written));
#ifdef BLOCK_REPORTING
        }
#endif
        iterations++;
    }

    if (total_bytes_read != total_bytes_written)
    {
        std::ostringstream error_msg;
        error_msg << " total_bytes_written: " << total_bytes_written
                  << " != "
                  << " total_bytes_read: " << total_bytes_read;
        stream_logger_->Error(error_msg.str());
        throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
    }
    else
    {
        std::ostringstream total_bytes_msg;
        total_bytes_msg << " total_bytes_written: " << total_bytes_written
            << ", "
            << " total_bytes_read: " << total_bytes_read;
        stream_logger_->Info(total_bytes_msg.str());
    }

    Standby();
}

void PlaybackBufferer::Write(std::shared_ptr<SharedBufferControl> shared_buffer_control,
                             char * const buffer,
                             uint64_t num_bytes_to_write,
                             uint64_t & bytes_written)
{
    //LOG_STREAM_ENTRY(stream_id_);

    assert(shared_buffer_control);

    qahw_out_buffer_t out_buf;
    memset(&out_buf, 0, sizeof(qahw_out_buffer_t)); // TO DO: Evaluate --
                                                        // strictly necessary?

    out_buf.buffer = buffer;
    // TO DO: The Write interface uses wide ints but the HAL API uses size_t: analyze.
    out_buf.bytes = static_cast<size_t>(num_bytes_to_write);

    ssize_t write_status = 0;
    bool write_lock_status = false;

    {
        std::unique_lock<std::timed_mutex> write_lock(shared_buffer_control->write_mutex_,
                                                      std::defer_lock);

        write_lock_status = write_lock.try_lock_for(condition_timeout_);
        //stream_logger_->API("qahw_out_write");
        write_status = qahw_out_write(out_stream_handle_, &out_buf);
    } // Exiting this block will call write_lock.unlock() via write_lock's destructor.

    if (!write_lock_status)
    {
        LogMessage message = stream_logger_->ErrorLogMessage("write_lock timed out");
        stream_logger_->Log(message);
        throw std::runtime_error{ message.ToString() };
    }

    if (write_status < 0)
    {
        std::ostringstream error_msg;
        error_msg << "qahw_out_write failed, returned " << write_status;
        stream_logger_->Error(error_msg.str());
        throw std::runtime_error(stream_logger_->ErrorLogMessage(error_msg.str()).ToString());
    }

    if (static_cast<uint64_t>(write_status) != num_bytes_to_write)
    {
        std::unique_lock<std::mutex> write_wait_lock(shared_buffer_control->write_wait_mutex_);
        bool write_wait_status = shared_buffer_control->write_condition_.wait_for(write_wait_lock,
                                         condition_timeout_,
                                         [=] { return shared_buffer_control->write_ready_; });
        if (write_wait_status)
        {
            shared_buffer_control->ResetWriteReady();
        }
        else
        {
            // TO DO: better to call copy ctor directly?
            LogMessage message = stream_logger_->ErrorLogMessage("timed out waiting for" \
                                                                 " write_ready_ to be signaled");
            stream_logger_->Log(message);
            throw std::runtime_error{ message.ToString() };
        }
    }

    // Previous branch catches all negative values of status; Need to investigate further.
    bytes_written = static_cast<size_t>(write_status);
}

void PlaybackBufferer::Drain(std::shared_ptr<SharedBufferControl> shared_buffer_control)
{
    LOG_STREAM_ENTRY(stream_id_);

    int drain_status = 0;
    bool drain_lock_status = false;

    {
        std::unique_lock<std::timed_mutex> drain_lock(shared_buffer_control->drain_mutex_,
                                                      std::defer_lock);

        drain_lock_status = drain_lock.try_lock_for(condition_timeout_);
        stream_logger_->API("qahw_out_drain");
        drain_status = qahw_out_drain(out_stream_handle_, QAHW_DRAIN_ALL);
    } // Exiting this block will call drain_lock.unlock() via drain_lock's destructor.

    if (!drain_lock_status)
    {
        // Better to call copy ctor directly?
        LogMessage message = stream_logger_->ErrorLogMessage("drain_lock timed out");
        stream_logger_->Log(message);
        throw std::runtime_error{ message.ToString() };
    }

    if (drain_status != 0)
    {
        std::ostringstream error_msg;
        error_msg << "qahw_out_drain failed, returned " << drain_status;
        stream_logger_->Error(error_msg.str());
        throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
    }

    std::unique_lock<std::mutex> drain_wait_lock(shared_buffer_control->drain_wait_mutex_);
    bool drain_wait_status = shared_buffer_control->drain_condition_.wait_for(drain_wait_lock,
                                     condition_timeout_,
                                     [=] { return shared_buffer_control->drain_ready_; });
    if (drain_wait_status)
    {
        shared_buffer_control->ResetDrainReady();
    }
    else
    {
        // Better to call copy ctor directly?
        LogMessage message = stream_logger_->ErrorLogMessage("timed out waiting for" \
                                                             " drain_ready_ to be signaled");
        stream_logger_->Log(message);
        throw std::runtime_error{ message.ToString() };
    }
}

void PlaybackBufferer::Standby()
{
    LOG_STREAM_ENTRY(stream_id_);

    // TO DO: Call standby in non-offload mode?
    stream_logger_->API("qahw_out_standby");
    int status = qahw_out_standby(out_stream_handle_);

    if (status != 0)
    {
        std::ostringstream error_msg;
        error_msg << "qahw_out_standby failed, returned " << status;
        stream_logger_->Error(error_msg.str());
        throw std::runtime_error{ stream_logger_->ErrorLogMessage(error_msg.str()).ToString() };
    }
}
}
