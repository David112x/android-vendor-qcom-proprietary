/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "file_capture_stream_factory.h"

namespace qti_hal_test
{
FileCaptureStreamFactory::FileCaptureStreamFactory()
{
    LOG_ENTRY;
}

FileCaptureStreamFactory::~FileCaptureStreamFactory()
{
    LOG_ENTRY;
}

std::unique_ptr<FileCaptureStream> FileCaptureStreamFactory::GetInstance(XmlConfig & xml_config)
{
    LOG_ENTRY;
    return GetInstance(0, xml_config);  // TO DO: Rethink -- better way to initialize stream id?
}

std::unique_ptr<FileCaptureStream> FileCaptureStreamFactory::GetInstance(int stream_id, XmlConfig & xml_config)
{
    LOG_STREAM_ENTRY(stream_id);
    std::string file_name;

    FileCaptureStreamConfig file_capture_stream_config;

    Deserialize(xml_config, file_name, file_capture_stream_config);

    std::shared_ptr<Sink> sink; // TO DO: Use shared pointer until you can compile with GCC version > 5.0

    sink = std::make_shared<WavWriter>(file_name,
                            file_capture_stream_config.sample_rate_,
                            file_capture_stream_config.channels_,
                            file_capture_stream_config.bit_width_);

    return std::make_unique<FileCaptureStream>(stream_id,
                                               file_capture_stream_config,
                                               sink);    // FileCaptureStream takes ownership
}

void FileCaptureStreamFactory::Deserialize(XmlConfig & xml_config,
                                           std::string & file_name,
                                           FileCaptureStreamConfig & file_capture_stream_config)
{
    LOG_ENTRY;

    xml_config.MoveToElement("FileCaptureStream"); // Moving here will cause an exception to be thrown
                                                      // if we're looking for the wrong "FileName" element.
    file_name = xml_config.GetElementText("FileName");

    file_capture_stream_config.Deserialize(xml_config);
}
}