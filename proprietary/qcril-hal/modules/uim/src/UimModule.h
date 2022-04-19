/******************************************************************************
#  @file    UimModem.h
#  @brief   Base class header file for UIM module
#
#  ---------------------------------------------------------------------------
#
#  Copyright (c) 2017 - 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#  ---------------------------------------------------------------------------
#******************************************************************************/

#pragma once
#include "framework/Module.h"
#include "framework/PendingMessageList.h"
#include "modules/uim/UimModemEndPoint.h"
#include "modules/uim/UimQmiUimRequestMsg.h"
#include "modules/uim/UimQmiUimResponseMsg.h"
#include "modules/uim/UimQmiUimIndicationMsg.h"
#include "modules/uim/UimChangeSubscriptionReqMsg.h"
#include "modules/uim/UimGetImsiRequestSyncMsg.h"
#include "modules/uim/UimGetCardStatusRequestSyncMsg.h"
#include "modules/uim/UimGetIccIdRequestSyncMsg.h"
#include "modules/uim/UimGetFacilityLockRequestMsg.h"
#include "modules/uim/UimSetFacilityLockRequestMsg.h"
#include "modules/uim/UimGetAtrRequestMsg.h"
#include "modules/uim/UimGetGIDRequestMsg.h"
#include "modules/uim/UimGetMccMncRequestMsg.h"

#include "modules/uim/UimGetCardStatusRequestMsg.h"
#include "modules/uim/UimEnterSimPinRequestMsg.h"
#include "modules/uim/UimEnterSimPukRequestMsg.h"
#include "modules/uim/UimChangeSimPinRequestMsg.h"
#include "modules/uim/UimEnterDePersoRequestMsg.h"
#include "modules/uim/UimISIMAuthenticationRequestMsg.h"
#include "modules/uim/UimSIMAuthenticationRequestMsg.h"
#include "modules/uim/UimGetImsiRequestMsg.h"
#include "modules/uim/UimSIMIORequestMsg.h"
#include "modules/uim/UimTransmitAPDURequestMsg.h"
#include "modules/uim/UimSIMOpenChannelRequestMsg.h"
#include "modules/uim/UimSIMCloseChannelRequestMsg.h"
#include "modules/uim/UimCardPowerReqMsg.h"

#include "modules/uim/UimVoltageStatusRequestMsg.h"
#include "modules/uim/UimPersoRequestMsg.h"
#include "modules/uim/UimGetPersoStatusRequestMsg.h"
#include "modules/uim/UimGetLPATermialCapabilitesRequestMsg.h"
#include "modules/uim/UimSetLPATermialCapabilitesRequestMsg.h"
#include "modules/uim/UimSetAPDUBehaviorRequestMsg.h"
#include "modules/uim/UimGBAGetIMPIRequestMsg.h"
#include "modules/uim/UimGBAInitRequestMsg.h"
#include "modules/uim/UimLpaReqMessage.h"
#include "modules/uim/UimSAPConnectionRequestMsg.h"
#include "modules/uim/UimSAPRequestMsg.h"
#include "modules/uim/UimRemoteSimlockRequestMsg.h"
#include "modules/uim/UimGetPhysicalSlotMappingRequestMsg.h"
#include "modules/uim/UimSwitchSlotRequestMsg.h"
#include "modules/uim/UimGetSlotStatusRequestMsg.h"

#include "UimTimerExpiryMsg.h"
#include <queue>

/*=========================================================================

  CLASS:  UimModule

===========================================================================*/
class UimModule : public Module
{
  private:
    bool                                               mReady;
    bool                                               mRestart;
    std::queue<std::shared_ptr<UimQmiUimRequestMsg>>   mQmiPendingList;
    PendingMessageList                                 uim_req_list;
    uint8_t                                            mQmiAsyncMsgCount;
    std::shared_ptr<UimModemEndPoint>                  mUimModemEndPoint;

  public:
    UimModule();
    ~UimModule();
    void init();
    void qcrilHalUimModuleCleanup();
    int UimSendQmiUimRequest(uint32_t msg_id, const void * params, std::shared_ptr<Message> req_ptr);

private:
    void UimProcessRilInit(std::shared_ptr<Message> msg_ptr);
    void UimHandleTimerRequest(std::shared_ptr<Message> msg_ptr);
    void UimHandleTimerExpiry(std::shared_ptr<UimTimerExpiryMsg> msg_ptr);
    void UimProcessQmiCommandCallback(std::shared_ptr<UimQmiUimResponseMsg> msg);
    void UimProcessQmiIndiaction(std::shared_ptr<UimQmiUimIndicationMsg> msg);

    void UimProcessChangeSubscription(std::shared_ptr<UimChangeSubscriptionReqMsg> msg);
    void UimProcessGetImsiReq(std::shared_ptr<UimGetImsiRequestSyncMsg> msg);
    void UimProcessGetCardStatusSyncReq(std::shared_ptr<UimGetCardStatusRequestSyncMsg> msg);
    void UimProcessGetIccIdSycnReq(std::shared_ptr<UimGetIccIdRequestSyncMsg> msg);
    void UimProcessGetCardStatusReq(std::shared_ptr<UimGetCardStatusRequestSyncMsg> msg);
    void UimProcessGetFacilityLockStatusReq(std::shared_ptr<UimGetFacilityLockRequestMsg> msg);
    void UimProcessSetFacilityLockReq(std::shared_ptr<UimSetFacilityLockRequestMsg> msg);
    void UimProcessGetAtrReq(std::shared_ptr<UimGetAtrRequestMsg> msg);
    void UimProcessGetGIDReq(std::shared_ptr<UimGetGIDRequestMsg> msg);
    void handleUimGetMccMncRequestMsg(std::shared_ptr<UimGetMccMncRequestMsg> req_ptr);

    void handleGetCardStatusRequestMsg(std::shared_ptr<UimGetCardStatusRequestMsg> req_ptr);
    void handleEnterSimPinRequestMsg(std::shared_ptr<UimEnterSimPinRequestMsg> req_ptr);
    void handleEnterSimPukRequestMsg(std::shared_ptr<UimEnterSimPukRequestMsg> req_ptr);
    void handleChangeSimPinRequestMsg(std::shared_ptr<UimChangeSimPinRequestMsg> req_ptr);
    void handleEnterDePersoRequestMsg(std::shared_ptr<UimEnterDePersoRequestMsg> req_ptr);
    void handleISIMAuthenticationRequestMsg(std::shared_ptr<UimISIMAuthenticationRequestMsg> req_ptr);
    void handleSIMAuthenticationRequestMsg(std::shared_ptr<UimSIMAuthenticationRequestMsg> req_ptr);
    void handleGetImsiRequestMsg(std::shared_ptr<UimGetImsiRequestMsg> req_ptr);
    void handleSIMIORequestMsg(std::shared_ptr<UimSIMIORequestMsg> req_ptr);
    void handleTransmitAPDURequestMsg(std::shared_ptr<UimTransmitAPDURequestMsg> req_ptr);
    void handleSIMOpenChannelRequestMsg(std::shared_ptr<UimSIMOpenChannelRequestMsg> req_ptr);
    void handleSIMCloseChannelRequestMsg(std::shared_ptr<UimSIMCloseChannelRequestMsg> req_ptr);
    void handleUimCardPowerReqMsg(std::shared_ptr<UimCardPowerReqMsg> req_ptr);

    void handleUimVoltageStatusRequestMsg(std::shared_ptr<UimVoltageStatusRequestMsg> req_ptr);
    void handleUimPersoRequestMsg(std::shared_ptr<UimPersoRequestMsg> req_ptr);
    void handleUimGetPersoStatusRequestMsg(std::shared_ptr<UimGetPersoStatusRequestMsg> req_ptr);
    void handleUimGetLPATermialCapabilitesRequestMsg(std::shared_ptr<UimGetLPATermialCapabilitesRequestMsg> req_ptr);
    void handleUimSetLPATermialCapabilitesRequestMsg(std::shared_ptr<UimSetLPATermialCapabilitesRequestMsg> req_ptr);
    void handleUimSetAPDUBehaviorRequestMsg(std::shared_ptr<UimSetAPDUBehaviorRequestMsg> req_ptr);
    void handleUimGBAGetIMPIRequestMsg(std::shared_ptr<UimGBAGetIMPIRequestMsg> req_ptr);
    void handleUimGBAInitRequestMsg(std::shared_ptr<UimGBAInitRequestMsg> req_ptr);
    void uim_process_lpa_request(std::shared_ptr<UimLpaReqMessage> msg);
    void handleUimSAPConnectionRequestMsg(std::shared_ptr<UimSAPConnectionRequestMsg> req_ptr);
    void handleUimSAPRequestMsg(std::shared_ptr<UimSAPRequestMsg> req_ptr);
    void handleUimRemoteSimlockRequestMsg(std::shared_ptr<UimRemoteSimlockRequestMsg> req_ptr);
    void handleUimGetPhysicalSlotMappingRequestMsg(std::shared_ptr<UimGetPhysicalSlotMappingRequestMsg> req_ptr);
    void handleUimSwitchSlotRequestMsg(std::shared_ptr<UimSwitchSlotRequestMsg> req_ptr);
    void handleUimGetSlotStatusRequestMsg(std::shared_ptr<UimGetSlotStatusRequestMsg> req_ptr);
};

UimModule *getUimModule();
