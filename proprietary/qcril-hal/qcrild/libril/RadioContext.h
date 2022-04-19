/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <framework/MessageContext.h>
#include <utils/StrongPointer.h>

template <typename T>
class RadioContextClass: public MessageContextBase<RadioContextClass<T>>
{
    private:
        int32_t serial;
        sp<T> service;
    public:
        RadioContextClass(sp<T> &&service, int32_t serial): MessageContextBase<RadioContextClass>(service? service->getInstanceId() : QCRIL_DEFAULT_INSTANCE_ID), serial(serial) {}
        std::string toString() {
            return std::string("IRadio(") + std::to_string(serial) + std::string(")");
        }
        int32_t getSerial() { return serial; }
        sp<T> getService() { return service; }


};
