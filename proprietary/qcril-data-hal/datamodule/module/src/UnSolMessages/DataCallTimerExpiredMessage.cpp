/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "UnSolMessages/DataCallTimerExpiredMessage.h"

using namespace rildata;

DataCallTimerExpiredMessage::DataCallTimerExpiredMessage(int cid,
                                                         DataCallTimerType type)
: UnSolicitedMessage(get_class_message_id()), mCid(cid), mType(type)
{
    mName = MESSAGE_NAME;
}

DataCallTimerExpiredMessage::~DataCallTimerExpiredMessage()
{
}

int DataCallTimerExpiredMessage::getCid() {
    return mCid;
}

DataCallTimerType DataCallTimerExpiredMessage::getType() {
    return mType;
}

string DataCallTimerExpiredMessage::getStringType(DataCallTimerType type) {
    switch (type) {
        case DataCallTimerType::Handover:
            return string("Handover");
        case DataCallTimerType::SetupDataCall:
            return string("SetupDataCall");
        case DataCallTimerType::PartialRetry:
            return string("PartialRetry");
        default:
            return string("UNKNOWN");
    }
}

std::shared_ptr<UnSolicitedMessage> DataCallTimerExpiredMessage::clone()
{
    return std::make_shared<DataCallTimerExpiredMessage>(mCid, mType);
}

string DataCallTimerExpiredMessage::dump()
{
    string msg = std::string(MESSAGE_NAME) + " cid=" + std::to_string(mCid) + " type=" + getStringType(mType);
    return msg;
}
