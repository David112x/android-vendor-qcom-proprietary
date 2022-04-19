/*===========================================================================
 *
 *    Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#pragma once

#include "android/hardware/radio/1.4/IRadioIndication.h"

using namespace ::android::hardware;
using namespace ::android::hardware::radio;
using ::android::hardware::Return;


class IRadioIndicationClientImpl : public V1_4::IRadioIndication {
public:
    virtual Return<void> radioStateChanged(V1_0::RadioIndicationType type, V1_0::RadioState radioState) override;

    virtual Return<void> callStateChanged(V1_0::RadioIndicationType type) override;

    virtual Return<void> networkStateChanged(V1_0::RadioIndicationType type) override;

    virtual Return<void> newSms(V1_0::RadioIndicationType type, const hidl_vec<uint8_t>& pdu) override;

    virtual Return<void> newSmsStatusReport(V1_0::RadioIndicationType type, const hidl_vec<uint8_t>& pdu) override {
        return Void();
    }

    virtual Return<void> newSmsOnSim(V1_0::RadioIndicationType type, int32_t recordNumber) override {
        return Void();
    }

    virtual Return<void> onUssd(V1_0::RadioIndicationType type, V1_0::UssdModeType modeType, const hidl_string& msg) override;

    virtual Return<void> nitzTimeReceived(V1_0::RadioIndicationType type, const hidl_string& nitzTime, uint64_t receivedTime) override {
        return Void();
    }

    virtual Return<void> currentSignalStrength(V1_0::RadioIndicationType type, const V1_0::SignalStrength& signalStrength) override;

    virtual Return<void> dataCallListChanged(V1_0::RadioIndicationType type, const hidl_vec<V1_0::SetupDataCallResult>& dcList) override {
        return Void();
    }

    virtual Return<void> suppSvcNotify(V1_0::RadioIndicationType type, const V1_0::SuppSvcNotification& suppSvc) override;

    virtual Return<void> stkSessionEnd(V1_0::RadioIndicationType type) override {
        return Void();
    }

    virtual Return<void> stkProactiveCommand(V1_0::RadioIndicationType type, const hidl_string& cmd) override {
        return Void();
    }

    virtual Return<void> stkEventNotify(V1_0::RadioIndicationType type, const hidl_string& cmd) override {
        return Void();
    }

    virtual Return<void> stkCallSetup(V1_0::RadioIndicationType type, int64_t timeout) override {
        return Void();
    }

    virtual Return<void> simSmsStorageFull(V1_0::RadioIndicationType type);

    virtual Return<void> simRefresh(V1_0::RadioIndicationType type, const V1_0::SimRefreshResult& refreshResult) override {
        return Void();
    }

    virtual Return<void> callRing(V1_0::RadioIndicationType type, bool isGsm, const V1_0::CdmaSignalInfoRecord& record) override;

    virtual Return<void> simStatusChanged(V1_0::RadioIndicationType type);

    virtual Return<void> cdmaNewSms(V1_0::RadioIndicationType type, const V1_0::CdmaSmsMessage& msg) override {
        return Void();
    }

    virtual Return<void> newBroadcastSms(V1_0::RadioIndicationType type,
        const hidl_vec<uint8_t>& data) override;

    virtual Return<void> cdmaRuimSmsStorageFull(V1_0::RadioIndicationType type);

    virtual Return<void> restrictedStateChanged(V1_0::RadioIndicationType type, V1_0::PhoneRestrictedState state) override;

    virtual Return<void> enterEmergencyCallbackMode(V1_0::RadioIndicationType type) override {
        return Void();
    }

    virtual Return<void> cdmaCallWaiting(V1_0::RadioIndicationType type, const V1_0::CdmaCallWaiting& callWaitingRecord) override;

    virtual Return<void> cdmaOtaProvisionStatus(V1_0::RadioIndicationType type, V1_0::CdmaOtaProvisionStatus status) override;

    virtual Return<void> cdmaInfoRec(V1_0::RadioIndicationType type, const V1_0::CdmaInformationRecords& records) override;

    virtual Return<void> indicateRingbackTone(V1_0::RadioIndicationType type, bool start) override;

    virtual Return<void> resendIncallMute(V1_0::RadioIndicationType type) override {
        return Void();
    }

    virtual Return<void> cdmaSubscriptionSourceChanged(V1_0::RadioIndicationType type, V1_0::CdmaSubscriptionSource cdmaSource) override {
        return Void();
    }

    virtual Return<void> cdmaPrlChanged(V1_0::RadioIndicationType type, int32_t version) override {
        return Void();
    }

    virtual Return<void> exitEmergencyCallbackMode(V1_0::RadioIndicationType type) override {
        return Void();
    }

    virtual Return<void> rilConnected(V1_0::RadioIndicationType type) override {
        return Void();
    }

    virtual Return<void> voiceRadioTechChanged(V1_0::RadioIndicationType type, V1_0::RadioTechnology rat) override;

    virtual Return<void> cellInfoList(V1_0::RadioIndicationType type, const hidl_vec<V1_0::CellInfo>& records) override {
        return Void();
    }

    virtual Return<void> imsNetworkStateChanged(V1_0::RadioIndicationType type) override;

    virtual Return<void> subscriptionStatusChanged(V1_0::RadioIndicationType type, bool activate) override;

    virtual Return<void> srvccStateNotify(V1_0::RadioIndicationType type, V1_0::SrvccState state) override;

    virtual Return<void> hardwareConfigChanged(V1_0::RadioIndicationType type, const hidl_vec<V1_0::HardwareConfig>& configs) override {
        return Void();
    }

    virtual Return<void> radioCapabilityIndication(V1_0::RadioIndicationType type, const V1_0::RadioCapability& rc) override {
        return Void();
    }

    virtual Return<void> onSupplementaryServiceIndication(V1_0::RadioIndicationType type, const V1_0::StkCcUnsolSsResult& ss) override;

    virtual Return<void> stkCallControlAlphaNotify(V1_0::RadioIndicationType type, const hidl_string& alpha) override;

    virtual Return<void> lceData(V1_0::RadioIndicationType type, const V1_0::LceDataInfo& lce) override {
        return Void();
    }

    virtual Return<void> pcoData(V1_0::RadioIndicationType type, const V1_0::PcoDataInfo& pco) override {
        return Void();
    }

    virtual Return<void> modemReset(V1_0::RadioIndicationType type, const hidl_string& reason) override {
        return Void();
    }

    virtual Return<void> carrierInfoForImsiEncryption(V1_0::RadioIndicationType info) override {
        return Void();
    }

    virtual Return<void> networkScanResult(V1_0::RadioIndicationType type, const V1_1::NetworkScanResult& result) override {
        return Void();
    }

    virtual Return<void> keepaliveStatus(V1_0::RadioIndicationType type, const V1_1::KeepaliveStatus& status) override {
        return Void();
    }

    virtual Return<void> networkScanResult_1_2(V1_0::RadioIndicationType type, const V1_2::NetworkScanResult& result) override {
        return Void();
    }

    virtual Return<void> cellInfoList_1_2(V1_0::RadioIndicationType type, const hidl_vec<V1_2::CellInfo>& records) override {
        return Void();
    }

    virtual Return<void> currentLinkCapacityEstimate(V1_0::RadioIndicationType type, const V1_2::LinkCapacityEstimate& lce) override {
        return Void();
    }

    virtual Return<void> currentPhysicalChannelConfigs(V1_0::RadioIndicationType type, const hidl_vec<V1_2::PhysicalChannelConfig>& configs) override {
        return Void();
    }

    virtual Return<void> currentSignalStrength_1_2(V1_0::RadioIndicationType type, const V1_2::SignalStrength& signalStrength) override;
    virtual Return<void> currentEmergencyNumberList(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::EmergencyNumber>& emergencyNumberList) override;

    virtual Return<void> cellInfoList_1_4(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::CellInfo>& records) override {
        return Void();
    }

    virtual Return<void> networkScanResult_1_4(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_4::NetworkScanResult& result) override;

    virtual Return<void> currentPhysicalChannelConfigs_1_4(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::PhysicalChannelConfig>& configs) override {
        return Void();
    }

    virtual Return<void> dataCallListChanged_1_4(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::SetupDataCallResult>& dcList) override {
        return Void();
    }

    virtual Return<void> currentSignalStrength_1_4(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_4::SignalStrength& signalStrength) override;
};
