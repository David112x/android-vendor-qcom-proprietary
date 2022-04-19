/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <memory>
#include <string>
#include <telephony/ril.h>
#include <framework/Message.h>
#include <framework/SolicitedMessage.h>
#include <framework/message_translator.h>
#include <framework/MessageContext.h>


struct qcril_event_context;

struct qcril_event_context: public MessageContext
{
    uint32_t token_id;
    int req_id;
    bool is_internal;
    RIL_Token t;
    std::shared_ptr<SolicitedMessage<generic_callback_payload>> msg;
    qcril_event_context(qcril_instance_id_e_type instance_id, int req_id, RIL_Token t):
        MessageContext(instance_id, *static_cast<uint32_t *>(t)),
        token_id(*static_cast<uint32_t *>(t)),
        req_id(req_id),
        is_internal(false),
        t(t)
    {
    }
    virtual ~qcril_event_context() {}

    std::string toString() {
        return std::string("Android Token id (") + std::to_string(token_id) + std::string(")");
    }
};
