/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <framework/MessageContext.h>
#include <utils/StrongPointer.h>

class OemHookContext : public MessageContext {
 private:
 public:
  OemHookContext(qcril_instance_id_e_type instance_id, uint32_t serial)
      : MessageContext(instance_id, serial) {}

  ~OemHookContext() {}

  std::string toString() {
    return std::string("IOemHook(") + std::to_string(serial) + std::string(")");
  }
};
