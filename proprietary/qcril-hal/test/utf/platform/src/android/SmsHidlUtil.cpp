/*===========================================================================
 *
 *    Copyright (c) 2018 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#include "platform/android/SmsHidlUtil.h"

int convertGsmSmsPayloadtoHidl(void *data, size_t datalen, V1_0::GsmSmsMessage& sms)
{
    auto payload = static_cast<const char **>(data);
    sms.smscPdu = payload[0];
    sms.pdu = payload[1];
    return 0;
}

int convertGsmSmsHidltoRilPayload(const V1_0::SendSmsResult& sms, RIL_SMS_Response& data) {
    data.messageRef = sms.messageRef;
    data.errorCode = sms.errorCode;
    data.ackPDU = nullptr;
    return 0;
}

V1_0::SmsWriteArgsStatus convertSmsWriteArgsStatustoHidl(int in)
{
    switch(in)
    {
        case 0:
            return V1_0::SmsWriteArgsStatus::REC_UNREAD;
        case 1:
            return V1_0::SmsWriteArgsStatus::REC_READ;
        case 2:
            return V1_0::SmsWriteArgsStatus::STO_UNSENT;
        case 3:
            return V1_0::SmsWriteArgsStatus::STO_SENT;
        default:
            return V1_0::SmsWriteArgsStatus::REC_UNREAD;
    }
}

int convertGsmSmsWritePayloadtoHidl(void *in, size_t datalen,
                                                android::hardware::radio::V1_0::SmsWriteArgs& out)
{
    auto payload = static_cast<RIL_SMS_WriteArgs *>(in);
    out.smsc = payload->smsc;
    out.pdu = payload->pdu;
    out.status = convertSmsWriteArgsStatustoHidl(payload->status);
    return 0;
}

V1_0::CdmaSmsWriteArgsStatus convertCdmaSmsWriteArgsStatustoHidl(int in)
{
    switch(in)
    {
        case 0:
            return V1_0::CdmaSmsWriteArgsStatus::REC_UNREAD;
        case 1:
            return V1_0::CdmaSmsWriteArgsStatus::REC_READ;
        case 2:
            return V1_0::CdmaSmsWriteArgsStatus::STO_UNSENT;
        case 3:
            return V1_0::CdmaSmsWriteArgsStatus::STO_SENT;
        default:
            return V1_0::CdmaSmsWriteArgsStatus::REC_UNREAD;
    }
}

V1_0::CdmaSmsDigitMode convertCdmaSmsDigitModetoHidl(RIL_CDMA_SMS_DigitMode in)
{
    switch(in)
    {
        case RIL_CDMA_SMS_DIGIT_MODE_4_BIT :
            return V1_0::CdmaSmsDigitMode::FOUR_BIT;
        case RIL_CDMA_SMS_DIGIT_MODE_8_BIT:
            return V1_0::CdmaSmsDigitMode::EIGHT_BIT;
        default:
            return V1_0::CdmaSmsDigitMode::FOUR_BIT;
    }
}

V1_0::CdmaSmsNumberMode convertCdmaSmsNumberModetoHidl(RIL_CDMA_SMS_NumberMode in)
{
    switch(in)
    {
        case RIL_CDMA_SMS_NUMBER_MODE_NOT_DATA_NETWORK :
            return V1_0::CdmaSmsNumberMode::NOT_DATA_NETWORK;
        case RIL_CDMA_SMS_NUMBER_MODE_DATA_NETWORK:
            return V1_0::CdmaSmsNumberMode::DATA_NETWORK;
        default:
            return V1_0::CdmaSmsNumberMode::NOT_DATA_NETWORK;
    }
}

V1_0::CdmaSmsNumberType convertCdmaSmsNumberTypetoHidl(RIL_CDMA_SMS_NumberType in)
{
    switch(in)
    {
        case RIL_CDMA_SMS_NUMBER_TYPE_UNKNOWN :
            return V1_0::CdmaSmsNumberType::UNKNOWN;
        case RIL_CDMA_SMS_NUMBER_TYPE_INTERNATIONAL_OR_DATA_IP :
            return V1_0::CdmaSmsNumberType::INTERNATIONAL_OR_DATA_IP;
        case RIL_CDMA_SMS_NUMBER_TYPE_NATIONAL_OR_INTERNET_MAIL :
            return V1_0::CdmaSmsNumberType::NATIONAL_OR_INTERNET_MAIL;
        case RIL_CDMA_SMS_NUMBER_TYPE_NETWORK :
            return V1_0::CdmaSmsNumberType::NETWORK;
        case RIL_CDMA_SMS_NUMBER_TYPE_SUBSCRIBER :
            return V1_0::CdmaSmsNumberType::SUBSCRIBER;
        case RIL_CDMA_SMS_NUMBER_TYPE_ALPHANUMERIC :
            return V1_0::CdmaSmsNumberType::ALPHANUMERIC;
        case RIL_CDMA_SMS_NUMBER_TYPE_ABBREVIATED :
            return V1_0::CdmaSmsNumberType::ABBREVIATED;
        case RIL_CDMA_SMS_NUMBER_TYPE_RESERVED_7 :
            return V1_0::CdmaSmsNumberType::RESERVED_7;
        default:
            return V1_0::CdmaSmsNumberType::UNKNOWN;
    }
}

V1_0::CdmaSmsNumberPlan convertCdmaSmsNumberPlantoHidl(RIL_CDMA_SMS_NumberPlan in)
{
    switch(in)
    {
        case RIL_CDMA_SMS_NUMBER_PLAN_UNKNOWN :
            return V1_0::CdmaSmsNumberPlan::UNKNOWN;
        case RIL_CDMA_SMS_NUMBER_PLAN_TELEPHONY :
            return V1_0::CdmaSmsNumberPlan::TELEPHONY;
        case RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_2 :
            return V1_0::CdmaSmsNumberPlan::RESERVED_2;
        case RIL_CDMA_SMS_NUMBER_PLAN_DATA :
            return V1_0::CdmaSmsNumberPlan::DATA;
        case RIL_CDMA_SMS_NUMBER_PLAN_TELEX :
            return V1_0::CdmaSmsNumberPlan::TELEX;
        case RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_5 :
            return V1_0::CdmaSmsNumberPlan::RESERVED_5;
        case RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_6 :
            return V1_0::CdmaSmsNumberPlan::RESERVED_6;
        case RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_7 :
            return V1_0::CdmaSmsNumberPlan::RESERVED_7;
        case RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_8 :
            return V1_0::CdmaSmsNumberPlan::RESERVED_8;
        case RIL_CDMA_SMS_NUMBER_PLAN_PRIVATE :
            return V1_0::CdmaSmsNumberPlan::PRIVATE;
        case RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_10 :
            return V1_0::CdmaSmsNumberPlan::RESERVED_10;
        case RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_11 :
            return V1_0::CdmaSmsNumberPlan::RESERVED_11;
        case RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_12 :
            return V1_0::CdmaSmsNumberPlan::RESERVED_12;
        case RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_13 :
            return V1_0::CdmaSmsNumberPlan::RESERVED_13;
        case RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_14 :
            return V1_0::CdmaSmsNumberPlan::RESERVED_14;
        case RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_15 :
            return V1_0::CdmaSmsNumberPlan::RESERVED_15;
        default:
            return V1_0::CdmaSmsNumberPlan::UNKNOWN;
    }
}

V1_0::CdmaSmsSubaddressType convertCdmaSmsSubAddrTypetoHidl(RIL_CDMA_SMS_SubaddressType in)
{
    switch(in)
    {
        case RIL_CDMA_SMS_SUBADDRESS_TYPE_NSAP :
            return V1_0::CdmaSmsSubaddressType::NSAP;
        case RIL_CDMA_SMS_SUBADDRESS_TYPE_USER_SPECIFIED:
            return V1_0::CdmaSmsSubaddressType::USER_SPECIFIED;
        default:
            return V1_0::CdmaSmsSubaddressType::NSAP;
    }
}

int convertRilCdmaSmstoHidl(RIL_CDMA_SMS_Message &in, android::hardware::radio::V1_0::CdmaSmsMessage &out)
{
    out.teleserviceId = in.uTeleserviceID;
    out.isServicePresent = ((in.bIsServicePresent == 0)?false:true);
    out.serviceCategory = in.uServicecategory;
    out.address.digitMode = convertCdmaSmsDigitModetoHidl(in.sAddress.digit_mode);
    out.address.numberMode = convertCdmaSmsNumberModetoHidl(in.sAddress.number_mode);
    out.address.numberType = convertCdmaSmsNumberTypetoHidl(in.sAddress.number_type);
    out.address.numberPlan = convertCdmaSmsNumberPlantoHidl(in.sAddress.number_plan);
    out.address.digits.resize(in.sAddress.number_of_digits);
    for(int i=0; i<in.sAddress.number_of_digits; i++)
    {
        out.address.digits[i] = in.sAddress.digits[i];
    }

    out.subAddress.subaddressType = convertCdmaSmsSubAddrTypetoHidl(in.sSubAddress.subaddressType);
    out.subAddress.odd = ((in.sSubAddress.odd == 0)?false:true);
    out.subAddress.digits.resize(in.sSubAddress.number_of_digits);
    for(int i=0; i<in.sSubAddress.number_of_digits; i++)
    {
        out.subAddress.digits[i] = in.sSubAddress.digits[i];
    }

    out.bearerData.resize(in.uBearerDataLen);
    for(int i=0; i<in.uBearerDataLen; i++)
    {
        out.bearerData[i] = in.aBearerData[i];
    }
    return 0;
}

int convertCdmaSmsWritePayloadtoHidl(void *in, size_t datalen, V1_0::CdmaSmsWriteArgs& out)
{
    auto payload = static_cast<RIL_CDMA_SMS_WriteArgs *>(in);
    out.status = convertCdmaSmsWriteArgsStatustoHidl(payload->status);
    convertRilCdmaSmstoHidl(payload->message, out.message);
    return 0;
}

V1_0::CdmaSmsErrorClass convertCdmaSmsErrorClasstiHidl(RIL_CDMA_SMS_ErrorClass in)
{
    switch(in)
    {
        case RIL_CDMA_SMS_NO_ERROR :
            return V1_0::CdmaSmsErrorClass::NO_ERROR;
        case RIL_CDMA_SMS_ERROR:
            return V1_0::CdmaSmsErrorClass::ERROR;
        default:
            return V1_0::CdmaSmsErrorClass::NO_ERROR;
    }
}

int convertCdmaSmsAcktoHidl(void *in, size_t datalen, V1_0::CdmaSmsAck& out)
{
    auto payload = static_cast<RIL_CDMA_SMS_Ack *>(in);
    out.smsCauseCode = payload->uSMSCauseCode;
    out.errorClass = convertCdmaSmsErrorClasstiHidl(payload->uErrorClass);
    return 0;
}

int convertCdmaBcConfigtoHidl(void *in, size_t datalen,
        android::hardware::hidl_vec<android::hardware::radio::V1_0::CdmaBroadcastSmsConfigInfo>& out)
{
    auto payload = static_cast<RIL_CDMA_BroadcastSmsConfigInfo **>(in);
    out.resize(datalen/sizeof(const RIL_CDMA_BroadcastSmsConfigInfo *));
    for(int i=0; i<out.size(); i++)
    {
        out[i].serviceCategory = payload[i]->service_category;
        out[i].language = payload[i]->language;
        out[i].selected = ((payload[i]->selected == 0)?false:true);
    }
    return 0;
}

int convertGsmBcConfigtoHidl(void *in, size_t datalen,
          android::hardware::hidl_vec<android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo>& out)
{
    auto payload = static_cast<RIL_GSM_BroadcastSmsConfigInfo **>(in);
    out.resize(datalen/sizeof(const RIL_GSM_BroadcastSmsConfigInfo *));
    for(int i=0; i<out.size(); i++)
    {
        out[i].fromServiceId = payload[i]->fromServiceId;
        out[i].toServiceId = payload[i]->toServiceId;
        out[i].fromCodeScheme = payload[i]->fromCodeScheme;
        out[i].toCodeScheme = payload[i]->toCodeScheme;
        out[i].selected = ((payload[i]->selected == 0)?false:true);
    }
    return 0;
}
