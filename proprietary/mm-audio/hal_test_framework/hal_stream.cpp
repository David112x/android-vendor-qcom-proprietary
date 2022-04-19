/**st
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "hal_stream.h"

namespace qti_hal_test
{
HALStream::HALStream(int stream_id) :
    stream_id_{ stream_id },
    stream_logger_{ std::make_shared<StreamLogger>(stream_id_, std::make_shared<LogWriter>())
}   // TO DO: Verify correct behavior of initialization
{
    LOG_STREAM_ENTRY(stream_id_);
}

HALStream::~HALStream()
{
    LOG_STREAM_ENTRY(stream_id_);
}
}