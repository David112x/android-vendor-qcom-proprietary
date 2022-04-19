/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <LocationAPI.h>
#include <log_util.h>

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

using ::android::hardware::hidl_vec;

using ::vendor::qti::gnss::V1_0::LocHidlLocation;

LocHidlGeofenceClient::LocHidlGeofenceClient(
        const sp<::vendor::qti::gnss::V1_0::ILocHidlGeofenceServiceCallback>& callback) :
                LocationAPIClientBase(), mGnssCbIface(callback){

    ENTRY_LOG();

    LocationCallbacks locationCallbacks;
    memset(&locationCallbacks, 0, sizeof(LocationCallbacks));
    locationCallbacks.size = sizeof(LocationCallbacks);

    locationCallbacks.trackingCb = nullptr;
    locationCallbacks.batchingCb = nullptr;

    locationCallbacks.geofenceBreachCb =
        [this](GeofenceBreachNotification geofenceBreachNotification) {
            onGeofenceBreachCb(geofenceBreachNotification);
        };

    locationCallbacks.geofenceStatusCb =
        [this](GeofenceStatusNotification geofenceStatusNotification) {
            onGeofenceStatusCb(geofenceStatusNotification);
        };

    locationCallbacks.gnssLocationInfoCb = nullptr;
    locationCallbacks.gnssNiCb = nullptr;
    locationCallbacks.gnssSvCb = nullptr;
    locationCallbacks.gnssNmeaCb = nullptr;
    locationCallbacks.gnssMeasurementsCb = nullptr;

    locAPISetCallbacks(locationCallbacks);
}

void LocHidlGeofenceClient::onGeofenceBreachCb(
        GeofenceBreachNotification notification)
{
    ENTRY_LOG();

    // Sanity checks
    if (mGnssCbIface == nullptr) {
        LOC_LOGE("mGnssCbIface NULL");
        return;
    }
    int count = (int)notification.count;
    if (count <= 0) {
        LOC_LOGE("Invalid count %d", count);
        return;
    }

    hidl_vec<int32_t> idVec;
    idVec.resize(count);
    for (int i = 0; i < count; i++) {
        idVec[i] = notification.ids[i];
    }

    LocHidlLocation gnssLocation;
    LocHidlUtils::locationToLocHidlLocation(notification.location, gnssLocation);

    TO_HIDL_CLIENT();
    auto r = mGnssCbIface->gnssGeofenceBreachCb(
            count, idVec, gnssLocation,
            (int32_t)notification.type, notification.timestamp);
    if (!r.isOk()) {
        LOC_LOGE("Error invoking HIDL CB [%s]", r.description().c_str());
    }
}

void LocHidlGeofenceClient::onGeofenceStatusCb(
        GeofenceStatusNotification notification)
{
    ENTRY_LOG();

    // Sanity checks
    if (mGnssCbIface == nullptr) {
        LOC_LOGE("mGnssCbIface NULL");
        return;
    }

    TO_HIDL_CLIENT();
    auto r = mGnssCbIface->gnssGeofenceStatusCb(
            (int32_t)notification.available, (int32_t)notification.techType);
    if (!r.isOk()) {
        LOC_LOGE("Error invoking HIDL CB [%s]", r.description().c_str());
    }
}

void LocHidlGeofenceClient::onAddGeofencesCb(
        size_t count, LocationError* errors, uint32_t* ids)
{
    ENTRY_LOG();

    // Sanity checks
    if (mGnssCbIface == nullptr) {
        LOC_LOGE("mGnssCbIface NULL");
        return;
    }
    if ((int)count <= 0) {
        LOC_LOGE("Invalid count %zu", count);
        return;
    }

    hidl_vec<int32_t> errorVec;
    errorVec.resize(count);
    for (size_t i = 0; i < count; i++) {
        errorVec[i] = errors[i];
    }

    hidl_vec<int32_t> idVec;
    idVec.resize(count);
    for (size_t i = 0; i < count; i++) {
        idVec[i] = ids[i];
    }

    TO_HIDL_CLIENT();
    auto r = mGnssCbIface->gnssAddGeofencesCb(count, errorVec, idVec);
    if (!r.isOk()) {
        LOC_LOGE("Error invoking HIDL CB [%s]", r.description().c_str());
    }
}

void LocHidlGeofenceClient::onRemoveGeofencesCb(
        size_t count, LocationError* errors, uint32_t* ids)
{
    ENTRY_LOG();

    // Sanity checks
    if (mGnssCbIface == nullptr) {
        LOC_LOGE("mGnssCbIface NULL");
        return;
    }
    if (count <= 0) {
        LOC_LOGE("Invalid count %zu", count);
        return;
    }

    hidl_vec<int32_t> errorVec;
    errorVec.resize(count);
    for (size_t i = 0; i < count; i++) {
        errorVec[i] = errors[i];
    }

    hidl_vec<int32_t> idVec;
    idVec.resize(count);
    for (size_t i = 0; i < count; i++) {
        idVec[i] = ids[i];
    }

    TO_HIDL_CLIENT();
    auto r = mGnssCbIface->gnssRemoveGeofencesCb(count, errorVec, idVec);
    if (!r.isOk()) {
        LOC_LOGE("Error invoking HIDL CB [%s]", r.description().c_str());
    }
}

void LocHidlGeofenceClient::onPauseGeofencesCb(
        size_t count, LocationError* errors, uint32_t* ids)
{
    ENTRY_LOG();

    // Sanity checks
    if (mGnssCbIface == nullptr) {
        LOC_LOGE("mGnssCbIface NULL");
        return;
    }
    if (count <= 0) {
        LOC_LOGE("Invalid count %zu", count);
        return;
    }

    hidl_vec<int32_t> errorVec;
    errorVec.resize(count);
    for (size_t i = 0; i < count; i++) {
        errorVec[i] = errors[i];
    }

    hidl_vec<int32_t> idVec;
    idVec.resize(count);
    for (size_t i = 0; i < count; i++) {
        idVec[i] = ids[i];
    }

    TO_HIDL_CLIENT();
    auto r = mGnssCbIface->gnssPauseGeofencesCb(count, errorVec, idVec);
    if (!r.isOk()) {
        LOC_LOGE("Error invoking HIDL CB [%s]", r.description().c_str());
    }
}

void LocHidlGeofenceClient::onResumeGeofencesCb(
        size_t count, LocationError* errors, uint32_t* ids)
{
    ENTRY_LOG();

    // Sanity checks
    if (mGnssCbIface == nullptr) {
        LOC_LOGE("mGnssCbIface NULL");
        return;
    }
    if (count <= 0) {
        LOC_LOGE("Invalid count %zu", count);
        return;
    }

    hidl_vec<int32_t> errorVec;
    errorVec.resize(count);
    for (size_t i = 0; i < count; i++) {
        errorVec[i] = errors[i];
    }

    hidl_vec<int32_t> idVec;
    idVec.resize(count);
    for (size_t i = 0; i < count; i++) {
        idVec[i] = ids[i];
    }

    TO_HIDL_CLIENT();
    auto r = mGnssCbIface->gnssResumeGeofencesCb(count, errorVec, idVec);
    if (!r.isOk()) {
        LOC_LOGE("Error invoking HIDL CB [%s]", r.description().c_str());
    }
}

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
