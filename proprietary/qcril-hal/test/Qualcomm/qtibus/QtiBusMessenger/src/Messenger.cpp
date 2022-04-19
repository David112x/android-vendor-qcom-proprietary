/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <gtest/gtest.h>
#include <qtibus/Messenger.h>
#include <qtibus/QtiBusTransportServer.h>
#include <QtiBusTransport.h>
#include "QtiBusShmTransport.h"

#include <sys/types.h>
#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <chrono>
#include "MessengerImpl.h"
#include <framework/add_message_id.h>
#include <qtibus/IPCMessage.h>

#define TAG "QtiBus"

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
bool receiver = false;
pid_t lastPid;
size_t remoteClients = 0;
message_id_t remoteRegistered = nullptr;

class TestMessage: public IPCMessage,
                    public add_message_id<TestMessage>
{
    public:
        static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.test.test_message";

    private:
    public:
        string dump() {
            return mName + "{}";
        }

        TestMessage():
            IPCMessage(get_class_message_id())
        {
            mName = MESSAGE_NAME;
        }

        std::shared_ptr<UnSolicitedMessage> clone() {
            return std::make_shared<TestMessage>();
        }

        void serialize(IPCOStream &os) {
        }

        void deserialize(IPCIStream &is) {
        }

        inline ~TestMessage() {
        }
};

class TestMessage2: public IPCMessage,
                    public add_message_id<TestMessage2>
{
    public:
        static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.test.test_message2";

    private:
        int intValue;
        std::string stringValue;

    public:
        string dump() {
            return mName + "{}";
        }

        TestMessage2():
            IPCMessage(get_class_message_id())
        {}

        TestMessage2(int intValue, std::string stringValue):
            IPCMessage(get_class_message_id()),
            intValue(intValue),
            stringValue(stringValue)
        {
            mName = MESSAGE_NAME;
        }

        std::shared_ptr<UnSolicitedMessage> clone() {
            auto ret = std::make_shared<TestMessage2>();
            if (ret) {
                ret->intValue = intValue;
                ret->stringValue = stringValue;
                ret->setIsRemote(getIsRemote());
            }
            return std::make_shared<TestMessage2>();
        }

        void serialize(IPCOStream &os) {
            size_t stringSize = 0;
            DEBUG_LOG(<<"Serializing");
            os << intValue <<
                stringSize <<
                stringValue;
        }

        void deserialize(IPCIStream &is) {
            size_t stringSize = 0;
            DEBUG_LOG(<<"De-serializing");
            is >> intValue >>
                stringSize >> std::setw(stringSize) >>
                stringValue;
        }

        inline ~TestMessage2() {
        }
};

std::shared_ptr<TestMessage> receivedMsg = nullptr;
std::shared_ptr<TestMessage2> receivedMsg2 = nullptr;

class ReceiverModule: public Module
{
    public:
        ReceiverModule() {
            mMessageHandler = {
                HANDLER(TestMessage, ReceiverModule::handleTestMessage),
            };
            mName = "ReceiverModule";
        }

        void handleTestMessage(std::shared_ptr<TestMessage> msg) {
            DEBUG_LOG( << "Received TestMessage" << std::endl);
            std::unique_lock<std::mutex> lock(mtx);
            receivedMsg = msg;
            cv.notify_all();
        }
};

class SenderModule: public Module
{
    public:
        SenderModule() {
            mMessageHandler = {
                HANDLER(TestMessage2, SenderModule::handleTestMessage),
            };
            mName = "SenderModule";
        }

        void handleTestMessage(std::shared_ptr<TestMessage2> msg) {
            DEBUG_LOG( << "Received TestMessage2" << std::endl);
            std::unique_lock<std::mutex> lock(mtx);
            receivedMsg2 = msg;
            cv.notify_all();
        }
};

TEST(Messenger, Initialization) {
    Messenger &msgr = Messenger::get();
    std::equal_to<message_id_t> compare;
    
    msgr.setOnClientConnectedCb([](pid_t pid) {
            DEBUG_LOG( << "Client connected. Pid: " << pid);
            std::unique_lock<std::mutex> lock(mtx);
            lastPid = pid;
            remoteClients++;
            DEBUG_LOG( << "remoteClients: " << remoteClients);
            cv.notify_all();

            if (!receiver) {
                DEBUG_LOG( << "Registering for TestMessage2" << std::endl);
                Messenger::get().registerForMessage(TestMessage2::get_class_message_id(),
                        [] (IPCIStream &is) -> std::shared_ptr<IPCMessage> {
                        DEBUG_LOG(<< "Constructing TestMessage2");
                        auto msg = std::static_pointer_cast<IPCMessage>(
                            std::make_shared<TestMessage2>());

                        if (msg) {
                            msg->deserialize(is);
                        }
                        DEBUG_LOG(<< ((msg) ? "Construction successful" : "Construction failed"));
                        return msg;
                });
            } else {
                DEBUG_LOG( << "Registering for TestMessage" << std::endl);
                Messenger::get().registerForMessage(TestMessage::get_class_message_id(),
                        [] (IPCIStream &is) -> std::shared_ptr<IPCMessage> {
                        DEBUG_LOG(<< "Constructing TestMessage");
                        auto msg = std::static_pointer_cast<IPCMessage>(
                            std::make_shared<TestMessage>());
                        DEBUG_LOG(<< ((msg) ? "Construction successful" : "Construction Failed"));
                        return msg;
                });
            }
    });

    msgr.setOnClientDeadCb([](pid_t pid) {
            DEBUG_LOG( << "Client disconnected Pid: " << pid);
            std::unique_lock<std::mutex> lock(mtx);
            remoteClients--;
            DEBUG_LOG( << "remoteClients: " << remoteClients);
            cv.notify_all();
    });
    msgr.setOnRemoteRegistrationCb([](pid_t sender, message_id_t msgId) {
            DEBUG_LOG( << "Remote client " << sender
                << " registered for message " << (msgId ? msgId->get_name() : ""));
            std::unique_lock<std::mutex> lock(mtx);
            remoteRegistered = msgId;
            cv.notify_all();
    });
    DEBUG_LOG( << "My PID: " << getpid());
    if (!receiver) {
        load_module<SenderModule> senderModule;
        senderModule.get_module();
        auto clients = remoteClients;

        DEBUG_LOG( << "Starting QtiBusTransportServer" << std::endl);
        QtiBusTransportServer::get().start();

        DEBUG_LOG( << "Starting messenger" << std::endl);
        msgr.start();
        DEBUG_LOG( << "Waiting for client to connect. Current: " << clients
                << " updated: " << remoteClients << std::endl);
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [clients]() {
                DEBUG_LOG( << "remoteClients: " << remoteClients << " clients: " << clients);
                return (remoteClients > clients);} );
        ASSERT_GT(remoteClients, clients);

        ASSERT_TRUE( remoteClients > 0);

        DEBUG_LOG(<< "Waiting for remoteRegistered" << std::endl);
        cv.wait(lock, []() { return remoteRegistered != nullptr; } );
        ASSERT_TRUE(compare(remoteRegistered, TestMessage::get_class_message_id()));

        auto msg = std::make_shared<TestMessage>();
        DEBUG_LOG( << "Broadcasting test message"  << std::endl);
        msg->broadcast();
        DEBUG_LOG( << "Done broacasting");

        DEBUG_LOG( << "Waiting for TestMessage2" << std::endl);
        cv.wait(lock, []() -> bool { return (receivedMsg2 != nullptr); });
        ASSERT_NE(receivedMsg2, nullptr);
    } else {
        load_module<ReceiverModule> receiverModule;
        receiverModule.get_module();
        DEBUG_LOG( << "Starting messenger" << std::endl);

        std::unique_lock<std::mutex> lock(mtx);
        msgr.start();

        DEBUG_LOG( << "Waiting for message" << std::endl);
        cv.wait(lock, []() -> bool { return (receivedMsg != nullptr); });
        ASSERT_TRUE(receivedMsg != nullptr);

        DEBUG_LOG( << "Waiting for remote registration" << std::endl);
        cv.wait(lock, []() ->bool { return remoteRegistered != nullptr; } );
        ASSERT_TRUE(compare(remoteRegistered, TestMessage2::get_class_message_id()));

        std::shared_ptr<TestMessage2> msg = std::make_shared<TestMessage2>();
        if (msg) {
            DEBUG_LOG( << "Sending TestMessage2" );
            msg->broadcast();
            sleep(1);
        }
    }
    DEBUG_LOG( << "Completed!!!" << std::endl);
    msgr.stop();
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    if (argc >= 2 && !strcmp(argv[1],"-r")) {
        receiver = true;
    }
    return RUN_ALL_TESTS();
}
