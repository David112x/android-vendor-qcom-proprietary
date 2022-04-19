/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "session_factory.h"

namespace qti_hal_test
{
SessionFactory::SessionFactory()
{
    LOG_ENTRY;
}

SessionFactory::~SessionFactory()
{
    LOG_ENTRY;
}

std::unique_ptr<Session> SessionFactory::GetInstance(XmlConfig & xml_config)
{
    LOG_ENTRY;

    std::unique_ptr<Session> session = std::make_unique<Session>();

    xml_config.MoveToElement("Session");

    int stream_id = 0;

    xml_config.MoveToElement("Streams"); // Validation only
    xml_config.MoveToElement("Stream");  // Priming move

    std::unique_ptr<Stream> stream;
    std::unique_ptr<HALStream> hal_stream;

    while (xml_config.IsMatch("Stream"))
    {
        xml_config.MoveToNextElement(); // Advance to the next concrete stream type

        if (xml_config.IsMatch("FileCaptureStream")) // TO DO: Consolidate the two calls on
        {                                            // xml_config.
             FileCaptureStreamFactory stream_factory;
             hal_stream = stream_factory.GetInstance(stream_id, xml_config);
        }
        else if (xml_config.IsMatch("FilePlaybackStream"))
        {
            FilePlaybackStreamFactory stream_factory;

#if 1
            hal_stream = stream_factory.GetInstance(stream_id,
                                                    session->GetSharedBufferControl(),
                                                    xml_config);
#else
            hal_stream = stream_factory.GetInstance(stream_id,
                                                    xml_config);
#endif
        }
#if 0
        else if (xml_config.IsMatch("FilePlaybackStreamMock"))
        {
            FilePlaybackStreamConfig file_playback_stream_config;

            xml_config.MoveToElement("FileName");

            file_playback_stream_config.Deserialize(xml_config);

            hal_stream = std::make_unique<FilePlaybackStreamMock>(stream_id,
                                                                  file_playback_stream_config,
                                                                  nullptr,
                                                                  nullptr,
                                                                  session->GetSharedBufferControl());
        }
        else if (xml_config.IsMatch("FilePlaybackStreamMock2"))
        {
            FilePlaybackStreamConfig file_playback_stream_config;

            xml_config.MoveToElement("FileName");

            file_playback_stream_config.Deserialize(xml_config);

            hal_stream = std::make_unique<FilePlaybackStreamMock2>(stream_id,
                                                                   file_playback_stream_config,
                                                                   nullptr,
                                                                   nullptr,
                                                                   session->GetSharedBufferControl());
        }
#endif
        else // else block is here because even though IsMatch will throw, we want to
        {    // modify the error to provide additional context about the Test name.
            std::ostringstream error_msg;
            // TO DO: add the row-col error from IsMatch to  the error message.
            error_msg << "Stream type " << xml_config.GetName() << " not found ";
            throw std::runtime_error{ error_msg.str() };
        }

        stream = std::make_unique<Stream>(stream_id, std::move(hal_stream));

        session->AddStream(std::move(stream));

        stream_id++;

        if (xml_config.GetName() != "Stream")
        {
            xml_config.MoveToNextElement(); // Move cursor to the next Stream
        }
    }

    return session;
}
}
