/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/SolicitedSyncMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "framework/Util.h"
#include "modules/sms/SmsImsServiceStatusInd.h"

class SmsGetImsServiceStatusMessage : public SolicitedSyncMessage<SmsImsServiceStatusInd::SmsImsServiceStatusInfo>,
                                    public add_message_id<SmsGetImsServiceStatusMessage>
{
public:
  static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.sms.ims.GET_SMS_IMS_SERVICE_STATUS";
  inline SmsGetImsServiceStatusMessage():SolicitedSyncMessage<SmsImsServiceStatusInd::SmsImsServiceStatusInfo>(get_class_message_id())
  {
    mName = MESSAGE_NAME;
  }
  ~SmsGetImsServiceStatusMessage(){}
  string dump(){ return mName; }
};