/******************************************************************************
#  Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/Module.h"
#include <framework/AddPendingMessageList.h>
#include "modules/qmi/DmsModemEndPoint.h"
#include "modules/qmi/QmiAsyncResponseMessage.h"
#include "modules/qmi/EndpointStatusIndMessage.h"

#include <modules/android/RilRequestMessage.h>

#include "interfaces/dms/RilRequestRadioPowerMessage.h"
#include "interfaces/dms/RilRequestGetModemActivityMessage.h"
#include "interfaces/dms/RilRequestGetDeviceIdentityMessage.h"
#include "interfaces/dms/RilRequestGetBaseBandVersionMessage.h"

#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

class DmsModule : public Module, public AddPendingMessageList {
  public:
    DmsModule();
    ~DmsModule();
    void init();
#ifdef QMI_RIL_UTF
    void qcrilHalDmsModuleCleanup();
#endif

  private:
    bool mReady = false;
    qtimutex::QtiRecursiveMutex mMutex;

  private:

    void handleQcrilInit(std::shared_ptr<Message> msg);
    void handleDmsEndpointStatusIndMessage(std::shared_ptr<Message> msg);

    void handleRadioPowerRequest(std::shared_ptr<RilRequestRadioPowerMessage> shared_msg);
    void handleGetActivityInfoRequest(std::shared_ptr<RilRequestGetModemActivityMessage> shared_msg);
    void handleBaseBandVersionRequest(std::shared_ptr<RilRequestGetBaseBandVersionMessage> shared_msg);
    void handleDeviceIdentiyRequest(std::shared_ptr<RilRequestGetDeviceIdentityMessage> shared_msg);
};

DmsModule &getDmsModule();

