/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <framework/Log.h>
#include <framework/MessageContext.h>
#include <utils/StrongPointer.h>

class QtiRadioContext : public MessageContext {
 private:
 public:
  QtiRadioContext(qcril_instance_id_e_type instance_id, uint32_t serial)
      : MessageContext(instance_id, serial) {}

  ~QtiRadioContext() = default;

  std::string toString() {
    return std::string("QtiRadio(") + std::to_string(serial) + std::string(")");
  }
};
