/******************************************************************************
  @file    qcril_qmi_oemhook_service.h
  @brief   qcril qmi - oemhook_service

  DESCRIPTION
    Implements the server side of the IQtiOemHook interface.

  ---------------------------------------------------------------------------

  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef VENDOR_QCOM_HARDWARE_QCRIL_QCRILHOOK_V1_0_QTIOEMHOOK_H
#define VENDOR_QCOM_HARDWARE_QCRIL_QCRILHOOK_V1_0_QTIOEMHOOK_H

#include <vendor/qti/hardware/radio/qcrilhook/1.0/IQtiOemHook.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <telephony/ril.h>
#include <QtiMutex.h>
#include <interfaces/pbm/pbm.h>
#include <interfaces/pbm/QcRilUnsolAdnRecordsOnSimMessage.h>
#include <interfaces/pbm/QcRilUnsolAdnInitDoneMessage.h>
#include <interfaces/mbn/QcRilUnsolMbnConfigResultMessage.h>
#include <interfaces/mbn/QcRilUnsolMbnConfigClearedMessage.h>
#include <interfaces/mbn/QcRilUnsolMbnValidateDumpedMessage.h>
#include <interfaces/mbn/QcRilUnsolMbnConfigListMessage.h>
#include <interfaces/mbn/QcRilUnsolMbnValidateConfigMessage.h>
#include "OemHookContext.h"
#include <modules/uim/UimSimlockTempUnlockExpireInd.h>
#include <modules/uim/UimCardStateChangeInd.h>
#include <modules/uim/UimSlotStatusInd.h>
#include <modules/uim/UimSimRefreshIndication.h>
#include <modules/uim/UimVoltageStatusInd.h>
#include <modules/uim_remote/UimRmtRemoteSimStatusIndMsg.h>
#include <modules/uim/UimOemHook.h>
#include <interfaces/voice/QcRilUnsolDtmfMessage.h>
#include <interfaces/voice/QcRilUnsolExtBurstIntlMessage.h>
#include <interfaces/voice/QcRilUnsolNssReleaseMessage.h>
#include <interfaces/voice/QcRilUnsolSuppSvcErrorCodeMessage.h>
#include <interfaces/voice/QcRilUnsolSpeechCodecInfoMessage.h>
#include <interfaces/nas/RilRequestStartNetworkScanMessage.h>
#include <interfaces/nas/RilRequestStopNetworkScanMessage.h>
#include "OemHookContext.h"

extern "C" {
   #include "qcrili.h"
}

void processIncomingOemhookMessage
(
    qcril_instance_id_e_type oemSocketInstanceId,
    int serial,
    unsigned char *data,
    size_t dataLen
);

using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::sp;
using ::android::wp;

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qcrilhook {
namespace V1_0 {
namespace implementation {

class OemHookImpl;

uint8_t oemHookRegisterService(qcril_instance_id_e_type oemSocketInstanceId);
sp<OemHookImpl> getOemHookService(qcril_instance_id_e_type oemSocketInstanceId);

void sendOemhookResponse
(
    qcril_instance_id_e_type oemSocketInstanceId,
    int32_t serial,
    RIL_Errno error,
    unsigned char* buf,
    size_t bufLen
);

void sendOemhookIndication
(
    qcril_instance_id_e_type oemSocketInstanceId,
    unsigned char *buf,
    size_t bufLen
);

class OemHookImpl : public IQtiOemHook, public hidl_death_recipient {
    sp<IQtiOemHookResponse> mResponseCb;
    sp<IQtiOemHookIndication> mIndicationCb;
    qcril_instance_id_e_type mInstanceId;
    qtimutex::QtiSharedMutex mCallbackLock;
    Module* mModule;

    sp<IQtiOemHookResponse> getResponseCallback();
    sp<IQtiOemHookIndication> getIndicationCallback();

    virtual Module* getOemHookServiceModule();

    // Function from hidl_death_recipient
    void serviceDied(uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &);
    std::shared_ptr<OemHookContext> getContext(uint32_t serial) {
        std::shared_ptr<OemHookContext> ctx = std::make_shared<OemHookContext>(mInstanceId, serial);
        return ctx;
    }
#ifdef QMI_RIL_UTF
  public:
    // make setCallback and clearCallbacks as public for UTF
#endif
    void clearCallbacks();

    // Functions from IQtiOemHook
    Return<void> setCallback(const sp<IQtiOemHookResponse> &responseCallback,
                             const sp<IQtiOemHookIndication> &indicationCallback);
    Return<void> oemHookRawRequest(int32_t serial, const hidl_vec<uint8_t>& data);
    Return<void> getAdnRecord(int32_t serial);
    Return<void> updateAdnRecord(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> setMbnConfig(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> getMbnConfig(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> getAvailableMbnConfig(int32_t serial);
    Return<void> cleanupMbnConfig(int32_t serial);
    Return<void> selectMbnConfig(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> getMetaInfo(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> deactivateMbnConfig(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> getQcVersionOfFile(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> validateMbnConfig(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> getQcVersionOfConfig(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> getOemVersionOfFile(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> getOemVersionOfConfig(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> activateMbnConfig(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> enableModemUpdate(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> uimGetVoltageStatus(int32_t serial);
    Return<void> uimSendCardPowerReq(int32_t serial, uint32_t cmd_id,
                                     uint8_t *data, uint32_t dataLen);
    Return<void> uimSendPersoReq(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> uimGetPersoStatus(int32_t serial);
    Return<void> uimSendDePersoReq(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> uimGetCardStateReq(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> uimGetCardAtrReq(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> uimSendApduReq(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> uimSwitchSlotReq(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> uimGetSlotMapping(int32_t serial);
    Return<void> uimSetApduBehaviorReq(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> uimGetSlotStatus(int32_t serial);
    Return<void> uimGetLPATC(int32_t serial);
    Return<void> uimSetLPATC(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> rejectIncomingCallWithCause21(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> getCurrentSetupCalls(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> requestSetupAnswer(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> requestSetLocalCallHold(int32_t serial, uint8_t *data, uint32_t dataLen);
    Return<void> startIncrementalScanRequest(int32_t serial);
    Return<void> cancelIncrementalScanRequest(int32_t serial);

    public:
        OemHookImpl(qcril_instance_id_e_type oemSocketInstanceId);
        void sendResponse(int32_t serial, RIL_Errno error, unsigned char* buf, size_t bufLen);
        void sendIndication(unsigned char *buf, size_t bufLen);
        void sendAdnRecords(std::shared_ptr<QcRilUnsolAdnRecordsOnSimMessage> msg);
        void sendAdnInitDone(std::shared_ptr<QcRilUnsolAdnInitDoneMessage> msg);
        void sendMbnConfigResult(std::shared_ptr<QcRilUnsolMbnConfigResultMessage> msg);
        void sendMbnConfigCleared(std::shared_ptr<QcRilUnsolMbnConfigClearedMessage> msg);
        void sendMbnValidateDumped(std::shared_ptr<QcRilUnsolMbnValidateDumpedMessage> msg);
        void sendMbnConfigList(std::shared_ptr<QcRilUnsolMbnConfigListMessage> msg);
        void sendMbnValidateConfig(std::shared_ptr<QcRilUnsolMbnValidateConfigMessage> msg);
        void uimSimlockTempUnlockExpireInd(std::shared_ptr<UimSimlockTempUnlockExpireInd> msg);
        void uimCardStateChangeInd(std::shared_ptr<UimCardStateChangeInd> msg);
        void uimSlotStatusInd(std::shared_ptr<UimSlotStatusInd> msg);
        void uimSimRefreshIndication(std::shared_ptr<UimSimRefreshIndication> msg);
        void uimVoltageStatusInd(std::shared_ptr<UimVoltageStatusInd> msg);
        void uimRmtRemoteSimStatusIndMsg(std::shared_ptr<UimRmtRemoteSimStatusIndMsg> msg);
        void sendUnsolCdmaBurstDtmf(std::shared_ptr<QcRilUnsolDtmfMessage> msg);
        void sendUnsolCdmaContDtmfStart(std::shared_ptr<QcRilUnsolDtmfMessage> msg);
        void sendUnsolCdmaContDtmfStop(std::shared_ptr<QcRilUnsolDtmfMessage> msg);
        void sendUnsolExtendedDbmIntl(std::shared_ptr<QcRilUnsolExtBurstIntlMessage> msg);
        void sendUnsolNssRelease(std::shared_ptr<QcRilUnsolNssReleaseMessage> msg);
        void sendUnsolSsErrorCode(std::shared_ptr<QcRilUnsolSuppSvcErrorCodeMessage> msg);
        void sendUnsolSpeechCodecInfo(std::shared_ptr<QcRilUnsolSpeechCodecInfoMessage> msg);
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace qcrilhook
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#ifdef QMI_RIL_UTF
using namespace vendor::qti::hardware::radio::qcrilhook::V1_0::implementation;

sp<OemHookImpl> &getOemHook();

extern "C" void resetOemHook();

#endif

#define OEMHOOK_HIDL_SEND_RESPONSE(slotId, radioResp, func, ...) \
    { \
    auto response = radioResp; \
    if (response) { \
        Return<void> retStatus = response->func(__VA_ARGS__); \
        if (!retStatus.isOk()) { \
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", retStatus.description().c_str()); \
        } \
    } else { \
        QCRIL_LOG_ERROR("%s: oemhookService[%d]->"#radioResp" == NULL", \
                __FUNCTION__,  slotId); \
    } \
    }

#define QCRIL_QMI_VOICE_DIAL_NUMBER_MAX_LEN 256

typedef struct {
    int             index;      /* Connection Index for use with, eg, AT+CHLD */
    int             toa;        /* type of address, eg 145 = intl */
    char            als;        /* ALS line indicator if available
                                   (0 = line 1) */
    char            isVoice;    /* nonzero if this is is a voice call */
    char            number[QCRIL_QMI_VOICE_DIAL_NUMBER_MAX_LEN];     /* Remote party number */
    int             numberPresentation; /* 0=Allowed, 1=Restricted, 2=Not Specified/Unknown 3=Payphone */
    char            name[QCRIL_QMI_VOICE_DIAL_NUMBER_MAX_LEN];       /* Remote party name */
    int             namePresentation; /* 0=Allowed, 1=Restricted, 2=Not Specified/Unknown 3=Payphone */
} qcril_qmi_voice_setup_call_info;

typedef struct {
    boolean rejection;
} qcril_qmi_voice_setup_answer_data_type;

/* OEM HOOK DTMF forward burst payload length (72 bytes)
 * on_length   : 4 bytes
 * off_length  : 4 bytes
 * dtmf_digits: 64 bytes */
#define QCRIL_QMI_VOICE_DTMF_FWD_BURST_PAYLOAD_LENGTH 72

#define QCRIL_QMI_VOICE_EXT_BRST_INTL_PAYLOAD_LENGTH 5

#endif  // VENDOR_QCOM_HARDWARE_QCRIL_QCRILHOOK_V1_0_QTIOEMHOOK_H
