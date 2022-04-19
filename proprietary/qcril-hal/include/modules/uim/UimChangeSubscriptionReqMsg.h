/******************************************************************************
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"

typedef enum {
  UIM_UICC_SUBSCRIPTION_DEACTIVATE = 0,
  UIM_UICC_SUBSCRIPTION_ACTIVATE   = 1
} RIL_UIM_UiccSubActStatus;

typedef enum {
  UIM_SUBSCRIPTION_1 = 0,
  UIM_SUBSCRIPTION_2 = 1,
  UIM_SUBSCRIPTION_3 = 2
} RIL_UIM_SubscriptionType;

typedef enum {
  QCRIL_SUBS_MODE_1X  = 0,
  QCRIL_SUBS_MODE_GW  = 1
} qcril_uim_subs_mode_pref;

typedef struct {
  uint8_t   slot;                    /* 0, 1, ... etc. */
  int       app_index;               /* array subscriptor from applications[RIL_CARD_MAX_APPS] in
                                        RIL_REQUEST_GET_SIM_STATUS */
  RIL_UIM_SubscriptionType  sub_type;    /* Indicates subscription 0 or subscription 1 */
  RIL_UIM_UiccSubActStatus  act_status;
} RIL_UIM_SelectUiccSub;

typedef struct {
  RIL_UIM_SelectUiccSub    uicc_subs_info;
  qcril_uim_subs_mode_pref subs_mode_pref;
} qcril_uim_uicc_subs_info_type;

/*===========================================================================

  CLASS:  UimChangeSubscriptionReqMsg

===========================================================================*/
/*!
    @brief
    Unsol UIM Message request for activate and deactivate subscription.

    0 - Deactivate
    1 - Activate

    @return
    None.
*/
/*=========================================================================*/
class UimChangeSubscriptionReqMsg : public SolicitedMessage<void>,
                                    public add_message_id<UimChangeSubscriptionReqMsg>
{
  private:
    qcril_uim_uicc_subs_info_type   mSubInfo;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.change_provision_request";
    UimChangeSubscriptionReqMsg() = delete;
    ~UimChangeSubscriptionReqMsg() = default;

    inline UimChangeSubscriptionReqMsg(const qcril_uim_uicc_subs_info_type &sub_info):
                                  SolicitedMessage<void>(get_class_message_id())
    {
      mSubInfo = sub_info;
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      string log = "Mode Pref: " + std::to_string(mSubInfo.subs_mode_pref);

      log = log + "Slot: " + std::to_string(mSubInfo.uicc_subs_info.slot);
      log = log + "App index: " + std::to_string(mSubInfo.uicc_subs_info.app_index);
      log = log + "subscription: " + std::to_string(mSubInfo.uicc_subs_info.sub_type);
      log = log + "Activation req: " + std::to_string(mSubInfo.uicc_subs_info.act_status);

      return mName + "{}" + log;
    }

    inline const qcril_uim_uicc_subs_info_type *get_sub_info(void)
    {
      return &mSubInfo;
    }
};
