/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"
#include "telephony/ril.h"

/* Request to start network scan
   @Receiver: NasModule
   */

class RilRequestStartNetworkScanMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestStartNetworkScanMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_START_NETWORK_SCAN";
    enum class RequestSource { RADIO, OEM};

    RilRequestStartNetworkScanMessage() = delete;
    ~RilRequestStartNetworkScanMessage() {}

    inline RilRequestStartNetworkScanMessage(std::shared_ptr<MessageContext> context,
        const RIL_NetworkScanRequest& scanReq) : QcRilRequestMessage(get_class_message_id(),
        context), mScanReq(scanReq) {
      mName = MESSAGE_NAME;
    }
    inline RilRequestStartNetworkScanMessage(std::shared_ptr<MessageContext> context) :
        QcRilRequestMessage(get_class_message_id(), context) {
      //set default values if request is sent from OEM HOOK service
      mScanReq.specifiers_length = 0;
      mName = MESSAGE_NAME;
    }
    const RIL_NetworkScanRequest& getNetworkScanRequest() { return mScanReq; }

    void setRequestSource(RilRequestStartNetworkScanMessage::RequestSource type) {
        mRequestSource = type;
    }
    RilRequestStartNetworkScanMessage::RequestSource getRequestSource() {
        return mRequestSource;
    }
private:
    RIL_NetworkScanRequest mScanReq;
    RequestSource mRequestSource = RequestSource::RADIO;
};
