/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include "QtiBusTransport.h"
#include <framework/message_id.h>
#include <functional>
#include <string>
#include <thread>

class QtiBusShmTransportGroup: public QtiBusTransportGroup
{
};

struct QtiBusShmTransportHdr;
struct QtiBusShmTransportMem;
enum class CommandId {
    REQUEST_ACK,
    NEW_CLIENT,
    NEW_MESSAGE,
    CLIENT_DEAD,
    LOCAL_DISCONNECT,
};

static inline std::ostream & operator <<(std::ostream& stream, CommandId id) {
    std::string name = "<<UNKNOWN>>";
    switch(id) {
        case CommandId::REQUEST_ACK:
            name = "REQUEST_ACK";
            break;
        case CommandId::NEW_CLIENT:
            name = "NEW_CLIENT";
            break;
        case CommandId::NEW_MESSAGE:
            name = "NEW_MESSAGE";
            break;
        case CommandId::CLIENT_DEAD:
            name = "CLIENT_DEAD";
            break;
        case CommandId::LOCAL_DISCONNECT:
            name = "LOCAL_DISCONNECT";
            break;
    }
    return stream << std::string("CommandId::") << name;
}

class QtiBusShmTransport: public QtiBusTransport
{
    private:
        std::string filename;
        int fd;
        int lockFd;
        void *ptr = nullptr;
        QtiBusShmTransportHdr *header;
        void                  *header_end;
        std::thread commandReceiver;
        int openTransport(const char *fname);
        bool registerLocalClient(size_t &idx);
        void sendCommand(CommandId cmd, unsigned arg1, unsigned arg2);
        void *mmapfd(int fd, bool init);
    public:
        QtiBusShmTransport(std::string filename);
        bool write(const std::string &buffer);
        std::string receive();
        void start();
        void stop();

        template <class T>
            void debugHook(T &);
};
