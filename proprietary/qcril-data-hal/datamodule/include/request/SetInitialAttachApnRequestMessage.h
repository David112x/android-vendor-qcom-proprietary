/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "framework/message_translator.h"
#include "framework/legacy.h"
#include "modules/android/ril_request_info.h"

#include "modules/android/version.h"
#include "telephony/ril.h"

#ifndef QCRIL_RIL_VERSION
#error "undefined QCRIL_RIL_VERSION!"
#endif

class SetInitialAttachApnRequestMessage : public SolicitedMessage<generic_callback_payload>,
                              public add_message_id<SetInitialAttachApnRequestMessage>
{
public:
  struct base_attach_params_t {
    std::string apn;
    std::string username;
    std::string password;
    uint8_t authType;
    std::string protocol;
    base_attach_params_t() {
       apn = "";
       username = "";
       password = "";
       authType = 0;
       protocol = "";
    }
   };
#if (QCRIL_RIL_VERSION >= 15)
   struct v15_attach_params_t : base_attach_params_t {
    std::string mvnoType;
    std::string mvnoMatchData;
    std::string roamingProtocol;
    uint32_t apnTypeBitmask;
    uint32_t bearerBitmask;
    uint8_t modemCognitive;
    uint8_t mtu;
    v15_attach_params_t() {
      mvnoType = "";
      mvnoMatchData = "";
      modemCognitive = 0;
      roamingProtocol = "";
      apnTypeBitmask = 0;
      bearerBitmask = 0;
      mtu = 0;
    }
   };
#endif

private:
  qcril_request_params_type *params;
#if (QCRIL_RIL_VERSION < 15)
  base_attach_params_t attach_params;
#else
  v15_attach_params_t attach_params;
#endif
  void deepCopy(const ril_request_type &request);
  bool allocate_and_copy_data(const char *src, char **des);

public:
 static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.set_initial_attach_apn";
 SetInitialAttachApnRequestMessage() = delete;
  ~SetInitialAttachApnRequestMessage();

  inline SetInitialAttachApnRequestMessage(const ril_request_type &request):
      SolicitedMessage<generic_callback_payload>(get_class_message_id())
  {
    mName = MESSAGE_NAME;
    params = nullptr;
    deepCopy(request);
  }

  inline const qcril_request_params_type *get_params() {
    return params;
  }

#if (QCRIL_RIL_VERSION < 15)

  inline const base_attach_params_t &get_attach_params() {
    return attach_params;
  }

#else

  inline const v15_attach_params_t &get_attach_params() {
    return attach_params;
  }

#endif

  string dump();
};

