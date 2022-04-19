/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <gtest/gtest.h>
#include <qtibus/Messenger.h>
#include <QtiBusTransport.h>
#include "QtiBusShmTransport.h"

#include <sys/types.h>
#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <chrono>
#include "MessengerImpl.h"

struct FetchXportPtr{};

template <>
void *MessengerImpl::debugHook<void *, FetchXportPtr>(FetchXportPtr &) {
    return xport;
}
template <class R, class T>
R Messenger::debugHook(T& param) {
    void *ret = nullptr;
    if (impl) {
        ret = impl->debugHook<R,T>(param);
    }
    return ret;
}

std::mutex mtx;
std::condition_variable cv;
pid_t newClientPid;
bool first = true;
std::string receivedMsg;

TEST(QtiShmBus, Creation) {
    QtiBusShmTransport *xport = new QtiBusShmTransport("QtiShmBusTest_shmbus");
    ASSERT_NE(xport, nullptr);

    xport->setOnClientConnectedCb([] (pid_t pid) {
                std::cerr << "New Client Connected. Pid: " << pid << std::endl;
                std::unique_lock<std::mutex> lock(mtx);
                newClientPid = pid;
                cv.notify_all();
            });
    xport->setOnClientDeadCb([] (pid_t pid) {
                std::cerr << "Client dead. Pid: " << pid << std::endl;
            });
    xport->setOnNewMessageCb([] (std::string msg) {
                std::cerr << "New Message <<" << msg << ">>" << std::endl;
                std::unique_lock<std::mutex> lock(mtx);
                receivedMsg = msg;
                cv.notify_all();
            });

    {
        newClientPid = 0;
        std::cerr << "My pid: " << getpid() << std::endl;
        xport->start();
        sleep(1);
        if (first) {
            std::unique_lock<std::mutex> lock(mtx);
            std::cerr << "Waiting for client to connect (Start another instance of this test with the \"-2\" flag" << std::endl;
            cv.wait(lock, 
                    []() { return newClientPid != 0; });
            // bool rc = cv.wait_until(lock, std::chrono::steady_clock::now() + std::chrono::seconds(10),
            //          []() { return newClientPid != 0; });
            // ASSERT_TRUE(rc);
            std::cerr << "Waiting for message from client" << std::endl;
            cv.wait(lock,
                    []() { return receivedMsg.size() > 0; });
        }

        std::string msg = "( " + std::to_string(getpid()) + ") Hello World" ;
        std::cerr << "Sending Message: <<" << msg << ">>" << std::endl;
        ASSERT_TRUE(xport->write(msg));

        if (!first) {
            std::unique_lock<std::mutex> lock(mtx);
            std::cerr << "Waiting for message from client" << std::endl;
            cv.wait(lock,
                    []() { return receivedMsg.size() > 0; });
        }

        sleep(1);
        std::cerr << "Stopping xport" << std::endl;
        xport->stop();
        sleep(1);
        std::cerr << "Ending" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    if (argc >= 2 && !strcmp(argv[1],"-2")) {
        first = false;
    }
    return RUN_ALL_TESTS();
}
