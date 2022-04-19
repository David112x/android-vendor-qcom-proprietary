/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <functional>
#include <string>
#include <log/log.h>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>
#include <iomanip>
#include <condition_variable>
#include <QtiMutex.h>

#ifndef QMI_RIL_UTF
#include <Logger.h>
using namespace QcSettingsD;
#endif

#ifndef __ANDROID__
static inline int gettid() {
    return syscall(SYS_gettid);
}
#endif

#ifdef __ANDROID__
#define DEBUG_LOG(...) (({ \
    std::stringstream os; \
    os __VA_ARGS__; \
    std::string msg = os.str();\
    QCRIL_HAL_LOG_DEBUG("%s", msg.c_str());\
}))
#else
extern thread_local char timebuffer[20];
#define DEBUG_LOG(...) (({ \
    std::stringstream os; \
    timespec thetime = {}; \
    clock_gettime(CLOCK_REALTIME, &thetime); \
    struct tm brokendown; \
    localtime_r(&thetime.tv_sec, &brokendown); \
    strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", &brokendown); \
    os << timebuffer << "." << std::setw(6) << std::setfill('0') << (thetime.tv_nsec / 1000) \
            << " [" << getpid() << "," << gettid() << "]" \
            << " QtiBus: (" << basename((char *)__FILE__) << ", " << __func__ << "): "; \
    os __VA_ARGS__ << std::endl; \
    std::cerr << os.str(); \
    (void)0; \
}))
#endif

class QtiBusTransport;

class QtiBusTransportGroup
{
    public:
        virtual ~QtiBusTransportGroup() {}
        enum class TransportEvt {
            CONNECTED,
            DISCONNECTED,
            READ_READY,
            WRITE_READY,
            ERROR
        };
        using TransportCb = std::function<void(QtiBusTransport &, TransportEvt)>;
        QtiBusTransportGroup(TransportCb &cb);
        void add(QtiBusTransport &transport);
};

class QtiBusTransport
{
    private:
        bool started = false;
        qtimutex::QtiSharedMutex mutex;
        std::condition_variable_any cv;

    public:
        virtual ~QtiBusTransport() {}
        virtual bool write(const std::string &buffer) = 0;
        virtual std::string receive() = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual bool getStarted() {
            return started;
        }

        void waitUntilStarted() {
            std::unique_lock<qtimutex::QtiSharedMutex> lock(mutex);

            if (started) {
                return;
            }
            cv.wait(lock, [this]() {return started;});
        }

        using OnClientConnectedCb = std::function<void(pid_t)>;
        void setOnClientConnectedCb(OnClientConnectedCb cb) {
            onClientConnected = cb;
        }
        using OnClientDeadCb = std::function<void(pid_t)>;
        void setOnClientDeadCb(OnClientDeadCb cb) {
            onClientDead = cb;
        }
        using OnNewMessageCb = std::function<void(std::string)>;
        void setOnNewMessageCb(OnNewMessageCb cb) {
            onNewMessage = cb;
        }

    protected:
        OnClientConnectedCb onClientConnected;
        OnClientDeadCb onClientDead;
        OnNewMessageCb onNewMessage;
        virtual void setStarted(bool started) {
            std::unique_lock<qtimutex::QtiSharedMutex> lock(mutex);

            this->started = started;

            cv.notify_all();
        }
};


