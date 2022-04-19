#include "log_message.h"

namespace qti_hal_test
{
LogMessage::LogMessage(std::chrono::steady_clock::time_point const & initial_time,
    LogMessage::MessageClass message_class,
    std::string && message) :
    LogMessage{ initial_time, -1, message_class, std::move(message) }
{
    stream_id_initialized_ = false;
}

LogMessage::LogMessage(std::chrono::steady_clock::time_point const & initial_time,
                       int stream_id,
                       LogMessage::MessageClass message_class,
                       std::string && message) :
    stream_id_{ stream_id },
    initial_time_{ initial_time },
    time_{ std::chrono::steady_clock::now() },
    message_class_{ message_class },
    message_{ std::move(message) }
{
    stream_id_initialized_ = true;
}

LogMessage::~LogMessage()
{
}

std::string LogMessage::ToString() const // TO DO: manage exceptions or make noexcept
{
    std::chrono::microseconds reporting_time{std::chrono::duration_cast<std::chrono::microseconds>(time_ - initial_time_)};

    std::string message;

    message.append(std::to_string(reporting_time.count()));
    message.append(" ");

    if (stream_id_initialized_)
    {
        message.append(std::to_string(stream_id_));
        message.append(" ");
    }

    message.append(" ");
    message.append(message_lut_.at(message_class_));
    message.append(" ");
    message.append(message_);

    return message;
}
}