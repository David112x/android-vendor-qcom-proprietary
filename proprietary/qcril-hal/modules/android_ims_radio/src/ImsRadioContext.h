/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <framework/Log.h>
#include <framework/MessageContext.h>
#include <utils/StrongPointer.h>

class ImsRadioContext : public MessageContext {
 private:
 public:
  ImsRadioContext(qcril_instance_id_e_type instance_id, uint32_t serial)
      : MessageContext(instance_id, serial) {}

  ~ImsRadioContext() { /*Log::getInstance().d("[ImsRadioContext]: destructor : " + toString()); */ }

  std::string toString() {
    return std::string("IImsRadio(") + std::to_string(serial) + std::string(")");
  }
};
