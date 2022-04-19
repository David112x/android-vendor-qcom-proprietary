#ifndef HIDL_GENERATED_ANDROID_HARDWARE_RADIO_V1_4_TYPES_H
#define HIDL_GENERATED_ANDROID_HARDWARE_RADIO_V1_4_TYPES_H

#include <android/hardware/radio/1.0/types.h>
#include <android/hardware/radio/1.1/types.h>
#include <android/hardware/radio/1.2/types.h>
#include <android/hidl/safe_union/1.0/types.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hardware {
namespace radio {
namespace V1_4 {

// Forward declaration for forward reference support:
enum class AccessNetwork : int32_t;
enum class ApnTypes : int32_t;
enum class EmergencyServiceCategory : int32_t;
enum class EmergencyNumberSource : int32_t;
struct EmergencyNumber;
enum class EmergencyCallRouting : int32_t;
enum class RadioTechnology : int32_t;
enum class RadioAccessFamily : int32_t;
enum class FrequencyRange : int32_t;
enum class DataCallFailCause : int32_t;
enum class DataConnActiveStatus : int32_t;
enum class PdpProtocolType : int32_t;
struct RadioFrequencyInfo;
struct PhysicalChannelConfig;
struct LteVopsInfo;
struct NrIndicators;
struct DataRegStateResult;
struct CellConfigLte;
struct CellInfoLte;
struct NrSignalStrength;
struct CellIdentityNr;
struct CellInfoNr;
struct CellInfo;
struct NetworkScanResult;
struct DataProfileInfo;
struct CardStatus;
struct RadioCapability;
struct SetupDataCallResult;
enum class SimLockMultiSimPolicy : int32_t;
struct CarrierRestrictionsWithPriority;
struct SignalStrength;

// Order of inner types was changed for forward reference support.

enum class AccessNetwork : int32_t {
    /**
     * GSM EDGE Radio Access Network
     */
    GERAN = 1,
    /**
     * Universal Terrestrial Radio Access Network
     */
    UTRAN = 2,
    /**
     * Evolved Universal Terrestrial Radio Access Network
     */
    EUTRAN = 3,
    /**
     * CDMA 2000 network
     */
    CDMA2000 = 4,
    /**
     * Interworking Wireless LAN
     */
    IWLAN = 5,
    /**
     * Unknown access network
     */
    UNKNOWN = 0,
};

enum class ApnTypes : int32_t {
    NONE = 0,
    DEFAULT = 1 /* (1 << 0) */,
    MMS = 2 /* (1 << 1) */,
    SUPL = 4 /* (1 << 2) */,
    DUN = 8 /* (1 << 3) */,
    HIPRI = 16 /* (1 << 4) */,
    FOTA = 32 /* (1 << 5) */,
    IMS = 64 /* (1 << 6) */,
    CBS = 128 /* (1 << 7) */,
    IA = 256 /* (1 << 8) */,
    EMERGENCY = 512 /* (1 << 9) */,
    ALL = 1023 /* (((((((((DEFAULT | MMS) | SUPL) | DUN) | HIPRI) | FOTA) | IMS) | CBS) | IA) | EMERGENCY) */,
    /**
     * Due to the addition of this new value, the value ALL defined in 1.0::ApnTypes is now
     * deprecated and should not be used.
     */
    MCX = 1024 /* (1 << 10) */,
};

/**
 * Defining Emergency Service Category as follows:
 * - General emergency call, all categories;
 * - Police;
 * - Ambulance;
 * - Fire Brigade;
 * - Marine Guard;
 * - Mountain Rescue;
 * - Manually Initiated eCall (MIeC);
 * - Automatically Initiated eCall (AIeC);
 * 
 * Category UNSPECIFIED (General emergency call, all categories) indicates that no specific
 * services are associated with this emergency number.
 * 
 * Reference: 3gpp 22.101, Section 10 - Emergency Calls
 */
enum class EmergencyServiceCategory : int32_t {
    /**
     * General emergency call, all categories
     */
    UNSPECIFIED = 0,
    POLICE = 1 /* (1 << 0) */,
    AMBULANCE = 2 /* (1 << 1) */,
    FIRE_BRIGADE = 4 /* (1 << 2) */,
    MARINE_GUARD = 8 /* (1 << 3) */,
    MOUNTAIN_RESCUE = 16 /* (1 << 4) */,
    /**
     * Manually Initiated eCall (MIeC)
     */
    MIEC = 32 /* (1 << 5) */,
    /**
     * Automatically Initiated eCall (AIeC)
     */
    AIEC = 64 /* (1 << 6) */,
};

/**
 * The source to tell where the corresponding @1.4::EmergencyNumber comes from.
 * 
 * Reference: 3gpp 22.101, Section 10 - Emergency Calls
 */
enum class EmergencyNumberSource : int32_t {
    /**
     * Indicates the number is from the network signal.
     */
    NETWORK_SIGNALING = 1 /* (1 << 0) */,
    /**
     * Indicates the number is from the sim card.
     */
    SIM = 2 /* (1 << 1) */,
    /**
     * Indicates the number is from the modem config.
     */
    MODEM_CONFIG = 4 /* (1 << 2) */,
    /**
     * Indicates the number is available as default. Per the reference, 112, 911 must always be
     * available; additionally, 000, 08, 110, 999, 118 and 119 must be available when sim is not
     * present.
     */
    DEFAULT = 8 /* (1 << 3) */,
};

/**
 * Emergency number contains information of number, one or more service category(s), zero or more
 * emergency uniform resource names, mobile country code (mcc), mobile network country (mnc) and
 * source(s) that indicate where it comes from.
 * 
 * If the emergency number is associated with country, field ‘mcc’ must be provided, otherwise
 * field ‘mcc’ must be an empty string. If the emergency number is associated with network
 * operator, field ‘mcc’ and 'mnc' must be provided, otherwise field ‘mnc’ must be an empty
 * string. If the emergency number is specified with emergency service category(s), field
 * 'categories' must be provided, otherwise field 'categories' must be
 * @1.4::EmergencyServiceCategories::UNSPECIFIED. If the emergency number is specified with
 * emergency uniform resource names (URN), field 'urns' must be provided, otherwise field 'urns'
 * must be an empty list.
 * 
 * A unique EmergencyNumber has a unique combination of ‘number’, ‘mcc’, 'mnc', 'categories' and
 * 'urns' fields. Multiple @1.4::EmergencyNumberSource should be merged into one 'sources' field
 * via bitwise-OR combination for the same EmergencyNumber.
 * 
 * Reference: 3gpp 22.101, Section 10 - Emergency Calls;
 *            3gpp 23.167, Section 6 - Functional description;
 *            3gpp 24.503, Section 5.1.6.8.1 - General;
 *            RFC 5031
 */
struct EmergencyNumber final {
    ::android::hardware::hidl_string number __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string mcc __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string mnc __attribute__ ((aligned(8)));
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::EmergencyServiceCategory> categories __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::hidl_string> urns __attribute__ ((aligned(8)));
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::EmergencyNumberSource> sources __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::EmergencyNumber, number) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::EmergencyNumber, mcc) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::EmergencyNumber, mnc) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::EmergencyNumber, categories) == 48, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::EmergencyNumber, urns) == 56, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::EmergencyNumber, sources) == 72, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::EmergencyNumber) == 80, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::EmergencyNumber) == 8, "wrong alignment");

/**
 * Indicates how the implementation should handle the emergency call if it is required by Android.
 */
enum class EmergencyCallRouting : int32_t {
    /**
     * Indicates Android does not require how to handle the corresponding emergency call; it is
     * decided by implementation.
     */
    UNKNOWN = 0,
    /**
     * Indicates the implementation must handle the call through emergency routing.
     */
    EMERGENCY = 1,
    /**
     * Indicates the implementation must handle the call through normal call routing.
     */
    NORMAL = 2,
};

enum class RadioTechnology : int32_t {
    UNKNOWN = 0,
    GPRS = 1,
    EDGE = 2,
    UMTS = 3,
    IS95A = 4,
    IS95B = 5,
    ONE_X_RTT = 6,
    EVDO_0 = 7,
    EVDO_A = 8,
    HSDPA = 9,
    HSUPA = 10,
    HSPA = 11,
    EVDO_B = 12,
    EHRPD = 13,
    LTE = 14,
    HSPAP = 15,
    GSM = 16,
    TD_SCDMA = 17,
    IWLAN = 18,
    LTE_CA = 19,
    /**
     * 5G NR. This is only use in 5G Standalone mode.
     */
    NR = 20,
};

enum class RadioAccessFamily : int32_t {
    UNKNOWN = 1 /* (1 << RadioTechnology:UNKNOWN) */,
    GPRS = 2 /* (1 << RadioTechnology:GPRS) */,
    EDGE = 4 /* (1 << RadioTechnology:EDGE) */,
    UMTS = 8 /* (1 << RadioTechnology:UMTS) */,
    IS95A = 16 /* (1 << RadioTechnology:IS95A) */,
    IS95B = 32 /* (1 << RadioTechnology:IS95B) */,
    ONE_X_RTT = 64 /* (1 << RadioTechnology:ONE_X_RTT) */,
    EVDO_0 = 128 /* (1 << RadioTechnology:EVDO_0) */,
    EVDO_A = 256 /* (1 << RadioTechnology:EVDO_A) */,
    HSDPA = 512 /* (1 << RadioTechnology:HSDPA) */,
    HSUPA = 1024 /* (1 << RadioTechnology:HSUPA) */,
    HSPA = 2048 /* (1 << RadioTechnology:HSPA) */,
    EVDO_B = 4096 /* (1 << RadioTechnology:EVDO_B) */,
    EHRPD = 8192 /* (1 << RadioTechnology:EHRPD) */,
    LTE = 16384 /* (1 << RadioTechnology:LTE) */,
    HSPAP = 32768 /* (1 << RadioTechnology:HSPAP) */,
    GSM = 65536 /* (1 << RadioTechnology:GSM) */,
    TD_SCDMA = 131072 /* (1 << RadioTechnology:TD_SCDMA) */,
    LTE_CA = 524288 /* (1 << RadioTechnology:LTE_CA) */,
    /**
     * 5G NR. This is only use in 5G Standalone mode.
     */
    NR = 1048576 /* (1 << RadioTechnology:NR) */,
};

/**
 * Mapping the frequency to a rough range.
 */
enum class FrequencyRange : int32_t {
    /**
     * Indicates the frequency range is below 1GHz.
     */
    LOW = 1,
    /**
     * Indicates the frequency range is between 1GHz and 3GHz.
     */
    MID = 2,
    /**
     * Indicates the frequency range is between 3GHz and 6GHz.
     */
    HIGH = 3,
    /**
     * Indicates the frequency range is above 6GHz (millimeter wave frequency).
     */
    MMWAVE = 4,
};

/**
 * Expose more setup data call failures.
 */
enum class DataCallFailCause : int32_t {
    NONE = 0,
    OPERATOR_BARRED = 8 /* 0x08 */,
    NAS_SIGNALLING = 14 /* 0x0E */,
    INSUFFICIENT_RESOURCES = 26 /* 0x1A */,
    MISSING_UKNOWN_APN = 27 /* 0x1B */,
    UNKNOWN_PDP_ADDRESS_TYPE = 28 /* 0x1C */,
    USER_AUTHENTICATION = 29 /* 0x1D */,
    ACTIVATION_REJECT_GGSN = 30 /* 0x1E */,
    ACTIVATION_REJECT_UNSPECIFIED = 31 /* 0x1F */,
    SERVICE_OPTION_NOT_SUPPORTED = 32 /* 0x20 */,
    SERVICE_OPTION_NOT_SUBSCRIBED = 33 /* 0x21 */,
    SERVICE_OPTION_OUT_OF_ORDER = 34 /* 0x22 */,
    NSAPI_IN_USE = 35 /* 0x23 */,
    REGULAR_DEACTIVATION = 36 /* 0x24 */,
    QOS_NOT_ACCEPTED = 37 /* 0x25 */,
    NETWORK_FAILURE = 38 /* 0x26 */,
    UMTS_REACTIVATION_REQ = 39 /* 0x27 */,
    FEATURE_NOT_SUPP = 40 /* 0x28 */,
    TFT_SEMANTIC_ERROR = 41 /* 0x29 */,
    TFT_SYTAX_ERROR = 42 /* 0x2A */,
    UNKNOWN_PDP_CONTEXT = 43 /* 0x2B */,
    FILTER_SEMANTIC_ERROR = 44 /* 0x2C */,
    FILTER_SYTAX_ERROR = 45 /* 0x2D */,
    PDP_WITHOUT_ACTIVE_TFT = 46 /* 0x2E */,
    ONLY_IPV4_ALLOWED = 50 /* 0x32 */,
    ONLY_IPV6_ALLOWED = 51 /* 0x33 */,
    ONLY_SINGLE_BEARER_ALLOWED = 52 /* 0x34 */,
    ESM_INFO_NOT_RECEIVED = 53 /* 0x35 */,
    PDN_CONN_DOES_NOT_EXIST = 54 /* 0x36 */,
    MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED = 55 /* 0x37 */,
    MAX_ACTIVE_PDP_CONTEXT_REACHED = 65 /* 0x41 */,
    UNSUPPORTED_APN_IN_CURRENT_PLMN = 66 /* 0x42 */,
    INVALID_TRANSACTION_ID = 81 /* 0x51 */,
    MESSAGE_INCORRECT_SEMANTIC = 95 /* 0x5F */,
    INVALID_MANDATORY_INFO = 96 /* 0x60 */,
    MESSAGE_TYPE_UNSUPPORTED = 97 /* 0x61 */,
    MSG_TYPE_NONCOMPATIBLE_STATE = 98 /* 0x62 */,
    UNKNOWN_INFO_ELEMENT = 99 /* 0x63 */,
    CONDITIONAL_IE_ERROR = 100 /* 0x64 */,
    MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE = 101 /* 0x65 */,
    PROTOCOL_ERRORS = 111 /* 0x6F */,
    APN_TYPE_CONFLICT = 112 /* 0x70 */,
    INVALID_PCSCF_ADDR = 113 /* 0x71 */,
    INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN = 114 /* 0x72 */,
    EMM_ACCESS_BARRED = 115 /* 0x73 */,
    EMERGENCY_IFACE_ONLY = 116 /* 0x74 */,
    IFACE_MISMATCH = 117 /* 0x75 */,
    COMPANION_IFACE_IN_USE = 118 /* 0x76 */,
    IP_ADDRESS_MISMATCH = 119 /* 0x77 */,
    IFACE_AND_POL_FAMILY_MISMATCH = 120 /* 0x78 */,
    EMM_ACCESS_BARRED_INFINITE_RETRY = 121 /* 0x79 */,
    AUTH_FAILURE_ON_EMERGENCY_CALL = 122 /* 0x7A */,
    OEM_DCFAILCAUSE_1 = 4097 /* 0x1001 */,
    OEM_DCFAILCAUSE_2 = 4098 /* 0x1002 */,
    OEM_DCFAILCAUSE_3 = 4099 /* 0x1003 */,
    OEM_DCFAILCAUSE_4 = 4100 /* 0x1004 */,
    OEM_DCFAILCAUSE_5 = 4101 /* 0x1005 */,
    OEM_DCFAILCAUSE_6 = 4102 /* 0x1006 */,
    OEM_DCFAILCAUSE_7 = 4103 /* 0x1007 */,
    OEM_DCFAILCAUSE_8 = 4104 /* 0x1008 */,
    OEM_DCFAILCAUSE_9 = 4105 /* 0x1009 */,
    OEM_DCFAILCAUSE_10 = 4106 /* 0x100A */,
    OEM_DCFAILCAUSE_11 = 4107 /* 0x100B */,
    OEM_DCFAILCAUSE_12 = 4108 /* 0x100C */,
    OEM_DCFAILCAUSE_13 = 4109 /* 0x100D */,
    OEM_DCFAILCAUSE_14 = 4110 /* 0x100E */,
    OEM_DCFAILCAUSE_15 = 4111 /* 0x100F */,
    VOICE_REGISTRATION_FAIL = -1 /* (-1) */,
    DATA_REGISTRATION_FAIL = -2 /* (-2) */,
    SIGNAL_LOST = -3 /* (-3) */,
    PREF_RADIO_TECH_CHANGED = -4 /* (-4) */,
    RADIO_POWER_OFF = -5 /* (-5) */,
    TETHERED_CALL_ACTIVE = -6 /* (-6) */,
    ERROR_UNSPECIFIED = 65535 /* 0xffff */,
    /**
     * Network cannot provide the requested service and PDP context is deactivated because of LLC
     * or SNDCP failure.
     */
    LLC_SNDCP = 25 /* 0x19 */,
    /**
     * UE requested to modify QoS parameters or the bearer control mode, which is not compatible
     * with the selected bearer control mode.
     */
    ACTIVATION_REJECTED_BCM_VIOLATION = 48 /* 0x30 */,
    /**
     * Network has already initiated the activation, modification, or deactivation of bearer
     * resources that was requested by the UE.
     */
    COLLISION_WITH_NETWORK_INITIATED_REQUEST = 56 /* 0x38 */,
    /**
     * Network supports IPv4v6 PDP type only. Non-IP type is not allowed. In LTE mode of operation,
     * this is a PDN throttling cause code, meaning the UE may throttle further requests to the
     * same APN.
     */
    ONLY_IPV4V6_ALLOWED = 57 /* 0x39 */,
    /**
     * Network supports non-IP PDP type only. IPv4, IPv6 and IPv4v6 is not allowed. In LTE mode of
     * operation, this is a PDN throttling cause code, meaning the UE can throttle further requests
     * to the same APN.
     */
    ONLY_NON_IP_ALLOWED = 58 /* 0x3A */,
    /**
     * QCI (QoS Class Identifier) indicated in the UE request cannot be supported.
     */
    UNSUPPORTED_QCI_VALUE = 59 /* 0x3B */,
    /**
     * Procedure requested by the UE was rejected because the bearer handling is not supported.
     */
    BEARER_HANDLING_NOT_SUPPORTED = 60 /* 0x3C */,
    /**
     * Not receiving a DNS address that was mandatory.
     */
    INVALID_DNS_ADDR = 123 /* 0x7B */,
    /**
     * Not receiving either a PCSCF or a DNS address, one of them being mandatory.
     */
    INVALID_PCSCF_OR_DNS_ADDRESS = 124 /* 0x7C */,
    /**
     * Emergency call bring up on a different ePDG.
     */
    CALL_PREEMPT_BY_EMERGENCY_APN = 127 /* 0x7F */,
    /**
     * UE performs a detach or disconnect PDN action based on TE requirements.
     */
    UE_INITIATED_DETACH_OR_DISCONNECT = 128 /* 0x80 */,
    /**
     * Reason unspecified for foreign agent rejected MIP (Mobile IP) registration.
     */
    MIP_FA_REASON_UNSPECIFIED = 2000 /* 0x7D0 */,
    /**
     * Foreign agent administratively prohibited MIP (Mobile IP) registration.
     */
    MIP_FA_ADMIN_PROHIBITED = 2001 /* 0x7D1 */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration because of insufficient resources.
     */
    MIP_FA_INSUFFICIENT_RESOURCES = 2002 /* 0x7D2 */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration because of MN-AAA authenticator was
     * wrong.
     */
    MIP_FA_MOBILE_NODE_AUTHENTICATION_FAILURE = 2003 /* 0x7D3 */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration because of home agent authentication
     * failure.
     */
    MIP_FA_HOME_AGENT_AUTHENTICATION_FAILURE = 2004 /* 0x7D4 */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration because of requested lifetime was too
     * long.
     */
    MIP_FA_REQUESTED_LIFETIME_TOO_LONG = 2005 /* 0x7D5 */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration because of malformed request.
     */
    MIP_FA_MALFORMED_REQUEST = 2006 /* 0x7D6 */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration because of malformed reply.
     */
    MIP_FA_MALFORMED_REPLY = 2007 /* 0x7D7 */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration because of requested encapsulation was
     * unavailable.
     */
    MIP_FA_ENCAPSULATION_UNAVAILABLE = 2008 /* 0x7D8 */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration of VJ Header Compression was unavailable.
     */
    MIP_FA_VJ_HEADER_COMPRESSION_UNAVAILABLE = 2009 /* 0x7D9 */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration because of reverse tunnel was
     * unavailable.
     */
    MIP_FA_REVERSE_TUNNEL_UNAVAILABLE = 2010 /* 0x7DA */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration because of reverse tunnel was mandatory
     * but not requested by device.
     */
    MIP_FA_REVERSE_TUNNEL_IS_MANDATORY = 2011 /* 0x7DB */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration because of delivery style was not
     * supported.
     */
    MIP_FA_DELIVERY_STYLE_NOT_SUPPORTED = 2012 /* 0x7DC */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration because of missing NAI (Network Access
     * Identifier).
     */
    MIP_FA_MISSING_NAI = 2013 /* 0x7DD */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration because of missing Home Agent.
     */
    MIP_FA_MISSING_HOME_AGENT = 2014 /* 0x7DE */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration because of missing Home Address.
     */
    MIP_FA_MISSING_HOME_ADDRESS = 2015 /* 0x7DF */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration because of unknown challenge.
     */
    MIP_FA_UNKNOWN_CHALLENGE = 2016 /* 0x7E0 */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration because of missing challenge.
     */
    MIP_FA_MISSING_CHALLENGE = 2017 /* 0x7E1 */,
    /**
     * Foreign agent rejected MIP (Mobile IP) registration because of stale challenge.
     */
    MIP_FA_STALE_CHALLENGE = 2018 /* 0x7E2 */,
    /**
     * Reason unspecified for home agent rejected MIP (Mobile IP) registration.
     */
    MIP_HA_REASON_UNSPECIFIED = 2019 /* 0x7E3 */,
    /**
     * Home agent administratively prohibited MIP (Mobile IP) registration.
     */
    MIP_HA_ADMIN_PROHIBITED = 2020 /* 0x7E4 */,
    /**
     * Home agent rejected MIP (Mobile IP) registration because of insufficient resources.
     */
    MIP_HA_INSUFFICIENT_RESOURCES = 2021 /* 0x7E5 */,
    /**
     * Home agent rejected MIP (Mobile IP) registration because of MN-HA authenticator was wrong.
     */
    MIP_HA_MOBILE_NODE_AUTHENTICATION_FAILURE = 2022 /* 0x7E6 */,
    /**
     * Home agent rejected MIP (Mobile IP) registration because of foreign agent authentication
     * failure.
     */
    MIP_HA_FOREIGN_AGENT_AUTHENTICATION_FAILURE = 2023 /* 0x7E7 */,
    /**
     * Home agent rejected MIP (Mobile IP) registration because of registration id mismatch.
     */
    MIP_HA_REGISTRATION_ID_MISMATCH = 2024 /* 0x7E8 */,
    /**
     * Home agent rejected MIP (Mobile IP) registration because of malformed request.
     */
    MIP_HA_MALFORMED_REQUEST = 2025 /* 0x7E9 */,
    /**
     * Home agent rejected MIP (Mobile IP) registration because of unknown home agent address.
     */
    MIP_HA_UNKNOWN_HOME_AGENT_ADDRESS = 2026 /* 0x7EA */,
    /**
     * Home agent rejected MIP (Mobile IP) registration because of reverse tunnel was unavailable.
     */
    MIP_HA_REVERSE_TUNNEL_UNAVAILABLE = 2027 /* 0x7EB */,
    /**
     * Home agent rejected MIP (Mobile IP) registration because of reverse tunnel is mandatory but
     * not requested by device.
     */
    MIP_HA_REVERSE_TUNNEL_IS_MANDATORY = 2028 /* 0x7EC */,
    /**
     * Home agent rejected MIP (Mobile IP) registration because of encapsulation unavailable.
     */
    MIP_HA_ENCAPSULATION_UNAVAILABLE = 2029 /* 0x7ED */,
    /**
     * Tearing down is in progress.
     */
    CLOSE_IN_PROGRESS = 2030 /* 0x7EE */,
    /**
     * Brought down by the network.
     */
    NETWORK_INITIATED_TERMINATION = 2031 /* 0x7EF */,
    /**
     * Another application in modem preempts the data call.
     */
    MODEM_APP_PREEMPTED = 2032 /* 0x7F0 */,
    /**
     * IPV4 PDN is in throttled state due to network providing only IPV6 address during the previous
     * VSNCP bringup (subs_limited_to_v6).
     */
    PDN_IPV4_CALL_DISALLOWED = 2033 /* 0x7F1 */,
    /**
     * IPV4 PDN is in throttled state due to previous VSNCP bringup failure(s).
     */
    PDN_IPV4_CALL_THROTTLED = 2034 /* 0x7F2 */,
    /**
     * IPV6 PDN is in throttled state due to network providing only IPV4 address during the previous
     * VSNCP bringup (subs_limited_to_v4).
     */
    PDN_IPV6_CALL_DISALLOWED = 2035 /* 0x7F3 */,
    /**
     * IPV6 PDN is in throttled state due to previous VSNCP bringup failure(s).
     */
    PDN_IPV6_CALL_THROTTLED = 2036 /* 0x7F4 */,
    /**
     * Modem restart.
     */
    MODEM_RESTART = 2037 /* 0x7F5 */,
    /**
     * PDP PPP calls are not supported.
     */
    PDP_PPP_NOT_SUPPORTED = 2038 /* 0x7F6 */,
    /**
     * RAT on which the data call is attempted/connected is no longer the preferred RAT.
     */
    UNPREFERRED_RAT = 2039 /* 0x7F7 */,
    /**
     * Physical link is in the process of cleanup.
     */
    PHYSICAL_LINK_CLOSE_IN_PROGRESS = 2040 /* 0x7F8 */,
    /**
     * Interface bring up is attempted for an APN that is yet to be handed over to target RAT.
     */
    APN_PENDING_HANDOVER = 2041 /* 0x7F9 */,
    /**
     * APN bearer type in the profile does not match preferred network mode.
     */
    PROFILE_BEARER_INCOMPATIBLE = 2042 /* 0x7FA */,
    /**
     * Card was refreshed or removed.
     */
    SIM_CARD_CHANGED = 2043 /* 0x7FB */,
    /**
     * Device is going into lower power mode or powering down.
     */
    LOW_POWER_MODE_OR_POWERING_DOWN = 2044 /* 0x7FC */,
    /**
     * APN has been disabled.
     */
    APN_DISABLED = 2045 /* 0x7FD */,
    /**
     * Maximum PPP inactivity timer expired.
     */
    MAX_PPP_INACTIVITY_TIMER_EXPIRED = 2046 /* 0x7FE */,
    /**
     * IPv6 address transfer failed.
     */
    IPV6_ADDRESS_TRANSFER_FAILED = 2047 /* 0x7FF */,
    /**
     * Target RAT swap failed.
     */
    TRAT_SWAP_FAILED = 2048 /* 0x800 */,
    /**
     * Device falls back from eHRPD to HRPD.
     */
    EHRPD_TO_HRPD_FALLBACK = 2049 /* 0x801 */,
    /**
     * UE is in MIP-only configuration but the MIP configuration fails on call bring up due to
     * incorrect provisioning.
     */
    MIP_CONFIG_FAILURE = 2050 /* 0x802 */,
    /**
     * PDN inactivity timer expired due to no data transmission in a configurable duration of time.
     */
    PDN_INACTIVITY_TIMER_EXPIRED = 2051 /* 0x803 */,
    /**
     * IPv4 data call bring up is rejected because the UE already maintains the allotted maximum
     * number of IPv4 data connections.
     */
    MAX_IPV4_CONNECTIONS = 2052 /* 0x804 */,
    /**
     * IPv6 data call bring up is rejected because the UE already maintains the allotted maximum
     * number of IPv6 data connections.
     */
    MAX_IPV6_CONNECTIONS = 2053 /* 0x805 */,
    /**
     * New PDN bring up is rejected during interface selection because the UE has already allotted
     * the available interfaces for other PDNs.
     */
    APN_MISMATCH = 2054 /* 0x806 */,
    /**
     * New call bring up is rejected since the existing data call IP type doesn't match the
     * requested IP.
     */
    IP_VERSION_MISMATCH = 2055 /* 0x807 */,
    /**
     * Dial up networking (DUN) call bring up is rejected since UE is in eHRPD RAT.
     */
    DUN_CALL_DISALLOWED = 2056 /* 0x808 */,
    /**
     * Rejected/Brought down since UE is transition between EPC and NONEPC RAT.
     */
    INTERNAL_EPC_NONEPC_TRANSITION = 2057 /* 0x809 */,
    /**
     * The current interface is being in use.
     */
    INTERFACE_IN_USE = 2058 /* 0x80A */,
    /**
     * PDN connection to the APN is disallowed on the roaming network.
     */
    APN_DISALLOWED_ON_ROAMING = 2059 /* 0x80B */,
    /**
     * APN-related parameters are changed.
     */
    APN_PARAMETERS_CHANGED = 2060 /* 0x80C */,
    /**
     * PDN is attempted to be brought up with NULL APN but NULL APN is not supported.
     */
    NULL_APN_DISALLOWED = 2061 /* 0x80D */,
    /**
     * Thermal level increases and causes calls to be torn down when normal mode of operation is
     * not allowed.
     */
    THERMAL_MITIGATION = 2062 /* 0x80E */,
    /**
     * PDN Connection to a given APN is disallowed because data is disabled from the device user
     * interface settings.
     */
    DATA_SETTINGS_DISABLED = 2063 /* 0x80F */,
    /**
     * PDN Connection to a given APN is disallowed because data roaming is disabled from the device
     * user interface settings and the UE is roaming.
     */
    DATA_ROAMING_SETTINGS_DISABLED = 2064 /* 0x810 */,
    /**
     * DDS (Default data subscription) switch occurs.
     */
    DDS_SWITCHED = 2065 /* 0x811 */,
    /**
     * PDN being brought up with an APN that is part of forbidden APN Name list.
     */
    FORBIDDEN_APN_NAME = 2066 /* 0x812 */,
    /**
     * Default data subscription switch is in progress.
     */
    DDS_SWITCH_IN_PROGRESS = 2067 /* 0x813 */,
    /**
     * Roaming is disallowed during call bring up.
     */
    CALL_DISALLOWED_IN_ROAMING = 2068 /* 0x814 */,
    /**
     * UE is unable to bring up a non-IP data call because the device is not camped on a NB1 cell.
     */
    NON_IP_NOT_SUPPORTED = 2069 /* 0x815 */,
    /**
     * Non-IP PDN is in throttled state due to previous VSNCP bringup failure(s).
     */
    PDN_NON_IP_CALL_THROTTLED = 2070 /* 0x816 */,
    /**
     * Non-IP PDN is in disallowed state due to the network providing only an IP address.
     */
    PDN_NON_IP_CALL_DISALLOWED = 2071 /* 0x817 */,
    /**
     * Device in CDMA locked state.
     */
    CDMA_LOCK = 2072 /* 0x818 */,
    /**
     * Received an intercept order from the base station.
     */
    CDMA_INTERCEPT = 2073 /* 0x819 */,
    /**
     * Receiving a reorder from the base station.
     */
    CDMA_REORDER = 2074 /* 0x81A */,
    /**
     * Receiving a release from the base station with a SO (Service Option) Reject reason.
     */
    CDMA_RELEASE_DUE_TO_SO_REJECTION = 2075 /* 0x81B */,
    /**
     * Receiving an incoming call from the base station.
     */
    CDMA_INCOMING_CALL = 2076 /* 0x81C */,
    /**
     * Received an alert stop from the base station due to incoming only.
     */
    CDMA_ALERT_STOP = 2077 /* 0x81D */,
    /**
     * Channel acquisition failures. This indicates that device has failed acquiring all the
     * channels in the PRL.
     */
    CHANNEL_ACQUISITION_FAILURE = 2078 /* 0x81E */,
    /**
     * Maximum access probes transmitted.
     */
    MAX_ACCESS_PROBE = 2079 /* 0x81F */,
    /**
     * Concurrent service is not supported by base station.
     */
    CONCURRENT_SERVICE_NOT_SUPPORTED_BY_BASE_STATION = 2080 /* 0x820 */,
    /**
     * There was no response received from the base station.
     */
    NO_RESPONSE_FROM_BASE_STATION = 2081 /* 0x821 */,
    /**
     * The base station rejecting the call.
     */
    REJECTED_BY_BASE_STATION = 2082 /* 0x822 */,
    /**
     * The concurrent services requested were not compatible.
     */
    CONCURRENT_SERVICES_INCOMPATIBLE = 2083 /* 0x823 */,
    /**
     * Device does not have CDMA service.
     */
    NO_CDMA_SERVICE = 2084 /* 0x824 */,
    /**
     * RUIM not being present.
     */
    RUIM_NOT_PRESENT = 2085 /* 0x825 */,
    /**
     * Receiving a retry order from the base station.
     */
    CDMA_RETRY_ORDER = 2086 /* 0x826 */,
    /**
     * Access blocked by the base station.
     */
    ACCESS_BLOCK = 2087 /* 0x827 */,
    /**
     * Access blocked by the base station for all mobile devices.
     */
    ACCESS_BLOCK_ALL = 2088 /* 0x828 */,
    /**
     * Maximum access probes for the IS-707B call.
     */
    IS707B_MAX_ACCESS_PROBES = 2089 /* 0x829 */,
    /**
     * Put device in thermal emergency.
     */
    THERMAL_EMERGENCY = 2090 /* 0x82A */,
    /**
     * In favor of a voice call or SMS when concurrent voice and data are not supported.
     */
    CONCURRENT_SERVICES_NOT_ALLOWED = 2091 /* 0x82B */,
    /**
     * The other clients rejected incoming call.
     */
    INCOMING_CALL_REJECTED = 2092 /* 0x82C */,
    /**
     * No service on the gateway.
     */
    NO_SERVICE_ON_GATEWAY = 2093 /* 0x82D */,
    /**
     * GPRS context is not available.
     */
    NO_GPRS_CONTEXT = 2094 /* 0x82E */,
    /**
     * Network refuses service to the MS because either an identity of the MS is not acceptable to
     * the network or the MS does not pass the authentication check.
     */
    ILLEGAL_MS = 2095 /* 0x82F */,
    /**
     * ME could not be authenticated and the ME used is not acceptable to the network.
     */
    ILLEGAL_ME = 2096 /* 0x830 */,
    /**
     * Not allowed to operate either GPRS or non-GPRS services.
     */
    GPRS_SERVICES_AND_NON_GPRS_SERVICES_NOT_ALLOWED = 2097 /* 0x831 */,
    /**
     * MS is not allowed to operate GPRS services.
     */
    GPRS_SERVICES_NOT_ALLOWED = 2098 /* 0x832 */,
    /**
     * No matching identity or context could be found in the network.
     */
    MS_IDENTITY_CANNOT_BE_DERIVED_BY_THE_NETWORK = 2099 /* 0x833 */,
    /**
     * Mobile reachable timer has expired, or the GMM context data related to the subscription does
     * not exist in the SGSN.
     */
    IMPLICITLY_DETACHED = 2100 /* 0x834 */,
    /**
     * UE requests GPRS service, or the network initiates a detach request in a PLMN which does not
     * offer roaming for GPRS services to that MS.
     */
    PLMN_NOT_ALLOWED = 2101 /* 0x835 */,
    /**
     * MS requests service, or the network initiates a detach request, in a location area where the
     * HPLMN determines that the MS, by subscription, is not allowed to operate.
     */
    LOCATION_AREA_NOT_ALLOWED = 2102 /* 0x836 */,
    /**
     * UE requests GPRS service or the network initiates a detach request in a PLMN that does not
     * offer roaming for GPRS services.
     */
    GPRS_SERVICES_NOT_ALLOWED_IN_THIS_PLMN = 2103 /* 0x837 */,
    /**
     * PDP context already exists.
     */
    PDP_DUPLICATE = 2104 /* 0x838 */,
    /**
     * RAT change on the UE.
     */
    UE_RAT_CHANGE = 2105 /* 0x839 */,
    /**
     * Network cannot serve a request from the MS due to congestion.
     */
    CONGESTION = 2106 /* 0x83A */,
    /**
     * MS requests an establishment of the radio access bearers for all active PDP contexts by
     * sending a service request message indicating data to the network, but the SGSN does not have
     * any active PDP context.
     */
    NO_PDP_CONTEXT_ACTIVATED = 2107 /* 0x83B */,
    /**
     * Access class blocking restrictions for the current camped cell.
     */
    ACCESS_CLASS_DSAC_REJECTION = 2108 /* 0x83C */,
    /**
     * SM attempts PDP activation for a maximum of four attempts.
     */
    PDP_ACTIVATE_MAX_RETRY_FAILED = 2109 /* 0x83D */,
    /**
     * Radio access bearer failure.
     */
    RADIO_ACCESS_BEARER_FAILURE = 2110 /* 0x83E */,
    /**
     * Invalid EPS bearer identity in the request.
     */
    ESM_UNKNOWN_EPS_BEARER_CONTEXT = 2111 /* 0x83F */,
    /**
     * Data radio bearer is released by the RRC.
     */
    DRB_RELEASED_BY_RRC = 2112 /* 0x840 */,
    /**
     * Indicate the connection was released.
     */
    CONNECTION_RELEASED = 2113 /* 0x841 */,
    /**
     * UE is detached.
     */
    EMM_DETACHED = 2114 /* 0x842 */,
    /**
     * Attach procedure is rejected by the network.
     */
    EMM_ATTACH_FAILED = 2115 /* 0x843 */,
    /**
     * Attach procedure is started for EMC purposes.
     */
    EMM_ATTACH_STARTED = 2116 /* 0x844 */,
    /**
     * Service request procedure failure.
     */
    LTE_NAS_SERVICE_REQUEST_FAILED = 2117 /* 0x845 */,
    /**
     * Active dedicated bearer was requested using the same default bearer ID.
     */
    DUPLICATE_BEARER_ID = 2118 /* 0x846 */,
    /**
     * Collision scenarios for the UE and network-initiated procedures.
     */
    ESM_COLLISION_SCENARIOS = 2119 /* 0x847 */,
    /**
     * Bearer must be deactivated to synchronize with the network.
     */
    ESM_BEARER_DEACTIVATED_TO_SYNC_WITH_NETWORK = 2120 /* 0x848 */,
    /**
     * Active dedicated bearer was requested for an existing default bearer.
     */
    ESM_NW_ACTIVATED_DED_BEARER_WITH_ID_OF_DEF_BEARER = 2121 /* 0x849 */,
    /**
     * Bad OTA message is received from the network.
     */
    ESM_BAD_OTA_MESSAGE = 2122 /* 0x84A */,
    /**
     * Download server rejected the call.
     */
    ESM_DOWNLOAD_SERVER_REJECTED_THE_CALL = 2123 /* 0x84B */,
    /**
     * PDN was disconnected by the downlaod server due to IRAT.
     */
    ESM_CONTEXT_TRANSFERRED_DUE_TO_IRAT = 2124 /* 0x84C */,
    /**
     * Dedicated bearer will be deactivated regardless of the network response.
     */
    DS_EXPLICIT_DEACTIVATION = 2125 /* 0x84D */,
    /**
     * No specific local cause is mentioned, usually a valid OTA cause.
     */
    ESM_LOCAL_CAUSE_NONE = 2126 /* 0x84E */,
    /**
     * Throttling is not needed for this service request failure.
     */
    LTE_THROTTLING_NOT_REQUIRED = 2127 /* 0x84F */,
    /**
     * Access control list check failure at the lower layer.
     */
    ACCESS_CONTROL_LIST_CHECK_FAILURE = 2128 /* 0x850 */,
    /**
     * Service is not allowed on the requested PLMN.
     */
    SERVICE_NOT_ALLOWED_ON_PLMN = 2129 /* 0x851 */,
    /**
     * T3417 timer expiration of the service request procedure.
     */
    EMM_T3417_EXPIRED = 2130 /* 0x852 */,
    /**
     * Extended service request fails due to expiration of the T3417 EXT timer.
     */
    EMM_T3417_EXT_EXPIRED = 2131 /* 0x853 */,
    /**
     * Transmission failure of radio resource control (RRC) uplink data.
     */
    RRC_UPLINK_DATA_TRANSMISSION_FAILURE = 2132 /* 0x854 */,
    /**
     * Radio resource control (RRC) uplink data delivery failed due to a handover.
     */
    RRC_UPLINK_DELIVERY_FAILED_DUE_TO_HANDOVER = 2133 /* 0x855 */,
    /**
     * Radio resource control (RRC) uplink data delivery failed due to a connection release.
     */
    RRC_UPLINK_CONNECTION_RELEASE = 2134 /* 0x856 */,
    /**
     * Radio resource control (RRC) uplink data delivery failed due to a radio link failure.
     */
    RRC_UPLINK_RADIO_LINK_FAILURE = 2135 /* 0x857 */,
    /**
     * Radio resource control (RRC) is not connected but the non-access stratum (NAS) sends an
     * uplink data request.
     */
    RRC_UPLINK_ERROR_REQUEST_FROM_NAS = 2136 /* 0x858 */,
    /**
     * Radio resource control (RRC) connection failure at access stratum.
     */
    RRC_CONNECTION_ACCESS_STRATUM_FAILURE = 2137 /* 0x859 */,
    /**
     * Radio resource control (RRC) connection establishment is aborted due to another procedure.
     */
    RRC_CONNECTION_ANOTHER_PROCEDURE_IN_PROGRESS = 2138 /* 0x85A */,
    /**
     * Radio resource control (RRC) connection establishment failed due to access barrred.
     */
    RRC_CONNECTION_ACCESS_BARRED = 2139 /* 0x85B */,
    /**
     * Radio resource control (RRC) connection establishment failed due to cell reselection at
     * access stratum.
     */
    RRC_CONNECTION_CELL_RESELECTION = 2140 /* 0x85C */,
    /**
     * Connection establishment failed due to configuration failure at the radio resource control
     * (RRC).
     */
    RRC_CONNECTION_CONFIG_FAILURE = 2141 /* 0x85D */,
    /**
     * Radio resource control (RRC) connection could not be established in the time limit.
     */
    RRC_CONNECTION_TIMER_EXPIRED = 2142 /* 0x85E */,
    /**
     * Connection establishment failed due to a link failure at the radio resource control (RRC).
     */
    RRC_CONNECTION_LINK_FAILURE = 2143 /* 0x85F */,
    /**
     * Connection establishment failed as the radio resource control (RRC) is not camped on any
     * cell.
     */
    RRC_CONNECTION_CELL_NOT_CAMPED = 2144 /* 0x860 */,
    /**
     * Connection establishment failed due to a service interval failure at the radio resource
     * control (RRC).
     */
    RRC_CONNECTION_SYSTEM_INTERVAL_FAILURE = 2145 /* 0x861 */,
    /**
     * Radio resource control (RRC) connection establishment failed due to the network rejecting the
     * UE connection request.
     */
    RRC_CONNECTION_REJECT_BY_NETWORK = 2146 /* 0x862 */,
    /**
     * Normal radio resource control (RRC) connection release.
     */
    RRC_CONNECTION_NORMAL_RELEASE = 2147 /* 0x863 */,
    /**
     * Radio resource control (RRC) connection release failed due to radio link failure conditions.
     */
    RRC_CONNECTION_RADIO_LINK_FAILURE = 2148 /* 0x864 */,
    /**
     * Radio resource control (RRC) connection re-establishment failure.
     */
    RRC_CONNECTION_REESTABLISHMENT_FAILURE = 2149 /* 0x865 */,
    /**
     * UE is out of service during the call register.
     */
    RRC_CONNECTION_OUT_OF_SERVICE_DURING_CELL_REGISTER = 2150 /* 0x866 */,
    /**
     * Connection has been released by the radio resource control (RRC) due to an abort request.
     */
    RRC_CONNECTION_ABORT_REQUEST = 2151 /* 0x867 */,
    /**
     * Radio resource control (RRC) connection released due to a system information block read
     * error.
     */
    RRC_CONNECTION_SYSTEM_INFORMATION_BLOCK_READ_ERROR = 2152 /* 0x868 */,
    /**
     * Network-initiated detach with reattach.
     */
    NETWORK_INITIATED_DETACH_WITH_AUTO_REATTACH = 2153 /* 0x869 */,
    /**
     * Network-initiated detach without reattach.
     */
    NETWORK_INITIATED_DETACH_NO_AUTO_REATTACH = 2154 /* 0x86A */,
    /**
     * ESM procedure maximum attempt timeout failure.
     */
    ESM_PROCEDURE_TIME_OUT = 2155 /* 0x86B */,
    /**
     * No PDP exists with the given connection ID while modifying or deactivating or activation for
     * an already active PDP.
     */
    INVALID_CONNECTION_ID = 2156 /* 0x86C */,
    /**
     * Maximum NSAPIs have been exceeded during PDP activation.
     */
    MAXIMIUM_NSAPIS_EXCEEDED = 2157 /* 0x86D */,
    /**
     * Primary context for NSAPI does not exist.
     */
    INVALID_PRIMARY_NSAPI = 2158 /* 0x86E */,
    /**
     * Unable to encode the OTA message for MT PDP or deactivate PDP.
     */
    CANNOT_ENCODE_OTA_MESSAGE = 2159 /* 0x86F */,
    /**
     * Radio access bearer is not established by the lower layers during activation, modification,
     * or deactivation.
     */
    RADIO_ACCESS_BEARER_SETUP_FAILURE = 2160 /* 0x870 */,
    /**
     * Expiration of the PDP establish timer with a maximum of five retries.
     */
    PDP_ESTABLISH_TIMEOUT_EXPIRED = 2161 /* 0x871 */,
    /**
     * Expiration of the PDP modify timer with a maximum of four retries.
     */
    PDP_MODIFY_TIMEOUT_EXPIRED = 2162 /* 0x872 */,
    /**
     * Expiration of the PDP deactivate timer with a maximum of four retries.
     */
    PDP_INACTIVE_TIMEOUT_EXPIRED = 2163 /* 0x873 */,
    /**
     * PDP activation failed due to RRC_ABORT or a forbidden PLMN.
     */
    PDP_LOWERLAYER_ERROR = 2164 /* 0x874 */,
    /**
     * MO PDP modify collision when the MT PDP is already in progress.
     */
    PDP_MODIFY_COLLISION = 2165 /* 0x875 */,
    /**
     * Maximum size of the L2 message was exceeded.
     */
    MAXINUM_SIZE_OF_L2_MESSAGE_EXCEEDED = 2166 /* 0x876 */,
    /**
     * Non-access stratum (NAS) request was rejected by the network.
     */
    NAS_REQUEST_REJECTED_BY_NETWORK = 2167 /* 0x877 */,
    /**
     * Radio resource control (RRC) connection establishment failure due to an error in the request
     * message.
     */
    RRC_CONNECTION_INVALID_REQUEST = 2168 /* 0x878 */,
    /**
     * Radio resource control (RRC) connection establishment failure due to a change in the tracking
     * area ID.
     */
    RRC_CONNECTION_TRACKING_AREA_ID_CHANGED = 2169 /* 0x879 */,
    /**
     * Radio resource control (RRC) connection establishment failure due to the RF was unavailable.
     */
    RRC_CONNECTION_RF_UNAVAILABLE = 2170 /* 0x87A */,
    /**
     * Radio resource control (RRC) connection was aborted before deactivating the LTE stack due to
     * a successful LTE to WCDMA/GSM/TD-SCDMA IRAT change.
     */
    RRC_CONNECTION_ABORTED_DUE_TO_IRAT_CHANGE = 2171 /* 0x87B */,
    /**
     * If the UE has an LTE radio link failure before security is established, the radio resource
     * control (RRC) connection must be released and the UE must return to idle.
     */
    RRC_CONNECTION_RELEASED_SECURITY_NOT_ACTIVE = 2172 /* 0x87C */,
    /**
     * Radio resource control (RRC) connection was aborted by the non-access stratum (NAS) after an
     * IRAT to LTE IRAT handover.
     */
    RRC_CONNECTION_ABORTED_AFTER_HANDOVER = 2173 /* 0x87D */,
    /**
     * Radio resource control (RRC) connection was aborted before deactivating the LTE stack after a
     * successful LTE to GSM/EDGE IRAT cell change order procedure.
     */
    RRC_CONNECTION_ABORTED_AFTER_IRAT_CELL_CHANGE = 2174 /* 0x87E */,
    /**
     * Radio resource control (RRC) connection was aborted in the middle of a LTE to GSM IRAT cell
     * change order procedure.
     */
    RRC_CONNECTION_ABORTED_DURING_IRAT_CELL_CHANGE = 2175 /* 0x87F */,
    /**
     * IMSI present in the UE is unknown in the home subscriber server.
     */
    IMSI_UNKNOWN_IN_HOME_SUBSCRIBER_SERVER = 2176 /* 0x880 */,
    /**
     * IMEI of the UE is not accepted by the network.
     */
    IMEI_NOT_ACCEPTED = 2177 /* 0x881 */,
    /**
     * EPS and non-EPS services are not allowed by the network.
     */
    EPS_SERVICES_AND_NON_EPS_SERVICES_NOT_ALLOWED = 2178 /* 0x882 */,
    /**
     * EPS services are not allowed in the PLMN.
     */
    EPS_SERVICES_NOT_ALLOWED_IN_PLMN = 2179 /* 0x883 */,
    /**
     * Mobile switching center is temporarily unreachable.
     */
    MSC_TEMPORARILY_NOT_REACHABLE = 2180 /* 0x884 */,
    /**
     * CS domain is not available.
     */
    CS_DOMAIN_NOT_AVAILABLE = 2181 /* 0x885 */,
    /**
     * ESM level failure.
     */
    ESM_FAILURE = 2182 /* 0x886 */,
    /**
     * MAC level failure.
     */
    MAC_FAILURE = 2183 /* 0x887 */,
    /**
     * Synchronization failure.
     */
    SYNCHRONIZATION_FAILURE = 2184 /* 0x888 */,
    /**
     * UE security capabilities mismatch.
     */
    UE_SECURITY_CAPABILITIES_MISMATCH = 2185 /* 0x889 */,
    /**
     * Unspecified security mode reject.
     */
    SECURITY_MODE_REJECTED = 2186 /* 0x88A */,
    /**
     * Unacceptable non-EPS authentication.
     */
    UNACCEPTABLE_NON_EPS_AUTHENTICATION = 2187 /* 0x88B */,
    /**
     * CS fallback call establishment is not allowed.
     */
    CS_FALLBACK_CALL_ESTABLISHMENT_NOT_ALLOWED = 2188 /* 0x88C */,
    /**
     * No EPS bearer context was activated.
     */
    NO_EPS_BEARER_CONTEXT_ACTIVATED = 2189 /* 0x88D */,
    /**
     * Invalid EMM state.
     */
    INVALID_EMM_STATE = 2190 /* 0x88E */,
    /**
     * Non-Access Spectrum layer failure.
     */
    NAS_LAYER_FAILURE = 2191 /* 0x88F */,
    /**
     * Multiple PDP call feature is disabled.
     */
    MULTIPLE_PDP_CALL_NOT_ALLOWED = 2192 /* 0x890 */,
    /**
     * Data call has been brought down because EMBMS is not enabled at the RRC layer.
     */
    EMBMS_NOT_ENABLED = 2193 /* 0x891 */,
    /**
     * Data call was unsuccessfully transferred during the IRAT handover.
     */
    IRAT_HANDOVER_FAILED = 2194 /* 0x892 */,
    /**
     * EMBMS data call has been successfully brought down.
     */
    EMBMS_REGULAR_DEACTIVATION = 2195 /* 0x893 */,
    /**
     * Test loop-back data call has been successfully brought down.
     */
    TEST_LOOPBACK_REGULAR_DEACTIVATION = 2196 /* 0x894 */,
    /**
     * Lower layer registration failure.
     */
    LOWER_LAYER_REGISTRATION_FAILURE = 2197 /* 0x895 */,
    /**
     * Network initiates a detach on LTE with error cause "data plan has been replenished or has
     * expired".
     */
    DATA_PLAN_EXPIRED = 2198 /* 0x896 */,
    /**
     * UMTS interface is brought down due to handover from UMTS to iWLAN.
     */
    UMTS_HANDOVER_TO_IWLAN = 2199 /* 0x897 */,
    /**
     * Received a connection deny due to general or network busy on EVDO network.
     */
    EVDO_CONNECTION_DENY_BY_GENERAL_OR_NETWORK_BUSY = 2200 /* 0x898 */,
    /**
     * Received a connection deny due to billing or authentication failure on EVDO network.
     */
    EVDO_CONNECTION_DENY_BY_BILLING_OR_AUTHENTICATION_FAILURE = 2201 /* 0x899 */,
    /**
     * HDR system has been changed due to redirection or the PRL was not preferred.
     */
    EVDO_HDR_CHANGED = 2202 /* 0x89A */,
    /**
     * Device exited HDR due to redirection or the PRL was not preferred.
     */
    EVDO_HDR_EXITED = 2203 /* 0x89B */,
    /**
     * Device does not have an HDR session.
     */
    EVDO_HDR_NO_SESSION = 2204 /* 0x89C */,
    /**
     * It is ending an HDR call origination in favor of a GPS fix.
     */
    EVDO_USING_GPS_FIX_INSTEAD_OF_HDR_CALL = 2205 /* 0x89D */,
    /**
     * Connection setup on the HDR system was time out.
     */
    EVDO_HDR_CONNECTION_SETUP_TIMEOUT = 2206 /* 0x89E */,
    /**
     * Device failed to acquire a co-located HDR for origination.
     */
    FAILED_TO_ACQUIRE_COLOCATED_HDR = 2207 /* 0x89F */,
    /**
     * OTASP commit is in progress.
     */
    OTASP_COMMIT_IN_PROGRESS = 2208 /* 0x8A0 */,
    /**
     * Device has no hybrid HDR service.
     */
    NO_HYBRID_HDR_SERVICE = 2209 /* 0x8A1 */,
    /**
     * HDR module could not be obtained because of the RF locked.
     */
    HDR_NO_LOCK_GRANTED = 2210 /* 0x8A2 */,
    /**
     * DBM or SMS is in progress.
     */
    DBM_OR_SMS_IN_PROGRESS = 2211 /* 0x8A3 */,
    /**
     * HDR module released the call due to fade.
     */
    HDR_FADE = 2212 /* 0x8A4 */,
    /**
     * HDR system access failure.
     */
    HDR_ACCESS_FAILURE = 2213 /* 0x8A5 */,
    /**
     * P_rev supported by 1 base station is less than 6, which is not supported for a 1X data call.
     * The UE must be in the footprint of BS which has p_rev >= 6 to support this SO33 call.
     */
    UNSUPPORTED_1X_PREV = 2214 /* 0x8A6 */,
    /**
     * Client ended the data call.
     */
    LOCAL_END = 2215 /* 0x8A7 */,
    /**
     * Device has no service.
     */
    NO_SERVICE = 2216 /* 0x8A8 */,
    /**
     * Device lost the system due to fade.
     */
    FADE = 2217 /* 0x8A9 */,
    /**
     * Receiving a release from the base station with no reason.
     */
    NORMAL_RELEASE = 2218 /* 0x8AA */,
    /**
     * Access attempt is already in progress.
     */
    ACCESS_ATTEMPT_ALREADY_IN_PROGRESS = 2219 /* 0x8AB */,
    /**
     * Device is in the process of redirecting or handing off to a different target system.
     */
    REDIRECTION_OR_HANDOFF_IN_PROGRESS = 2220 /* 0x8AC */,
    /**
     * Device is operating in Emergency mode.
     */
    EMERGENCY_MODE = 2221 /* 0x8AD */,
    /**
     * Device is in use (e.g., voice call).
     */
    PHONE_IN_USE = 2222 /* 0x8AE */,
    /**
     * Device operational mode is different from the mode requested in the traffic channel bring up.
     */
    INVALID_MODE = 2223 /* 0x8AF */,
    /**
     * SIM was marked by the network as invalid for the circuit and/or packet service domain.
     */
    INVALID_SIM_STATE = 2224 /* 0x8B0 */,
    /**
     * There is no co-located HDR.
     */
    NO_COLLOCATED_HDR = 2225 /* 0x8B1 */,
    /**
     * UE is entering power save mode.
     */
    UE_IS_ENTERING_POWERSAVE_MODE = 2226 /* 0x8B2 */,
    /**
     * Dual switch from single standby to dual standby is in progress.
     */
    DUAL_SWITCH = 2227 /* 0x8B3 */,
    /**
     * Data call bring up fails in the PPP setup due to a timeout. (e.g., an LCP conf ack was not
     * received from the network)
     */
    PPP_TIMEOUT = 2228 /* 0x8B4 */,
    /**
     * Data call bring up fails in the PPP setup due to an authorization failure.
     * (e.g., authorization is required, but not negotiated with the network during an LCP phase)
     */
    PPP_AUTH_FAILURE = 2229 /* 0x8B5 */,
    /**
     * Data call bring up fails in the PPP setup due to an option mismatch.
     */
    PPP_OPTION_MISMATCH = 2230 /* 0x8B6 */,
    /**
     * Data call bring up fails in the PPP setup due to a PAP failure.
     */
    PPP_PAP_FAILURE = 2231 /* 0x8B7 */,
    /**
     * Data call bring up fails in the PPP setup due to a CHAP failure.
     */
    PPP_CHAP_FAILURE = 2232 /* 0x8B8 */,
    /**
     * Data call bring up fails in the PPP setup because the PPP is in the process of cleaning the
     * previous PPP session.
     */
    PPP_CLOSE_IN_PROGRESS = 2233 /* 0x8B9 */,
    /**
     * IPv6 interface bring up fails because the network provided only the IPv4 address for the
     * upcoming PDN permanent client can reattempt a IPv6 call bring up after the IPv4 interface is
     * also brought down. However, there is no guarantee that the network will provide a IPv6
     * address.
     */
    LIMITED_TO_IPV4 = 2234 /* 0x8BA */,
    /**
     * IPv4 interface bring up fails because the network provided only the IPv6 address for the
     * upcoming PDN permanent client can reattempt a IPv4 call bring up after the IPv6 interface is
     * also brought down. However there is no guarantee that the network will provide a IPv4
     * address.
     */
    LIMITED_TO_IPV6 = 2235 /* 0x8BB */,
    /**
     * Data call bring up fails in the VSNCP phase due to a VSNCP timeout error.
     */
    VSNCP_TIMEOUT = 2236 /* 0x8BC */,
    /**
     * Data call bring up fails in the VSNCP phase due to a general error. It's used when there is
     * no other specific error code available to report the failure.
     */
    VSNCP_GEN_ERROR = 2237 /* 0x8BD */,
    /**
     * Data call bring up fails in the VSNCP phase due to a network rejection of the VSNCP
     * configuration request because the requested APN is unauthorized.
     */
    VSNCP_APN_UNATHORIZED = 2238 /* 0x8BE */,
    /**
     * Data call bring up fails in the VSNCP phase due to a network rejection of the VSNCP
     * configuration request because the PDN limit has been exceeded.
     */
    VSNCP_PDN_LIMIT_EXCEEDED = 2239 /* 0x8BF */,
    /**
     * Data call bring up fails in the VSNCP phase due to the network rejected the VSNCP
     * configuration request due to no PDN gateway address.
     */
    VSNCP_NO_PDN_GATEWAY_ADDRESS = 2240 /* 0x8C0 */,
    /**
     * Data call bring up fails in the VSNCP phase due to a network rejection of the VSNCP
     * configuration request because the PDN gateway is unreachable.
     */
    VSNCP_PDN_GATEWAY_UNREACHABLE = 2241 /* 0x8C1 */,
    /**
     * Data call bring up fails in the VSNCP phase due to a network rejection of the VSNCP
     * configuration request due to a PDN gateway reject.
     */
    VSNCP_PDN_GATEWAY_REJECT = 2242 /* 0x8C2 */,
    /**
     * Data call bring up fails in the VSNCP phase due to a network rejection of the VSNCP
     * configuration request with the reason of insufficient parameter.
     */
    VSNCP_INSUFFICIENT_PARAMETERS = 2243 /* 0x8C3 */,
    /**
     * Data call bring up fails in the VSNCP phase due to a network rejection of the VSNCP
     * configuration request with the reason of resource unavailable.
     */
    VSNCP_RESOURCE_UNAVAILABLE = 2244 /* 0x8C4 */,
    /**
     * Data call bring up fails in the VSNCP phase due to a network rejection of the VSNCP
     * configuration request with the reason of administratively prohibited at the HSGW.
     */
    VSNCP_ADMINISTRATIVELY_PROHIBITED = 2245 /* 0x8C5 */,
    /**
     * Data call bring up fails in the VSNCP phase due to a network rejection of PDN ID in use, or
     * all existing PDNs are brought down with this end reason because one of the PDN bring up was
     * rejected by the network with the reason of PDN ID in use.
     */
    VSNCP_PDN_ID_IN_USE = 2246 /* 0x8C6 */,
    /**
     * Data call bring up fails in the VSNCP phase due to a network rejection of the VSNCP
     * configuration request for the reason of subscriber limitation.
     */
    VSNCP_SUBSCRIBER_LIMITATION = 2247 /* 0x8C7 */,
    /**
     * Data call bring up fails in the VSNCP phase due to a network rejection of the VSNCP
     * configuration request because the PDN exists for this APN.
     */
    VSNCP_PDN_EXISTS_FOR_THIS_APN = 2248 /* 0x8C8 */,
    /**
     * Data call bring up fails in the VSNCP phase due to a network rejection of the VSNCP
     * configuration request with reconnect to this PDN not allowed, or an active data call is
     * terminated by the network because reconnection to this PDN is not allowed. Upon receiving
     * this error code from the network, the modem infinitely throttles the PDN until the next power
     * cycle.
     */
    VSNCP_RECONNECT_NOT_ALLOWED = 2249 /* 0x8C9 */,
    /**
     * Device failure to obtain the prefix from the network.
     */
    IPV6_PREFIX_UNAVAILABLE = 2250 /* 0x8CA */,
    /**
     * System preference change back to SRAT during handoff
     */
    HANDOFF_PREFERENCE_CHANGED = 2251 /* 0x8CB */,
};

/**
 * Data connection active status
 */
enum class DataConnActiveStatus : int32_t {
    /**
     * Indicates the data connection is inactive.
     */
    INACTIVE = 0,
    /**
     * Indicates the data connection is active with physical link dormant.
     */
    DORMANT = 1,
    /**
     * Indicates the data connection is active with physical link up.
     */
    ACTIVE = 2,
};

/**
 * Specifies the type of packet data protocol which is defined in TS 27.007 section 10.1.1.
 */
enum class PdpProtocolType : int32_t {
    /**
     * Unknown protocol
     */
    UNKNOWN = -1 /* (-1) */,
    /**
     * Internet protocol
     */
    IP = 0,
    /**
     * Internet protocol, version 6
     */
    IPV6 = 1,
    /**
     * Virtual PDP type introduced to handle dual IP stack UE capability.
     */
    IPV4V6 = 2,
    /**
     * Point to point protocol
     */
    PPP = 3,
    /**
     * Transfer of Non-IP data to external packet data network
     */
    NON_IP = 4,
    /**
     * Transfer of Unstructured data to the Data Network via N6
     */
    UNSTRUCTURED = 5,
};

struct RadioFrequencyInfo final {
    enum class hidl_discriminator : uint8_t {
        range = 0,  // ::android::hardware::radio::V1_4::FrequencyRange
        channelNumber = 1,  // int32_t
    };

    RadioFrequencyInfo();
    ~RadioFrequencyInfo();
    RadioFrequencyInfo(RadioFrequencyInfo&&);
    RadioFrequencyInfo(const RadioFrequencyInfo&);
    RadioFrequencyInfo& operator=(RadioFrequencyInfo&&);
    RadioFrequencyInfo& operator=(const RadioFrequencyInfo&);

    void range(::android::hardware::radio::V1_4::FrequencyRange);
    ::android::hardware::radio::V1_4::FrequencyRange& range();
    ::android::hardware::radio::V1_4::FrequencyRange range() const;

    void channelNumber(int32_t);
    int32_t& channelNumber();
    int32_t channelNumber() const;

    // Utility methods
    hidl_discriminator getDiscriminator() const;

    constexpr size_t hidl_getUnionOffset() const {
        return offsetof(::android::hardware::radio::V1_4::RadioFrequencyInfo, hidl_u);
    }

private:
    void hidl_destructUnion();

    hidl_discriminator hidl_d __attribute__ ((aligned(1))) ;
    union hidl_union final {
        ::android::hardware::radio::V1_4::FrequencyRange range __attribute__ ((aligned(4)));
        int32_t channelNumber __attribute__ ((aligned(4)));

        hidl_union();
        ~hidl_union();
    } hidl_u;

    static_assert(sizeof(::android::hardware::radio::V1_4::RadioFrequencyInfo::hidl_union) == 4, "wrong size");
    static_assert(__alignof(::android::hardware::radio::V1_4::RadioFrequencyInfo::hidl_union) == 4, "wrong alignment");
    static_assert(sizeof(::android::hardware::radio::V1_4::RadioFrequencyInfo::hidl_discriminator) == 1, "wrong size");
    static_assert(__alignof(::android::hardware::radio::V1_4::RadioFrequencyInfo::hidl_discriminator) == 1, "wrong alignment");
};

static_assert(sizeof(::android::hardware::radio::V1_4::RadioFrequencyInfo) == 8, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::RadioFrequencyInfo) == 4, "wrong alignment");

struct PhysicalChannelConfig final {
    ::android::hardware::radio::V1_2::PhysicalChannelConfig base __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_4::RadioTechnology rat __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_4::RadioFrequencyInfo rfInfo __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<int32_t> contextIds __attribute__ ((aligned(8)));
    uint32_t physicalCellId __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::PhysicalChannelConfig, base) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::PhysicalChannelConfig, rat) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::PhysicalChannelConfig, rfInfo) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::PhysicalChannelConfig, contextIds) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::PhysicalChannelConfig, physicalCellId) == 40, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::PhysicalChannelConfig) == 48, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::PhysicalChannelConfig) == 8, "wrong alignment");

/**
 * Type to define the LTE specific network capabilities for voice over PS including
 * emergency and normal voice calls.
 */
struct LteVopsInfo final {
    bool isVopsSupported __attribute__ ((aligned(1)));
    bool isEmcBearerSupported __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::LteVopsInfo, isVopsSupported) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::LteVopsInfo, isEmcBearerSupported) == 1, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::LteVopsInfo) == 2, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::LteVopsInfo) == 1, "wrong alignment");

/**
 * The parameters of NR 5G Non-Standalone.
 */
struct NrIndicators final {
    bool isEndcAvailable __attribute__ ((aligned(1)));
    bool isDcNrRestricted __attribute__ ((aligned(1)));
    bool isNrAvailable __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::NrIndicators, isEndcAvailable) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::NrIndicators, isDcNrRestricted) == 1, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::NrIndicators, isNrAvailable) == 2, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::NrIndicators) == 3, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::NrIndicators) == 1, "wrong alignment");

struct DataRegStateResult final {
    // Forward declaration for forward reference support:
    struct VopsInfo;

    struct VopsInfo final {
        enum class hidl_discriminator : uint8_t {
            noinit = 0,  // ::android::hidl::safe_union::V1_0::Monostate
            lteVopsInfo = 1,  // ::android::hardware::radio::V1_4::LteVopsInfo
        };

        VopsInfo();
        ~VopsInfo();
        VopsInfo(VopsInfo&&);
        VopsInfo(const VopsInfo&);
        VopsInfo& operator=(VopsInfo&&);
        VopsInfo& operator=(const VopsInfo&);

        void noinit(const ::android::hidl::safe_union::V1_0::Monostate&);
        void noinit(::android::hidl::safe_union::V1_0::Monostate&&);
        ::android::hidl::safe_union::V1_0::Monostate& noinit();
        const ::android::hidl::safe_union::V1_0::Monostate& noinit() const;

        void lteVopsInfo(const ::android::hardware::radio::V1_4::LteVopsInfo&);
        void lteVopsInfo(::android::hardware::radio::V1_4::LteVopsInfo&&);
        ::android::hardware::radio::V1_4::LteVopsInfo& lteVopsInfo();
        const ::android::hardware::radio::V1_4::LteVopsInfo& lteVopsInfo() const;

        // Utility methods
        hidl_discriminator getDiscriminator() const;

        constexpr size_t hidl_getUnionOffset() const {
            return offsetof(::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo, hidl_u);
        }

    private:
        void hidl_destructUnion();

        hidl_discriminator hidl_d __attribute__ ((aligned(1))) ;
        union hidl_union final {
            ::android::hidl::safe_union::V1_0::Monostate noinit __attribute__ ((aligned(1)));
            ::android::hardware::radio::V1_4::LteVopsInfo lteVopsInfo __attribute__ ((aligned(1)));

            hidl_union();
            ~hidl_union();
        } hidl_u;

        static_assert(sizeof(::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::hidl_union) == 2, "wrong size");
        static_assert(__alignof(::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::hidl_union) == 1, "wrong alignment");
        static_assert(sizeof(::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::hidl_discriminator) == 1, "wrong size");
        static_assert(__alignof(::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::hidl_discriminator) == 1, "wrong alignment");
    };

    static_assert(sizeof(::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo) == 3, "wrong size");
    static_assert(__alignof(::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo) == 1, "wrong alignment");

    ::android::hardware::radio::V1_2::DataRegStateResult base __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo vopsInfo __attribute__ ((aligned(1)));
    ::android::hardware::radio::V1_4::NrIndicators nrIndicators __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::DataRegStateResult, base) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataRegStateResult, vopsInfo) == 104, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataRegStateResult, nrIndicators) == 107, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::DataRegStateResult) == 112, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::DataRegStateResult) == 8, "wrong alignment");

/**
 * Contains the configuration of the LTE cell tower.
 */
struct CellConfigLte final {
    bool isEndcAvailable __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::CellConfigLte, isEndcAvailable) == 0, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::CellConfigLte) == 1, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::CellConfigLte) == 1, "wrong alignment");

/**
 * Inherits from @1.2::CellInfoLte, in order to add the LTE configuration.
 */
struct CellInfoLte final {
    ::android::hardware::radio::V1_2::CellInfoLte base __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_4::CellConfigLte cellConfig __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::CellInfoLte, base) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::CellInfoLte, cellConfig) == 112, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::CellInfoLte) == 120, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::CellInfoLte) == 8, "wrong alignment");

struct NrSignalStrength final {
    int32_t ssRsrp __attribute__ ((aligned(4)));
    int32_t ssRsrq __attribute__ ((aligned(4)));
    int32_t ssSinr __attribute__ ((aligned(4)));
    int32_t csiRsrp __attribute__ ((aligned(4)));
    int32_t csiRsrq __attribute__ ((aligned(4)));
    int32_t csiSinr __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::NrSignalStrength, ssRsrp) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::NrSignalStrength, ssRsrq) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::NrSignalStrength, ssSinr) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::NrSignalStrength, csiRsrp) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::NrSignalStrength, csiRsrq) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::NrSignalStrength, csiSinr) == 20, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::NrSignalStrength) == 24, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::NrSignalStrength) == 4, "wrong alignment");

struct CellIdentityNr final {
    ::android::hardware::hidl_string mcc __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string mnc __attribute__ ((aligned(8)));
    uint64_t nci __attribute__ ((aligned(8)));
    uint32_t pci __attribute__ ((aligned(4)));
    int32_t tac __attribute__ ((aligned(4)));
    int32_t nrarfcn __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_2::CellIdentityOperatorNames operatorNames __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::CellIdentityNr, mcc) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::CellIdentityNr, mnc) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::CellIdentityNr, nci) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::CellIdentityNr, pci) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::CellIdentityNr, tac) == 44, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::CellIdentityNr, nrarfcn) == 48, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::CellIdentityNr, operatorNames) == 56, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::CellIdentityNr) == 88, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::CellIdentityNr) == 8, "wrong alignment");

struct CellInfoNr final {
    ::android::hardware::radio::V1_4::NrSignalStrength signalStrength __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_4::CellIdentityNr cellidentity __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::CellInfoNr, signalStrength) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::CellInfoNr, cellidentity) == 24, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::CellInfoNr) == 112, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::CellInfoNr) == 8, "wrong alignment");

/**
 * Overwritten from @1.2::CellInfo in order to update the CellInfoLte to 1.4 version.
 */
struct CellInfo final {
    // Forward declaration for forward reference support:
    struct Info;

    struct Info final {
        enum class hidl_discriminator : uint8_t {
            gsm = 0,  // ::android::hardware::radio::V1_2::CellInfoGsm
            cdma = 1,  // ::android::hardware::radio::V1_2::CellInfoCdma
            wcdma = 2,  // ::android::hardware::radio::V1_2::CellInfoWcdma
            tdscdma = 3,  // ::android::hardware::radio::V1_2::CellInfoTdscdma
            lte = 4,  // ::android::hardware::radio::V1_4::CellInfoLte
            nr = 5,  // ::android::hardware::radio::V1_4::CellInfoNr
        };

        Info();
        ~Info();
        Info(Info&&);
        Info(const Info&);
        Info& operator=(Info&&);
        Info& operator=(const Info&);

        void gsm(const ::android::hardware::radio::V1_2::CellInfoGsm&);
        void gsm(::android::hardware::radio::V1_2::CellInfoGsm&&);
        ::android::hardware::radio::V1_2::CellInfoGsm& gsm();
        const ::android::hardware::radio::V1_2::CellInfoGsm& gsm() const;

        void cdma(const ::android::hardware::radio::V1_2::CellInfoCdma&);
        void cdma(::android::hardware::radio::V1_2::CellInfoCdma&&);
        ::android::hardware::radio::V1_2::CellInfoCdma& cdma();
        const ::android::hardware::radio::V1_2::CellInfoCdma& cdma() const;

        void wcdma(const ::android::hardware::radio::V1_2::CellInfoWcdma&);
        void wcdma(::android::hardware::radio::V1_2::CellInfoWcdma&&);
        ::android::hardware::radio::V1_2::CellInfoWcdma& wcdma();
        const ::android::hardware::radio::V1_2::CellInfoWcdma& wcdma() const;

        void tdscdma(const ::android::hardware::radio::V1_2::CellInfoTdscdma&);
        void tdscdma(::android::hardware::radio::V1_2::CellInfoTdscdma&&);
        ::android::hardware::radio::V1_2::CellInfoTdscdma& tdscdma();
        const ::android::hardware::radio::V1_2::CellInfoTdscdma& tdscdma() const;

        void lte(const ::android::hardware::radio::V1_4::CellInfoLte&);
        void lte(::android::hardware::radio::V1_4::CellInfoLte&&);
        ::android::hardware::radio::V1_4::CellInfoLte& lte();
        const ::android::hardware::radio::V1_4::CellInfoLte& lte() const;

        void nr(const ::android::hardware::radio::V1_4::CellInfoNr&);
        void nr(::android::hardware::radio::V1_4::CellInfoNr&&);
        ::android::hardware::radio::V1_4::CellInfoNr& nr();
        const ::android::hardware::radio::V1_4::CellInfoNr& nr() const;

        // Utility methods
        hidl_discriminator getDiscriminator() const;

        constexpr size_t hidl_getUnionOffset() const {
            return offsetof(::android::hardware::radio::V1_4::CellInfo::Info, hidl_u);
        }

    private:
        void hidl_destructUnion();

        hidl_discriminator hidl_d __attribute__ ((aligned(1))) ;
        union hidl_union final {
            ::android::hardware::radio::V1_2::CellInfoGsm gsm __attribute__ ((aligned(8)));
            ::android::hardware::radio::V1_2::CellInfoCdma cdma __attribute__ ((aligned(8)));
            ::android::hardware::radio::V1_2::CellInfoWcdma wcdma __attribute__ ((aligned(8)));
            ::android::hardware::radio::V1_2::CellInfoTdscdma tdscdma __attribute__ ((aligned(8)));
            ::android::hardware::radio::V1_4::CellInfoLte lte __attribute__ ((aligned(8)));
            ::android::hardware::radio::V1_4::CellInfoNr nr __attribute__ ((aligned(8)));

            hidl_union();
            ~hidl_union();
        } hidl_u;

        static_assert(sizeof(::android::hardware::radio::V1_4::CellInfo::Info::hidl_union) == 120, "wrong size");
        static_assert(__alignof(::android::hardware::radio::V1_4::CellInfo::Info::hidl_union) == 8, "wrong alignment");
        static_assert(sizeof(::android::hardware::radio::V1_4::CellInfo::Info::hidl_discriminator) == 1, "wrong size");
        static_assert(__alignof(::android::hardware::radio::V1_4::CellInfo::Info::hidl_discriminator) == 1, "wrong alignment");
    };

    static_assert(sizeof(::android::hardware::radio::V1_4::CellInfo::Info) == 128, "wrong size");
    static_assert(__alignof(::android::hardware::radio::V1_4::CellInfo::Info) == 8, "wrong alignment");

    bool isRegistered __attribute__ ((aligned(1)));
    ::android::hardware::radio::V1_2::CellConnectionStatus connectionStatus __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_4::CellInfo::Info info __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::CellInfo, isRegistered) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::CellInfo, connectionStatus) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::CellInfo, info) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::CellInfo) == 136, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::CellInfo) == 8, "wrong alignment");

/**
 * Overwritten from @1.2::NetworkScanResult in order to update the CellInfo to 1.4 version.
 */
struct NetworkScanResult final {
    ::android::hardware::radio::V1_1::ScanStatus status __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::RadioError error __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::CellInfo> networkInfos __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::NetworkScanResult, status) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::NetworkScanResult, error) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::NetworkScanResult, networkInfos) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::NetworkScanResult) == 24, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::NetworkScanResult) == 8, "wrong alignment");

/**
 * Overwritten from @1.0::DataProfileInfo in order to deprecate 'mvnoType', and 'mvnoMatchData'.
 * In the future, this must be extended instead of overwritten.
 * Also added 'preferred' and 'persistent' in this version.
 */
struct DataProfileInfo final {
    ::android::hardware::radio::V1_0::DataProfileId profileId __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string apn __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_4::PdpProtocolType protocol __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_4::PdpProtocolType roamingProtocol __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::ApnAuthType authType __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string user __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string password __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::DataProfileInfoType type __attribute__ ((aligned(4)));
    int32_t maxConnsTime __attribute__ ((aligned(4)));
    int32_t maxConns __attribute__ ((aligned(4)));
    int32_t waitTime __attribute__ ((aligned(4)));
    bool enabled __attribute__ ((aligned(1)));
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::ApnTypes> supportedApnTypesBitmap __attribute__ ((aligned(4)));
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::RadioAccessFamily> bearerBitmap __attribute__ ((aligned(4)));
    int32_t mtu __attribute__ ((aligned(4)));
    bool preferred __attribute__ ((aligned(1)));
    bool persistent __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, profileId) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, apn) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, protocol) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, roamingProtocol) == 28, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, authType) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, user) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, password) == 56, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, type) == 72, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, maxConnsTime) == 76, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, maxConns) == 80, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, waitTime) == 84, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, enabled) == 88, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, supportedApnTypesBitmap) == 92, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, bearerBitmap) == 96, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, mtu) == 100, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, preferred) == 104, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::DataProfileInfo, persistent) == 105, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::DataProfileInfo) == 112, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::DataProfileInfo) == 8, "wrong alignment");

struct CardStatus final {
    ::android::hardware::radio::V1_2::CardStatus base __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string eid __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::CardStatus, base) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::CardStatus, eid) == 80, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::CardStatus) == 96, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::CardStatus) == 8, "wrong alignment");

/**
 * Overwritten from @1.0::RadioCapability in order to use the latest RadioAccessFamily.
 */
struct RadioCapability final {
    int32_t session __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::RadioCapabilityPhase phase __attribute__ ((aligned(4)));
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::RadioAccessFamily> raf __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string logicalModemUuid __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::RadioCapabilityStatus status __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::RadioCapability, session) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::RadioCapability, phase) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::RadioCapability, raf) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::RadioCapability, logicalModemUuid) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::RadioCapability, status) == 32, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::RadioCapability) == 40, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::RadioCapability) == 8, "wrong alignment");

/**
 * Overwritten from @1.0::SetupDataCallResult in order to update the DataCallFailCause to 1.4
 * version.
 */
struct SetupDataCallResult final {
    ::android::hardware::radio::V1_4::DataCallFailCause cause __attribute__ ((aligned(4)));
    int32_t suggestedRetryTime __attribute__ ((aligned(4)));
    int32_t cid __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_4::DataConnActiveStatus active __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_4::PdpProtocolType type __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string ifname __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::hidl_string> addresses __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::hidl_string> dnses __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::hidl_string> gateways __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::hidl_string> pcscf __attribute__ ((aligned(8)));
    int32_t mtu __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::SetupDataCallResult, cause) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::SetupDataCallResult, suggestedRetryTime) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::SetupDataCallResult, cid) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::SetupDataCallResult, active) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::SetupDataCallResult, type) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::SetupDataCallResult, ifname) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::SetupDataCallResult, addresses) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::SetupDataCallResult, dnses) == 56, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::SetupDataCallResult, gateways) == 72, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::SetupDataCallResult, pcscf) == 88, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::SetupDataCallResult, mtu) == 104, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::SetupDataCallResult) == 112, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::SetupDataCallResult) == 8, "wrong alignment");

enum class SimLockMultiSimPolicy : int32_t {
    /**
     * Indicates that configuration applies to each slot independently.
     */
    NO_MULTISIM_POLICY = 0,
    /**
     * Indicates that any SIM card can be used as far as one valid card is present in the device.
     * For the modem, a SIM card is valid when its content (i.e. MCC, MNC, GID, SPN) matches the
     * carrier restriction configuration.
     */
    ONE_VALID_SIM_MUST_BE_PRESENT = 1,
};

struct CarrierRestrictionsWithPriority final {
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::Carrier> allowedCarriers __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::Carrier> excludedCarriers __attribute__ ((aligned(8)));
    bool allowedCarriersPrioritized __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority, allowedCarriers) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority, excludedCarriers) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority, allowedCarriersPrioritized) == 32, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority) == 40, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority) == 8, "wrong alignment");

/**
 * Overwritten from @1.2::SignalStrength in order to add signal strength for NR.
 */
struct SignalStrength final {
    ::android::hardware::radio::V1_0::GsmSignalStrength gsm __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::CdmaSignalStrength cdma __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::EvdoSignalStrength evdo __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::LteSignalStrength lte __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_2::TdscdmaSignalStrength tdscdma __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_2::WcdmaSignalStrength wcdma __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_4::NrSignalStrength nr __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_4::SignalStrength, gsm) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::SignalStrength, cdma) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::SignalStrength, evdo) == 20, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::SignalStrength, lte) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::SignalStrength, tdscdma) == 56, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::SignalStrength, wcdma) == 68, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_4::SignalStrength, nr) == 84, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_4::SignalStrength) == 108, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_4::SignalStrength) == 4, "wrong alignment");

//
// type declarations for package
//

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::android::hardware::radio::V1_4::AccessNetwork o);

constexpr int32_t operator|(const ::android::hardware::radio::V1_4::AccessNetwork lhs, const ::android::hardware::radio::V1_4::AccessNetwork rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_4::AccessNetwork rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::android::hardware::radio::V1_4::AccessNetwork lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::AccessNetwork lhs, const ::android::hardware::radio::V1_4::AccessNetwork rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_4::AccessNetwork rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::AccessNetwork lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_4::AccessNetwork e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_4::AccessNetwork e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::android::hardware::radio::V1_4::ApnTypes o);

constexpr int32_t operator|(const ::android::hardware::radio::V1_4::ApnTypes lhs, const ::android::hardware::radio::V1_4::ApnTypes rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_4::ApnTypes rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::android::hardware::radio::V1_4::ApnTypes lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::ApnTypes lhs, const ::android::hardware::radio::V1_4::ApnTypes rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_4::ApnTypes rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::ApnTypes lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_4::ApnTypes e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_4::ApnTypes e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::android::hardware::radio::V1_4::EmergencyServiceCategory o);

constexpr int32_t operator|(const ::android::hardware::radio::V1_4::EmergencyServiceCategory lhs, const ::android::hardware::radio::V1_4::EmergencyServiceCategory rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_4::EmergencyServiceCategory rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::android::hardware::radio::V1_4::EmergencyServiceCategory lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::EmergencyServiceCategory lhs, const ::android::hardware::radio::V1_4::EmergencyServiceCategory rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_4::EmergencyServiceCategory rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::EmergencyServiceCategory lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_4::EmergencyServiceCategory e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_4::EmergencyServiceCategory e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::android::hardware::radio::V1_4::EmergencyNumberSource o);

constexpr int32_t operator|(const ::android::hardware::radio::V1_4::EmergencyNumberSource lhs, const ::android::hardware::radio::V1_4::EmergencyNumberSource rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_4::EmergencyNumberSource rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::android::hardware::radio::V1_4::EmergencyNumberSource lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::EmergencyNumberSource lhs, const ::android::hardware::radio::V1_4::EmergencyNumberSource rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_4::EmergencyNumberSource rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::EmergencyNumberSource lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_4::EmergencyNumberSource e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_4::EmergencyNumberSource e) {
    v &= static_cast<int32_t>(e);
    return v;
}

static inline std::string toString(const ::android::hardware::radio::V1_4::EmergencyNumber& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::EmergencyNumber& lhs, const ::android::hardware::radio::V1_4::EmergencyNumber& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::EmergencyNumber& lhs, const ::android::hardware::radio::V1_4::EmergencyNumber& rhs);

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::android::hardware::radio::V1_4::EmergencyCallRouting o);

constexpr int32_t operator|(const ::android::hardware::radio::V1_4::EmergencyCallRouting lhs, const ::android::hardware::radio::V1_4::EmergencyCallRouting rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_4::EmergencyCallRouting rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::android::hardware::radio::V1_4::EmergencyCallRouting lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::EmergencyCallRouting lhs, const ::android::hardware::radio::V1_4::EmergencyCallRouting rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_4::EmergencyCallRouting rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::EmergencyCallRouting lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_4::EmergencyCallRouting e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_4::EmergencyCallRouting e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::android::hardware::radio::V1_4::RadioTechnology o);

constexpr int32_t operator|(const ::android::hardware::radio::V1_4::RadioTechnology lhs, const ::android::hardware::radio::V1_4::RadioTechnology rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_4::RadioTechnology rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::android::hardware::radio::V1_4::RadioTechnology lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::RadioTechnology lhs, const ::android::hardware::radio::V1_4::RadioTechnology rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_4::RadioTechnology rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::RadioTechnology lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_4::RadioTechnology e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_4::RadioTechnology e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::android::hardware::radio::V1_4::RadioAccessFamily o);

constexpr int32_t operator|(const ::android::hardware::radio::V1_4::RadioAccessFamily lhs, const ::android::hardware::radio::V1_4::RadioAccessFamily rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_4::RadioAccessFamily rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::android::hardware::radio::V1_4::RadioAccessFamily lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::RadioAccessFamily lhs, const ::android::hardware::radio::V1_4::RadioAccessFamily rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_4::RadioAccessFamily rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::RadioAccessFamily lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_4::RadioAccessFamily e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_4::RadioAccessFamily e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::android::hardware::radio::V1_4::FrequencyRange o);

constexpr int32_t operator|(const ::android::hardware::radio::V1_4::FrequencyRange lhs, const ::android::hardware::radio::V1_4::FrequencyRange rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_4::FrequencyRange rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::android::hardware::radio::V1_4::FrequencyRange lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::FrequencyRange lhs, const ::android::hardware::radio::V1_4::FrequencyRange rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_4::FrequencyRange rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::FrequencyRange lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_4::FrequencyRange e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_4::FrequencyRange e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::android::hardware::radio::V1_4::DataCallFailCause o);

constexpr int32_t operator|(const ::android::hardware::radio::V1_4::DataCallFailCause lhs, const ::android::hardware::radio::V1_4::DataCallFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_4::DataCallFailCause rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::android::hardware::radio::V1_4::DataCallFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::DataCallFailCause lhs, const ::android::hardware::radio::V1_4::DataCallFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_4::DataCallFailCause rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::DataCallFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_4::DataCallFailCause e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_4::DataCallFailCause e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::android::hardware::radio::V1_4::DataConnActiveStatus o);

constexpr int32_t operator|(const ::android::hardware::radio::V1_4::DataConnActiveStatus lhs, const ::android::hardware::radio::V1_4::DataConnActiveStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_4::DataConnActiveStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::android::hardware::radio::V1_4::DataConnActiveStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::DataConnActiveStatus lhs, const ::android::hardware::radio::V1_4::DataConnActiveStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_4::DataConnActiveStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::DataConnActiveStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_4::DataConnActiveStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_4::DataConnActiveStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::android::hardware::radio::V1_4::PdpProtocolType o);

constexpr int32_t operator|(const ::android::hardware::radio::V1_4::PdpProtocolType lhs, const ::android::hardware::radio::V1_4::PdpProtocolType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_4::PdpProtocolType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::android::hardware::radio::V1_4::PdpProtocolType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::PdpProtocolType lhs, const ::android::hardware::radio::V1_4::PdpProtocolType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_4::PdpProtocolType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::PdpProtocolType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_4::PdpProtocolType e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_4::PdpProtocolType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

static inline std::string toString(const ::android::hardware::radio::V1_4::RadioFrequencyInfo& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::RadioFrequencyInfo& lhs, const ::android::hardware::radio::V1_4::RadioFrequencyInfo& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::RadioFrequencyInfo& lhs, const ::android::hardware::radio::V1_4::RadioFrequencyInfo& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::PhysicalChannelConfig& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::PhysicalChannelConfig& lhs, const ::android::hardware::radio::V1_4::PhysicalChannelConfig& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::PhysicalChannelConfig& lhs, const ::android::hardware::radio::V1_4::PhysicalChannelConfig& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::LteVopsInfo& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::LteVopsInfo& lhs, const ::android::hardware::radio::V1_4::LteVopsInfo& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::LteVopsInfo& lhs, const ::android::hardware::radio::V1_4::LteVopsInfo& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::NrIndicators& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::NrIndicators& lhs, const ::android::hardware::radio::V1_4::NrIndicators& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::NrIndicators& lhs, const ::android::hardware::radio::V1_4::NrIndicators& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo& lhs, const ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo& lhs, const ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::DataRegStateResult& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::DataRegStateResult& lhs, const ::android::hardware::radio::V1_4::DataRegStateResult& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::DataRegStateResult& lhs, const ::android::hardware::radio::V1_4::DataRegStateResult& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::CellConfigLte& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::CellConfigLte& lhs, const ::android::hardware::radio::V1_4::CellConfigLte& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::CellConfigLte& lhs, const ::android::hardware::radio::V1_4::CellConfigLte& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::CellInfoLte& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::CellInfoLte& lhs, const ::android::hardware::radio::V1_4::CellInfoLte& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::CellInfoLte& lhs, const ::android::hardware::radio::V1_4::CellInfoLte& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::NrSignalStrength& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::NrSignalStrength& lhs, const ::android::hardware::radio::V1_4::NrSignalStrength& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::NrSignalStrength& lhs, const ::android::hardware::radio::V1_4::NrSignalStrength& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::CellIdentityNr& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::CellIdentityNr& lhs, const ::android::hardware::radio::V1_4::CellIdentityNr& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::CellIdentityNr& lhs, const ::android::hardware::radio::V1_4::CellIdentityNr& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::CellInfoNr& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::CellInfoNr& lhs, const ::android::hardware::radio::V1_4::CellInfoNr& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::CellInfoNr& lhs, const ::android::hardware::radio::V1_4::CellInfoNr& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::CellInfo::Info& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::CellInfo::Info& lhs, const ::android::hardware::radio::V1_4::CellInfo::Info& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::CellInfo::Info& lhs, const ::android::hardware::radio::V1_4::CellInfo::Info& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::CellInfo& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::CellInfo& lhs, const ::android::hardware::radio::V1_4::CellInfo& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::CellInfo& lhs, const ::android::hardware::radio::V1_4::CellInfo& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::NetworkScanResult& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::NetworkScanResult& lhs, const ::android::hardware::radio::V1_4::NetworkScanResult& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::NetworkScanResult& lhs, const ::android::hardware::radio::V1_4::NetworkScanResult& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::DataProfileInfo& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::DataProfileInfo& lhs, const ::android::hardware::radio::V1_4::DataProfileInfo& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::DataProfileInfo& lhs, const ::android::hardware::radio::V1_4::DataProfileInfo& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::CardStatus& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::CardStatus& lhs, const ::android::hardware::radio::V1_4::CardStatus& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::CardStatus& lhs, const ::android::hardware::radio::V1_4::CardStatus& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::RadioCapability& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::RadioCapability& lhs, const ::android::hardware::radio::V1_4::RadioCapability& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::RadioCapability& lhs, const ::android::hardware::radio::V1_4::RadioCapability& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::SetupDataCallResult& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::SetupDataCallResult& lhs, const ::android::hardware::radio::V1_4::SetupDataCallResult& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::SetupDataCallResult& lhs, const ::android::hardware::radio::V1_4::SetupDataCallResult& rhs);

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::android::hardware::radio::V1_4::SimLockMultiSimPolicy o);

constexpr int32_t operator|(const ::android::hardware::radio::V1_4::SimLockMultiSimPolicy lhs, const ::android::hardware::radio::V1_4::SimLockMultiSimPolicy rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_4::SimLockMultiSimPolicy rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::android::hardware::radio::V1_4::SimLockMultiSimPolicy lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::SimLockMultiSimPolicy lhs, const ::android::hardware::radio::V1_4::SimLockMultiSimPolicy rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_4::SimLockMultiSimPolicy rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::android::hardware::radio::V1_4::SimLockMultiSimPolicy lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_4::SimLockMultiSimPolicy e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_4::SimLockMultiSimPolicy e) {
    v &= static_cast<int32_t>(e);
    return v;
}

static inline std::string toString(const ::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority& lhs, const ::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority& lhs, const ::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority& rhs);

static inline std::string toString(const ::android::hardware::radio::V1_4::SignalStrength& o);
static inline bool operator==(const ::android::hardware::radio::V1_4::SignalStrength& lhs, const ::android::hardware::radio::V1_4::SignalStrength& rhs);
static inline bool operator!=(const ::android::hardware::radio::V1_4::SignalStrength& lhs, const ::android::hardware::radio::V1_4::SignalStrength& rhs);

//
// type header definitions for package
//

template<>
inline std::string toString<::android::hardware::radio::V1_4::AccessNetwork>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::AccessNetwork> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_4::AccessNetwork::GERAN) == static_cast<int32_t>(::android::hardware::radio::V1_4::AccessNetwork::GERAN)) {
        os += (first ? "" : " | ");
        os += "GERAN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::AccessNetwork::GERAN;
    }
    if ((o & ::android::hardware::radio::V1_4::AccessNetwork::UTRAN) == static_cast<int32_t>(::android::hardware::radio::V1_4::AccessNetwork::UTRAN)) {
        os += (first ? "" : " | ");
        os += "UTRAN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::AccessNetwork::UTRAN;
    }
    if ((o & ::android::hardware::radio::V1_4::AccessNetwork::EUTRAN) == static_cast<int32_t>(::android::hardware::radio::V1_4::AccessNetwork::EUTRAN)) {
        os += (first ? "" : " | ");
        os += "EUTRAN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::AccessNetwork::EUTRAN;
    }
    if ((o & ::android::hardware::radio::V1_4::AccessNetwork::CDMA2000) == static_cast<int32_t>(::android::hardware::radio::V1_4::AccessNetwork::CDMA2000)) {
        os += (first ? "" : " | ");
        os += "CDMA2000";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::AccessNetwork::CDMA2000;
    }
    if ((o & ::android::hardware::radio::V1_4::AccessNetwork::IWLAN) == static_cast<int32_t>(::android::hardware::radio::V1_4::AccessNetwork::IWLAN)) {
        os += (first ? "" : " | ");
        os += "IWLAN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::AccessNetwork::IWLAN;
    }
    if ((o & ::android::hardware::radio::V1_4::AccessNetwork::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_4::AccessNetwork::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::AccessNetwork::UNKNOWN;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_4::AccessNetwork o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_4::AccessNetwork::GERAN) {
        return "GERAN";
    }
    if (o == ::android::hardware::radio::V1_4::AccessNetwork::UTRAN) {
        return "UTRAN";
    }
    if (o == ::android::hardware::radio::V1_4::AccessNetwork::EUTRAN) {
        return "EUTRAN";
    }
    if (o == ::android::hardware::radio::V1_4::AccessNetwork::CDMA2000) {
        return "CDMA2000";
    }
    if (o == ::android::hardware::radio::V1_4::AccessNetwork::IWLAN) {
        return "IWLAN";
    }
    if (o == ::android::hardware::radio::V1_4::AccessNetwork::UNKNOWN) {
        return "UNKNOWN";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::android::hardware::radio::V1_4::ApnTypes>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::ApnTypes> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_4::ApnTypes::NONE) == static_cast<int32_t>(::android::hardware::radio::V1_4::ApnTypes::NONE)) {
        os += (first ? "" : " | ");
        os += "NONE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::ApnTypes::NONE;
    }
    if ((o & ::android::hardware::radio::V1_4::ApnTypes::DEFAULT) == static_cast<int32_t>(::android::hardware::radio::V1_4::ApnTypes::DEFAULT)) {
        os += (first ? "" : " | ");
        os += "DEFAULT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::ApnTypes::DEFAULT;
    }
    if ((o & ::android::hardware::radio::V1_4::ApnTypes::MMS) == static_cast<int32_t>(::android::hardware::radio::V1_4::ApnTypes::MMS)) {
        os += (first ? "" : " | ");
        os += "MMS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::ApnTypes::MMS;
    }
    if ((o & ::android::hardware::radio::V1_4::ApnTypes::SUPL) == static_cast<int32_t>(::android::hardware::radio::V1_4::ApnTypes::SUPL)) {
        os += (first ? "" : " | ");
        os += "SUPL";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::ApnTypes::SUPL;
    }
    if ((o & ::android::hardware::radio::V1_4::ApnTypes::DUN) == static_cast<int32_t>(::android::hardware::radio::V1_4::ApnTypes::DUN)) {
        os += (first ? "" : " | ");
        os += "DUN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::ApnTypes::DUN;
    }
    if ((o & ::android::hardware::radio::V1_4::ApnTypes::HIPRI) == static_cast<int32_t>(::android::hardware::radio::V1_4::ApnTypes::HIPRI)) {
        os += (first ? "" : " | ");
        os += "HIPRI";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::ApnTypes::HIPRI;
    }
    if ((o & ::android::hardware::radio::V1_4::ApnTypes::FOTA) == static_cast<int32_t>(::android::hardware::radio::V1_4::ApnTypes::FOTA)) {
        os += (first ? "" : " | ");
        os += "FOTA";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::ApnTypes::FOTA;
    }
    if ((o & ::android::hardware::radio::V1_4::ApnTypes::IMS) == static_cast<int32_t>(::android::hardware::radio::V1_4::ApnTypes::IMS)) {
        os += (first ? "" : " | ");
        os += "IMS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::ApnTypes::IMS;
    }
    if ((o & ::android::hardware::radio::V1_4::ApnTypes::CBS) == static_cast<int32_t>(::android::hardware::radio::V1_4::ApnTypes::CBS)) {
        os += (first ? "" : " | ");
        os += "CBS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::ApnTypes::CBS;
    }
    if ((o & ::android::hardware::radio::V1_4::ApnTypes::IA) == static_cast<int32_t>(::android::hardware::radio::V1_4::ApnTypes::IA)) {
        os += (first ? "" : " | ");
        os += "IA";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::ApnTypes::IA;
    }
    if ((o & ::android::hardware::radio::V1_4::ApnTypes::EMERGENCY) == static_cast<int32_t>(::android::hardware::radio::V1_4::ApnTypes::EMERGENCY)) {
        os += (first ? "" : " | ");
        os += "EMERGENCY";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::ApnTypes::EMERGENCY;
    }
    if ((o & ::android::hardware::radio::V1_4::ApnTypes::ALL) == static_cast<int32_t>(::android::hardware::radio::V1_4::ApnTypes::ALL)) {
        os += (first ? "" : " | ");
        os += "ALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::ApnTypes::ALL;
    }
    if ((o & ::android::hardware::radio::V1_4::ApnTypes::MCX) == static_cast<int32_t>(::android::hardware::radio::V1_4::ApnTypes::MCX)) {
        os += (first ? "" : " | ");
        os += "MCX";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::ApnTypes::MCX;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_4::ApnTypes o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_4::ApnTypes::NONE) {
        return "NONE";
    }
    if (o == ::android::hardware::radio::V1_4::ApnTypes::DEFAULT) {
        return "DEFAULT";
    }
    if (o == ::android::hardware::radio::V1_4::ApnTypes::MMS) {
        return "MMS";
    }
    if (o == ::android::hardware::radio::V1_4::ApnTypes::SUPL) {
        return "SUPL";
    }
    if (o == ::android::hardware::radio::V1_4::ApnTypes::DUN) {
        return "DUN";
    }
    if (o == ::android::hardware::radio::V1_4::ApnTypes::HIPRI) {
        return "HIPRI";
    }
    if (o == ::android::hardware::radio::V1_4::ApnTypes::FOTA) {
        return "FOTA";
    }
    if (o == ::android::hardware::radio::V1_4::ApnTypes::IMS) {
        return "IMS";
    }
    if (o == ::android::hardware::radio::V1_4::ApnTypes::CBS) {
        return "CBS";
    }
    if (o == ::android::hardware::radio::V1_4::ApnTypes::IA) {
        return "IA";
    }
    if (o == ::android::hardware::radio::V1_4::ApnTypes::EMERGENCY) {
        return "EMERGENCY";
    }
    if (o == ::android::hardware::radio::V1_4::ApnTypes::ALL) {
        return "ALL";
    }
    if (o == ::android::hardware::radio::V1_4::ApnTypes::MCX) {
        return "MCX";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::android::hardware::radio::V1_4::EmergencyServiceCategory>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::EmergencyServiceCategory> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_4::EmergencyServiceCategory::UNSPECIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_4::EmergencyServiceCategory::UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "UNSPECIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::EmergencyServiceCategory::UNSPECIFIED;
    }
    if ((o & ::android::hardware::radio::V1_4::EmergencyServiceCategory::POLICE) == static_cast<int32_t>(::android::hardware::radio::V1_4::EmergencyServiceCategory::POLICE)) {
        os += (first ? "" : " | ");
        os += "POLICE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::EmergencyServiceCategory::POLICE;
    }
    if ((o & ::android::hardware::radio::V1_4::EmergencyServiceCategory::AMBULANCE) == static_cast<int32_t>(::android::hardware::radio::V1_4::EmergencyServiceCategory::AMBULANCE)) {
        os += (first ? "" : " | ");
        os += "AMBULANCE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::EmergencyServiceCategory::AMBULANCE;
    }
    if ((o & ::android::hardware::radio::V1_4::EmergencyServiceCategory::FIRE_BRIGADE) == static_cast<int32_t>(::android::hardware::radio::V1_4::EmergencyServiceCategory::FIRE_BRIGADE)) {
        os += (first ? "" : " | ");
        os += "FIRE_BRIGADE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::EmergencyServiceCategory::FIRE_BRIGADE;
    }
    if ((o & ::android::hardware::radio::V1_4::EmergencyServiceCategory::MARINE_GUARD) == static_cast<int32_t>(::android::hardware::radio::V1_4::EmergencyServiceCategory::MARINE_GUARD)) {
        os += (first ? "" : " | ");
        os += "MARINE_GUARD";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::EmergencyServiceCategory::MARINE_GUARD;
    }
    if ((o & ::android::hardware::radio::V1_4::EmergencyServiceCategory::MOUNTAIN_RESCUE) == static_cast<int32_t>(::android::hardware::radio::V1_4::EmergencyServiceCategory::MOUNTAIN_RESCUE)) {
        os += (first ? "" : " | ");
        os += "MOUNTAIN_RESCUE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::EmergencyServiceCategory::MOUNTAIN_RESCUE;
    }
    if ((o & ::android::hardware::radio::V1_4::EmergencyServiceCategory::MIEC) == static_cast<int32_t>(::android::hardware::radio::V1_4::EmergencyServiceCategory::MIEC)) {
        os += (first ? "" : " | ");
        os += "MIEC";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::EmergencyServiceCategory::MIEC;
    }
    if ((o & ::android::hardware::radio::V1_4::EmergencyServiceCategory::AIEC) == static_cast<int32_t>(::android::hardware::radio::V1_4::EmergencyServiceCategory::AIEC)) {
        os += (first ? "" : " | ");
        os += "AIEC";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::EmergencyServiceCategory::AIEC;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_4::EmergencyServiceCategory o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_4::EmergencyServiceCategory::UNSPECIFIED) {
        return "UNSPECIFIED";
    }
    if (o == ::android::hardware::radio::V1_4::EmergencyServiceCategory::POLICE) {
        return "POLICE";
    }
    if (o == ::android::hardware::radio::V1_4::EmergencyServiceCategory::AMBULANCE) {
        return "AMBULANCE";
    }
    if (o == ::android::hardware::radio::V1_4::EmergencyServiceCategory::FIRE_BRIGADE) {
        return "FIRE_BRIGADE";
    }
    if (o == ::android::hardware::radio::V1_4::EmergencyServiceCategory::MARINE_GUARD) {
        return "MARINE_GUARD";
    }
    if (o == ::android::hardware::radio::V1_4::EmergencyServiceCategory::MOUNTAIN_RESCUE) {
        return "MOUNTAIN_RESCUE";
    }
    if (o == ::android::hardware::radio::V1_4::EmergencyServiceCategory::MIEC) {
        return "MIEC";
    }
    if (o == ::android::hardware::radio::V1_4::EmergencyServiceCategory::AIEC) {
        return "AIEC";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::android::hardware::radio::V1_4::EmergencyNumberSource>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::EmergencyNumberSource> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_4::EmergencyNumberSource::NETWORK_SIGNALING) == static_cast<int32_t>(::android::hardware::radio::V1_4::EmergencyNumberSource::NETWORK_SIGNALING)) {
        os += (first ? "" : " | ");
        os += "NETWORK_SIGNALING";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::EmergencyNumberSource::NETWORK_SIGNALING;
    }
    if ((o & ::android::hardware::radio::V1_4::EmergencyNumberSource::SIM) == static_cast<int32_t>(::android::hardware::radio::V1_4::EmergencyNumberSource::SIM)) {
        os += (first ? "" : " | ");
        os += "SIM";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::EmergencyNumberSource::SIM;
    }
    if ((o & ::android::hardware::radio::V1_4::EmergencyNumberSource::MODEM_CONFIG) == static_cast<int32_t>(::android::hardware::radio::V1_4::EmergencyNumberSource::MODEM_CONFIG)) {
        os += (first ? "" : " | ");
        os += "MODEM_CONFIG";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::EmergencyNumberSource::MODEM_CONFIG;
    }
    if ((o & ::android::hardware::radio::V1_4::EmergencyNumberSource::DEFAULT) == static_cast<int32_t>(::android::hardware::radio::V1_4::EmergencyNumberSource::DEFAULT)) {
        os += (first ? "" : " | ");
        os += "DEFAULT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::EmergencyNumberSource::DEFAULT;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_4::EmergencyNumberSource o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_4::EmergencyNumberSource::NETWORK_SIGNALING) {
        return "NETWORK_SIGNALING";
    }
    if (o == ::android::hardware::radio::V1_4::EmergencyNumberSource::SIM) {
        return "SIM";
    }
    if (o == ::android::hardware::radio::V1_4::EmergencyNumberSource::MODEM_CONFIG) {
        return "MODEM_CONFIG";
    }
    if (o == ::android::hardware::radio::V1_4::EmergencyNumberSource::DEFAULT) {
        return "DEFAULT";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::android::hardware::radio::V1_4::EmergencyNumber& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".number = ";
    os += ::android::hardware::toString(o.number);
    os += ", .mcc = ";
    os += ::android::hardware::toString(o.mcc);
    os += ", .mnc = ";
    os += ::android::hardware::toString(o.mnc);
    os += ", .categories = ";
    os += ::android::hardware::radio::V1_4::toString<::android::hardware::radio::V1_4::EmergencyServiceCategory>(o.categories);
    os += ", .urns = ";
    os += ::android::hardware::toString(o.urns);
    os += ", .sources = ";
    os += ::android::hardware::radio::V1_4::toString<::android::hardware::radio::V1_4::EmergencyNumberSource>(o.sources);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::EmergencyNumber& lhs, const ::android::hardware::radio::V1_4::EmergencyNumber& rhs) {
    if (lhs.number != rhs.number) {
        return false;
    }
    if (lhs.mcc != rhs.mcc) {
        return false;
    }
    if (lhs.mnc != rhs.mnc) {
        return false;
    }
    if (lhs.categories != rhs.categories) {
        return false;
    }
    if (lhs.urns != rhs.urns) {
        return false;
    }
    if (lhs.sources != rhs.sources) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::EmergencyNumber& lhs, const ::android::hardware::radio::V1_4::EmergencyNumber& rhs){
    return !(lhs == rhs);
}

template<>
inline std::string toString<::android::hardware::radio::V1_4::EmergencyCallRouting>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::EmergencyCallRouting> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_4::EmergencyCallRouting::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_4::EmergencyCallRouting::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::EmergencyCallRouting::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_4::EmergencyCallRouting::EMERGENCY) == static_cast<int32_t>(::android::hardware::radio::V1_4::EmergencyCallRouting::EMERGENCY)) {
        os += (first ? "" : " | ");
        os += "EMERGENCY";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::EmergencyCallRouting::EMERGENCY;
    }
    if ((o & ::android::hardware::radio::V1_4::EmergencyCallRouting::NORMAL) == static_cast<int32_t>(::android::hardware::radio::V1_4::EmergencyCallRouting::NORMAL)) {
        os += (first ? "" : " | ");
        os += "NORMAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::EmergencyCallRouting::NORMAL;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_4::EmergencyCallRouting o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_4::EmergencyCallRouting::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_4::EmergencyCallRouting::EMERGENCY) {
        return "EMERGENCY";
    }
    if (o == ::android::hardware::radio::V1_4::EmergencyCallRouting::NORMAL) {
        return "NORMAL";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::android::hardware::radio::V1_4::RadioTechnology>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::RadioTechnology> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::GPRS) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::GPRS)) {
        os += (first ? "" : " | ");
        os += "GPRS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::GPRS;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::EDGE) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::EDGE)) {
        os += (first ? "" : " | ");
        os += "EDGE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::EDGE;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::UMTS) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::UMTS)) {
        os += (first ? "" : " | ");
        os += "UMTS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::UMTS;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::IS95A) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::IS95A)) {
        os += (first ? "" : " | ");
        os += "IS95A";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::IS95A;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::IS95B) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::IS95B)) {
        os += (first ? "" : " | ");
        os += "IS95B";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::IS95B;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::ONE_X_RTT) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::ONE_X_RTT)) {
        os += (first ? "" : " | ");
        os += "ONE_X_RTT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::ONE_X_RTT;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::EVDO_0) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::EVDO_0)) {
        os += (first ? "" : " | ");
        os += "EVDO_0";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::EVDO_0;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::EVDO_A) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::EVDO_A)) {
        os += (first ? "" : " | ");
        os += "EVDO_A";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::EVDO_A;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::HSDPA) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::HSDPA)) {
        os += (first ? "" : " | ");
        os += "HSDPA";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::HSDPA;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::HSUPA) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::HSUPA)) {
        os += (first ? "" : " | ");
        os += "HSUPA";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::HSUPA;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::HSPA) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::HSPA)) {
        os += (first ? "" : " | ");
        os += "HSPA";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::HSPA;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::EVDO_B) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::EVDO_B)) {
        os += (first ? "" : " | ");
        os += "EVDO_B";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::EVDO_B;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::EHRPD) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::EHRPD)) {
        os += (first ? "" : " | ");
        os += "EHRPD";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::EHRPD;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::LTE) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::LTE)) {
        os += (first ? "" : " | ");
        os += "LTE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::LTE;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::HSPAP) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::HSPAP)) {
        os += (first ? "" : " | ");
        os += "HSPAP";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::HSPAP;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::GSM) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::GSM)) {
        os += (first ? "" : " | ");
        os += "GSM";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::GSM;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::TD_SCDMA) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::TD_SCDMA)) {
        os += (first ? "" : " | ");
        os += "TD_SCDMA";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::TD_SCDMA;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::IWLAN) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::IWLAN)) {
        os += (first ? "" : " | ");
        os += "IWLAN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::IWLAN;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::LTE_CA) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::LTE_CA)) {
        os += (first ? "" : " | ");
        os += "LTE_CA";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::LTE_CA;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioTechnology::NR) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioTechnology::NR)) {
        os += (first ? "" : " | ");
        os += "NR";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioTechnology::NR;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_4::RadioTechnology o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::GPRS) {
        return "GPRS";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::EDGE) {
        return "EDGE";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::UMTS) {
        return "UMTS";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::IS95A) {
        return "IS95A";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::IS95B) {
        return "IS95B";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::ONE_X_RTT) {
        return "ONE_X_RTT";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::EVDO_0) {
        return "EVDO_0";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::EVDO_A) {
        return "EVDO_A";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::HSDPA) {
        return "HSDPA";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::HSUPA) {
        return "HSUPA";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::HSPA) {
        return "HSPA";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::EVDO_B) {
        return "EVDO_B";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::EHRPD) {
        return "EHRPD";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::LTE) {
        return "LTE";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::HSPAP) {
        return "HSPAP";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::GSM) {
        return "GSM";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::TD_SCDMA) {
        return "TD_SCDMA";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::IWLAN) {
        return "IWLAN";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::LTE_CA) {
        return "LTE_CA";
    }
    if (o == ::android::hardware::radio::V1_4::RadioTechnology::NR) {
        return "NR";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::android::hardware::radio::V1_4::RadioAccessFamily>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::RadioAccessFamily> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::GPRS) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::GPRS)) {
        os += (first ? "" : " | ");
        os += "GPRS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::GPRS;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::EDGE) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::EDGE)) {
        os += (first ? "" : " | ");
        os += "EDGE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::EDGE;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::UMTS) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::UMTS)) {
        os += (first ? "" : " | ");
        os += "UMTS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::UMTS;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::IS95A) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::IS95A)) {
        os += (first ? "" : " | ");
        os += "IS95A";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::IS95A;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::IS95B) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::IS95B)) {
        os += (first ? "" : " | ");
        os += "IS95B";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::IS95B;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::ONE_X_RTT) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::ONE_X_RTT)) {
        os += (first ? "" : " | ");
        os += "ONE_X_RTT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::ONE_X_RTT;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::EVDO_0) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::EVDO_0)) {
        os += (first ? "" : " | ");
        os += "EVDO_0";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::EVDO_0;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::EVDO_A) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::EVDO_A)) {
        os += (first ? "" : " | ");
        os += "EVDO_A";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::EVDO_A;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::HSDPA) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::HSDPA)) {
        os += (first ? "" : " | ");
        os += "HSDPA";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::HSDPA;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::HSUPA) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::HSUPA)) {
        os += (first ? "" : " | ");
        os += "HSUPA";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::HSUPA;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::HSPA) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::HSPA)) {
        os += (first ? "" : " | ");
        os += "HSPA";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::HSPA;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::EVDO_B) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::EVDO_B)) {
        os += (first ? "" : " | ");
        os += "EVDO_B";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::EVDO_B;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::EHRPD) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::EHRPD)) {
        os += (first ? "" : " | ");
        os += "EHRPD";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::EHRPD;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::LTE) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::LTE)) {
        os += (first ? "" : " | ");
        os += "LTE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::LTE;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::HSPAP) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::HSPAP)) {
        os += (first ? "" : " | ");
        os += "HSPAP";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::HSPAP;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::GSM) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::GSM)) {
        os += (first ? "" : " | ");
        os += "GSM";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::GSM;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::TD_SCDMA) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::TD_SCDMA)) {
        os += (first ? "" : " | ");
        os += "TD_SCDMA";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::TD_SCDMA;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::LTE_CA) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::LTE_CA)) {
        os += (first ? "" : " | ");
        os += "LTE_CA";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::LTE_CA;
    }
    if ((o & ::android::hardware::radio::V1_4::RadioAccessFamily::NR) == static_cast<int32_t>(::android::hardware::radio::V1_4::RadioAccessFamily::NR)) {
        os += (first ? "" : " | ");
        os += "NR";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::RadioAccessFamily::NR;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_4::RadioAccessFamily o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::GPRS) {
        return "GPRS";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::EDGE) {
        return "EDGE";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::UMTS) {
        return "UMTS";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::IS95A) {
        return "IS95A";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::IS95B) {
        return "IS95B";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::ONE_X_RTT) {
        return "ONE_X_RTT";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::EVDO_0) {
        return "EVDO_0";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::EVDO_A) {
        return "EVDO_A";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::HSDPA) {
        return "HSDPA";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::HSUPA) {
        return "HSUPA";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::HSPA) {
        return "HSPA";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::EVDO_B) {
        return "EVDO_B";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::EHRPD) {
        return "EHRPD";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::LTE) {
        return "LTE";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::HSPAP) {
        return "HSPAP";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::GSM) {
        return "GSM";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::TD_SCDMA) {
        return "TD_SCDMA";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::LTE_CA) {
        return "LTE_CA";
    }
    if (o == ::android::hardware::radio::V1_4::RadioAccessFamily::NR) {
        return "NR";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::android::hardware::radio::V1_4::FrequencyRange>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::FrequencyRange> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_4::FrequencyRange::LOW) == static_cast<int32_t>(::android::hardware::radio::V1_4::FrequencyRange::LOW)) {
        os += (first ? "" : " | ");
        os += "LOW";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::FrequencyRange::LOW;
    }
    if ((o & ::android::hardware::radio::V1_4::FrequencyRange::MID) == static_cast<int32_t>(::android::hardware::radio::V1_4::FrequencyRange::MID)) {
        os += (first ? "" : " | ");
        os += "MID";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::FrequencyRange::MID;
    }
    if ((o & ::android::hardware::radio::V1_4::FrequencyRange::HIGH) == static_cast<int32_t>(::android::hardware::radio::V1_4::FrequencyRange::HIGH)) {
        os += (first ? "" : " | ");
        os += "HIGH";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::FrequencyRange::HIGH;
    }
    if ((o & ::android::hardware::radio::V1_4::FrequencyRange::MMWAVE) == static_cast<int32_t>(::android::hardware::radio::V1_4::FrequencyRange::MMWAVE)) {
        os += (first ? "" : " | ");
        os += "MMWAVE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::FrequencyRange::MMWAVE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_4::FrequencyRange o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_4::FrequencyRange::LOW) {
        return "LOW";
    }
    if (o == ::android::hardware::radio::V1_4::FrequencyRange::MID) {
        return "MID";
    }
    if (o == ::android::hardware::radio::V1_4::FrequencyRange::HIGH) {
        return "HIGH";
    }
    if (o == ::android::hardware::radio::V1_4::FrequencyRange::MMWAVE) {
        return "MMWAVE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::android::hardware::radio::V1_4::DataCallFailCause>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::DataCallFailCause> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NONE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NONE)) {
        os += (first ? "" : " | ");
        os += "NONE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NONE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OPERATOR_BARRED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OPERATOR_BARRED)) {
        os += (first ? "" : " | ");
        os += "OPERATOR_BARRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OPERATOR_BARRED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NAS_SIGNALLING) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NAS_SIGNALLING)) {
        os += (first ? "" : " | ");
        os += "NAS_SIGNALLING";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NAS_SIGNALLING;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::INSUFFICIENT_RESOURCES) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::INSUFFICIENT_RESOURCES)) {
        os += (first ? "" : " | ");
        os += "INSUFFICIENT_RESOURCES";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::INSUFFICIENT_RESOURCES;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MISSING_UKNOWN_APN) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MISSING_UKNOWN_APN)) {
        os += (first ? "" : " | ");
        os += "MISSING_UKNOWN_APN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MISSING_UKNOWN_APN;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::UNKNOWN_PDP_ADDRESS_TYPE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::UNKNOWN_PDP_ADDRESS_TYPE)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN_PDP_ADDRESS_TYPE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::UNKNOWN_PDP_ADDRESS_TYPE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::USER_AUTHENTICATION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::USER_AUTHENTICATION)) {
        os += (first ? "" : " | ");
        os += "USER_AUTHENTICATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::USER_AUTHENTICATION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ACTIVATION_REJECT_GGSN) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ACTIVATION_REJECT_GGSN)) {
        os += (first ? "" : " | ");
        os += "ACTIVATION_REJECT_GGSN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ACTIVATION_REJECT_GGSN;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ACTIVATION_REJECT_UNSPECIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ACTIVATION_REJECT_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "ACTIVATION_REJECT_UNSPECIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ACTIVATION_REJECT_UNSPECIFIED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_OPTION_NOT_SUPPORTED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_OPTION_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "SERVICE_OPTION_NOT_SUPPORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_OPTION_NOT_SUPPORTED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_OPTION_NOT_SUBSCRIBED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_OPTION_NOT_SUBSCRIBED)) {
        os += (first ? "" : " | ");
        os += "SERVICE_OPTION_NOT_SUBSCRIBED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_OPTION_NOT_SUBSCRIBED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_OPTION_OUT_OF_ORDER) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_OPTION_OUT_OF_ORDER)) {
        os += (first ? "" : " | ");
        os += "SERVICE_OPTION_OUT_OF_ORDER";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_OPTION_OUT_OF_ORDER;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NSAPI_IN_USE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NSAPI_IN_USE)) {
        os += (first ? "" : " | ");
        os += "NSAPI_IN_USE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NSAPI_IN_USE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::REGULAR_DEACTIVATION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::REGULAR_DEACTIVATION)) {
        os += (first ? "" : " | ");
        os += "REGULAR_DEACTIVATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::REGULAR_DEACTIVATION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::QOS_NOT_ACCEPTED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::QOS_NOT_ACCEPTED)) {
        os += (first ? "" : " | ");
        os += "QOS_NOT_ACCEPTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::QOS_NOT_ACCEPTED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_FAILURE)) {
        os += (first ? "" : " | ");
        os += "NETWORK_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::UMTS_REACTIVATION_REQ) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::UMTS_REACTIVATION_REQ)) {
        os += (first ? "" : " | ");
        os += "UMTS_REACTIVATION_REQ";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::UMTS_REACTIVATION_REQ;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::FEATURE_NOT_SUPP) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::FEATURE_NOT_SUPP)) {
        os += (first ? "" : " | ");
        os += "FEATURE_NOT_SUPP";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::FEATURE_NOT_SUPP;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::TFT_SEMANTIC_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::TFT_SEMANTIC_ERROR)) {
        os += (first ? "" : " | ");
        os += "TFT_SEMANTIC_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::TFT_SEMANTIC_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::TFT_SYTAX_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::TFT_SYTAX_ERROR)) {
        os += (first ? "" : " | ");
        os += "TFT_SYTAX_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::TFT_SYTAX_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::UNKNOWN_PDP_CONTEXT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::UNKNOWN_PDP_CONTEXT)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN_PDP_CONTEXT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::UNKNOWN_PDP_CONTEXT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::FILTER_SEMANTIC_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::FILTER_SEMANTIC_ERROR)) {
        os += (first ? "" : " | ");
        os += "FILTER_SEMANTIC_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::FILTER_SEMANTIC_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::FILTER_SYTAX_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::FILTER_SYTAX_ERROR)) {
        os += (first ? "" : " | ");
        os += "FILTER_SYTAX_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::FILTER_SYTAX_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDP_WITHOUT_ACTIVE_TFT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDP_WITHOUT_ACTIVE_TFT)) {
        os += (first ? "" : " | ");
        os += "PDP_WITHOUT_ACTIVE_TFT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDP_WITHOUT_ACTIVE_TFT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_IPV4_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ONLY_IPV4_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "ONLY_IPV4_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_IPV4_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_IPV6_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ONLY_IPV6_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "ONLY_IPV6_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_IPV6_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_SINGLE_BEARER_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ONLY_SINGLE_BEARER_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "ONLY_SINGLE_BEARER_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_SINGLE_BEARER_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ESM_INFO_NOT_RECEIVED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ESM_INFO_NOT_RECEIVED)) {
        os += (first ? "" : " | ");
        os += "ESM_INFO_NOT_RECEIVED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ESM_INFO_NOT_RECEIVED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDN_CONN_DOES_NOT_EXIST) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDN_CONN_DOES_NOT_EXIST)) {
        os += (first ? "" : " | ");
        os += "PDN_CONN_DOES_NOT_EXIST";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDN_CONN_DOES_NOT_EXIST;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MAX_ACTIVE_PDP_CONTEXT_REACHED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MAX_ACTIVE_PDP_CONTEXT_REACHED)) {
        os += (first ? "" : " | ");
        os += "MAX_ACTIVE_PDP_CONTEXT_REACHED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MAX_ACTIVE_PDP_CONTEXT_REACHED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::UNSUPPORTED_APN_IN_CURRENT_PLMN) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::UNSUPPORTED_APN_IN_CURRENT_PLMN)) {
        os += (first ? "" : " | ");
        os += "UNSUPPORTED_APN_IN_CURRENT_PLMN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::UNSUPPORTED_APN_IN_CURRENT_PLMN;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_TRANSACTION_ID) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::INVALID_TRANSACTION_ID)) {
        os += (first ? "" : " | ");
        os += "INVALID_TRANSACTION_ID";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_TRANSACTION_ID;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MESSAGE_INCORRECT_SEMANTIC) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MESSAGE_INCORRECT_SEMANTIC)) {
        os += (first ? "" : " | ");
        os += "MESSAGE_INCORRECT_SEMANTIC";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MESSAGE_INCORRECT_SEMANTIC;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_MANDATORY_INFO) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::INVALID_MANDATORY_INFO)) {
        os += (first ? "" : " | ");
        os += "INVALID_MANDATORY_INFO";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_MANDATORY_INFO;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MESSAGE_TYPE_UNSUPPORTED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MESSAGE_TYPE_UNSUPPORTED)) {
        os += (first ? "" : " | ");
        os += "MESSAGE_TYPE_UNSUPPORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MESSAGE_TYPE_UNSUPPORTED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MSG_TYPE_NONCOMPATIBLE_STATE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MSG_TYPE_NONCOMPATIBLE_STATE)) {
        os += (first ? "" : " | ");
        os += "MSG_TYPE_NONCOMPATIBLE_STATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MSG_TYPE_NONCOMPATIBLE_STATE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::UNKNOWN_INFO_ELEMENT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::UNKNOWN_INFO_ELEMENT)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN_INFO_ELEMENT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::UNKNOWN_INFO_ELEMENT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CONDITIONAL_IE_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CONDITIONAL_IE_ERROR)) {
        os += (first ? "" : " | ");
        os += "CONDITIONAL_IE_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CONDITIONAL_IE_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE)) {
        os += (first ? "" : " | ");
        os += "MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PROTOCOL_ERRORS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PROTOCOL_ERRORS)) {
        os += (first ? "" : " | ");
        os += "PROTOCOL_ERRORS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PROTOCOL_ERRORS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::APN_TYPE_CONFLICT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::APN_TYPE_CONFLICT)) {
        os += (first ? "" : " | ");
        os += "APN_TYPE_CONFLICT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::APN_TYPE_CONFLICT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_PCSCF_ADDR) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::INVALID_PCSCF_ADDR)) {
        os += (first ? "" : " | ");
        os += "INVALID_PCSCF_ADDR";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_PCSCF_ADDR;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN)) {
        os += (first ? "" : " | ");
        os += "INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EMM_ACCESS_BARRED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EMM_ACCESS_BARRED)) {
        os += (first ? "" : " | ");
        os += "EMM_ACCESS_BARRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EMM_ACCESS_BARRED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EMERGENCY_IFACE_ONLY) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EMERGENCY_IFACE_ONLY)) {
        os += (first ? "" : " | ");
        os += "EMERGENCY_IFACE_ONLY";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EMERGENCY_IFACE_ONLY;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::IFACE_MISMATCH) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::IFACE_MISMATCH)) {
        os += (first ? "" : " | ");
        os += "IFACE_MISMATCH";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::IFACE_MISMATCH;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::COMPANION_IFACE_IN_USE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::COMPANION_IFACE_IN_USE)) {
        os += (first ? "" : " | ");
        os += "COMPANION_IFACE_IN_USE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::COMPANION_IFACE_IN_USE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::IP_ADDRESS_MISMATCH) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::IP_ADDRESS_MISMATCH)) {
        os += (first ? "" : " | ");
        os += "IP_ADDRESS_MISMATCH";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::IP_ADDRESS_MISMATCH;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::IFACE_AND_POL_FAMILY_MISMATCH) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::IFACE_AND_POL_FAMILY_MISMATCH)) {
        os += (first ? "" : " | ");
        os += "IFACE_AND_POL_FAMILY_MISMATCH";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::IFACE_AND_POL_FAMILY_MISMATCH;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EMM_ACCESS_BARRED_INFINITE_RETRY) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EMM_ACCESS_BARRED_INFINITE_RETRY)) {
        os += (first ? "" : " | ");
        os += "EMM_ACCESS_BARRED_INFINITE_RETRY";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EMM_ACCESS_BARRED_INFINITE_RETRY;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::AUTH_FAILURE_ON_EMERGENCY_CALL) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::AUTH_FAILURE_ON_EMERGENCY_CALL)) {
        os += (first ? "" : " | ");
        os += "AUTH_FAILURE_ON_EMERGENCY_CALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::AUTH_FAILURE_ON_EMERGENCY_CALL;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_1) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_1)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_1";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_1;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_2) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_2)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_2";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_2;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_3) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_3)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_3";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_3;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_4) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_4)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_4";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_4;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_5) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_5)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_5";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_5;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_6) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_6)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_6";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_6;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_7) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_7)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_7";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_7;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_8) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_8)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_8";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_8;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_9) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_9)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_9";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_9;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_10) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_10)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_10";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_10;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_11) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_11)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_11";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_11;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_12) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_12)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_12";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_12;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_13) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_13)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_13";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_13;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_14) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_14)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_14";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_14;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_15) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_15)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_15";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_15;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::VOICE_REGISTRATION_FAIL) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::VOICE_REGISTRATION_FAIL)) {
        os += (first ? "" : " | ");
        os += "VOICE_REGISTRATION_FAIL";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::VOICE_REGISTRATION_FAIL;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::DATA_REGISTRATION_FAIL) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::DATA_REGISTRATION_FAIL)) {
        os += (first ? "" : " | ");
        os += "DATA_REGISTRATION_FAIL";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::DATA_REGISTRATION_FAIL;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::SIGNAL_LOST) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::SIGNAL_LOST)) {
        os += (first ? "" : " | ");
        os += "SIGNAL_LOST";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::SIGNAL_LOST;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PREF_RADIO_TECH_CHANGED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PREF_RADIO_TECH_CHANGED)) {
        os += (first ? "" : " | ");
        os += "PREF_RADIO_TECH_CHANGED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PREF_RADIO_TECH_CHANGED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RADIO_POWER_OFF) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RADIO_POWER_OFF)) {
        os += (first ? "" : " | ");
        os += "RADIO_POWER_OFF";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RADIO_POWER_OFF;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::TETHERED_CALL_ACTIVE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::TETHERED_CALL_ACTIVE)) {
        os += (first ? "" : " | ");
        os += "TETHERED_CALL_ACTIVE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::TETHERED_CALL_ACTIVE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ERROR_UNSPECIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ERROR_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "ERROR_UNSPECIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ERROR_UNSPECIFIED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::LLC_SNDCP) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::LLC_SNDCP)) {
        os += (first ? "" : " | ");
        os += "LLC_SNDCP";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::LLC_SNDCP;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ACTIVATION_REJECTED_BCM_VIOLATION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ACTIVATION_REJECTED_BCM_VIOLATION)) {
        os += (first ? "" : " | ");
        os += "ACTIVATION_REJECTED_BCM_VIOLATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ACTIVATION_REJECTED_BCM_VIOLATION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::COLLISION_WITH_NETWORK_INITIATED_REQUEST) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::COLLISION_WITH_NETWORK_INITIATED_REQUEST)) {
        os += (first ? "" : " | ");
        os += "COLLISION_WITH_NETWORK_INITIATED_REQUEST";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::COLLISION_WITH_NETWORK_INITIATED_REQUEST;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_IPV4V6_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ONLY_IPV4V6_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "ONLY_IPV4V6_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_IPV4V6_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_NON_IP_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ONLY_NON_IP_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "ONLY_NON_IP_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_NON_IP_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::UNSUPPORTED_QCI_VALUE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::UNSUPPORTED_QCI_VALUE)) {
        os += (first ? "" : " | ");
        os += "UNSUPPORTED_QCI_VALUE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::UNSUPPORTED_QCI_VALUE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::BEARER_HANDLING_NOT_SUPPORTED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::BEARER_HANDLING_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "BEARER_HANDLING_NOT_SUPPORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::BEARER_HANDLING_NOT_SUPPORTED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_DNS_ADDR) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::INVALID_DNS_ADDR)) {
        os += (first ? "" : " | ");
        os += "INVALID_DNS_ADDR";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_DNS_ADDR;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_PCSCF_OR_DNS_ADDRESS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::INVALID_PCSCF_OR_DNS_ADDRESS)) {
        os += (first ? "" : " | ");
        os += "INVALID_PCSCF_OR_DNS_ADDRESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_PCSCF_OR_DNS_ADDRESS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CALL_PREEMPT_BY_EMERGENCY_APN) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CALL_PREEMPT_BY_EMERGENCY_APN)) {
        os += (first ? "" : " | ");
        os += "CALL_PREEMPT_BY_EMERGENCY_APN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CALL_PREEMPT_BY_EMERGENCY_APN;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::UE_INITIATED_DETACH_OR_DISCONNECT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::UE_INITIATED_DETACH_OR_DISCONNECT)) {
        os += (first ? "" : " | ");
        os += "UE_INITIATED_DETACH_OR_DISCONNECT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::UE_INITIATED_DETACH_OR_DISCONNECT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REASON_UNSPECIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REASON_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_REASON_UNSPECIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REASON_UNSPECIFIED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_ADMIN_PROHIBITED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_ADMIN_PROHIBITED)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_ADMIN_PROHIBITED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_ADMIN_PROHIBITED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_INSUFFICIENT_RESOURCES) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_INSUFFICIENT_RESOURCES)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_INSUFFICIENT_RESOURCES";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_INSUFFICIENT_RESOURCES;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MOBILE_NODE_AUTHENTICATION_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MOBILE_NODE_AUTHENTICATION_FAILURE)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_MOBILE_NODE_AUTHENTICATION_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MOBILE_NODE_AUTHENTICATION_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_HOME_AGENT_AUTHENTICATION_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_HOME_AGENT_AUTHENTICATION_FAILURE)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_HOME_AGENT_AUTHENTICATION_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_HOME_AGENT_AUTHENTICATION_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REQUESTED_LIFETIME_TOO_LONG) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REQUESTED_LIFETIME_TOO_LONG)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_REQUESTED_LIFETIME_TOO_LONG";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REQUESTED_LIFETIME_TOO_LONG;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MALFORMED_REQUEST) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MALFORMED_REQUEST)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_MALFORMED_REQUEST";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MALFORMED_REQUEST;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MALFORMED_REPLY) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MALFORMED_REPLY)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_MALFORMED_REPLY";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MALFORMED_REPLY;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_ENCAPSULATION_UNAVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_ENCAPSULATION_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_ENCAPSULATION_UNAVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_ENCAPSULATION_UNAVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_VJ_HEADER_COMPRESSION_UNAVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_VJ_HEADER_COMPRESSION_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_VJ_HEADER_COMPRESSION_UNAVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_VJ_HEADER_COMPRESSION_UNAVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REVERSE_TUNNEL_UNAVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REVERSE_TUNNEL_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_REVERSE_TUNNEL_UNAVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REVERSE_TUNNEL_UNAVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REVERSE_TUNNEL_IS_MANDATORY) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REVERSE_TUNNEL_IS_MANDATORY)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_REVERSE_TUNNEL_IS_MANDATORY";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REVERSE_TUNNEL_IS_MANDATORY;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_DELIVERY_STYLE_NOT_SUPPORTED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_DELIVERY_STYLE_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_DELIVERY_STYLE_NOT_SUPPORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_DELIVERY_STYLE_NOT_SUPPORTED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_NAI) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_NAI)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_MISSING_NAI";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_NAI;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_HOME_AGENT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_HOME_AGENT)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_MISSING_HOME_AGENT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_HOME_AGENT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_HOME_ADDRESS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_HOME_ADDRESS)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_MISSING_HOME_ADDRESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_HOME_ADDRESS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_UNKNOWN_CHALLENGE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_UNKNOWN_CHALLENGE)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_UNKNOWN_CHALLENGE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_UNKNOWN_CHALLENGE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_CHALLENGE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_CHALLENGE)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_MISSING_CHALLENGE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_CHALLENGE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_STALE_CHALLENGE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_STALE_CHALLENGE)) {
        os += (first ? "" : " | ");
        os += "MIP_FA_STALE_CHALLENGE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_STALE_CHALLENGE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REASON_UNSPECIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REASON_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "MIP_HA_REASON_UNSPECIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REASON_UNSPECIFIED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_ADMIN_PROHIBITED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_ADMIN_PROHIBITED)) {
        os += (first ? "" : " | ");
        os += "MIP_HA_ADMIN_PROHIBITED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_ADMIN_PROHIBITED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_INSUFFICIENT_RESOURCES) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_INSUFFICIENT_RESOURCES)) {
        os += (first ? "" : " | ");
        os += "MIP_HA_INSUFFICIENT_RESOURCES";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_INSUFFICIENT_RESOURCES;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_MOBILE_NODE_AUTHENTICATION_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_MOBILE_NODE_AUTHENTICATION_FAILURE)) {
        os += (first ? "" : " | ");
        os += "MIP_HA_MOBILE_NODE_AUTHENTICATION_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_MOBILE_NODE_AUTHENTICATION_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_FOREIGN_AGENT_AUTHENTICATION_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_FOREIGN_AGENT_AUTHENTICATION_FAILURE)) {
        os += (first ? "" : " | ");
        os += "MIP_HA_FOREIGN_AGENT_AUTHENTICATION_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_FOREIGN_AGENT_AUTHENTICATION_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REGISTRATION_ID_MISMATCH) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REGISTRATION_ID_MISMATCH)) {
        os += (first ? "" : " | ");
        os += "MIP_HA_REGISTRATION_ID_MISMATCH";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REGISTRATION_ID_MISMATCH;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_MALFORMED_REQUEST) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_MALFORMED_REQUEST)) {
        os += (first ? "" : " | ");
        os += "MIP_HA_MALFORMED_REQUEST";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_MALFORMED_REQUEST;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_UNKNOWN_HOME_AGENT_ADDRESS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_UNKNOWN_HOME_AGENT_ADDRESS)) {
        os += (first ? "" : " | ");
        os += "MIP_HA_UNKNOWN_HOME_AGENT_ADDRESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_UNKNOWN_HOME_AGENT_ADDRESS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REVERSE_TUNNEL_UNAVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REVERSE_TUNNEL_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "MIP_HA_REVERSE_TUNNEL_UNAVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REVERSE_TUNNEL_UNAVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REVERSE_TUNNEL_IS_MANDATORY) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REVERSE_TUNNEL_IS_MANDATORY)) {
        os += (first ? "" : " | ");
        os += "MIP_HA_REVERSE_TUNNEL_IS_MANDATORY";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REVERSE_TUNNEL_IS_MANDATORY;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_ENCAPSULATION_UNAVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_ENCAPSULATION_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "MIP_HA_ENCAPSULATION_UNAVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_ENCAPSULATION_UNAVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CLOSE_IN_PROGRESS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CLOSE_IN_PROGRESS)) {
        os += (first ? "" : " | ");
        os += "CLOSE_IN_PROGRESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CLOSE_IN_PROGRESS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_INITIATED_TERMINATION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_INITIATED_TERMINATION)) {
        os += (first ? "" : " | ");
        os += "NETWORK_INITIATED_TERMINATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_INITIATED_TERMINATION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MODEM_APP_PREEMPTED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MODEM_APP_PREEMPTED)) {
        os += (first ? "" : " | ");
        os += "MODEM_APP_PREEMPTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MODEM_APP_PREEMPTED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV4_CALL_DISALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV4_CALL_DISALLOWED)) {
        os += (first ? "" : " | ");
        os += "PDN_IPV4_CALL_DISALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV4_CALL_DISALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV4_CALL_THROTTLED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV4_CALL_THROTTLED)) {
        os += (first ? "" : " | ");
        os += "PDN_IPV4_CALL_THROTTLED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV4_CALL_THROTTLED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV6_CALL_DISALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV6_CALL_DISALLOWED)) {
        os += (first ? "" : " | ");
        os += "PDN_IPV6_CALL_DISALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV6_CALL_DISALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV6_CALL_THROTTLED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV6_CALL_THROTTLED)) {
        os += (first ? "" : " | ");
        os += "PDN_IPV6_CALL_THROTTLED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV6_CALL_THROTTLED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MODEM_RESTART) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MODEM_RESTART)) {
        os += (first ? "" : " | ");
        os += "MODEM_RESTART";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MODEM_RESTART;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDP_PPP_NOT_SUPPORTED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDP_PPP_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "PDP_PPP_NOT_SUPPORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDP_PPP_NOT_SUPPORTED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::UNPREFERRED_RAT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::UNPREFERRED_RAT)) {
        os += (first ? "" : " | ");
        os += "UNPREFERRED_RAT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::UNPREFERRED_RAT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PHYSICAL_LINK_CLOSE_IN_PROGRESS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PHYSICAL_LINK_CLOSE_IN_PROGRESS)) {
        os += (first ? "" : " | ");
        os += "PHYSICAL_LINK_CLOSE_IN_PROGRESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PHYSICAL_LINK_CLOSE_IN_PROGRESS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::APN_PENDING_HANDOVER) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::APN_PENDING_HANDOVER)) {
        os += (first ? "" : " | ");
        os += "APN_PENDING_HANDOVER";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::APN_PENDING_HANDOVER;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PROFILE_BEARER_INCOMPATIBLE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PROFILE_BEARER_INCOMPATIBLE)) {
        os += (first ? "" : " | ");
        os += "PROFILE_BEARER_INCOMPATIBLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PROFILE_BEARER_INCOMPATIBLE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::SIM_CARD_CHANGED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::SIM_CARD_CHANGED)) {
        os += (first ? "" : " | ");
        os += "SIM_CARD_CHANGED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::SIM_CARD_CHANGED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::LOW_POWER_MODE_OR_POWERING_DOWN) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::LOW_POWER_MODE_OR_POWERING_DOWN)) {
        os += (first ? "" : " | ");
        os += "LOW_POWER_MODE_OR_POWERING_DOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::LOW_POWER_MODE_OR_POWERING_DOWN;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::APN_DISABLED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::APN_DISABLED)) {
        os += (first ? "" : " | ");
        os += "APN_DISABLED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::APN_DISABLED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MAX_PPP_INACTIVITY_TIMER_EXPIRED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MAX_PPP_INACTIVITY_TIMER_EXPIRED)) {
        os += (first ? "" : " | ");
        os += "MAX_PPP_INACTIVITY_TIMER_EXPIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MAX_PPP_INACTIVITY_TIMER_EXPIRED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::IPV6_ADDRESS_TRANSFER_FAILED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::IPV6_ADDRESS_TRANSFER_FAILED)) {
        os += (first ? "" : " | ");
        os += "IPV6_ADDRESS_TRANSFER_FAILED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::IPV6_ADDRESS_TRANSFER_FAILED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::TRAT_SWAP_FAILED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::TRAT_SWAP_FAILED)) {
        os += (first ? "" : " | ");
        os += "TRAT_SWAP_FAILED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::TRAT_SWAP_FAILED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EHRPD_TO_HRPD_FALLBACK) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EHRPD_TO_HRPD_FALLBACK)) {
        os += (first ? "" : " | ");
        os += "EHRPD_TO_HRPD_FALLBACK";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EHRPD_TO_HRPD_FALLBACK;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MIP_CONFIG_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MIP_CONFIG_FAILURE)) {
        os += (first ? "" : " | ");
        os += "MIP_CONFIG_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MIP_CONFIG_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDN_INACTIVITY_TIMER_EXPIRED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDN_INACTIVITY_TIMER_EXPIRED)) {
        os += (first ? "" : " | ");
        os += "PDN_INACTIVITY_TIMER_EXPIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDN_INACTIVITY_TIMER_EXPIRED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MAX_IPV4_CONNECTIONS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MAX_IPV4_CONNECTIONS)) {
        os += (first ? "" : " | ");
        os += "MAX_IPV4_CONNECTIONS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MAX_IPV4_CONNECTIONS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MAX_IPV6_CONNECTIONS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MAX_IPV6_CONNECTIONS)) {
        os += (first ? "" : " | ");
        os += "MAX_IPV6_CONNECTIONS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MAX_IPV6_CONNECTIONS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::APN_MISMATCH) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::APN_MISMATCH)) {
        os += (first ? "" : " | ");
        os += "APN_MISMATCH";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::APN_MISMATCH;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::IP_VERSION_MISMATCH) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::IP_VERSION_MISMATCH)) {
        os += (first ? "" : " | ");
        os += "IP_VERSION_MISMATCH";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::IP_VERSION_MISMATCH;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::DUN_CALL_DISALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::DUN_CALL_DISALLOWED)) {
        os += (first ? "" : " | ");
        os += "DUN_CALL_DISALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::DUN_CALL_DISALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::INTERNAL_EPC_NONEPC_TRANSITION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::INTERNAL_EPC_NONEPC_TRANSITION)) {
        os += (first ? "" : " | ");
        os += "INTERNAL_EPC_NONEPC_TRANSITION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::INTERNAL_EPC_NONEPC_TRANSITION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::INTERFACE_IN_USE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::INTERFACE_IN_USE)) {
        os += (first ? "" : " | ");
        os += "INTERFACE_IN_USE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::INTERFACE_IN_USE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::APN_DISALLOWED_ON_ROAMING) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::APN_DISALLOWED_ON_ROAMING)) {
        os += (first ? "" : " | ");
        os += "APN_DISALLOWED_ON_ROAMING";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::APN_DISALLOWED_ON_ROAMING;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::APN_PARAMETERS_CHANGED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::APN_PARAMETERS_CHANGED)) {
        os += (first ? "" : " | ");
        os += "APN_PARAMETERS_CHANGED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::APN_PARAMETERS_CHANGED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NULL_APN_DISALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NULL_APN_DISALLOWED)) {
        os += (first ? "" : " | ");
        os += "NULL_APN_DISALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NULL_APN_DISALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::THERMAL_MITIGATION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::THERMAL_MITIGATION)) {
        os += (first ? "" : " | ");
        os += "THERMAL_MITIGATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::THERMAL_MITIGATION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::DATA_SETTINGS_DISABLED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::DATA_SETTINGS_DISABLED)) {
        os += (first ? "" : " | ");
        os += "DATA_SETTINGS_DISABLED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::DATA_SETTINGS_DISABLED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::DATA_ROAMING_SETTINGS_DISABLED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::DATA_ROAMING_SETTINGS_DISABLED)) {
        os += (first ? "" : " | ");
        os += "DATA_ROAMING_SETTINGS_DISABLED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::DATA_ROAMING_SETTINGS_DISABLED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::DDS_SWITCHED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::DDS_SWITCHED)) {
        os += (first ? "" : " | ");
        os += "DDS_SWITCHED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::DDS_SWITCHED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::FORBIDDEN_APN_NAME) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::FORBIDDEN_APN_NAME)) {
        os += (first ? "" : " | ");
        os += "FORBIDDEN_APN_NAME";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::FORBIDDEN_APN_NAME;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::DDS_SWITCH_IN_PROGRESS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::DDS_SWITCH_IN_PROGRESS)) {
        os += (first ? "" : " | ");
        os += "DDS_SWITCH_IN_PROGRESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::DDS_SWITCH_IN_PROGRESS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CALL_DISALLOWED_IN_ROAMING) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CALL_DISALLOWED_IN_ROAMING)) {
        os += (first ? "" : " | ");
        os += "CALL_DISALLOWED_IN_ROAMING";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CALL_DISALLOWED_IN_ROAMING;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NON_IP_NOT_SUPPORTED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NON_IP_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "NON_IP_NOT_SUPPORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NON_IP_NOT_SUPPORTED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDN_NON_IP_CALL_THROTTLED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDN_NON_IP_CALL_THROTTLED)) {
        os += (first ? "" : " | ");
        os += "PDN_NON_IP_CALL_THROTTLED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDN_NON_IP_CALL_THROTTLED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDN_NON_IP_CALL_DISALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDN_NON_IP_CALL_DISALLOWED)) {
        os += (first ? "" : " | ");
        os += "PDN_NON_IP_CALL_DISALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDN_NON_IP_CALL_DISALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_LOCK) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CDMA_LOCK)) {
        os += (first ? "" : " | ");
        os += "CDMA_LOCK";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_LOCK;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_INTERCEPT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CDMA_INTERCEPT)) {
        os += (first ? "" : " | ");
        os += "CDMA_INTERCEPT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_INTERCEPT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_REORDER) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CDMA_REORDER)) {
        os += (first ? "" : " | ");
        os += "CDMA_REORDER";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_REORDER;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_RELEASE_DUE_TO_SO_REJECTION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CDMA_RELEASE_DUE_TO_SO_REJECTION)) {
        os += (first ? "" : " | ");
        os += "CDMA_RELEASE_DUE_TO_SO_REJECTION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_RELEASE_DUE_TO_SO_REJECTION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_INCOMING_CALL) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CDMA_INCOMING_CALL)) {
        os += (first ? "" : " | ");
        os += "CDMA_INCOMING_CALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_INCOMING_CALL;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_ALERT_STOP) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CDMA_ALERT_STOP)) {
        os += (first ? "" : " | ");
        os += "CDMA_ALERT_STOP";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_ALERT_STOP;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CHANNEL_ACQUISITION_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CHANNEL_ACQUISITION_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CHANNEL_ACQUISITION_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CHANNEL_ACQUISITION_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MAX_ACCESS_PROBE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MAX_ACCESS_PROBE)) {
        os += (first ? "" : " | ");
        os += "MAX_ACCESS_PROBE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MAX_ACCESS_PROBE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CONCURRENT_SERVICE_NOT_SUPPORTED_BY_BASE_STATION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CONCURRENT_SERVICE_NOT_SUPPORTED_BY_BASE_STATION)) {
        os += (first ? "" : " | ");
        os += "CONCURRENT_SERVICE_NOT_SUPPORTED_BY_BASE_STATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CONCURRENT_SERVICE_NOT_SUPPORTED_BY_BASE_STATION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NO_RESPONSE_FROM_BASE_STATION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NO_RESPONSE_FROM_BASE_STATION)) {
        os += (first ? "" : " | ");
        os += "NO_RESPONSE_FROM_BASE_STATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NO_RESPONSE_FROM_BASE_STATION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::REJECTED_BY_BASE_STATION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::REJECTED_BY_BASE_STATION)) {
        os += (first ? "" : " | ");
        os += "REJECTED_BY_BASE_STATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::REJECTED_BY_BASE_STATION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CONCURRENT_SERVICES_INCOMPATIBLE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CONCURRENT_SERVICES_INCOMPATIBLE)) {
        os += (first ? "" : " | ");
        os += "CONCURRENT_SERVICES_INCOMPATIBLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CONCURRENT_SERVICES_INCOMPATIBLE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NO_CDMA_SERVICE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NO_CDMA_SERVICE)) {
        os += (first ? "" : " | ");
        os += "NO_CDMA_SERVICE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NO_CDMA_SERVICE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RUIM_NOT_PRESENT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RUIM_NOT_PRESENT)) {
        os += (first ? "" : " | ");
        os += "RUIM_NOT_PRESENT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RUIM_NOT_PRESENT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_RETRY_ORDER) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CDMA_RETRY_ORDER)) {
        os += (first ? "" : " | ");
        os += "CDMA_RETRY_ORDER";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_RETRY_ORDER;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_BLOCK) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_BLOCK)) {
        os += (first ? "" : " | ");
        os += "ACCESS_BLOCK";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_BLOCK;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_BLOCK_ALL) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_BLOCK_ALL)) {
        os += (first ? "" : " | ");
        os += "ACCESS_BLOCK_ALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_BLOCK_ALL;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::IS707B_MAX_ACCESS_PROBES) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::IS707B_MAX_ACCESS_PROBES)) {
        os += (first ? "" : " | ");
        os += "IS707B_MAX_ACCESS_PROBES";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::IS707B_MAX_ACCESS_PROBES;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::THERMAL_EMERGENCY) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::THERMAL_EMERGENCY)) {
        os += (first ? "" : " | ");
        os += "THERMAL_EMERGENCY";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::THERMAL_EMERGENCY;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CONCURRENT_SERVICES_NOT_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CONCURRENT_SERVICES_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "CONCURRENT_SERVICES_NOT_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CONCURRENT_SERVICES_NOT_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::INCOMING_CALL_REJECTED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::INCOMING_CALL_REJECTED)) {
        os += (first ? "" : " | ");
        os += "INCOMING_CALL_REJECTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::INCOMING_CALL_REJECTED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NO_SERVICE_ON_GATEWAY) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NO_SERVICE_ON_GATEWAY)) {
        os += (first ? "" : " | ");
        os += "NO_SERVICE_ON_GATEWAY";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NO_SERVICE_ON_GATEWAY;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NO_GPRS_CONTEXT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NO_GPRS_CONTEXT)) {
        os += (first ? "" : " | ");
        os += "NO_GPRS_CONTEXT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NO_GPRS_CONTEXT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ILLEGAL_MS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ILLEGAL_MS)) {
        os += (first ? "" : " | ");
        os += "ILLEGAL_MS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ILLEGAL_MS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ILLEGAL_ME) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ILLEGAL_ME)) {
        os += (first ? "" : " | ");
        os += "ILLEGAL_ME";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ILLEGAL_ME;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::GPRS_SERVICES_AND_NON_GPRS_SERVICES_NOT_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::GPRS_SERVICES_AND_NON_GPRS_SERVICES_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "GPRS_SERVICES_AND_NON_GPRS_SERVICES_NOT_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::GPRS_SERVICES_AND_NON_GPRS_SERVICES_NOT_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::GPRS_SERVICES_NOT_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::GPRS_SERVICES_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "GPRS_SERVICES_NOT_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::GPRS_SERVICES_NOT_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MS_IDENTITY_CANNOT_BE_DERIVED_BY_THE_NETWORK) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MS_IDENTITY_CANNOT_BE_DERIVED_BY_THE_NETWORK)) {
        os += (first ? "" : " | ");
        os += "MS_IDENTITY_CANNOT_BE_DERIVED_BY_THE_NETWORK";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MS_IDENTITY_CANNOT_BE_DERIVED_BY_THE_NETWORK;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::IMPLICITLY_DETACHED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::IMPLICITLY_DETACHED)) {
        os += (first ? "" : " | ");
        os += "IMPLICITLY_DETACHED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::IMPLICITLY_DETACHED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PLMN_NOT_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PLMN_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "PLMN_NOT_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PLMN_NOT_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::LOCATION_AREA_NOT_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::LOCATION_AREA_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "LOCATION_AREA_NOT_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::LOCATION_AREA_NOT_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::GPRS_SERVICES_NOT_ALLOWED_IN_THIS_PLMN) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::GPRS_SERVICES_NOT_ALLOWED_IN_THIS_PLMN)) {
        os += (first ? "" : " | ");
        os += "GPRS_SERVICES_NOT_ALLOWED_IN_THIS_PLMN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::GPRS_SERVICES_NOT_ALLOWED_IN_THIS_PLMN;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDP_DUPLICATE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDP_DUPLICATE)) {
        os += (first ? "" : " | ");
        os += "PDP_DUPLICATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDP_DUPLICATE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::UE_RAT_CHANGE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::UE_RAT_CHANGE)) {
        os += (first ? "" : " | ");
        os += "UE_RAT_CHANGE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::UE_RAT_CHANGE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CONGESTION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CONGESTION)) {
        os += (first ? "" : " | ");
        os += "CONGESTION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CONGESTION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NO_PDP_CONTEXT_ACTIVATED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NO_PDP_CONTEXT_ACTIVATED)) {
        os += (first ? "" : " | ");
        os += "NO_PDP_CONTEXT_ACTIVATED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NO_PDP_CONTEXT_ACTIVATED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_CLASS_DSAC_REJECTION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_CLASS_DSAC_REJECTION)) {
        os += (first ? "" : " | ");
        os += "ACCESS_CLASS_DSAC_REJECTION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_CLASS_DSAC_REJECTION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDP_ACTIVATE_MAX_RETRY_FAILED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDP_ACTIVATE_MAX_RETRY_FAILED)) {
        os += (first ? "" : " | ");
        os += "PDP_ACTIVATE_MAX_RETRY_FAILED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDP_ACTIVATE_MAX_RETRY_FAILED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RADIO_ACCESS_BEARER_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RADIO_ACCESS_BEARER_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RADIO_ACCESS_BEARER_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RADIO_ACCESS_BEARER_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ESM_UNKNOWN_EPS_BEARER_CONTEXT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ESM_UNKNOWN_EPS_BEARER_CONTEXT)) {
        os += (first ? "" : " | ");
        os += "ESM_UNKNOWN_EPS_BEARER_CONTEXT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ESM_UNKNOWN_EPS_BEARER_CONTEXT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::DRB_RELEASED_BY_RRC) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::DRB_RELEASED_BY_RRC)) {
        os += (first ? "" : " | ");
        os += "DRB_RELEASED_BY_RRC";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::DRB_RELEASED_BY_RRC;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CONNECTION_RELEASED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CONNECTION_RELEASED)) {
        os += (first ? "" : " | ");
        os += "CONNECTION_RELEASED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CONNECTION_RELEASED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EMM_DETACHED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EMM_DETACHED)) {
        os += (first ? "" : " | ");
        os += "EMM_DETACHED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EMM_DETACHED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EMM_ATTACH_FAILED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EMM_ATTACH_FAILED)) {
        os += (first ? "" : " | ");
        os += "EMM_ATTACH_FAILED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EMM_ATTACH_FAILED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EMM_ATTACH_STARTED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EMM_ATTACH_STARTED)) {
        os += (first ? "" : " | ");
        os += "EMM_ATTACH_STARTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EMM_ATTACH_STARTED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::LTE_NAS_SERVICE_REQUEST_FAILED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::LTE_NAS_SERVICE_REQUEST_FAILED)) {
        os += (first ? "" : " | ");
        os += "LTE_NAS_SERVICE_REQUEST_FAILED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::LTE_NAS_SERVICE_REQUEST_FAILED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::DUPLICATE_BEARER_ID) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::DUPLICATE_BEARER_ID)) {
        os += (first ? "" : " | ");
        os += "DUPLICATE_BEARER_ID";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::DUPLICATE_BEARER_ID;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ESM_COLLISION_SCENARIOS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ESM_COLLISION_SCENARIOS)) {
        os += (first ? "" : " | ");
        os += "ESM_COLLISION_SCENARIOS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ESM_COLLISION_SCENARIOS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ESM_BEARER_DEACTIVATED_TO_SYNC_WITH_NETWORK) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ESM_BEARER_DEACTIVATED_TO_SYNC_WITH_NETWORK)) {
        os += (first ? "" : " | ");
        os += "ESM_BEARER_DEACTIVATED_TO_SYNC_WITH_NETWORK";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ESM_BEARER_DEACTIVATED_TO_SYNC_WITH_NETWORK;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ESM_NW_ACTIVATED_DED_BEARER_WITH_ID_OF_DEF_BEARER) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ESM_NW_ACTIVATED_DED_BEARER_WITH_ID_OF_DEF_BEARER)) {
        os += (first ? "" : " | ");
        os += "ESM_NW_ACTIVATED_DED_BEARER_WITH_ID_OF_DEF_BEARER";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ESM_NW_ACTIVATED_DED_BEARER_WITH_ID_OF_DEF_BEARER;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ESM_BAD_OTA_MESSAGE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ESM_BAD_OTA_MESSAGE)) {
        os += (first ? "" : " | ");
        os += "ESM_BAD_OTA_MESSAGE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ESM_BAD_OTA_MESSAGE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ESM_DOWNLOAD_SERVER_REJECTED_THE_CALL) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ESM_DOWNLOAD_SERVER_REJECTED_THE_CALL)) {
        os += (first ? "" : " | ");
        os += "ESM_DOWNLOAD_SERVER_REJECTED_THE_CALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ESM_DOWNLOAD_SERVER_REJECTED_THE_CALL;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ESM_CONTEXT_TRANSFERRED_DUE_TO_IRAT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ESM_CONTEXT_TRANSFERRED_DUE_TO_IRAT)) {
        os += (first ? "" : " | ");
        os += "ESM_CONTEXT_TRANSFERRED_DUE_TO_IRAT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ESM_CONTEXT_TRANSFERRED_DUE_TO_IRAT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::DS_EXPLICIT_DEACTIVATION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::DS_EXPLICIT_DEACTIVATION)) {
        os += (first ? "" : " | ");
        os += "DS_EXPLICIT_DEACTIVATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::DS_EXPLICIT_DEACTIVATION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ESM_LOCAL_CAUSE_NONE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ESM_LOCAL_CAUSE_NONE)) {
        os += (first ? "" : " | ");
        os += "ESM_LOCAL_CAUSE_NONE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ESM_LOCAL_CAUSE_NONE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::LTE_THROTTLING_NOT_REQUIRED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::LTE_THROTTLING_NOT_REQUIRED)) {
        os += (first ? "" : " | ");
        os += "LTE_THROTTLING_NOT_REQUIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::LTE_THROTTLING_NOT_REQUIRED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_CONTROL_LIST_CHECK_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_CONTROL_LIST_CHECK_FAILURE)) {
        os += (first ? "" : " | ");
        os += "ACCESS_CONTROL_LIST_CHECK_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_CONTROL_LIST_CHECK_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_NOT_ALLOWED_ON_PLMN) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_NOT_ALLOWED_ON_PLMN)) {
        os += (first ? "" : " | ");
        os += "SERVICE_NOT_ALLOWED_ON_PLMN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_NOT_ALLOWED_ON_PLMN;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EMM_T3417_EXPIRED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EMM_T3417_EXPIRED)) {
        os += (first ? "" : " | ");
        os += "EMM_T3417_EXPIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EMM_T3417_EXPIRED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EMM_T3417_EXT_EXPIRED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EMM_T3417_EXT_EXPIRED)) {
        os += (first ? "" : " | ");
        os += "EMM_T3417_EXT_EXPIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EMM_T3417_EXT_EXPIRED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_DATA_TRANSMISSION_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_DATA_TRANSMISSION_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RRC_UPLINK_DATA_TRANSMISSION_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_DATA_TRANSMISSION_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_DELIVERY_FAILED_DUE_TO_HANDOVER) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_DELIVERY_FAILED_DUE_TO_HANDOVER)) {
        os += (first ? "" : " | ");
        os += "RRC_UPLINK_DELIVERY_FAILED_DUE_TO_HANDOVER";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_DELIVERY_FAILED_DUE_TO_HANDOVER;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_CONNECTION_RELEASE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_CONNECTION_RELEASE)) {
        os += (first ? "" : " | ");
        os += "RRC_UPLINK_CONNECTION_RELEASE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_CONNECTION_RELEASE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_RADIO_LINK_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_RADIO_LINK_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RRC_UPLINK_RADIO_LINK_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_RADIO_LINK_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_ERROR_REQUEST_FROM_NAS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_ERROR_REQUEST_FROM_NAS)) {
        os += (first ? "" : " | ");
        os += "RRC_UPLINK_ERROR_REQUEST_FROM_NAS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_ERROR_REQUEST_FROM_NAS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ACCESS_STRATUM_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ACCESS_STRATUM_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_ACCESS_STRATUM_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ACCESS_STRATUM_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ANOTHER_PROCEDURE_IN_PROGRESS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ANOTHER_PROCEDURE_IN_PROGRESS)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_ANOTHER_PROCEDURE_IN_PROGRESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ANOTHER_PROCEDURE_IN_PROGRESS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ACCESS_BARRED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ACCESS_BARRED)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_ACCESS_BARRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ACCESS_BARRED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_CELL_RESELECTION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_CELL_RESELECTION)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_CELL_RESELECTION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_CELL_RESELECTION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_CONFIG_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_CONFIG_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_CONFIG_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_CONFIG_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_TIMER_EXPIRED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_TIMER_EXPIRED)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_TIMER_EXPIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_TIMER_EXPIRED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_LINK_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_LINK_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_LINK_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_LINK_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_CELL_NOT_CAMPED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_CELL_NOT_CAMPED)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_CELL_NOT_CAMPED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_CELL_NOT_CAMPED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_SYSTEM_INTERVAL_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_SYSTEM_INTERVAL_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_SYSTEM_INTERVAL_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_SYSTEM_INTERVAL_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_REJECT_BY_NETWORK) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_REJECT_BY_NETWORK)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_REJECT_BY_NETWORK";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_REJECT_BY_NETWORK;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_NORMAL_RELEASE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_NORMAL_RELEASE)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_NORMAL_RELEASE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_NORMAL_RELEASE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_RADIO_LINK_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_RADIO_LINK_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_RADIO_LINK_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_RADIO_LINK_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_REESTABLISHMENT_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_REESTABLISHMENT_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_REESTABLISHMENT_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_REESTABLISHMENT_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_OUT_OF_SERVICE_DURING_CELL_REGISTER) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_OUT_OF_SERVICE_DURING_CELL_REGISTER)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_OUT_OF_SERVICE_DURING_CELL_REGISTER";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_OUT_OF_SERVICE_DURING_CELL_REGISTER;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORT_REQUEST) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORT_REQUEST)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_ABORT_REQUEST";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORT_REQUEST;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_SYSTEM_INFORMATION_BLOCK_READ_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_SYSTEM_INFORMATION_BLOCK_READ_ERROR)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_SYSTEM_INFORMATION_BLOCK_READ_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_SYSTEM_INFORMATION_BLOCK_READ_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_INITIATED_DETACH_WITH_AUTO_REATTACH) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_INITIATED_DETACH_WITH_AUTO_REATTACH)) {
        os += (first ? "" : " | ");
        os += "NETWORK_INITIATED_DETACH_WITH_AUTO_REATTACH";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_INITIATED_DETACH_WITH_AUTO_REATTACH;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_INITIATED_DETACH_NO_AUTO_REATTACH) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_INITIATED_DETACH_NO_AUTO_REATTACH)) {
        os += (first ? "" : " | ");
        os += "NETWORK_INITIATED_DETACH_NO_AUTO_REATTACH";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_INITIATED_DETACH_NO_AUTO_REATTACH;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ESM_PROCEDURE_TIME_OUT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ESM_PROCEDURE_TIME_OUT)) {
        os += (first ? "" : " | ");
        os += "ESM_PROCEDURE_TIME_OUT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ESM_PROCEDURE_TIME_OUT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_CONNECTION_ID) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::INVALID_CONNECTION_ID)) {
        os += (first ? "" : " | ");
        os += "INVALID_CONNECTION_ID";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_CONNECTION_ID;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MAXIMIUM_NSAPIS_EXCEEDED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MAXIMIUM_NSAPIS_EXCEEDED)) {
        os += (first ? "" : " | ");
        os += "MAXIMIUM_NSAPIS_EXCEEDED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MAXIMIUM_NSAPIS_EXCEEDED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_PRIMARY_NSAPI) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::INVALID_PRIMARY_NSAPI)) {
        os += (first ? "" : " | ");
        os += "INVALID_PRIMARY_NSAPI";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_PRIMARY_NSAPI;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CANNOT_ENCODE_OTA_MESSAGE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CANNOT_ENCODE_OTA_MESSAGE)) {
        os += (first ? "" : " | ");
        os += "CANNOT_ENCODE_OTA_MESSAGE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CANNOT_ENCODE_OTA_MESSAGE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RADIO_ACCESS_BEARER_SETUP_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RADIO_ACCESS_BEARER_SETUP_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RADIO_ACCESS_BEARER_SETUP_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RADIO_ACCESS_BEARER_SETUP_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDP_ESTABLISH_TIMEOUT_EXPIRED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDP_ESTABLISH_TIMEOUT_EXPIRED)) {
        os += (first ? "" : " | ");
        os += "PDP_ESTABLISH_TIMEOUT_EXPIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDP_ESTABLISH_TIMEOUT_EXPIRED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDP_MODIFY_TIMEOUT_EXPIRED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDP_MODIFY_TIMEOUT_EXPIRED)) {
        os += (first ? "" : " | ");
        os += "PDP_MODIFY_TIMEOUT_EXPIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDP_MODIFY_TIMEOUT_EXPIRED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDP_INACTIVE_TIMEOUT_EXPIRED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDP_INACTIVE_TIMEOUT_EXPIRED)) {
        os += (first ? "" : " | ");
        os += "PDP_INACTIVE_TIMEOUT_EXPIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDP_INACTIVE_TIMEOUT_EXPIRED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDP_LOWERLAYER_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDP_LOWERLAYER_ERROR)) {
        os += (first ? "" : " | ");
        os += "PDP_LOWERLAYER_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDP_LOWERLAYER_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PDP_MODIFY_COLLISION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PDP_MODIFY_COLLISION)) {
        os += (first ? "" : " | ");
        os += "PDP_MODIFY_COLLISION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PDP_MODIFY_COLLISION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MAXINUM_SIZE_OF_L2_MESSAGE_EXCEEDED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MAXINUM_SIZE_OF_L2_MESSAGE_EXCEEDED)) {
        os += (first ? "" : " | ");
        os += "MAXINUM_SIZE_OF_L2_MESSAGE_EXCEEDED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MAXINUM_SIZE_OF_L2_MESSAGE_EXCEEDED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NAS_REQUEST_REJECTED_BY_NETWORK) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NAS_REQUEST_REJECTED_BY_NETWORK)) {
        os += (first ? "" : " | ");
        os += "NAS_REQUEST_REJECTED_BY_NETWORK";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NAS_REQUEST_REJECTED_BY_NETWORK;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_INVALID_REQUEST) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_INVALID_REQUEST)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_INVALID_REQUEST";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_INVALID_REQUEST;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_TRACKING_AREA_ID_CHANGED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_TRACKING_AREA_ID_CHANGED)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_TRACKING_AREA_ID_CHANGED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_TRACKING_AREA_ID_CHANGED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_RF_UNAVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_RF_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_RF_UNAVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_RF_UNAVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_DUE_TO_IRAT_CHANGE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_DUE_TO_IRAT_CHANGE)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_ABORTED_DUE_TO_IRAT_CHANGE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_DUE_TO_IRAT_CHANGE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_RELEASED_SECURITY_NOT_ACTIVE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_RELEASED_SECURITY_NOT_ACTIVE)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_RELEASED_SECURITY_NOT_ACTIVE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_RELEASED_SECURITY_NOT_ACTIVE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_AFTER_HANDOVER) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_AFTER_HANDOVER)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_ABORTED_AFTER_HANDOVER";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_AFTER_HANDOVER;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_AFTER_IRAT_CELL_CHANGE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_AFTER_IRAT_CELL_CHANGE)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_ABORTED_AFTER_IRAT_CELL_CHANGE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_AFTER_IRAT_CELL_CHANGE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_DURING_IRAT_CELL_CHANGE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_DURING_IRAT_CELL_CHANGE)) {
        os += (first ? "" : " | ");
        os += "RRC_CONNECTION_ABORTED_DURING_IRAT_CELL_CHANGE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_DURING_IRAT_CELL_CHANGE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::IMSI_UNKNOWN_IN_HOME_SUBSCRIBER_SERVER) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::IMSI_UNKNOWN_IN_HOME_SUBSCRIBER_SERVER)) {
        os += (first ? "" : " | ");
        os += "IMSI_UNKNOWN_IN_HOME_SUBSCRIBER_SERVER";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::IMSI_UNKNOWN_IN_HOME_SUBSCRIBER_SERVER;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::IMEI_NOT_ACCEPTED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::IMEI_NOT_ACCEPTED)) {
        os += (first ? "" : " | ");
        os += "IMEI_NOT_ACCEPTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::IMEI_NOT_ACCEPTED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EPS_SERVICES_AND_NON_EPS_SERVICES_NOT_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EPS_SERVICES_AND_NON_EPS_SERVICES_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "EPS_SERVICES_AND_NON_EPS_SERVICES_NOT_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EPS_SERVICES_AND_NON_EPS_SERVICES_NOT_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EPS_SERVICES_NOT_ALLOWED_IN_PLMN) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EPS_SERVICES_NOT_ALLOWED_IN_PLMN)) {
        os += (first ? "" : " | ");
        os += "EPS_SERVICES_NOT_ALLOWED_IN_PLMN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EPS_SERVICES_NOT_ALLOWED_IN_PLMN;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MSC_TEMPORARILY_NOT_REACHABLE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MSC_TEMPORARILY_NOT_REACHABLE)) {
        os += (first ? "" : " | ");
        os += "MSC_TEMPORARILY_NOT_REACHABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MSC_TEMPORARILY_NOT_REACHABLE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CS_DOMAIN_NOT_AVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CS_DOMAIN_NOT_AVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CS_DOMAIN_NOT_AVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CS_DOMAIN_NOT_AVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ESM_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ESM_FAILURE)) {
        os += (first ? "" : " | ");
        os += "ESM_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ESM_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MAC_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MAC_FAILURE)) {
        os += (first ? "" : " | ");
        os += "MAC_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MAC_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::SYNCHRONIZATION_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::SYNCHRONIZATION_FAILURE)) {
        os += (first ? "" : " | ");
        os += "SYNCHRONIZATION_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::SYNCHRONIZATION_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::UE_SECURITY_CAPABILITIES_MISMATCH) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::UE_SECURITY_CAPABILITIES_MISMATCH)) {
        os += (first ? "" : " | ");
        os += "UE_SECURITY_CAPABILITIES_MISMATCH";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::UE_SECURITY_CAPABILITIES_MISMATCH;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::SECURITY_MODE_REJECTED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::SECURITY_MODE_REJECTED)) {
        os += (first ? "" : " | ");
        os += "SECURITY_MODE_REJECTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::SECURITY_MODE_REJECTED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::UNACCEPTABLE_NON_EPS_AUTHENTICATION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::UNACCEPTABLE_NON_EPS_AUTHENTICATION)) {
        os += (first ? "" : " | ");
        os += "UNACCEPTABLE_NON_EPS_AUTHENTICATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::UNACCEPTABLE_NON_EPS_AUTHENTICATION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::CS_FALLBACK_CALL_ESTABLISHMENT_NOT_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::CS_FALLBACK_CALL_ESTABLISHMENT_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "CS_FALLBACK_CALL_ESTABLISHMENT_NOT_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::CS_FALLBACK_CALL_ESTABLISHMENT_NOT_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NO_EPS_BEARER_CONTEXT_ACTIVATED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NO_EPS_BEARER_CONTEXT_ACTIVATED)) {
        os += (first ? "" : " | ");
        os += "NO_EPS_BEARER_CONTEXT_ACTIVATED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NO_EPS_BEARER_CONTEXT_ACTIVATED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_EMM_STATE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::INVALID_EMM_STATE)) {
        os += (first ? "" : " | ");
        os += "INVALID_EMM_STATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_EMM_STATE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NAS_LAYER_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NAS_LAYER_FAILURE)) {
        os += (first ? "" : " | ");
        os += "NAS_LAYER_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NAS_LAYER_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::MULTIPLE_PDP_CALL_NOT_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::MULTIPLE_PDP_CALL_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "MULTIPLE_PDP_CALL_NOT_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::MULTIPLE_PDP_CALL_NOT_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EMBMS_NOT_ENABLED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EMBMS_NOT_ENABLED)) {
        os += (first ? "" : " | ");
        os += "EMBMS_NOT_ENABLED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EMBMS_NOT_ENABLED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::IRAT_HANDOVER_FAILED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::IRAT_HANDOVER_FAILED)) {
        os += (first ? "" : " | ");
        os += "IRAT_HANDOVER_FAILED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::IRAT_HANDOVER_FAILED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EMBMS_REGULAR_DEACTIVATION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EMBMS_REGULAR_DEACTIVATION)) {
        os += (first ? "" : " | ");
        os += "EMBMS_REGULAR_DEACTIVATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EMBMS_REGULAR_DEACTIVATION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::TEST_LOOPBACK_REGULAR_DEACTIVATION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::TEST_LOOPBACK_REGULAR_DEACTIVATION)) {
        os += (first ? "" : " | ");
        os += "TEST_LOOPBACK_REGULAR_DEACTIVATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::TEST_LOOPBACK_REGULAR_DEACTIVATION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::LOWER_LAYER_REGISTRATION_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::LOWER_LAYER_REGISTRATION_FAILURE)) {
        os += (first ? "" : " | ");
        os += "LOWER_LAYER_REGISTRATION_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::LOWER_LAYER_REGISTRATION_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::DATA_PLAN_EXPIRED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::DATA_PLAN_EXPIRED)) {
        os += (first ? "" : " | ");
        os += "DATA_PLAN_EXPIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::DATA_PLAN_EXPIRED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::UMTS_HANDOVER_TO_IWLAN) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::UMTS_HANDOVER_TO_IWLAN)) {
        os += (first ? "" : " | ");
        os += "UMTS_HANDOVER_TO_IWLAN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::UMTS_HANDOVER_TO_IWLAN;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_CONNECTION_DENY_BY_GENERAL_OR_NETWORK_BUSY) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EVDO_CONNECTION_DENY_BY_GENERAL_OR_NETWORK_BUSY)) {
        os += (first ? "" : " | ");
        os += "EVDO_CONNECTION_DENY_BY_GENERAL_OR_NETWORK_BUSY";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_CONNECTION_DENY_BY_GENERAL_OR_NETWORK_BUSY;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_CONNECTION_DENY_BY_BILLING_OR_AUTHENTICATION_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EVDO_CONNECTION_DENY_BY_BILLING_OR_AUTHENTICATION_FAILURE)) {
        os += (first ? "" : " | ");
        os += "EVDO_CONNECTION_DENY_BY_BILLING_OR_AUTHENTICATION_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_CONNECTION_DENY_BY_BILLING_OR_AUTHENTICATION_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_CHANGED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_CHANGED)) {
        os += (first ? "" : " | ");
        os += "EVDO_HDR_CHANGED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_CHANGED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_EXITED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_EXITED)) {
        os += (first ? "" : " | ");
        os += "EVDO_HDR_EXITED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_EXITED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_NO_SESSION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_NO_SESSION)) {
        os += (first ? "" : " | ");
        os += "EVDO_HDR_NO_SESSION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_NO_SESSION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_USING_GPS_FIX_INSTEAD_OF_HDR_CALL) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EVDO_USING_GPS_FIX_INSTEAD_OF_HDR_CALL)) {
        os += (first ? "" : " | ");
        os += "EVDO_USING_GPS_FIX_INSTEAD_OF_HDR_CALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_USING_GPS_FIX_INSTEAD_OF_HDR_CALL;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_CONNECTION_SETUP_TIMEOUT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_CONNECTION_SETUP_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "EVDO_HDR_CONNECTION_SETUP_TIMEOUT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_CONNECTION_SETUP_TIMEOUT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::FAILED_TO_ACQUIRE_COLOCATED_HDR) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::FAILED_TO_ACQUIRE_COLOCATED_HDR)) {
        os += (first ? "" : " | ");
        os += "FAILED_TO_ACQUIRE_COLOCATED_HDR";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::FAILED_TO_ACQUIRE_COLOCATED_HDR;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::OTASP_COMMIT_IN_PROGRESS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::OTASP_COMMIT_IN_PROGRESS)) {
        os += (first ? "" : " | ");
        os += "OTASP_COMMIT_IN_PROGRESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::OTASP_COMMIT_IN_PROGRESS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NO_HYBRID_HDR_SERVICE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NO_HYBRID_HDR_SERVICE)) {
        os += (first ? "" : " | ");
        os += "NO_HYBRID_HDR_SERVICE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NO_HYBRID_HDR_SERVICE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::HDR_NO_LOCK_GRANTED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::HDR_NO_LOCK_GRANTED)) {
        os += (first ? "" : " | ");
        os += "HDR_NO_LOCK_GRANTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::HDR_NO_LOCK_GRANTED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::DBM_OR_SMS_IN_PROGRESS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::DBM_OR_SMS_IN_PROGRESS)) {
        os += (first ? "" : " | ");
        os += "DBM_OR_SMS_IN_PROGRESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::DBM_OR_SMS_IN_PROGRESS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::HDR_FADE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::HDR_FADE)) {
        os += (first ? "" : " | ");
        os += "HDR_FADE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::HDR_FADE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::HDR_ACCESS_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::HDR_ACCESS_FAILURE)) {
        os += (first ? "" : " | ");
        os += "HDR_ACCESS_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::HDR_ACCESS_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::UNSUPPORTED_1X_PREV) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::UNSUPPORTED_1X_PREV)) {
        os += (first ? "" : " | ");
        os += "UNSUPPORTED_1X_PREV";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::UNSUPPORTED_1X_PREV;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::LOCAL_END) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::LOCAL_END)) {
        os += (first ? "" : " | ");
        os += "LOCAL_END";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::LOCAL_END;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NO_SERVICE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NO_SERVICE)) {
        os += (first ? "" : " | ");
        os += "NO_SERVICE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NO_SERVICE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::FADE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::FADE)) {
        os += (first ? "" : " | ");
        os += "FADE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::FADE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NORMAL_RELEASE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NORMAL_RELEASE)) {
        os += (first ? "" : " | ");
        os += "NORMAL_RELEASE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NORMAL_RELEASE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_ATTEMPT_ALREADY_IN_PROGRESS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_ATTEMPT_ALREADY_IN_PROGRESS)) {
        os += (first ? "" : " | ");
        os += "ACCESS_ATTEMPT_ALREADY_IN_PROGRESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_ATTEMPT_ALREADY_IN_PROGRESS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::REDIRECTION_OR_HANDOFF_IN_PROGRESS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::REDIRECTION_OR_HANDOFF_IN_PROGRESS)) {
        os += (first ? "" : " | ");
        os += "REDIRECTION_OR_HANDOFF_IN_PROGRESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::REDIRECTION_OR_HANDOFF_IN_PROGRESS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::EMERGENCY_MODE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::EMERGENCY_MODE)) {
        os += (first ? "" : " | ");
        os += "EMERGENCY_MODE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::EMERGENCY_MODE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PHONE_IN_USE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PHONE_IN_USE)) {
        os += (first ? "" : " | ");
        os += "PHONE_IN_USE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PHONE_IN_USE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_MODE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::INVALID_MODE)) {
        os += (first ? "" : " | ");
        os += "INVALID_MODE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_MODE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_SIM_STATE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::INVALID_SIM_STATE)) {
        os += (first ? "" : " | ");
        os += "INVALID_SIM_STATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_SIM_STATE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::NO_COLLOCATED_HDR) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::NO_COLLOCATED_HDR)) {
        os += (first ? "" : " | ");
        os += "NO_COLLOCATED_HDR";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::NO_COLLOCATED_HDR;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::UE_IS_ENTERING_POWERSAVE_MODE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::UE_IS_ENTERING_POWERSAVE_MODE)) {
        os += (first ? "" : " | ");
        os += "UE_IS_ENTERING_POWERSAVE_MODE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::UE_IS_ENTERING_POWERSAVE_MODE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::DUAL_SWITCH) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::DUAL_SWITCH)) {
        os += (first ? "" : " | ");
        os += "DUAL_SWITCH";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::DUAL_SWITCH;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PPP_TIMEOUT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PPP_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "PPP_TIMEOUT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PPP_TIMEOUT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PPP_AUTH_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PPP_AUTH_FAILURE)) {
        os += (first ? "" : " | ");
        os += "PPP_AUTH_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PPP_AUTH_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PPP_OPTION_MISMATCH) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PPP_OPTION_MISMATCH)) {
        os += (first ? "" : " | ");
        os += "PPP_OPTION_MISMATCH";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PPP_OPTION_MISMATCH;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PPP_PAP_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PPP_PAP_FAILURE)) {
        os += (first ? "" : " | ");
        os += "PPP_PAP_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PPP_PAP_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PPP_CHAP_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PPP_CHAP_FAILURE)) {
        os += (first ? "" : " | ");
        os += "PPP_CHAP_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PPP_CHAP_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::PPP_CLOSE_IN_PROGRESS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::PPP_CLOSE_IN_PROGRESS)) {
        os += (first ? "" : " | ");
        os += "PPP_CLOSE_IN_PROGRESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::PPP_CLOSE_IN_PROGRESS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::LIMITED_TO_IPV4) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::LIMITED_TO_IPV4)) {
        os += (first ? "" : " | ");
        os += "LIMITED_TO_IPV4";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::LIMITED_TO_IPV4;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::LIMITED_TO_IPV6) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::LIMITED_TO_IPV6)) {
        os += (first ? "" : " | ");
        os += "LIMITED_TO_IPV6";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::LIMITED_TO_IPV6;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_TIMEOUT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "VSNCP_TIMEOUT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_TIMEOUT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_GEN_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_GEN_ERROR)) {
        os += (first ? "" : " | ");
        os += "VSNCP_GEN_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_GEN_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_APN_UNATHORIZED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_APN_UNATHORIZED)) {
        os += (first ? "" : " | ");
        os += "VSNCP_APN_UNATHORIZED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_APN_UNATHORIZED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_LIMIT_EXCEEDED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_LIMIT_EXCEEDED)) {
        os += (first ? "" : " | ");
        os += "VSNCP_PDN_LIMIT_EXCEEDED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_LIMIT_EXCEEDED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_NO_PDN_GATEWAY_ADDRESS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_NO_PDN_GATEWAY_ADDRESS)) {
        os += (first ? "" : " | ");
        os += "VSNCP_NO_PDN_GATEWAY_ADDRESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_NO_PDN_GATEWAY_ADDRESS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_GATEWAY_UNREACHABLE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_GATEWAY_UNREACHABLE)) {
        os += (first ? "" : " | ");
        os += "VSNCP_PDN_GATEWAY_UNREACHABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_GATEWAY_UNREACHABLE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_GATEWAY_REJECT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_GATEWAY_REJECT)) {
        os += (first ? "" : " | ");
        os += "VSNCP_PDN_GATEWAY_REJECT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_GATEWAY_REJECT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_INSUFFICIENT_PARAMETERS) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_INSUFFICIENT_PARAMETERS)) {
        os += (first ? "" : " | ");
        os += "VSNCP_INSUFFICIENT_PARAMETERS";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_INSUFFICIENT_PARAMETERS;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_RESOURCE_UNAVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_RESOURCE_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "VSNCP_RESOURCE_UNAVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_RESOURCE_UNAVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_ADMINISTRATIVELY_PROHIBITED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_ADMINISTRATIVELY_PROHIBITED)) {
        os += (first ? "" : " | ");
        os += "VSNCP_ADMINISTRATIVELY_PROHIBITED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_ADMINISTRATIVELY_PROHIBITED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_ID_IN_USE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_ID_IN_USE)) {
        os += (first ? "" : " | ");
        os += "VSNCP_PDN_ID_IN_USE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_ID_IN_USE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_SUBSCRIBER_LIMITATION) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_SUBSCRIBER_LIMITATION)) {
        os += (first ? "" : " | ");
        os += "VSNCP_SUBSCRIBER_LIMITATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_SUBSCRIBER_LIMITATION;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_EXISTS_FOR_THIS_APN) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_EXISTS_FOR_THIS_APN)) {
        os += (first ? "" : " | ");
        os += "VSNCP_PDN_EXISTS_FOR_THIS_APN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_EXISTS_FOR_THIS_APN;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_RECONNECT_NOT_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_RECONNECT_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "VSNCP_RECONNECT_NOT_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_RECONNECT_NOT_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::IPV6_PREFIX_UNAVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::IPV6_PREFIX_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "IPV6_PREFIX_UNAVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::IPV6_PREFIX_UNAVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataCallFailCause::HANDOFF_PREFERENCE_CHANGED) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataCallFailCause::HANDOFF_PREFERENCE_CHANGED)) {
        os += (first ? "" : " | ");
        os += "HANDOFF_PREFERENCE_CHANGED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataCallFailCause::HANDOFF_PREFERENCE_CHANGED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_4::DataCallFailCause o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NONE) {
        return "NONE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OPERATOR_BARRED) {
        return "OPERATOR_BARRED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NAS_SIGNALLING) {
        return "NAS_SIGNALLING";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::INSUFFICIENT_RESOURCES) {
        return "INSUFFICIENT_RESOURCES";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MISSING_UKNOWN_APN) {
        return "MISSING_UKNOWN_APN";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::UNKNOWN_PDP_ADDRESS_TYPE) {
        return "UNKNOWN_PDP_ADDRESS_TYPE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::USER_AUTHENTICATION) {
        return "USER_AUTHENTICATION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ACTIVATION_REJECT_GGSN) {
        return "ACTIVATION_REJECT_GGSN";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ACTIVATION_REJECT_UNSPECIFIED) {
        return "ACTIVATION_REJECT_UNSPECIFIED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_OPTION_NOT_SUPPORTED) {
        return "SERVICE_OPTION_NOT_SUPPORTED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_OPTION_NOT_SUBSCRIBED) {
        return "SERVICE_OPTION_NOT_SUBSCRIBED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_OPTION_OUT_OF_ORDER) {
        return "SERVICE_OPTION_OUT_OF_ORDER";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NSAPI_IN_USE) {
        return "NSAPI_IN_USE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::REGULAR_DEACTIVATION) {
        return "REGULAR_DEACTIVATION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::QOS_NOT_ACCEPTED) {
        return "QOS_NOT_ACCEPTED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_FAILURE) {
        return "NETWORK_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::UMTS_REACTIVATION_REQ) {
        return "UMTS_REACTIVATION_REQ";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::FEATURE_NOT_SUPP) {
        return "FEATURE_NOT_SUPP";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::TFT_SEMANTIC_ERROR) {
        return "TFT_SEMANTIC_ERROR";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::TFT_SYTAX_ERROR) {
        return "TFT_SYTAX_ERROR";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::UNKNOWN_PDP_CONTEXT) {
        return "UNKNOWN_PDP_CONTEXT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::FILTER_SEMANTIC_ERROR) {
        return "FILTER_SEMANTIC_ERROR";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::FILTER_SYTAX_ERROR) {
        return "FILTER_SYTAX_ERROR";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDP_WITHOUT_ACTIVE_TFT) {
        return "PDP_WITHOUT_ACTIVE_TFT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_IPV4_ALLOWED) {
        return "ONLY_IPV4_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_IPV6_ALLOWED) {
        return "ONLY_IPV6_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_SINGLE_BEARER_ALLOWED) {
        return "ONLY_SINGLE_BEARER_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ESM_INFO_NOT_RECEIVED) {
        return "ESM_INFO_NOT_RECEIVED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDN_CONN_DOES_NOT_EXIST) {
        return "PDN_CONN_DOES_NOT_EXIST";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED) {
        return "MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MAX_ACTIVE_PDP_CONTEXT_REACHED) {
        return "MAX_ACTIVE_PDP_CONTEXT_REACHED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::UNSUPPORTED_APN_IN_CURRENT_PLMN) {
        return "UNSUPPORTED_APN_IN_CURRENT_PLMN";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_TRANSACTION_ID) {
        return "INVALID_TRANSACTION_ID";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MESSAGE_INCORRECT_SEMANTIC) {
        return "MESSAGE_INCORRECT_SEMANTIC";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_MANDATORY_INFO) {
        return "INVALID_MANDATORY_INFO";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MESSAGE_TYPE_UNSUPPORTED) {
        return "MESSAGE_TYPE_UNSUPPORTED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MSG_TYPE_NONCOMPATIBLE_STATE) {
        return "MSG_TYPE_NONCOMPATIBLE_STATE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::UNKNOWN_INFO_ELEMENT) {
        return "UNKNOWN_INFO_ELEMENT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CONDITIONAL_IE_ERROR) {
        return "CONDITIONAL_IE_ERROR";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE) {
        return "MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PROTOCOL_ERRORS) {
        return "PROTOCOL_ERRORS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::APN_TYPE_CONFLICT) {
        return "APN_TYPE_CONFLICT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_PCSCF_ADDR) {
        return "INVALID_PCSCF_ADDR";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN) {
        return "INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EMM_ACCESS_BARRED) {
        return "EMM_ACCESS_BARRED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EMERGENCY_IFACE_ONLY) {
        return "EMERGENCY_IFACE_ONLY";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::IFACE_MISMATCH) {
        return "IFACE_MISMATCH";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::COMPANION_IFACE_IN_USE) {
        return "COMPANION_IFACE_IN_USE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::IP_ADDRESS_MISMATCH) {
        return "IP_ADDRESS_MISMATCH";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::IFACE_AND_POL_FAMILY_MISMATCH) {
        return "IFACE_AND_POL_FAMILY_MISMATCH";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EMM_ACCESS_BARRED_INFINITE_RETRY) {
        return "EMM_ACCESS_BARRED_INFINITE_RETRY";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::AUTH_FAILURE_ON_EMERGENCY_CALL) {
        return "AUTH_FAILURE_ON_EMERGENCY_CALL";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_1) {
        return "OEM_DCFAILCAUSE_1";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_2) {
        return "OEM_DCFAILCAUSE_2";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_3) {
        return "OEM_DCFAILCAUSE_3";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_4) {
        return "OEM_DCFAILCAUSE_4";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_5) {
        return "OEM_DCFAILCAUSE_5";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_6) {
        return "OEM_DCFAILCAUSE_6";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_7) {
        return "OEM_DCFAILCAUSE_7";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_8) {
        return "OEM_DCFAILCAUSE_8";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_9) {
        return "OEM_DCFAILCAUSE_9";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_10) {
        return "OEM_DCFAILCAUSE_10";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_11) {
        return "OEM_DCFAILCAUSE_11";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_12) {
        return "OEM_DCFAILCAUSE_12";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_13) {
        return "OEM_DCFAILCAUSE_13";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_14) {
        return "OEM_DCFAILCAUSE_14";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_15) {
        return "OEM_DCFAILCAUSE_15";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::VOICE_REGISTRATION_FAIL) {
        return "VOICE_REGISTRATION_FAIL";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::DATA_REGISTRATION_FAIL) {
        return "DATA_REGISTRATION_FAIL";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::SIGNAL_LOST) {
        return "SIGNAL_LOST";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PREF_RADIO_TECH_CHANGED) {
        return "PREF_RADIO_TECH_CHANGED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RADIO_POWER_OFF) {
        return "RADIO_POWER_OFF";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::TETHERED_CALL_ACTIVE) {
        return "TETHERED_CALL_ACTIVE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ERROR_UNSPECIFIED) {
        return "ERROR_UNSPECIFIED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::LLC_SNDCP) {
        return "LLC_SNDCP";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ACTIVATION_REJECTED_BCM_VIOLATION) {
        return "ACTIVATION_REJECTED_BCM_VIOLATION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::COLLISION_WITH_NETWORK_INITIATED_REQUEST) {
        return "COLLISION_WITH_NETWORK_INITIATED_REQUEST";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_IPV4V6_ALLOWED) {
        return "ONLY_IPV4V6_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_NON_IP_ALLOWED) {
        return "ONLY_NON_IP_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::UNSUPPORTED_QCI_VALUE) {
        return "UNSUPPORTED_QCI_VALUE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::BEARER_HANDLING_NOT_SUPPORTED) {
        return "BEARER_HANDLING_NOT_SUPPORTED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_DNS_ADDR) {
        return "INVALID_DNS_ADDR";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_PCSCF_OR_DNS_ADDRESS) {
        return "INVALID_PCSCF_OR_DNS_ADDRESS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CALL_PREEMPT_BY_EMERGENCY_APN) {
        return "CALL_PREEMPT_BY_EMERGENCY_APN";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::UE_INITIATED_DETACH_OR_DISCONNECT) {
        return "UE_INITIATED_DETACH_OR_DISCONNECT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REASON_UNSPECIFIED) {
        return "MIP_FA_REASON_UNSPECIFIED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_ADMIN_PROHIBITED) {
        return "MIP_FA_ADMIN_PROHIBITED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_INSUFFICIENT_RESOURCES) {
        return "MIP_FA_INSUFFICIENT_RESOURCES";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MOBILE_NODE_AUTHENTICATION_FAILURE) {
        return "MIP_FA_MOBILE_NODE_AUTHENTICATION_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_HOME_AGENT_AUTHENTICATION_FAILURE) {
        return "MIP_FA_HOME_AGENT_AUTHENTICATION_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REQUESTED_LIFETIME_TOO_LONG) {
        return "MIP_FA_REQUESTED_LIFETIME_TOO_LONG";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MALFORMED_REQUEST) {
        return "MIP_FA_MALFORMED_REQUEST";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MALFORMED_REPLY) {
        return "MIP_FA_MALFORMED_REPLY";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_ENCAPSULATION_UNAVAILABLE) {
        return "MIP_FA_ENCAPSULATION_UNAVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_VJ_HEADER_COMPRESSION_UNAVAILABLE) {
        return "MIP_FA_VJ_HEADER_COMPRESSION_UNAVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REVERSE_TUNNEL_UNAVAILABLE) {
        return "MIP_FA_REVERSE_TUNNEL_UNAVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REVERSE_TUNNEL_IS_MANDATORY) {
        return "MIP_FA_REVERSE_TUNNEL_IS_MANDATORY";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_DELIVERY_STYLE_NOT_SUPPORTED) {
        return "MIP_FA_DELIVERY_STYLE_NOT_SUPPORTED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_NAI) {
        return "MIP_FA_MISSING_NAI";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_HOME_AGENT) {
        return "MIP_FA_MISSING_HOME_AGENT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_HOME_ADDRESS) {
        return "MIP_FA_MISSING_HOME_ADDRESS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_UNKNOWN_CHALLENGE) {
        return "MIP_FA_UNKNOWN_CHALLENGE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_CHALLENGE) {
        return "MIP_FA_MISSING_CHALLENGE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_STALE_CHALLENGE) {
        return "MIP_FA_STALE_CHALLENGE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REASON_UNSPECIFIED) {
        return "MIP_HA_REASON_UNSPECIFIED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_ADMIN_PROHIBITED) {
        return "MIP_HA_ADMIN_PROHIBITED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_INSUFFICIENT_RESOURCES) {
        return "MIP_HA_INSUFFICIENT_RESOURCES";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_MOBILE_NODE_AUTHENTICATION_FAILURE) {
        return "MIP_HA_MOBILE_NODE_AUTHENTICATION_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_FOREIGN_AGENT_AUTHENTICATION_FAILURE) {
        return "MIP_HA_FOREIGN_AGENT_AUTHENTICATION_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REGISTRATION_ID_MISMATCH) {
        return "MIP_HA_REGISTRATION_ID_MISMATCH";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_MALFORMED_REQUEST) {
        return "MIP_HA_MALFORMED_REQUEST";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_UNKNOWN_HOME_AGENT_ADDRESS) {
        return "MIP_HA_UNKNOWN_HOME_AGENT_ADDRESS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REVERSE_TUNNEL_UNAVAILABLE) {
        return "MIP_HA_REVERSE_TUNNEL_UNAVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REVERSE_TUNNEL_IS_MANDATORY) {
        return "MIP_HA_REVERSE_TUNNEL_IS_MANDATORY";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_ENCAPSULATION_UNAVAILABLE) {
        return "MIP_HA_ENCAPSULATION_UNAVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CLOSE_IN_PROGRESS) {
        return "CLOSE_IN_PROGRESS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_INITIATED_TERMINATION) {
        return "NETWORK_INITIATED_TERMINATION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MODEM_APP_PREEMPTED) {
        return "MODEM_APP_PREEMPTED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV4_CALL_DISALLOWED) {
        return "PDN_IPV4_CALL_DISALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV4_CALL_THROTTLED) {
        return "PDN_IPV4_CALL_THROTTLED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV6_CALL_DISALLOWED) {
        return "PDN_IPV6_CALL_DISALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV6_CALL_THROTTLED) {
        return "PDN_IPV6_CALL_THROTTLED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MODEM_RESTART) {
        return "MODEM_RESTART";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDP_PPP_NOT_SUPPORTED) {
        return "PDP_PPP_NOT_SUPPORTED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::UNPREFERRED_RAT) {
        return "UNPREFERRED_RAT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PHYSICAL_LINK_CLOSE_IN_PROGRESS) {
        return "PHYSICAL_LINK_CLOSE_IN_PROGRESS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::APN_PENDING_HANDOVER) {
        return "APN_PENDING_HANDOVER";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PROFILE_BEARER_INCOMPATIBLE) {
        return "PROFILE_BEARER_INCOMPATIBLE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::SIM_CARD_CHANGED) {
        return "SIM_CARD_CHANGED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::LOW_POWER_MODE_OR_POWERING_DOWN) {
        return "LOW_POWER_MODE_OR_POWERING_DOWN";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::APN_DISABLED) {
        return "APN_DISABLED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MAX_PPP_INACTIVITY_TIMER_EXPIRED) {
        return "MAX_PPP_INACTIVITY_TIMER_EXPIRED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::IPV6_ADDRESS_TRANSFER_FAILED) {
        return "IPV6_ADDRESS_TRANSFER_FAILED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::TRAT_SWAP_FAILED) {
        return "TRAT_SWAP_FAILED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EHRPD_TO_HRPD_FALLBACK) {
        return "EHRPD_TO_HRPD_FALLBACK";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MIP_CONFIG_FAILURE) {
        return "MIP_CONFIG_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDN_INACTIVITY_TIMER_EXPIRED) {
        return "PDN_INACTIVITY_TIMER_EXPIRED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MAX_IPV4_CONNECTIONS) {
        return "MAX_IPV4_CONNECTIONS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MAX_IPV6_CONNECTIONS) {
        return "MAX_IPV6_CONNECTIONS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::APN_MISMATCH) {
        return "APN_MISMATCH";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::IP_VERSION_MISMATCH) {
        return "IP_VERSION_MISMATCH";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::DUN_CALL_DISALLOWED) {
        return "DUN_CALL_DISALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::INTERNAL_EPC_NONEPC_TRANSITION) {
        return "INTERNAL_EPC_NONEPC_TRANSITION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::INTERFACE_IN_USE) {
        return "INTERFACE_IN_USE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::APN_DISALLOWED_ON_ROAMING) {
        return "APN_DISALLOWED_ON_ROAMING";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::APN_PARAMETERS_CHANGED) {
        return "APN_PARAMETERS_CHANGED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NULL_APN_DISALLOWED) {
        return "NULL_APN_DISALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::THERMAL_MITIGATION) {
        return "THERMAL_MITIGATION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::DATA_SETTINGS_DISABLED) {
        return "DATA_SETTINGS_DISABLED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::DATA_ROAMING_SETTINGS_DISABLED) {
        return "DATA_ROAMING_SETTINGS_DISABLED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::DDS_SWITCHED) {
        return "DDS_SWITCHED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::FORBIDDEN_APN_NAME) {
        return "FORBIDDEN_APN_NAME";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::DDS_SWITCH_IN_PROGRESS) {
        return "DDS_SWITCH_IN_PROGRESS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CALL_DISALLOWED_IN_ROAMING) {
        return "CALL_DISALLOWED_IN_ROAMING";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NON_IP_NOT_SUPPORTED) {
        return "NON_IP_NOT_SUPPORTED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDN_NON_IP_CALL_THROTTLED) {
        return "PDN_NON_IP_CALL_THROTTLED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDN_NON_IP_CALL_DISALLOWED) {
        return "PDN_NON_IP_CALL_DISALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_LOCK) {
        return "CDMA_LOCK";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_INTERCEPT) {
        return "CDMA_INTERCEPT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_REORDER) {
        return "CDMA_REORDER";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_RELEASE_DUE_TO_SO_REJECTION) {
        return "CDMA_RELEASE_DUE_TO_SO_REJECTION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_INCOMING_CALL) {
        return "CDMA_INCOMING_CALL";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_ALERT_STOP) {
        return "CDMA_ALERT_STOP";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CHANNEL_ACQUISITION_FAILURE) {
        return "CHANNEL_ACQUISITION_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MAX_ACCESS_PROBE) {
        return "MAX_ACCESS_PROBE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CONCURRENT_SERVICE_NOT_SUPPORTED_BY_BASE_STATION) {
        return "CONCURRENT_SERVICE_NOT_SUPPORTED_BY_BASE_STATION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NO_RESPONSE_FROM_BASE_STATION) {
        return "NO_RESPONSE_FROM_BASE_STATION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::REJECTED_BY_BASE_STATION) {
        return "REJECTED_BY_BASE_STATION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CONCURRENT_SERVICES_INCOMPATIBLE) {
        return "CONCURRENT_SERVICES_INCOMPATIBLE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NO_CDMA_SERVICE) {
        return "NO_CDMA_SERVICE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RUIM_NOT_PRESENT) {
        return "RUIM_NOT_PRESENT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_RETRY_ORDER) {
        return "CDMA_RETRY_ORDER";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_BLOCK) {
        return "ACCESS_BLOCK";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_BLOCK_ALL) {
        return "ACCESS_BLOCK_ALL";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::IS707B_MAX_ACCESS_PROBES) {
        return "IS707B_MAX_ACCESS_PROBES";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::THERMAL_EMERGENCY) {
        return "THERMAL_EMERGENCY";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CONCURRENT_SERVICES_NOT_ALLOWED) {
        return "CONCURRENT_SERVICES_NOT_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::INCOMING_CALL_REJECTED) {
        return "INCOMING_CALL_REJECTED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NO_SERVICE_ON_GATEWAY) {
        return "NO_SERVICE_ON_GATEWAY";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NO_GPRS_CONTEXT) {
        return "NO_GPRS_CONTEXT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ILLEGAL_MS) {
        return "ILLEGAL_MS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ILLEGAL_ME) {
        return "ILLEGAL_ME";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::GPRS_SERVICES_AND_NON_GPRS_SERVICES_NOT_ALLOWED) {
        return "GPRS_SERVICES_AND_NON_GPRS_SERVICES_NOT_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::GPRS_SERVICES_NOT_ALLOWED) {
        return "GPRS_SERVICES_NOT_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MS_IDENTITY_CANNOT_BE_DERIVED_BY_THE_NETWORK) {
        return "MS_IDENTITY_CANNOT_BE_DERIVED_BY_THE_NETWORK";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::IMPLICITLY_DETACHED) {
        return "IMPLICITLY_DETACHED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PLMN_NOT_ALLOWED) {
        return "PLMN_NOT_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::LOCATION_AREA_NOT_ALLOWED) {
        return "LOCATION_AREA_NOT_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::GPRS_SERVICES_NOT_ALLOWED_IN_THIS_PLMN) {
        return "GPRS_SERVICES_NOT_ALLOWED_IN_THIS_PLMN";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDP_DUPLICATE) {
        return "PDP_DUPLICATE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::UE_RAT_CHANGE) {
        return "UE_RAT_CHANGE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CONGESTION) {
        return "CONGESTION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NO_PDP_CONTEXT_ACTIVATED) {
        return "NO_PDP_CONTEXT_ACTIVATED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_CLASS_DSAC_REJECTION) {
        return "ACCESS_CLASS_DSAC_REJECTION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDP_ACTIVATE_MAX_RETRY_FAILED) {
        return "PDP_ACTIVATE_MAX_RETRY_FAILED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RADIO_ACCESS_BEARER_FAILURE) {
        return "RADIO_ACCESS_BEARER_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ESM_UNKNOWN_EPS_BEARER_CONTEXT) {
        return "ESM_UNKNOWN_EPS_BEARER_CONTEXT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::DRB_RELEASED_BY_RRC) {
        return "DRB_RELEASED_BY_RRC";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CONNECTION_RELEASED) {
        return "CONNECTION_RELEASED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EMM_DETACHED) {
        return "EMM_DETACHED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EMM_ATTACH_FAILED) {
        return "EMM_ATTACH_FAILED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EMM_ATTACH_STARTED) {
        return "EMM_ATTACH_STARTED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::LTE_NAS_SERVICE_REQUEST_FAILED) {
        return "LTE_NAS_SERVICE_REQUEST_FAILED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::DUPLICATE_BEARER_ID) {
        return "DUPLICATE_BEARER_ID";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ESM_COLLISION_SCENARIOS) {
        return "ESM_COLLISION_SCENARIOS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ESM_BEARER_DEACTIVATED_TO_SYNC_WITH_NETWORK) {
        return "ESM_BEARER_DEACTIVATED_TO_SYNC_WITH_NETWORK";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ESM_NW_ACTIVATED_DED_BEARER_WITH_ID_OF_DEF_BEARER) {
        return "ESM_NW_ACTIVATED_DED_BEARER_WITH_ID_OF_DEF_BEARER";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ESM_BAD_OTA_MESSAGE) {
        return "ESM_BAD_OTA_MESSAGE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ESM_DOWNLOAD_SERVER_REJECTED_THE_CALL) {
        return "ESM_DOWNLOAD_SERVER_REJECTED_THE_CALL";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ESM_CONTEXT_TRANSFERRED_DUE_TO_IRAT) {
        return "ESM_CONTEXT_TRANSFERRED_DUE_TO_IRAT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::DS_EXPLICIT_DEACTIVATION) {
        return "DS_EXPLICIT_DEACTIVATION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ESM_LOCAL_CAUSE_NONE) {
        return "ESM_LOCAL_CAUSE_NONE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::LTE_THROTTLING_NOT_REQUIRED) {
        return "LTE_THROTTLING_NOT_REQUIRED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_CONTROL_LIST_CHECK_FAILURE) {
        return "ACCESS_CONTROL_LIST_CHECK_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_NOT_ALLOWED_ON_PLMN) {
        return "SERVICE_NOT_ALLOWED_ON_PLMN";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EMM_T3417_EXPIRED) {
        return "EMM_T3417_EXPIRED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EMM_T3417_EXT_EXPIRED) {
        return "EMM_T3417_EXT_EXPIRED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_DATA_TRANSMISSION_FAILURE) {
        return "RRC_UPLINK_DATA_TRANSMISSION_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_DELIVERY_FAILED_DUE_TO_HANDOVER) {
        return "RRC_UPLINK_DELIVERY_FAILED_DUE_TO_HANDOVER";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_CONNECTION_RELEASE) {
        return "RRC_UPLINK_CONNECTION_RELEASE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_RADIO_LINK_FAILURE) {
        return "RRC_UPLINK_RADIO_LINK_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_ERROR_REQUEST_FROM_NAS) {
        return "RRC_UPLINK_ERROR_REQUEST_FROM_NAS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ACCESS_STRATUM_FAILURE) {
        return "RRC_CONNECTION_ACCESS_STRATUM_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ANOTHER_PROCEDURE_IN_PROGRESS) {
        return "RRC_CONNECTION_ANOTHER_PROCEDURE_IN_PROGRESS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ACCESS_BARRED) {
        return "RRC_CONNECTION_ACCESS_BARRED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_CELL_RESELECTION) {
        return "RRC_CONNECTION_CELL_RESELECTION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_CONFIG_FAILURE) {
        return "RRC_CONNECTION_CONFIG_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_TIMER_EXPIRED) {
        return "RRC_CONNECTION_TIMER_EXPIRED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_LINK_FAILURE) {
        return "RRC_CONNECTION_LINK_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_CELL_NOT_CAMPED) {
        return "RRC_CONNECTION_CELL_NOT_CAMPED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_SYSTEM_INTERVAL_FAILURE) {
        return "RRC_CONNECTION_SYSTEM_INTERVAL_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_REJECT_BY_NETWORK) {
        return "RRC_CONNECTION_REJECT_BY_NETWORK";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_NORMAL_RELEASE) {
        return "RRC_CONNECTION_NORMAL_RELEASE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_RADIO_LINK_FAILURE) {
        return "RRC_CONNECTION_RADIO_LINK_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_REESTABLISHMENT_FAILURE) {
        return "RRC_CONNECTION_REESTABLISHMENT_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_OUT_OF_SERVICE_DURING_CELL_REGISTER) {
        return "RRC_CONNECTION_OUT_OF_SERVICE_DURING_CELL_REGISTER";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORT_REQUEST) {
        return "RRC_CONNECTION_ABORT_REQUEST";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_SYSTEM_INFORMATION_BLOCK_READ_ERROR) {
        return "RRC_CONNECTION_SYSTEM_INFORMATION_BLOCK_READ_ERROR";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_INITIATED_DETACH_WITH_AUTO_REATTACH) {
        return "NETWORK_INITIATED_DETACH_WITH_AUTO_REATTACH";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_INITIATED_DETACH_NO_AUTO_REATTACH) {
        return "NETWORK_INITIATED_DETACH_NO_AUTO_REATTACH";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ESM_PROCEDURE_TIME_OUT) {
        return "ESM_PROCEDURE_TIME_OUT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_CONNECTION_ID) {
        return "INVALID_CONNECTION_ID";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MAXIMIUM_NSAPIS_EXCEEDED) {
        return "MAXIMIUM_NSAPIS_EXCEEDED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_PRIMARY_NSAPI) {
        return "INVALID_PRIMARY_NSAPI";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CANNOT_ENCODE_OTA_MESSAGE) {
        return "CANNOT_ENCODE_OTA_MESSAGE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RADIO_ACCESS_BEARER_SETUP_FAILURE) {
        return "RADIO_ACCESS_BEARER_SETUP_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDP_ESTABLISH_TIMEOUT_EXPIRED) {
        return "PDP_ESTABLISH_TIMEOUT_EXPIRED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDP_MODIFY_TIMEOUT_EXPIRED) {
        return "PDP_MODIFY_TIMEOUT_EXPIRED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDP_INACTIVE_TIMEOUT_EXPIRED) {
        return "PDP_INACTIVE_TIMEOUT_EXPIRED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDP_LOWERLAYER_ERROR) {
        return "PDP_LOWERLAYER_ERROR";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PDP_MODIFY_COLLISION) {
        return "PDP_MODIFY_COLLISION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MAXINUM_SIZE_OF_L2_MESSAGE_EXCEEDED) {
        return "MAXINUM_SIZE_OF_L2_MESSAGE_EXCEEDED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NAS_REQUEST_REJECTED_BY_NETWORK) {
        return "NAS_REQUEST_REJECTED_BY_NETWORK";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_INVALID_REQUEST) {
        return "RRC_CONNECTION_INVALID_REQUEST";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_TRACKING_AREA_ID_CHANGED) {
        return "RRC_CONNECTION_TRACKING_AREA_ID_CHANGED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_RF_UNAVAILABLE) {
        return "RRC_CONNECTION_RF_UNAVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_DUE_TO_IRAT_CHANGE) {
        return "RRC_CONNECTION_ABORTED_DUE_TO_IRAT_CHANGE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_RELEASED_SECURITY_NOT_ACTIVE) {
        return "RRC_CONNECTION_RELEASED_SECURITY_NOT_ACTIVE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_AFTER_HANDOVER) {
        return "RRC_CONNECTION_ABORTED_AFTER_HANDOVER";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_AFTER_IRAT_CELL_CHANGE) {
        return "RRC_CONNECTION_ABORTED_AFTER_IRAT_CELL_CHANGE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_DURING_IRAT_CELL_CHANGE) {
        return "RRC_CONNECTION_ABORTED_DURING_IRAT_CELL_CHANGE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::IMSI_UNKNOWN_IN_HOME_SUBSCRIBER_SERVER) {
        return "IMSI_UNKNOWN_IN_HOME_SUBSCRIBER_SERVER";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::IMEI_NOT_ACCEPTED) {
        return "IMEI_NOT_ACCEPTED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EPS_SERVICES_AND_NON_EPS_SERVICES_NOT_ALLOWED) {
        return "EPS_SERVICES_AND_NON_EPS_SERVICES_NOT_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EPS_SERVICES_NOT_ALLOWED_IN_PLMN) {
        return "EPS_SERVICES_NOT_ALLOWED_IN_PLMN";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MSC_TEMPORARILY_NOT_REACHABLE) {
        return "MSC_TEMPORARILY_NOT_REACHABLE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CS_DOMAIN_NOT_AVAILABLE) {
        return "CS_DOMAIN_NOT_AVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ESM_FAILURE) {
        return "ESM_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MAC_FAILURE) {
        return "MAC_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::SYNCHRONIZATION_FAILURE) {
        return "SYNCHRONIZATION_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::UE_SECURITY_CAPABILITIES_MISMATCH) {
        return "UE_SECURITY_CAPABILITIES_MISMATCH";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::SECURITY_MODE_REJECTED) {
        return "SECURITY_MODE_REJECTED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::UNACCEPTABLE_NON_EPS_AUTHENTICATION) {
        return "UNACCEPTABLE_NON_EPS_AUTHENTICATION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::CS_FALLBACK_CALL_ESTABLISHMENT_NOT_ALLOWED) {
        return "CS_FALLBACK_CALL_ESTABLISHMENT_NOT_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NO_EPS_BEARER_CONTEXT_ACTIVATED) {
        return "NO_EPS_BEARER_CONTEXT_ACTIVATED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_EMM_STATE) {
        return "INVALID_EMM_STATE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NAS_LAYER_FAILURE) {
        return "NAS_LAYER_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::MULTIPLE_PDP_CALL_NOT_ALLOWED) {
        return "MULTIPLE_PDP_CALL_NOT_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EMBMS_NOT_ENABLED) {
        return "EMBMS_NOT_ENABLED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::IRAT_HANDOVER_FAILED) {
        return "IRAT_HANDOVER_FAILED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EMBMS_REGULAR_DEACTIVATION) {
        return "EMBMS_REGULAR_DEACTIVATION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::TEST_LOOPBACK_REGULAR_DEACTIVATION) {
        return "TEST_LOOPBACK_REGULAR_DEACTIVATION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::LOWER_LAYER_REGISTRATION_FAILURE) {
        return "LOWER_LAYER_REGISTRATION_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::DATA_PLAN_EXPIRED) {
        return "DATA_PLAN_EXPIRED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::UMTS_HANDOVER_TO_IWLAN) {
        return "UMTS_HANDOVER_TO_IWLAN";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_CONNECTION_DENY_BY_GENERAL_OR_NETWORK_BUSY) {
        return "EVDO_CONNECTION_DENY_BY_GENERAL_OR_NETWORK_BUSY";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_CONNECTION_DENY_BY_BILLING_OR_AUTHENTICATION_FAILURE) {
        return "EVDO_CONNECTION_DENY_BY_BILLING_OR_AUTHENTICATION_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_CHANGED) {
        return "EVDO_HDR_CHANGED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_EXITED) {
        return "EVDO_HDR_EXITED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_NO_SESSION) {
        return "EVDO_HDR_NO_SESSION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_USING_GPS_FIX_INSTEAD_OF_HDR_CALL) {
        return "EVDO_USING_GPS_FIX_INSTEAD_OF_HDR_CALL";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_CONNECTION_SETUP_TIMEOUT) {
        return "EVDO_HDR_CONNECTION_SETUP_TIMEOUT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::FAILED_TO_ACQUIRE_COLOCATED_HDR) {
        return "FAILED_TO_ACQUIRE_COLOCATED_HDR";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::OTASP_COMMIT_IN_PROGRESS) {
        return "OTASP_COMMIT_IN_PROGRESS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NO_HYBRID_HDR_SERVICE) {
        return "NO_HYBRID_HDR_SERVICE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::HDR_NO_LOCK_GRANTED) {
        return "HDR_NO_LOCK_GRANTED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::DBM_OR_SMS_IN_PROGRESS) {
        return "DBM_OR_SMS_IN_PROGRESS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::HDR_FADE) {
        return "HDR_FADE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::HDR_ACCESS_FAILURE) {
        return "HDR_ACCESS_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::UNSUPPORTED_1X_PREV) {
        return "UNSUPPORTED_1X_PREV";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::LOCAL_END) {
        return "LOCAL_END";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NO_SERVICE) {
        return "NO_SERVICE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::FADE) {
        return "FADE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NORMAL_RELEASE) {
        return "NORMAL_RELEASE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_ATTEMPT_ALREADY_IN_PROGRESS) {
        return "ACCESS_ATTEMPT_ALREADY_IN_PROGRESS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::REDIRECTION_OR_HANDOFF_IN_PROGRESS) {
        return "REDIRECTION_OR_HANDOFF_IN_PROGRESS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::EMERGENCY_MODE) {
        return "EMERGENCY_MODE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PHONE_IN_USE) {
        return "PHONE_IN_USE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_MODE) {
        return "INVALID_MODE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_SIM_STATE) {
        return "INVALID_SIM_STATE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::NO_COLLOCATED_HDR) {
        return "NO_COLLOCATED_HDR";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::UE_IS_ENTERING_POWERSAVE_MODE) {
        return "UE_IS_ENTERING_POWERSAVE_MODE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::DUAL_SWITCH) {
        return "DUAL_SWITCH";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PPP_TIMEOUT) {
        return "PPP_TIMEOUT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PPP_AUTH_FAILURE) {
        return "PPP_AUTH_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PPP_OPTION_MISMATCH) {
        return "PPP_OPTION_MISMATCH";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PPP_PAP_FAILURE) {
        return "PPP_PAP_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PPP_CHAP_FAILURE) {
        return "PPP_CHAP_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::PPP_CLOSE_IN_PROGRESS) {
        return "PPP_CLOSE_IN_PROGRESS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::LIMITED_TO_IPV4) {
        return "LIMITED_TO_IPV4";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::LIMITED_TO_IPV6) {
        return "LIMITED_TO_IPV6";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_TIMEOUT) {
        return "VSNCP_TIMEOUT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_GEN_ERROR) {
        return "VSNCP_GEN_ERROR";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_APN_UNATHORIZED) {
        return "VSNCP_APN_UNATHORIZED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_LIMIT_EXCEEDED) {
        return "VSNCP_PDN_LIMIT_EXCEEDED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_NO_PDN_GATEWAY_ADDRESS) {
        return "VSNCP_NO_PDN_GATEWAY_ADDRESS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_GATEWAY_UNREACHABLE) {
        return "VSNCP_PDN_GATEWAY_UNREACHABLE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_GATEWAY_REJECT) {
        return "VSNCP_PDN_GATEWAY_REJECT";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_INSUFFICIENT_PARAMETERS) {
        return "VSNCP_INSUFFICIENT_PARAMETERS";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_RESOURCE_UNAVAILABLE) {
        return "VSNCP_RESOURCE_UNAVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_ADMINISTRATIVELY_PROHIBITED) {
        return "VSNCP_ADMINISTRATIVELY_PROHIBITED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_ID_IN_USE) {
        return "VSNCP_PDN_ID_IN_USE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_SUBSCRIBER_LIMITATION) {
        return "VSNCP_SUBSCRIBER_LIMITATION";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_EXISTS_FOR_THIS_APN) {
        return "VSNCP_PDN_EXISTS_FOR_THIS_APN";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_RECONNECT_NOT_ALLOWED) {
        return "VSNCP_RECONNECT_NOT_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::IPV6_PREFIX_UNAVAILABLE) {
        return "IPV6_PREFIX_UNAVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_4::DataCallFailCause::HANDOFF_PREFERENCE_CHANGED) {
        return "HANDOFF_PREFERENCE_CHANGED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::android::hardware::radio::V1_4::DataConnActiveStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::DataConnActiveStatus> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_4::DataConnActiveStatus::INACTIVE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataConnActiveStatus::INACTIVE)) {
        os += (first ? "" : " | ");
        os += "INACTIVE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataConnActiveStatus::INACTIVE;
    }
    if ((o & ::android::hardware::radio::V1_4::DataConnActiveStatus::DORMANT) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataConnActiveStatus::DORMANT)) {
        os += (first ? "" : " | ");
        os += "DORMANT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataConnActiveStatus::DORMANT;
    }
    if ((o & ::android::hardware::radio::V1_4::DataConnActiveStatus::ACTIVE) == static_cast<int32_t>(::android::hardware::radio::V1_4::DataConnActiveStatus::ACTIVE)) {
        os += (first ? "" : " | ");
        os += "ACTIVE";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::DataConnActiveStatus::ACTIVE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_4::DataConnActiveStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_4::DataConnActiveStatus::INACTIVE) {
        return "INACTIVE";
    }
    if (o == ::android::hardware::radio::V1_4::DataConnActiveStatus::DORMANT) {
        return "DORMANT";
    }
    if (o == ::android::hardware::radio::V1_4::DataConnActiveStatus::ACTIVE) {
        return "ACTIVE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::android::hardware::radio::V1_4::PdpProtocolType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::PdpProtocolType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_4::PdpProtocolType::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_4::PdpProtocolType::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::PdpProtocolType::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_4::PdpProtocolType::IP) == static_cast<int32_t>(::android::hardware::radio::V1_4::PdpProtocolType::IP)) {
        os += (first ? "" : " | ");
        os += "IP";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::PdpProtocolType::IP;
    }
    if ((o & ::android::hardware::radio::V1_4::PdpProtocolType::IPV6) == static_cast<int32_t>(::android::hardware::radio::V1_4::PdpProtocolType::IPV6)) {
        os += (first ? "" : " | ");
        os += "IPV6";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::PdpProtocolType::IPV6;
    }
    if ((o & ::android::hardware::radio::V1_4::PdpProtocolType::IPV4V6) == static_cast<int32_t>(::android::hardware::radio::V1_4::PdpProtocolType::IPV4V6)) {
        os += (first ? "" : " | ");
        os += "IPV4V6";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::PdpProtocolType::IPV4V6;
    }
    if ((o & ::android::hardware::radio::V1_4::PdpProtocolType::PPP) == static_cast<int32_t>(::android::hardware::radio::V1_4::PdpProtocolType::PPP)) {
        os += (first ? "" : " | ");
        os += "PPP";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::PdpProtocolType::PPP;
    }
    if ((o & ::android::hardware::radio::V1_4::PdpProtocolType::NON_IP) == static_cast<int32_t>(::android::hardware::radio::V1_4::PdpProtocolType::NON_IP)) {
        os += (first ? "" : " | ");
        os += "NON_IP";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::PdpProtocolType::NON_IP;
    }
    if ((o & ::android::hardware::radio::V1_4::PdpProtocolType::UNSTRUCTURED) == static_cast<int32_t>(::android::hardware::radio::V1_4::PdpProtocolType::UNSTRUCTURED)) {
        os += (first ? "" : " | ");
        os += "UNSTRUCTURED";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::PdpProtocolType::UNSTRUCTURED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_4::PdpProtocolType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_4::PdpProtocolType::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_4::PdpProtocolType::IP) {
        return "IP";
    }
    if (o == ::android::hardware::radio::V1_4::PdpProtocolType::IPV6) {
        return "IPV6";
    }
    if (o == ::android::hardware::radio::V1_4::PdpProtocolType::IPV4V6) {
        return "IPV4V6";
    }
    if (o == ::android::hardware::radio::V1_4::PdpProtocolType::PPP) {
        return "PPP";
    }
    if (o == ::android::hardware::radio::V1_4::PdpProtocolType::NON_IP) {
        return "NON_IP";
    }
    if (o == ::android::hardware::radio::V1_4::PdpProtocolType::UNSTRUCTURED) {
        return "UNSTRUCTURED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::android::hardware::radio::V1_4::RadioFrequencyInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";

    switch (o.getDiscriminator()) {
        case ::android::hardware::radio::V1_4::RadioFrequencyInfo::hidl_discriminator::range: {
            os += ".range = ";
            os += toString(o.range());
            break;
        }
        case ::android::hardware::radio::V1_4::RadioFrequencyInfo::hidl_discriminator::channelNumber: {
            os += ".channelNumber = ";
            os += toString(o.channelNumber());
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) o.getDiscriminator()) + ").").c_str());
        }
    }
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::RadioFrequencyInfo& lhs, const ::android::hardware::radio::V1_4::RadioFrequencyInfo& rhs) {
    if (lhs.getDiscriminator() != rhs.getDiscriminator()) {
        return false;
    }
    switch (lhs.getDiscriminator()) {
        case ::android::hardware::radio::V1_4::RadioFrequencyInfo::hidl_discriminator::range: {
            return (lhs.range() == rhs.range());
        }
        case ::android::hardware::radio::V1_4::RadioFrequencyInfo::hidl_discriminator::channelNumber: {
            return (lhs.channelNumber() == rhs.channelNumber());
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) lhs.getDiscriminator()) + ").").c_str());
        }
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::RadioFrequencyInfo& lhs, const ::android::hardware::radio::V1_4::RadioFrequencyInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::PhysicalChannelConfig& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".base = ";
    os += ::android::hardware::radio::V1_2::toString(o.base);
    os += ", .rat = ";
    os += ::android::hardware::radio::V1_4::toString(o.rat);
    os += ", .rfInfo = ";
    os += ::android::hardware::radio::V1_4::toString(o.rfInfo);
    os += ", .contextIds = ";
    os += ::android::hardware::toString(o.contextIds);
    os += ", .physicalCellId = ";
    os += ::android::hardware::toString(o.physicalCellId);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::PhysicalChannelConfig& lhs, const ::android::hardware::radio::V1_4::PhysicalChannelConfig& rhs) {
    if (lhs.base != rhs.base) {
        return false;
    }
    if (lhs.rat != rhs.rat) {
        return false;
    }
    if (lhs.rfInfo != rhs.rfInfo) {
        return false;
    }
    if (lhs.contextIds != rhs.contextIds) {
        return false;
    }
    if (lhs.physicalCellId != rhs.physicalCellId) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::PhysicalChannelConfig& lhs, const ::android::hardware::radio::V1_4::PhysicalChannelConfig& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::LteVopsInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".isVopsSupported = ";
    os += ::android::hardware::toString(o.isVopsSupported);
    os += ", .isEmcBearerSupported = ";
    os += ::android::hardware::toString(o.isEmcBearerSupported);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::LteVopsInfo& lhs, const ::android::hardware::radio::V1_4::LteVopsInfo& rhs) {
    if (lhs.isVopsSupported != rhs.isVopsSupported) {
        return false;
    }
    if (lhs.isEmcBearerSupported != rhs.isEmcBearerSupported) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::LteVopsInfo& lhs, const ::android::hardware::radio::V1_4::LteVopsInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::NrIndicators& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".isEndcAvailable = ";
    os += ::android::hardware::toString(o.isEndcAvailable);
    os += ", .isDcNrRestricted = ";
    os += ::android::hardware::toString(o.isDcNrRestricted);
    os += ", .isNrAvailable = ";
    os += ::android::hardware::toString(o.isNrAvailable);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::NrIndicators& lhs, const ::android::hardware::radio::V1_4::NrIndicators& rhs) {
    if (lhs.isEndcAvailable != rhs.isEndcAvailable) {
        return false;
    }
    if (lhs.isDcNrRestricted != rhs.isDcNrRestricted) {
        return false;
    }
    if (lhs.isNrAvailable != rhs.isNrAvailable) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::NrIndicators& lhs, const ::android::hardware::radio::V1_4::NrIndicators& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";

    switch (o.getDiscriminator()) {
        case ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::hidl_discriminator::noinit: {
            os += ".noinit = ";
            os += toString(o.noinit());
            break;
        }
        case ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::hidl_discriminator::lteVopsInfo: {
            os += ".lteVopsInfo = ";
            os += toString(o.lteVopsInfo());
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) o.getDiscriminator()) + ").").c_str());
        }
    }
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo& lhs, const ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo& rhs) {
    if (lhs.getDiscriminator() != rhs.getDiscriminator()) {
        return false;
    }
    switch (lhs.getDiscriminator()) {
        case ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::hidl_discriminator::noinit: {
            return (lhs.noinit() == rhs.noinit());
        }
        case ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo::hidl_discriminator::lteVopsInfo: {
            return (lhs.lteVopsInfo() == rhs.lteVopsInfo());
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) lhs.getDiscriminator()) + ").").c_str());
        }
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo& lhs, const ::android::hardware::radio::V1_4::DataRegStateResult::VopsInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::DataRegStateResult& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".base = ";
    os += ::android::hardware::radio::V1_2::toString(o.base);
    os += ", .vopsInfo = ";
    os += ::android::hardware::radio::V1_4::toString(o.vopsInfo);
    os += ", .nrIndicators = ";
    os += ::android::hardware::radio::V1_4::toString(o.nrIndicators);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::DataRegStateResult& lhs, const ::android::hardware::radio::V1_4::DataRegStateResult& rhs) {
    if (lhs.base != rhs.base) {
        return false;
    }
    if (lhs.vopsInfo != rhs.vopsInfo) {
        return false;
    }
    if (lhs.nrIndicators != rhs.nrIndicators) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::DataRegStateResult& lhs, const ::android::hardware::radio::V1_4::DataRegStateResult& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::CellConfigLte& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".isEndcAvailable = ";
    os += ::android::hardware::toString(o.isEndcAvailable);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::CellConfigLte& lhs, const ::android::hardware::radio::V1_4::CellConfigLte& rhs) {
    if (lhs.isEndcAvailable != rhs.isEndcAvailable) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::CellConfigLte& lhs, const ::android::hardware::radio::V1_4::CellConfigLte& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::CellInfoLte& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".base = ";
    os += ::android::hardware::radio::V1_2::toString(o.base);
    os += ", .cellConfig = ";
    os += ::android::hardware::radio::V1_4::toString(o.cellConfig);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::CellInfoLte& lhs, const ::android::hardware::radio::V1_4::CellInfoLte& rhs) {
    if (lhs.base != rhs.base) {
        return false;
    }
    if (lhs.cellConfig != rhs.cellConfig) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::CellInfoLte& lhs, const ::android::hardware::radio::V1_4::CellInfoLte& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::NrSignalStrength& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".ssRsrp = ";
    os += ::android::hardware::toString(o.ssRsrp);
    os += ", .ssRsrq = ";
    os += ::android::hardware::toString(o.ssRsrq);
    os += ", .ssSinr = ";
    os += ::android::hardware::toString(o.ssSinr);
    os += ", .csiRsrp = ";
    os += ::android::hardware::toString(o.csiRsrp);
    os += ", .csiRsrq = ";
    os += ::android::hardware::toString(o.csiRsrq);
    os += ", .csiSinr = ";
    os += ::android::hardware::toString(o.csiSinr);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::NrSignalStrength& lhs, const ::android::hardware::radio::V1_4::NrSignalStrength& rhs) {
    if (lhs.ssRsrp != rhs.ssRsrp) {
        return false;
    }
    if (lhs.ssRsrq != rhs.ssRsrq) {
        return false;
    }
    if (lhs.ssSinr != rhs.ssSinr) {
        return false;
    }
    if (lhs.csiRsrp != rhs.csiRsrp) {
        return false;
    }
    if (lhs.csiRsrq != rhs.csiRsrq) {
        return false;
    }
    if (lhs.csiSinr != rhs.csiSinr) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::NrSignalStrength& lhs, const ::android::hardware::radio::V1_4::NrSignalStrength& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::CellIdentityNr& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".mcc = ";
    os += ::android::hardware::toString(o.mcc);
    os += ", .mnc = ";
    os += ::android::hardware::toString(o.mnc);
    os += ", .nci = ";
    os += ::android::hardware::toString(o.nci);
    os += ", .pci = ";
    os += ::android::hardware::toString(o.pci);
    os += ", .tac = ";
    os += ::android::hardware::toString(o.tac);
    os += ", .nrarfcn = ";
    os += ::android::hardware::toString(o.nrarfcn);
    os += ", .operatorNames = ";
    os += ::android::hardware::radio::V1_2::toString(o.operatorNames);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::CellIdentityNr& lhs, const ::android::hardware::radio::V1_4::CellIdentityNr& rhs) {
    if (lhs.mcc != rhs.mcc) {
        return false;
    }
    if (lhs.mnc != rhs.mnc) {
        return false;
    }
    if (lhs.nci != rhs.nci) {
        return false;
    }
    if (lhs.pci != rhs.pci) {
        return false;
    }
    if (lhs.tac != rhs.tac) {
        return false;
    }
    if (lhs.nrarfcn != rhs.nrarfcn) {
        return false;
    }
    if (lhs.operatorNames != rhs.operatorNames) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::CellIdentityNr& lhs, const ::android::hardware::radio::V1_4::CellIdentityNr& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::CellInfoNr& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".signalStrength = ";
    os += ::android::hardware::radio::V1_4::toString(o.signalStrength);
    os += ", .cellidentity = ";
    os += ::android::hardware::radio::V1_4::toString(o.cellidentity);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::CellInfoNr& lhs, const ::android::hardware::radio::V1_4::CellInfoNr& rhs) {
    if (lhs.signalStrength != rhs.signalStrength) {
        return false;
    }
    if (lhs.cellidentity != rhs.cellidentity) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::CellInfoNr& lhs, const ::android::hardware::radio::V1_4::CellInfoNr& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::CellInfo::Info& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";

    switch (o.getDiscriminator()) {
        case ::android::hardware::radio::V1_4::CellInfo::Info::hidl_discriminator::gsm: {
            os += ".gsm = ";
            os += toString(o.gsm());
            break;
        }
        case ::android::hardware::radio::V1_4::CellInfo::Info::hidl_discriminator::cdma: {
            os += ".cdma = ";
            os += toString(o.cdma());
            break;
        }
        case ::android::hardware::radio::V1_4::CellInfo::Info::hidl_discriminator::wcdma: {
            os += ".wcdma = ";
            os += toString(o.wcdma());
            break;
        }
        case ::android::hardware::radio::V1_4::CellInfo::Info::hidl_discriminator::tdscdma: {
            os += ".tdscdma = ";
            os += toString(o.tdscdma());
            break;
        }
        case ::android::hardware::radio::V1_4::CellInfo::Info::hidl_discriminator::lte: {
            os += ".lte = ";
            os += toString(o.lte());
            break;
        }
        case ::android::hardware::radio::V1_4::CellInfo::Info::hidl_discriminator::nr: {
            os += ".nr = ";
            os += toString(o.nr());
            break;
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) o.getDiscriminator()) + ").").c_str());
        }
    }
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::CellInfo::Info& lhs, const ::android::hardware::radio::V1_4::CellInfo::Info& rhs) {
    if (lhs.getDiscriminator() != rhs.getDiscriminator()) {
        return false;
    }
    switch (lhs.getDiscriminator()) {
        case ::android::hardware::radio::V1_4::CellInfo::Info::hidl_discriminator::gsm: {
            return (lhs.gsm() == rhs.gsm());
        }
        case ::android::hardware::radio::V1_4::CellInfo::Info::hidl_discriminator::cdma: {
            return (lhs.cdma() == rhs.cdma());
        }
        case ::android::hardware::radio::V1_4::CellInfo::Info::hidl_discriminator::wcdma: {
            return (lhs.wcdma() == rhs.wcdma());
        }
        case ::android::hardware::radio::V1_4::CellInfo::Info::hidl_discriminator::tdscdma: {
            return (lhs.tdscdma() == rhs.tdscdma());
        }
        case ::android::hardware::radio::V1_4::CellInfo::Info::hidl_discriminator::lte: {
            return (lhs.lte() == rhs.lte());
        }
        case ::android::hardware::radio::V1_4::CellInfo::Info::hidl_discriminator::nr: {
            return (lhs.nr() == rhs.nr());
        }
        default: {
            ::android::hardware::details::logAlwaysFatal((
                    "Unknown union discriminator (value: " +
                    std::to_string((uint8_t) lhs.getDiscriminator()) + ").").c_str());
        }
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::CellInfo::Info& lhs, const ::android::hardware::radio::V1_4::CellInfo::Info& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::CellInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".isRegistered = ";
    os += ::android::hardware::toString(o.isRegistered);
    os += ", .connectionStatus = ";
    os += ::android::hardware::radio::V1_2::toString(o.connectionStatus);
    os += ", .info = ";
    os += ::android::hardware::radio::V1_4::toString(o.info);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::CellInfo& lhs, const ::android::hardware::radio::V1_4::CellInfo& rhs) {
    if (lhs.isRegistered != rhs.isRegistered) {
        return false;
    }
    if (lhs.connectionStatus != rhs.connectionStatus) {
        return false;
    }
    if (lhs.info != rhs.info) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::CellInfo& lhs, const ::android::hardware::radio::V1_4::CellInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::NetworkScanResult& o) {
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

static inline bool operator==(const ::android::hardware::radio::V1_4::NetworkScanResult& lhs, const ::android::hardware::radio::V1_4::NetworkScanResult& rhs) {
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

static inline bool operator!=(const ::android::hardware::radio::V1_4::NetworkScanResult& lhs, const ::android::hardware::radio::V1_4::NetworkScanResult& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::DataProfileInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".profileId = ";
    os += ::android::hardware::radio::V1_0::toString(o.profileId);
    os += ", .apn = ";
    os += ::android::hardware::toString(o.apn);
    os += ", .protocol = ";
    os += ::android::hardware::radio::V1_4::toString(o.protocol);
    os += ", .roamingProtocol = ";
    os += ::android::hardware::radio::V1_4::toString(o.roamingProtocol);
    os += ", .authType = ";
    os += ::android::hardware::radio::V1_0::toString(o.authType);
    os += ", .user = ";
    os += ::android::hardware::toString(o.user);
    os += ", .password = ";
    os += ::android::hardware::toString(o.password);
    os += ", .type = ";
    os += ::android::hardware::radio::V1_0::toString(o.type);
    os += ", .maxConnsTime = ";
    os += ::android::hardware::toString(o.maxConnsTime);
    os += ", .maxConns = ";
    os += ::android::hardware::toString(o.maxConns);
    os += ", .waitTime = ";
    os += ::android::hardware::toString(o.waitTime);
    os += ", .enabled = ";
    os += ::android::hardware::toString(o.enabled);
    os += ", .supportedApnTypesBitmap = ";
    os += ::android::hardware::radio::V1_4::toString<::android::hardware::radio::V1_4::ApnTypes>(o.supportedApnTypesBitmap);
    os += ", .bearerBitmap = ";
    os += ::android::hardware::radio::V1_4::toString<::android::hardware::radio::V1_4::RadioAccessFamily>(o.bearerBitmap);
    os += ", .mtu = ";
    os += ::android::hardware::toString(o.mtu);
    os += ", .preferred = ";
    os += ::android::hardware::toString(o.preferred);
    os += ", .persistent = ";
    os += ::android::hardware::toString(o.persistent);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::DataProfileInfo& lhs, const ::android::hardware::radio::V1_4::DataProfileInfo& rhs) {
    if (lhs.profileId != rhs.profileId) {
        return false;
    }
    if (lhs.apn != rhs.apn) {
        return false;
    }
    if (lhs.protocol != rhs.protocol) {
        return false;
    }
    if (lhs.roamingProtocol != rhs.roamingProtocol) {
        return false;
    }
    if (lhs.authType != rhs.authType) {
        return false;
    }
    if (lhs.user != rhs.user) {
        return false;
    }
    if (lhs.password != rhs.password) {
        return false;
    }
    if (lhs.type != rhs.type) {
        return false;
    }
    if (lhs.maxConnsTime != rhs.maxConnsTime) {
        return false;
    }
    if (lhs.maxConns != rhs.maxConns) {
        return false;
    }
    if (lhs.waitTime != rhs.waitTime) {
        return false;
    }
    if (lhs.enabled != rhs.enabled) {
        return false;
    }
    if (lhs.supportedApnTypesBitmap != rhs.supportedApnTypesBitmap) {
        return false;
    }
    if (lhs.bearerBitmap != rhs.bearerBitmap) {
        return false;
    }
    if (lhs.mtu != rhs.mtu) {
        return false;
    }
    if (lhs.preferred != rhs.preferred) {
        return false;
    }
    if (lhs.persistent != rhs.persistent) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::DataProfileInfo& lhs, const ::android::hardware::radio::V1_4::DataProfileInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::CardStatus& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".base = ";
    os += ::android::hardware::radio::V1_2::toString(o.base);
    os += ", .eid = ";
    os += ::android::hardware::toString(o.eid);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::CardStatus& lhs, const ::android::hardware::radio::V1_4::CardStatus& rhs) {
    if (lhs.base != rhs.base) {
        return false;
    }
    if (lhs.eid != rhs.eid) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::CardStatus& lhs, const ::android::hardware::radio::V1_4::CardStatus& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::RadioCapability& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".session = ";
    os += ::android::hardware::toString(o.session);
    os += ", .phase = ";
    os += ::android::hardware::radio::V1_0::toString(o.phase);
    os += ", .raf = ";
    os += ::android::hardware::radio::V1_4::toString<::android::hardware::radio::V1_4::RadioAccessFamily>(o.raf);
    os += ", .logicalModemUuid = ";
    os += ::android::hardware::toString(o.logicalModemUuid);
    os += ", .status = ";
    os += ::android::hardware::radio::V1_0::toString(o.status);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::RadioCapability& lhs, const ::android::hardware::radio::V1_4::RadioCapability& rhs) {
    if (lhs.session != rhs.session) {
        return false;
    }
    if (lhs.phase != rhs.phase) {
        return false;
    }
    if (lhs.raf != rhs.raf) {
        return false;
    }
    if (lhs.logicalModemUuid != rhs.logicalModemUuid) {
        return false;
    }
    if (lhs.status != rhs.status) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::RadioCapability& lhs, const ::android::hardware::radio::V1_4::RadioCapability& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::SetupDataCallResult& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cause = ";
    os += ::android::hardware::radio::V1_4::toString(o.cause);
    os += ", .suggestedRetryTime = ";
    os += ::android::hardware::toString(o.suggestedRetryTime);
    os += ", .cid = ";
    os += ::android::hardware::toString(o.cid);
    os += ", .active = ";
    os += ::android::hardware::radio::V1_4::toString(o.active);
    os += ", .type = ";
    os += ::android::hardware::radio::V1_4::toString(o.type);
    os += ", .ifname = ";
    os += ::android::hardware::toString(o.ifname);
    os += ", .addresses = ";
    os += ::android::hardware::toString(o.addresses);
    os += ", .dnses = ";
    os += ::android::hardware::toString(o.dnses);
    os += ", .gateways = ";
    os += ::android::hardware::toString(o.gateways);
    os += ", .pcscf = ";
    os += ::android::hardware::toString(o.pcscf);
    os += ", .mtu = ";
    os += ::android::hardware::toString(o.mtu);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::SetupDataCallResult& lhs, const ::android::hardware::radio::V1_4::SetupDataCallResult& rhs) {
    if (lhs.cause != rhs.cause) {
        return false;
    }
    if (lhs.suggestedRetryTime != rhs.suggestedRetryTime) {
        return false;
    }
    if (lhs.cid != rhs.cid) {
        return false;
    }
    if (lhs.active != rhs.active) {
        return false;
    }
    if (lhs.type != rhs.type) {
        return false;
    }
    if (lhs.ifname != rhs.ifname) {
        return false;
    }
    if (lhs.addresses != rhs.addresses) {
        return false;
    }
    if (lhs.dnses != rhs.dnses) {
        return false;
    }
    if (lhs.gateways != rhs.gateways) {
        return false;
    }
    if (lhs.pcscf != rhs.pcscf) {
        return false;
    }
    if (lhs.mtu != rhs.mtu) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::SetupDataCallResult& lhs, const ::android::hardware::radio::V1_4::SetupDataCallResult& rhs){
    return !(lhs == rhs);
}

template<>
inline std::string toString<::android::hardware::radio::V1_4::SimLockMultiSimPolicy>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::SimLockMultiSimPolicy> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_4::SimLockMultiSimPolicy::NO_MULTISIM_POLICY) == static_cast<int32_t>(::android::hardware::radio::V1_4::SimLockMultiSimPolicy::NO_MULTISIM_POLICY)) {
        os += (first ? "" : " | ");
        os += "NO_MULTISIM_POLICY";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::SimLockMultiSimPolicy::NO_MULTISIM_POLICY;
    }
    if ((o & ::android::hardware::radio::V1_4::SimLockMultiSimPolicy::ONE_VALID_SIM_MUST_BE_PRESENT) == static_cast<int32_t>(::android::hardware::radio::V1_4::SimLockMultiSimPolicy::ONE_VALID_SIM_MUST_BE_PRESENT)) {
        os += (first ? "" : " | ");
        os += "ONE_VALID_SIM_MUST_BE_PRESENT";
        first = false;
        flipped |= ::android::hardware::radio::V1_4::SimLockMultiSimPolicy::ONE_VALID_SIM_MUST_BE_PRESENT;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_4::SimLockMultiSimPolicy o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_4::SimLockMultiSimPolicy::NO_MULTISIM_POLICY) {
        return "NO_MULTISIM_POLICY";
    }
    if (o == ::android::hardware::radio::V1_4::SimLockMultiSimPolicy::ONE_VALID_SIM_MUST_BE_PRESENT) {
        return "ONE_VALID_SIM_MUST_BE_PRESENT";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".allowedCarriers = ";
    os += ::android::hardware::toString(o.allowedCarriers);
    os += ", .excludedCarriers = ";
    os += ::android::hardware::toString(o.excludedCarriers);
    os += ", .allowedCarriersPrioritized = ";
    os += ::android::hardware::toString(o.allowedCarriersPrioritized);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority& lhs, const ::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority& rhs) {
    if (lhs.allowedCarriers != rhs.allowedCarriers) {
        return false;
    }
    if (lhs.excludedCarriers != rhs.excludedCarriers) {
        return false;
    }
    if (lhs.allowedCarriersPrioritized != rhs.allowedCarriersPrioritized) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority& lhs, const ::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_4::SignalStrength& o) {
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
    os += ", .tdscdma = ";
    os += ::android::hardware::radio::V1_2::toString(o.tdscdma);
    os += ", .wcdma = ";
    os += ::android::hardware::radio::V1_2::toString(o.wcdma);
    os += ", .nr = ";
    os += ::android::hardware::radio::V1_4::toString(o.nr);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_4::SignalStrength& lhs, const ::android::hardware::radio::V1_4::SignalStrength& rhs) {
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
    if (lhs.tdscdma != rhs.tdscdma) {
        return false;
    }
    if (lhs.wcdma != rhs.wcdma) {
        return false;
    }
    if (lhs.nr != rhs.nr) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_4::SignalStrength& lhs, const ::android::hardware::radio::V1_4::SignalStrength& rhs){
    return !(lhs == rhs);
}


}  // namespace V1_4
}  // namespace radio
}  // namespace hardware
}  // namespace android

//
// global type declarations for package
//

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::android::hardware::radio::V1_4::AccessNetwork, 6> hidl_enum_values<::android::hardware::radio::V1_4::AccessNetwork> = {
    ::android::hardware::radio::V1_4::AccessNetwork::GERAN,
    ::android::hardware::radio::V1_4::AccessNetwork::UTRAN,
    ::android::hardware::radio::V1_4::AccessNetwork::EUTRAN,
    ::android::hardware::radio::V1_4::AccessNetwork::CDMA2000,
    ::android::hardware::radio::V1_4::AccessNetwork::IWLAN,
    ::android::hardware::radio::V1_4::AccessNetwork::UNKNOWN,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::android::hardware::radio::V1_4::ApnTypes, 13> hidl_enum_values<::android::hardware::radio::V1_4::ApnTypes> = {
    ::android::hardware::radio::V1_4::ApnTypes::NONE,
    ::android::hardware::radio::V1_4::ApnTypes::DEFAULT,
    ::android::hardware::radio::V1_4::ApnTypes::MMS,
    ::android::hardware::radio::V1_4::ApnTypes::SUPL,
    ::android::hardware::radio::V1_4::ApnTypes::DUN,
    ::android::hardware::radio::V1_4::ApnTypes::HIPRI,
    ::android::hardware::radio::V1_4::ApnTypes::FOTA,
    ::android::hardware::radio::V1_4::ApnTypes::IMS,
    ::android::hardware::radio::V1_4::ApnTypes::CBS,
    ::android::hardware::radio::V1_4::ApnTypes::IA,
    ::android::hardware::radio::V1_4::ApnTypes::EMERGENCY,
    ::android::hardware::radio::V1_4::ApnTypes::ALL,
    ::android::hardware::radio::V1_4::ApnTypes::MCX,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::android::hardware::radio::V1_4::EmergencyServiceCategory, 8> hidl_enum_values<::android::hardware::radio::V1_4::EmergencyServiceCategory> = {
    ::android::hardware::radio::V1_4::EmergencyServiceCategory::UNSPECIFIED,
    ::android::hardware::radio::V1_4::EmergencyServiceCategory::POLICE,
    ::android::hardware::radio::V1_4::EmergencyServiceCategory::AMBULANCE,
    ::android::hardware::radio::V1_4::EmergencyServiceCategory::FIRE_BRIGADE,
    ::android::hardware::radio::V1_4::EmergencyServiceCategory::MARINE_GUARD,
    ::android::hardware::radio::V1_4::EmergencyServiceCategory::MOUNTAIN_RESCUE,
    ::android::hardware::radio::V1_4::EmergencyServiceCategory::MIEC,
    ::android::hardware::radio::V1_4::EmergencyServiceCategory::AIEC,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::android::hardware::radio::V1_4::EmergencyNumberSource, 4> hidl_enum_values<::android::hardware::radio::V1_4::EmergencyNumberSource> = {
    ::android::hardware::radio::V1_4::EmergencyNumberSource::NETWORK_SIGNALING,
    ::android::hardware::radio::V1_4::EmergencyNumberSource::SIM,
    ::android::hardware::radio::V1_4::EmergencyNumberSource::MODEM_CONFIG,
    ::android::hardware::radio::V1_4::EmergencyNumberSource::DEFAULT,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::android::hardware::radio::V1_4::EmergencyCallRouting, 3> hidl_enum_values<::android::hardware::radio::V1_4::EmergencyCallRouting> = {
    ::android::hardware::radio::V1_4::EmergencyCallRouting::UNKNOWN,
    ::android::hardware::radio::V1_4::EmergencyCallRouting::EMERGENCY,
    ::android::hardware::radio::V1_4::EmergencyCallRouting::NORMAL,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::android::hardware::radio::V1_4::RadioTechnology, 21> hidl_enum_values<::android::hardware::radio::V1_4::RadioTechnology> = {
    ::android::hardware::radio::V1_4::RadioTechnology::UNKNOWN,
    ::android::hardware::radio::V1_4::RadioTechnology::GPRS,
    ::android::hardware::radio::V1_4::RadioTechnology::EDGE,
    ::android::hardware::radio::V1_4::RadioTechnology::UMTS,
    ::android::hardware::radio::V1_4::RadioTechnology::IS95A,
    ::android::hardware::radio::V1_4::RadioTechnology::IS95B,
    ::android::hardware::radio::V1_4::RadioTechnology::ONE_X_RTT,
    ::android::hardware::radio::V1_4::RadioTechnology::EVDO_0,
    ::android::hardware::radio::V1_4::RadioTechnology::EVDO_A,
    ::android::hardware::radio::V1_4::RadioTechnology::HSDPA,
    ::android::hardware::radio::V1_4::RadioTechnology::HSUPA,
    ::android::hardware::radio::V1_4::RadioTechnology::HSPA,
    ::android::hardware::radio::V1_4::RadioTechnology::EVDO_B,
    ::android::hardware::radio::V1_4::RadioTechnology::EHRPD,
    ::android::hardware::radio::V1_4::RadioTechnology::LTE,
    ::android::hardware::radio::V1_4::RadioTechnology::HSPAP,
    ::android::hardware::radio::V1_4::RadioTechnology::GSM,
    ::android::hardware::radio::V1_4::RadioTechnology::TD_SCDMA,
    ::android::hardware::radio::V1_4::RadioTechnology::IWLAN,
    ::android::hardware::radio::V1_4::RadioTechnology::LTE_CA,
    ::android::hardware::radio::V1_4::RadioTechnology::NR,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::android::hardware::radio::V1_4::RadioAccessFamily, 20> hidl_enum_values<::android::hardware::radio::V1_4::RadioAccessFamily> = {
    ::android::hardware::radio::V1_4::RadioAccessFamily::UNKNOWN,
    ::android::hardware::radio::V1_4::RadioAccessFamily::GPRS,
    ::android::hardware::radio::V1_4::RadioAccessFamily::EDGE,
    ::android::hardware::radio::V1_4::RadioAccessFamily::UMTS,
    ::android::hardware::radio::V1_4::RadioAccessFamily::IS95A,
    ::android::hardware::radio::V1_4::RadioAccessFamily::IS95B,
    ::android::hardware::radio::V1_4::RadioAccessFamily::ONE_X_RTT,
    ::android::hardware::radio::V1_4::RadioAccessFamily::EVDO_0,
    ::android::hardware::radio::V1_4::RadioAccessFamily::EVDO_A,
    ::android::hardware::radio::V1_4::RadioAccessFamily::HSDPA,
    ::android::hardware::radio::V1_4::RadioAccessFamily::HSUPA,
    ::android::hardware::radio::V1_4::RadioAccessFamily::HSPA,
    ::android::hardware::radio::V1_4::RadioAccessFamily::EVDO_B,
    ::android::hardware::radio::V1_4::RadioAccessFamily::EHRPD,
    ::android::hardware::radio::V1_4::RadioAccessFamily::LTE,
    ::android::hardware::radio::V1_4::RadioAccessFamily::HSPAP,
    ::android::hardware::radio::V1_4::RadioAccessFamily::GSM,
    ::android::hardware::radio::V1_4::RadioAccessFamily::TD_SCDMA,
    ::android::hardware::radio::V1_4::RadioAccessFamily::LTE_CA,
    ::android::hardware::radio::V1_4::RadioAccessFamily::NR,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::android::hardware::radio::V1_4::FrequencyRange, 4> hidl_enum_values<::android::hardware::radio::V1_4::FrequencyRange> = {
    ::android::hardware::radio::V1_4::FrequencyRange::LOW,
    ::android::hardware::radio::V1_4::FrequencyRange::MID,
    ::android::hardware::radio::V1_4::FrequencyRange::HIGH,
    ::android::hardware::radio::V1_4::FrequencyRange::MMWAVE,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::android::hardware::radio::V1_4::DataCallFailCause, 337> hidl_enum_values<::android::hardware::radio::V1_4::DataCallFailCause> = {
    ::android::hardware::radio::V1_4::DataCallFailCause::NONE,
    ::android::hardware::radio::V1_4::DataCallFailCause::OPERATOR_BARRED,
    ::android::hardware::radio::V1_4::DataCallFailCause::NAS_SIGNALLING,
    ::android::hardware::radio::V1_4::DataCallFailCause::INSUFFICIENT_RESOURCES,
    ::android::hardware::radio::V1_4::DataCallFailCause::MISSING_UKNOWN_APN,
    ::android::hardware::radio::V1_4::DataCallFailCause::UNKNOWN_PDP_ADDRESS_TYPE,
    ::android::hardware::radio::V1_4::DataCallFailCause::USER_AUTHENTICATION,
    ::android::hardware::radio::V1_4::DataCallFailCause::ACTIVATION_REJECT_GGSN,
    ::android::hardware::radio::V1_4::DataCallFailCause::ACTIVATION_REJECT_UNSPECIFIED,
    ::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_OPTION_NOT_SUPPORTED,
    ::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_OPTION_NOT_SUBSCRIBED,
    ::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_OPTION_OUT_OF_ORDER,
    ::android::hardware::radio::V1_4::DataCallFailCause::NSAPI_IN_USE,
    ::android::hardware::radio::V1_4::DataCallFailCause::REGULAR_DEACTIVATION,
    ::android::hardware::radio::V1_4::DataCallFailCause::QOS_NOT_ACCEPTED,
    ::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::UMTS_REACTIVATION_REQ,
    ::android::hardware::radio::V1_4::DataCallFailCause::FEATURE_NOT_SUPP,
    ::android::hardware::radio::V1_4::DataCallFailCause::TFT_SEMANTIC_ERROR,
    ::android::hardware::radio::V1_4::DataCallFailCause::TFT_SYTAX_ERROR,
    ::android::hardware::radio::V1_4::DataCallFailCause::UNKNOWN_PDP_CONTEXT,
    ::android::hardware::radio::V1_4::DataCallFailCause::FILTER_SEMANTIC_ERROR,
    ::android::hardware::radio::V1_4::DataCallFailCause::FILTER_SYTAX_ERROR,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDP_WITHOUT_ACTIVE_TFT,
    ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_IPV4_ALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_IPV6_ALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_SINGLE_BEARER_ALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::ESM_INFO_NOT_RECEIVED,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDN_CONN_DOES_NOT_EXIST,
    ::android::hardware::radio::V1_4::DataCallFailCause::MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::MAX_ACTIVE_PDP_CONTEXT_REACHED,
    ::android::hardware::radio::V1_4::DataCallFailCause::UNSUPPORTED_APN_IN_CURRENT_PLMN,
    ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_TRANSACTION_ID,
    ::android::hardware::radio::V1_4::DataCallFailCause::MESSAGE_INCORRECT_SEMANTIC,
    ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_MANDATORY_INFO,
    ::android::hardware::radio::V1_4::DataCallFailCause::MESSAGE_TYPE_UNSUPPORTED,
    ::android::hardware::radio::V1_4::DataCallFailCause::MSG_TYPE_NONCOMPATIBLE_STATE,
    ::android::hardware::radio::V1_4::DataCallFailCause::UNKNOWN_INFO_ELEMENT,
    ::android::hardware::radio::V1_4::DataCallFailCause::CONDITIONAL_IE_ERROR,
    ::android::hardware::radio::V1_4::DataCallFailCause::MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE,
    ::android::hardware::radio::V1_4::DataCallFailCause::PROTOCOL_ERRORS,
    ::android::hardware::radio::V1_4::DataCallFailCause::APN_TYPE_CONFLICT,
    ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_PCSCF_ADDR,
    ::android::hardware::radio::V1_4::DataCallFailCause::INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN,
    ::android::hardware::radio::V1_4::DataCallFailCause::EMM_ACCESS_BARRED,
    ::android::hardware::radio::V1_4::DataCallFailCause::EMERGENCY_IFACE_ONLY,
    ::android::hardware::radio::V1_4::DataCallFailCause::IFACE_MISMATCH,
    ::android::hardware::radio::V1_4::DataCallFailCause::COMPANION_IFACE_IN_USE,
    ::android::hardware::radio::V1_4::DataCallFailCause::IP_ADDRESS_MISMATCH,
    ::android::hardware::radio::V1_4::DataCallFailCause::IFACE_AND_POL_FAMILY_MISMATCH,
    ::android::hardware::radio::V1_4::DataCallFailCause::EMM_ACCESS_BARRED_INFINITE_RETRY,
    ::android::hardware::radio::V1_4::DataCallFailCause::AUTH_FAILURE_ON_EMERGENCY_CALL,
    ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_1,
    ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_2,
    ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_3,
    ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_4,
    ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_5,
    ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_6,
    ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_7,
    ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_8,
    ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_9,
    ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_10,
    ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_11,
    ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_12,
    ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_13,
    ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_14,
    ::android::hardware::radio::V1_4::DataCallFailCause::OEM_DCFAILCAUSE_15,
    ::android::hardware::radio::V1_4::DataCallFailCause::VOICE_REGISTRATION_FAIL,
    ::android::hardware::radio::V1_4::DataCallFailCause::DATA_REGISTRATION_FAIL,
    ::android::hardware::radio::V1_4::DataCallFailCause::SIGNAL_LOST,
    ::android::hardware::radio::V1_4::DataCallFailCause::PREF_RADIO_TECH_CHANGED,
    ::android::hardware::radio::V1_4::DataCallFailCause::RADIO_POWER_OFF,
    ::android::hardware::radio::V1_4::DataCallFailCause::TETHERED_CALL_ACTIVE,
    ::android::hardware::radio::V1_4::DataCallFailCause::ERROR_UNSPECIFIED,
    ::android::hardware::radio::V1_4::DataCallFailCause::LLC_SNDCP,
    ::android::hardware::radio::V1_4::DataCallFailCause::ACTIVATION_REJECTED_BCM_VIOLATION,
    ::android::hardware::radio::V1_4::DataCallFailCause::COLLISION_WITH_NETWORK_INITIATED_REQUEST,
    ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_IPV4V6_ALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::ONLY_NON_IP_ALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::UNSUPPORTED_QCI_VALUE,
    ::android::hardware::radio::V1_4::DataCallFailCause::BEARER_HANDLING_NOT_SUPPORTED,
    ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_DNS_ADDR,
    ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_PCSCF_OR_DNS_ADDRESS,
    ::android::hardware::radio::V1_4::DataCallFailCause::CALL_PREEMPT_BY_EMERGENCY_APN,
    ::android::hardware::radio::V1_4::DataCallFailCause::UE_INITIATED_DETACH_OR_DISCONNECT,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REASON_UNSPECIFIED,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_ADMIN_PROHIBITED,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_INSUFFICIENT_RESOURCES,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MOBILE_NODE_AUTHENTICATION_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_HOME_AGENT_AUTHENTICATION_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REQUESTED_LIFETIME_TOO_LONG,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MALFORMED_REQUEST,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MALFORMED_REPLY,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_ENCAPSULATION_UNAVAILABLE,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_VJ_HEADER_COMPRESSION_UNAVAILABLE,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REVERSE_TUNNEL_UNAVAILABLE,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_REVERSE_TUNNEL_IS_MANDATORY,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_DELIVERY_STYLE_NOT_SUPPORTED,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_NAI,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_HOME_AGENT,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_HOME_ADDRESS,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_UNKNOWN_CHALLENGE,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_MISSING_CHALLENGE,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_FA_STALE_CHALLENGE,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REASON_UNSPECIFIED,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_ADMIN_PROHIBITED,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_INSUFFICIENT_RESOURCES,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_MOBILE_NODE_AUTHENTICATION_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_FOREIGN_AGENT_AUTHENTICATION_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REGISTRATION_ID_MISMATCH,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_MALFORMED_REQUEST,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_UNKNOWN_HOME_AGENT_ADDRESS,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REVERSE_TUNNEL_UNAVAILABLE,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_REVERSE_TUNNEL_IS_MANDATORY,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_HA_ENCAPSULATION_UNAVAILABLE,
    ::android::hardware::radio::V1_4::DataCallFailCause::CLOSE_IN_PROGRESS,
    ::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_INITIATED_TERMINATION,
    ::android::hardware::radio::V1_4::DataCallFailCause::MODEM_APP_PREEMPTED,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV4_CALL_DISALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV4_CALL_THROTTLED,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV6_CALL_DISALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDN_IPV6_CALL_THROTTLED,
    ::android::hardware::radio::V1_4::DataCallFailCause::MODEM_RESTART,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDP_PPP_NOT_SUPPORTED,
    ::android::hardware::radio::V1_4::DataCallFailCause::UNPREFERRED_RAT,
    ::android::hardware::radio::V1_4::DataCallFailCause::PHYSICAL_LINK_CLOSE_IN_PROGRESS,
    ::android::hardware::radio::V1_4::DataCallFailCause::APN_PENDING_HANDOVER,
    ::android::hardware::radio::V1_4::DataCallFailCause::PROFILE_BEARER_INCOMPATIBLE,
    ::android::hardware::radio::V1_4::DataCallFailCause::SIM_CARD_CHANGED,
    ::android::hardware::radio::V1_4::DataCallFailCause::LOW_POWER_MODE_OR_POWERING_DOWN,
    ::android::hardware::radio::V1_4::DataCallFailCause::APN_DISABLED,
    ::android::hardware::radio::V1_4::DataCallFailCause::MAX_PPP_INACTIVITY_TIMER_EXPIRED,
    ::android::hardware::radio::V1_4::DataCallFailCause::IPV6_ADDRESS_TRANSFER_FAILED,
    ::android::hardware::radio::V1_4::DataCallFailCause::TRAT_SWAP_FAILED,
    ::android::hardware::radio::V1_4::DataCallFailCause::EHRPD_TO_HRPD_FALLBACK,
    ::android::hardware::radio::V1_4::DataCallFailCause::MIP_CONFIG_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDN_INACTIVITY_TIMER_EXPIRED,
    ::android::hardware::radio::V1_4::DataCallFailCause::MAX_IPV4_CONNECTIONS,
    ::android::hardware::radio::V1_4::DataCallFailCause::MAX_IPV6_CONNECTIONS,
    ::android::hardware::radio::V1_4::DataCallFailCause::APN_MISMATCH,
    ::android::hardware::radio::V1_4::DataCallFailCause::IP_VERSION_MISMATCH,
    ::android::hardware::radio::V1_4::DataCallFailCause::DUN_CALL_DISALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::INTERNAL_EPC_NONEPC_TRANSITION,
    ::android::hardware::radio::V1_4::DataCallFailCause::INTERFACE_IN_USE,
    ::android::hardware::radio::V1_4::DataCallFailCause::APN_DISALLOWED_ON_ROAMING,
    ::android::hardware::radio::V1_4::DataCallFailCause::APN_PARAMETERS_CHANGED,
    ::android::hardware::radio::V1_4::DataCallFailCause::NULL_APN_DISALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::THERMAL_MITIGATION,
    ::android::hardware::radio::V1_4::DataCallFailCause::DATA_SETTINGS_DISABLED,
    ::android::hardware::radio::V1_4::DataCallFailCause::DATA_ROAMING_SETTINGS_DISABLED,
    ::android::hardware::radio::V1_4::DataCallFailCause::DDS_SWITCHED,
    ::android::hardware::radio::V1_4::DataCallFailCause::FORBIDDEN_APN_NAME,
    ::android::hardware::radio::V1_4::DataCallFailCause::DDS_SWITCH_IN_PROGRESS,
    ::android::hardware::radio::V1_4::DataCallFailCause::CALL_DISALLOWED_IN_ROAMING,
    ::android::hardware::radio::V1_4::DataCallFailCause::NON_IP_NOT_SUPPORTED,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDN_NON_IP_CALL_THROTTLED,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDN_NON_IP_CALL_DISALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_LOCK,
    ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_INTERCEPT,
    ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_REORDER,
    ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_RELEASE_DUE_TO_SO_REJECTION,
    ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_INCOMING_CALL,
    ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_ALERT_STOP,
    ::android::hardware::radio::V1_4::DataCallFailCause::CHANNEL_ACQUISITION_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::MAX_ACCESS_PROBE,
    ::android::hardware::radio::V1_4::DataCallFailCause::CONCURRENT_SERVICE_NOT_SUPPORTED_BY_BASE_STATION,
    ::android::hardware::radio::V1_4::DataCallFailCause::NO_RESPONSE_FROM_BASE_STATION,
    ::android::hardware::radio::V1_4::DataCallFailCause::REJECTED_BY_BASE_STATION,
    ::android::hardware::radio::V1_4::DataCallFailCause::CONCURRENT_SERVICES_INCOMPATIBLE,
    ::android::hardware::radio::V1_4::DataCallFailCause::NO_CDMA_SERVICE,
    ::android::hardware::radio::V1_4::DataCallFailCause::RUIM_NOT_PRESENT,
    ::android::hardware::radio::V1_4::DataCallFailCause::CDMA_RETRY_ORDER,
    ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_BLOCK,
    ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_BLOCK_ALL,
    ::android::hardware::radio::V1_4::DataCallFailCause::IS707B_MAX_ACCESS_PROBES,
    ::android::hardware::radio::V1_4::DataCallFailCause::THERMAL_EMERGENCY,
    ::android::hardware::radio::V1_4::DataCallFailCause::CONCURRENT_SERVICES_NOT_ALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::INCOMING_CALL_REJECTED,
    ::android::hardware::radio::V1_4::DataCallFailCause::NO_SERVICE_ON_GATEWAY,
    ::android::hardware::radio::V1_4::DataCallFailCause::NO_GPRS_CONTEXT,
    ::android::hardware::radio::V1_4::DataCallFailCause::ILLEGAL_MS,
    ::android::hardware::radio::V1_4::DataCallFailCause::ILLEGAL_ME,
    ::android::hardware::radio::V1_4::DataCallFailCause::GPRS_SERVICES_AND_NON_GPRS_SERVICES_NOT_ALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::GPRS_SERVICES_NOT_ALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::MS_IDENTITY_CANNOT_BE_DERIVED_BY_THE_NETWORK,
    ::android::hardware::radio::V1_4::DataCallFailCause::IMPLICITLY_DETACHED,
    ::android::hardware::radio::V1_4::DataCallFailCause::PLMN_NOT_ALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::LOCATION_AREA_NOT_ALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::GPRS_SERVICES_NOT_ALLOWED_IN_THIS_PLMN,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDP_DUPLICATE,
    ::android::hardware::radio::V1_4::DataCallFailCause::UE_RAT_CHANGE,
    ::android::hardware::radio::V1_4::DataCallFailCause::CONGESTION,
    ::android::hardware::radio::V1_4::DataCallFailCause::NO_PDP_CONTEXT_ACTIVATED,
    ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_CLASS_DSAC_REJECTION,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDP_ACTIVATE_MAX_RETRY_FAILED,
    ::android::hardware::radio::V1_4::DataCallFailCause::RADIO_ACCESS_BEARER_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::ESM_UNKNOWN_EPS_BEARER_CONTEXT,
    ::android::hardware::radio::V1_4::DataCallFailCause::DRB_RELEASED_BY_RRC,
    ::android::hardware::radio::V1_4::DataCallFailCause::CONNECTION_RELEASED,
    ::android::hardware::radio::V1_4::DataCallFailCause::EMM_DETACHED,
    ::android::hardware::radio::V1_4::DataCallFailCause::EMM_ATTACH_FAILED,
    ::android::hardware::radio::V1_4::DataCallFailCause::EMM_ATTACH_STARTED,
    ::android::hardware::radio::V1_4::DataCallFailCause::LTE_NAS_SERVICE_REQUEST_FAILED,
    ::android::hardware::radio::V1_4::DataCallFailCause::DUPLICATE_BEARER_ID,
    ::android::hardware::radio::V1_4::DataCallFailCause::ESM_COLLISION_SCENARIOS,
    ::android::hardware::radio::V1_4::DataCallFailCause::ESM_BEARER_DEACTIVATED_TO_SYNC_WITH_NETWORK,
    ::android::hardware::radio::V1_4::DataCallFailCause::ESM_NW_ACTIVATED_DED_BEARER_WITH_ID_OF_DEF_BEARER,
    ::android::hardware::radio::V1_4::DataCallFailCause::ESM_BAD_OTA_MESSAGE,
    ::android::hardware::radio::V1_4::DataCallFailCause::ESM_DOWNLOAD_SERVER_REJECTED_THE_CALL,
    ::android::hardware::radio::V1_4::DataCallFailCause::ESM_CONTEXT_TRANSFERRED_DUE_TO_IRAT,
    ::android::hardware::radio::V1_4::DataCallFailCause::DS_EXPLICIT_DEACTIVATION,
    ::android::hardware::radio::V1_4::DataCallFailCause::ESM_LOCAL_CAUSE_NONE,
    ::android::hardware::radio::V1_4::DataCallFailCause::LTE_THROTTLING_NOT_REQUIRED,
    ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_CONTROL_LIST_CHECK_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::SERVICE_NOT_ALLOWED_ON_PLMN,
    ::android::hardware::radio::V1_4::DataCallFailCause::EMM_T3417_EXPIRED,
    ::android::hardware::radio::V1_4::DataCallFailCause::EMM_T3417_EXT_EXPIRED,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_DATA_TRANSMISSION_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_DELIVERY_FAILED_DUE_TO_HANDOVER,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_CONNECTION_RELEASE,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_RADIO_LINK_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_UPLINK_ERROR_REQUEST_FROM_NAS,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ACCESS_STRATUM_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ANOTHER_PROCEDURE_IN_PROGRESS,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ACCESS_BARRED,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_CELL_RESELECTION,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_CONFIG_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_TIMER_EXPIRED,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_LINK_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_CELL_NOT_CAMPED,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_SYSTEM_INTERVAL_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_REJECT_BY_NETWORK,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_NORMAL_RELEASE,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_RADIO_LINK_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_REESTABLISHMENT_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_OUT_OF_SERVICE_DURING_CELL_REGISTER,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORT_REQUEST,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_SYSTEM_INFORMATION_BLOCK_READ_ERROR,
    ::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_INITIATED_DETACH_WITH_AUTO_REATTACH,
    ::android::hardware::radio::V1_4::DataCallFailCause::NETWORK_INITIATED_DETACH_NO_AUTO_REATTACH,
    ::android::hardware::radio::V1_4::DataCallFailCause::ESM_PROCEDURE_TIME_OUT,
    ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_CONNECTION_ID,
    ::android::hardware::radio::V1_4::DataCallFailCause::MAXIMIUM_NSAPIS_EXCEEDED,
    ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_PRIMARY_NSAPI,
    ::android::hardware::radio::V1_4::DataCallFailCause::CANNOT_ENCODE_OTA_MESSAGE,
    ::android::hardware::radio::V1_4::DataCallFailCause::RADIO_ACCESS_BEARER_SETUP_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDP_ESTABLISH_TIMEOUT_EXPIRED,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDP_MODIFY_TIMEOUT_EXPIRED,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDP_INACTIVE_TIMEOUT_EXPIRED,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDP_LOWERLAYER_ERROR,
    ::android::hardware::radio::V1_4::DataCallFailCause::PDP_MODIFY_COLLISION,
    ::android::hardware::radio::V1_4::DataCallFailCause::MAXINUM_SIZE_OF_L2_MESSAGE_EXCEEDED,
    ::android::hardware::radio::V1_4::DataCallFailCause::NAS_REQUEST_REJECTED_BY_NETWORK,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_INVALID_REQUEST,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_TRACKING_AREA_ID_CHANGED,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_RF_UNAVAILABLE,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_DUE_TO_IRAT_CHANGE,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_RELEASED_SECURITY_NOT_ACTIVE,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_AFTER_HANDOVER,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_AFTER_IRAT_CELL_CHANGE,
    ::android::hardware::radio::V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_DURING_IRAT_CELL_CHANGE,
    ::android::hardware::radio::V1_4::DataCallFailCause::IMSI_UNKNOWN_IN_HOME_SUBSCRIBER_SERVER,
    ::android::hardware::radio::V1_4::DataCallFailCause::IMEI_NOT_ACCEPTED,
    ::android::hardware::radio::V1_4::DataCallFailCause::EPS_SERVICES_AND_NON_EPS_SERVICES_NOT_ALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::EPS_SERVICES_NOT_ALLOWED_IN_PLMN,
    ::android::hardware::radio::V1_4::DataCallFailCause::MSC_TEMPORARILY_NOT_REACHABLE,
    ::android::hardware::radio::V1_4::DataCallFailCause::CS_DOMAIN_NOT_AVAILABLE,
    ::android::hardware::radio::V1_4::DataCallFailCause::ESM_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::MAC_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::SYNCHRONIZATION_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::UE_SECURITY_CAPABILITIES_MISMATCH,
    ::android::hardware::radio::V1_4::DataCallFailCause::SECURITY_MODE_REJECTED,
    ::android::hardware::radio::V1_4::DataCallFailCause::UNACCEPTABLE_NON_EPS_AUTHENTICATION,
    ::android::hardware::radio::V1_4::DataCallFailCause::CS_FALLBACK_CALL_ESTABLISHMENT_NOT_ALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::NO_EPS_BEARER_CONTEXT_ACTIVATED,
    ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_EMM_STATE,
    ::android::hardware::radio::V1_4::DataCallFailCause::NAS_LAYER_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::MULTIPLE_PDP_CALL_NOT_ALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::EMBMS_NOT_ENABLED,
    ::android::hardware::radio::V1_4::DataCallFailCause::IRAT_HANDOVER_FAILED,
    ::android::hardware::radio::V1_4::DataCallFailCause::EMBMS_REGULAR_DEACTIVATION,
    ::android::hardware::radio::V1_4::DataCallFailCause::TEST_LOOPBACK_REGULAR_DEACTIVATION,
    ::android::hardware::radio::V1_4::DataCallFailCause::LOWER_LAYER_REGISTRATION_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::DATA_PLAN_EXPIRED,
    ::android::hardware::radio::V1_4::DataCallFailCause::UMTS_HANDOVER_TO_IWLAN,
    ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_CONNECTION_DENY_BY_GENERAL_OR_NETWORK_BUSY,
    ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_CONNECTION_DENY_BY_BILLING_OR_AUTHENTICATION_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_CHANGED,
    ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_EXITED,
    ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_NO_SESSION,
    ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_USING_GPS_FIX_INSTEAD_OF_HDR_CALL,
    ::android::hardware::radio::V1_4::DataCallFailCause::EVDO_HDR_CONNECTION_SETUP_TIMEOUT,
    ::android::hardware::radio::V1_4::DataCallFailCause::FAILED_TO_ACQUIRE_COLOCATED_HDR,
    ::android::hardware::radio::V1_4::DataCallFailCause::OTASP_COMMIT_IN_PROGRESS,
    ::android::hardware::radio::V1_4::DataCallFailCause::NO_HYBRID_HDR_SERVICE,
    ::android::hardware::radio::V1_4::DataCallFailCause::HDR_NO_LOCK_GRANTED,
    ::android::hardware::radio::V1_4::DataCallFailCause::DBM_OR_SMS_IN_PROGRESS,
    ::android::hardware::radio::V1_4::DataCallFailCause::HDR_FADE,
    ::android::hardware::radio::V1_4::DataCallFailCause::HDR_ACCESS_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::UNSUPPORTED_1X_PREV,
    ::android::hardware::radio::V1_4::DataCallFailCause::LOCAL_END,
    ::android::hardware::radio::V1_4::DataCallFailCause::NO_SERVICE,
    ::android::hardware::radio::V1_4::DataCallFailCause::FADE,
    ::android::hardware::radio::V1_4::DataCallFailCause::NORMAL_RELEASE,
    ::android::hardware::radio::V1_4::DataCallFailCause::ACCESS_ATTEMPT_ALREADY_IN_PROGRESS,
    ::android::hardware::radio::V1_4::DataCallFailCause::REDIRECTION_OR_HANDOFF_IN_PROGRESS,
    ::android::hardware::radio::V1_4::DataCallFailCause::EMERGENCY_MODE,
    ::android::hardware::radio::V1_4::DataCallFailCause::PHONE_IN_USE,
    ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_MODE,
    ::android::hardware::radio::V1_4::DataCallFailCause::INVALID_SIM_STATE,
    ::android::hardware::radio::V1_4::DataCallFailCause::NO_COLLOCATED_HDR,
    ::android::hardware::radio::V1_4::DataCallFailCause::UE_IS_ENTERING_POWERSAVE_MODE,
    ::android::hardware::radio::V1_4::DataCallFailCause::DUAL_SWITCH,
    ::android::hardware::radio::V1_4::DataCallFailCause::PPP_TIMEOUT,
    ::android::hardware::radio::V1_4::DataCallFailCause::PPP_AUTH_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::PPP_OPTION_MISMATCH,
    ::android::hardware::radio::V1_4::DataCallFailCause::PPP_PAP_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::PPP_CHAP_FAILURE,
    ::android::hardware::radio::V1_4::DataCallFailCause::PPP_CLOSE_IN_PROGRESS,
    ::android::hardware::radio::V1_4::DataCallFailCause::LIMITED_TO_IPV4,
    ::android::hardware::radio::V1_4::DataCallFailCause::LIMITED_TO_IPV6,
    ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_TIMEOUT,
    ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_GEN_ERROR,
    ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_APN_UNATHORIZED,
    ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_LIMIT_EXCEEDED,
    ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_NO_PDN_GATEWAY_ADDRESS,
    ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_GATEWAY_UNREACHABLE,
    ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_GATEWAY_REJECT,
    ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_INSUFFICIENT_PARAMETERS,
    ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_RESOURCE_UNAVAILABLE,
    ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_ADMINISTRATIVELY_PROHIBITED,
    ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_ID_IN_USE,
    ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_SUBSCRIBER_LIMITATION,
    ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_PDN_EXISTS_FOR_THIS_APN,
    ::android::hardware::radio::V1_4::DataCallFailCause::VSNCP_RECONNECT_NOT_ALLOWED,
    ::android::hardware::radio::V1_4::DataCallFailCause::IPV6_PREFIX_UNAVAILABLE,
    ::android::hardware::radio::V1_4::DataCallFailCause::HANDOFF_PREFERENCE_CHANGED,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::android::hardware::radio::V1_4::DataConnActiveStatus, 3> hidl_enum_values<::android::hardware::radio::V1_4::DataConnActiveStatus> = {
    ::android::hardware::radio::V1_4::DataConnActiveStatus::INACTIVE,
    ::android::hardware::radio::V1_4::DataConnActiveStatus::DORMANT,
    ::android::hardware::radio::V1_4::DataConnActiveStatus::ACTIVE,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::android::hardware::radio::V1_4::PdpProtocolType, 7> hidl_enum_values<::android::hardware::radio::V1_4::PdpProtocolType> = {
    ::android::hardware::radio::V1_4::PdpProtocolType::UNKNOWN,
    ::android::hardware::radio::V1_4::PdpProtocolType::IP,
    ::android::hardware::radio::V1_4::PdpProtocolType::IPV6,
    ::android::hardware::radio::V1_4::PdpProtocolType::IPV4V6,
    ::android::hardware::radio::V1_4::PdpProtocolType::PPP,
    ::android::hardware::radio::V1_4::PdpProtocolType::NON_IP,
    ::android::hardware::radio::V1_4::PdpProtocolType::UNSTRUCTURED,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::android::hardware::radio::V1_4::SimLockMultiSimPolicy, 2> hidl_enum_values<::android::hardware::radio::V1_4::SimLockMultiSimPolicy> = {
    ::android::hardware::radio::V1_4::SimLockMultiSimPolicy::NO_MULTISIM_POLICY,
    ::android::hardware::radio::V1_4::SimLockMultiSimPolicy::ONE_VALID_SIM_MUST_BE_PRESENT,
};
}  // namespace details
}  // namespace hardware
}  // namespace android


#endif  // HIDL_GENERATED_ANDROID_HARDWARE_RADIO_V1_4_TYPES_H
