/*===========================================================================
 *
 *    Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#pragma once

#include "android/hardware/radio/1.4/IRadioResponse.h"

using namespace ::android::hardware;
using namespace ::android::hardware::radio;
using ::android::hardware::Return;

class IRadioResponseClientImpl : public V1_4::IRadioResponse {
    virtual Return<void> getIccCardStatusResponse(const V1_0::RadioResponseInfo& info, const V1_0::CardStatus& cardStatus) override {
        return Void();
    }

    virtual Return<void> supplyIccPinForAppResponse(const V1_0::RadioResponseInfo& info, int32_t remainingRetries) override {
        return Void();
    }

    virtual Return<void> supplyIccPukForAppResponse(const V1_0::RadioResponseInfo& info, int32_t remainingRetries) override {
        return Void();
    }

    virtual Return<void> supplyIccPin2ForAppResponse(const V1_0::RadioResponseInfo& info, int32_t remainingRetries) override {
        return Void();
    }

    virtual Return<void> supplyIccPuk2ForAppResponse(const V1_0::RadioResponseInfo& info, int32_t remainingRetries) override {
        return Void();
    }

    virtual Return<void> changeIccPinForAppResponse(const V1_0::RadioResponseInfo& info, int32_t remainingRetries) override {
        return Void();
    }

    virtual Return<void> changeIccPin2ForAppResponse(const V1_0::RadioResponseInfo& info, int32_t remainingRetries) override {
        return Void();
    }

    virtual Return<void> supplyNetworkDepersonalizationResponse(const V1_0::RadioResponseInfo& info, int32_t remainingRetries) override {
        return Void();
    }

    virtual Return<void> getCurrentCallsResponse(const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::Call>& calls) override;

    virtual Return<void> dialResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getIMSIForAppResponse(const V1_0::RadioResponseInfo& info, const hidl_string& imsi) override {
        return Void();
    }

    virtual Return<void> hangupConnectionResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> hangupWaitingOrBackgroundResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> hangupForegroundResumeBackgroundResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> switchWaitingOrHoldingAndActiveResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> conferenceResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> rejectCallResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getLastCallFailCauseResponse(const V1_0::RadioResponseInfo& info, const V1_0::LastCallFailCauseInfo& failCauseinfo) override;

    virtual Return<void> getSignalStrengthResponse(const V1_0::RadioResponseInfo& info, const V1_0::SignalStrength& sigStrength) override;

    virtual Return<void> getVoiceRegistrationStateResponse(const V1_0::RadioResponseInfo& info, const V1_0::VoiceRegStateResult& voiceRegResponse) override;

    virtual Return<void> getDataRegistrationStateResponse(const V1_0::RadioResponseInfo& info, const V1_0::DataRegStateResult& dataRegResponse) override;

    virtual Return<void> getOperatorResponse(const V1_0::RadioResponseInfo& info, const hidl_string& longName, const hidl_string& shortName, const hidl_string& numeric) override;

    virtual Return<void> setRadioPowerResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> sendDtmfResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> sendSmsResponse(const V1_0::RadioResponseInfo& info, const V1_0::SendSmsResult& sms) override;

    virtual Return<void> sendSMSExpectMoreResponse(const V1_0::RadioResponseInfo& info, const V1_0::SendSmsResult& sms) override {
        return Void();
    }

    virtual Return<void> setupDataCallResponse(const V1_0::RadioResponseInfo& info, const V1_0::SetupDataCallResult& dcResponse) override {
        return Void();
    }

    virtual Return<void> iccIOForAppResponse(const V1_0::RadioResponseInfo& info, const V1_0::IccIoResult& iccIo) override;

    virtual Return<void> sendUssdResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> cancelPendingUssdResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getClirResponse(const V1_0::RadioResponseInfo& info, int32_t n, int32_t m) override;

    virtual Return<void> setClirResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getCallForwardStatusResponse(const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::CallForwardInfo>& callForwardInfos) override;

    virtual Return<void> setCallForwardResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getCallWaitingResponse(const V1_0::RadioResponseInfo& info, bool enable, int32_t serviceClass) override;

    virtual Return<void> setCallWaitingResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> acknowledgeLastIncomingGsmSmsResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> acceptCallResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> deactivateDataCallResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> getFacilityLockForAppResponse(const V1_0::RadioResponseInfo& info, int32_t response) override;

    virtual Return<void> setFacilityLockForAppResponse(const V1_0::RadioResponseInfo& info, int32_t retry) override;

    virtual Return<void> setBarringPasswordResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getNetworkSelectionModeResponse(const V1_0::RadioResponseInfo& info, bool manual) override;

    virtual Return<void> setNetworkSelectionModeAutomaticResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> setNetworkSelectionModeManualResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> getAvailableNetworksResponse(const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::OperatorInfo>& networkInfos) override;

    virtual Return<void> startDtmfResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> stopDtmfResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getBasebandVersionResponse(const V1_0::RadioResponseInfo& info, const hidl_string& version) override;

    virtual Return<void> separateConnectionResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> setMuteResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getMuteResponse(const V1_0::RadioResponseInfo& info, bool enable) override;

    virtual Return<void> getClipResponse(const V1_0::RadioResponseInfo& info, V1_0::ClipStatus status) override;

    virtual Return<void> getDataCallListResponse(const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::SetupDataCallResult>& dcResponse) override {
        return Void();
    }

    virtual Return<void> setSuppServiceNotificationsResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> writeSmsToSimResponse(const V1_0::RadioResponseInfo& info, int32_t index) override;

    virtual Return<void> deleteSmsOnSimResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> setBandModeResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getAvailableBandModesResponse(const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::RadioBandMode>& bandModes) override;

    virtual Return<void> sendEnvelopeResponse(const V1_0::RadioResponseInfo& info, const hidl_string& commandResponse) override {
        return Void();
    }

    virtual Return<void> sendTerminalResponseToSimResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> handleStkCallSetupRequestFromSimResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> explicitCallTransferResponse(const V1_0::RadioResponseInfo& info) override ;

    virtual Return<void> setPreferredNetworkTypeResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getPreferredNetworkTypeResponse(const V1_0::RadioResponseInfo& info, V1_0::PreferredNetworkType nwType) override;

    virtual Return<void> getNeighboringCidsResponse(const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::NeighboringCell>& cells) override;

    virtual Return<void> setLocationUpdatesResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> setCdmaSubscriptionSourceResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> setCdmaRoamingPreferenceResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getCdmaRoamingPreferenceResponse(const V1_0::RadioResponseInfo& info, V1_0::CdmaRoamingType type) override ;

    virtual Return<void> setTTYModeResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getTTYModeResponse(const V1_0::RadioResponseInfo& info, V1_0::TtyMode mode) override;

    virtual Return<void> setPreferredVoicePrivacyResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getPreferredVoicePrivacyResponse(const V1_0::RadioResponseInfo& info, bool enable) override;

    virtual Return<void> sendCDMAFeatureCodeResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> sendBurstDtmfResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> sendCdmaSmsResponse(const V1_0::RadioResponseInfo& info, const V1_0::SendSmsResult& sms) override {
        return Void();
    }

    virtual Return<void> acknowledgeLastIncomingCdmaSmsResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getGsmBroadcastConfigResponse(const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::GsmBroadcastSmsConfigInfo>& configs) override;

    virtual Return<void> setGsmBroadcastConfigResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> setGsmBroadcastActivationResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getCdmaBroadcastConfigResponse(const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::CdmaBroadcastSmsConfigInfo>& configs) override;

    virtual Return<void> setCdmaBroadcastConfigResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> setCdmaBroadcastActivationResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getCDMASubscriptionResponse(const V1_0::RadioResponseInfo& info, const hidl_string& mdn, const hidl_string& hSid, const hidl_string& hNid, const hidl_string& min, const hidl_string& prl) override {
        return Void();
    }

    virtual Return<void> writeSmsToRuimResponse(const V1_0::RadioResponseInfo& info, uint32_t index) override;

    virtual Return<void> deleteSmsOnRuimResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getDeviceIdentityResponse(const V1_0::RadioResponseInfo& info, const hidl_string& imei, const hidl_string& imeisv, const hidl_string& esn, const hidl_string& meid) override;

    virtual Return<void> exitEmergencyCallbackModeResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> getSmscAddressResponse(const V1_0::RadioResponseInfo& info, const hidl_string& smsc) override;

    virtual Return<void> setSmscAddressResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> reportSmsMemoryStatusResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> reportStkServiceIsRunningResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> getCdmaSubscriptionSourceResponse(const V1_0::RadioResponseInfo& info, V1_0::CdmaSubscriptionSource source) override;

    virtual Return<void> requestIsimAuthenticationResponse(const V1_0::RadioResponseInfo& info, const hidl_string& response) override {
        return Void();
    }

    virtual Return<void> acknowledgeIncomingGsmSmsWithPduResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> sendEnvelopeWithStatusResponse(const V1_0::RadioResponseInfo& info, const V1_0::IccIoResult& iccIo) override {
        return Void();
    }

    virtual Return<void> getVoiceRadioTechnologyResponse(const V1_0::RadioResponseInfo& info, V1_0::RadioTechnology rat) override;

    virtual Return<void> getCellInfoListResponse(const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::CellInfo>& cellInfo) override;

    virtual Return<void> setCellInfoListRateResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> setInitialAttachApnResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> getImsRegistrationStateResponse(const V1_0::RadioResponseInfo& info, bool isRegistered, V1_0::RadioTechnologyFamily ratFamily) override;

    virtual Return<void> sendImsSmsResponse(const V1_0::RadioResponseInfo& info, const V1_0::SendSmsResult& sms) override {
        return Void();
    }

    virtual Return<void> iccTransmitApduBasicChannelResponse(const V1_0::RadioResponseInfo& info, const V1_0::IccIoResult& result) override {
        return Void();
    }

    virtual Return<void> iccOpenLogicalChannelResponse(const V1_0::RadioResponseInfo& info,
                           int32_t channelId, const hidl_vec<int8_t>& selectResponse) override ;

    virtual Return<void> iccCloseLogicalChannelResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> iccTransmitApduLogicalChannelResponse(const V1_0::RadioResponseInfo& info, const V1_0::IccIoResult& result) override {
        return Void();
    }

    virtual Return<void> nvReadItemResponse(const V1_0::RadioResponseInfo& info, const hidl_string& result) override {
        return Void();
    }

    virtual Return<void> nvWriteItemResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> nvWriteCdmaPrlResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> nvResetConfigResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> setUiccSubscriptionResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> setDataAllowedResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> getHardwareConfigResponse(const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::HardwareConfig>& config) override {
        return Void();
    }

    virtual Return<void> requestIccSimAuthenticationResponse(const V1_0::RadioResponseInfo& info, const V1_0::IccIoResult& result) override {
        return Void();
    }

    virtual Return<void> setDataProfileResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> requestShutdownResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getRadioCapabilityResponse(const V1_0::RadioResponseInfo& info, const V1_0::RadioCapability& rc) override {
        return Void();
    }

    virtual Return<void> setRadioCapabilityResponse(const V1_0::RadioResponseInfo& info, const V1_0::RadioCapability& rc) override {
        return Void();
    }

    virtual Return<void> startLceServiceResponse(const V1_0::RadioResponseInfo& info, const V1_0::LceStatusInfo& statusInfo) override {
        return Void();
    }

    virtual Return<void> stopLceServiceResponse(const V1_0::RadioResponseInfo& info, const V1_0::LceStatusInfo& statusInfo) override {
        return Void();
    }

    virtual Return<void> pullLceDataResponse(const V1_0::RadioResponseInfo& info, const V1_0::LceDataInfo& lceInfo) override {
        return Void();
    }

    virtual Return<void> getModemActivityInfoResponse(const V1_0::RadioResponseInfo& info, const V1_0::ActivityStatsInfo& activityInfo) override;

    virtual Return<void> setAllowedCarriersResponse(const V1_0::RadioResponseInfo& info, int32_t numAllowed) override {
        return Void();
    }

    virtual Return<void> getAllowedCarriersResponse(const V1_0::RadioResponseInfo& info, bool allAllowed, const V1_0::CarrierRestrictions& carriers) override {
        return Void();
    }

    virtual Return<void> sendDeviceStateResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> setIndicationFilterResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> setSimCardPowerResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> acknowledgeRequest(int32_t serial) override {
        return Void();
    }

    virtual Return<void> setCarrierInfoForImsiEncryptionResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> setSimCardPowerResponse_1_1(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> startNetworkScanResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> stopNetworkScanResponse(const V1_0::RadioResponseInfo& info) override;

    virtual Return<void> startKeepaliveResponse(const V1_0::RadioResponseInfo& info, const V1_1::KeepaliveStatus& status) override {
        return Void();
    }

    virtual Return<void> stopKeepaliveResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> getCellInfoListResponse_1_2(const V1_0::RadioResponseInfo& info, const hidl_vec<V1_2::CellInfo>& cellInfo) override;

    virtual Return<void> getIccCardStatusResponse_1_2(const V1_0::RadioResponseInfo& info,
              const V1_2::CardStatus& cardStatus) override;

    virtual Return<void> setSignalStrengthReportingCriteriaResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> setLinkCapacityReportingCriteriaResponse(const V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> getCurrentCallsResponse_1_2(const V1_0::RadioResponseInfo& info, const hidl_vec<V1_2::Call>& calls) override;

    virtual Return<void> getSignalStrengthResponse_1_2(const V1_0::RadioResponseInfo& info, const V1_2::SignalStrength& signalStrength) override;

    virtual Return<void> getVoiceRegistrationStateResponse_1_2(const V1_0::RadioResponseInfo& info, const V1_2::VoiceRegStateResult& voiceRegResponse) override;

    virtual Return<void> getDataRegistrationStateResponse_1_2(const V1_0::RadioResponseInfo& info, const V1_2::DataRegStateResult& dataRegResponse) override;

    virtual Return<void> setSystemSelectionChannelsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> enableModemResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getModemStackStatusResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, bool isEnabled) override;

    virtual Return<void> emergencyDialResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> startNetworkScanResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;

    virtual Return<void> getCellInfoListResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::CellInfo>& cellInfo) override;

    virtual Return<void> getDataRegistrationStateResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_4::DataRegStateResult& dataRegResponse) override;

    virtual Return<void> getIccCardStatusResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_4::CardStatus& cardStatus) override;

    virtual Return<void> getPreferredNetworkTypeBitmapResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::RadioAccessFamily> networkTypeBitmap) override {
        return Void();
    }

    virtual Return<void> setPreferredNetworkTypeBitmapResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> getDataCallListResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::SetupDataCallResult>& dcResponse) override {
        return Void();
    }

    virtual Return<void> setupDataCallResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_4::SetupDataCallResult& dcResponse) override {
        return Void();
    }

    virtual Return<void> setAllowedCarriersResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override {
        return Void();
    }

    virtual Return<void> getAllowedCarriersResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority& carriers, ::android::hardware::radio::V1_4::SimLockMultiSimPolicy multiSimPolicy) override {
        return Void();
    }

    virtual Return<void> getSignalStrengthResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_4::SignalStrength& signalStrength) override;

};
