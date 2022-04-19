/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once
#include <framework/UnSolicitedMessage.h>
#include <ostream>
#include <istream>

#define QCRIL_REMOTE_IPC_ENABLED

using IPCOStream = std::ostream;
using IPCIStream = std::istream;

class IPCMessage: public UnSolicitedMessage {
    private:
        bool isRemote = false;
    public:
        IPCMessage() = delete;
        IPCMessage(message_id_t id): UnSolicitedMessage(id) {}
        virtual void serialize(IPCOStream &os) = 0;
        virtual void deserialize(IPCIStream &is) = 0;
        void setIsRemote(bool isRemote) {
            this->isRemote = isRemote;
        }
        bool getIsRemote() {
            return isRemote;
        }
};
