/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "UnSolMessages/HandoverInformationIndMessage.h"

using namespace rildata;

HandoverInformationIndMessage::HandoverInformationIndMessage
(HandoffState_t setState, IpFamilyType_t setIpType, int setCid) : UnSolicitedMessage(get_class_message_id()),
    state(setState), ipType(setIpType), cid(setCid), isFailReasonSet(false), failReason {}
{
    mName = MESSAGE_NAME;
}

HandoverInformationIndMessage::~HandoverInformationIndMessage()
{
}

void HandoverInformationIndMessage::setFailReason(wds_handoff_failure_reason_type_v01 setReason) {
    failReason = setReason;
    isFailReasonSet = true;
}

HandoffState_t HandoverInformationIndMessage::getState()
{
    return state;
}

IpFamilyType_t HandoverInformationIndMessage::getIpType()
{
    return ipType;
}

int HandoverInformationIndMessage::getCid()
{
    return cid;
}

bool HandoverInformationIndMessage::hasFailReason()
{
    return isFailReasonSet;
}

wds_handoff_failure_reason_type_v01 HandoverInformationIndMessage::getFailReason()
{
    return failReason;
}

std::shared_ptr<UnSolicitedMessage> HandoverInformationIndMessage::clone()
{
    auto copy = std::make_shared<HandoverInformationIndMessage>(state, ipType, cid);
    if (copy != nullptr && copy->hasFailReason()) {
        copy->setFailReason(failReason);
    }
    return copy;
}

string HandoverInformationIndMessage::dump()
{
    return std::string(MESSAGE_NAME) + " state=" + std::to_string(static_cast<int>(state)) +
            " ipType=" + std::to_string(static_cast<int>(ipType)) + " cid=" +
            std::to_string(cid) + " hasFailReason=" + std::to_string(isFailReasonSet);
}
