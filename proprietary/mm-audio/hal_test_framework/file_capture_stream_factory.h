/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef FILE_CAPTURE_STREAM_FACTORY_H_
#define FILE_CAPTURE_STREAM_FACTORY_H_

#include <cassert>
#include <memory>
#include <stdexcept>

#include "file_capture_stream.h"
#include "file_capture_stream_config.h"
#include "file_reader.h"
#include "sink.h"
#include "wav_writer.h"
#include "xml_config.h"

namespace qti_hal_test
{
class FileCaptureStreamFactory
{
public:
    FileCaptureStreamFactory();
    ~FileCaptureStreamFactory();

    std::unique_ptr<FileCaptureStream> GetInstance(XmlConfig & xml_config);
    std::unique_ptr<FileCaptureStream> GetInstance(int stream_id, XmlConfig & xml_config);

    void Deserialize(XmlConfig & xml_config,
                     std::string & file_name,
                     FileCaptureStreamConfig & file_capture_stream_config);
};
}
#endif // FILE_CAPTURE_STREAM_FACTORY_H_