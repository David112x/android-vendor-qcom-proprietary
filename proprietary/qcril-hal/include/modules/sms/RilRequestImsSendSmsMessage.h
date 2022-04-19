/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "framework/legacy.h"
#include "modules/sms/qcril_qmi_sms_types.h"


/* Request to send a IMS SMS Message (including GSM and CDMA)
   @Receiver: SmsModule
*/
class RilRequestImsSendSmsMessage : public SolicitedMessage<RilSendSmsResult_t>,
                                    public add_message_id<RilRequestImsSendSmsMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_IMS_SEND_SMS";

    RilRequestImsSendSmsMessage() = delete;
    ~RilRequestImsSendSmsMessage() {}

    explicit RilRequestImsSendSmsMessage(uint32_t msgRef, RIL_RadioTechnologyFamily tech,
        bool retry) : SolicitedMessage<RilSendSmsResult_t>(get_class_message_id()),
                      mRef(msgRef), mRetry(retry), mTech(tech) {}

    template<typename T1, typename T2>
    inline void setGsmPayload(T1&& smscPdu, T2&& pdu) {
        if (mTech == RADIO_TECH_3GPP) {
            mGsmSmscPdu = std::forward<T1>(smscPdu);
            mGsmPdu = std::forward<T2>(pdu);
        }
    }
    void setCdmaPayload(const RIL_CDMA_SMS_Message& payload);

    RIL_CDMA_SMS_Message& getCdmaPayload();
    const string& getGsmSmscPdu();
    const string& getGsmPdu();

    bool isRetry();
    uint32_t getMessageReference();
    RIL_RadioTechnologyFamily getRadioTechFamily();

    string dump();

  private:
    const uint32_t mRef;
    const bool mRetry;
    const RIL_RadioTechnologyFamily mTech;
    string mGsmSmscPdu;
    string mGsmPdu;
    RIL_CDMA_SMS_Message mCdmaPayload;
};
