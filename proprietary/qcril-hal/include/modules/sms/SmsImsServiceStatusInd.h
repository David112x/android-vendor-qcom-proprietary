/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <framework/legacy.h>
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>


class SmsImsServiceStatusInd : public UnSolicitedMessage, public add_message_id<SmsImsServiceStatusInd> {
  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.sms.ims.serivce_status_ind";

    enum class SmsImsServiceStatus {
      NO_SERVICE,
      LIMITED_SERVICE,
      FULL_SERVICE,
    };

    struct SmsImsServiceStatusInfo {
      SmsImsServiceStatus sms_ims_service_status;
    };

    SmsImsServiceStatusInd(SmsImsServiceStatusInfo smsImsServiceStatusInfo):UnSolicitedMessage(get_class_message_id())
    {
      mName = SmsImsServiceStatusInd::MESSAGE_NAME;
      // sms ims service status
      mSmsImsServiceStatusInfo.sms_ims_service_status = smsImsServiceStatusInfo.sms_ims_service_status;
    }

    inline ~SmsImsServiceStatusInd() {}

    std::shared_ptr<UnSolicitedMessage> clone() {
      return (std::make_shared<SmsImsServiceStatusInd>(mSmsImsServiceStatusInfo));
    }

    string dump() {
      return SmsImsServiceStatusInd::MESSAGE_NAME;
    }

    inline SmsImsServiceStatusInfo &getSmsImsServiceStatusInfo() {
      return mSmsImsServiceStatusInfo;
    }

  private:
    SmsImsServiceStatusInfo mSmsImsServiceStatusInfo;
};
