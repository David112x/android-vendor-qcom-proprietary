/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include <framework/Module.h>
#include <framework/QcrilInitMessage.h>
#include <interfaces/ims/QcRilUnsolImsRegStateMessage.h>
#include <interfaces/voice/QcRilUnsolOnUssdMessage.h>
#include <interfaces/voice/QcRilUnsolConfParticipantStatusInfoMessage.h>
#include <interfaces/voice/QcRilUnsolCallRingingMessage.h>
#include <interfaces/voice/QcRilUnsolImsCallStatusMessage.h>
#include <interfaces/voice/QcRilUnsolImsConferenceInfoMessage.h>
#include <interfaces/voice/QcRilUnsolImsHandoverMessage.h>
#include <interfaces/voice/QcRilUnsolImsModifyCallMessage.h>
#include <interfaces/voice/QcRilUnsolRingbackToneMessage.h>
#include <interfaces/voice/QcRilUnsolImsTtyNotificationMessage.h>
#include <interfaces/voice/QcRilUnsolImsViceInfoMessage.h>
#include <interfaces/sms/QcRilUnsolMessageWaitingInfoMessage.h>
#include <modules/android_ims_radio/hidl_impl/base/qcril_qmi_ims_radio_service.h>
#include "interfaces/ims/QcRilUnsolImsRttMessage.h"
#include "interfaces/ims/QcRilUnsolImsVowifiCallQuality.h"
#include "interfaces/ims/QcRilUnsolImsVopsIndication.h"
#include "interfaces/ims/QcRilUnsolImsSubConfigIndication.h"
#include "interfaces/ims/QcRilUnsolImsGeoLocationInfo.h"
#include "interfaces/ims/QcRilUnsolImsSrvStatusIndication.h"
#include "interfaces/ims/QcRilUnsolImsSsacInfoIndication.h"
#include "interfaces/ims/QcRilUnsolImsRegBlockStatusMessage.h"
#include "modules/sms/RilUnsolIncomingImsSMSMessage.h"
#include "modules/sms/RilUnsolNewImsSmsStatusReportMessage.h"
#include "interfaces/ims/QcRilUnsolImsPendingMultiLineStatus.h"
#include "interfaces/ims/QcRilUnsolImsMultiIdentityStatusMessage.h"
#include "interfaces/ims/QcRilUnsolImsVoiceInfo.h"
#include "interfaces/ims/QcRilUnsolImsEnterEcbmIndication.h"
#include "interfaces/ims/QcRilUnsolImsExitEcbmIndication.h"
#include "interfaces/ims/QcRilUnsolImsRadioStateIndication.h"
#include "interfaces/ims/QcRilUnsolImsWfcRoamingConfigIndication.h"

class AndroidImsRadioModule : public Module {
 public:
  AndroidImsRadioModule();
  ~AndroidImsRadioModule();
  void init();
#ifdef QMI_RIL_UTF
  void cleanup();
#endif

 private:
  void handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg);
  void handleQcRilUnsolImsRegStateMessage(std::shared_ptr<QcRilUnsolImsRegStateMessage> msg);
  void handleQcRilUnsolImsRttMessage(std::shared_ptr<QcRilUnsolImsRttMessage> msg);
  void handleQcRilUnsolImsVowifiCallQuality(std::shared_ptr<QcRilUnsolImsVowifiCallQuality> msg);
  void handleQcRilUnsolVopsMessage(std::shared_ptr<QcRilUnsolImsVopsIndication> msg);
  void handleQcRilUnsolSubConfigIndication(std::shared_ptr<QcRilUnsolImsSubConfigIndication> msg);
  void handleQcRilUnsolGeoLocationChange(std::shared_ptr<QcRilUnsolImsGeoLocationInfo> msg);
  void handleQcRilUnsolServiceStatusChange(std::shared_ptr<QcRilUnsolImsSrvStatusIndication> msg);
  void handleQcRilUnsolSsacInfoIndication(std::shared_ptr<QcRilUnsolImsSsacInfoIndication> msg);
  void handleQcRilUnsolRegBlockChange(std::shared_ptr<QcRilUnsolImsRegBlockStatusMessage> msg);
  void handleQcRilUnsolImsCallStatusMessage(std::shared_ptr<QcRilUnsolImsCallStatusMessage> msg);
  void handleQcRilUnsolImsModifyCallMessage(std::shared_ptr<QcRilUnsolImsModifyCallMessage> msg);
  void handleQcRilUnsolCallRingingMessage(std::shared_ptr<QcRilUnsolCallRingingMessage> msg);
  void handleQcRilUnsolRingbackToneMessage(
      std::shared_ptr<QcRilUnsolRingbackToneMessage> msg);
  void handleQcRilUnsolImsHandoverMessage(std::shared_ptr<QcRilUnsolImsHandoverMessage> msg);
  void handleQcRilUnsolImsTtyNotificationMessage(
      std::shared_ptr<QcRilUnsolImsTtyNotificationMessage> msg);
  void handleQcRilUnsolImsConferenceInfoMessage(
      std::shared_ptr<QcRilUnsolImsConferenceInfoMessage> msg);
  void handleQcRilUnsolImsViceInfoMessage(std::shared_ptr<QcRilUnsolImsViceInfoMessage> msg);
  void handleQcRilUnsolConfParticipantStatusInfoMessage(
      std::shared_ptr<QcRilUnsolConfParticipantStatusInfoMessage> msg);
  void handleQcRilUnsolAutoCallRejectionMessage(
      std::shared_ptr<QcRilUnsolAutoCallRejectionMessage> msg);
  void handleQcRilUnsolSuppSvcNotificationMessage(
      std::shared_ptr<QcRilUnsolSuppSvcNotificationMessage> msg);
  void handleQcRilUnsolSupplementaryServiceMessage(
      std::shared_ptr<QcRilUnsolSupplementaryServiceMessage> msg);
  void handleQcRilUnsolIncomingImsSMSMessage(std::shared_ptr<RilUnsolIncomingImsSMSMessage> msg);
  void handleQcRilNewImsSmsStatusReportMessage(
      std::shared_ptr<RilUnsolNewImsSmsStatusReportMessage> msg);
  void handleQcRilUnsolVoiceInfoChange(std::shared_ptr<QcRilUnsolImsVoiceInfo> msg);
  void handleQcRilUnsolExitEcbmIndication(std::shared_ptr<QcRilUnsolImsExitEcbmIndication> msg);
  void handleQcRilUnsolEnterEcbmIndication(std::shared_ptr<QcRilUnsolImsEnterEcbmIndication> msg);
  void handleQcRilUnsolRadioStateIndication(std::shared_ptr<QcRilUnsolImsRadioStateIndication> msg);
  void handleQcRilUnsolMessageWaitingInfoMessage(
      std::shared_ptr<QcRilUnsolMessageWaitingInfoMessage> msg);
  void handleQcRilUnsolWfcRoamingConfigSupport(std::shared_ptr<QcRilUnsolImsWfcRoamingConfigIndication> msg);
  void handleQcRilUnsolOnUssdMessage(std::shared_ptr<QcRilUnsolOnUssdMessage> msg);

  void handleQcRilUnsolPendingMultiLineStatus(
    std::shared_ptr<QcRilUnsolImsPendingMultiLineStatus> msg);
  void handleQcRilUnsolMultiIdentityStatus(
    std::shared_ptr<QcRilUnsolImsMultiIdentityStatusMessage> msg);
  void initImsRadio(qcril_instance_id_e_type instance_id);

  android::sp<ImsRadioImplBase> mImsRadio;
};

#ifdef QMI_RIL_UTF
void qcril_qmi_hal_android_ims_radio_module_cleanup();
#endif
