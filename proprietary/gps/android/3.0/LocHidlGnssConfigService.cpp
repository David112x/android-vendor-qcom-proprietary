/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define LOG_TAG "LocSvc_HIDL_GnssConfigService"
#define LOG_NDEBUG 0

#include "LocHidlGnssConfigService.h"

#define LOC_HIDL_VERSION V3_0

#include <log_util.h>
#include <gps_extended_c.h>
#include <dlfcn.h>

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

typedef const GnssInterface* (getLocationInterface)();

#define GNSS_CONFIG_SERVICE_ERROR 1
#define GNSS_CONFIG_SERVICE_SUCCESS 0

sp<LocHidlDeathRecipient> LocHidlGnssConfigService::mDeathRecipient = nullptr;
sp<ILocHidlGnssConfigServiceCallback> LocHidlGnssConfigService::sCallbackIface = nullptr;

void
LocHidlGnssConfigService::getGnssSvTypeConfigCallback(
        const GnssSvTypeConfig& config)
{
    ENTRY_LOG();

    if (mDeathRecipient != nullptr && mDeathRecipient->peerDied()) {
        ALOGE("%s] Peer Died.", __func__);
        sCallbackIface = nullptr;
        return;
    }

    if (sCallbackIface == nullptr) {
        LOC_LOGE("sCallbackIface NULL");
        return;
    }

    // convert config to vector
    std::vector<GnssConstellationType> disabledSvTypeVec;
    svTypeConfigToVec(config, disabledSvTypeVec);

    TO_HIDL_CLIENT();
    auto r = sCallbackIface->getGnssSvTypeConfigCb(disabledSvTypeVec);
    if (!r.isOk()) {
        LOC_LOGE("Error invoking HIDL CB [%s]", r.description().c_str());
    }
}

const GnssInterface* LocHidlGnssConfigService::getGnssInterface()
{
    ENTRY_LOG();

    if (nullptr == mGnssInterface && !mGetGnssInterfaceFailed) {
        LOC_LOGD("%s]: loading libgnss.so::getGnssInterface ...", __func__);
        getLocationInterface* getter = NULL;
        const char *error = NULL;
        dlerror();
        void *handle = dlopen("libgnss.so", RTLD_NOW);
        if (NULL == handle)  {
            LOC_LOGE("dlopen for libgnss.so failed");
        } else if (NULL != (error = dlerror()))  {
            LOC_LOGE("dlopen for libgnss.so failed, error = %s", error);
        } else {
            getter = (getLocationInterface*)dlsym(handle, "getGnssInterface");
            if ((error = dlerror()) != NULL)  {
                LOC_LOGE("dlsym for libgnss.so::getGnssInterface failed, error = %s", error);
                getter = NULL;
            }
        }

        if (NULL == getter) {
            mGetGnssInterfaceFailed = true;
        } else {
            mGnssInterface = (const GnssInterface*)(*getter)();
        }
    }
    return mGnssInterface;
}

LocHidlGnssConfigService::LocHidlGnssConfigService() :
        mGnssInterface(nullptr), mGetGnssInterfaceFailed(false)
{
    ENTRY_LOG();
}

LocHidlGnssConfigService::~LocHidlGnssConfigService()
{
    ENTRY_LOG();
}

// Methods from ::vendor::qti::gnss::V2_1::ILocHidlGnssConfigService follow.
Return<bool> LocHidlGnssConfigService::init(
        const sp<ILocHidlGnssConfigServiceCallback>& callback)
{

    static uint64_t deathCount = 0;
    FROM_HIDL_CLIENT();

    // Register death recipient
    if (mDeathRecipient == nullptr) {
        mDeathRecipient = new LocHidlDeathRecipient([] {
                    LOC_LOGE("%s]: ILocHidlGnssConfigServiceCallback died.", __func__);
                });
    }
    mDeathRecipient->registerToPeer(callback, deathCount++);
    sCallbackIface = callback;

    return true;
}

Return<void> LocHidlGnssConfigService::getGnssSvTypeConfig()
{
    FROM_HIDL_CLIENT();

    const GnssInterface* gnssInterface = getGnssInterface();
    if (nullptr == gnssInterface) {
        LOC_LOGE("Failed to get GNSS Interface");
        return Void();
    }
    GnssSvTypeConfigCallback callback = [](const GnssSvTypeConfig& config) {
        LocHidlGnssConfigService::getGnssSvTypeConfigCallback(config);
    };
    gnssInterface->gnssGetSvTypeConfig(callback);

    return Void();
}

Return<void> LocHidlGnssConfigService::setGnssSvTypeConfig(
        const hidl_vec<GnssConstellationType>& disabledSvTypeVec)
{
    FROM_HIDL_CLIENT();

    const GnssInterface* gnssInterface = getGnssInterface();
    if (nullptr == gnssInterface) {
        LOC_LOGE("Failed to get GNSS Interface");
        return Void();
    }

    GnssSvTypeConfig config = {};
    config.size = sizeof(config);
    svTypeVecToConfig(disabledSvTypeVec, config);

    gnssInterface->gnssUpdateSvTypeConfig(config);

    return Void();
}

Return<void> LocHidlGnssConfigService::resetGnssSvTypeConfig()
{
    FROM_HIDL_CLIENT();

    const GnssInterface* gnssInterface = getGnssInterface();
    if (nullptr == gnssInterface) {
        LOC_LOGE("Failed to get GNSS Interface");
        return Void();
    }
    gnssInterface->gnssResetSvTypeConfig();

    return Void();
}

void LocHidlGnssConfigService::svTypeVecToConfig(
        const std::vector<GnssConstellationType>& disabledSvTypeVec, GnssSvTypeConfig& config)
{
    bool found = false;
    for(GnssConstellationType svType: disabledSvTypeVec) {

        found = true;
        switch(svType) {
        case GnssConstellationType::GLONASS:
            config.blacklistedSvTypesMask |= GNSS_SV_TYPES_MASK_GLO_BIT;
            break;
        case GnssConstellationType::QZSS:
            config.blacklistedSvTypesMask |= GNSS_SV_TYPES_MASK_QZSS_BIT;
            break;
        case GnssConstellationType::BEIDOU:
            config.blacklistedSvTypesMask |= GNSS_SV_TYPES_MASK_BDS_BIT;
            break;
        case GnssConstellationType::GALILEO:
            config.blacklistedSvTypesMask |= GNSS_SV_TYPES_MASK_GAL_BIT;
            break;
        default:
            found = false;
        }
        // Check if enabled sv type
        if (!found) {
            svType = intToGnssConstellation((uint8_t)(~(int32_t)svType));
            switch(svType) {
            case GnssConstellationType::GLONASS:
                config.enabledSvTypesMask |= GNSS_SV_TYPES_MASK_GLO_BIT;
                break;
            case GnssConstellationType::QZSS:
                config.enabledSvTypesMask |= GNSS_SV_TYPES_MASK_QZSS_BIT;
                break;
            case GnssConstellationType::BEIDOU:
                config.enabledSvTypesMask |= GNSS_SV_TYPES_MASK_BDS_BIT;
                break;
            case GnssConstellationType::GALILEO:
                config.enabledSvTypesMask |= GNSS_SV_TYPES_MASK_GAL_BIT;
                break;
            default:
                LOC_LOGE("Invalid sv type %d", svType);
            }
        }
    }
}

void LocHidlGnssConfigService::svTypeConfigToVec(
        const GnssSvTypeConfig& config, std::vector<GnssConstellationType>& svTypeVec)
{
    if (config.blacklistedSvTypesMask & GNSS_SV_TYPES_MASK_GLO_BIT) {
        svTypeVec.push_back(GnssConstellationType::GLONASS);
    }
    if (config.blacklistedSvTypesMask & GNSS_SV_TYPES_MASK_QZSS_BIT) {
        svTypeVec.push_back(GnssConstellationType::QZSS);
    }
    if (config.blacklistedSvTypesMask & GNSS_SV_TYPES_MASK_BDS_BIT) {
        svTypeVec.push_back(GnssConstellationType::BEIDOU);
    }
    if (config.blacklistedSvTypesMask & GNSS_SV_TYPES_MASK_GAL_BIT) {
        svTypeVec.push_back(GnssConstellationType::GALILEO);
    }

    // Set enabled values
    if (config.enabledSvTypesMask & GNSS_SV_TYPES_MASK_GLO_BIT) {
        svTypeVec.push_back((GnssConstellationType)
                (~gnssConstellationToInt(GnssConstellationType::GLONASS)));
    }
    if (config.enabledSvTypesMask & GNSS_SV_TYPES_MASK_QZSS_BIT) {
        svTypeVec.push_back((GnssConstellationType)
                (~gnssConstellationToInt(GnssConstellationType::QZSS)));
    }
    if (config.enabledSvTypesMask & GNSS_SV_TYPES_MASK_BDS_BIT) {
        svTypeVec.push_back((GnssConstellationType)
                (~gnssConstellationToInt(GnssConstellationType::BEIDOU)));
    }
    if (config.enabledSvTypesMask & GNSS_SV_TYPES_MASK_GAL_BIT) {
        svTypeVec.push_back((GnssConstellationType)
                (~gnssConstellationToInt(GnssConstellationType::GALILEO)));
    }
}

GnssConstellationType LocHidlGnssConfigService::intToGnssConstellation(int32_t svTypeInt) {

    switch(svTypeInt) {
    case 1:
        return GnssConstellationType::GPS;
    case 2:
        return GnssConstellationType::SBAS;
    case 3:
        return GnssConstellationType::GLONASS;
    case 4:
        return GnssConstellationType::QZSS;
    case 5:
        return GnssConstellationType::BEIDOU;
    case 6:
        return GnssConstellationType::GALILEO;
    default:
        return GnssConstellationType::UNKNOWN;
    }
}

int32_t LocHidlGnssConfigService::gnssConstellationToInt(GnssConstellationType svType) {

    switch(svType) {
    case GnssConstellationType::GPS:
        return 1;
    case GnssConstellationType::SBAS:
        return 2;
    case GnssConstellationType::GLONASS:
        return 3;
    case GnssConstellationType::QZSS:
        return 4;
    case GnssConstellationType::BEIDOU:
        return 5;
    case GnssConstellationType::GALILEO:
        return 6;
    default:
        return 0;
    }
}

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor

#undef LOC_HIDL_VERSION
