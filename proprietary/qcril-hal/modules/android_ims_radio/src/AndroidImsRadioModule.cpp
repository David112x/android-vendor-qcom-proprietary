/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#define TAG "RILQ"

#include <AndroidImsRadioModule.h>
#include <framework/Log.h>
#include <modules/android_ims_radio/hidl_impl/1.5/qcril_qmi_ims_radio_service_1_5.h>
#include <modules/android_ims_radio/hidl_impl/1.6/qcril_qmi_ims_radio_service_1_6.h>
#include <modules/android_ims_radio/hidl_impl/base/qcril_qmi_ims_radio_service.h>
#include <cstring>
extern "C" {
#include "qcril_other.h"
}

#define QCRIL_HIDL_DEV_SERVICE_VERSION "persist.vendor.radio.hidl_dev_service"

static load_module<AndroidImsRadioModule> sAndroidImsRadioModule;

AndroidImsRadioModule *getAndroidImsRadioModule() {
  return &(sAndroidImsRadioModule.get_module());
}

/*
 * 1. Indicate your preference for looper.
 * 2. Subscribe to the list of messages via mMessageHandler.
 * 3. Follow RAII practice.
 */
AndroidImsRadioModule::AndroidImsRadioModule() {
  mName = "AndroidImsRadioModule";

  using std::placeholders::_1;
  mMessageHandler = {
      HANDLER(QcrilInitMessage, AndroidImsRadioModule::handleQcrilInit),
      HANDLER(QcRilUnsolImsRegStateMessage,
              AndroidImsRadioModule::handleQcRilUnsolImsRegStateMessage),
      HANDLER(QcRilUnsolImsRttMessage,
              AndroidImsRadioModule::handleQcRilUnsolImsRttMessage),
      HANDLER(QcRilUnsolImsVowifiCallQuality,
              AndroidImsRadioModule::handleQcRilUnsolImsVowifiCallQuality),
      HANDLER(QcRilUnsolImsVopsIndication,
              AndroidImsRadioModule::handleQcRilUnsolVopsMessage),
      HANDLER(QcRilUnsolImsSubConfigIndication,
              AndroidImsRadioModule::handleQcRilUnsolSubConfigIndication),
      HANDLER(QcRilUnsolImsGeoLocationInfo,
              AndroidImsRadioModule::handleQcRilUnsolGeoLocationChange),
      HANDLER(QcRilUnsolImsSrvStatusIndication,
              AndroidImsRadioModule::handleQcRilUnsolServiceStatusChange),
      HANDLER(QcRilUnsolImsSsacInfoIndication,
              AndroidImsRadioModule::handleQcRilUnsolSsacInfoIndication),
      HANDLER(QcRilUnsolImsRegBlockStatusMessage,
              AndroidImsRadioModule::handleQcRilUnsolRegBlockChange),
      HANDLER(QcRilUnsolImsCallStatusMessage,
              AndroidImsRadioModule::handleQcRilUnsolImsCallStatusMessage),
      HANDLER(QcRilUnsolImsModifyCallMessage,
              AndroidImsRadioModule::handleQcRilUnsolImsModifyCallMessage),
      HANDLER(QcRilUnsolCallRingingMessage,
              AndroidImsRadioModule::handleQcRilUnsolCallRingingMessage),
      HANDLER(QcRilUnsolRingbackToneMessage,
              AndroidImsRadioModule::handleQcRilUnsolRingbackToneMessage),
      HANDLER(QcRilUnsolImsHandoverMessage,
              AndroidImsRadioModule::handleQcRilUnsolImsHandoverMessage),
      HANDLER(QcRilUnsolImsTtyNotificationMessage,
              AndroidImsRadioModule::handleQcRilUnsolImsTtyNotificationMessage),
      HANDLER(QcRilUnsolImsConferenceInfoMessage,
              AndroidImsRadioModule::handleQcRilUnsolImsConferenceInfoMessage),
      HANDLER(QcRilUnsolImsViceInfoMessage,
              AndroidImsRadioModule::handleQcRilUnsolImsViceInfoMessage),
      HANDLER(QcRilUnsolConfParticipantStatusInfoMessage,
              AndroidImsRadioModule::handleQcRilUnsolConfParticipantStatusInfoMessage),
      HANDLER(QcRilUnsolAutoCallRejectionMessage,
              AndroidImsRadioModule::handleQcRilUnsolAutoCallRejectionMessage),
      HANDLER(QcRilUnsolSuppSvcNotificationMessage,
              AndroidImsRadioModule::handleQcRilUnsolSuppSvcNotificationMessage),
      HANDLER(QcRilUnsolSupplementaryServiceMessage,
              AndroidImsRadioModule::handleQcRilUnsolSupplementaryServiceMessage),
      HANDLER(RilUnsolIncomingImsSMSMessage,
              AndroidImsRadioModule::handleQcRilUnsolIncomingImsSMSMessage),
      HANDLER(RilUnsolNewImsSmsStatusReportMessage,
              AndroidImsRadioModule::handleQcRilNewImsSmsStatusReportMessage),
      HANDLER(QcRilUnsolImsPendingMultiLineStatus,
              AndroidImsRadioModule::handleQcRilUnsolPendingMultiLineStatus),
      HANDLER(QcRilUnsolImsMultiIdentityStatusMessage,
              AndroidImsRadioModule::handleQcRilUnsolMultiIdentityStatus),
      HANDLER(QcRilUnsolImsVoiceInfo,
              AndroidImsRadioModule::handleQcRilUnsolVoiceInfoChange),
      HANDLER(QcRilUnsolImsExitEcbmIndication,
              AndroidImsRadioModule::handleQcRilUnsolExitEcbmIndication),
      HANDLER(QcRilUnsolImsEnterEcbmIndication,
              AndroidImsRadioModule::handleQcRilUnsolEnterEcbmIndication),
      HANDLER(QcRilUnsolImsRadioStateIndication,
              AndroidImsRadioModule::handleQcRilUnsolRadioStateIndication),
      HANDLER(QcRilUnsolMessageWaitingInfoMessage,
              AndroidImsRadioModule::handleQcRilUnsolMessageWaitingInfoMessage),
      HANDLER(QcRilUnsolImsWfcRoamingConfigIndication,
              AndroidImsRadioModule::handleQcRilUnsolWfcRoamingConfigSupport),
      HANDLER(QcRilUnsolOnUssdMessage,
              AndroidImsRadioModule::handleQcRilUnsolOnUssdMessage),
  };
}

/* Follow RAII.
 */
AndroidImsRadioModule::~AndroidImsRadioModule() {}

/*
 * Module specific initialization that does not belong to RAII .
 */
void AndroidImsRadioModule::init() {
  Module::init();
}

/*
 * List of individual private handlers for the subscribed messages.
 */
void AndroidImsRadioModule::handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  Log::getInstance().d("[" + mName +
                       "]: get_instance_id = " + std::to_string(msg->get_instance_id()));
  /* Init ims radio services.*/
  initImsRadio(msg->get_instance_id());
}

void AndroidImsRadioModule::handleQcRilUnsolImsRegStateMessage(
    std::shared_ptr<QcRilUnsolImsRegStateMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnRegistrationChanged(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolImsRttMessage(
    std::shared_ptr<QcRilUnsolImsRttMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnRTTMessage(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolImsVowifiCallQuality(
    std::shared_ptr<QcRilUnsolImsVowifiCallQuality> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnVowifiCallQuality(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolVopsMessage(
    std::shared_ptr<QcRilUnsolImsVopsIndication> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnVopsChanged(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolSubConfigIndication(
    std::shared_ptr<QcRilUnsolImsSubConfigIndication> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnSubConfigChanged(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolGeoLocationChange(
    std::shared_ptr<QcRilUnsolImsGeoLocationInfo> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnGeoLocationChange(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolServiceStatusChange(
    std::shared_ptr<QcRilUnsolImsSrvStatusIndication> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnServiceStatusChange(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolSsacInfoIndication(
    std::shared_ptr<QcRilUnsolImsSsacInfoIndication> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnSsacInfoChange(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolRegBlockChange(
    std::shared_ptr<QcRilUnsolImsRegBlockStatusMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnRegBlockStatusChange(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolImsCallStatusMessage(
    std::shared_ptr<QcRilUnsolImsCallStatusMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnCallStateChanged(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolImsModifyCallMessage(
    std::shared_ptr<QcRilUnsolImsModifyCallMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnModifyCall(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolCallRingingMessage(
    std::shared_ptr<QcRilUnsolCallRingingMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (msg && msg->isIms()) {
    if (mImsRadio != nullptr) {
      mImsRadio->notifyOnRing(msg);
    }
  }
}


void AndroidImsRadioModule::handleQcRilUnsolRingbackToneMessage(
    std::shared_ptr<QcRilUnsolRingbackToneMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (msg && msg->isIms()) {
    if (mImsRadio != nullptr) {
      mImsRadio->notifyOnRingbackTone(msg);
    }
  }
}

void AndroidImsRadioModule::handleQcRilUnsolImsHandoverMessage(
    std::shared_ptr<QcRilUnsolImsHandoverMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnHandover(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolImsTtyNotificationMessage(
    std::shared_ptr<QcRilUnsolImsTtyNotificationMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnTtyNotification(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolImsConferenceInfoMessage(
    std::shared_ptr<QcRilUnsolImsConferenceInfoMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnRefreshConferenceInfo(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolImsViceInfoMessage(
    std::shared_ptr<QcRilUnsolImsViceInfoMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnRefreshViceInfo(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolConfParticipantStatusInfoMessage(
    std::shared_ptr<QcRilUnsolConfParticipantStatusInfoMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnParticipantStatusInfo(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolAutoCallRejectionMessage(
    std::shared_ptr<QcRilUnsolAutoCallRejectionMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnIncomingCallAutoRejected(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolSuppSvcNotificationMessage(
    std::shared_ptr<QcRilUnsolSuppSvcNotificationMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio && msg && msg->isIms()) {
    mImsRadio->notifyOnSuppServiceNotification(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolSupplementaryServiceMessage(
    std::shared_ptr<QcRilUnsolSupplementaryServiceMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio && msg && msg->isIms()) {
    mImsRadio->notifyOnSupplementaryServiceIndication(msg);
  }
}
void AndroidImsRadioModule::handleQcRilUnsolPendingMultiLineStatus(
    std::shared_ptr<QcRilUnsolImsPendingMultiLineStatus> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnPendingMultiIdentityStatus(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolMultiIdentityStatus(
    std::shared_ptr<QcRilUnsolImsMultiIdentityStatusMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnMultiIdentityLineStatus(msg);
  }
}


void AndroidImsRadioModule::handleQcRilUnsolIncomingImsSMSMessage(
    std::shared_ptr<RilUnsolIncomingImsSMSMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyIncomingImsSms(msg);
  }
}

void AndroidImsRadioModule::handleQcRilNewImsSmsStatusReportMessage(
    std::shared_ptr<RilUnsolNewImsSmsStatusReportMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyNewImsSmsStatusReport(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolVoiceInfoChange(
    std::shared_ptr<QcRilUnsolImsVoiceInfo> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnVoiceInfoStatusChange(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolExitEcbmIndication(
    std::shared_ptr<QcRilUnsolImsExitEcbmIndication> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnExitEcbmIndication(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolEnterEcbmIndication(
    std::shared_ptr<QcRilUnsolImsEnterEcbmIndication> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnEnterEcbmIndication(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolRadioStateIndication(
    std::shared_ptr<QcRilUnsolImsRadioStateIndication> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnRadioStateChanged(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolMessageWaitingInfoMessage(
    std::shared_ptr<QcRilUnsolMessageWaitingInfoMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnMessageWaiting(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolWfcRoamingConfigSupport(
    std::shared_ptr<QcRilUnsolImsWfcRoamingConfigIndication> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnModemSupportsWfcRoamingModeConfiguration(msg);
  }
}

void AndroidImsRadioModule::handleQcRilUnsolOnUssdMessage(
    std::shared_ptr<QcRilUnsolOnUssdMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mImsRadio != nullptr) {
    mImsRadio->notifyOnUssdMessageFailed(msg);
  }
}

void AndroidImsRadioModule::initImsRadio(qcril_instance_id_e_type instance_id) {
  boolean useHidlDevVersion = FALSE;
  if (mImsRadio == nullptr) {
    qmi_ril_get_property_value_from_boolean(QCRIL_HIDL_DEV_SERVICE_VERSION, &useHidlDevVersion,
                                            FALSE);
    if (useHidlDevVersion) {
      // Create development version of IImsRadio hidl service.
      // mImsRadio = new
      //      vendor::qti::hardware::radio::ims::V1_5::implementation::ImsRadioImpl_1_5();
      // mImsRadio = new
      //      vendor::qti::hardware::radio::ims::V1_4::implementation::ImsRadioImpl_1_4();
      // mImsRadio = new
      //      vendor::qti::hardware::radio::ims::V1_0::implementation::ImsRadioImpl();
      mImsRadio = new vendor::qti::hardware::radio::ims::V1_6::implementation::ImsRadioImpl_1_6(
        instance_id);
    }
    if (mImsRadio == nullptr) {
      // Create freezed version of IImsRadio hidl service.
      mImsRadio = new vendor::qti::hardware::radio::ims::V1_5::implementation::ImsRadioImpl_1_5(
          instance_id);
    }
    if (mImsRadio != nullptr) {
      mImsRadio->registerService();
    }
  }
}

#ifdef QMI_RIL_UTF
void AndroidImsRadioModule::cleanup() {
  if (mImsRadio) {
    // mImsRadio->clearCallbacks();
  }
}

void qcril_qmi_hal_android_ims_radio_module_cleanup() {
  QCRIL_LOG_INFO("qcril_qmi_hal_android_ims_radio_module_cleanup");
  getAndroidImsRadioModule()->cleanup();
}
#endif
