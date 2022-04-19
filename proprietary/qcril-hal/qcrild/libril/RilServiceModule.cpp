/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#define TAG "RilServiceModule"
#include "RilServiceModule.h"

namespace android {
extern void storeNITZTimeData(const std::string& time);
}

RilServiceModule::RilServiceModule(qcril_instance_id_e_type instance)
        : mInstance(instance) {
    mName = "RilServiceModule";

    using std::placeholders::_1;
    mMessageHandler = {
        HANDLER(QcrilInitMessage, RilServiceModule::handleQcrilInit),
        HANDLER(RilUnsolIncoming3GppSMSMessage, RilServiceModule::handleIncoming3GppSMSMessage),
        HANDLER(RilUnsolIncoming3Gpp2SMSMessage, RilServiceModule::handleIncoming3Gpp2SMSMessage),
        HANDLER(RilUnsolNewSmsOnSimMessage, RilServiceModule::handleNewSmsOnSimMessage),
        HANDLER(RilUnsolNewSmsStatusReportMessage, RilServiceModule::handleNewSmsStatusReportMessage),
        HANDLER(RilUnsolNewBroadcastSmsMessage, RilServiceModule::handleNewBroadcastSmsMessage),
        HANDLER(RilUnsolStkCCAlphaNotifyMessage, RilServiceModule::handleStkCCAlphaNotifyMessage),
        HANDLER(RilUnsolCdmaRuimSmsStorageFullMessage, RilServiceModule::handleCdmaRuimSmsStorageFullMessage),
        HANDLER(RilUnsolSimSmsStorageFullMessage, RilServiceModule::handleSimSmsStorageFullMessage),
        HANDLER(RilUnsolImsNetworkStateChangedMessage, RilServiceModule::handleImsNetworkStateChangedMessage),

        HANDLER(QcRilUnsolCallStateChangeMessage, RilServiceModule::handleQcRilUnsolCallStateChangeMessage),
        HANDLER(QcRilUnsolCallRingingMessage, RilServiceModule::handleQcRilUnsolCallRingingMessage),
        HANDLER(QcRilUnsolSupplementaryServiceMessage, RilServiceModule::handleQcRilUnsolSupplementaryServiceMessage),
        HANDLER(QcRilUnsolSrvccStatusMessage, RilServiceModule::handleQcRilUnsolSrvccStatusMessage),
        HANDLER(QcRilUnsolRingbackToneMessage, RilServiceModule::handleQcRilUnsolRingbackToneMessage),
        HANDLER(QcRilUnsolCdmaOtaProvisionStatusMessage, RilServiceModule::handleQcRilUnsolCdmaOtaProvisionStatusMessage),
        HANDLER(QcRilUnsolCdmaCallWaitingMessage, RilServiceModule::handleQcRilUnsolCdmaCallWaitingMessage),
        HANDLER(QcRilUnsolSuppSvcNotificationMessage, RilServiceModule::handleQcRilUnsolSuppSvcNotificationMessage),
        HANDLER(QcRilUnsolOnUssdMessage, RilServiceModule::handleQcRilUnsolOnUssdMessage),
        HANDLER(QcRilUnsolCdmaInfoRecordMessage, RilServiceModule::handleQcRilUnsolCdmaInfoRecordMessage),

        HANDLER(RilAcknowledgeRequestMessage, RilServiceModule::handleAcknowledgeRequestMessage),
        HANDLER(RilUnsolNetworkStateChangedMessage, RilServiceModule::handleNetworkStateChangedMessage),
        HANDLER(RilUnsolNitzTimeReceivedMessage, RilServiceModule::handleNitzTimeReceivedMessage),
        HANDLER(RilUnsolVoiceRadioTechChangedMessage, RilServiceModule::handleVoiceRadioTechChangedMessage),
        HANDLER(RilUnsolNetworkScanResultMessage, RilServiceModule::handleNetworkScanResultMessage),
        HANDLER(RilUnsolSignalStrengthMessage, RilServiceModule::handleSignalStrengthMessage),
        HANDLER(RilUnsolEmergencyCallbackModeMessage, RilServiceModule::handleEmergencyCallbackModeMessage),
        HANDLER(RilUnsolRadioCapabilityMessage, RilServiceModule::handlelRadioCapabilityMessage),
        HANDLER(RilUnsolCdmaPrlChangedMessage, RilServiceModule::handleCdmaPrlChangedMessage),
        HANDLER(RilUnsolRestrictedStateChangedMessage, RilServiceModule::handleRestrictedStateChangedMessage),
        HANDLER(RilUnsolUiccSubsStatusChangedMessage, RilServiceModule::handleUiccSubsStatusChangedMessage),
        HANDLER(RilUnsolRadioStateChangedMessage, RilServiceModule::handleRadioStateChangedMessage),

        HANDLER(UimSimStatusChangedInd, RilServiceModule::handleUimSimStatusChangedInd),
        HANDLER(UimSimRefreshIndication, RilServiceModule::handleUimSimRefreshIndication),
        HANDLER(GstkUnsolIndMsg, RilServiceModule::handleGstkUnsolIndMsg),
    };
}

sp<RadioImpl> RilServiceModule::getRadioImpl()
{
    auto rs = getRadioService(mInstance);
    return rs;
}

void RilServiceModule::handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg) {
    QCRIL_LOG_INFO("Handling QcrilInitMessage %s", msg->dump().c_str());
}

void RilServiceModule::handleIncoming3GppSMSMessage(
        std::shared_ptr<RilUnsolIncoming3GppSMSMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendNewSms(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleIncoming3Gpp2SMSMessage(
        std::shared_ptr<RilUnsolIncoming3Gpp2SMSMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendNewCdmaSms(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleNewSmsOnSimMessage(
        std::shared_ptr<RilUnsolNewSmsOnSimMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendNewSmsOnSim(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleNewSmsStatusReportMessage(
        std::shared_ptr<RilUnsolNewSmsStatusReportMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendNewSmsStatusReport(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleNewBroadcastSmsMessage(
        std::shared_ptr<RilUnsolNewBroadcastSmsMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendNewBroadcastSms(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleStkCCAlphaNotifyMessage(
        std::shared_ptr<RilUnsolStkCCAlphaNotifyMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendStkCCAlphaNotify(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleCdmaRuimSmsStorageFullMessage(
        std::shared_ptr<RilUnsolCdmaRuimSmsStorageFullMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendCdmaRuimSmsStorageFull(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleSimSmsStorageFullMessage(
        std::shared_ptr<RilUnsolSimSmsStorageFullMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendSimSmsStorageFull(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleImsNetworkStateChangedMessage(
        std::shared_ptr<RilUnsolImsNetworkStateChangedMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendImsNetworkStateChanged(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleQcRilUnsolCallRingingMessage(
    std::shared_ptr<QcRilUnsolCallRingingMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

  if (msg && !msg->isIms()) {
    auto ri = getRadioImpl();
    if (ri) {
      // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
      android::grabPartialWakeLock();

      auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
      radioServiceRwlockPtr->lock_shared();

      ri->sendCallRing(msg);

      radioServiceRwlockPtr->unlock_shared();
    }
  }
}

void RilServiceModule::handleQcRilUnsolCallStateChangeMessage(
    std::shared_ptr<QcRilUnsolCallStateChangeMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

  if (msg && !msg->isIms()) {
    auto ri = getRadioImpl();
    if (ri) {
      // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
      android::grabPartialWakeLock();

      auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
      radioServiceRwlockPtr->lock_shared();

      ri->sendCallStateChanged(msg);

      radioServiceRwlockPtr->unlock_shared();
    }
  }
}

void RilServiceModule::handleQcRilUnsolSupplementaryServiceMessage(
    std::shared_ptr<QcRilUnsolSupplementaryServiceMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

  if (msg && !msg->isIms()) {
    auto ri = getRadioImpl();
    if (ri) {
      // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
      android::grabPartialWakeLock();

      auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
      radioServiceRwlockPtr->lock_shared();

      ri->sendOnSupplementaryServiceIndication(msg);

      radioServiceRwlockPtr->unlock_shared();
    }
  }
}
void RilServiceModule::handleQcRilUnsolSrvccStatusMessage(
    std::shared_ptr<QcRilUnsolSrvccStatusMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

  auto ri = getRadioImpl();
  if (ri) {
    // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
    android::grabPartialWakeLock();

    auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
    radioServiceRwlockPtr->lock_shared();

    ri->sendSrvccStateNotify(msg);

    radioServiceRwlockPtr->unlock_shared();
  }
}
void RilServiceModule::handleQcRilUnsolRingbackToneMessage(
    std::shared_ptr<QcRilUnsolRingbackToneMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

  if (msg && !msg->isIms()) {
    auto ri = getRadioImpl();
    if (ri) {
      // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
      android::grabPartialWakeLock();

      auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
      radioServiceRwlockPtr->lock_shared();

      ri->sendIndicateRingbackTone(msg);

      radioServiceRwlockPtr->unlock_shared();
    }
  }
}
void RilServiceModule::handleQcRilUnsolCdmaOtaProvisionStatusMessage(
    std::shared_ptr<QcRilUnsolCdmaOtaProvisionStatusMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

  auto ri = getRadioImpl();
  if (ri) {
    // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
    android::grabPartialWakeLock();

    auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
    radioServiceRwlockPtr->lock_shared();

    ri->sendCdmaOtaProvisionStatus(msg);

    radioServiceRwlockPtr->unlock_shared();
  }
}
void RilServiceModule::handleQcRilUnsolCdmaCallWaitingMessage(
    std::shared_ptr<QcRilUnsolCdmaCallWaitingMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

  auto ri = getRadioImpl();
  if (ri) {
    // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
    android::grabPartialWakeLock();

    auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
    radioServiceRwlockPtr->lock_shared();

    ri->sendCdmaCallWaiting(msg);

    radioServiceRwlockPtr->unlock_shared();
  }
}
void RilServiceModule::handleQcRilUnsolSuppSvcNotificationMessage(
    std::shared_ptr<QcRilUnsolSuppSvcNotificationMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

  if (msg && !msg->isIms()) {
    auto ri = getRadioImpl();
    if (ri) {
      // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
      android::grabPartialWakeLock();

      auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
      radioServiceRwlockPtr->lock_shared();

      ri->sendSuppSvcNotify(msg);

      radioServiceRwlockPtr->unlock_shared();
    }
  }
}
void RilServiceModule::handleQcRilUnsolOnUssdMessage(std::shared_ptr<QcRilUnsolOnUssdMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

  auto ri = getRadioImpl();
  if (ri) {
    // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
    android::grabPartialWakeLock();

    auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
    radioServiceRwlockPtr->lock_shared();

    ri->sendOnUssd(msg);

    radioServiceRwlockPtr->unlock_shared();
  }
}
void RilServiceModule::handleQcRilUnsolCdmaInfoRecordMessage(std::shared_ptr<QcRilUnsolCdmaInfoRecordMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

  auto ri = getRadioImpl();
  if (ri) {
    // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
    android::grabPartialWakeLock();

    auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
    radioServiceRwlockPtr->lock_shared();

    ri->sendCdmaInfoRec(msg);

    radioServiceRwlockPtr->unlock_shared();
  }
}

void RilServiceModule::handleUimSimStatusChangedInd(
        std::shared_ptr<UimSimStatusChangedInd> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendSimStatusChanged(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleUimSimRefreshIndication(
        std::shared_ptr<UimSimRefreshIndication> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendSimRefresh(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleGstkUnsolIndMsg(
        std::shared_ptr<GstkUnsolIndMsg> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendGstkIndication(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleAcknowledgeRequestMessage(
        std::shared_ptr<RilAcknowledgeRequestMessage> msg) {
    auto ri = getRadioImpl();
    if (ri) {
        ri->sendAcknowledgeRequest(msg);
    }
}

void RilServiceModule::handleNetworkStateChangedMessage(
        std::shared_ptr<RilUnsolNetworkStateChangedMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendNetworkStateChanged(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleNitzTimeReceivedMessage(
        std::shared_ptr<RilUnsolNitzTimeReceivedMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    int retVal = 0;
    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        retVal = ri->sendNitzTimeReceived(msg);

        radioServiceRwlockPtr->unlock_shared();
    }

    if (retVal != 0) {
        QCRIL_LOG_INFO("store the last reported NITZ time");
        android::storeNITZTimeData(msg->getNitzTime());
    }
}

void RilServiceModule::handleVoiceRadioTechChangedMessage(
        std::shared_ptr<RilUnsolVoiceRadioTechChangedMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendVoiceRadioTechChanged(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleNetworkScanResultMessage(
        std::shared_ptr<RilUnsolNetworkScanResultMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendNetworkScanResult(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleSignalStrengthMessage(
        std::shared_ptr<RilUnsolSignalStrengthMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendSignalStrength(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleEmergencyCallbackModeMessage(
        std::shared_ptr<RilUnsolEmergencyCallbackModeMessage> msg) {
    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendEmergencyCallbackMode(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handlelRadioCapabilityMessage(
        std::shared_ptr<RilUnsolRadioCapabilityMessage> msg) {
    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendRadioCapability(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleCdmaPrlChangedMessage(
        std::shared_ptr<RilUnsolCdmaPrlChangedMessage> msg) {
    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendCdmaPrlChanged(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleRestrictedStateChangedMessage(
        std::shared_ptr<RilUnsolRestrictedStateChangedMessage> msg) {
    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendRestrictedStateChanged(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleUiccSubsStatusChangedMessage(
        std::shared_ptr<RilUnsolUiccSubsStatusChangedMessage> msg) {
    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendUiccSubsStatusChanged(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule::handleRadioStateChangedMessage(
        std::shared_ptr<RilUnsolRadioStateChangedMessage> msg) {
    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will acknowldge this UNSOL, which leads to release this wakelock
        android::grabPartialWakeLock();

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        ri->sendRadioStateChanged(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}
