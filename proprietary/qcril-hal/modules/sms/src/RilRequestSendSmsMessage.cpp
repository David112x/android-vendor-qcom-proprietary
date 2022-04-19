/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestSendSmsMessage.h"
#include "telephony/ril.h"

const string& RilRequestSendSmsMessage::getSmscPdu() {
    return mSmscPdu;
}

const string& RilRequestSendSmsMessage::getPdu() {
    return mPdu;
}

bool RilRequestSendSmsMessage::getLinkCtlEnable() {
    return mLinkCtlEnable;
}

string RilRequestSendSmsMessage::dump() {
    return mName + " { Link Contrl Enabled  = " + (mLinkCtlEnable ?
            "true" : "false") + " pdu = " + mPdu + " smsc = "
            + mSmscPdu + " }";
}
