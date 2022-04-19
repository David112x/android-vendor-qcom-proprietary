/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef PLAYBACK_BUFFERER_H_
#define PLAYBACK_BUFFERER_H_

#include <cassert>
#include <condition_variable>
#include <chrono>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#include "logger.h"
#include "platform_dependencies.h"
#include "shared_buffer_control.h"
#include "source.h"
#include "stream_logger.h"

namespace qti_hal_test
{
class PlaybackBufferer
{
public:
    PlaybackBufferer() = delete;
    PlaybackBufferer(int stream_id,
                     qahw_stream_handle_t * const out_stream_handle,
                     std::shared_ptr<Source> source,
                     bool is_offload,
                     std::chrono::seconds condition_timeout,
                     std::shared_ptr<StreamLogger> stream_logger);

    ~PlaybackBufferer();

    PlaybackBufferer(PlaybackBufferer const & rhs) = delete;
    PlaybackBufferer & operator=(PlaybackBufferer const & rhs) = delete;

    PlaybackBufferer(PlaybackBufferer && rhs) noexcept;
    PlaybackBufferer & operator=(PlaybackBufferer && rhs) noexcept = delete;

    void operator()(std::exception_ptr & stream_exception,
                    std::shared_ptr<SharedBufferControl> shared_buffer_control);

private:
    size_t GetBufferSize() const;

    void BufferSamples(std::shared_ptr<SharedBufferControl> shared_buffer_control);

    void Write(std::shared_ptr<SharedBufferControl> shared_buffer_control,
               char * const buffer,
               uint64_t num_bytes_to_write,
               uint64_t & bytes_written);

    void Drain(std::shared_ptr<SharedBufferControl> shared_buffer_control);

    void Standby();

    int stream_id_{ -1 };

    qahw_stream_handle_t *  out_stream_handle_;

    std::shared_ptr<Source> source_;

    bool                    is_offload_;

    std::unique_ptr<char[]> buffer_;
    size_t                  buffer_size_;

    std::chrono::seconds    condition_timeout_;

    std::shared_ptr<StreamLogger> stream_logger_;
};
}

#endif // PLAYBACK_BUFFERER_H_