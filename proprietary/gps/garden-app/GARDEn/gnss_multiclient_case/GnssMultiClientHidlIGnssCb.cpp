/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/
#define LOG_NDEBUG 0
#define LOG_TAG "GARDEN_GMCC_HidlIGnssCb"

#include "GnssMultiClientHidlIGnssCb.h"
#include "GnssMultiClientCaseUtils.h"

namespace garden {

GnssMultiClientHidlIGnssCb::GnssMultiClientHidlIGnssCb(
        GnssMultiClientHidlIGnss* hidlGnss)
{
    mHidlGnss = hidlGnss;
}

GnssMultiClientHidlIGnssCb::~GnssMultiClientHidlIGnssCb() {}

Return<void>
GnssMultiClientHidlIGnssCb::gnssMeasurementCb(
        const IGnssMeasurementCallback_V1_1::GnssData& )
{
    return Void();
}

Return<void> GnssMultiClientHidlIGnssCb::GnssMeasurementCb(
        const IGnssMeasurementCallback_V1_0::GnssData& )
{
    return Void();
}
Return<void> GnssMultiClientHidlIGnssCb::gnssMeasurementCb_2_0(const GnssData& data) {
    CONDPRINTLN("## %s]: () ##, num of measurements: %zu", __FUNCTION__, data.measurements.size());
    return Void();
}
Return<void> GnssMultiClientHidlIGnssCb::nfwNotifyCb(
        const IGnssVisibilityControlCallback::NfwNotification& notification)
{
    CONDPRINTLN("## %s]: () ##, app: %s, PROT: %d, PNAME: %s REQ: %d, REQID: %s"
        ", RESP: %d, EM: %d, CA: %d",
        __FUNCTION__, notification.proxyAppPackageName.c_str(), notification.protocolStack,
        notification.otherProtocolStackName.c_str(), notification.requestor,
        notification.requestorId.c_str(), notification.responseType,
        notification.inEmergencyMode, notification.isCachedLocation);
    return Void();
}

Return<bool> GnssMultiClientHidlIGnssCb::isInEmergencySession()
{
    return false;
}

Return<void> GnssMultiClientHidlIGnssCb::gnssLocationBatchCb(
        const android::hardware::hidl_vec<GnssLocation>& locations) {
    CONDPRINTLN("## %s]: () ##, num of batched locations: %u" ,
        __FUNCTION__, locations.size());
    return Void();
}
Return<void> GnssMultiClientHidlIGnssCb::gnssLocationCb(
        const ::android::hardware::gnss::V1_0::GnssLocation& location)
{
    CONDPRINTLN("HIDL Client (gnssLocationCb) - LAT: %f, LON: %f, ALT: %f ACC: %f, TIME: %llu",
             location.latitudeDegrees, location.longitudeDegrees, location.altitudeMeters,
            location.horizontalAccuracyMeters, (long long)location.timestamp);
    return Void();
}

Return<void> GnssMultiClientHidlIGnssCb::gnssStatusCb(
        ::android::hardware::gnss::V1_0::IGnssCallback::GnssStatusValue status)
{
    return Void();
}
Return<void> GnssMultiClientHidlIGnssCb::gnssSvStatusCb(
        const ::android::hardware::gnss::V1_0::IGnssCallback::GnssSvStatus& svInfo)
{
    return Void();
}
Return<void> GnssMultiClientHidlIGnssCb::gnssNmeaCb(int64_t timestamp, const ::android::hardware::hidl_string& nmea)
{
    return Void();
}
Return<void> GnssMultiClientHidlIGnssCb::gnssSetCapabilitesCb(
        ::android::hardware::hidl_bitfield<::android::hardware::gnss::V1_0::IGnssCallback::Capabilities> capabilities)
{
    return Void();
}
Return<void> GnssMultiClientHidlIGnssCb::gnssAcquireWakelockCb()
{
    return Void();
}
Return<void> GnssMultiClientHidlIGnssCb::gnssReleaseWakelockCb()
{
    return Void();
}
Return<void> GnssMultiClientHidlIGnssCb::gnssRequestTimeCb()
{
    return Void();
}
Return<void> GnssMultiClientHidlIGnssCb::gnssSetSystemInfoCb(
        const ::android::hardware::gnss::V1_0::IGnssCallback::GnssSystemInfo& info)
{
    return Void();
}
Return<void> GnssMultiClientHidlIGnssCb::gnssNameCb(
        const ::android::hardware::hidl_string& name)
{
    return Void();
}
Return<void> GnssMultiClientHidlIGnssCb::gnssRequestLocationCb(
        bool independentFromGnss)
{
    return Void();
}

Return<void> GnssMultiClientHidlIGnssCb::gnssSetCapabilitiesCb_2_0(
        ::android::hardware::hidl_bitfield<Capabilities> capabilities) {
    CONDPRINTLN("%s: capabilities=%u\n", __func__, capabilities);
    return Void();
}
Return<void> GnssMultiClientHidlIGnssCb::gnssLocationCb_2_0(const GnssLocation& location) {
    CONDPRINTLN("## %s]: () ##, latitudeDegrees: %f, longitudeDegrees: %f, gnssLocationFlags: %u,"
            " ElapsedRealtimeFlags: %u" ,
        __FUNCTION__, location.v1_0.latitudeDegrees, location.v1_0.longitudeDegrees,
        location.v1_0.gnssLocationFlags, location.elapsedRealtime.flags);
    return Void();
}
Return<void> GnssMultiClientHidlIGnssCb::gnssRequestLocationCb_2_0(bool independentFromGnss,
        bool isUserEmergency) {
    CONDPRINTLN("## %s]: () ##, independentFromGnss: %d, isUserEmergency: %d",
        __FUNCTION__, independentFromGnss, isUserEmergency);
    return Void();
}
Return<void> GnssMultiClientHidlIGnssCb::gnssSvStatusCb_2_0(
        const android::hardware::hidl_vec<GnssSvInfo>& svInfoList) {
    CONDPRINTLN("## %s]: () ##, SV Count: %u", __FUNCTION__, svInfoList.size());
    return Void();
}
Return<void> GnssMultiClientHidlIGnssCb::agnssStatusCb(IAGnssCallback_V2_0::AGnssType type,
        IAGnssCallback_V2_0::AGnssStatusValue status) {
    CONDPRINTLN("## %s]: () ##, AGnssType: %u, AGnssStatus: %u", __FUNCTION__, type, status);
    return Void();
}

} // namespace garden
