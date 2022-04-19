/******************************************************************************
 * #  Copyright (c) 2019 Qualcomm Technologies, Inc.
 * #  All Rights Reserved.
 * #  Confidential and Proprietary - Qualcomm Technologies, Inc.
 * #******************************************************************************/

#define LOG_TAG "android.hardware.radio@1.4::types"

#include <android/hardware/radio/1.4/types.h>
#include <log/log.h>
#include <cutils/trace.h>
#ifndef QMI_RIL_UTF
#include <hidl/HidlTransportSupport.h>

#include <hidl/Static.h>
#include <hwbinder/ProcessState.h>
#include <utils/Trace.h>
#include <android/hardware/radio/1.4/hwtypes.h>
#endif

namespace android {
namespace hardware {
namespace radio {
namespace V1_4 {

::android::hardware::radio::V1_4::RadioFrequencyInfo::RadioFrequencyInfo() {
    static_assert(offsetof(::android::hardware::radio::V1_4::RadioFrequencyInfo, hidl_d) == 0, "wrong offset");
    static_assert(offsetof(::android::hardware::radio::V1_4::RadioFrequencyInfo, hidl_u) == 4, "wrong offset");

    hidl_d = hidl_discriminator::range;
    new (&hidl_u.range) ::android::hardware::radio::V1_4::FrequencyRange();
}

::android::hardware::radio::V1_4::RadioFrequencyInfo::~RadioFrequencyInfo() {
    hidl_destructUnion();
}

::android::hardware::radio::V1_4::RadioFrequencyInfo::RadioFrequencyInfo(RadioFrequencyInfo&& other) {
    switch (other.hidl_d) {
        case hidl_discriminator::range: {
            new (&hidl_u.range) ::android::hardware::radio::V1_4::FrequencyRange(std::move(other.hidl_u.range));
            break;
        }
        case hidl_discriminator::channelNumber: {
            new (&hidl_u.channelNumber) int32_t(std::move(other.hidl_u.channelNumber));
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) other.hidl_d) + ").").c_str());
        }
    }

    hidl_d = other.hidl_d;
}

::android::hardware::radio::V1_4::RadioFrequencyInfo::RadioFrequencyInfo(const RadioFrequencyInfo& other) {
    switch (other.hidl_d) {
        case hidl_discriminator::range: {
            new (&hidl_u.range) ::android::hardware::radio::V1_4::FrequencyRange(other.hidl_u.range);
            break;
        }
        case hidl_discriminator::channelNumber: {
            new (&hidl_u.channelNumber) int32_t(other.hidl_u.channelNumber);
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) other.hidl_d) + ").").c_str());
        }
    }

    hidl_d = other.hidl_d;
}

::android::hardware::radio::V1_4::RadioFrequencyInfo& (::android::hardware::radio::V1_4::RadioFrequencyInfo::operator=)(RadioFrequencyInfo&& other) {
    if (this == &other) { return *this; }

    switch (other.hidl_d) {
        case hidl_discriminator::range: {
            range(std::move(other.hidl_u.range));
            break;
        }
        case hidl_discriminator::channelNumber: {
            channelNumber(std::move(other.hidl_u.channelNumber));
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) other.hidl_d) + ").").c_str());
        }
    }
    return *this;
}

::android::hardware::radio::V1_4::RadioFrequencyInfo& (::android::hardware::radio::V1_4::RadioFrequencyInfo::operator=)(const RadioFrequencyInfo& other) {
    if (this == &other) { return *this; }

    switch (other.hidl_d) {
        case hidl_discriminator::range: {
            range(other.hidl_u.range);
            break;
        }
        case hidl_discriminator::channelNumber: {
            channelNumber(other.hidl_u.channelNumber);
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) other.hidl_d) + ").").c_str());
        }
    }
    return *this;
}

void ::android::hardware::radio::V1_4::RadioFrequencyInfo::hidl_destructUnion() {
    switch (hidl_d) {
        case hidl_discriminator::range: {
            ::android::hardware::details::destructElement(&(hidl_u.range));
            break;
        }
        case hidl_discriminator::channelNumber: {
            ::android::hardware::details::destructElement(&(hidl_u.channelNumber));
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) hidl_d) + ").").c_str());
        }
    }

}

void ::android::hardware::radio::V1_4::RadioFrequencyInfo::range(::android::hardware::radio::V1_4::FrequencyRange o) {
    if (hidl_d != hidl_discriminator::range) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.range) ::android::hardware::radio::V1_4::FrequencyRange(o);
        hidl_d = hidl_discriminator::range;
    }
    else if (&(hidl_u.range) != &o) {
        hidl_u.range = o;
    }
}

::android::hardware::radio::V1_4::FrequencyRange& (::android::hardware::radio::V1_4::RadioFrequencyInfo::range)() {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::range)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::range));
    }

    return hidl_u.range;
}

::android::hardware::radio::V1_4::FrequencyRange (::android::hardware::radio::V1_4::RadioFrequencyInfo::range)() const {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::range)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::range));
    }

    return hidl_u.range;
}

void ::android::hardware::radio::V1_4::RadioFrequencyInfo::channelNumber(int32_t o) {
    if (hidl_d != hidl_discriminator::channelNumber) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.channelNumber) int32_t(o);
        hidl_d = hidl_discriminator::channelNumber;
    }
    else if (&(hidl_u.channelNumber) != &o) {
        hidl_u.channelNumber = o;
    }
}

int32_t& (::android::hardware::radio::V1_4::RadioFrequencyInfo::channelNumber)() {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::channelNumber)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::channelNumber));
    }

    return hidl_u.channelNumber;
}

int32_t (::android::hardware::radio::V1_4::RadioFrequencyInfo::channelNumber)() const {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::channelNumber)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::channelNumber));
    }

    return hidl_u.channelNumber;
}

::android::hardware::radio::V1_4::RadioFrequencyInfo::hidl_union::hidl_union() {}

::android::hardware::radio::V1_4::RadioFrequencyInfo::hidl_union::~hidl_union() {}

::android::hardware::radio::V1_4::RadioFrequencyInfo::hidl_discriminator (::android::hardware::radio::V1_4::RadioFrequencyInfo::getDiscriminator)() const {
    return hidl_d;
}

::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::VopsInfo() {
    static_assert(offsetof(::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo, hidl_d) == 0, "wrong offset");
    static_assert(offsetof(::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo, hidl_u) == 1, "wrong offset");

    hidl_d = hidl_discriminator::noinit;
    new (&hidl_u.noinit) ::android::hidl::safe_union::V1_0::Monostate();
}

::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::~VopsInfo() {
    hidl_destructUnion();
}

::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::VopsInfo(VopsInfo&& other) {
    switch (other.hidl_d) {
        case hidl_discriminator::noinit: {
            new (&hidl_u.noinit) ::android::hidl::safe_union::V1_0::Monostate(std::move(other.hidl_u.noinit));
            break;
        }
        case hidl_discriminator::lteVopsInfo: {
            new (&hidl_u.lteVopsInfo) ::android::hardware::radio::V1_4::LteVopsInfo(std::move(other.hidl_u.lteVopsInfo));
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) other.hidl_d) + ").").c_str());
        }
    }

    hidl_d = other.hidl_d;
}

::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::VopsInfo(const VopsInfo& other) {
    switch (other.hidl_d) {
        case hidl_discriminator::noinit: {
            new (&hidl_u.noinit) ::android::hidl::safe_union::V1_0::Monostate(other.hidl_u.noinit);
            break;
        }
        case hidl_discriminator::lteVopsInfo: {
            new (&hidl_u.lteVopsInfo) ::android::hardware::radio::V1_4::LteVopsInfo(other.hidl_u.lteVopsInfo);
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) other.hidl_d) + ").").c_str());
        }
    }

    hidl_d = other.hidl_d;
}

::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo& (::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::operator=)(VopsInfo&& other) {
    if (this == &other) { return *this; }

    switch (other.hidl_d) {
        case hidl_discriminator::noinit: {
            noinit(std::move(other.hidl_u.noinit));
            break;
        }
        case hidl_discriminator::lteVopsInfo: {
            lteVopsInfo(std::move(other.hidl_u.lteVopsInfo));
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) other.hidl_d) + ").").c_str());
        }
    }
    return *this;
}

::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo& (::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::operator=)(const VopsInfo& other) {
    if (this == &other) { return *this; }

    switch (other.hidl_d) {
        case hidl_discriminator::noinit: {
            noinit(other.hidl_u.noinit);
            break;
        }
        case hidl_discriminator::lteVopsInfo: {
            lteVopsInfo(other.hidl_u.lteVopsInfo);
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) other.hidl_d) + ").").c_str());
        }
    }
    return *this;
}

void ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::hidl_destructUnion() {
    switch (hidl_d) {
        case hidl_discriminator::noinit: {
            ::android::hardware::details::destructElement(&(hidl_u.noinit));
            break;
        }
        case hidl_discriminator::lteVopsInfo: {
            ::android::hardware::details::destructElement(&(hidl_u.lteVopsInfo));
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) hidl_d) + ").").c_str());
        }
    }

}

void ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::noinit(const ::android::hidl::safe_union::V1_0::Monostate& o) {
    if (hidl_d != hidl_discriminator::noinit) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.noinit) ::android::hidl::safe_union::V1_0::Monostate(o);
        hidl_d = hidl_discriminator::noinit;
    }
    else if (&(hidl_u.noinit) != &o) {
        hidl_u.noinit = o;
    }
}

void ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::noinit(::android::hidl::safe_union::V1_0::Monostate&& o) {
    if (hidl_d != hidl_discriminator::noinit) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.noinit) ::android::hidl::safe_union::V1_0::Monostate(std::move(o));
        hidl_d = hidl_discriminator::noinit;
    }
    else if (&(hidl_u.noinit) != &o) {
        hidl_u.noinit = std::move(o);
    }
}

::android::hidl::safe_union::V1_0::Monostate& (::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::noinit)() {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::noinit)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::noinit));
    }

    return hidl_u.noinit;
}

const ::android::hidl::safe_union::V1_0::Monostate& (::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::noinit)() const {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::noinit)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::noinit));
    }

    return hidl_u.noinit;
}

void ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::lteVopsInfo(const ::android::hardware::radio::V1_4::LteVopsInfo& o) {
    if (hidl_d != hidl_discriminator::lteVopsInfo) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.lteVopsInfo) ::android::hardware::radio::V1_4::LteVopsInfo(o);
        hidl_d = hidl_discriminator::lteVopsInfo;
    }
    else if (&(hidl_u.lteVopsInfo) != &o) {
        hidl_u.lteVopsInfo = o;
    }
}

void ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::lteVopsInfo(::android::hardware::radio::V1_4::LteVopsInfo&& o) {
    if (hidl_d != hidl_discriminator::lteVopsInfo) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.lteVopsInfo) ::android::hardware::radio::V1_4::LteVopsInfo(std::move(o));
        hidl_d = hidl_discriminator::lteVopsInfo;
    }
    else if (&(hidl_u.lteVopsInfo) != &o) {
        hidl_u.lteVopsInfo = std::move(o);
    }
}

::android::hardware::radio::V1_4::LteVopsInfo& (::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::lteVopsInfo)() {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::lteVopsInfo)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::lteVopsInfo));
    }

    return hidl_u.lteVopsInfo;
}

const ::android::hardware::radio::V1_4::LteVopsInfo& (::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::lteVopsInfo)() const {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::lteVopsInfo)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::lteVopsInfo));
    }

    return hidl_u.lteVopsInfo;
}

::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::hidl_union::hidl_union() {}

::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::hidl_union::~hidl_union() {}

::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::hidl_discriminator (::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::getDiscriminator)() const {
    return hidl_d;
}

::android::hardware::radio::V1_4::CellInfo::Info::Info() {
    static_assert(offsetof(::android::hardware::radio::V1_4::CellInfo::Info, hidl_d) == 0, "wrong offset");
    static_assert(offsetof(::android::hardware::radio::V1_4::CellInfo::Info, hidl_u) == 8, "wrong offset");

    hidl_d = hidl_discriminator::gsm;
    new (&hidl_u.gsm) ::android::hardware::radio::V1_2::CellInfoGsm();
}

::android::hardware::radio::V1_4::CellInfo::Info::~Info() {
    hidl_destructUnion();
}

::android::hardware::radio::V1_4::CellInfo::Info::Info(Info&& other) {
    switch (other.hidl_d) {
        case hidl_discriminator::gsm: {
            new (&hidl_u.gsm) ::android::hardware::radio::V1_2::CellInfoGsm(std::move(other.hidl_u.gsm));
            break;
        }
        case hidl_discriminator::cdma: {
            new (&hidl_u.cdma) ::android::hardware::radio::V1_2::CellInfoCdma(std::move(other.hidl_u.cdma));
            break;
        }
        case hidl_discriminator::wcdma: {
            new (&hidl_u.wcdma) ::android::hardware::radio::V1_2::CellInfoWcdma(std::move(other.hidl_u.wcdma));
            break;
        }
        case hidl_discriminator::tdscdma: {
            new (&hidl_u.tdscdma) ::android::hardware::radio::V1_2::CellInfoTdscdma(std::move(other.hidl_u.tdscdma));
            break;
        }
        case hidl_discriminator::lte: {
            new (&hidl_u.lte) ::android::hardware::radio::V1_4::CellInfoLte(std::move(other.hidl_u.lte));
            break;
        }
        case hidl_discriminator::nr: {
            new (&hidl_u.nr) ::android::hardware::radio::V1_4::CellInfoNr(std::move(other.hidl_u.nr));
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) other.hidl_d) + ").").c_str());
        }
    }

    hidl_d = other.hidl_d;
}

::android::hardware::radio::V1_4::CellInfo::Info::Info(const Info& other) {
    switch (other.hidl_d) {
        case hidl_discriminator::gsm: {
            new (&hidl_u.gsm) ::android::hardware::radio::V1_2::CellInfoGsm(other.hidl_u.gsm);
            break;
        }
        case hidl_discriminator::cdma: {
            new (&hidl_u.cdma) ::android::hardware::radio::V1_2::CellInfoCdma(other.hidl_u.cdma);
            break;
        }
        case hidl_discriminator::wcdma: {
            new (&hidl_u.wcdma) ::android::hardware::radio::V1_2::CellInfoWcdma(other.hidl_u.wcdma);
            break;
        }
        case hidl_discriminator::tdscdma: {
            new (&hidl_u.tdscdma) ::android::hardware::radio::V1_2::CellInfoTdscdma(other.hidl_u.tdscdma);
            break;
        }
        case hidl_discriminator::lte: {
            new (&hidl_u.lte) ::android::hardware::radio::V1_4::CellInfoLte(other.hidl_u.lte);
            break;
        }
        case hidl_discriminator::nr: {
            new (&hidl_u.nr) ::android::hardware::radio::V1_4::CellInfoNr(other.hidl_u.nr);
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) other.hidl_d) + ").").c_str());
        }
    }

    hidl_d = other.hidl_d;
}

::android::hardware::radio::V1_4::CellInfo::Info& (::android::hardware::radio::V1_4::CellInfo::Info::operator=)(Info&& other) {
    if (this == &other) { return *this; }

    switch (other.hidl_d) {
        case hidl_discriminator::gsm: {
            gsm(std::move(other.hidl_u.gsm));
            break;
        }
        case hidl_discriminator::cdma: {
            cdma(std::move(other.hidl_u.cdma));
            break;
        }
        case hidl_discriminator::wcdma: {
            wcdma(std::move(other.hidl_u.wcdma));
            break;
        }
        case hidl_discriminator::tdscdma: {
            tdscdma(std::move(other.hidl_u.tdscdma));
            break;
        }
        case hidl_discriminator::lte: {
            lte(std::move(other.hidl_u.lte));
            break;
        }
        case hidl_discriminator::nr: {
            nr(std::move(other.hidl_u.nr));
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) other.hidl_d) + ").").c_str());
        }
    }
    return *this;
}

::android::hardware::radio::V1_4::CellInfo::Info& (::android::hardware::radio::V1_4::CellInfo::Info::operator=)(const Info& other) {
    if (this == &other) { return *this; }

    switch (other.hidl_d) {
        case hidl_discriminator::gsm: {
            gsm(other.hidl_u.gsm);
            break;
        }
        case hidl_discriminator::cdma: {
            cdma(other.hidl_u.cdma);
            break;
        }
        case hidl_discriminator::wcdma: {
            wcdma(other.hidl_u.wcdma);
            break;
        }
        case hidl_discriminator::tdscdma: {
            tdscdma(other.hidl_u.tdscdma);
            break;
        }
        case hidl_discriminator::lte: {
            lte(other.hidl_u.lte);
            break;
        }
        case hidl_discriminator::nr: {
            nr(other.hidl_u.nr);
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) other.hidl_d) + ").").c_str());
        }
    }
    return *this;
}

void ::android::hardware::radio::V1_4::CellInfo::Info::hidl_destructUnion() {
    switch (hidl_d) {
        case hidl_discriminator::gsm: {
            ::android::hardware::details::destructElement(&(hidl_u.gsm));
            break;
        }
        case hidl_discriminator::cdma: {
            ::android::hardware::details::destructElement(&(hidl_u.cdma));
            break;
        }
        case hidl_discriminator::wcdma: {
            ::android::hardware::details::destructElement(&(hidl_u.wcdma));
            break;
        }
        case hidl_discriminator::tdscdma: {
            ::android::hardware::details::destructElement(&(hidl_u.tdscdma));
            break;
        }
        case hidl_discriminator::lte: {
            ::android::hardware::details::destructElement(&(hidl_u.lte));
            break;
        }
        case hidl_discriminator::nr: {
            ::android::hardware::details::destructElement(&(hidl_u.nr));
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) hidl_d) + ").").c_str());
        }
    }

}

void ::android::hardware::radio::V1_4::CellInfo::Info::gsm(const ::android::hardware::radio::V1_2::CellInfoGsm& o) {
    if (hidl_d != hidl_discriminator::gsm) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.gsm) ::android::hardware::radio::V1_2::CellInfoGsm(o);
        hidl_d = hidl_discriminator::gsm;
    }
    else if (&(hidl_u.gsm) != &o) {
        hidl_u.gsm = o;
    }
}

void ::android::hardware::radio::V1_4::CellInfo::Info::gsm(::android::hardware::radio::V1_2::CellInfoGsm&& o) {
    if (hidl_d != hidl_discriminator::gsm) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.gsm) ::android::hardware::radio::V1_2::CellInfoGsm(std::move(o));
        hidl_d = hidl_discriminator::gsm;
    }
    else if (&(hidl_u.gsm) != &o) {
        hidl_u.gsm = std::move(o);
    }
}

::android::hardware::radio::V1_2::CellInfoGsm& (::android::hardware::radio::V1_4::CellInfo::Info::gsm)() {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::gsm)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::gsm));
    }

    return hidl_u.gsm;
}

const ::android::hardware::radio::V1_2::CellInfoGsm& (::android::hardware::radio::V1_4::CellInfo::Info::gsm)() const {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::gsm)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::gsm));
    }

    return hidl_u.gsm;
}

void ::android::hardware::radio::V1_4::CellInfo::Info::cdma(const ::android::hardware::radio::V1_2::CellInfoCdma& o) {
    if (hidl_d != hidl_discriminator::cdma) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.cdma) ::android::hardware::radio::V1_2::CellInfoCdma(o);
        hidl_d = hidl_discriminator::cdma;
    }
    else if (&(hidl_u.cdma) != &o) {
        hidl_u.cdma = o;
    }
}

void ::android::hardware::radio::V1_4::CellInfo::Info::cdma(::android::hardware::radio::V1_2::CellInfoCdma&& o) {
    if (hidl_d != hidl_discriminator::cdma) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.cdma) ::android::hardware::radio::V1_2::CellInfoCdma(std::move(o));
        hidl_d = hidl_discriminator::cdma;
    }
    else if (&(hidl_u.cdma) != &o) {
        hidl_u.cdma = std::move(o);
    }
}

::android::hardware::radio::V1_2::CellInfoCdma& (::android::hardware::radio::V1_4::CellInfo::Info::cdma)() {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::cdma)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::cdma));
    }

    return hidl_u.cdma;
}

const ::android::hardware::radio::V1_2::CellInfoCdma& (::android::hardware::radio::V1_4::CellInfo::Info::cdma)() const {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::cdma)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::cdma));
    }

    return hidl_u.cdma;
}

void ::android::hardware::radio::V1_4::CellInfo::Info::wcdma(const ::android::hardware::radio::V1_2::CellInfoWcdma& o) {
    if (hidl_d != hidl_discriminator::wcdma) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.wcdma) ::android::hardware::radio::V1_2::CellInfoWcdma(o);
        hidl_d = hidl_discriminator::wcdma;
    }
    else if (&(hidl_u.wcdma) != &o) {
        hidl_u.wcdma = o;
    }
}

void ::android::hardware::radio::V1_4::CellInfo::Info::wcdma(::android::hardware::radio::V1_2::CellInfoWcdma&& o) {
    if (hidl_d != hidl_discriminator::wcdma) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.wcdma) ::android::hardware::radio::V1_2::CellInfoWcdma(std::move(o));
        hidl_d = hidl_discriminator::wcdma;
    }
    else if (&(hidl_u.wcdma) != &o) {
        hidl_u.wcdma = std::move(o);
    }
}

::android::hardware::radio::V1_2::CellInfoWcdma& (::android::hardware::radio::V1_4::CellInfo::Info::wcdma)() {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::wcdma)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::wcdma));
    }

    return hidl_u.wcdma;
}

const ::android::hardware::radio::V1_2::CellInfoWcdma& (::android::hardware::radio::V1_4::CellInfo::Info::wcdma)() const {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::wcdma)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::wcdma));
    }

    return hidl_u.wcdma;
}

void ::android::hardware::radio::V1_4::CellInfo::Info::tdscdma(const ::android::hardware::radio::V1_2::CellInfoTdscdma& o) {
    if (hidl_d != hidl_discriminator::tdscdma) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.tdscdma) ::android::hardware::radio::V1_2::CellInfoTdscdma(o);
        hidl_d = hidl_discriminator::tdscdma;
    }
    else if (&(hidl_u.tdscdma) != &o) {
        hidl_u.tdscdma = o;
    }
}

void ::android::hardware::radio::V1_4::CellInfo::Info::tdscdma(::android::hardware::radio::V1_2::CellInfoTdscdma&& o) {
    if (hidl_d != hidl_discriminator::tdscdma) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.tdscdma) ::android::hardware::radio::V1_2::CellInfoTdscdma(std::move(o));
        hidl_d = hidl_discriminator::tdscdma;
    }
    else if (&(hidl_u.tdscdma) != &o) {
        hidl_u.tdscdma = std::move(o);
    }
}

::android::hardware::radio::V1_2::CellInfoTdscdma& (::android::hardware::radio::V1_4::CellInfo::Info::tdscdma)() {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::tdscdma)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::tdscdma));
    }

    return hidl_u.tdscdma;
}

const ::android::hardware::radio::V1_2::CellInfoTdscdma& (::android::hardware::radio::V1_4::CellInfo::Info::tdscdma)() const {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::tdscdma)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::tdscdma));
    }

    return hidl_u.tdscdma;
}

void ::android::hardware::radio::V1_4::CellInfo::Info::lte(const ::android::hardware::radio::V1_4::CellInfoLte& o) {
    if (hidl_d != hidl_discriminator::lte) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.lte) ::android::hardware::radio::V1_4::CellInfoLte(o);
        hidl_d = hidl_discriminator::lte;
    }
    else if (&(hidl_u.lte) != &o) {
        hidl_u.lte = o;
    }
}

void ::android::hardware::radio::V1_4::CellInfo::Info::lte(::android::hardware::radio::V1_4::CellInfoLte&& o) {
    if (hidl_d != hidl_discriminator::lte) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.lte) ::android::hardware::radio::V1_4::CellInfoLte(std::move(o));
        hidl_d = hidl_discriminator::lte;
    }
    else if (&(hidl_u.lte) != &o) {
        hidl_u.lte = std::move(o);
    }
}

::android::hardware::radio::V1_4::CellInfoLte& (::android::hardware::radio::V1_4::CellInfo::Info::lte)() {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::lte)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::lte));
    }

    return hidl_u.lte;
}

const ::android::hardware::radio::V1_4::CellInfoLte& (::android::hardware::radio::V1_4::CellInfo::Info::lte)() const {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::lte)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::lte));
    }

    return hidl_u.lte;
}

void ::android::hardware::radio::V1_4::CellInfo::Info::nr(const ::android::hardware::radio::V1_4::CellInfoNr& o) {
    if (hidl_d != hidl_discriminator::nr) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.nr) ::android::hardware::radio::V1_4::CellInfoNr(o);
        hidl_d = hidl_discriminator::nr;
    }
    else if (&(hidl_u.nr) != &o) {
        hidl_u.nr = o;
    }
}

void ::android::hardware::radio::V1_4::CellInfo::Info::nr(::android::hardware::radio::V1_4::CellInfoNr&& o) {
    if (hidl_d != hidl_discriminator::nr) {
        hidl_destructUnion();
        ::std::memset(&hidl_u, 0, sizeof(hidl_u));

        new (&hidl_u.nr) ::android::hardware::radio::V1_4::CellInfoNr(std::move(o));
        hidl_d = hidl_discriminator::nr;
    }
    else if (&(hidl_u.nr) != &o) {
        hidl_u.nr = std::move(o);
    }
}

::android::hardware::radio::V1_4::CellInfoNr& (::android::hardware::radio::V1_4::CellInfo::Info::nr)() {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::nr)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::nr));
    }

    return hidl_u.nr;
}

const ::android::hardware::radio::V1_4::CellInfoNr& (::android::hardware::radio::V1_4::CellInfo::Info::nr)() const {
    if (CC_UNLIKELY(hidl_d != hidl_discriminator::nr)) {
        LOG_ALWAYS_FATAL("Bad safe_union access: safe_union has discriminator %" PRIu64 " but discriminator %" PRIu64 " was accessed.",
                static_cast<uint64_t>(hidl_d), static_cast<uint64_t>(hidl_discriminator::nr));
    }

    return hidl_u.nr;
}

::android::hardware::radio::V1_4::CellInfo::Info::hidl_union::hidl_union() {}

::android::hardware::radio::V1_4::CellInfo::Info::hidl_union::~hidl_union() {}

::android::hardware::radio::V1_4::CellInfo::Info::hidl_discriminator (::android::hardware::radio::V1_4::CellInfo::Info::getDiscriminator)() const {
    return hidl_d;
}

static_assert(sizeof(::android::hardware::MQDescriptor<char, ::android::hardware::kSynchronizedReadWrite>) == 32, "wrong size");
static_assert(sizeof(::android::hardware::hidl_handle) == 16, "wrong size");
static_assert(sizeof(::android::hardware::hidl_memory) == 40, "wrong size");
static_assert(sizeof(::android::hardware::hidl_string) == 16, "wrong size");
static_assert(sizeof(::android::hardware::hidl_vec<char>) == 16, "wrong size");

}  // namespace V1_4
}  // namespace radio
}  // namespace hardware
}  // namespace android
