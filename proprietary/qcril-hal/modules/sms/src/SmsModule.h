/******************************************************************************
*  Copyright (c) 2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*******************************************************************************/
#pragma once

#include <framework/Module.h>
#include <framework/Dispatcher.h>
#include <framework/AddPendingMessageList.h>
#include <modules/qmi/SmsModemEndPoint.h>
#include <modules/qmi/QmiAsyncResponseMessage.h>
#include <modules/android/RilRequestMessage.h>
#include <modules/sms/RilRequestSendSmsMessage.h>
#include <modules/sms/RilRequestWriteSmsToSimMessage.h>
#include <modules/sms/RilRequestGsmSetBroadcastSmsConfigMessage.h>
#include <modules/sms/RilRequestCdmaSendSmsMessage.h>
#include <modules/sms/RilRequestCdmaSetBroadcastSmsConfigMessage.h>
#include <modules/sms/RilRequestImsSendSmsMessage.h>
#include <modules/sms/RilRequestAckGsmSmsMessage.h>
#include <modules/sms/RilRequestAckCdmaSmsMessage.h>
#include <modules/sms/RilRequestDeleteSmsOnSimMessage.h>
#include <modules/sms/RilRequestCdmaWriteSmsToRuimMessage.h>
#include <modules/sms/RilRequestCdmaDeleteSmsOnRuimMessage.h>
#include <modules/sms/RilRequestGetSmscAddressMessage.h>
#include <modules/sms/RilRequestSetSmscAddressMessage.h>
#include <modules/sms/RilRequestGetGsmBroadcastConfigMessage.h>
#include <modules/sms/RilRequestGetCdmaBroadcastConfigMessage.h>
#include <modules/sms/RilRequestGsmSmsBroadcastActivateMessage.h>
#include <modules/sms/RilRequestCdmaSmsBroadcastActivateMessage.h>
#include <modules/sms/RilRequestReportSmsMemoryStatusMessage.h>
#include <modules/sms/RilRequestGetImsRegistrationMessage.h>
#include <modules/sms/RilRequestAckImsSmsMessage.h>
#include <modules/sms/SmsGetImsServiceStatusMessage.h>

#include "comdef.h"

class SmsModule: public Module, public AddPendingMessageList {
    public:
        SmsModule();
        ~SmsModule();
        void init();
#ifdef QMI_RIL_UTF
        void qcrilHalSmsModuleCleanup();
#endif

    private:
        bool mReady = false;
        qtimutex::QtiRecursiveMutex mMutex;

        void handleQcrilInit(std::shared_ptr<Message> msg);
        void handleSmsQmiIndMessage(std::shared_ptr<Message> msg);
        void handleQmiAsyncRespMessage(std::shared_ptr<QmiAsyncResponseMessage> msg);

        void handleSendSms(std::shared_ptr<RilRequestSendSmsMessage> msg);
        void handleSmsAcknowledge(std::shared_ptr<RilRequestAckGsmSmsMessage> msg);
        void handleWriteSmsToSim(std::shared_ptr<RilRequestWriteSmsToSimMessage> msg);
        void handleDeleteSmsOnSim(std::shared_ptr<RilRequestDeleteSmsOnSimMessage> msg);
        void handleGetSmscAddress(std::shared_ptr<RilRequestGetSmscAddressMessage> msg);
        void handleSetSmscAddress(std::shared_ptr<RilRequestSetSmscAddressMessage> msg);
        void handleReportSmsMemoryStatus(std::shared_ptr<RilRequestReportSmsMemoryStatusMessage> msg);
        void handleGsmGetBroadcastSmsConfig(std::shared_ptr<RilRequestGetGsmBroadcastConfigMessage> msg);
        void handleGsmSetBroadcastSmsConfig(std::shared_ptr<RilRequestGsmSetBroadcastSmsConfigMessage> msg);
        void handleGsmSmsBroadcastActivation(std::shared_ptr<RilRequestGsmSmsBroadcastActivateMessage> msg);
        void handleCdmaSendSms(std::shared_ptr<RilRequestCdmaSendSmsMessage> msg);
        void handleCdmaSmsAcknowledge(std::shared_ptr<RilRequestAckCdmaSmsMessage> msg);
        void handleCdmaWriteSmsToRuim(std::shared_ptr<RilRequestCdmaWriteSmsToRuimMessage> msg);
        void handleCdmaDeleteSmsOnRuim(std::shared_ptr<RilRequestCdmaDeleteSmsOnRuimMessage> msg);
        void handleCdmaGetBroadcastSmsConfig(std::shared_ptr<RilRequestGetCdmaBroadcastConfigMessage> msg);
        void handleCdmaSetBroadcastSmsConfig(std::shared_ptr<RilRequestCdmaSetBroadcastSmsConfigMessage> msg);
        void handleCdmaSmsBroadcastActivation(std::shared_ptr<RilRequestCdmaSmsBroadcastActivateMessage> msg);
        void handleImsRegistrationState(std::shared_ptr<RilRequestGetImsRegistrationMessage> msg);
        void handleImsSendSms(std::shared_ptr<RilRequestImsSendSmsMessage> msg);
        void handleImsSmsAcknowledge(std::shared_ptr<RilRequestAckImsSmsMessage> msg);
        void handleSmsEndPointStatusIndMessage(std::shared_ptr<Message> msg);
        void handleSmsGetImsServiceStatusMessage(std::shared_ptr<SmsGetImsServiceStatusMessage> msg);
};

SmsModule& getSmsModule();
