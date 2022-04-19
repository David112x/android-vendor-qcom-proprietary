#include "stream_logger.h"

namespace qti_hal_test
{
StreamLogger::StreamLogger(int stream_id):
    stream_id_{ stream_id },
    initial_time_{ std::chrono::steady_clock::now() },
    log_writer_{ std::make_shared<LogWriter>() }
{
    LOG_STREAM_ENTRY(stream_id_);
}

StreamLogger::StreamLogger(int stream_id, std::shared_ptr<LogWriter> log_writer) :
    stream_id_{ stream_id },
    initial_time_{ std::chrono::steady_clock::now() },
    log_writer_{ log_writer }
{
    LOG_STREAM_ENTRY(stream_id_);
}

StreamLogger::~StreamLogger()
{
    LOG_STREAM_ENTRY(stream_id_);
}

void StreamLogger::API(std::string && api_name) const
{
#ifdef USE_THREAD_SAFE_QUEUE
    messages_.Push(LogMessage(initial_time_, stream_id_, LogMessage::MessageClass::API, std::move(api_name)));
    log_writer_->Write(messages_.TryPop());
#else
    messages_.emplace(initial_time_, stream_id_, LogMessage::MessageClass::API, std::move(api_name));
#endif
}

void StreamLogger::Enter(std::string && name) const
{
#ifdef USE_THREAD_SAFE_QUEUE
    messages_.Push(LogMessage(initial_time_, stream_id_, LogMessage::MessageClass::Enter, std::move(name)));
    log_writer_->Write(messages_.TryPop());
#else
    messages_.emplace(initial_time_, stream_id_, LogMessage::MessageClass::Enter, std::move(name));
#endif
}

void StreamLogger::Exit(std::string && name) const
{
#ifdef USE_THREAD_SAFE_QUEUE
    messages_.Push(LogMessage(initial_time_, stream_id_, LogMessage::MessageClass::Exit, std::move(name)));
    log_writer_->Write(messages_.TryPop());
#else
    messages_.emplace(initial_time_, stream_id_, LogMessage::MessageClass::Exit, std::move(name));
#endif
}

void StreamLogger::Error(std::string && message) const
{
#ifdef USE_THREAD_SAFE_QUEUE
    messages_.Push(LogMessage(initial_time_, stream_id_, LogMessage::MessageClass::Error, std::move(message)));
    log_writer_->Write(messages_.TryPop());
#else
    messages_.emplace(initial_time_, stream_id_, LogMessage::MessageClass::Error, std::move(message));
#endif
}

void StreamLogger::Warning(std::string && message) const
{
#ifdef USE_THREAD_SAFE_QUEUE
    messages_.Push(LogMessage(initial_time_, stream_id_, LogMessage::MessageClass::Warning, std::move(message)));
    log_writer_->Write(messages_.TryPop());
#else
    messages_.emplace(initial_time_, stream_id_, LogMessage::MessageClass::Warning, std::move(message));
#endif
}

void StreamLogger::Info(std::string && message) const
{
#ifdef USE_THREAD_SAFE_QUEUE
    messages_.Push(LogMessage(initial_time_, stream_id_, LogMessage::MessageClass::Info, std::move(message)));
    log_writer_->Write(messages_.TryPop());
#else
    messages_.emplace(initial_time_, stream_id_, LogMessage::MessageClass::Info, std::move(message));
#endif
}

void StreamLogger::FrameworkError(std::string && message) const
{
#ifdef USE_THREAD_SAFE_QUEUE
    messages_.Push(LogMessage(initial_time_, stream_id_, LogMessage::MessageClass::FrameworkError, std::move(message)));
    log_writer_->Write(messages_.TryPop());
#else
    messages_.emplace(initial_time_, stream_id_, LogMessage::MessageClass::FrameworkError, std::move(message));
#endif
}

void StreamLogger::FrameworkWarning(std::string && message) const
{
#ifdef USE_THREAD_SAFE_QUEUE
    messages_.Push(LogMessage(initial_time_, stream_id_, LogMessage::MessageClass::FrameworkWarning, std::move(message)));
    log_writer_->Write(messages_.TryPop());
#else
    messages_.emplace(initial_time_, stream_id_, LogMessage::MessageClass::FrameworkWarning, std::move(message));
#endif
}

void StreamLogger::FrameworkInfo(std::string && message) const
{
#ifdef USE_THREAD_SAFE_QUEUE
    messages_.Push(LogMessage(initial_time_, stream_id_, LogMessage::MessageClass::FrameworkInfo, std::move(message)));
    log_writer_->Write(messages_.TryPop());
#else
    messages_.emplace(initial_time_, stream_id_, LogMessage::MessageClass::FrameworkInfo, std::move(message));
#endif
}

LogMessage StreamLogger::ErrorLogMessage(std::string && message) const
{
    return LogMessage(initial_time_, stream_id_, LogMessage::MessageClass::Error, std::move(message));
}

void StreamLogger::Log(LogMessage message) const
{
#ifdef USE_THREAD_SAFE_QUEUE
    messages_.Push(message);
    log_writer_->Write(messages_.TryPop());
#else
    messages_.push(message);
#endif
}

}