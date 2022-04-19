#ifndef HIDL_GENERATED_ANDROID_HARDWARE_RADIO_V1_2_TYPES_H
#define HIDL_GENERATED_ANDROID_HARDWARE_RADIO_V1_2_TYPES_H

#include <android/hardware/radio/1.0/types.h>
#include <android/hardware/radio/1.1/types.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hardware {
namespace radio {
namespace V1_2 {

// Forward declaration for forward reference support:
enum class RadioConst : int32_t;
enum class ScanIntervalRange : int32_t;
enum class MaxSearchTimeRange : int32_t;
enum class IncrementalResultsPeriodicityRange : int32_t;
enum class CellConnectionStatus : int32_t;
enum class IndicationFilter : int32_t;
enum class AudioQuality : int32_t;
struct NetworkScanRequest;
struct NetworkScanResult;
struct CellIdentityOperatorNames;
struct CellIdentityCdma;
struct CellIdentityGsm;
struct CellIdentityLte;
struct CellIdentityTdscdma;
struct CellIdentityWcdma;
struct CellInfoGsm;
struct WcdmaSignalStrength;
struct CellInfoWcdma;
struct CellInfoCdma;
struct CellInfoLte;
struct TdscdmaSignalStrength;
struct CellInfoTdscdma;
struct CellInfo;
struct CardStatus;
struct LinkCapacityEstimate;
struct PhysicalChannelConfig;
enum class AccessNetwork : int32_t;
enum class DataRequestReason : int32_t;
struct Call;
struct SignalStrength;
struct CellIdentity;
struct VoiceRegStateResult;
struct DataRegStateResult;

// Order of inner types was changed for forward reference support.

enum class RadioConst : int32_t {
    CDMA_ALPHA_INFO_BUFFER_LENGTH = 64,
    CDMA_NUMBER_INFO_BUFFER_LENGTH = 81,
    MAX_RILDS = 3,
    MAX_SOCKET_NAME_LENGTH = 6,
    MAX_CLIENT_ID_LENGTH = 2,
    MAX_DEBUG_SOCKET_NAME_LENGTH = 12,
    MAX_QEMU_PIPE_NAME_LENGTH = 11,
    MAX_UUID_LENGTH = 64,
    CARD_MAX_APPS = 8,
    CDMA_MAX_NUMBER_OF_INFO_RECS = 10,
    SS_INFO_MAX = 4,
    NUM_SERVICE_CLASSES = 7,
    NUM_TX_POWER_LEVELS = 5,
    RADIO_ACCESS_SPECIFIER_MAX_SIZE = 8,
};

/**
 * values are in seconds
 */
enum class ScanIntervalRange : int32_t {
    MIN = 5,
    MAX = 300,
};

/**
 * value are in seconds
 */
enum class MaxSearchTimeRange : int32_t {
    MIN = 60,
    MAX = 3600,
};

/**
 * values are in seconds
 */
enum class IncrementalResultsPeriodicityRange : int32_t {
    MIN = 1,
    MAX = 10,
};

enum class CellConnectionStatus : int32_t {
    /**
     * Cell is not a serving cell.
     */
    NONE = 0,
    /**
     * UE has connection to cell for signalling and possibly data (3GPP 36.331, 25.331).
     */
    PRIMARY_SERVING = 1, // (::android::hardware::radio::V1_2::CellConnectionStatus.NONE implicitly + 1)
    /**
     * UE has connection to cell for data (3GPP 36.331, 25.331).
     */
    SECONDARY_SERVING = 2, // (::android::hardware::radio::V1_2::CellConnectionStatus.PRIMARY_SERVING implicitly + 1)
};

/**
 * Overwritten from @1.0::IndicationFilter in order to redefine ALL. In the future, this should
 * be extended instead of overwritten.
 */
enum class IndicationFilter : int32_t {
    NONE = 0,
    ALL = -1, // (~0)
    /**
     * When this bit is set, modem must send the signal strength update through
     * IRadioIndication.currentSignalStrength() when all criteria specified by
     * IRadio.setSignalStrengthReportingCriteria() are met.
     */
    SIGNAL_STRENGTH = 1, // (1 << 0)
    /**
     * When this bit is set, modem must invoke IRadioIndication.networkStateChanged() when any field
     * in VoiceRegStateResult or DataRegStateResult changes. When this bit is not set, modem must
     * suppress IRadioIndication.networkStateChanged() when there are only changes from
     * insignificant fields. Modem must invoke IRadioIndication.networkStateChanged() when
     * significant fields are updated regardless of whether this bit is set.
     * 
     * The following fields are considered significant: VoiceRegStateResult.regState,
     * VoiceRegStateResult.rat, DataRegStateResult.regState, DataRegStateResult.rat.
     */
    FULL_NETWORK_STATE = 2, // (1 << 1)
    /**
     * When this bit is set, modem must send IRadioIndication.dataCallListChanged() whenever any
     * field in ITypes.SetupDataCallResult changes. When this bit is not set, modem must suppress
     * the indication when the only changed field is 'active' (for data dormancy). For all other
     * field changes, the modem must send IRadioIndication.dataCallListChanged() regardless of
     * whether this bit is set.
     */
    DATA_CALL_DORMANCY_CHANGED = 4, // (1 << 2)
    /**
     * When this bit is set, modem must send the link capacity update through
     * IRadioIndication.currentLinkCapacityEstimate() when all criteria specified by
     * IRadio.setLinkCapacityReportingCriteria() are met.
     */
    LINK_CAPACITY_ESTIMATE = 8, // (1 << 3)
    /**
     * When this bit is set, the modem must send the physical channel configuration update through
     * IRadioIndication.currentPhysicalChannelConfigs() when the configuration has changed. It is
     * recommended that this be reported whenever link capacity or signal strength is reported.
     */
    PHYSICAL_CHANNEL_CONFIG = 16, // (1 << 4)
};

/**
 * Audio codec which is used on GSM, UMTS, and CDMA. These values must be opaque
 * to the Android framework. Only for display.
 */
enum class AudioQuality : int32_t {
    /**
     * Unspecified audio codec  */
    UNSPECIFIED = 0,
    /**
     * AMR (Narrowband) audio codec  */
    AMR = 1, // (::android::hardware::radio::V1_2::AudioQuality.UNSPECIFIED implicitly + 1)
    /**
     * AMR (Wideband) audio codec  */
    AMR_WB = 2, // (::android::hardware::radio::V1_2::AudioQuality.AMR implicitly + 1)
    /**
     * GSM Enhanced Full-Rate audio codec  */
    GSM_EFR = 3, // (::android::hardware::radio::V1_2::AudioQuality.AMR_WB implicitly + 1)
    /**
     * GSM Full-Rate audio codec  */
    GSM_FR = 4, // (::android::hardware::radio::V1_2::AudioQuality.GSM_EFR implicitly + 1)
    /**
     * GSM Half-Rate audio codec  */
    GSM_HR = 5, // (::android::hardware::radio::V1_2::AudioQuality.GSM_FR implicitly + 1)
    /**
     * Enhanced Variable rate codec  */
    EVRC = 6, // (::android::hardware::radio::V1_2::AudioQuality.GSM_HR implicitly + 1)
    /**
     * Enhanced Variable rate codec revision B  */
    EVRC_B = 7, // (::android::hardware::radio::V1_2::AudioQuality.EVRC implicitly + 1)
    /**
     * Enhanced Variable rate codec (Wideband)  */
    EVRC_WB = 8, // (::android::hardware::radio::V1_2::AudioQuality.EVRC_B implicitly + 1)
    /**
     * Enhanced Variable rate codec (Narrowband)  */
    EVRC_NW = 9, // (::android::hardware::radio::V1_2::AudioQuality.EVRC_WB implicitly + 1)
};

struct NetworkScanRequest final {
    ::android::hardware::radio::V1_1::ScanType type __attribute__ ((aligned(4)));
    int32_t interval __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_1::RadioAccessSpecifier> specifiers __attribute__ ((aligned(8)));
    int32_t maxSearchTime __attribute__ ((aligned(4)));
    bool incrementalResults __attribute__ ((aligned(1)));
    int32_t incrementalResultsPeriodicity __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::hidl_string> mccMncs __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::NetworkScanRequest, type) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::NetworkScanRequest, interval) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::NetworkScanRequest, specifiers) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::NetworkScanRequest, maxSearchTime) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::NetworkScanRequest, incrementalResults) == 28, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::NetworkScanRequest, incrementalResultsPeriodicity) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::NetworkScanRequest, mccMncs) == 40, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::NetworkScanRequest) == 56, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::NetworkScanRequest) == 8, "wrong alignment");

struct NetworkScanResult final {
    ::android::hardware::radio::V1_1::ScanStatus status __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::RadioError error __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_2::CellInfo> networkInfos __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::NetworkScanResult, status) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::NetworkScanResult, error) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::NetworkScanResult, networkInfos) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::NetworkScanResult) == 24, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::NetworkScanResult) == 8, "wrong alignment");

struct CellIdentityOperatorNames final {
    ::android::hardware::hidl_string alphaLong __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string alphaShort __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentityOperatorNames, alphaLong) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentityOperatorNames, alphaShort) == 16, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::CellIdentityOperatorNames) == 32, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::CellIdentityOperatorNames) == 8, "wrong alignment");

struct CellIdentityCdma final {
    ::android::hardware::radio::V1_0::CellIdentityCdma base __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_2::CellIdentityOperatorNames operatorNames __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentityCdma, base) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentityCdma, operatorNames) == 24, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::CellIdentityCdma) == 56, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::CellIdentityCdma) == 8, "wrong alignment");

struct CellIdentityGsm final {
    ::android::hardware::radio::V1_0::CellIdentityGsm base __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_2::CellIdentityOperatorNames operatorNames __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentityGsm, base) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentityGsm, operatorNames) == 48, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::CellIdentityGsm) == 80, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::CellIdentityGsm) == 8, "wrong alignment");

struct CellIdentityLte final {
    ::android::hardware::radio::V1_0::CellIdentityLte base __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_2::CellIdentityOperatorNames operatorNames __attribute__ ((aligned(8)));
    int32_t bandwidth __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentityLte, base) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentityLte, operatorNames) == 48, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentityLte, bandwidth) == 80, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::CellIdentityLte) == 88, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::CellIdentityLte) == 8, "wrong alignment");

struct CellIdentityTdscdma final {
    ::android::hardware::radio::V1_0::CellIdentityTdscdma base __attribute__ ((aligned(8)));
    int32_t uarfcn __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_2::CellIdentityOperatorNames operatorNames __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentityTdscdma, base) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentityTdscdma, uarfcn) == 48, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentityTdscdma, operatorNames) == 56, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::CellIdentityTdscdma) == 88, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::CellIdentityTdscdma) == 8, "wrong alignment");

struct CellIdentityWcdma final {
    ::android::hardware::radio::V1_0::CellIdentityWcdma base __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_2::CellIdentityOperatorNames operatorNames __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentityWcdma, base) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentityWcdma, operatorNames) == 48, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::CellIdentityWcdma) == 80, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::CellIdentityWcdma) == 8, "wrong alignment");

struct CellInfoGsm final {
    ::android::hardware::radio::V1_2::CellIdentityGsm cellIdentityGsm __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::GsmSignalStrength signalStrengthGsm __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::CellInfoGsm, cellIdentityGsm) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellInfoGsm, signalStrengthGsm) == 80, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::CellInfoGsm) == 96, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::CellInfoGsm) == 8, "wrong alignment");

struct WcdmaSignalStrength final {
    ::android::hardware::radio::V1_0::WcdmaSignalStrength base __attribute__ ((aligned(4)));
    uint32_t rscp __attribute__ ((aligned(4)));
    uint32_t ecno __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::WcdmaSignalStrength, base) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::WcdmaSignalStrength, rscp) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::WcdmaSignalStrength, ecno) == 12, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::WcdmaSignalStrength) == 16, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::WcdmaSignalStrength) == 4, "wrong alignment");

struct CellInfoWcdma final {
    ::android::hardware::radio::V1_2::CellIdentityWcdma cellIdentityWcdma __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_2::WcdmaSignalStrength signalStrengthWcdma __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::CellInfoWcdma, cellIdentityWcdma) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellInfoWcdma, signalStrengthWcdma) == 80, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::CellInfoWcdma) == 96, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::CellInfoWcdma) == 8, "wrong alignment");

struct CellInfoCdma final {
    ::android::hardware::radio::V1_2::CellIdentityCdma cellIdentityCdma __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::CdmaSignalStrength signalStrengthCdma __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::EvdoSignalStrength signalStrengthEvdo __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::CellInfoCdma, cellIdentityCdma) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellInfoCdma, signalStrengthCdma) == 56, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellInfoCdma, signalStrengthEvdo) == 64, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::CellInfoCdma) == 80, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::CellInfoCdma) == 8, "wrong alignment");

struct CellInfoLte final {
    ::android::hardware::radio::V1_2::CellIdentityLte cellIdentityLte __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::LteSignalStrength signalStrengthLte __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::CellInfoLte, cellIdentityLte) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellInfoLte, signalStrengthLte) == 88, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::CellInfoLte) == 112, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::CellInfoLte) == 8, "wrong alignment");

struct TdscdmaSignalStrength final {
    uint32_t signalStrength __attribute__ ((aligned(4)));
    uint32_t bitErrorRate __attribute__ ((aligned(4)));
    uint32_t rscp __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::TdscdmaSignalStrength, signalStrength) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::TdscdmaSignalStrength, bitErrorRate) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::TdscdmaSignalStrength, rscp) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::TdscdmaSignalStrength) == 12, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::TdscdmaSignalStrength) == 4, "wrong alignment");

struct CellInfoTdscdma final {
    ::android::hardware::radio::V1_2::CellIdentityTdscdma cellIdentityTdscdma __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_2::TdscdmaSignalStrength signalStrengthTdscdma __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::CellInfoTdscdma, cellIdentityTdscdma) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellInfoTdscdma, signalStrengthTdscdma) == 88, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::CellInfoTdscdma) == 104, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::CellInfoTdscdma) == 8, "wrong alignment");

struct CellInfo final {
    ::android::hardware::radio::V1_0::CellInfoType cellInfoType __attribute__ ((aligned(4)));
    bool registered __attribute__ ((aligned(1)));
    ::android::hardware::radio::V1_0::TimeStampType timeStampType __attribute__ ((aligned(4)));
    uint64_t timeStamp __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_2::CellInfoGsm> gsm __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_2::CellInfoCdma> cdma __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_2::CellInfoLte> lte __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_2::CellInfoWcdma> wcdma __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_2::CellInfoTdscdma> tdscdma __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_2::CellConnectionStatus connectionStatus __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::CellInfo, cellInfoType) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellInfo, registered) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellInfo, timeStampType) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellInfo, timeStamp) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellInfo, gsm) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellInfo, cdma) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellInfo, lte) == 56, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellInfo, wcdma) == 72, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellInfo, tdscdma) == 88, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellInfo, connectionStatus) == 104, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::CellInfo) == 112, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::CellInfo) == 8, "wrong alignment");

struct CardStatus final {
    ::android::hardware::radio::V1_0::CardStatus base __attribute__ ((aligned(8)));
    uint32_t physicalSlotId __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string atr __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string iccid __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::CardStatus, base) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CardStatus, physicalSlotId) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CardStatus, atr) == 48, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CardStatus, iccid) == 64, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::CardStatus) == 80, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::CardStatus) == 8, "wrong alignment");

struct LinkCapacityEstimate final {
    uint32_t downlinkCapacityKbps __attribute__ ((aligned(4)));
    uint32_t uplinkCapacityKbps __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::LinkCapacityEstimate, downlinkCapacityKbps) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::LinkCapacityEstimate, uplinkCapacityKbps) == 4, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::LinkCapacityEstimate) == 8, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::LinkCapacityEstimate) == 4, "wrong alignment");

struct PhysicalChannelConfig final {
    ::android::hardware::radio::V1_2::CellConnectionStatus status __attribute__ ((aligned(4)));
    int32_t cellBandwidthDownlink __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::PhysicalChannelConfig, status) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::PhysicalChannelConfig, cellBandwidthDownlink) == 4, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::PhysicalChannelConfig) == 8, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::PhysicalChannelConfig) == 4, "wrong alignment");

enum class AccessNetwork : int32_t {
    /**
     * GSM EDGE Radio Access Network  */
    GERAN = 1,
    /**
     * Universal Terrestrial Radio Access Network  */
    UTRAN = 2,
    /**
     * Evolved Universal Terrestrial Radio Access Network  */
    EUTRAN = 3,
    /**
     * CDMA 2000 network  */
    CDMA2000 = 4,
    /**
     * Interworking Wireless LAN  */
    IWLAN = 5,
};

enum class DataRequestReason : int32_t {
    /**
     * The reason of the data request is normal
     */
    NORMAL = 1,
    /**
     * The reason of the data request is device shutdown
     */
    SHUTDOWN = 2,
    /**
     * The reason of the data request is IWLAN data handover to another transport
     * (e.g. from cellular to wifi or vise versa)
     */
    HANDOVER = 3,
};

struct Call final {
    ::android::hardware::radio::V1_0::Call base __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_2::AudioQuality audioQuality __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::Call, base) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::Call, audioQuality) == 88, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::Call) == 96, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::Call) == 8, "wrong alignment");

struct SignalStrength final {
    ::android::hardware::radio::V1_0::GsmSignalStrength gsm __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::CdmaSignalStrength cdma __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::EvdoSignalStrength evdo __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::LteSignalStrength lte __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::TdScdmaSignalStrength tdScdma __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_2::WcdmaSignalStrength wcdma __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::SignalStrength, gsm) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::SignalStrength, cdma) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::SignalStrength, evdo) == 20, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::SignalStrength, lte) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::SignalStrength, tdScdma) == 56, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::SignalStrength, wcdma) == 60, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::SignalStrength) == 76, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::SignalStrength) == 4, "wrong alignment");

struct CellIdentity final {
    ::android::hardware::radio::V1_0::CellInfoType cellInfoType __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_2::CellIdentityGsm> cellIdentityGsm __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_2::CellIdentityWcdma> cellIdentityWcdma __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_2::CellIdentityCdma> cellIdentityCdma __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_2::CellIdentityLte> cellIdentityLte __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_2::CellIdentityTdscdma> cellIdentityTdscdma __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentity, cellInfoType) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentity, cellIdentityGsm) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentity, cellIdentityWcdma) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentity, cellIdentityCdma) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentity, cellIdentityLte) == 56, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::CellIdentity, cellIdentityTdscdma) == 72, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::CellIdentity) == 88, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::CellIdentity) == 8, "wrong alignment");

struct VoiceRegStateResult final {
    ::android::hardware::radio::V1_0::RegState regState __attribute__ ((aligned(4)));
    int32_t rat __attribute__ ((aligned(4)));
    bool cssSupported __attribute__ ((aligned(1)));
    int32_t roamingIndicator __attribute__ ((aligned(4)));
    int32_t systemIsInPrl __attribute__ ((aligned(4)));
    int32_t defaultRoamingIndicator __attribute__ ((aligned(4)));
    int32_t reasonForDenial __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_2::CellIdentity cellIdentity __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::VoiceRegStateResult, regState) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::VoiceRegStateResult, rat) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::VoiceRegStateResult, cssSupported) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::VoiceRegStateResult, roamingIndicator) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::VoiceRegStateResult, systemIsInPrl) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::VoiceRegStateResult, defaultRoamingIndicator) == 20, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::VoiceRegStateResult, reasonForDenial) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::VoiceRegStateResult, cellIdentity) == 32, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::VoiceRegStateResult) == 120, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::VoiceRegStateResult) == 8, "wrong alignment");

struct DataRegStateResult final {
    ::android::hardware::radio::V1_0::RegState regState __attribute__ ((aligned(4)));
    int32_t rat __attribute__ ((aligned(4)));
    int32_t reasonDataDenied __attribute__ ((aligned(4)));
    int32_t maxDataCalls __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_2::CellIdentity cellIdentity __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_2::DataRegStateResult, regState) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::DataRegStateResult, rat) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::DataRegStateResult, reasonDataDenied) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::DataRegStateResult, maxDataCalls) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_2::DataRegStateResult, cellIdentity) == 16, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_2::DataRegStateResult) == 104, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_2::DataRegStateResult) == 8, "wrong alignment");

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::RadioConst lhs, const ::android::hardware::radio::V1_2::RadioConst rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_2::RadioConst rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::RadioConst lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::RadioConst lhs, const ::android::hardware::radio::V1_2::RadioConst rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_2::RadioConst rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::RadioConst lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_2::RadioConst e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_2::RadioConst e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_2::RadioConst>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_2::RadioConst> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_2::RadioConst::CDMA_ALPHA_INFO_BUFFER_LENGTH) == static_cast<int32_t>(::android::hardware::radio::V1_2::RadioConst::CDMA_ALPHA_INFO_BUFFER_LENGTH)) {
        os += (first ? "" : " | ");
        os += "CDMA_ALPHA_INFO_BUFFER_LENGTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::RadioConst::CDMA_ALPHA_INFO_BUFFER_LENGTH;
    }
    if ((o & ::android::hardware::radio::V1_2::RadioConst::CDMA_NUMBER_INFO_BUFFER_LENGTH) == static_cast<int32_t>(::android::hardware::radio::V1_2::RadioConst::CDMA_NUMBER_INFO_BUFFER_LENGTH)) {
        os += (first ? "" : " | ");
        os += "CDMA_NUMBER_INFO_BUFFER_LENGTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::RadioConst::CDMA_NUMBER_INFO_BUFFER_LENGTH;
    }
    if ((o & ::android::hardware::radio::V1_2::RadioConst::MAX_RILDS) == static_cast<int32_t>(::android::hardware::radio::V1_2::RadioConst::MAX_RILDS)) {
        os += (first ? "" : " | ");
        os += "MAX_RILDS";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::RadioConst::MAX_RILDS;
    }
    if ((o & ::android::hardware::radio::V1_2::RadioConst::MAX_SOCKET_NAME_LENGTH) == static_cast<int32_t>(::android::hardware::radio::V1_2::RadioConst::MAX_SOCKET_NAME_LENGTH)) {
        os += (first ? "" : " | ");
        os += "MAX_SOCKET_NAME_LENGTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::RadioConst::MAX_SOCKET_NAME_LENGTH;
    }
    if ((o & ::android::hardware::radio::V1_2::RadioConst::MAX_CLIENT_ID_LENGTH) == static_cast<int32_t>(::android::hardware::radio::V1_2::RadioConst::MAX_CLIENT_ID_LENGTH)) {
        os += (first ? "" : " | ");
        os += "MAX_CLIENT_ID_LENGTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::RadioConst::MAX_CLIENT_ID_LENGTH;
    }
    if ((o & ::android::hardware::radio::V1_2::RadioConst::MAX_DEBUG_SOCKET_NAME_LENGTH) == static_cast<int32_t>(::android::hardware::radio::V1_2::RadioConst::MAX_DEBUG_SOCKET_NAME_LENGTH)) {
        os += (first ? "" : " | ");
        os += "MAX_DEBUG_SOCKET_NAME_LENGTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::RadioConst::MAX_DEBUG_SOCKET_NAME_LENGTH;
    }
    if ((o & ::android::hardware::radio::V1_2::RadioConst::MAX_QEMU_PIPE_NAME_LENGTH) == static_cast<int32_t>(::android::hardware::radio::V1_2::RadioConst::MAX_QEMU_PIPE_NAME_LENGTH)) {
        os += (first ? "" : " | ");
        os += "MAX_QEMU_PIPE_NAME_LENGTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::RadioConst::MAX_QEMU_PIPE_NAME_LENGTH;
    }
    if ((o & ::android::hardware::radio::V1_2::RadioConst::MAX_UUID_LENGTH) == static_cast<int32_t>(::android::hardware::radio::V1_2::RadioConst::MAX_UUID_LENGTH)) {
        os += (first ? "" : " | ");
        os += "MAX_UUID_LENGTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::RadioConst::MAX_UUID_LENGTH;
    }
    if ((o & ::android::hardware::radio::V1_2::RadioConst::CARD_MAX_APPS) == static_cast<int32_t>(::android::hardware::radio::V1_2::RadioConst::CARD_MAX_APPS)) {
        os += (first ? "" : " | ");
        os += "CARD_MAX_APPS";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::RadioConst::CARD_MAX_APPS;
    }
    if ((o & ::android::hardware::radio::V1_2::RadioConst::CDMA_MAX_NUMBER_OF_INFO_RECS) == static_cast<int32_t>(::android::hardware::radio::V1_2::RadioConst::CDMA_MAX_NUMBER_OF_INFO_RECS)) {
        os += (first ? "" : " | ");
        os += "CDMA_MAX_NUMBER_OF_INFO_RECS";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::RadioConst::CDMA_MAX_NUMBER_OF_INFO_RECS;
    }
    if ((o & ::android::hardware::radio::V1_2::RadioConst::SS_INFO_MAX) == static_cast<int32_t>(::android::hardware::radio::V1_2::RadioConst::SS_INFO_MAX)) {
        os += (first ? "" : " | ");
        os += "SS_INFO_MAX";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::RadioConst::SS_INFO_MAX;
    }
    if ((o & ::android::hardware::radio::V1_2::RadioConst::NUM_SERVICE_CLASSES) == static_cast<int32_t>(::android::hardware::radio::V1_2::RadioConst::NUM_SERVICE_CLASSES)) {
        os += (first ? "" : " | ");
        os += "NUM_SERVICE_CLASSES";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::RadioConst::NUM_SERVICE_CLASSES;
    }
    if ((o & ::android::hardware::radio::V1_2::RadioConst::NUM_TX_POWER_LEVELS) == static_cast<int32_t>(::android::hardware::radio::V1_2::RadioConst::NUM_TX_POWER_LEVELS)) {
        os += (first ? "" : " | ");
        os += "NUM_TX_POWER_LEVELS";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::RadioConst::NUM_TX_POWER_LEVELS;
    }
    if ((o & ::android::hardware::radio::V1_2::RadioConst::RADIO_ACCESS_SPECIFIER_MAX_SIZE) == static_cast<int32_t>(::android::hardware::radio::V1_2::RadioConst::RADIO_ACCESS_SPECIFIER_MAX_SIZE)) {
        os += (first ? "" : " | ");
        os += "RADIO_ACCESS_SPECIFIER_MAX_SIZE";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::RadioConst::RADIO_ACCESS_SPECIFIER_MAX_SIZE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_2::RadioConst o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_2::RadioConst::CDMA_ALPHA_INFO_BUFFER_LENGTH) {
        return "CDMA_ALPHA_INFO_BUFFER_LENGTH";
    }
    if (o == ::android::hardware::radio::V1_2::RadioConst::CDMA_NUMBER_INFO_BUFFER_LENGTH) {
        return "CDMA_NUMBER_INFO_BUFFER_LENGTH";
    }
    if (o == ::android::hardware::radio::V1_2::RadioConst::MAX_RILDS) {
        return "MAX_RILDS";
    }
    if (o == ::android::hardware::radio::V1_2::RadioConst::MAX_SOCKET_NAME_LENGTH) {
        return "MAX_SOCKET_NAME_LENGTH";
    }
    if (o == ::android::hardware::radio::V1_2::RadioConst::MAX_CLIENT_ID_LENGTH) {
        return "MAX_CLIENT_ID_LENGTH";
    }
    if (o == ::android::hardware::radio::V1_2::RadioConst::MAX_DEBUG_SOCKET_NAME_LENGTH) {
        return "MAX_DEBUG_SOCKET_NAME_LENGTH";
    }
    if (o == ::android::hardware::radio::V1_2::RadioConst::MAX_QEMU_PIPE_NAME_LENGTH) {
        return "MAX_QEMU_PIPE_NAME_LENGTH";
    }
    if (o == ::android::hardware::radio::V1_2::RadioConst::MAX_UUID_LENGTH) {
        return "MAX_UUID_LENGTH";
    }
    if (o == ::android::hardware::radio::V1_2::RadioConst::CARD_MAX_APPS) {
        return "CARD_MAX_APPS";
    }
    if (o == ::android::hardware::radio::V1_2::RadioConst::CDMA_MAX_NUMBER_OF_INFO_RECS) {
        return "CDMA_MAX_NUMBER_OF_INFO_RECS";
    }
    if (o == ::android::hardware::radio::V1_2::RadioConst::SS_INFO_MAX) {
        return "SS_INFO_MAX";
    }
    if (o == ::android::hardware::radio::V1_2::RadioConst::NUM_SERVICE_CLASSES) {
        return "NUM_SERVICE_CLASSES";
    }
    if (o == ::android::hardware::radio::V1_2::RadioConst::NUM_TX_POWER_LEVELS) {
        return "NUM_TX_POWER_LEVELS";
    }
    if (o == ::android::hardware::radio::V1_2::RadioConst::RADIO_ACCESS_SPECIFIER_MAX_SIZE) {
        return "RADIO_ACCESS_SPECIFIER_MAX_SIZE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::ScanIntervalRange lhs, const ::android::hardware::radio::V1_2::ScanIntervalRange rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_2::ScanIntervalRange rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::ScanIntervalRange lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::ScanIntervalRange lhs, const ::android::hardware::radio::V1_2::ScanIntervalRange rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_2::ScanIntervalRange rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::ScanIntervalRange lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_2::ScanIntervalRange e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_2::ScanIntervalRange e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_2::ScanIntervalRange>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_2::ScanIntervalRange> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_2::ScanIntervalRange::MIN) == static_cast<int32_t>(::android::hardware::radio::V1_2::ScanIntervalRange::MIN)) {
        os += (first ? "" : " | ");
        os += "MIN";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::ScanIntervalRange::MIN;
    }
    if ((o & ::android::hardware::radio::V1_2::ScanIntervalRange::MAX) == static_cast<int32_t>(::android::hardware::radio::V1_2::ScanIntervalRange::MAX)) {
        os += (first ? "" : " | ");
        os += "MAX";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::ScanIntervalRange::MAX;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_2::ScanIntervalRange o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_2::ScanIntervalRange::MIN) {
        return "MIN";
    }
    if (o == ::android::hardware::radio::V1_2::ScanIntervalRange::MAX) {
        return "MAX";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::MaxSearchTimeRange lhs, const ::android::hardware::radio::V1_2::MaxSearchTimeRange rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_2::MaxSearchTimeRange rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::MaxSearchTimeRange lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::MaxSearchTimeRange lhs, const ::android::hardware::radio::V1_2::MaxSearchTimeRange rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_2::MaxSearchTimeRange rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::MaxSearchTimeRange lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_2::MaxSearchTimeRange e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_2::MaxSearchTimeRange e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_2::MaxSearchTimeRange>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_2::MaxSearchTimeRange> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_2::MaxSearchTimeRange::MIN) == static_cast<int32_t>(::android::hardware::radio::V1_2::MaxSearchTimeRange::MIN)) {
        os += (first ? "" : " | ");
        os += "MIN";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::MaxSearchTimeRange::MIN;
    }
    if ((o & ::android::hardware::radio::V1_2::MaxSearchTimeRange::MAX) == static_cast<int32_t>(::android::hardware::radio::V1_2::MaxSearchTimeRange::MAX)) {
        os += (first ? "" : " | ");
        os += "MAX";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::MaxSearchTimeRange::MAX;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_2::MaxSearchTimeRange o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_2::MaxSearchTimeRange::MIN) {
        return "MIN";
    }
    if (o == ::android::hardware::radio::V1_2::MaxSearchTimeRange::MAX) {
        return "MAX";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange lhs, const ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange lhs, const ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange::MIN) == static_cast<int32_t>(::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange::MIN)) {
        os += (first ? "" : " | ");
        os += "MIN";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange::MIN;
    }
    if ((o & ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange::MAX) == static_cast<int32_t>(::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange::MAX)) {
        os += (first ? "" : " | ");
        os += "MAX";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange::MAX;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange::MIN) {
        return "MIN";
    }
    if (o == ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange::MAX) {
        return "MAX";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::CellConnectionStatus lhs, const ::android::hardware::radio::V1_2::CellConnectionStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_2::CellConnectionStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::CellConnectionStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::CellConnectionStatus lhs, const ::android::hardware::radio::V1_2::CellConnectionStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_2::CellConnectionStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::CellConnectionStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_2::CellConnectionStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_2::CellConnectionStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_2::CellConnectionStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_2::CellConnectionStatus> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_2::CellConnectionStatus::NONE) == static_cast<int32_t>(::android::hardware::radio::V1_2::CellConnectionStatus::NONE)) {
        os += (first ? "" : " | ");
        os += "NONE";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::CellConnectionStatus::NONE;
    }
    if ((o & ::android::hardware::radio::V1_2::CellConnectionStatus::PRIMARY_SERVING) == static_cast<int32_t>(::android::hardware::radio::V1_2::CellConnectionStatus::PRIMARY_SERVING)) {
        os += (first ? "" : " | ");
        os += "PRIMARY_SERVING";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::CellConnectionStatus::PRIMARY_SERVING;
    }
    if ((o & ::android::hardware::radio::V1_2::CellConnectionStatus::SECONDARY_SERVING) == static_cast<int32_t>(::android::hardware::radio::V1_2::CellConnectionStatus::SECONDARY_SERVING)) {
        os += (first ? "" : " | ");
        os += "SECONDARY_SERVING";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::CellConnectionStatus::SECONDARY_SERVING;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_2::CellConnectionStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_2::CellConnectionStatus::NONE) {
        return "NONE";
    }
    if (o == ::android::hardware::radio::V1_2::CellConnectionStatus::PRIMARY_SERVING) {
        return "PRIMARY_SERVING";
    }
    if (o == ::android::hardware::radio::V1_2::CellConnectionStatus::SECONDARY_SERVING) {
        return "SECONDARY_SERVING";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::IndicationFilter lhs, const ::android::hardware::radio::V1_2::IndicationFilter rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_2::IndicationFilter rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::IndicationFilter lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::IndicationFilter lhs, const ::android::hardware::radio::V1_2::IndicationFilter rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_2::IndicationFilter rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::IndicationFilter lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_2::IndicationFilter e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_2::IndicationFilter e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_2::IndicationFilter>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_2::IndicationFilter> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_2::IndicationFilter::NONE) == static_cast<int32_t>(::android::hardware::radio::V1_2::IndicationFilter::NONE)) {
        os += (first ? "" : " | ");
        os += "NONE";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::IndicationFilter::NONE;
    }
    if ((o & ::android::hardware::radio::V1_2::IndicationFilter::ALL) == static_cast<int32_t>(::android::hardware::radio::V1_2::IndicationFilter::ALL)) {
        os += (first ? "" : " | ");
        os += "ALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::IndicationFilter::ALL;
    }
    if ((o & ::android::hardware::radio::V1_2::IndicationFilter::SIGNAL_STRENGTH) == static_cast<int32_t>(::android::hardware::radio::V1_2::IndicationFilter::SIGNAL_STRENGTH)) {
        os += (first ? "" : " | ");
        os += "SIGNAL_STRENGTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::IndicationFilter::SIGNAL_STRENGTH;
    }
    if ((o & ::android::hardware::radio::V1_2::IndicationFilter::FULL_NETWORK_STATE) == static_cast<int32_t>(::android::hardware::radio::V1_2::IndicationFilter::FULL_NETWORK_STATE)) {
        os += (first ? "" : " | ");
        os += "FULL_NETWORK_STATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::IndicationFilter::FULL_NETWORK_STATE;
    }
    if ((o & ::android::hardware::radio::V1_2::IndicationFilter::DATA_CALL_DORMANCY_CHANGED) == static_cast<int32_t>(::android::hardware::radio::V1_2::IndicationFilter::DATA_CALL_DORMANCY_CHANGED)) {
        os += (first ? "" : " | ");
        os += "DATA_CALL_DORMANCY_CHANGED";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::IndicationFilter::DATA_CALL_DORMANCY_CHANGED;
    }
    if ((o & ::android::hardware::radio::V1_2::IndicationFilter::LINK_CAPACITY_ESTIMATE) == static_cast<int32_t>(::android::hardware::radio::V1_2::IndicationFilter::LINK_CAPACITY_ESTIMATE)) {
        os += (first ? "" : " | ");
        os += "LINK_CAPACITY_ESTIMATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::IndicationFilter::LINK_CAPACITY_ESTIMATE;
    }
    if ((o & ::android::hardware::radio::V1_2::IndicationFilter::PHYSICAL_CHANNEL_CONFIG) == static_cast<int32_t>(::android::hardware::radio::V1_2::IndicationFilter::PHYSICAL_CHANNEL_CONFIG)) {
        os += (first ? "" : " | ");
        os += "PHYSICAL_CHANNEL_CONFIG";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::IndicationFilter::PHYSICAL_CHANNEL_CONFIG;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_2::IndicationFilter o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_2::IndicationFilter::NONE) {
        return "NONE";
    }
    if (o == ::android::hardware::radio::V1_2::IndicationFilter::ALL) {
        return "ALL";
    }
    if (o == ::android::hardware::radio::V1_2::IndicationFilter::SIGNAL_STRENGTH) {
        return "SIGNAL_STRENGTH";
    }
    if (o == ::android::hardware::radio::V1_2::IndicationFilter::FULL_NETWORK_STATE) {
        return "FULL_NETWORK_STATE";
    }
    if (o == ::android::hardware::radio::V1_2::IndicationFilter::DATA_CALL_DORMANCY_CHANGED) {
        return "DATA_CALL_DORMANCY_CHANGED";
    }
    if (o == ::android::hardware::radio::V1_2::IndicationFilter::LINK_CAPACITY_ESTIMATE) {
        return "LINK_CAPACITY_ESTIMATE";
    }
    if (o == ::android::hardware::radio::V1_2::IndicationFilter::PHYSICAL_CHANNEL_CONFIG) {
        return "PHYSICAL_CHANNEL_CONFIG";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::AudioQuality lhs, const ::android::hardware::radio::V1_2::AudioQuality rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_2::AudioQuality rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::AudioQuality lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::AudioQuality lhs, const ::android::hardware::radio::V1_2::AudioQuality rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_2::AudioQuality rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::AudioQuality lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_2::AudioQuality e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_2::AudioQuality e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_2::AudioQuality>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_2::AudioQuality> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_2::AudioQuality::UNSPECIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_2::AudioQuality::UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "UNSPECIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::AudioQuality::UNSPECIFIED;
    }
    if ((o & ::android::hardware::radio::V1_2::AudioQuality::AMR) == static_cast<int32_t>(::android::hardware::radio::V1_2::AudioQuality::AMR)) {
        os += (first ? "" : " | ");
        os += "AMR";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::AudioQuality::AMR;
    }
    if ((o & ::android::hardware::radio::V1_2::AudioQuality::AMR_WB) == static_cast<int32_t>(::android::hardware::radio::V1_2::AudioQuality::AMR_WB)) {
        os += (first ? "" : " | ");
        os += "AMR_WB";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::AudioQuality::AMR_WB;
    }
    if ((o & ::android::hardware::radio::V1_2::AudioQuality::GSM_EFR) == static_cast<int32_t>(::android::hardware::radio::V1_2::AudioQuality::GSM_EFR)) {
        os += (first ? "" : " | ");
        os += "GSM_EFR";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::AudioQuality::GSM_EFR;
    }
    if ((o & ::android::hardware::radio::V1_2::AudioQuality::GSM_FR) == static_cast<int32_t>(::android::hardware::radio::V1_2::AudioQuality::GSM_FR)) {
        os += (first ? "" : " | ");
        os += "GSM_FR";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::AudioQuality::GSM_FR;
    }
    if ((o & ::android::hardware::radio::V1_2::AudioQuality::GSM_HR) == static_cast<int32_t>(::android::hardware::radio::V1_2::AudioQuality::GSM_HR)) {
        os += (first ? "" : " | ");
        os += "GSM_HR";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::AudioQuality::GSM_HR;
    }
    if ((o & ::android::hardware::radio::V1_2::AudioQuality::EVRC) == static_cast<int32_t>(::android::hardware::radio::V1_2::AudioQuality::EVRC)) {
        os += (first ? "" : " | ");
        os += "EVRC";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::AudioQuality::EVRC;
    }
    if ((o & ::android::hardware::radio::V1_2::AudioQuality::EVRC_B) == static_cast<int32_t>(::android::hardware::radio::V1_2::AudioQuality::EVRC_B)) {
        os += (first ? "" : " | ");
        os += "EVRC_B";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::AudioQuality::EVRC_B;
    }
    if ((o & ::android::hardware::radio::V1_2::AudioQuality::EVRC_WB) == static_cast<int32_t>(::android::hardware::radio::V1_2::AudioQuality::EVRC_WB)) {
        os += (first ? "" : " | ");
        os += "EVRC_WB";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::AudioQuality::EVRC_WB;
    }
    if ((o & ::android::hardware::radio::V1_2::AudioQuality::EVRC_NW) == static_cast<int32_t>(::android::hardware::radio::V1_2::AudioQuality::EVRC_NW)) {
        os += (first ? "" : " | ");
        os += "EVRC_NW";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::AudioQuality::EVRC_NW;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_2::AudioQuality o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_2::AudioQuality::UNSPECIFIED) {
        return "UNSPECIFIED";
    }
    if (o == ::android::hardware::radio::V1_2::AudioQuality::AMR) {
        return "AMR";
    }
    if (o == ::android::hardware::radio::V1_2::AudioQuality::AMR_WB) {
        return "AMR_WB";
    }
    if (o == ::android::hardware::radio::V1_2::AudioQuality::GSM_EFR) {
        return "GSM_EFR";
    }
    if (o == ::android::hardware::radio::V1_2::AudioQuality::GSM_FR) {
        return "GSM_FR";
    }
    if (o == ::android::hardware::radio::V1_2::AudioQuality::GSM_HR) {
        return "GSM_HR";
    }
    if (o == ::android::hardware::radio::V1_2::AudioQuality::EVRC) {
        return "EVRC";
    }
    if (o == ::android::hardware::radio::V1_2::AudioQuality::EVRC_B) {
        return "EVRC_B";
    }
    if (o == ::android::hardware::radio::V1_2::AudioQuality::EVRC_WB) {
        return "EVRC_WB";
    }
    if (o == ::android::hardware::radio::V1_2::AudioQuality::EVRC_NW) {
        return "EVRC_NW";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::android::hardware::radio::V1_2::NetworkScanRequest& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".type = ";
    os += ::android::hardware::radio::V1_1::toString(o.type);
    os += ", .interval = ";
    os += ::android::hardware::toString(o.interval);
    os += ", .specifiers = ";
    os += ::android::hardware::toString(o.specifiers);
    os += ", .maxSearchTime = ";
    os += ::android::hardware::toString(o.maxSearchTime);
    os += ", .incrementalResults = ";
    os += ::android::hardware::toString(o.incrementalResults);
    os += ", .incrementalResultsPeriodicity = ";
    os += ::android::hardware::toString(o.incrementalResultsPeriodicity);
    os += ", .mccMncs = ";
    os += ::android::hardware::toString(o.mccMncs);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::NetworkScanRequest& lhs, const ::android::hardware::radio::V1_2::NetworkScanRequest& rhs) {
    if (lhs.type != rhs.type) {
        return false;
    }
    if (lhs.interval != rhs.interval) {
        return false;
    }
    if (lhs.specifiers != rhs.specifiers) {
        return false;
    }
    if (lhs.maxSearchTime != rhs.maxSearchTime) {
        return false;
    }
    if (lhs.incrementalResults != rhs.incrementalResults) {
        return false;
    }
    if (lhs.incrementalResultsPeriodicity != rhs.incrementalResultsPeriodicity) {
        return false;
    }
    if (lhs.mccMncs != rhs.mccMncs) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::NetworkScanRequest& lhs,const ::android::hardware::radio::V1_2::NetworkScanRequest& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::NetworkScanResult& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".status = ";
    os += ::android::hardware::radio::V1_1::toString(o.status);
    os += ", .error = ";
    os += ::android::hardware::radio::V1_0::toString(o.error);
    os += ", .networkInfos = ";
    os += ::android::hardware::toString(o.networkInfos);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::NetworkScanResult& lhs, const ::android::hardware::radio::V1_2::NetworkScanResult& rhs) {
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.error != rhs.error) {
        return false;
    }
    if (lhs.networkInfos != rhs.networkInfos) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::NetworkScanResult& lhs,const ::android::hardware::radio::V1_2::NetworkScanResult& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::CellIdentityOperatorNames& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".alphaLong = ";
    os += ::android::hardware::toString(o.alphaLong);
    os += ", .alphaShort = ";
    os += ::android::hardware::toString(o.alphaShort);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::CellIdentityOperatorNames& lhs, const ::android::hardware::radio::V1_2::CellIdentityOperatorNames& rhs) {
    if (lhs.alphaLong != rhs.alphaLong) {
        return false;
    }
    if (lhs.alphaShort != rhs.alphaShort) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::CellIdentityOperatorNames& lhs,const ::android::hardware::radio::V1_2::CellIdentityOperatorNames& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::CellIdentityCdma& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".base = ";
    os += ::android::hardware::radio::V1_0::toString(o.base);
    os += ", .operatorNames = ";
    os += ::android::hardware::radio::V1_2::toString(o.operatorNames);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::CellIdentityCdma& lhs, const ::android::hardware::radio::V1_2::CellIdentityCdma& rhs) {
    if (lhs.base != rhs.base) {
        return false;
    }
    if (lhs.operatorNames != rhs.operatorNames) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::CellIdentityCdma& lhs,const ::android::hardware::radio::V1_2::CellIdentityCdma& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::CellIdentityGsm& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".base = ";
    os += ::android::hardware::radio::V1_0::toString(o.base);
    os += ", .operatorNames = ";
    os += ::android::hardware::radio::V1_2::toString(o.operatorNames);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::CellIdentityGsm& lhs, const ::android::hardware::radio::V1_2::CellIdentityGsm& rhs) {
    if (lhs.base != rhs.base) {
        return false;
    }
    if (lhs.operatorNames != rhs.operatorNames) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::CellIdentityGsm& lhs,const ::android::hardware::radio::V1_2::CellIdentityGsm& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::CellIdentityLte& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".base = ";
    os += ::android::hardware::radio::V1_0::toString(o.base);
    os += ", .operatorNames = ";
    os += ::android::hardware::radio::V1_2::toString(o.operatorNames);
    os += ", .bandwidth = ";
    os += ::android::hardware::toString(o.bandwidth);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::CellIdentityLte& lhs, const ::android::hardware::radio::V1_2::CellIdentityLte& rhs) {
    if (lhs.base != rhs.base) {
        return false;
    }
    if (lhs.operatorNames != rhs.operatorNames) {
        return false;
    }
    if (lhs.bandwidth != rhs.bandwidth) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::CellIdentityLte& lhs,const ::android::hardware::radio::V1_2::CellIdentityLte& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::CellIdentityTdscdma& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".base = ";
    os += ::android::hardware::radio::V1_0::toString(o.base);
    os += ", .uarfcn = ";
    os += ::android::hardware::toString(o.uarfcn);
    os += ", .operatorNames = ";
    os += ::android::hardware::radio::V1_2::toString(o.operatorNames);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::CellIdentityTdscdma& lhs, const ::android::hardware::radio::V1_2::CellIdentityTdscdma& rhs) {
    if (lhs.base != rhs.base) {
        return false;
    }
    if (lhs.uarfcn != rhs.uarfcn) {
        return false;
    }
    if (lhs.operatorNames != rhs.operatorNames) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::CellIdentityTdscdma& lhs,const ::android::hardware::radio::V1_2::CellIdentityTdscdma& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::CellIdentityWcdma& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".base = ";
    os += ::android::hardware::radio::V1_0::toString(o.base);
    os += ", .operatorNames = ";
    os += ::android::hardware::radio::V1_2::toString(o.operatorNames);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::CellIdentityWcdma& lhs, const ::android::hardware::radio::V1_2::CellIdentityWcdma& rhs) {
    if (lhs.base != rhs.base) {
        return false;
    }
    if (lhs.operatorNames != rhs.operatorNames) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::CellIdentityWcdma& lhs,const ::android::hardware::radio::V1_2::CellIdentityWcdma& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::CellInfoGsm& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cellIdentityGsm = ";
    os += ::android::hardware::radio::V1_2::toString(o.cellIdentityGsm);
    os += ", .signalStrengthGsm = ";
    os += ::android::hardware::radio::V1_0::toString(o.signalStrengthGsm);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::CellInfoGsm& lhs, const ::android::hardware::radio::V1_2::CellInfoGsm& rhs) {
    if (lhs.cellIdentityGsm != rhs.cellIdentityGsm) {
        return false;
    }
    if (lhs.signalStrengthGsm != rhs.signalStrengthGsm) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::CellInfoGsm& lhs,const ::android::hardware::radio::V1_2::CellInfoGsm& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::WcdmaSignalStrength& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".base = ";
    os += ::android::hardware::radio::V1_0::toString(o.base);
    os += ", .rscp = ";
    os += ::android::hardware::toString(o.rscp);
    os += ", .ecno = ";
    os += ::android::hardware::toString(o.ecno);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::WcdmaSignalStrength& lhs, const ::android::hardware::radio::V1_2::WcdmaSignalStrength& rhs) {
    if (lhs.base != rhs.base) {
        return false;
    }
    if (lhs.rscp != rhs.rscp) {
        return false;
    }
    if (lhs.ecno != rhs.ecno) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::WcdmaSignalStrength& lhs,const ::android::hardware::radio::V1_2::WcdmaSignalStrength& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::CellInfoWcdma& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cellIdentityWcdma = ";
    os += ::android::hardware::radio::V1_2::toString(o.cellIdentityWcdma);
    os += ", .signalStrengthWcdma = ";
    os += ::android::hardware::radio::V1_2::toString(o.signalStrengthWcdma);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::CellInfoWcdma& lhs, const ::android::hardware::radio::V1_2::CellInfoWcdma& rhs) {
    if (lhs.cellIdentityWcdma != rhs.cellIdentityWcdma) {
        return false;
    }
    if (lhs.signalStrengthWcdma != rhs.signalStrengthWcdma) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::CellInfoWcdma& lhs,const ::android::hardware::radio::V1_2::CellInfoWcdma& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::CellInfoCdma& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cellIdentityCdma = ";
    os += ::android::hardware::radio::V1_2::toString(o.cellIdentityCdma);
    os += ", .signalStrengthCdma = ";
    os += ::android::hardware::radio::V1_0::toString(o.signalStrengthCdma);
    os += ", .signalStrengthEvdo = ";
    os += ::android::hardware::radio::V1_0::toString(o.signalStrengthEvdo);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::CellInfoCdma& lhs, const ::android::hardware::radio::V1_2::CellInfoCdma& rhs) {
    if (lhs.cellIdentityCdma != rhs.cellIdentityCdma) {
        return false;
    }
    if (lhs.signalStrengthCdma != rhs.signalStrengthCdma) {
        return false;
    }
    if (lhs.signalStrengthEvdo != rhs.signalStrengthEvdo) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::CellInfoCdma& lhs,const ::android::hardware::radio::V1_2::CellInfoCdma& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::CellInfoLte& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cellIdentityLte = ";
    os += ::android::hardware::radio::V1_2::toString(o.cellIdentityLte);
    os += ", .signalStrengthLte = ";
    os += ::android::hardware::radio::V1_0::toString(o.signalStrengthLte);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::CellInfoLte& lhs, const ::android::hardware::radio::V1_2::CellInfoLte& rhs) {
    if (lhs.cellIdentityLte != rhs.cellIdentityLte) {
        return false;
    }
    if (lhs.signalStrengthLte != rhs.signalStrengthLte) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::CellInfoLte& lhs,const ::android::hardware::radio::V1_2::CellInfoLte& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::TdscdmaSignalStrength& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".signalStrength = ";
    os += ::android::hardware::toString(o.signalStrength);
    os += ", .bitErrorRate = ";
    os += ::android::hardware::toString(o.bitErrorRate);
    os += ", .rscp = ";
    os += ::android::hardware::toString(o.rscp);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::TdscdmaSignalStrength& lhs, const ::android::hardware::radio::V1_2::TdscdmaSignalStrength& rhs) {
    if (lhs.signalStrength != rhs.signalStrength) {
        return false;
    }
    if (lhs.bitErrorRate != rhs.bitErrorRate) {
        return false;
    }
    if (lhs.rscp != rhs.rscp) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::TdscdmaSignalStrength& lhs,const ::android::hardware::radio::V1_2::TdscdmaSignalStrength& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::CellInfoTdscdma& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cellIdentityTdscdma = ";
    os += ::android::hardware::radio::V1_2::toString(o.cellIdentityTdscdma);
    os += ", .signalStrengthTdscdma = ";
    os += ::android::hardware::radio::V1_2::toString(o.signalStrengthTdscdma);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::CellInfoTdscdma& lhs, const ::android::hardware::radio::V1_2::CellInfoTdscdma& rhs) {
    if (lhs.cellIdentityTdscdma != rhs.cellIdentityTdscdma) {
        return false;
    }
    if (lhs.signalStrengthTdscdma != rhs.signalStrengthTdscdma) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::CellInfoTdscdma& lhs,const ::android::hardware::radio::V1_2::CellInfoTdscdma& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::CellInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cellInfoType = ";
    os += ::android::hardware::radio::V1_0::toString(o.cellInfoType);
    os += ", .registered = ";
    os += ::android::hardware::toString(o.registered);
    os += ", .timeStampType = ";
    os += ::android::hardware::radio::V1_0::toString(o.timeStampType);
    os += ", .timeStamp = ";
    os += ::android::hardware::toString(o.timeStamp);
    os += ", .gsm = ";
    os += ::android::hardware::toString(o.gsm);
    os += ", .cdma = ";
    os += ::android::hardware::toString(o.cdma);
    os += ", .lte = ";
    os += ::android::hardware::toString(o.lte);
    os += ", .wcdma = ";
    os += ::android::hardware::toString(o.wcdma);
    os += ", .tdscdma = ";
    os += ::android::hardware::toString(o.tdscdma);
    os += ", .connectionStatus = ";
    os += ::android::hardware::radio::V1_2::toString(o.connectionStatus);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::CellInfo& lhs, const ::android::hardware::radio::V1_2::CellInfo& rhs) {
    if (lhs.cellInfoType != rhs.cellInfoType) {
        return false;
    }
    if (lhs.registered != rhs.registered) {
        return false;
    }
    if (lhs.timeStampType != rhs.timeStampType) {
        return false;
    }
    if (lhs.timeStamp != rhs.timeStamp) {
        return false;
    }
    if (lhs.gsm != rhs.gsm) {
        return false;
    }
    if (lhs.cdma != rhs.cdma) {
        return false;
    }
    if (lhs.lte != rhs.lte) {
        return false;
    }
    if (lhs.wcdma != rhs.wcdma) {
        return false;
    }
    if (lhs.tdscdma != rhs.tdscdma) {
        return false;
    }
    if (lhs.connectionStatus != rhs.connectionStatus) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::CellInfo& lhs,const ::android::hardware::radio::V1_2::CellInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::CardStatus& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".base = ";
    os += ::android::hardware::radio::V1_0::toString(o.base);
    os += ", .physicalSlotId = ";
    os += ::android::hardware::toString(o.physicalSlotId);
    os += ", .atr = ";
    os += ::android::hardware::toString(o.atr);
    os += ", .iccid = ";
    os += ::android::hardware::toString(o.iccid);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::CardStatus& lhs, const ::android::hardware::radio::V1_2::CardStatus& rhs) {
    if (lhs.base != rhs.base) {
        return false;
    }
    if (lhs.physicalSlotId != rhs.physicalSlotId) {
        return false;
    }
    if (lhs.atr != rhs.atr) {
        return false;
    }
    if (lhs.iccid != rhs.iccid) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::CardStatus& lhs,const ::android::hardware::radio::V1_2::CardStatus& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::LinkCapacityEstimate& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".downlinkCapacityKbps = ";
    os += ::android::hardware::toString(o.downlinkCapacityKbps);
    os += ", .uplinkCapacityKbps = ";
    os += ::android::hardware::toString(o.uplinkCapacityKbps);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::LinkCapacityEstimate& lhs, const ::android::hardware::radio::V1_2::LinkCapacityEstimate& rhs) {
    if (lhs.downlinkCapacityKbps != rhs.downlinkCapacityKbps) {
        return false;
    }
    if (lhs.uplinkCapacityKbps != rhs.uplinkCapacityKbps) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::LinkCapacityEstimate& lhs,const ::android::hardware::radio::V1_2::LinkCapacityEstimate& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::PhysicalChannelConfig& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".status = ";
    os += ::android::hardware::radio::V1_2::toString(o.status);
    os += ", .cellBandwidthDownlink = ";
    os += ::android::hardware::toString(o.cellBandwidthDownlink);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::PhysicalChannelConfig& lhs, const ::android::hardware::radio::V1_2::PhysicalChannelConfig& rhs) {
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.cellBandwidthDownlink != rhs.cellBandwidthDownlink) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::PhysicalChannelConfig& lhs,const ::android::hardware::radio::V1_2::PhysicalChannelConfig& rhs){
    return !(lhs == rhs);
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::AccessNetwork lhs, const ::android::hardware::radio::V1_2::AccessNetwork rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_2::AccessNetwork rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::AccessNetwork lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::AccessNetwork lhs, const ::android::hardware::radio::V1_2::AccessNetwork rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_2::AccessNetwork rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::AccessNetwork lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_2::AccessNetwork e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_2::AccessNetwork e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_2::AccessNetwork>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_2::AccessNetwork> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_2::AccessNetwork::GERAN) == static_cast<int32_t>(::android::hardware::radio::V1_2::AccessNetwork::GERAN)) {
        os += (first ? "" : " | ");
        os += "GERAN";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::AccessNetwork::GERAN;
    }
    if ((o & ::android::hardware::radio::V1_2::AccessNetwork::UTRAN) == static_cast<int32_t>(::android::hardware::radio::V1_2::AccessNetwork::UTRAN)) {
        os += (first ? "" : " | ");
        os += "UTRAN";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::AccessNetwork::UTRAN;
    }
    if ((o & ::android::hardware::radio::V1_2::AccessNetwork::EUTRAN) == static_cast<int32_t>(::android::hardware::radio::V1_2::AccessNetwork::EUTRAN)) {
        os += (first ? "" : " | ");
        os += "EUTRAN";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::AccessNetwork::EUTRAN;
    }
    if ((o & ::android::hardware::radio::V1_2::AccessNetwork::CDMA2000) == static_cast<int32_t>(::android::hardware::radio::V1_2::AccessNetwork::CDMA2000)) {
        os += (first ? "" : " | ");
        os += "CDMA2000";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::AccessNetwork::CDMA2000;
    }
    if ((o & ::android::hardware::radio::V1_2::AccessNetwork::IWLAN) == static_cast<int32_t>(::android::hardware::radio::V1_2::AccessNetwork::IWLAN)) {
        os += (first ? "" : " | ");
        os += "IWLAN";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::AccessNetwork::IWLAN;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_2::AccessNetwork o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_2::AccessNetwork::GERAN) {
        return "GERAN";
    }
    if (o == ::android::hardware::radio::V1_2::AccessNetwork::UTRAN) {
        return "UTRAN";
    }
    if (o == ::android::hardware::radio::V1_2::AccessNetwork::EUTRAN) {
        return "EUTRAN";
    }
    if (o == ::android::hardware::radio::V1_2::AccessNetwork::CDMA2000) {
        return "CDMA2000";
    }
    if (o == ::android::hardware::radio::V1_2::AccessNetwork::IWLAN) {
        return "IWLAN";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::DataRequestReason lhs, const ::android::hardware::radio::V1_2::DataRequestReason rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_2::DataRequestReason rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_2::DataRequestReason lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::DataRequestReason lhs, const ::android::hardware::radio::V1_2::DataRequestReason rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_2::DataRequestReason rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_2::DataRequestReason lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_2::DataRequestReason e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_2::DataRequestReason e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_2::DataRequestReason>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_2::DataRequestReason> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_2::DataRequestReason::NORMAL) == static_cast<int32_t>(::android::hardware::radio::V1_2::DataRequestReason::NORMAL)) {
        os += (first ? "" : " | ");
        os += "NORMAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::DataRequestReason::NORMAL;
    }
    if ((o & ::android::hardware::radio::V1_2::DataRequestReason::SHUTDOWN) == static_cast<int32_t>(::android::hardware::radio::V1_2::DataRequestReason::SHUTDOWN)) {
        os += (first ? "" : " | ");
        os += "SHUTDOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::DataRequestReason::SHUTDOWN;
    }
    if ((o & ::android::hardware::radio::V1_2::DataRequestReason::HANDOVER) == static_cast<int32_t>(::android::hardware::radio::V1_2::DataRequestReason::HANDOVER)) {
        os += (first ? "" : " | ");
        os += "HANDOVER";
        first = false;
        flipped |= ::android::hardware::radio::V1_2::DataRequestReason::HANDOVER;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_2::DataRequestReason o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_2::DataRequestReason::NORMAL) {
        return "NORMAL";
    }
    if (o == ::android::hardware::radio::V1_2::DataRequestReason::SHUTDOWN) {
        return "SHUTDOWN";
    }
    if (o == ::android::hardware::radio::V1_2::DataRequestReason::HANDOVER) {
        return "HANDOVER";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::android::hardware::radio::V1_2::Call& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".base = ";
    os += ::android::hardware::radio::V1_0::toString(o.base);
    os += ", .audioQuality = ";
    os += ::android::hardware::radio::V1_2::toString(o.audioQuality);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::Call& lhs, const ::android::hardware::radio::V1_2::Call& rhs) {
    if (lhs.base != rhs.base) {
        return false;
    }
    if (lhs.audioQuality != rhs.audioQuality) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::Call& lhs,const ::android::hardware::radio::V1_2::Call& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::SignalStrength& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".gsm = ";
    os += ::android::hardware::radio::V1_0::toString(o.gsm);
    os += ", .cdma = ";
    os += ::android::hardware::radio::V1_0::toString(o.cdma);
    os += ", .evdo = ";
    os += ::android::hardware::radio::V1_0::toString(o.evdo);
    os += ", .lte = ";
    os += ::android::hardware::radio::V1_0::toString(o.lte);
    os += ", .tdScdma = ";
    os += ::android::hardware::radio::V1_0::toString(o.tdScdma);
    os += ", .wcdma = ";
    os += ::android::hardware::radio::V1_2::toString(o.wcdma);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::SignalStrength& lhs, const ::android::hardware::radio::V1_2::SignalStrength& rhs) {
    if (lhs.gsm != rhs.gsm) {
        return false;
    }
    if (lhs.cdma != rhs.cdma) {
        return false;
    }
    if (lhs.evdo != rhs.evdo) {
        return false;
    }
    if (lhs.lte != rhs.lte) {
        return false;
    }
    if (lhs.tdScdma != rhs.tdScdma) {
        return false;
    }
    if (lhs.wcdma != rhs.wcdma) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::SignalStrength& lhs,const ::android::hardware::radio::V1_2::SignalStrength& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::CellIdentity& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cellInfoType = ";
    os += ::android::hardware::radio::V1_0::toString(o.cellInfoType);
    os += ", .cellIdentityGsm = ";
    os += ::android::hardware::toString(o.cellIdentityGsm);
    os += ", .cellIdentityWcdma = ";
    os += ::android::hardware::toString(o.cellIdentityWcdma);
    os += ", .cellIdentityCdma = ";
    os += ::android::hardware::toString(o.cellIdentityCdma);
    os += ", .cellIdentityLte = ";
    os += ::android::hardware::toString(o.cellIdentityLte);
    os += ", .cellIdentityTdscdma = ";
    os += ::android::hardware::toString(o.cellIdentityTdscdma);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::CellIdentity& lhs, const ::android::hardware::radio::V1_2::CellIdentity& rhs) {
    if (lhs.cellInfoType != rhs.cellInfoType) {
        return false;
    }
    if (lhs.cellIdentityGsm != rhs.cellIdentityGsm) {
        return false;
    }
    if (lhs.cellIdentityWcdma != rhs.cellIdentityWcdma) {
        return false;
    }
    if (lhs.cellIdentityCdma != rhs.cellIdentityCdma) {
        return false;
    }
    if (lhs.cellIdentityLte != rhs.cellIdentityLte) {
        return false;
    }
    if (lhs.cellIdentityTdscdma != rhs.cellIdentityTdscdma) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::CellIdentity& lhs,const ::android::hardware::radio::V1_2::CellIdentity& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::VoiceRegStateResult& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".regState = ";
    os += ::android::hardware::radio::V1_0::toString(o.regState);
    os += ", .rat = ";
    os += ::android::hardware::toString(o.rat);
    os += ", .cssSupported = ";
    os += ::android::hardware::toString(o.cssSupported);
    os += ", .roamingIndicator = ";
    os += ::android::hardware::toString(o.roamingIndicator);
    os += ", .systemIsInPrl = ";
    os += ::android::hardware::toString(o.systemIsInPrl);
    os += ", .defaultRoamingIndicator = ";
    os += ::android::hardware::toString(o.defaultRoamingIndicator);
    os += ", .reasonForDenial = ";
    os += ::android::hardware::toString(o.reasonForDenial);
    os += ", .cellIdentity = ";
    os += ::android::hardware::radio::V1_2::toString(o.cellIdentity);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::VoiceRegStateResult& lhs, const ::android::hardware::radio::V1_2::VoiceRegStateResult& rhs) {
    if (lhs.regState != rhs.regState) {
        return false;
    }
    if (lhs.rat != rhs.rat) {
        return false;
    }
    if (lhs.cssSupported != rhs.cssSupported) {
        return false;
    }
    if (lhs.roamingIndicator != rhs.roamingIndicator) {
        return false;
    }
    if (lhs.systemIsInPrl != rhs.systemIsInPrl) {
        return false;
    }
    if (lhs.defaultRoamingIndicator != rhs.defaultRoamingIndicator) {
        return false;
    }
    if (lhs.reasonForDenial != rhs.reasonForDenial) {
        return false;
    }
    if (lhs.cellIdentity != rhs.cellIdentity) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::VoiceRegStateResult& lhs,const ::android::hardware::radio::V1_2::VoiceRegStateResult& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_2::DataRegStateResult& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".regState = ";
    os += ::android::hardware::radio::V1_0::toString(o.regState);
    os += ", .rat = ";
    os += ::android::hardware::toString(o.rat);
    os += ", .reasonDataDenied = ";
    os += ::android::hardware::toString(o.reasonDataDenied);
    os += ", .maxDataCalls = ";
    os += ::android::hardware::toString(o.maxDataCalls);
    os += ", .cellIdentity = ";
    os += ::android::hardware::radio::V1_2::toString(o.cellIdentity);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_2::DataRegStateResult& lhs, const ::android::hardware::radio::V1_2::DataRegStateResult& rhs) {
    if (lhs.regState != rhs.regState) {
        return false;
    }
    if (lhs.rat != rhs.rat) {
        return false;
    }
    if (lhs.reasonDataDenied != rhs.reasonDataDenied) {
        return false;
    }
    if (lhs.maxDataCalls != rhs.maxDataCalls) {
        return false;
    }
    if (lhs.cellIdentity != rhs.cellIdentity) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_2::DataRegStateResult& lhs,const ::android::hardware::radio::V1_2::DataRegStateResult& rhs){
    return !(lhs == rhs);
}


}  // namespace V1_2
}  // namespace radio
}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_2::RadioConst>
{
    const ::android::hardware::radio::V1_2::RadioConst* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_2::RadioConst* end() { return begin() + 14; }
    private:
    static const ::android::hardware::radio::V1_2::RadioConst* static_begin() {
        static const ::android::hardware::radio::V1_2::RadioConst kVals[14] {
            ::android::hardware::radio::V1_2::RadioConst::CDMA_ALPHA_INFO_BUFFER_LENGTH,
            ::android::hardware::radio::V1_2::RadioConst::CDMA_NUMBER_INFO_BUFFER_LENGTH,
            ::android::hardware::radio::V1_2::RadioConst::MAX_RILDS,
            ::android::hardware::radio::V1_2::RadioConst::MAX_SOCKET_NAME_LENGTH,
            ::android::hardware::radio::V1_2::RadioConst::MAX_CLIENT_ID_LENGTH,
            ::android::hardware::radio::V1_2::RadioConst::MAX_DEBUG_SOCKET_NAME_LENGTH,
            ::android::hardware::radio::V1_2::RadioConst::MAX_QEMU_PIPE_NAME_LENGTH,
            ::android::hardware::radio::V1_2::RadioConst::MAX_UUID_LENGTH,
            ::android::hardware::radio::V1_2::RadioConst::CARD_MAX_APPS,
            ::android::hardware::radio::V1_2::RadioConst::CDMA_MAX_NUMBER_OF_INFO_RECS,
            ::android::hardware::radio::V1_2::RadioConst::SS_INFO_MAX,
            ::android::hardware::radio::V1_2::RadioConst::NUM_SERVICE_CLASSES,
            ::android::hardware::radio::V1_2::RadioConst::NUM_TX_POWER_LEVELS,
            ::android::hardware::radio::V1_2::RadioConst::RADIO_ACCESS_SPECIFIER_MAX_SIZE,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_2::ScanIntervalRange>
{
    const ::android::hardware::radio::V1_2::ScanIntervalRange* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_2::ScanIntervalRange* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_2::ScanIntervalRange* static_begin() {
        static const ::android::hardware::radio::V1_2::ScanIntervalRange kVals[2] {
            ::android::hardware::radio::V1_2::ScanIntervalRange::MIN,
            ::android::hardware::radio::V1_2::ScanIntervalRange::MAX,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_2::MaxSearchTimeRange>
{
    const ::android::hardware::radio::V1_2::MaxSearchTimeRange* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_2::MaxSearchTimeRange* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_2::MaxSearchTimeRange* static_begin() {
        static const ::android::hardware::radio::V1_2::MaxSearchTimeRange kVals[2] {
            ::android::hardware::radio::V1_2::MaxSearchTimeRange::MIN,
            ::android::hardware::radio::V1_2::MaxSearchTimeRange::MAX,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange>
{
    const ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange* static_begin() {
        static const ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange kVals[2] {
            ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange::MIN,
            ::android::hardware::radio::V1_2::IncrementalResultsPeriodicityRange::MAX,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_2::CellConnectionStatus>
{
    const ::android::hardware::radio::V1_2::CellConnectionStatus* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_2::CellConnectionStatus* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_2::CellConnectionStatus* static_begin() {
        static const ::android::hardware::radio::V1_2::CellConnectionStatus kVals[3] {
            ::android::hardware::radio::V1_2::CellConnectionStatus::NONE,
            ::android::hardware::radio::V1_2::CellConnectionStatus::PRIMARY_SERVING,
            ::android::hardware::radio::V1_2::CellConnectionStatus::SECONDARY_SERVING,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_2::IndicationFilter>
{
    const ::android::hardware::radio::V1_2::IndicationFilter* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_2::IndicationFilter* end() { return begin() + 7; }
    private:
    static const ::android::hardware::radio::V1_2::IndicationFilter* static_begin() {
        static const ::android::hardware::radio::V1_2::IndicationFilter kVals[7] {
            ::android::hardware::radio::V1_2::IndicationFilter::NONE,
            ::android::hardware::radio::V1_2::IndicationFilter::ALL,
            ::android::hardware::radio::V1_2::IndicationFilter::SIGNAL_STRENGTH,
            ::android::hardware::radio::V1_2::IndicationFilter::FULL_NETWORK_STATE,
            ::android::hardware::radio::V1_2::IndicationFilter::DATA_CALL_DORMANCY_CHANGED,
            ::android::hardware::radio::V1_2::IndicationFilter::LINK_CAPACITY_ESTIMATE,
            ::android::hardware::radio::V1_2::IndicationFilter::PHYSICAL_CHANNEL_CONFIG,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_2::AudioQuality>
{
    const ::android::hardware::radio::V1_2::AudioQuality* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_2::AudioQuality* end() { return begin() + 10; }
    private:
    static const ::android::hardware::radio::V1_2::AudioQuality* static_begin() {
        static const ::android::hardware::radio::V1_2::AudioQuality kVals[10] {
            ::android::hardware::radio::V1_2::AudioQuality::UNSPECIFIED,
            ::android::hardware::radio::V1_2::AudioQuality::AMR,
            ::android::hardware::radio::V1_2::AudioQuality::AMR_WB,
            ::android::hardware::radio::V1_2::AudioQuality::GSM_EFR,
            ::android::hardware::radio::V1_2::AudioQuality::GSM_FR,
            ::android::hardware::radio::V1_2::AudioQuality::GSM_HR,
            ::android::hardware::radio::V1_2::AudioQuality::EVRC,
            ::android::hardware::radio::V1_2::AudioQuality::EVRC_B,
            ::android::hardware::radio::V1_2::AudioQuality::EVRC_WB,
            ::android::hardware::radio::V1_2::AudioQuality::EVRC_NW,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_2::AccessNetwork>
{
    const ::android::hardware::radio::V1_2::AccessNetwork* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_2::AccessNetwork* end() { return begin() + 5; }
    private:
    static const ::android::hardware::radio::V1_2::AccessNetwork* static_begin() {
        static const ::android::hardware::radio::V1_2::AccessNetwork kVals[5] {
            ::android::hardware::radio::V1_2::AccessNetwork::GERAN,
            ::android::hardware::radio::V1_2::AccessNetwork::UTRAN,
            ::android::hardware::radio::V1_2::AccessNetwork::EUTRAN,
            ::android::hardware::radio::V1_2::AccessNetwork::CDMA2000,
            ::android::hardware::radio::V1_2::AccessNetwork::IWLAN,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_2::DataRequestReason>
{
    const ::android::hardware::radio::V1_2::DataRequestReason* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_2::DataRequestReason* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_2::DataRequestReason* static_begin() {
        static const ::android::hardware::radio::V1_2::DataRequestReason kVals[3] {
            ::android::hardware::radio::V1_2::DataRequestReason::NORMAL,
            ::android::hardware::radio::V1_2::DataRequestReason::SHUTDOWN,
            ::android::hardware::radio::V1_2::DataRequestReason::HANDOVER,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HARDWARE_RADIO_V1_2_TYPES_H
