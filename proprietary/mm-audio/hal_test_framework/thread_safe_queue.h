/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

// This code was adapted from "C++ Concurrency in Action, Practical Multithreading" by Anthony Williams
// source code available under BSL - 1.0.
// https://manning-content.s3.amazonaws.com/download/0/78f6c43-a41b-4eb0-82f2-44c24eba51ad/CCiA_SourceCode.zip

#ifndef THREAD_SAFE_QUEUE_H_
#define THREAD_SAFE_QUEUE_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

namespace qti_hal_test
{
template<typename T>
class ThreadSafeQueue
{
public:

    ThreadSafeQueue() {};

    void WaitAndPop(T& value)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        data_condition_.wait(lock, [this] { return !data_queue_.empty(); });
        value = std::move(*data_queue_.front());
        data_queue_.pop();
    }

    bool TryPop(T& value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (data_queue_.empty())
        {
            return false;
        }

        value = std::move(*data_queue_.front());
        return true;
    }

    std::shared_ptr<T> WaitAndPop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        data_condition_.wait(lock, [this] { return !data_queue_.empty(); });

        std::shared_ptr<T> data = data_queue_.front();

        data_queue_.pop();
        return data;
    }

    std::shared_ptr<T> TryPop()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (data_queue_.empty())
        {
            return std::shared_ptr<T>();
        }

        std::shared_ptr<T> data = data_queue_.front();
        data_queue_.pop();

        return data;
    }

    void Push(T value)
    {
        std::shared_ptr<T> data(std::make_shared<T>(std::move(value)));
        std::lock_guard<std::mutex> lock(mutex_);

        data_queue_.push(data);
        data_condition_.notify_one(); // TO DO: Check to see if notify_all() performs better.
    }

    bool Empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return data_queue_.empty();
    }

#if 1
    ThreadSafeQueue(ThreadSafeQueue const & rhs) = delete;
    ThreadSafeQueue & operator=(ThreadSafeQueue const & rhs) = delete;

    ThreadSafeQueue(ThreadSafeQueue && rhs) noexcept = delete;
    ThreadSafeQueue & operator=(ThreadSafeQueue && rhs) noexcept = delete;
#endif

private:
    mutable std::mutex mutex_;
    std::queue<std::shared_ptr<T>> data_queue_;
    std::condition_variable data_condition_;
};
}
#endif // THREAD_SAFE_QUEUE_H_