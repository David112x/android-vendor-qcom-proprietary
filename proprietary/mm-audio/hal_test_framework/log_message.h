/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef LOG_MESSAGE_H_
#define LOG_MESSAGE_H_

#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <thread>

namespace qti_hal_test
{
class LogMessage
{
public:
    enum class MessageClass { API,
                              Enter,
                              Exit,
                              Error,
                              Warning,
                              Info,
                              FrameworkError,
                              FrameworkWarning,
                              FrameworkInfo};

    LogMessage() {};
    LogMessage(std::chrono::steady_clock::time_point const & initial_time,
               LogMessage::MessageClass message_class,
               std::string && message); // TO DO: Should you really pass by rvalure reference?
                                        // This seems to work: the address of the argument in the
                                        // constructor call is the same as message, e.g.:

                                        // api_name[0]	113 'q'	char
                                        // &api_name[0]	0x00a3f478 "qahw_open_output_stream"
                                        // message_[0]	113 'q'	char
                                        // &message_[0]	0x00a3f478 "qahw_open_output_stream"
                                        // message[0]	113 'q'	char
                                        // &message[0]	0x00a3f478 "qahw_open_output_stream"
                                        // message_[0]	113 'q'	char

    LogMessage(std::chrono::steady_clock::time_point const & initial_time,
               int stream_id,
               LogMessage::MessageClass message_class,
               std::string && message);  // TO DO: Should you really pass by rvalure reference?

    ~LogMessage();

    LogMessage(LogMessage const & rhs) = default; // StreamLogger::ErrorLogMessage requires this.
    LogMessage & operator=(LogMessage const & rhs) = delete;

    LogMessage(LogMessage && rhs) noexcept = default; // StreamLogger::ErrorLogMessage requires this.
    LogMessage & operator=(LogMessage && rhs) noexcept = delete;

    std::string ToString() const;

private:
    int stream_id_;
    bool stream_id_initialized_;

    std::chrono::steady_clock::time_point initial_time_;
    std::chrono::steady_clock::time_point time_;

    MessageClass message_class_;
    std::string message_;

    const std::map<MessageClass, std::string> message_lut_
    {
        { MessageClass::API,              std::string{ "API             "} },
        { MessageClass::Enter,            std::string{ "Enter           "} },
        { MessageClass::Exit,             std::string{ "Exit            "} },
        { MessageClass::Error,            std::string{ "Error           "} },
        { MessageClass::Warning,          std::string{ "Warning         "} },
        { MessageClass::Info,             std::string{ "Info            "} },
        { MessageClass::FrameworkError,   std::string{ "FrameworkError  "} },
        { MessageClass::FrameworkWarning, std::string{ "FrameworkWarning"} },
        { MessageClass::FrameworkInfo,    std::string{ "FrameworkInfo   "} }
    };
};
}
#endif // LOG_MESSAGE_H_