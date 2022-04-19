/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <sstream>
#include <qtibus/Messenger.h>
#include <qtibus/IPCMessage.h>
#include "QtiBusSocketTransport.h"
#include <framework/message_id.h>
#include <framework/Module.h>
#include <QtiMutex.h>
#include "MessengerImpl.h"
#ifdef __ANDROID__
#define SHM_BUS_FILENAME "/data/vendor/radio/shmbus"
#else
#define SHM_BUS_FILENAME "shmbus"
#endif

#define TAG "Messenger"

#ifndef __ANDROID__
thread_local char timebuffer[20];
#endif

class MessengerModule: public Module {
    public:
        MessengerModule() {
            mMessageHandler = {};
            mName = "MessengerModule";
        }

        void handleOutgoingMessages( std::shared_ptr<IPCMessage> msg) {
            DEBUG_LOG( << "handleOutgoingMessages " << (msg ? msg->dump() : "<<NULL>>"));
            if (msg) {
                if (!msg->getIsRemote()) {
                    message_id_t msgId = msg->get_message_id();
                    DEBUG_LOG( << "hash for msgId: " << msgId->get_name() << ": " << std::hash<message_id_t>()(msgId));
                    std::stringstream payload;
                    msg->serialize(payload);
                    Messenger::get().deliverMessage(msgId, payload.str());
                }
            }
        }

        void proxyRegisterForMessage(const message_id_t &msgId) {
            DEBUG_LOG( << "Registering with Dispatcher. msgId: " << msgId << " ( " << (void *)msgId);
            DEBUG_LOG( << "hash for msgId: " << msgId->get_name() << ": " << std::hash<message_id_t>()(msgId));
            mMessageHandler[msgId] = [this] (std::shared_ptr<Message> msg) {
                handleOutgoingMessages(std::static_pointer_cast<IPCMessage>(msg));
            };
            Dispatcher::getInstance().registerHandler(msgId, this);
            DEBUG_LOG( << "Done registering with Dispatcher" << std::endl);
        }

};

load_module<MessengerModule> messengerModule;
void MessengerImpl::localMessageDelivery(std::string msgIdString, std::string payload) {
    message_id_t msgId = REG_MSG(msgIdString);
    if (msgId) {
        DEBUG_LOG(<< "Locking mutex");
        std::shared_lock<qtimutex::QtiSharedMutex> lock(mutex);
        DEBUG_LOG(<< "Done locking mutex");
        auto it = registeredMessages.find(msgId);
        if (it != registeredMessages.end() &&
                (*it).second) {
            DEBUG_LOG( << "Calling constructor for msg " << Dispatcher::getInstance().getMessageName(msgId));
            std::stringstream is(payload);
            auto msg = ((*it).second)(is);
            if (msg) {
                msg->setIsRemote(true);
                msg->broadcast();
            }
        }
    }
}

MessengerImpl::MessengerImpl(Messenger &owner):
    xport(nullptr),
    started(false),
    owner(owner)
{
}

bool MessengerImpl::start() {
    DEBUG_LOG( << "> xport: " << xport << " started: " << started << std::endl);
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mutex);
    if (started) return true;
    if (!xport) {
        xport = new QtiBusSocketTransport(IPC_SOCKET_NAME);
        if (!xport) return false;
    }

    started = true;
    xport->setOnClientConnectedCb([this](pid_t pid){
        DEBUG_LOG( << "client pid: " << pid);
            DEBUG_LOG( << "Sending registered messages. Count: " << registeredMessages.size());
            for (auto msg: registeredMessages) {
                registerForMessage(msg.first, msg.second, true);
            }
            if (owner.onClientConnected) {
                DEBUG_LOG( << "Calling user onClientConnected");
                owner.onClientConnected(pid);
            } else {
                DEBUG_LOG( << "User onClientConnected is null");
            }
        });
    xport->setOnClientDeadCb([this](pid_t pid){
        DEBUG_LOG( << "Client " << pid << " dead");
            if (owner.onClientDead) {
                DEBUG_LOG(<< "Calling callback");
                owner.onClientDead(pid);
            }
        });
    xport->setOnNewMessageCb([this](std::string msg){
            std::stringstream is(msg);

            size_t overall_size = 0;
            DEBUG_LOG(<<"Message received.");
            is.read((char *)&overall_size, sizeof(overall_size));
            DEBUG_LOG(<<"overall_size: " << overall_size);

            MessengerCommands cmd = MessengerCommands::UNKNOWN;
            is.read((char *)&cmd, sizeof(cmd));

            switch(cmd) {
                case MessengerCommands::DELIVER_MESSAGE:
                    {
                        DEBUG_LOG(<< "DELIVER_MESSAGE from remote");
                        size_t msgIdSize = 0;
                        is.read((char *)&msgIdSize, sizeof(msgIdSize));
                        is.width(msgIdSize);
                        DEBUG_LOG(<< "msgIdSize: " << msgIdSize);
                        std::string msgIdString;
                        is >> std::noskipws >> msgIdString;
                        DEBUG_LOG(<< "msgIdString: " << msgIdString);

                        size_t payload_size = 0;
                        is.read((char *)&payload_size, sizeof(payload_size));
                        DEBUG_LOG(<< "payload_size: " << payload_size);
                        std::string payload;
                        is.width(payload_size);
                        is >> std::noskipws >> payload;
                        localMessageDelivery(msgIdString, payload);
                    }
                    break;
                case MessengerCommands::REGISTER_MESSAGE:
                    {
                        DEBUG_LOG(<< "REGISTER_MESSAGE from remote");
                        pid_t sender = 0;
                        is.read((char *)&sender, sizeof(sender));
                        DEBUG_LOG(<< "sender: " << sender);
                        size_t msgIdSize = 0;

                        is.read((char *)&msgIdSize, sizeof(msgIdSize));
                        DEBUG_LOG(<< "msgIdSize: " << msgIdSize);
                        std::string msgIdString;
                        is.width(msgIdSize);
                        is >> msgIdString;
                        DEBUG_LOG(<< "msgIdString: " << msgIdString);
                        localRegisterMessage(sender, msgIdString);
                    }
                    break;
                default:
                    break;
            }
        });
    xport->start();
    DEBUG_LOG( << " < xport: " << xport << " started: " << started << std::endl);
    return true;
}

void MessengerImpl::localRegisterMessage(pid_t sender, std::string msgIdString) {
    message_id_t msgid = REG_MSG(msgIdString);
    DEBUG_LOG( << "Registering with dispatcher. Sender: " << sender
            << " Message id: " << msgid << std::endl);

    messengerModule.get_module().proxyRegisterForMessage(msgid);
    if (owner.onRemoteRegistration) {
        owner.onRemoteRegistration(sender, msgid);
    }
}

bool MessengerImpl::stop() {
    DEBUG_LOG( << "> xport: " << xport << " started: " << started << std::endl);
    auto tmpport = xport;

    {
        std::unique_lock<qtimutex::QtiSharedMutex> lock(mutex);
        if (!xport) return false;
        if (!started) return true;

        started = false;
        tmpport = xport;
        xport = nullptr;
    }

    tmpport->stop();
    delete tmpport;

    DEBUG_LOG( << " started: " << started << std::endl);
    return true;
}

void MessengerImpl::registerForMessage(const message_id_t &msgid, Messenger::MsgConstructor_t cnstrct,
        bool reRegister) {

    if (!msgid) return;
    DEBUG_LOG( << "msgid: " << msgid->get_name());
    if (msgid->get_name().size() == 0) return;

    if (!started) {
        start();
        assert(started);
        if (!started) {
            return;
        }
    }

    {
        std::unique_lock<qtimutex::QtiSharedMutex> lock(mutex);
        auto it = registeredMessages.find(msgid);
        if ( it == registeredMessages.end() || reRegister) {
            registeredMessages[msgid] = cnstrct;
            std::stringstream os;
            size_t overall_size = 0;
            std::string msgIdString {msgid->get_name()};
            size_t msgIdSize = msgIdString.size();


            // Write the overall size.
            // Come back to it later, when we have completed writing
            // and have the overall size.
            std::stringstream::pos_type pos = os.tellp();
            os.write((char *)&overall_size, sizeof(overall_size));

            // Write the MessengerCommand
            MessengerCommands cmd = MessengerCommands::REGISTER_MESSAGE;
            os.write((char *)&cmd, sizeof(cmd));
            DEBUG_LOG(<< "Command: " << cmd);

            // Write the sender
            pid_t pid = getpid();
            os.write((char *)&pid, sizeof(pid));
            DEBUG_LOG(<< "Sender: " << pid);

            // Write the message id
            os.write((char *)&msgIdSize, sizeof(msgIdSize));
            os << std::noskipws <<
                msgIdString;

            DEBUG_LOG(<< "msgIdString: " << msgIdString);

            os.seekp(0, std::ios_base::seekdir::end);
            overall_size = os.tellp() - pos;

            os.seekp(pos);
            os.write((char *)&overall_size, sizeof(overall_size));
            os.seekp(pos);

            DEBUG_LOG( << "Registering for message: " << msgid <<
                    " with remote clients. overall_size: " << overall_size);
            xport->write(os.str());
        }
    }

}

void MessengerImpl::deliverMessage(const message_id_t &msgid, std::string payload) {

    if (!msgid) return;
    DEBUG_LOG( << "msgid: " << msgid->get_name());
    if (start()) {
        std::stringstream os;
        size_t overall_size = 0;

        auto pos = os.tellp();
        os.write((char *)&overall_size, sizeof(overall_size));

        MessengerCommands cmd = MessengerCommands::DELIVER_MESSAGE;
        os.write((char *)&cmd, sizeof(cmd));
        DEBUG_LOG(<< "Command: " << cmd);

        std::string msgIdString{msgid->get_name()};
        DEBUG_LOG(<< "msgIdString: " << msgIdString);

        size_t msgIdSize = msgIdString.size();
        DEBUG_LOG(<< "msgIdSize: " << msgIdSize);
        os.write((char *)&msgIdSize, sizeof(msgIdSize));
        os.width(msgIdString.size());
        os << std::noskipws << msgIdString;

        size_t payload_size = payload.size();
        os.write((char *)&payload_size, sizeof(payload_size));
        DEBUG_LOG(<< "payload_size: " << payload_size);

        os.width(payload_size);
        os << std::noskipws << payload;

        os.seekp(0, std::ios_base::seekdir::end);
        auto pos_end = os.tellp();

        overall_size = pos_end - pos;

        DEBUG_LOG(<< "overall_size: " << overall_size);

        os.seekp(pos);
        os.write((char *)&overall_size, sizeof(overall_size));
        os.seekp(pos);
        std::string buffer = os.str();
        std::shared_lock <qtimutex::QtiSharedMutex> lock(mutex);
        if (xport) {
            xport->write(buffer);
        }
    }
}

Messenger::Messenger() {
}

Messenger &Messenger::get() {
    static Messenger msgr;
    return msgr;
}
void Messenger::registerForMessage(const message_id_t &msgid, Messenger::MsgConstructor_t cnstrct)
{
    if (impl) {
        impl->registerForMessage(msgid, cnstrct);
    }
}

void Messenger::deliverMessage(const message_id_t &msgid, std::string payload) {
    if (impl) {
        impl->deliverMessage(msgid, payload);
    }
}
void Messenger::start() {

    if (!impl) {
        impl = new MessengerImpl(*this);
    }

    if (impl) {
        impl->start();
    }
}
void Messenger::stop() {
    if (impl) {
        impl->stop();
        delete impl;
        impl = nullptr;
    }
}
