/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <string>
#include <framework/legacy.h>
struct MessageContext
{
    uint32_t serial; // For legacy purposes
    qcril_instance_id_e_type instance_id;

    MessageContext(qcril_instance_id_e_type instance_id): instance_id(instance_id)
    {}
    MessageContext(qcril_instance_id_e_type instance_id, uint32_t serial):
        serial(serial),
        instance_id(instance_id)
    {}
    virtual ~MessageContext() {}
    qcril_instance_id_e_type getInstanceId() {
        return instance_id;
    }
    virtual std::string toString() {
        return std::string("MessageContext Serial (") + std::to_string(serial) + std::string(")");
    }
};

template <class T>
struct MessageContextBase: public MessageContext
{

    MessageContextBase(qcril_instance_id_e_type instance_id): MessageContext(instance_id) {}
    MessageContextBase(qcril_instance_id_e_type instance_id, uint32_t serial):
        MessageContext(instance_id, serial) {}
#if 0
     std::string toString() {
         return T::toString();
     }
#endif
     qcril_instance_id_e_type getInstanceId() { return MessageContext::instance_id; }
};

