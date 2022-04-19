#ifndef HIDL_GENERATED_ANDROID_HARDWARE_RADIO_V1_0_TYPES_H
#define HIDL_GENERATED_ANDROID_HARDWARE_RADIO_V1_0_TYPES_H

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hardware {
namespace radio {
namespace V1_0 {

// Forward declaration for forward reference support:
enum class RadioConst : int32_t;
enum class RadioCdmaSmsConst : int32_t;
enum class RadioError : int32_t;
enum class RadioResponseType : int32_t;
enum class RadioIndicationType : int32_t;
enum class RestrictedState : int32_t;
enum class CardState : int32_t;
enum class PinState : int32_t;
enum class AppType : int32_t;
enum class AppState : int32_t;
enum class PersoSubstate : int32_t;
enum class RadioState : int32_t;
enum class SapConnectRsp : int32_t;
enum class SapDisconnectType : int32_t;
enum class SapApduType : int32_t;
enum class SapResultCode : int32_t;
enum class SapStatus : int32_t;
enum class SapTransferProtocol : int32_t;
enum class CallState : int32_t;
enum class UusType : int32_t;
enum class UusDcs : int32_t;
enum class CallPresentation : int32_t;
enum class Clir : int32_t;
enum class LastCallFailCause : int32_t;
enum class DataCallFailCause : int32_t;
enum class RegState : int32_t;
enum class RadioTechnology : int32_t;
enum class DataProfileId : int32_t;
enum class SmsAcknowledgeFailCause : int32_t;
enum class CallForwardInfoStatus : int32_t;
enum class ClipStatus : int32_t;
enum class SmsWriteArgsStatus : int32_t;
enum class RadioBandMode : int32_t;
enum class OperatorStatus : int32_t;
enum class PreferredNetworkType : int32_t;
enum class CdmaSubscriptionSource : int32_t;
enum class CdmaRoamingType : int32_t;
enum class TtyMode : int32_t;
enum class NvItem : int32_t;
enum class ResetNvType : int32_t;
enum class HardwareConfigType : int32_t;
enum class HardwareConfigState : int32_t;
enum class LceStatus : int32_t;
enum class CarrierMatchType : int32_t;
struct NeighboringCell;
enum class CdmaSmsDigitMode : int32_t;
enum class CdmaSmsNumberMode : int32_t;
enum class CdmaSmsNumberType : int32_t;
enum class CdmaSmsNumberPlan : int32_t;
enum class CdmaSmsSubaddressType : int32_t;
enum class CdmaSmsErrorClass : int32_t;
enum class CdmaSmsWriteArgsStatus : int32_t;
enum class CellInfoType : int32_t;
enum class TimeStampType : int32_t;
enum class ApnAuthType : int32_t;
enum class RadioTechnologyFamily : int32_t;
enum class RadioCapabilityPhase : int32_t;
enum class RadioCapabilityStatus : int32_t;
enum class RadioAccessFamily : int32_t;
enum class UssdModeType : int32_t;
enum class SimRefreshType : int32_t;
enum class SrvccState : int32_t;
enum class UiccSubActStatus : int32_t;
enum class SubscriptionType : int32_t;
enum class DataProfileInfoType : int32_t;
enum class PhoneRestrictedState : int32_t;
enum class CdmaCallWaitingNumberPresentation : int32_t;
enum class CdmaCallWaitingNumberType : int32_t;
enum class CdmaCallWaitingNumberPlan : int32_t;
enum class CdmaOtaProvisionStatus : int32_t;
enum class CdmaInfoRecName : int32_t;
enum class CdmaRedirectingReason : int32_t;
enum class SsServiceType : int32_t;
enum class SsRequestType : int32_t;
enum class SsTeleserviceType : int32_t;
enum class SuppServiceClass : int32_t;
enum class ApnTypes : int32_t;
enum class IndicationFilter : int32_t;
enum class MvnoType : int32_t;
enum class DeviceStateType : int32_t;
enum class P2Constant : int32_t;
struct RadioResponseInfo;
struct AppStatus;
struct CardStatus;
struct UusInfo;
struct Call;
struct Dial;
struct LastCallFailCauseInfo;
struct GsmSignalStrength;
struct WcdmaSignalStrength;
struct CdmaSignalStrength;
struct EvdoSignalStrength;
struct LteSignalStrength;
struct TdScdmaSignalStrength;
struct SignalStrength;
struct SendSmsResult;
struct SetupDataCallResult;
struct IccIo;
struct IccIoResult;
struct CallForwardInfo;
struct OperatorInfo;
struct SmsWriteArgs;
struct CdmaSmsAddress;
struct CdmaSmsSubaddress;
struct CdmaSmsMessage;
struct CdmaSmsAck;
struct CdmaBroadcastSmsConfigInfo;
struct CdmaSmsWriteArgs;
struct GsmBroadcastSmsConfigInfo;
struct CellIdentityGsm;
struct CellIdentityWcdma;
struct CellIdentityCdma;
struct CellIdentityLte;
struct CellIdentityTdscdma;
struct CellInfoGsm;
struct CellInfoWcdma;
struct CellInfoCdma;
struct CellInfoLte;
struct CellInfoTdscdma;
struct CellInfo;
struct CellIdentity;
struct VoiceRegStateResult;
struct DataRegStateResult;
struct GsmSmsMessage;
struct ImsSmsMessage;
struct SimApdu;
struct NvWriteItem;
struct SelectUiccSub;
struct HardwareConfigModem;
struct HardwareConfigSim;
struct HardwareConfig;
struct DataProfileInfo;
struct RadioCapability;
struct LceStatusInfo;
struct LceDataInfo;
struct ActivityStatsInfo;
struct Carrier;
struct CarrierRestrictions;
struct SuppSvcNotification;
struct SimRefreshResult;
struct CdmaSignalInfoRecord;
struct CdmaCallWaiting;
struct CdmaDisplayInfoRecord;
struct CdmaNumberInfoRecord;
struct CdmaRedirectingNumberInfoRecord;
struct CdmaLineControlInfoRecord;
struct CdmaT53ClirInfoRecord;
struct CdmaT53AudioControlInfoRecord;
struct CdmaInformationRecord;
struct CdmaInformationRecords;
struct CfData;
struct SsInfoData;
struct StkCcUnsolSsResult;
struct PcoDataInfo;

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
};

enum class RadioCdmaSmsConst : int32_t {
    ADDRESS_MAX = 36,
    SUBADDRESS_MAX = 36,
    BEARER_DATA_MAX = 255,
    UDH_MAX_SND_SIZE = 128,
    UDH_EO_DATA_SEGMENT_MAX = 131,
    MAX_UD_HEADERS = 7,
    USER_DATA_MAX = 229,
    UDH_LARGE_PIC_SIZE = 128,
    UDH_SMALL_PIC_SIZE = 32,
    UDH_VAR_PIC_SIZE = 134,
    UDH_ANIM_NUM_BITMAPS = 4,
    UDH_LARGE_BITMAP_SIZE = 32,
    UDH_SMALL_BITMAP_SIZE = 8,
    UDH_OTHER_SIZE = 226,
    IP_ADDRESS_SIZE = 4,
};

enum class RadioError : int32_t {
    NONE = 0,
    RADIO_NOT_AVAILABLE = 1,
    GENERIC_FAILURE = 2,
    PASSWORD_INCORRECT = 3,
    SIM_PIN2 = 4,
    SIM_PUK2 = 5,
    REQUEST_NOT_SUPPORTED = 6,
    CANCELLED = 7,
    OP_NOT_ALLOWED_DURING_VOICE_CALL = 8,
    OP_NOT_ALLOWED_BEFORE_REG_TO_NW = 9,
    SMS_SEND_FAIL_RETRY = 10,
    SIM_ABSENT = 11,
    SUBSCRIPTION_NOT_AVAILABLE = 12,
    MODE_NOT_SUPPORTED = 13,
    FDN_CHECK_FAILURE = 14,
    ILLEGAL_SIM_OR_ME = 15,
    MISSING_RESOURCE = 16,
    NO_SUCH_ELEMENT = 17,
    DIAL_MODIFIED_TO_USSD = 18,
    DIAL_MODIFIED_TO_SS = 19,
    DIAL_MODIFIED_TO_DIAL = 20,
    USSD_MODIFIED_TO_DIAL = 21,
    USSD_MODIFIED_TO_SS = 22,
    USSD_MODIFIED_TO_USSD = 23,
    SS_MODIFIED_TO_DIAL = 24,
    SS_MODIFIED_TO_USSD = 25,
    SUBSCRIPTION_NOT_SUPPORTED = 26,
    SS_MODIFIED_TO_SS = 27,
    LCE_NOT_SUPPORTED = 36,
    NO_MEMORY = 37,
    INTERNAL_ERR = 38,
    SYSTEM_ERR = 39,
    MODEM_ERR = 40,
    INVALID_STATE = 41,
    NO_RESOURCES = 42,
    SIM_ERR = 43,
    INVALID_ARGUMENTS = 44,
    INVALID_SIM_STATE = 45,
    INVALID_MODEM_STATE = 46,
    INVALID_CALL_ID = 47,
    NO_SMS_TO_ACK = 48,
    NETWORK_ERR = 49,
    REQUEST_RATE_LIMITED = 50,
    SIM_BUSY = 51,
    SIM_FULL = 52,
    NETWORK_REJECT = 53,
    OPERATION_NOT_ALLOWED = 54,
    EMPTY_RECORD = 55,
    INVALID_SMS_FORMAT = 56,
    ENCODING_ERR = 57,
    INVALID_SMSC_ADDRESS = 58,
    NO_SUCH_ENTRY = 59,
    NETWORK_NOT_READY = 60,
    NOT_PROVISIONED = 61,
    NO_SUBSCRIPTION = 62,
    NO_NETWORK_FOUND = 63,
    DEVICE_IN_USE = 64,
    ABORTED = 65,
    INVALID_RESPONSE = 66,
    OEM_ERROR_1 = 501,
    OEM_ERROR_2 = 502,
    OEM_ERROR_3 = 503,
    OEM_ERROR_4 = 504,
    OEM_ERROR_5 = 505,
    OEM_ERROR_6 = 506,
    OEM_ERROR_7 = 507,
    OEM_ERROR_8 = 508,
    OEM_ERROR_9 = 509,
    OEM_ERROR_10 = 510,
    OEM_ERROR_11 = 511,
    OEM_ERROR_12 = 512,
    OEM_ERROR_13 = 513,
    OEM_ERROR_14 = 514,
    OEM_ERROR_15 = 515,
    OEM_ERROR_16 = 516,
    OEM_ERROR_17 = 517,
    OEM_ERROR_18 = 518,
    OEM_ERROR_19 = 519,
    OEM_ERROR_20 = 520,
    OEM_ERROR_21 = 521,
    OEM_ERROR_22 = 522,
    OEM_ERROR_23 = 523,
    OEM_ERROR_24 = 524,
    OEM_ERROR_25 = 525,
};

enum class RadioResponseType : int32_t {
    SOLICITED = 0,
    SOLICITED_ACK = 1, // (::android::hardware::radio::V1_0::RadioResponseType.SOLICITED implicitly + 1)
    SOLICITED_ACK_EXP = 2, // (::android::hardware::radio::V1_0::RadioResponseType.SOLICITED_ACK implicitly + 1)
};

enum class RadioIndicationType : int32_t {
    UNSOLICITED = 0,
    UNSOLICITED_ACK_EXP = 1, // (::android::hardware::radio::V1_0::RadioIndicationType.UNSOLICITED implicitly + 1)
};

enum class RestrictedState : int32_t {
    NONE = 0,
    CS_EMERGENCY = 1,
    CS_NORMAL = 2,
    CS_ALL = 4,
    PS_ALL = 16,
};

enum class CardState : int32_t {
    ABSENT = 0,
    PRESENT = 1, // (::android::hardware::radio::V1_0::CardState.ABSENT implicitly + 1)
    ERROR = 2, // (::android::hardware::radio::V1_0::CardState.PRESENT implicitly + 1)
    RESTRICTED = 3, // (::android::hardware::radio::V1_0::CardState.ERROR implicitly + 1)
};

enum class PinState : int32_t {
    UNKNOWN = 0,
    ENABLED_NOT_VERIFIED = 1, // (::android::hardware::radio::V1_0::PinState.UNKNOWN implicitly + 1)
    ENABLED_VERIFIED = 2, // (::android::hardware::radio::V1_0::PinState.ENABLED_NOT_VERIFIED implicitly + 1)
    DISABLED = 3, // (::android::hardware::radio::V1_0::PinState.ENABLED_VERIFIED implicitly + 1)
    ENABLED_BLOCKED = 4, // (::android::hardware::radio::V1_0::PinState.DISABLED implicitly + 1)
    ENABLED_PERM_BLOCKED = 5, // (::android::hardware::radio::V1_0::PinState.ENABLED_BLOCKED implicitly + 1)
};

enum class AppType : int32_t {
    UNKNOWN = 0,
    SIM = 1, // (::android::hardware::radio::V1_0::AppType.UNKNOWN implicitly + 1)
    USIM = 2, // (::android::hardware::radio::V1_0::AppType.SIM implicitly + 1)
    RUIM = 3, // (::android::hardware::radio::V1_0::AppType.USIM implicitly + 1)
    CSIM = 4, // (::android::hardware::radio::V1_0::AppType.RUIM implicitly + 1)
    ISIM = 5, // (::android::hardware::radio::V1_0::AppType.CSIM implicitly + 1)
};

enum class AppState : int32_t {
    UNKNOWN = 0,
    DETECTED = 1, // (::android::hardware::radio::V1_0::AppState.UNKNOWN implicitly + 1)
    PIN = 2, // (::android::hardware::radio::V1_0::AppState.DETECTED implicitly + 1)
    PUK = 3, // (::android::hardware::radio::V1_0::AppState.PIN implicitly + 1)
    SUBSCRIPTION_PERSO = 4, // (::android::hardware::radio::V1_0::AppState.PUK implicitly + 1)
    READY = 5, // (::android::hardware::radio::V1_0::AppState.SUBSCRIPTION_PERSO implicitly + 1)
};

enum class PersoSubstate : int32_t {
    UNKNOWN = 0,
    IN_PROGRESS = 1, // (::android::hardware::radio::V1_0::PersoSubstate.UNKNOWN implicitly + 1)
    READY = 2, // (::android::hardware::radio::V1_0::PersoSubstate.IN_PROGRESS implicitly + 1)
    SIM_NETWORK = 3, // (::android::hardware::radio::V1_0::PersoSubstate.READY implicitly + 1)
    SIM_NETWORK_SUBSET = 4, // (::android::hardware::radio::V1_0::PersoSubstate.SIM_NETWORK implicitly + 1)
    SIM_CORPORATE = 5, // (::android::hardware::radio::V1_0::PersoSubstate.SIM_NETWORK_SUBSET implicitly + 1)
    SIM_SERVICE_PROVIDER = 6, // (::android::hardware::radio::V1_0::PersoSubstate.SIM_CORPORATE implicitly + 1)
    SIM_SIM = 7, // (::android::hardware::radio::V1_0::PersoSubstate.SIM_SERVICE_PROVIDER implicitly + 1)
    SIM_NETWORK_PUK = 8, // (::android::hardware::radio::V1_0::PersoSubstate.SIM_SIM implicitly + 1)
    SIM_NETWORK_SUBSET_PUK = 9, // (::android::hardware::radio::V1_0::PersoSubstate.SIM_NETWORK_PUK implicitly + 1)
    SIM_CORPORATE_PUK = 10, // (::android::hardware::radio::V1_0::PersoSubstate.SIM_NETWORK_SUBSET_PUK implicitly + 1)
    SIM_SERVICE_PROVIDER_PUK = 11, // (::android::hardware::radio::V1_0::PersoSubstate.SIM_CORPORATE_PUK implicitly + 1)
    SIM_SIM_PUK = 12, // (::android::hardware::radio::V1_0::PersoSubstate.SIM_SERVICE_PROVIDER_PUK implicitly + 1)
    RUIM_NETWORK1 = 13, // (::android::hardware::radio::V1_0::PersoSubstate.SIM_SIM_PUK implicitly + 1)
    RUIM_NETWORK2 = 14, // (::android::hardware::radio::V1_0::PersoSubstate.RUIM_NETWORK1 implicitly + 1)
    RUIM_HRPD = 15, // (::android::hardware::radio::V1_0::PersoSubstate.RUIM_NETWORK2 implicitly + 1)
    RUIM_CORPORATE = 16, // (::android::hardware::radio::V1_0::PersoSubstate.RUIM_HRPD implicitly + 1)
    RUIM_SERVICE_PROVIDER = 17, // (::android::hardware::radio::V1_0::PersoSubstate.RUIM_CORPORATE implicitly + 1)
    RUIM_RUIM = 18, // (::android::hardware::radio::V1_0::PersoSubstate.RUIM_SERVICE_PROVIDER implicitly + 1)
    RUIM_NETWORK1_PUK = 19, // (::android::hardware::radio::V1_0::PersoSubstate.RUIM_RUIM implicitly + 1)
    RUIM_NETWORK2_PUK = 20, // (::android::hardware::radio::V1_0::PersoSubstate.RUIM_NETWORK1_PUK implicitly + 1)
    RUIM_HRPD_PUK = 21, // (::android::hardware::radio::V1_0::PersoSubstate.RUIM_NETWORK2_PUK implicitly + 1)
    RUIM_CORPORATE_PUK = 22, // (::android::hardware::radio::V1_0::PersoSubstate.RUIM_HRPD_PUK implicitly + 1)
    RUIM_SERVICE_PROVIDER_PUK = 23, // (::android::hardware::radio::V1_0::PersoSubstate.RUIM_CORPORATE_PUK implicitly + 1)
    RUIM_RUIM_PUK = 24, // (::android::hardware::radio::V1_0::PersoSubstate.RUIM_SERVICE_PROVIDER_PUK implicitly + 1)
};

enum class RadioState : int32_t {
    OFF = 0,
    UNAVAILABLE = 1,
    ON = 10,
};

enum class SapConnectRsp : int32_t {
    SUCCESS = 0,
    CONNECT_FAILURE = 1, // (::android::hardware::radio::V1_0::SapConnectRsp.SUCCESS implicitly + 1)
    MSG_SIZE_TOO_LARGE = 2, // (::android::hardware::radio::V1_0::SapConnectRsp.CONNECT_FAILURE implicitly + 1)
    MSG_SIZE_TOO_SMALL = 3, // (::android::hardware::radio::V1_0::SapConnectRsp.MSG_SIZE_TOO_LARGE implicitly + 1)
    CONNECT_OK_CALL_ONGOING = 4, // (::android::hardware::radio::V1_0::SapConnectRsp.MSG_SIZE_TOO_SMALL implicitly + 1)
};

enum class SapDisconnectType : int32_t {
    GRACEFUL = 0,
    IMMEDIATE = 1, // (::android::hardware::radio::V1_0::SapDisconnectType.GRACEFUL implicitly + 1)
};

enum class SapApduType : int32_t {
    APDU = 0,
    APDU7816 = 1, // (::android::hardware::radio::V1_0::SapApduType.APDU implicitly + 1)
};

enum class SapResultCode : int32_t {
    SUCCESS = 0,
    GENERIC_FAILURE = 1, // (::android::hardware::radio::V1_0::SapResultCode.SUCCESS implicitly + 1)
    CARD_NOT_ACCESSSIBLE = 2, // (::android::hardware::radio::V1_0::SapResultCode.GENERIC_FAILURE implicitly + 1)
    CARD_ALREADY_POWERED_OFF = 3, // (::android::hardware::radio::V1_0::SapResultCode.CARD_NOT_ACCESSSIBLE implicitly + 1)
    CARD_REMOVED = 4, // (::android::hardware::radio::V1_0::SapResultCode.CARD_ALREADY_POWERED_OFF implicitly + 1)
    CARD_ALREADY_POWERED_ON = 5, // (::android::hardware::radio::V1_0::SapResultCode.CARD_REMOVED implicitly + 1)
    DATA_NOT_AVAILABLE = 6, // (::android::hardware::radio::V1_0::SapResultCode.CARD_ALREADY_POWERED_ON implicitly + 1)
    NOT_SUPPORTED = 7, // (::android::hardware::radio::V1_0::SapResultCode.DATA_NOT_AVAILABLE implicitly + 1)
};

enum class SapStatus : int32_t {
    UNKNOWN_ERROR = 0,
    CARD_RESET = 1, // (::android::hardware::radio::V1_0::SapStatus.UNKNOWN_ERROR implicitly + 1)
    CARD_NOT_ACCESSIBLE = 2, // (::android::hardware::radio::V1_0::SapStatus.CARD_RESET implicitly + 1)
    CARD_REMOVED = 3, // (::android::hardware::radio::V1_0::SapStatus.CARD_NOT_ACCESSIBLE implicitly + 1)
    CARD_INSERTED = 4, // (::android::hardware::radio::V1_0::SapStatus.CARD_REMOVED implicitly + 1)
    RECOVERED = 5, // (::android::hardware::radio::V1_0::SapStatus.CARD_INSERTED implicitly + 1)
};

enum class SapTransferProtocol : int32_t {
    T0 = 0,
    T1 = 1, // (::android::hardware::radio::V1_0::SapTransferProtocol.T0 implicitly + 1)
};

enum class CallState : int32_t {
    ACTIVE = 0,
    HOLDING = 1, // (::android::hardware::radio::V1_0::CallState.ACTIVE implicitly + 1)
    DIALING = 2, // (::android::hardware::radio::V1_0::CallState.HOLDING implicitly + 1)
    ALERTING = 3, // (::android::hardware::radio::V1_0::CallState.DIALING implicitly + 1)
    INCOMING = 4, // (::android::hardware::radio::V1_0::CallState.ALERTING implicitly + 1)
    WAITING = 5, // (::android::hardware::radio::V1_0::CallState.INCOMING implicitly + 1)
};

/**
 * User-to-User signaling Info activation types derived from 3GPP 23.087 v8.0
 */
enum class UusType : int32_t {
    TYPE1_IMPLICIT = 0,
    TYPE1_REQUIRED = 1, // (::android::hardware::radio::V1_0::UusType.TYPE1_IMPLICIT implicitly + 1)
    TYPE1_NOT_REQUIRED = 2, // (::android::hardware::radio::V1_0::UusType.TYPE1_REQUIRED implicitly + 1)
    TYPE2_REQUIRED = 3, // (::android::hardware::radio::V1_0::UusType.TYPE1_NOT_REQUIRED implicitly + 1)
    TYPE2_NOT_REQUIRED = 4, // (::android::hardware::radio::V1_0::UusType.TYPE2_REQUIRED implicitly + 1)
    TYPE3_REQUIRED = 5, // (::android::hardware::radio::V1_0::UusType.TYPE2_NOT_REQUIRED implicitly + 1)
    TYPE3_NOT_REQUIRED = 6, // (::android::hardware::radio::V1_0::UusType.TYPE3_REQUIRED implicitly + 1)
};

/**
 * User-to-User Signaling Information data coding schemes. Possible values for
 * Octet 3 (Protocol Discriminator field) in the UUIE. The values have been
 * specified in section 10.5.4.25 of 3GPP TS 24.008
 */
enum class UusDcs : int32_t {
    USP = 0,
    OSIHLP = 1, // (::android::hardware::radio::V1_0::UusDcs.USP implicitly + 1)
    X244 = 2, // (::android::hardware::radio::V1_0::UusDcs.OSIHLP implicitly + 1)
    RMCF = 3, // (::android::hardware::radio::V1_0::UusDcs.X244 implicitly + 1)
    IA5C = 4, // (::android::hardware::radio::V1_0::UusDcs.RMCF implicitly + 1)
};

enum class CallPresentation : int32_t {
    ALLOWED = 0,
    RESTRICTED = 1, // (::android::hardware::radio::V1_0::CallPresentation.ALLOWED implicitly + 1)
    UNKNOWN = 2, // (::android::hardware::radio::V1_0::CallPresentation.RESTRICTED implicitly + 1)
    PAYPHONE = 3, // (::android::hardware::radio::V1_0::CallPresentation.UNKNOWN implicitly + 1)
};

enum class Clir : int32_t {
    DEFAULT = 0,
    INVOCATION = 1, // (::android::hardware::radio::V1_0::Clir.DEFAULT implicitly + 1)
    SUPPRESSION = 2, // (::android::hardware::radio::V1_0::Clir.INVOCATION implicitly + 1)
};

enum class LastCallFailCause : int32_t {
    UNOBTAINABLE_NUMBER = 1,
    NO_ROUTE_TO_DESTINATION = 3,
    CHANNEL_UNACCEPTABLE = 6,
    OPERATOR_DETERMINED_BARRING = 8,
    NORMAL = 16,
    BUSY = 17,
    NO_USER_RESPONDING = 18,
    NO_ANSWER_FROM_USER = 19,
    CALL_REJECTED = 21,
    NUMBER_CHANGED = 22,
    PREEMPTION = 25,
    DESTINATION_OUT_OF_ORDER = 27,
    INVALID_NUMBER_FORMAT = 28,
    FACILITY_REJECTED = 29,
    RESP_TO_STATUS_ENQUIRY = 30,
    NORMAL_UNSPECIFIED = 31,
    CONGESTION = 34,
    NETWORK_OUT_OF_ORDER = 38,
    TEMPORARY_FAILURE = 41,
    SWITCHING_EQUIPMENT_CONGESTION = 42,
    ACCESS_INFORMATION_DISCARDED = 43,
    REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE = 44,
    RESOURCES_UNAVAILABLE_OR_UNSPECIFIED = 47,
    QOS_UNAVAILABLE = 49,
    REQUESTED_FACILITY_NOT_SUBSCRIBED = 50,
    INCOMING_CALLS_BARRED_WITHIN_CUG = 55,
    BEARER_CAPABILITY_NOT_AUTHORIZED = 57,
    BEARER_CAPABILITY_UNAVAILABLE = 58,
    SERVICE_OPTION_NOT_AVAILABLE = 63,
    BEARER_SERVICE_NOT_IMPLEMENTED = 65,
    ACM_LIMIT_EXCEEDED = 68,
    REQUESTED_FACILITY_NOT_IMPLEMENTED = 69,
    ONLY_DIGITAL_INFORMATION_BEARER_AVAILABLE = 70,
    SERVICE_OR_OPTION_NOT_IMPLEMENTED = 79,
    INVALID_TRANSACTION_IDENTIFIER = 81,
    USER_NOT_MEMBER_OF_CUG = 87,
    INCOMPATIBLE_DESTINATION = 88,
    INVALID_TRANSIT_NW_SELECTION = 91,
    SEMANTICALLY_INCORRECT_MESSAGE = 95,
    INVALID_MANDATORY_INFORMATION = 96,
    MESSAGE_TYPE_NON_IMPLEMENTED = 97,
    MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE = 98,
    INFORMATION_ELEMENT_NON_EXISTENT = 99,
    CONDITIONAL_IE_ERROR = 100,
    MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE = 101,
    RECOVERY_ON_TIMER_EXPIRED = 102,
    PROTOCOL_ERROR_UNSPECIFIED = 111,
    INTERWORKING_UNSPECIFIED = 127,
    CALL_BARRED = 240,
    FDN_BLOCKED = 241,
    IMSI_UNKNOWN_IN_VLR = 242,
    IMEI_NOT_ACCEPTED = 243,
    DIAL_MODIFIED_TO_USSD = 244,
    DIAL_MODIFIED_TO_SS = 245,
    DIAL_MODIFIED_TO_DIAL = 246,
    RADIO_OFF = 247,
    OUT_OF_SERVICE = 248,
    NO_VALID_SIM = 249,
    RADIO_INTERNAL_ERROR = 250,
    NETWORK_RESP_TIMEOUT = 251,
    NETWORK_REJECT = 252,
    RADIO_ACCESS_FAILURE = 253,
    RADIO_LINK_FAILURE = 254,
    RADIO_LINK_LOST = 255,
    RADIO_UPLINK_FAILURE = 256,
    RADIO_SETUP_FAILURE = 257,
    RADIO_RELEASE_NORMAL = 258,
    RADIO_RELEASE_ABNORMAL = 259,
    ACCESS_CLASS_BLOCKED = 260,
    NETWORK_DETACH = 261,
    CDMA_LOCKED_UNTIL_POWER_CYCLE = 1000,
    CDMA_DROP = 1001,
    CDMA_INTERCEPT = 1002,
    CDMA_REORDER = 1003,
    CDMA_SO_REJECT = 1004,
    CDMA_RETRY_ORDER = 1005,
    CDMA_ACCESS_FAILURE = 1006,
    CDMA_PREEMPTED = 1007,
    CDMA_NOT_EMERGENCY = 1008,
    CDMA_ACCESS_BLOCKED = 1009,
    /**
     * OEM specific error codes. Used to distinguish error from
     * CALL_FAIL_ERROR_UNSPECIFIED and help assist debugging  */
    OEM_CAUSE_1 = 61441,
    OEM_CAUSE_2 = 61442,
    OEM_CAUSE_3 = 61443,
    OEM_CAUSE_4 = 61444,
    OEM_CAUSE_5 = 61445,
    OEM_CAUSE_6 = 61446,
    OEM_CAUSE_7 = 61447,
    OEM_CAUSE_8 = 61448,
    OEM_CAUSE_9 = 61449,
    OEM_CAUSE_10 = 61450,
    OEM_CAUSE_11 = 61451,
    OEM_CAUSE_12 = 61452,
    OEM_CAUSE_13 = 61453,
    OEM_CAUSE_14 = 61454,
    OEM_CAUSE_15 = 61455,
    ERROR_UNSPECIFIED = 65535,
};

enum class DataCallFailCause : int32_t {
    NONE = 0,
    OPERATOR_BARRED = 8,
    NAS_SIGNALLING = 14,
    INSUFFICIENT_RESOURCES = 26,
    MISSING_UKNOWN_APN = 27,
    UNKNOWN_PDP_ADDRESS_TYPE = 28,
    USER_AUTHENTICATION = 29,
    ACTIVATION_REJECT_GGSN = 30,
    ACTIVATION_REJECT_UNSPECIFIED = 31,
    SERVICE_OPTION_NOT_SUPPORTED = 32,
    SERVICE_OPTION_NOT_SUBSCRIBED = 33,
    SERVICE_OPTION_OUT_OF_ORDER = 34,
    NSAPI_IN_USE = 35,
    REGULAR_DEACTIVATION = 36,
    QOS_NOT_ACCEPTED = 37,
    NETWORK_FAILURE = 38,
    UMTS_REACTIVATION_REQ = 39,
    FEATURE_NOT_SUPP = 40,
    TFT_SEMANTIC_ERROR = 41,
    TFT_SYTAX_ERROR = 42,
    UNKNOWN_PDP_CONTEXT = 43,
    FILTER_SEMANTIC_ERROR = 44,
    FILTER_SYTAX_ERROR = 45,
    PDP_WITHOUT_ACTIVE_TFT = 46,
    ONLY_IPV4_ALLOWED = 50,
    ONLY_IPV6_ALLOWED = 51,
    ONLY_SINGLE_BEARER_ALLOWED = 52,
    ESM_INFO_NOT_RECEIVED = 53,
    PDN_CONN_DOES_NOT_EXIST = 54,
    MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED = 55,
    MAX_ACTIVE_PDP_CONTEXT_REACHED = 65,
    UNSUPPORTED_APN_IN_CURRENT_PLMN = 66,
    INVALID_TRANSACTION_ID = 81,
    MESSAGE_INCORRECT_SEMANTIC = 95,
    INVALID_MANDATORY_INFO = 96,
    MESSAGE_TYPE_UNSUPPORTED = 97,
    MSG_TYPE_NONCOMPATIBLE_STATE = 98,
    UNKNOWN_INFO_ELEMENT = 99,
    CONDITIONAL_IE_ERROR = 100,
    MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE = 101,
    PROTOCOL_ERRORS = 111,
    APN_TYPE_CONFLICT = 112,
    INVALID_PCSCF_ADDR = 113,
    INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN = 114,
    EMM_ACCESS_BARRED = 115,
    EMERGENCY_IFACE_ONLY = 116,
    IFACE_MISMATCH = 117,
    COMPANION_IFACE_IN_USE = 118,
    IP_ADDRESS_MISMATCH = 119,
    IFACE_AND_POL_FAMILY_MISMATCH = 120,
    EMM_ACCESS_BARRED_INFINITE_RETRY = 121,
    AUTH_FAILURE_ON_EMERGENCY_CALL = 122,
    OEM_DCFAILCAUSE_1 = 4097,
    OEM_DCFAILCAUSE_2 = 4098,
    OEM_DCFAILCAUSE_3 = 4099,
    OEM_DCFAILCAUSE_4 = 4100,
    OEM_DCFAILCAUSE_5 = 4101,
    OEM_DCFAILCAUSE_6 = 4102,
    OEM_DCFAILCAUSE_7 = 4103,
    OEM_DCFAILCAUSE_8 = 4104,
    OEM_DCFAILCAUSE_9 = 4105,
    OEM_DCFAILCAUSE_10 = 4106,
    OEM_DCFAILCAUSE_11 = 4107,
    OEM_DCFAILCAUSE_12 = 4108,
    OEM_DCFAILCAUSE_13 = 4109,
    OEM_DCFAILCAUSE_14 = 4110,
    OEM_DCFAILCAUSE_15 = 4111,
    VOICE_REGISTRATION_FAIL = -1, // (-1)
    DATA_REGISTRATION_FAIL = -2, // (-2)
    SIGNAL_LOST = -3, // (-3)
    PREF_RADIO_TECH_CHANGED = -4, // (-4)
    RADIO_POWER_OFF = -5, // (-5)
    TETHERED_CALL_ACTIVE = -6, // (-6)
    ERROR_UNSPECIFIED = 65535,
};

/**
 * Please note that registration state UNKNOWN is
 * treated as "out of service" in the Android telephony.
 * Registration state REG_DENIED must be returned if Location Update
 * Reject (with cause 17 - Network Failure) is received
 * repeatedly from the network, to facilitate
 * "managed roaming"
 */
enum class RegState : int32_t {
    NOT_REG_MT_NOT_SEARCHING_OP = 0,
    REG_HOME = 1,
    NOT_REG_MT_SEARCHING_OP = 2,
    REG_DENIED = 3,
    UNKNOWN = 4,
    REG_ROAMING = 5,
    NOT_REG_MT_NOT_SEARCHING_OP_EM = 10,
    NOT_REG_MT_SEARCHING_OP_EM = 12,
    REG_DENIED_EM = 13,
    UNKNOWN_EM = 14,
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
};

enum class DataProfileId : int32_t {
    DEFAULT = 0,
    TETHERED = 1,
    IMS = 2,
    FOTA = 3,
    CBS = 4,
    OEM_BASE = 1000,
    INVALID = -1,
};

enum class SmsAcknowledgeFailCause : int32_t {
    MEMORY_CAPACITY_EXCEEDED = 211,
    UNSPECIFIED_ERROR = 255,
};

enum class CallForwardInfoStatus : int32_t {
    DISABLE = 0,
    ENABLE = 1, // (::android::hardware::radio::V1_0::CallForwardInfoStatus.DISABLE implicitly + 1)
    INTERROGATE = 2, // (::android::hardware::radio::V1_0::CallForwardInfoStatus.ENABLE implicitly + 1)
    REGISTRATION = 3, // (::android::hardware::radio::V1_0::CallForwardInfoStatus.INTERROGATE implicitly + 1)
    ERASURE = 4, // (::android::hardware::radio::V1_0::CallForwardInfoStatus.REGISTRATION implicitly + 1)
};

enum class ClipStatus : int32_t {
    CLIP_PROVISIONED = 0,
    CLIP_UNPROVISIONED = 1, // (::android::hardware::radio::V1_0::ClipStatus.CLIP_PROVISIONED implicitly + 1)
    UNKNOWN = 2, // (::android::hardware::radio::V1_0::ClipStatus.CLIP_UNPROVISIONED implicitly + 1)
};

enum class SmsWriteArgsStatus : int32_t {
    REC_UNREAD = 0,
    REC_READ = 1, // (::android::hardware::radio::V1_0::SmsWriteArgsStatus.REC_UNREAD implicitly + 1)
    STO_UNSENT = 2, // (::android::hardware::radio::V1_0::SmsWriteArgsStatus.REC_READ implicitly + 1)
    STO_SENT = 3, // (::android::hardware::radio::V1_0::SmsWriteArgsStatus.STO_UNSENT implicitly + 1)
};

enum class RadioBandMode : int32_t {
    BAND_MODE_UNSPECIFIED = 0,
    BAND_MODE_EURO = 1, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_UNSPECIFIED implicitly + 1)
    BAND_MODE_USA = 2, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_EURO implicitly + 1)
    BAND_MODE_JPN = 3, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_USA implicitly + 1)
    BAND_MODE_AUS = 4, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_JPN implicitly + 1)
    BAND_MODE_AUS_2 = 5, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_AUS implicitly + 1)
    BAND_MODE_CELL_800 = 6, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_AUS_2 implicitly + 1)
    BAND_MODE_PCS = 7, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_CELL_800 implicitly + 1)
    BAND_MODE_JTACS = 8, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_PCS implicitly + 1)
    BAND_MODE_KOREA_PCS = 9, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_JTACS implicitly + 1)
    BAND_MODE_5_450M = 10, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_KOREA_PCS implicitly + 1)
    BAND_MODE_IMT2000 = 11, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_5_450M implicitly + 1)
    BAND_MODE_7_700M_2 = 12, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_IMT2000 implicitly + 1)
    BAND_MODE_8_1800M = 13, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_7_700M_2 implicitly + 1)
    BAND_MODE_9_900M = 14, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_8_1800M implicitly + 1)
    BAND_MODE_10_800M_2 = 15, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_9_900M implicitly + 1)
    BAND_MODE_EURO_PAMR_400M = 16, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_10_800M_2 implicitly + 1)
    BAND_MODE_AWS = 17, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_EURO_PAMR_400M implicitly + 1)
    BAND_MODE_USA_2500M = 18, // (::android::hardware::radio::V1_0::RadioBandMode.BAND_MODE_AWS implicitly + 1)
};

enum class OperatorStatus : int32_t {
    UNKNOWN = 0,
    AVAILABLE = 1, // (::android::hardware::radio::V1_0::OperatorStatus.UNKNOWN implicitly + 1)
    CURRENT = 2, // (::android::hardware::radio::V1_0::OperatorStatus.AVAILABLE implicitly + 1)
    FORBIDDEN = 3, // (::android::hardware::radio::V1_0::OperatorStatus.CURRENT implicitly + 1)
};

enum class PreferredNetworkType : int32_t {
    GSM_WCDMA = 0,
    GSM_ONLY = 1, // (::android::hardware::radio::V1_0::PreferredNetworkType.GSM_WCDMA implicitly + 1)
    WCDMA = 2, // (::android::hardware::radio::V1_0::PreferredNetworkType.GSM_ONLY implicitly + 1)
    GSM_WCDMA_AUTO = 3, // (::android::hardware::radio::V1_0::PreferredNetworkType.WCDMA implicitly + 1)
    CDMA_EVDO_AUTO = 4, // (::android::hardware::radio::V1_0::PreferredNetworkType.GSM_WCDMA_AUTO implicitly + 1)
    CDMA_ONLY = 5, // (::android::hardware::radio::V1_0::PreferredNetworkType.CDMA_EVDO_AUTO implicitly + 1)
    EVDO_ONLY = 6, // (::android::hardware::radio::V1_0::PreferredNetworkType.CDMA_ONLY implicitly + 1)
    GSM_WCDMA_CDMA_EVDO_AUTO = 7, // (::android::hardware::radio::V1_0::PreferredNetworkType.EVDO_ONLY implicitly + 1)
    LTE_CDMA_EVDO = 8, // (::android::hardware::radio::V1_0::PreferredNetworkType.GSM_WCDMA_CDMA_EVDO_AUTO implicitly + 1)
    LTE_GSM_WCDMA = 9, // (::android::hardware::radio::V1_0::PreferredNetworkType.LTE_CDMA_EVDO implicitly + 1)
    LTE_CMDA_EVDO_GSM_WCDMA = 10, // (::android::hardware::radio::V1_0::PreferredNetworkType.LTE_GSM_WCDMA implicitly + 1)
    LTE_ONLY = 11, // (::android::hardware::radio::V1_0::PreferredNetworkType.LTE_CMDA_EVDO_GSM_WCDMA implicitly + 1)
    LTE_WCDMA = 12, // (::android::hardware::radio::V1_0::PreferredNetworkType.LTE_ONLY implicitly + 1)
    TD_SCDMA_ONLY = 13, // (::android::hardware::radio::V1_0::PreferredNetworkType.LTE_WCDMA implicitly + 1)
    TD_SCDMA_WCDMA = 14, // (::android::hardware::radio::V1_0::PreferredNetworkType.TD_SCDMA_ONLY implicitly + 1)
    TD_SCDMA_LTE = 15, // (::android::hardware::radio::V1_0::PreferredNetworkType.TD_SCDMA_WCDMA implicitly + 1)
    TD_SCDMA_GSM = 16, // (::android::hardware::radio::V1_0::PreferredNetworkType.TD_SCDMA_LTE implicitly + 1)
    TD_SCDMA_GSM_LTE = 17, // (::android::hardware::radio::V1_0::PreferredNetworkType.TD_SCDMA_GSM implicitly + 1)
    TD_SCDMA_GSM_WCDMA = 18, // (::android::hardware::radio::V1_0::PreferredNetworkType.TD_SCDMA_GSM_LTE implicitly + 1)
    TD_SCDMA_WCDMA_LTE = 19, // (::android::hardware::radio::V1_0::PreferredNetworkType.TD_SCDMA_GSM_WCDMA implicitly + 1)
    TD_SCDMA_GSM_WCDMA_LTE = 20, // (::android::hardware::radio::V1_0::PreferredNetworkType.TD_SCDMA_WCDMA_LTE implicitly + 1)
    TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO = 21, // (::android::hardware::radio::V1_0::PreferredNetworkType.TD_SCDMA_GSM_WCDMA_LTE implicitly + 1)
    TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA = 22, // (::android::hardware::radio::V1_0::PreferredNetworkType.TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO implicitly + 1)
};

enum class CdmaSubscriptionSource : int32_t {
    RUIM_SIM = 0,
    NV = 1, // (::android::hardware::radio::V1_0::CdmaSubscriptionSource.RUIM_SIM implicitly + 1)
};

enum class CdmaRoamingType : int32_t {
    HOME_NETWORK = 0,
    AFFILIATED_ROAM = 1, // (::android::hardware::radio::V1_0::CdmaRoamingType.HOME_NETWORK implicitly + 1)
    ANY_ROAM = 2, // (::android::hardware::radio::V1_0::CdmaRoamingType.AFFILIATED_ROAM implicitly + 1)
};

enum class TtyMode : int32_t {
    OFF = 0,
    FULL = 1, // (::android::hardware::radio::V1_0::TtyMode.OFF implicitly + 1)
    HCO = 2, // (::android::hardware::radio::V1_0::TtyMode.FULL implicitly + 1)
    VCO = 3, // (::android::hardware::radio::V1_0::TtyMode.HCO implicitly + 1)
};

enum class NvItem : int32_t {
    CDMA_MEID = 1,
    CDMA_MIN = 2,
    CDMA_MDN = 3,
    CDMA_ACCOLC = 4,
    DEVICE_MSL = 11,
    RTN_RECONDITIONED_STATUS = 12,
    RTN_ACTIVATION_DATE = 13,
    RTN_LIFE_TIMER = 14,
    RTN_LIFE_CALLS = 15,
    RTN_LIFE_DATA_TX = 16,
    RTN_LIFE_DATA_RX = 17,
    OMADM_HFA_LEVEL = 18,
    MIP_PROFILE_NAI = 31,
    MIP_PROFILE_HOME_ADDRESS = 32,
    MIP_PROFILE_AAA_AUTH = 33,
    MIP_PROFILE_HA_AUTH = 34,
    MIP_PROFILE_PRI_HA_ADDR = 35,
    MIP_PROFILE_SEC_HA_ADDR = 36,
    MIP_PROFILE_REV_TUN_PREF = 37,
    MIP_PROFILE_HA_SPI = 38,
    MIP_PROFILE_AAA_SPI = 39,
    MIP_PROFILE_MN_HA_SS = 40,
    MIP_PROFILE_MN_AAA_SS = 41,
    CDMA_PRL_VERSION = 51,
    CDMA_BC10 = 52,
    CDMA_BC14 = 53,
    CDMA_SO68 = 54,
    CDMA_SO73_COP0 = 55,
    CDMA_SO73_COP1TO7 = 56,
    CDMA_1X_ADVANCED_ENABLED = 57,
    CDMA_EHRPD_ENABLED = 58,
    CDMA_EHRPD_FORCED = 59,
    LTE_BAND_ENABLE_25 = 71,
    LTE_BAND_ENABLE_26 = 72,
    LTE_BAND_ENABLE_41 = 73,
    LTE_SCAN_PRIORITY_25 = 74,
    LTE_SCAN_PRIORITY_26 = 75,
    LTE_SCAN_PRIORITY_41 = 76,
    LTE_HIDDEN_BAND_PRIORITY_25 = 77,
    LTE_HIDDEN_BAND_PRIORITY_26 = 78,
    LTE_HIDDEN_BAND_PRIORITY_41 = 79,
};

enum class ResetNvType : int32_t {
    RELOAD = 0,
    ERASE = 1, // (::android::hardware::radio::V1_0::ResetNvType.RELOAD implicitly + 1)
    FACTORY_RESET = 2, // (::android::hardware::radio::V1_0::ResetNvType.ERASE implicitly + 1)
};

enum class HardwareConfigType : int32_t {
    MODEM = 0,
    SIM = 1, // (::android::hardware::radio::V1_0::HardwareConfigType.MODEM implicitly + 1)
};

enum class HardwareConfigState : int32_t {
    ENABLED = 0,
    STANDBY = 1, // (::android::hardware::radio::V1_0::HardwareConfigState.ENABLED implicitly + 1)
    DISABLED = 2, // (::android::hardware::radio::V1_0::HardwareConfigState.STANDBY implicitly + 1)
};

enum class LceStatus : int32_t {
    NOT_SUPPORTED = 0,
    STOPPED = 1, // (::android::hardware::radio::V1_0::LceStatus.NOT_SUPPORTED implicitly + 1)
    ACTIVE = 2, // (::android::hardware::radio::V1_0::LceStatus.STOPPED implicitly + 1)
};

enum class CarrierMatchType : int32_t {
    ALL = 0,
    SPN = 1,
    IMSI_PREFIX = 2,
    GID1 = 3,
    GID2 = 4,
};

struct NeighboringCell final {
    ::android::hardware::hidl_string cid __attribute__ ((aligned(8)));
    int32_t rssi __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::NeighboringCell, cid) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::NeighboringCell, rssi) == 16, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::NeighboringCell) == 24, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::NeighboringCell) == 8, "wrong alignment");

enum class CdmaSmsDigitMode : int32_t {
    FOUR_BIT = 0,
    EIGHT_BIT = 1, // (::android::hardware::radio::V1_0::CdmaSmsDigitMode.FOUR_BIT implicitly + 1)
};

enum class CdmaSmsNumberMode : int32_t {
    NOT_DATA_NETWORK = 0,
    DATA_NETWORK = 1, // (::android::hardware::radio::V1_0::CdmaSmsNumberMode.NOT_DATA_NETWORK implicitly + 1)
};

enum class CdmaSmsNumberType : int32_t {
    UNKNOWN = 0,
    INTERNATIONAL_OR_DATA_IP = 1, // (::android::hardware::radio::V1_0::CdmaSmsNumberType.UNKNOWN implicitly + 1)
    NATIONAL_OR_INTERNET_MAIL = 2, // (::android::hardware::radio::V1_0::CdmaSmsNumberType.INTERNATIONAL_OR_DATA_IP implicitly + 1)
    NETWORK = 3, // (::android::hardware::radio::V1_0::CdmaSmsNumberType.NATIONAL_OR_INTERNET_MAIL implicitly + 1)
    SUBSCRIBER = 4, // (::android::hardware::radio::V1_0::CdmaSmsNumberType.NETWORK implicitly + 1)
    ALPHANUMERIC = 5, // (::android::hardware::radio::V1_0::CdmaSmsNumberType.SUBSCRIBER implicitly + 1)
    ABBREVIATED = 6, // (::android::hardware::radio::V1_0::CdmaSmsNumberType.ALPHANUMERIC implicitly + 1)
    RESERVED_7 = 7, // (::android::hardware::radio::V1_0::CdmaSmsNumberType.ABBREVIATED implicitly + 1)
};

enum class CdmaSmsNumberPlan : int32_t {
    UNKNOWN = 0,
    TELEPHONY = 1, // (::android::hardware::radio::V1_0::CdmaSmsNumberPlan.UNKNOWN implicitly + 1)
    RESERVED_2 = 2, // (::android::hardware::radio::V1_0::CdmaSmsNumberPlan.TELEPHONY implicitly + 1)
    DATA = 3, // (::android::hardware::radio::V1_0::CdmaSmsNumberPlan.RESERVED_2 implicitly + 1)
    TELEX = 4, // (::android::hardware::radio::V1_0::CdmaSmsNumberPlan.DATA implicitly + 1)
    RESERVED_5 = 5, // (::android::hardware::radio::V1_0::CdmaSmsNumberPlan.TELEX implicitly + 1)
    RESERVED_6 = 6, // (::android::hardware::radio::V1_0::CdmaSmsNumberPlan.RESERVED_5 implicitly + 1)
    RESERVED_7 = 7, // (::android::hardware::radio::V1_0::CdmaSmsNumberPlan.RESERVED_6 implicitly + 1)
    RESERVED_8 = 8, // (::android::hardware::radio::V1_0::CdmaSmsNumberPlan.RESERVED_7 implicitly + 1)
    PRIVATE = 9, // (::android::hardware::radio::V1_0::CdmaSmsNumberPlan.RESERVED_8 implicitly + 1)
    RESERVED_10 = 10, // (::android::hardware::radio::V1_0::CdmaSmsNumberPlan.PRIVATE implicitly + 1)
    RESERVED_11 = 11, // (::android::hardware::radio::V1_0::CdmaSmsNumberPlan.RESERVED_10 implicitly + 1)
    RESERVED_12 = 12, // (::android::hardware::radio::V1_0::CdmaSmsNumberPlan.RESERVED_11 implicitly + 1)
    RESERVED_13 = 13, // (::android::hardware::radio::V1_0::CdmaSmsNumberPlan.RESERVED_12 implicitly + 1)
    RESERVED_14 = 14, // (::android::hardware::radio::V1_0::CdmaSmsNumberPlan.RESERVED_13 implicitly + 1)
    RESERVED_15 = 15, // (::android::hardware::radio::V1_0::CdmaSmsNumberPlan.RESERVED_14 implicitly + 1)
};

enum class CdmaSmsSubaddressType : int32_t {
    NSAP = 0,
    USER_SPECIFIED = 1, // (::android::hardware::radio::V1_0::CdmaSmsSubaddressType.NSAP implicitly + 1)
};

enum class CdmaSmsErrorClass : int32_t {
    NO_ERROR = 0,
    ERROR = 1, // (::android::hardware::radio::V1_0::CdmaSmsErrorClass.NO_ERROR implicitly + 1)
};

enum class CdmaSmsWriteArgsStatus : int32_t {
    REC_UNREAD = 0,
    REC_READ = 1, // (::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus.REC_UNREAD implicitly + 1)
    STO_UNSENT = 2, // (::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus.REC_READ implicitly + 1)
    STO_SENT = 3, // (::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus.STO_UNSENT implicitly + 1)
};

enum class CellInfoType : int32_t {
    NONE = 0,
    GSM = 1,
    CDMA = 2,
    LTE = 3,
    WCDMA = 4,
    TD_SCDMA = 5,
};

enum class TimeStampType : int32_t {
    UNKNOWN = 0,
    ANTENNA = 1, // (::android::hardware::radio::V1_0::TimeStampType.UNKNOWN implicitly + 1)
    MODEM = 2, // (::android::hardware::radio::V1_0::TimeStampType.ANTENNA implicitly + 1)
    OEM_RIL = 3, // (::android::hardware::radio::V1_0::TimeStampType.MODEM implicitly + 1)
    JAVA_RIL = 4, // (::android::hardware::radio::V1_0::TimeStampType.OEM_RIL implicitly + 1)
};

enum class ApnAuthType : int32_t {
    NO_PAP_NO_CHAP = 0,
    PAP_NO_CHAP = 1, // (::android::hardware::radio::V1_0::ApnAuthType.NO_PAP_NO_CHAP implicitly + 1)
    NO_PAP_CHAP = 2, // (::android::hardware::radio::V1_0::ApnAuthType.PAP_NO_CHAP implicitly + 1)
    PAP_CHAP = 3, // (::android::hardware::radio::V1_0::ApnAuthType.NO_PAP_CHAP implicitly + 1)
};

enum class RadioTechnologyFamily : int32_t {
    THREE_GPP = 0,
    THREE_GPP2 = 1, // (::android::hardware::radio::V1_0::RadioTechnologyFamily.THREE_GPP implicitly + 1)
};

enum class RadioCapabilityPhase : int32_t {
    CONFIGURED = 0,
    START = 1,
    APPLY = 2,
    UNSOL_RSP = 3,
    FINISH = 4,
};

enum class RadioCapabilityStatus : int32_t {
    NONE = 0,
    SUCCESS = 1,
    FAIL = 2,
};

enum class RadioAccessFamily : int32_t {
    UNKNOWN = 1, // (1 << RadioTechnology:UNKNOWN)
    GPRS = 2, // (1 << RadioTechnology:GPRS)
    EDGE = 4, // (1 << RadioTechnology:EDGE)
    UMTS = 8, // (1 << RadioTechnology:UMTS)
    IS95A = 16, // (1 << RadioTechnology:IS95A)
    IS95B = 32, // (1 << RadioTechnology:IS95B)
    ONE_X_RTT = 64, // (1 << RadioTechnology:ONE_X_RTT)
    EVDO_0 = 128, // (1 << RadioTechnology:EVDO_0)
    EVDO_A = 256, // (1 << RadioTechnology:EVDO_A)
    HSDPA = 512, // (1 << RadioTechnology:HSDPA)
    HSUPA = 1024, // (1 << RadioTechnology:HSUPA)
    HSPA = 2048, // (1 << RadioTechnology:HSPA)
    EVDO_B = 4096, // (1 << RadioTechnology:EVDO_B)
    EHRPD = 8192, // (1 << RadioTechnology:EHRPD)
    LTE = 16384, // (1 << RadioTechnology:LTE)
    HSPAP = 32768, // (1 << RadioTechnology:HSPAP)
    GSM = 65536, // (1 << RadioTechnology:GSM)
    TD_SCDMA = 131072, // (1 << RadioTechnology:TD_SCDMA)
    LTE_CA = 524288, // (1 << RadioTechnology:LTE_CA)
};

enum class UssdModeType : int32_t {
    NOTIFY = 0,
    REQUEST = 1, // (::android::hardware::radio::V1_0::UssdModeType.NOTIFY implicitly + 1)
    NW_RELEASE = 2, // (::android::hardware::radio::V1_0::UssdModeType.REQUEST implicitly + 1)
    LOCAL_CLIENT = 3, // (::android::hardware::radio::V1_0::UssdModeType.NW_RELEASE implicitly + 1)
    NOT_SUPPORTED = 4, // (::android::hardware::radio::V1_0::UssdModeType.LOCAL_CLIENT implicitly + 1)
    NW_TIMEOUT = 5, // (::android::hardware::radio::V1_0::UssdModeType.NOT_SUPPORTED implicitly + 1)
};

enum class SimRefreshType : int32_t {
    SIM_FILE_UPDATE = 0,
    SIM_INIT = 1,
    SIM_RESET = 2,
};

enum class SrvccState : int32_t {
    HANDOVER_STARTED = 0,
    HANDOVER_COMPLETED = 1,
    HANDOVER_FAILED = 2,
    HANDOVER_CANCELED = 3,
};

enum class UiccSubActStatus : int32_t {
    DEACTIVATE = 0,
    ACTIVATE = 1, // (::android::hardware::radio::V1_0::UiccSubActStatus.DEACTIVATE implicitly + 1)
};

enum class SubscriptionType : int32_t {
    SUBSCRIPTION_1 = 0,
    SUBSCRIPTION_2 = 1, // (::android::hardware::radio::V1_0::SubscriptionType.SUBSCRIPTION_1 implicitly + 1)
    SUBSCRIPTION_3 = 2, // (::android::hardware::radio::V1_0::SubscriptionType.SUBSCRIPTION_2 implicitly + 1)
};

enum class DataProfileInfoType : int32_t {
    COMMON = 0,
    THREE_GPP = 1, // (::android::hardware::radio::V1_0::DataProfileInfoType.COMMON implicitly + 1)
    THREE_GPP2 = 2, // (::android::hardware::radio::V1_0::DataProfileInfoType.THREE_GPP implicitly + 1)
};

enum class PhoneRestrictedState : int32_t {
    NONE = 0,
    CS_EMERGENCY = 1,
    CS_NORMAL = 2,
    CS_ALL = 4,
    PS_ALL = 16,
};

enum class CdmaCallWaitingNumberPresentation : int32_t {
    ALLOWED = 0,
    RESTRICTED = 1,
    UNKNOWN = 2,
};

enum class CdmaCallWaitingNumberType : int32_t {
    UNKNOWN = 0,
    INTERNATIONAL = 1,
    NATIONAL = 2,
    NETWORK_SPECIFIC = 3,
    SUBSCRIBER = 4,
};

enum class CdmaCallWaitingNumberPlan : int32_t {
    UNKNOWN = 0,
    ISDN = 1,
    DATA = 3,
    TELEX = 4,
    NATIONAL = 8,
    PRIVATE = 9,
};

enum class CdmaOtaProvisionStatus : int32_t {
    SPL_UNLOCKED = 0,
    SPC_RETRIES_EXCEEDED = 1, // (::android::hardware::radio::V1_0::CdmaOtaProvisionStatus.SPL_UNLOCKED implicitly + 1)
    A_KEY_EXCHANGED = 2, // (::android::hardware::radio::V1_0::CdmaOtaProvisionStatus.SPC_RETRIES_EXCEEDED implicitly + 1)
    SSD_UPDATED = 3, // (::android::hardware::radio::V1_0::CdmaOtaProvisionStatus.A_KEY_EXCHANGED implicitly + 1)
    NAM_DOWNLOADED = 4, // (::android::hardware::radio::V1_0::CdmaOtaProvisionStatus.SSD_UPDATED implicitly + 1)
    MDN_DOWNLOADED = 5, // (::android::hardware::radio::V1_0::CdmaOtaProvisionStatus.NAM_DOWNLOADED implicitly + 1)
    IMSI_DOWNLOADED = 6, // (::android::hardware::radio::V1_0::CdmaOtaProvisionStatus.MDN_DOWNLOADED implicitly + 1)
    PRL_DOWNLOADED = 7, // (::android::hardware::radio::V1_0::CdmaOtaProvisionStatus.IMSI_DOWNLOADED implicitly + 1)
    COMMITTED = 8, // (::android::hardware::radio::V1_0::CdmaOtaProvisionStatus.PRL_DOWNLOADED implicitly + 1)
    OTAPA_STARTED = 9, // (::android::hardware::radio::V1_0::CdmaOtaProvisionStatus.COMMITTED implicitly + 1)
    OTAPA_STOPPED = 10, // (::android::hardware::radio::V1_0::CdmaOtaProvisionStatus.OTAPA_STARTED implicitly + 1)
    OTAPA_ABORTED = 11, // (::android::hardware::radio::V1_0::CdmaOtaProvisionStatus.OTAPA_STOPPED implicitly + 1)
};

/**
 * Names of the CDMA info records (C.S0005 section 3.7.5)  */
enum class CdmaInfoRecName : int32_t {
    DISPLAY = 0,
    CALLED_PARTY_NUMBER = 1, // (::android::hardware::radio::V1_0::CdmaInfoRecName.DISPLAY implicitly + 1)
    CALLING_PARTY_NUMBER = 2, // (::android::hardware::radio::V1_0::CdmaInfoRecName.CALLED_PARTY_NUMBER implicitly + 1)
    CONNECTED_NUMBER = 3, // (::android::hardware::radio::V1_0::CdmaInfoRecName.CALLING_PARTY_NUMBER implicitly + 1)
    SIGNAL = 4, // (::android::hardware::radio::V1_0::CdmaInfoRecName.CONNECTED_NUMBER implicitly + 1)
    REDIRECTING_NUMBER = 5, // (::android::hardware::radio::V1_0::CdmaInfoRecName.SIGNAL implicitly + 1)
    LINE_CONTROL = 6, // (::android::hardware::radio::V1_0::CdmaInfoRecName.REDIRECTING_NUMBER implicitly + 1)
    EXTENDED_DISPLAY = 7, // (::android::hardware::radio::V1_0::CdmaInfoRecName.LINE_CONTROL implicitly + 1)
    T53_CLIR = 8, // (::android::hardware::radio::V1_0::CdmaInfoRecName.EXTENDED_DISPLAY implicitly + 1)
    T53_RELEASE = 9, // (::android::hardware::radio::V1_0::CdmaInfoRecName.T53_CLIR implicitly + 1)
    T53_AUDIO_CONTROL = 10, // (::android::hardware::radio::V1_0::CdmaInfoRecName.T53_RELEASE implicitly + 1)
};

/**
 * Redirecting Number Information Record as defined in C.S0005 section 3.7.5.11  */
enum class CdmaRedirectingReason : int32_t {
    UNKNOWN = 0,
    CALL_FORWARDING_BUSY = 1,
    CALL_FORWARDING_NO_REPLY = 2,
    CALLED_DTE_OUT_OF_ORDER = 9,
    CALL_FORWARDING_BY_THE_CALLED_DTE = 10,
    CALL_FORWARDING_UNCONDITIONAL = 15,
    RESERVED = 16, // (::android::hardware::radio::V1_0::CdmaRedirectingReason.CALL_FORWARDING_UNCONDITIONAL implicitly + 1)
};

enum class SsServiceType : int32_t {
    CFU = 0,
    CF_BUSY = 1, // (::android::hardware::radio::V1_0::SsServiceType.CFU implicitly + 1)
    CF_NO_REPLY = 2, // (::android::hardware::radio::V1_0::SsServiceType.CF_BUSY implicitly + 1)
    CF_NOT_REACHABLE = 3, // (::android::hardware::radio::V1_0::SsServiceType.CF_NO_REPLY implicitly + 1)
    CF_ALL = 4, // (::android::hardware::radio::V1_0::SsServiceType.CF_NOT_REACHABLE implicitly + 1)
    CF_ALL_CONDITIONAL = 5, // (::android::hardware::radio::V1_0::SsServiceType.CF_ALL implicitly + 1)
    CLIP = 6, // (::android::hardware::radio::V1_0::SsServiceType.CF_ALL_CONDITIONAL implicitly + 1)
    CLIR = 7, // (::android::hardware::radio::V1_0::SsServiceType.CLIP implicitly + 1)
    COLP = 8, // (::android::hardware::radio::V1_0::SsServiceType.CLIR implicitly + 1)
    COLR = 9, // (::android::hardware::radio::V1_0::SsServiceType.COLP implicitly + 1)
    WAIT = 10, // (::android::hardware::radio::V1_0::SsServiceType.COLR implicitly + 1)
    BAOC = 11, // (::android::hardware::radio::V1_0::SsServiceType.WAIT implicitly + 1)
    BAOIC = 12, // (::android::hardware::radio::V1_0::SsServiceType.BAOC implicitly + 1)
    BAOIC_EXC_HOME = 13, // (::android::hardware::radio::V1_0::SsServiceType.BAOIC implicitly + 1)
    BAIC = 14, // (::android::hardware::radio::V1_0::SsServiceType.BAOIC_EXC_HOME implicitly + 1)
    BAIC_ROAMING = 15, // (::android::hardware::radio::V1_0::SsServiceType.BAIC implicitly + 1)
    ALL_BARRING = 16, // (::android::hardware::radio::V1_0::SsServiceType.BAIC_ROAMING implicitly + 1)
    OUTGOING_BARRING = 17, // (::android::hardware::radio::V1_0::SsServiceType.ALL_BARRING implicitly + 1)
    INCOMING_BARRING = 18, // (::android::hardware::radio::V1_0::SsServiceType.OUTGOING_BARRING implicitly + 1)
};

enum class SsRequestType : int32_t {
    ACTIVATION = 0,
    DEACTIVATION = 1, // (::android::hardware::radio::V1_0::SsRequestType.ACTIVATION implicitly + 1)
    INTERROGATION = 2, // (::android::hardware::radio::V1_0::SsRequestType.DEACTIVATION implicitly + 1)
    REGISTRATION = 3, // (::android::hardware::radio::V1_0::SsRequestType.INTERROGATION implicitly + 1)
    ERASURE = 4, // (::android::hardware::radio::V1_0::SsRequestType.REGISTRATION implicitly + 1)
};

enum class SsTeleserviceType : int32_t {
    ALL_TELE_AND_BEARER_SERVICES = 0,
    ALL_TELESEVICES = 1, // (::android::hardware::radio::V1_0::SsTeleserviceType.ALL_TELE_AND_BEARER_SERVICES implicitly + 1)
    TELEPHONY = 2, // (::android::hardware::radio::V1_0::SsTeleserviceType.ALL_TELESEVICES implicitly + 1)
    ALL_DATA_TELESERVICES = 3, // (::android::hardware::radio::V1_0::SsTeleserviceType.TELEPHONY implicitly + 1)
    SMS_SERVICES = 4, // (::android::hardware::radio::V1_0::SsTeleserviceType.ALL_DATA_TELESERVICES implicitly + 1)
    ALL_TELESERVICES_EXCEPT_SMS = 5, // (::android::hardware::radio::V1_0::SsTeleserviceType.SMS_SERVICES implicitly + 1)
};

enum class SuppServiceClass : int32_t {
    NONE = 0,
    VOICE = 1, // (1 << 0)
    DATA = 2, // (1 << 1)
    FAX = 4, // (1 << 2)
    SMS = 8, // (1 << 3)
    DATA_SYNC = 16, // (1 << 4)
    DATA_ASYNC = 32, // (1 << 5)
    PACKET = 64, // (1 << 6)
    PAD = 128, // (1 << 7)
    MAX = 128, // (1 << 7)
};

enum class ApnTypes : int32_t {
    NONE = 0,
    DEFAULT = 1, // (1 << 0)
    MMS = 2, // (1 << 1)
    SUPL = 4, // (1 << 2)
    DUN = 8, // (1 << 3)
    HIPRI = 16, // (1 << 4)
    FOTA = 32, // (1 << 5)
    IMS = 64, // (1 << 6)
    CBS = 128, // (1 << 7)
    IA = 256, // (1 << 8)
    EMERGENCY = 512, // (1 << 9)
    ALL = 1023, // (((((((((DEFAULT | MMS) | SUPL) | DUN) | HIPRI) | FOTA) | IMS) | CBS) | IA) | EMERGENCY)
};

enum class IndicationFilter : int32_t {
    NONE = 0,
    SIGNAL_STRENGTH = 1, // (1 << 0)
    FULL_NETWORK_STATE = 2, // (1 << 1)
    DATA_CALL_DORMANCY_CHANGED = 4, // (1 << 2)
    ALL = 7, // ((SIGNAL_STRENGTH | FULL_NETWORK_STATE) | DATA_CALL_DORMANCY_CHANGED)
};

enum class MvnoType : int32_t {
    NONE = 0,
    IMSI = 1, // (::android::hardware::radio::V1_0::MvnoType.NONE implicitly + 1)
    GID = 2, // (::android::hardware::radio::V1_0::MvnoType.IMSI implicitly + 1)
    SPN = 3, // (::android::hardware::radio::V1_0::MvnoType.GID implicitly + 1)
};

enum class DeviceStateType : int32_t {
    POWER_SAVE_MODE = 0,
    CHARGING_STATE = 1, // (::android::hardware::radio::V1_0::DeviceStateType.POWER_SAVE_MODE implicitly + 1)
    LOW_DATA_EXPECTED = 2, // (::android::hardware::radio::V1_0::DeviceStateType.CHARGING_STATE implicitly + 1)
};

enum class P2Constant : int32_t {
    NO_P2 = -1, // (-1)
};

struct RadioResponseInfo final {
    ::android::hardware::radio::V1_0::RadioResponseType type __attribute__ ((aligned(4)));
    int32_t serial __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::RadioError error __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::RadioResponseInfo, type) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::RadioResponseInfo, serial) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::RadioResponseInfo, error) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::RadioResponseInfo) == 12, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::RadioResponseInfo) == 4, "wrong alignment");

struct AppStatus final {
    ::android::hardware::radio::V1_0::AppType appType __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::AppState appState __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::PersoSubstate persoSubstate __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string aidPtr __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string appLabelPtr __attribute__ ((aligned(8)));
    int32_t pin1Replaced __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::PinState pin1 __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::PinState pin2 __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::AppStatus, appType) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::AppStatus, appState) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::AppStatus, persoSubstate) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::AppStatus, aidPtr) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::AppStatus, appLabelPtr) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::AppStatus, pin1Replaced) == 48, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::AppStatus, pin1) == 52, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::AppStatus, pin2) == 56, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::AppStatus) == 64, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::AppStatus) == 8, "wrong alignment");

struct CardStatus final {
    ::android::hardware::radio::V1_0::CardState cardState __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::PinState universalPinState __attribute__ ((aligned(4)));
    int32_t gsmUmtsSubscriptionAppIndex __attribute__ ((aligned(4)));
    int32_t cdmaSubscriptionAppIndex __attribute__ ((aligned(4)));
    int32_t imsSubscriptionAppIndex __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::AppStatus> applications __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CardStatus, cardState) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CardStatus, universalPinState) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CardStatus, gsmUmtsSubscriptionAppIndex) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CardStatus, cdmaSubscriptionAppIndex) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CardStatus, imsSubscriptionAppIndex) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CardStatus, applications) == 24, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CardStatus) == 40, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CardStatus) == 8, "wrong alignment");

/**
 * User-to-User Signaling Information defined in 3GPP 23.087 v8.0
 */
struct UusInfo final {
    ::android::hardware::radio::V1_0::UusType uusType __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::UusDcs uusDcs __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string uusData __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::UusInfo, uusType) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::UusInfo, uusDcs) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::UusInfo, uusData) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::UusInfo) == 24, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::UusInfo) == 8, "wrong alignment");

struct Call final {
    ::android::hardware::radio::V1_0::CallState state __attribute__ ((aligned(4)));
    int32_t index __attribute__ ((aligned(4)));
    int32_t toa __attribute__ ((aligned(4)));
    bool isMpty __attribute__ ((aligned(1)));
    bool isMT __attribute__ ((aligned(1)));
    uint8_t als __attribute__ ((aligned(1)));
    bool isVoice __attribute__ ((aligned(1)));
    bool isVoicePrivacy __attribute__ ((aligned(1)));
    ::android::hardware::hidl_string number __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::CallPresentation numberPresentation __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string name __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::CallPresentation namePresentation __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::UusInfo> uusInfo __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::Call, state) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Call, index) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Call, toa) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Call, isMpty) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Call, isMT) == 13, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Call, als) == 14, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Call, isVoice) == 15, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Call, isVoicePrivacy) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Call, number) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Call, numberPresentation) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Call, name) == 48, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Call, namePresentation) == 64, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Call, uusInfo) == 72, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::Call) == 88, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::Call) == 8, "wrong alignment");

struct Dial final {
    ::android::hardware::hidl_string address __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::Clir clir __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::UusInfo> uusInfo __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::Dial, address) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Dial, clir) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Dial, uusInfo) == 24, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::Dial) == 40, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::Dial) == 8, "wrong alignment");

struct LastCallFailCauseInfo final {
    ::android::hardware::radio::V1_0::LastCallFailCause causeCode __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string vendorCause __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::LastCallFailCauseInfo, causeCode) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::LastCallFailCauseInfo, vendorCause) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::LastCallFailCauseInfo) == 24, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::LastCallFailCauseInfo) == 8, "wrong alignment");

struct GsmSignalStrength final {
    uint32_t signalStrength __attribute__ ((aligned(4)));
    uint32_t bitErrorRate __attribute__ ((aligned(4)));
    int32_t timingAdvance __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::GsmSignalStrength, signalStrength) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::GsmSignalStrength, bitErrorRate) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::GsmSignalStrength, timingAdvance) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::GsmSignalStrength) == 12, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::GsmSignalStrength) == 4, "wrong alignment");

struct WcdmaSignalStrength final {
    int32_t signalStrength __attribute__ ((aligned(4)));
    int32_t bitErrorRate __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::WcdmaSignalStrength, signalStrength) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::WcdmaSignalStrength, bitErrorRate) == 4, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::WcdmaSignalStrength) == 8, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::WcdmaSignalStrength) == 4, "wrong alignment");

struct CdmaSignalStrength final {
    uint32_t dbm __attribute__ ((aligned(4)));
    uint32_t ecio __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSignalStrength, dbm) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSignalStrength, ecio) == 4, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaSignalStrength) == 8, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaSignalStrength) == 4, "wrong alignment");

struct EvdoSignalStrength final {
    uint32_t dbm __attribute__ ((aligned(4)));
    uint32_t ecio __attribute__ ((aligned(4)));
    uint32_t signalNoiseRatio __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::EvdoSignalStrength, dbm) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::EvdoSignalStrength, ecio) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::EvdoSignalStrength, signalNoiseRatio) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::EvdoSignalStrength) == 12, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::EvdoSignalStrength) == 4, "wrong alignment");

struct LteSignalStrength final {
    uint32_t signalStrength __attribute__ ((aligned(4)));
    uint32_t rsrp __attribute__ ((aligned(4)));
    uint32_t rsrq __attribute__ ((aligned(4)));
    int32_t rssnr __attribute__ ((aligned(4)));
    uint32_t cqi __attribute__ ((aligned(4)));
    uint32_t timingAdvance __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::LteSignalStrength, signalStrength) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::LteSignalStrength, rsrp) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::LteSignalStrength, rsrq) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::LteSignalStrength, rssnr) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::LteSignalStrength, cqi) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::LteSignalStrength, timingAdvance) == 20, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::LteSignalStrength) == 24, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::LteSignalStrength) == 4, "wrong alignment");

struct TdScdmaSignalStrength final {
    uint32_t rscp __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::TdScdmaSignalStrength, rscp) == 0, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::TdScdmaSignalStrength) == 4, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::TdScdmaSignalStrength) == 4, "wrong alignment");

struct SignalStrength final {
    ::android::hardware::radio::V1_0::GsmSignalStrength gw __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::CdmaSignalStrength cdma __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::EvdoSignalStrength evdo __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::LteSignalStrength lte __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::TdScdmaSignalStrength tdScdma __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::SignalStrength, gw) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SignalStrength, cdma) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SignalStrength, evdo) == 20, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SignalStrength, lte) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SignalStrength, tdScdma) == 56, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::SignalStrength) == 60, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::SignalStrength) == 4, "wrong alignment");

struct SendSmsResult final {
    int32_t messageRef __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string ackPDU __attribute__ ((aligned(8)));
    int32_t errorCode __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::SendSmsResult, messageRef) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SendSmsResult, ackPDU) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SendSmsResult, errorCode) == 24, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::SendSmsResult) == 32, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::SendSmsResult) == 8, "wrong alignment");

struct SetupDataCallResult final {
    ::android::hardware::radio::V1_0::DataCallFailCause status __attribute__ ((aligned(4)));
    int32_t suggestedRetryTime __attribute__ ((aligned(4)));
    int32_t cid __attribute__ ((aligned(4)));
    int32_t active __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string type __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string ifname __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string addresses __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string dnses __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string gateways __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string pcscf __attribute__ ((aligned(8)));
    int32_t mtu __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::SetupDataCallResult, status) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SetupDataCallResult, suggestedRetryTime) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SetupDataCallResult, cid) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SetupDataCallResult, active) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SetupDataCallResult, type) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SetupDataCallResult, ifname) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SetupDataCallResult, addresses) == 48, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SetupDataCallResult, dnses) == 64, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SetupDataCallResult, gateways) == 80, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SetupDataCallResult, pcscf) == 96, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SetupDataCallResult, mtu) == 112, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::SetupDataCallResult) == 120, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::SetupDataCallResult) == 8, "wrong alignment");

struct IccIo final {
    int32_t command __attribute__ ((aligned(4)));
    int32_t fileId __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string path __attribute__ ((aligned(8)));
    int32_t p1 __attribute__ ((aligned(4)));
    int32_t p2 __attribute__ ((aligned(4)));
    int32_t p3 __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string data __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string pin2 __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string aid __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::IccIo, command) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::IccIo, fileId) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::IccIo, path) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::IccIo, p1) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::IccIo, p2) == 28, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::IccIo, p3) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::IccIo, data) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::IccIo, pin2) == 56, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::IccIo, aid) == 72, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::IccIo) == 88, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::IccIo) == 8, "wrong alignment");

struct IccIoResult final {
    int32_t sw1 __attribute__ ((aligned(4)));
    int32_t sw2 __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string simResponse __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::IccIoResult, sw1) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::IccIoResult, sw2) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::IccIoResult, simResponse) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::IccIoResult) == 24, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::IccIoResult) == 8, "wrong alignment");

struct CallForwardInfo final {
    ::android::hardware::radio::V1_0::CallForwardInfoStatus status __attribute__ ((aligned(4)));
    int32_t reason __attribute__ ((aligned(4)));
    int32_t serviceClass __attribute__ ((aligned(4)));
    int32_t toa __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string number __attribute__ ((aligned(8)));
    int32_t timeSeconds __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CallForwardInfo, status) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CallForwardInfo, reason) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CallForwardInfo, serviceClass) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CallForwardInfo, toa) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CallForwardInfo, number) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CallForwardInfo, timeSeconds) == 32, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CallForwardInfo) == 40, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CallForwardInfo) == 8, "wrong alignment");

struct OperatorInfo final {
    ::android::hardware::hidl_string alphaLong __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string alphaShort __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string operatorNumeric __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::OperatorStatus status __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::OperatorInfo, alphaLong) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::OperatorInfo, alphaShort) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::OperatorInfo, operatorNumeric) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::OperatorInfo, status) == 48, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::OperatorInfo) == 56, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::OperatorInfo) == 8, "wrong alignment");

struct SmsWriteArgs final {
    ::android::hardware::radio::V1_0::SmsWriteArgsStatus status __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string pdu __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string smsc __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::SmsWriteArgs, status) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SmsWriteArgs, pdu) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SmsWriteArgs, smsc) == 24, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::SmsWriteArgs) == 40, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::SmsWriteArgs) == 8, "wrong alignment");

struct CdmaSmsAddress final {
    ::android::hardware::radio::V1_0::CdmaSmsDigitMode digitMode __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::CdmaSmsNumberMode numberMode __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::CdmaSmsNumberType numberType __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::CdmaSmsNumberPlan numberPlan __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> digits __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsAddress, digitMode) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsAddress, numberMode) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsAddress, numberType) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsAddress, numberPlan) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsAddress, digits) == 16, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaSmsAddress) == 32, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaSmsAddress) == 8, "wrong alignment");

struct CdmaSmsSubaddress final {
    ::android::hardware::radio::V1_0::CdmaSmsSubaddressType subaddressType __attribute__ ((aligned(4)));
    bool odd __attribute__ ((aligned(1)));
    ::android::hardware::hidl_vec<uint8_t> digits __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsSubaddress, subaddressType) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsSubaddress, odd) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsSubaddress, digits) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaSmsSubaddress) == 24, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaSmsSubaddress) == 8, "wrong alignment");

struct CdmaSmsMessage final {
    int32_t teleserviceId __attribute__ ((aligned(4)));
    bool isServicePresent __attribute__ ((aligned(1)));
    int32_t serviceCategory __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::CdmaSmsAddress address __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::CdmaSmsSubaddress subAddress __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<uint8_t> bearerData __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsMessage, teleserviceId) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsMessage, isServicePresent) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsMessage, serviceCategory) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsMessage, address) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsMessage, subAddress) == 48, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsMessage, bearerData) == 72, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaSmsMessage) == 88, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaSmsMessage) == 8, "wrong alignment");

struct CdmaSmsAck final {
    ::android::hardware::radio::V1_0::CdmaSmsErrorClass errorClass __attribute__ ((aligned(4)));
    int32_t smsCauseCode __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsAck, errorClass) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsAck, smsCauseCode) == 4, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaSmsAck) == 8, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaSmsAck) == 4, "wrong alignment");

struct CdmaBroadcastSmsConfigInfo final {
    int32_t serviceCategory __attribute__ ((aligned(4)));
    int32_t language __attribute__ ((aligned(4)));
    bool selected __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaBroadcastSmsConfigInfo, serviceCategory) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaBroadcastSmsConfigInfo, language) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaBroadcastSmsConfigInfo, selected) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaBroadcastSmsConfigInfo) == 12, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaBroadcastSmsConfigInfo) == 4, "wrong alignment");

struct CdmaSmsWriteArgs final {
    ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus status __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::CdmaSmsMessage message __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsWriteArgs, status) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSmsWriteArgs, message) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaSmsWriteArgs) == 96, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaSmsWriteArgs) == 8, "wrong alignment");

/**
 * Which types of Cell Broadcast Message (CBM) are to be received by the ME
 * 
 * fromServiceID - uToServiceID defines a range of CBM message identifiers
 * whose value is 0x0000 - 0xFFFF as defined in TS 23.041 9.4.1.2.2 for GMS
 * and 9.4.4.2.2 for UMTS. All other values must be treated as empty
 * CBM message ID.
 * 
 * fromCodeScheme - uToCodeScheme defines a range of CBM data coding schemes
 * whose value is 0x00 - 0xFF as defined in TS 23.041 9.4.1.2.3 for GMS
 * and 9.4.4.2.3 for UMTS.
 * All other values must be treated as empty CBM data coding scheme.
 * 
 * selected false means message types specified in <fromServiceId, toServiceId>
 * and <fromCodeScheme, toCodeScheme>are not accepted, while true means accepted.
 */
struct GsmBroadcastSmsConfigInfo final {
    int32_t fromServiceId __attribute__ ((aligned(4)));
    int32_t toServiceId __attribute__ ((aligned(4)));
    int32_t fromCodeScheme __attribute__ ((aligned(4)));
    int32_t toCodeScheme __attribute__ ((aligned(4)));
    bool selected __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo, fromServiceId) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo, toServiceId) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo, fromCodeScheme) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo, toCodeScheme) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo, selected) == 16, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo) == 20, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo) == 4, "wrong alignment");

struct CellIdentityGsm final {
    ::android::hardware::hidl_string mcc __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string mnc __attribute__ ((aligned(8)));
    int32_t lac __attribute__ ((aligned(4)));
    int32_t cid __attribute__ ((aligned(4)));
    int32_t arfcn __attribute__ ((aligned(4)));
    uint8_t bsic __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityGsm, mcc) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityGsm, mnc) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityGsm, lac) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityGsm, cid) == 36, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityGsm, arfcn) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityGsm, bsic) == 44, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CellIdentityGsm) == 48, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CellIdentityGsm) == 8, "wrong alignment");

struct CellIdentityWcdma final {
    ::android::hardware::hidl_string mcc __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string mnc __attribute__ ((aligned(8)));
    int32_t lac __attribute__ ((aligned(4)));
    int32_t cid __attribute__ ((aligned(4)));
    int32_t psc __attribute__ ((aligned(4)));
    int32_t uarfcn __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityWcdma, mcc) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityWcdma, mnc) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityWcdma, lac) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityWcdma, cid) == 36, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityWcdma, psc) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityWcdma, uarfcn) == 44, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CellIdentityWcdma) == 48, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CellIdentityWcdma) == 8, "wrong alignment");

struct CellIdentityCdma final {
    int32_t networkId __attribute__ ((aligned(4)));
    int32_t systemId __attribute__ ((aligned(4)));
    int32_t baseStationId __attribute__ ((aligned(4)));
    int32_t longitude __attribute__ ((aligned(4)));
    int32_t latitude __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityCdma, networkId) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityCdma, systemId) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityCdma, baseStationId) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityCdma, longitude) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityCdma, latitude) == 16, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CellIdentityCdma) == 20, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CellIdentityCdma) == 4, "wrong alignment");

struct CellIdentityLte final {
    ::android::hardware::hidl_string mcc __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string mnc __attribute__ ((aligned(8)));
    int32_t ci __attribute__ ((aligned(4)));
    int32_t pci __attribute__ ((aligned(4)));
    int32_t tac __attribute__ ((aligned(4)));
    int32_t earfcn __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityLte, mcc) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityLte, mnc) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityLte, ci) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityLte, pci) == 36, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityLte, tac) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityLte, earfcn) == 44, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CellIdentityLte) == 48, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CellIdentityLte) == 8, "wrong alignment");

struct CellIdentityTdscdma final {
    ::android::hardware::hidl_string mcc __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string mnc __attribute__ ((aligned(8)));
    int32_t lac __attribute__ ((aligned(4)));
    int32_t cid __attribute__ ((aligned(4)));
    int32_t cpid __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityTdscdma, mcc) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityTdscdma, mnc) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityTdscdma, lac) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityTdscdma, cid) == 36, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentityTdscdma, cpid) == 40, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CellIdentityTdscdma) == 48, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CellIdentityTdscdma) == 8, "wrong alignment");

struct CellInfoGsm final {
    ::android::hardware::radio::V1_0::CellIdentityGsm cellIdentityGsm __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::GsmSignalStrength signalStrengthGsm __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CellInfoGsm, cellIdentityGsm) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellInfoGsm, signalStrengthGsm) == 48, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CellInfoGsm) == 64, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CellInfoGsm) == 8, "wrong alignment");

struct CellInfoWcdma final {
    ::android::hardware::radio::V1_0::CellIdentityWcdma cellIdentityWcdma __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::WcdmaSignalStrength signalStrengthWcdma __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CellInfoWcdma, cellIdentityWcdma) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellInfoWcdma, signalStrengthWcdma) == 48, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CellInfoWcdma) == 56, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CellInfoWcdma) == 8, "wrong alignment");

struct CellInfoCdma final {
    ::android::hardware::radio::V1_0::CellIdentityCdma cellIdentityCdma __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::CdmaSignalStrength signalStrengthCdma __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::EvdoSignalStrength signalStrengthEvdo __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CellInfoCdma, cellIdentityCdma) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellInfoCdma, signalStrengthCdma) == 20, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellInfoCdma, signalStrengthEvdo) == 28, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CellInfoCdma) == 40, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CellInfoCdma) == 4, "wrong alignment");

struct CellInfoLte final {
    ::android::hardware::radio::V1_0::CellIdentityLte cellIdentityLte __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::LteSignalStrength signalStrengthLte __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CellInfoLte, cellIdentityLte) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellInfoLte, signalStrengthLte) == 48, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CellInfoLte) == 72, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CellInfoLte) == 8, "wrong alignment");

struct CellInfoTdscdma final {
    ::android::hardware::radio::V1_0::CellIdentityTdscdma cellIdentityTdscdma __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::TdScdmaSignalStrength signalStrengthTdscdma __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CellInfoTdscdma, cellIdentityTdscdma) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellInfoTdscdma, signalStrengthTdscdma) == 48, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CellInfoTdscdma) == 56, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CellInfoTdscdma) == 8, "wrong alignment");

struct CellInfo final {
    ::android::hardware::radio::V1_0::CellInfoType cellInfoType __attribute__ ((aligned(4)));
    bool registered __attribute__ ((aligned(1)));
    ::android::hardware::radio::V1_0::TimeStampType timeStampType __attribute__ ((aligned(4)));
    uint64_t timeStamp __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CellInfoGsm> gsm __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CellInfoCdma> cdma __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CellInfoLte> lte __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CellInfoWcdma> wcdma __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CellInfoTdscdma> tdscdma __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CellInfo, cellInfoType) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellInfo, registered) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellInfo, timeStampType) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellInfo, timeStamp) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellInfo, gsm) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellInfo, cdma) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellInfo, lte) == 56, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellInfo, wcdma) == 72, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellInfo, tdscdma) == 88, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CellInfo) == 104, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CellInfo) == 8, "wrong alignment");

struct CellIdentity final {
    ::android::hardware::radio::V1_0::CellInfoType cellInfoType __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CellIdentityGsm> cellIdentityGsm __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CellIdentityWcdma> cellIdentityWcdma __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CellIdentityCdma> cellIdentityCdma __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CellIdentityLte> cellIdentityLte __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CellIdentityTdscdma> cellIdentityTdscdma __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentity, cellInfoType) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentity, cellIdentityGsm) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentity, cellIdentityWcdma) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentity, cellIdentityCdma) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentity, cellIdentityLte) == 56, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CellIdentity, cellIdentityTdscdma) == 72, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CellIdentity) == 88, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CellIdentity) == 8, "wrong alignment");

struct VoiceRegStateResult final {
    ::android::hardware::radio::V1_0::RegState regState __attribute__ ((aligned(4)));
    int32_t rat __attribute__ ((aligned(4)));
    bool cssSupported __attribute__ ((aligned(1)));
    int32_t roamingIndicator __attribute__ ((aligned(4)));
    int32_t systemIsInPrl __attribute__ ((aligned(4)));
    int32_t defaultRoamingIndicator __attribute__ ((aligned(4)));
    int32_t reasonForDenial __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::CellIdentity cellIdentity __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::VoiceRegStateResult, regState) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::VoiceRegStateResult, rat) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::VoiceRegStateResult, cssSupported) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::VoiceRegStateResult, roamingIndicator) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::VoiceRegStateResult, systemIsInPrl) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::VoiceRegStateResult, defaultRoamingIndicator) == 20, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::VoiceRegStateResult, reasonForDenial) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::VoiceRegStateResult, cellIdentity) == 32, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::VoiceRegStateResult) == 120, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::VoiceRegStateResult) == 8, "wrong alignment");

struct DataRegStateResult final {
    ::android::hardware::radio::V1_0::RegState regState __attribute__ ((aligned(4)));
    int32_t rat __attribute__ ((aligned(4)));
    int32_t reasonDataDenied __attribute__ ((aligned(4)));
    int32_t maxDataCalls __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::CellIdentity cellIdentity __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::DataRegStateResult, regState) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataRegStateResult, rat) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataRegStateResult, reasonDataDenied) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataRegStateResult, maxDataCalls) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataRegStateResult, cellIdentity) == 16, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::DataRegStateResult) == 104, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::DataRegStateResult) == 8, "wrong alignment");

struct GsmSmsMessage final {
    ::android::hardware::hidl_string smscPdu __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string pdu __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::GsmSmsMessage, smscPdu) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::GsmSmsMessage, pdu) == 16, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::GsmSmsMessage) == 32, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::GsmSmsMessage) == 8, "wrong alignment");

struct ImsSmsMessage final {
    ::android::hardware::radio::V1_0::RadioTechnologyFamily tech __attribute__ ((aligned(4)));
    bool retry __attribute__ ((aligned(1)));
    int32_t messageRef __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CdmaSmsMessage> cdmaMessage __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::GsmSmsMessage> gsmMessage __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::ImsSmsMessage, tech) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::ImsSmsMessage, retry) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::ImsSmsMessage, messageRef) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::ImsSmsMessage, cdmaMessage) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::ImsSmsMessage, gsmMessage) == 32, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::ImsSmsMessage) == 48, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::ImsSmsMessage) == 8, "wrong alignment");

struct SimApdu final {
    int32_t sessionId __attribute__ ((aligned(4)));
    int32_t cla __attribute__ ((aligned(4)));
    int32_t instruction __attribute__ ((aligned(4)));
    int32_t p1 __attribute__ ((aligned(4)));
    int32_t p2 __attribute__ ((aligned(4)));
    int32_t p3 __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string data __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::SimApdu, sessionId) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SimApdu, cla) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SimApdu, instruction) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SimApdu, p1) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SimApdu, p2) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SimApdu, p3) == 20, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SimApdu, data) == 24, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::SimApdu) == 40, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::SimApdu) == 8, "wrong alignment");

struct NvWriteItem final {
    ::android::hardware::radio::V1_0::NvItem itemId __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string value __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::NvWriteItem, itemId) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::NvWriteItem, value) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::NvWriteItem) == 24, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::NvWriteItem) == 8, "wrong alignment");

struct SelectUiccSub final {
    int32_t slot __attribute__ ((aligned(4)));
    int32_t appIndex __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::SubscriptionType subType __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::UiccSubActStatus actStatus __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::SelectUiccSub, slot) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SelectUiccSub, appIndex) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SelectUiccSub, subType) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SelectUiccSub, actStatus) == 12, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::SelectUiccSub) == 16, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::SelectUiccSub) == 4, "wrong alignment");

struct HardwareConfigModem final {
    int32_t rilModel __attribute__ ((aligned(4)));
    uint32_t rat __attribute__ ((aligned(4)));
    int32_t maxVoice __attribute__ ((aligned(4)));
    int32_t maxData __attribute__ ((aligned(4)));
    int32_t maxStandby __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::HardwareConfigModem, rilModel) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::HardwareConfigModem, rat) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::HardwareConfigModem, maxVoice) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::HardwareConfigModem, maxData) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::HardwareConfigModem, maxStandby) == 16, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::HardwareConfigModem) == 20, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::HardwareConfigModem) == 4, "wrong alignment");

struct HardwareConfigSim final {
    ::android::hardware::hidl_string modemUuid __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::HardwareConfigSim, modemUuid) == 0, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::HardwareConfigSim) == 16, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::HardwareConfigSim) == 8, "wrong alignment");

struct HardwareConfig final {
    ::android::hardware::radio::V1_0::HardwareConfigType type __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string uuid __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::HardwareConfigState state __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::HardwareConfigModem> modem __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::HardwareConfigSim> sim __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::HardwareConfig, type) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::HardwareConfig, uuid) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::HardwareConfig, state) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::HardwareConfig, modem) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::HardwareConfig, sim) == 48, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::HardwareConfig) == 64, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::HardwareConfig) == 8, "wrong alignment");

struct DataProfileInfo final {
    ::android::hardware::radio::V1_0::DataProfileId profileId __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string apn __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string protocol __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string roamingProtocol __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::ApnAuthType authType __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string user __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string password __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::DataProfileInfoType type __attribute__ ((aligned(4)));
    int32_t maxConnsTime __attribute__ ((aligned(4)));
    int32_t maxConns __attribute__ ((aligned(4)));
    int32_t waitTime __attribute__ ((aligned(4)));
    bool enabled __attribute__ ((aligned(1)));
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::ApnTypes> supportedApnTypesBitmap __attribute__ ((aligned(4)));
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::RadioAccessFamily> bearerBitmap __attribute__ ((aligned(4)));
    int32_t mtu __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::MvnoType mvnoType __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string mvnoMatchData __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, profileId) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, apn) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, protocol) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, roamingProtocol) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, authType) == 56, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, user) == 64, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, password) == 80, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, type) == 96, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, maxConnsTime) == 100, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, maxConns) == 104, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, waitTime) == 108, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, enabled) == 112, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, supportedApnTypesBitmap) == 116, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, bearerBitmap) == 120, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, mtu) == 124, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, mvnoType) == 128, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::DataProfileInfo, mvnoMatchData) == 136, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::DataProfileInfo) == 152, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::DataProfileInfo) == 8, "wrong alignment");

struct RadioCapability final {
    int32_t session __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::RadioCapabilityPhase phase __attribute__ ((aligned(4)));
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::RadioAccessFamily> raf __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string logicalModemUuid __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::RadioCapabilityStatus status __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::RadioCapability, session) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::RadioCapability, phase) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::RadioCapability, raf) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::RadioCapability, logicalModemUuid) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::RadioCapability, status) == 32, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::RadioCapability) == 40, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::RadioCapability) == 8, "wrong alignment");

struct LceStatusInfo final {
    ::android::hardware::radio::V1_0::LceStatus lceStatus __attribute__ ((aligned(4)));
    uint8_t actualIntervalMs __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::LceStatusInfo, lceStatus) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::LceStatusInfo, actualIntervalMs) == 4, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::LceStatusInfo) == 8, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::LceStatusInfo) == 4, "wrong alignment");

struct LceDataInfo final {
    uint32_t lastHopCapacityKbps __attribute__ ((aligned(4)));
    uint8_t confidenceLevel __attribute__ ((aligned(1)));
    bool lceSuspended __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::LceDataInfo, lastHopCapacityKbps) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::LceDataInfo, confidenceLevel) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::LceDataInfo, lceSuspended) == 5, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::LceDataInfo) == 8, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::LceDataInfo) == 4, "wrong alignment");

struct ActivityStatsInfo final {
    uint32_t sleepModeTimeMs __attribute__ ((aligned(4)));
    uint32_t idleModeTimeMs __attribute__ ((aligned(4)));
    ::android::hardware::hidl_array<uint32_t, 5 /* RadioConst:NUM_TX_POWER_LEVELS */> txmModetimeMs __attribute__ ((aligned(4)));
    uint32_t rxModeTimeMs __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::ActivityStatsInfo, sleepModeTimeMs) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::ActivityStatsInfo, idleModeTimeMs) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::ActivityStatsInfo, txmModetimeMs) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::ActivityStatsInfo, rxModeTimeMs) == 28, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::ActivityStatsInfo) == 32, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::ActivityStatsInfo) == 4, "wrong alignment");

struct Carrier final {
    ::android::hardware::hidl_string mcc __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string mnc __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::CarrierMatchType matchType __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string matchData __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::Carrier, mcc) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Carrier, mnc) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Carrier, matchType) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::Carrier, matchData) == 40, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::Carrier) == 56, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::Carrier) == 8, "wrong alignment");

struct CarrierRestrictions final {
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::Carrier> allowedCarriers __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::Carrier> excludedCarriers __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CarrierRestrictions, allowedCarriers) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CarrierRestrictions, excludedCarriers) == 16, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CarrierRestrictions) == 32, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CarrierRestrictions) == 8, "wrong alignment");

struct SuppSvcNotification final {
    bool isMT __attribute__ ((aligned(1)));
    int32_t code __attribute__ ((aligned(4)));
    int32_t index __attribute__ ((aligned(4)));
    int32_t type __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string number __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::SuppSvcNotification, isMT) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SuppSvcNotification, code) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SuppSvcNotification, index) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SuppSvcNotification, type) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SuppSvcNotification, number) == 16, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::SuppSvcNotification) == 32, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::SuppSvcNotification) == 8, "wrong alignment");

struct SimRefreshResult final {
    ::android::hardware::radio::V1_0::SimRefreshType type __attribute__ ((aligned(4)));
    int32_t efId __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string aid __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::SimRefreshResult, type) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SimRefreshResult, efId) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::SimRefreshResult, aid) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::SimRefreshResult) == 24, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::SimRefreshResult) == 8, "wrong alignment");

/**
 * CDMA Signal Information Record as defined in C.S0005 section 3.7.5.5  */
struct CdmaSignalInfoRecord final {
    bool isPresent __attribute__ ((aligned(1)));
    int8_t signalType __attribute__ ((aligned(1)));
    int8_t alertPitch __attribute__ ((aligned(1)));
    int8_t signal __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSignalInfoRecord, isPresent) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSignalInfoRecord, signalType) == 1, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSignalInfoRecord, alertPitch) == 2, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaSignalInfoRecord, signal) == 3, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaSignalInfoRecord) == 4, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaSignalInfoRecord) == 1, "wrong alignment");

struct CdmaCallWaiting final {
    ::android::hardware::hidl_string number __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation numberPresentation __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string name __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::CdmaSignalInfoRecord signalInfoRecord __attribute__ ((aligned(1)));
    ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType numberType __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan numberPlan __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaCallWaiting, number) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaCallWaiting, numberPresentation) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaCallWaiting, name) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaCallWaiting, signalInfoRecord) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaCallWaiting, numberType) == 44, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaCallWaiting, numberPlan) == 48, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaCallWaiting) == 56, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaCallWaiting) == 8, "wrong alignment");

/**
 * Display Info Rec as defined in C.S0005 section 3.7.5.1
 * Extended Display Info Rec as defined in C.S0005 section 3.7.5.16
 * Note: the Extended Display info rec contains multiple records of the
 * form: display_tag, display_len, and display_len occurrences of the
 * char field if the display_tag is not 10000000 or 10000001.
 * To save space, the records are stored consecutively in a byte buffer.
 * The display_tag, display_len and chari fields are all 1 byte.
 */
struct CdmaDisplayInfoRecord final {
    ::android::hardware::hidl_string alphaBuf __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaDisplayInfoRecord, alphaBuf) == 0, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaDisplayInfoRecord) == 16, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaDisplayInfoRecord) == 8, "wrong alignment");

/**
 * Called Party Number Info Rec as defined in C.S0005 section 3.7.5.2
 * Calling Party Number Info Rec as defined in C.S0005 section 3.7.5.3
 * Connected Number Info Rec as defined in C.S0005 section 3.7.5.4
 */
struct CdmaNumberInfoRecord final {
    ::android::hardware::hidl_string number __attribute__ ((aligned(8)));
    uint8_t numberType __attribute__ ((aligned(1)));
    uint8_t numberPlan __attribute__ ((aligned(1)));
    uint8_t pi __attribute__ ((aligned(1)));
    uint8_t si __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaNumberInfoRecord, number) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaNumberInfoRecord, numberType) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaNumberInfoRecord, numberPlan) == 17, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaNumberInfoRecord, pi) == 18, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaNumberInfoRecord, si) == 19, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaNumberInfoRecord) == 24, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaNumberInfoRecord) == 8, "wrong alignment");

struct CdmaRedirectingNumberInfoRecord final {
    ::android::hardware::radio::V1_0::CdmaNumberInfoRecord redirectingNumber __attribute__ ((aligned(8)));
    ::android::hardware::radio::V1_0::CdmaRedirectingReason redirectingReason __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaRedirectingNumberInfoRecord, redirectingNumber) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaRedirectingNumberInfoRecord, redirectingReason) == 24, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaRedirectingNumberInfoRecord) == 32, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaRedirectingNumberInfoRecord) == 8, "wrong alignment");

/**
 * Line Control Information Record as defined in C.S0005 section 3.7.5.15  */
struct CdmaLineControlInfoRecord final {
    uint8_t lineCtrlPolarityIncluded __attribute__ ((aligned(1)));
    uint8_t lineCtrlToggle __attribute__ ((aligned(1)));
    uint8_t lineCtrlReverse __attribute__ ((aligned(1)));
    uint8_t lineCtrlPowerDenial __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaLineControlInfoRecord, lineCtrlPolarityIncluded) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaLineControlInfoRecord, lineCtrlToggle) == 1, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaLineControlInfoRecord, lineCtrlReverse) == 2, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaLineControlInfoRecord, lineCtrlPowerDenial) == 3, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaLineControlInfoRecord) == 4, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaLineControlInfoRecord) == 1, "wrong alignment");

/**
 * T53 CLIR Information Record  */
struct CdmaT53ClirInfoRecord final {
    uint8_t cause __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaT53ClirInfoRecord, cause) == 0, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaT53ClirInfoRecord) == 1, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaT53ClirInfoRecord) == 1, "wrong alignment");

/**
 * T53 Audio Control Information Record  */
struct CdmaT53AudioControlInfoRecord final {
    uint8_t upLink __attribute__ ((aligned(1)));
    uint8_t downLink __attribute__ ((aligned(1)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaT53AudioControlInfoRecord, upLink) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaT53AudioControlInfoRecord, downLink) == 1, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaT53AudioControlInfoRecord) == 2, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaT53AudioControlInfoRecord) == 1, "wrong alignment");

struct CdmaInformationRecord final {
    ::android::hardware::radio::V1_0::CdmaInfoRecName name __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CdmaDisplayInfoRecord> display __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CdmaNumberInfoRecord> number __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CdmaSignalInfoRecord> signal __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CdmaRedirectingNumberInfoRecord> redir __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CdmaLineControlInfoRecord> lineCtrl __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CdmaT53ClirInfoRecord> clir __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CdmaT53AudioControlInfoRecord> audioCtrl __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaInformationRecord, name) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaInformationRecord, display) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaInformationRecord, number) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaInformationRecord, signal) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaInformationRecord, redir) == 56, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaInformationRecord, lineCtrl) == 72, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaInformationRecord, clir) == 88, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::CdmaInformationRecord, audioCtrl) == 104, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaInformationRecord) == 120, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaInformationRecord) == 8, "wrong alignment");

struct CdmaInformationRecords final {
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CdmaInformationRecord> infoRec __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CdmaInformationRecords, infoRec) == 0, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CdmaInformationRecords) == 16, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CdmaInformationRecords) == 8, "wrong alignment");

struct CfData final {
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CallForwardInfo> cfInfo __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::CfData, cfInfo) == 0, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::CfData) == 16, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::CfData) == 8, "wrong alignment");

struct SsInfoData final {
    ::android::hardware::hidl_vec<int32_t> ssInfo __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::SsInfoData, ssInfo) == 0, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::SsInfoData) == 16, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::SsInfoData) == 8, "wrong alignment");

struct StkCcUnsolSsResult final {
    ::android::hardware::radio::V1_0::SsServiceType serviceType __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::SsRequestType requestType __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::SsTeleserviceType teleserviceType __attribute__ ((aligned(4)));
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::SuppServiceClass> serviceClass __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::RadioError result __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::SsInfoData> ssInfo __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CfData> cfData __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::StkCcUnsolSsResult, serviceType) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::StkCcUnsolSsResult, requestType) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::StkCcUnsolSsResult, teleserviceType) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::StkCcUnsolSsResult, serviceClass) == 12, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::StkCcUnsolSsResult, result) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::StkCcUnsolSsResult, ssInfo) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::StkCcUnsolSsResult, cfData) == 40, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::StkCcUnsolSsResult) == 56, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::StkCcUnsolSsResult) == 8, "wrong alignment");

struct PcoDataInfo final {
    int32_t cid __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string bearerProto __attribute__ ((aligned(8)));
    int32_t pcoId __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> contents __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_0::PcoDataInfo, cid) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::PcoDataInfo, bearerProto) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::PcoDataInfo, pcoId) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_0::PcoDataInfo, contents) == 32, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_0::PcoDataInfo) == 48, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_0::PcoDataInfo) == 8, "wrong alignment");

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioConst lhs, const ::android::hardware::radio::V1_0::RadioConst rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioConst rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioConst lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioConst lhs, const ::android::hardware::radio::V1_0::RadioConst rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioConst rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioConst lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::RadioConst e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::RadioConst e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::RadioConst>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::RadioConst> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::RadioConst::CDMA_ALPHA_INFO_BUFFER_LENGTH) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioConst::CDMA_ALPHA_INFO_BUFFER_LENGTH)) {
        os += (first ? "" : " | ");
        os += "CDMA_ALPHA_INFO_BUFFER_LENGTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioConst::CDMA_ALPHA_INFO_BUFFER_LENGTH;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioConst::CDMA_NUMBER_INFO_BUFFER_LENGTH) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioConst::CDMA_NUMBER_INFO_BUFFER_LENGTH)) {
        os += (first ? "" : " | ");
        os += "CDMA_NUMBER_INFO_BUFFER_LENGTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioConst::CDMA_NUMBER_INFO_BUFFER_LENGTH;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioConst::MAX_RILDS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioConst::MAX_RILDS)) {
        os += (first ? "" : " | ");
        os += "MAX_RILDS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioConst::MAX_RILDS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioConst::MAX_SOCKET_NAME_LENGTH) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioConst::MAX_SOCKET_NAME_LENGTH)) {
        os += (first ? "" : " | ");
        os += "MAX_SOCKET_NAME_LENGTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioConst::MAX_SOCKET_NAME_LENGTH;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioConst::MAX_CLIENT_ID_LENGTH) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioConst::MAX_CLIENT_ID_LENGTH)) {
        os += (first ? "" : " | ");
        os += "MAX_CLIENT_ID_LENGTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioConst::MAX_CLIENT_ID_LENGTH;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioConst::MAX_DEBUG_SOCKET_NAME_LENGTH) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioConst::MAX_DEBUG_SOCKET_NAME_LENGTH)) {
        os += (first ? "" : " | ");
        os += "MAX_DEBUG_SOCKET_NAME_LENGTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioConst::MAX_DEBUG_SOCKET_NAME_LENGTH;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioConst::MAX_QEMU_PIPE_NAME_LENGTH) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioConst::MAX_QEMU_PIPE_NAME_LENGTH)) {
        os += (first ? "" : " | ");
        os += "MAX_QEMU_PIPE_NAME_LENGTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioConst::MAX_QEMU_PIPE_NAME_LENGTH;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioConst::MAX_UUID_LENGTH) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioConst::MAX_UUID_LENGTH)) {
        os += (first ? "" : " | ");
        os += "MAX_UUID_LENGTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioConst::MAX_UUID_LENGTH;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioConst::CARD_MAX_APPS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioConst::CARD_MAX_APPS)) {
        os += (first ? "" : " | ");
        os += "CARD_MAX_APPS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioConst::CARD_MAX_APPS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioConst::CDMA_MAX_NUMBER_OF_INFO_RECS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioConst::CDMA_MAX_NUMBER_OF_INFO_RECS)) {
        os += (first ? "" : " | ");
        os += "CDMA_MAX_NUMBER_OF_INFO_RECS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioConst::CDMA_MAX_NUMBER_OF_INFO_RECS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioConst::SS_INFO_MAX) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioConst::SS_INFO_MAX)) {
        os += (first ? "" : " | ");
        os += "SS_INFO_MAX";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioConst::SS_INFO_MAX;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioConst::NUM_SERVICE_CLASSES) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioConst::NUM_SERVICE_CLASSES)) {
        os += (first ? "" : " | ");
        os += "NUM_SERVICE_CLASSES";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioConst::NUM_SERVICE_CLASSES;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioConst::NUM_TX_POWER_LEVELS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioConst::NUM_TX_POWER_LEVELS)) {
        os += (first ? "" : " | ");
        os += "NUM_TX_POWER_LEVELS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioConst::NUM_TX_POWER_LEVELS;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::RadioConst o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::RadioConst::CDMA_ALPHA_INFO_BUFFER_LENGTH) {
        return "CDMA_ALPHA_INFO_BUFFER_LENGTH";
    }
    if (o == ::android::hardware::radio::V1_0::RadioConst::CDMA_NUMBER_INFO_BUFFER_LENGTH) {
        return "CDMA_NUMBER_INFO_BUFFER_LENGTH";
    }
    if (o == ::android::hardware::radio::V1_0::RadioConst::MAX_RILDS) {
        return "MAX_RILDS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioConst::MAX_SOCKET_NAME_LENGTH) {
        return "MAX_SOCKET_NAME_LENGTH";
    }
    if (o == ::android::hardware::radio::V1_0::RadioConst::MAX_CLIENT_ID_LENGTH) {
        return "MAX_CLIENT_ID_LENGTH";
    }
    if (o == ::android::hardware::radio::V1_0::RadioConst::MAX_DEBUG_SOCKET_NAME_LENGTH) {
        return "MAX_DEBUG_SOCKET_NAME_LENGTH";
    }
    if (o == ::android::hardware::radio::V1_0::RadioConst::MAX_QEMU_PIPE_NAME_LENGTH) {
        return "MAX_QEMU_PIPE_NAME_LENGTH";
    }
    if (o == ::android::hardware::radio::V1_0::RadioConst::MAX_UUID_LENGTH) {
        return "MAX_UUID_LENGTH";
    }
    if (o == ::android::hardware::radio::V1_0::RadioConst::CARD_MAX_APPS) {
        return "CARD_MAX_APPS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioConst::CDMA_MAX_NUMBER_OF_INFO_RECS) {
        return "CDMA_MAX_NUMBER_OF_INFO_RECS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioConst::SS_INFO_MAX) {
        return "SS_INFO_MAX";
    }
    if (o == ::android::hardware::radio::V1_0::RadioConst::NUM_SERVICE_CLASSES) {
        return "NUM_SERVICE_CLASSES";
    }
    if (o == ::android::hardware::radio::V1_0::RadioConst::NUM_TX_POWER_LEVELS) {
        return "NUM_TX_POWER_LEVELS";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioCdmaSmsConst lhs, const ::android::hardware::radio::V1_0::RadioCdmaSmsConst rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioCdmaSmsConst rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioCdmaSmsConst lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioCdmaSmsConst lhs, const ::android::hardware::radio::V1_0::RadioCdmaSmsConst rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioCdmaSmsConst rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioCdmaSmsConst lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::RadioCdmaSmsConst e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::RadioCdmaSmsConst e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::RadioCdmaSmsConst>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::RadioCdmaSmsConst> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::RadioCdmaSmsConst::ADDRESS_MAX) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCdmaSmsConst::ADDRESS_MAX)) {
        os += (first ? "" : " | ");
        os += "ADDRESS_MAX";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCdmaSmsConst::ADDRESS_MAX;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCdmaSmsConst::SUBADDRESS_MAX) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCdmaSmsConst::SUBADDRESS_MAX)) {
        os += (first ? "" : " | ");
        os += "SUBADDRESS_MAX";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCdmaSmsConst::SUBADDRESS_MAX;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCdmaSmsConst::BEARER_DATA_MAX) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCdmaSmsConst::BEARER_DATA_MAX)) {
        os += (first ? "" : " | ");
        os += "BEARER_DATA_MAX";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCdmaSmsConst::BEARER_DATA_MAX;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_MAX_SND_SIZE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_MAX_SND_SIZE)) {
        os += (first ? "" : " | ");
        os += "UDH_MAX_SND_SIZE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_MAX_SND_SIZE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_EO_DATA_SEGMENT_MAX) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_EO_DATA_SEGMENT_MAX)) {
        os += (first ? "" : " | ");
        os += "UDH_EO_DATA_SEGMENT_MAX";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_EO_DATA_SEGMENT_MAX;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCdmaSmsConst::MAX_UD_HEADERS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCdmaSmsConst::MAX_UD_HEADERS)) {
        os += (first ? "" : " | ");
        os += "MAX_UD_HEADERS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCdmaSmsConst::MAX_UD_HEADERS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCdmaSmsConst::USER_DATA_MAX) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCdmaSmsConst::USER_DATA_MAX)) {
        os += (first ? "" : " | ");
        os += "USER_DATA_MAX";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCdmaSmsConst::USER_DATA_MAX;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_LARGE_PIC_SIZE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_LARGE_PIC_SIZE)) {
        os += (first ? "" : " | ");
        os += "UDH_LARGE_PIC_SIZE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_LARGE_PIC_SIZE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_SMALL_PIC_SIZE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_SMALL_PIC_SIZE)) {
        os += (first ? "" : " | ");
        os += "UDH_SMALL_PIC_SIZE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_SMALL_PIC_SIZE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_VAR_PIC_SIZE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_VAR_PIC_SIZE)) {
        os += (first ? "" : " | ");
        os += "UDH_VAR_PIC_SIZE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_VAR_PIC_SIZE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_ANIM_NUM_BITMAPS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_ANIM_NUM_BITMAPS)) {
        os += (first ? "" : " | ");
        os += "UDH_ANIM_NUM_BITMAPS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_ANIM_NUM_BITMAPS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_LARGE_BITMAP_SIZE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_LARGE_BITMAP_SIZE)) {
        os += (first ? "" : " | ");
        os += "UDH_LARGE_BITMAP_SIZE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_LARGE_BITMAP_SIZE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_SMALL_BITMAP_SIZE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_SMALL_BITMAP_SIZE)) {
        os += (first ? "" : " | ");
        os += "UDH_SMALL_BITMAP_SIZE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_SMALL_BITMAP_SIZE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_OTHER_SIZE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_OTHER_SIZE)) {
        os += (first ? "" : " | ");
        os += "UDH_OTHER_SIZE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_OTHER_SIZE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCdmaSmsConst::IP_ADDRESS_SIZE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCdmaSmsConst::IP_ADDRESS_SIZE)) {
        os += (first ? "" : " | ");
        os += "IP_ADDRESS_SIZE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCdmaSmsConst::IP_ADDRESS_SIZE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::RadioCdmaSmsConst o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::RadioCdmaSmsConst::ADDRESS_MAX) {
        return "ADDRESS_MAX";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCdmaSmsConst::SUBADDRESS_MAX) {
        return "SUBADDRESS_MAX";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCdmaSmsConst::BEARER_DATA_MAX) {
        return "BEARER_DATA_MAX";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_MAX_SND_SIZE) {
        return "UDH_MAX_SND_SIZE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_EO_DATA_SEGMENT_MAX) {
        return "UDH_EO_DATA_SEGMENT_MAX";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCdmaSmsConst::MAX_UD_HEADERS) {
        return "MAX_UD_HEADERS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCdmaSmsConst::USER_DATA_MAX) {
        return "USER_DATA_MAX";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_LARGE_PIC_SIZE) {
        return "UDH_LARGE_PIC_SIZE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_SMALL_PIC_SIZE) {
        return "UDH_SMALL_PIC_SIZE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_VAR_PIC_SIZE) {
        return "UDH_VAR_PIC_SIZE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_ANIM_NUM_BITMAPS) {
        return "UDH_ANIM_NUM_BITMAPS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_LARGE_BITMAP_SIZE) {
        return "UDH_LARGE_BITMAP_SIZE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_SMALL_BITMAP_SIZE) {
        return "UDH_SMALL_BITMAP_SIZE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_OTHER_SIZE) {
        return "UDH_OTHER_SIZE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCdmaSmsConst::IP_ADDRESS_SIZE) {
        return "IP_ADDRESS_SIZE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioError lhs, const ::android::hardware::radio::V1_0::RadioError rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioError rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioError lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioError lhs, const ::android::hardware::radio::V1_0::RadioError rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioError rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioError lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::RadioError e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::RadioError e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::RadioError>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::RadioError> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::RadioError::NONE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::NONE)) {
        os += (first ? "" : " | ");
        os += "NONE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::NONE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::RADIO_NOT_AVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::RADIO_NOT_AVAILABLE)) {
        os += (first ? "" : " | ");
        os += "RADIO_NOT_AVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::RADIO_NOT_AVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::GENERIC_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::GENERIC_FAILURE)) {
        os += (first ? "" : " | ");
        os += "GENERIC_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::GENERIC_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::PASSWORD_INCORRECT) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::PASSWORD_INCORRECT)) {
        os += (first ? "" : " | ");
        os += "PASSWORD_INCORRECT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::PASSWORD_INCORRECT;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::SIM_PIN2) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::SIM_PIN2)) {
        os += (first ? "" : " | ");
        os += "SIM_PIN2";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::SIM_PIN2;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::SIM_PUK2) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::SIM_PUK2)) {
        os += (first ? "" : " | ");
        os += "SIM_PUK2";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::SIM_PUK2;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::REQUEST_NOT_SUPPORTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::REQUEST_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "REQUEST_NOT_SUPPORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::REQUEST_NOT_SUPPORTED;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::CANCELLED) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::CANCELLED)) {
        os += (first ? "" : " | ");
        os += "CANCELLED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::CANCELLED;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OP_NOT_ALLOWED_DURING_VOICE_CALL) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OP_NOT_ALLOWED_DURING_VOICE_CALL)) {
        os += (first ? "" : " | ");
        os += "OP_NOT_ALLOWED_DURING_VOICE_CALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OP_NOT_ALLOWED_DURING_VOICE_CALL;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OP_NOT_ALLOWED_BEFORE_REG_TO_NW) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OP_NOT_ALLOWED_BEFORE_REG_TO_NW)) {
        os += (first ? "" : " | ");
        os += "OP_NOT_ALLOWED_BEFORE_REG_TO_NW";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OP_NOT_ALLOWED_BEFORE_REG_TO_NW;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::SMS_SEND_FAIL_RETRY) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::SMS_SEND_FAIL_RETRY)) {
        os += (first ? "" : " | ");
        os += "SMS_SEND_FAIL_RETRY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::SMS_SEND_FAIL_RETRY;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::SIM_ABSENT) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::SIM_ABSENT)) {
        os += (first ? "" : " | ");
        os += "SIM_ABSENT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::SIM_ABSENT;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::SUBSCRIPTION_NOT_AVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::SUBSCRIPTION_NOT_AVAILABLE)) {
        os += (first ? "" : " | ");
        os += "SUBSCRIPTION_NOT_AVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::SUBSCRIPTION_NOT_AVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::MODE_NOT_SUPPORTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::MODE_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "MODE_NOT_SUPPORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::MODE_NOT_SUPPORTED;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::FDN_CHECK_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::FDN_CHECK_FAILURE)) {
        os += (first ? "" : " | ");
        os += "FDN_CHECK_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::FDN_CHECK_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::ILLEGAL_SIM_OR_ME) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::ILLEGAL_SIM_OR_ME)) {
        os += (first ? "" : " | ");
        os += "ILLEGAL_SIM_OR_ME";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::ILLEGAL_SIM_OR_ME;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::MISSING_RESOURCE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::MISSING_RESOURCE)) {
        os += (first ? "" : " | ");
        os += "MISSING_RESOURCE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::MISSING_RESOURCE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::NO_SUCH_ELEMENT) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::NO_SUCH_ELEMENT)) {
        os += (first ? "" : " | ");
        os += "NO_SUCH_ELEMENT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::NO_SUCH_ELEMENT;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::DIAL_MODIFIED_TO_USSD) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::DIAL_MODIFIED_TO_USSD)) {
        os += (first ? "" : " | ");
        os += "DIAL_MODIFIED_TO_USSD";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::DIAL_MODIFIED_TO_USSD;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::DIAL_MODIFIED_TO_SS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::DIAL_MODIFIED_TO_SS)) {
        os += (first ? "" : " | ");
        os += "DIAL_MODIFIED_TO_SS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::DIAL_MODIFIED_TO_SS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::DIAL_MODIFIED_TO_DIAL) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::DIAL_MODIFIED_TO_DIAL)) {
        os += (first ? "" : " | ");
        os += "DIAL_MODIFIED_TO_DIAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::DIAL_MODIFIED_TO_DIAL;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::USSD_MODIFIED_TO_DIAL) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::USSD_MODIFIED_TO_DIAL)) {
        os += (first ? "" : " | ");
        os += "USSD_MODIFIED_TO_DIAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::USSD_MODIFIED_TO_DIAL;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::USSD_MODIFIED_TO_SS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::USSD_MODIFIED_TO_SS)) {
        os += (first ? "" : " | ");
        os += "USSD_MODIFIED_TO_SS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::USSD_MODIFIED_TO_SS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::USSD_MODIFIED_TO_USSD) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::USSD_MODIFIED_TO_USSD)) {
        os += (first ? "" : " | ");
        os += "USSD_MODIFIED_TO_USSD";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::USSD_MODIFIED_TO_USSD;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::SS_MODIFIED_TO_DIAL) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::SS_MODIFIED_TO_DIAL)) {
        os += (first ? "" : " | ");
        os += "SS_MODIFIED_TO_DIAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::SS_MODIFIED_TO_DIAL;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::SS_MODIFIED_TO_USSD) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::SS_MODIFIED_TO_USSD)) {
        os += (first ? "" : " | ");
        os += "SS_MODIFIED_TO_USSD";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::SS_MODIFIED_TO_USSD;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::SUBSCRIPTION_NOT_SUPPORTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::SUBSCRIPTION_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "SUBSCRIPTION_NOT_SUPPORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::SUBSCRIPTION_NOT_SUPPORTED;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::SS_MODIFIED_TO_SS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::SS_MODIFIED_TO_SS)) {
        os += (first ? "" : " | ");
        os += "SS_MODIFIED_TO_SS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::SS_MODIFIED_TO_SS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::LCE_NOT_SUPPORTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::LCE_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "LCE_NOT_SUPPORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::LCE_NOT_SUPPORTED;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::NO_MEMORY) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::NO_MEMORY)) {
        os += (first ? "" : " | ");
        os += "NO_MEMORY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::NO_MEMORY;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::INTERNAL_ERR) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::INTERNAL_ERR)) {
        os += (first ? "" : " | ");
        os += "INTERNAL_ERR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::INTERNAL_ERR;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::SYSTEM_ERR) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::SYSTEM_ERR)) {
        os += (first ? "" : " | ");
        os += "SYSTEM_ERR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::SYSTEM_ERR;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::MODEM_ERR) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::MODEM_ERR)) {
        os += (first ? "" : " | ");
        os += "MODEM_ERR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::MODEM_ERR;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::INVALID_STATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::INVALID_STATE)) {
        os += (first ? "" : " | ");
        os += "INVALID_STATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::INVALID_STATE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::NO_RESOURCES) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::NO_RESOURCES)) {
        os += (first ? "" : " | ");
        os += "NO_RESOURCES";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::NO_RESOURCES;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::SIM_ERR) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::SIM_ERR)) {
        os += (first ? "" : " | ");
        os += "SIM_ERR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::SIM_ERR;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::INVALID_ARGUMENTS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::INVALID_ARGUMENTS)) {
        os += (first ? "" : " | ");
        os += "INVALID_ARGUMENTS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::INVALID_ARGUMENTS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::INVALID_SIM_STATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::INVALID_SIM_STATE)) {
        os += (first ? "" : " | ");
        os += "INVALID_SIM_STATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::INVALID_SIM_STATE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::INVALID_MODEM_STATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::INVALID_MODEM_STATE)) {
        os += (first ? "" : " | ");
        os += "INVALID_MODEM_STATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::INVALID_MODEM_STATE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::INVALID_CALL_ID) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::INVALID_CALL_ID)) {
        os += (first ? "" : " | ");
        os += "INVALID_CALL_ID";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::INVALID_CALL_ID;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::NO_SMS_TO_ACK) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::NO_SMS_TO_ACK)) {
        os += (first ? "" : " | ");
        os += "NO_SMS_TO_ACK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::NO_SMS_TO_ACK;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::NETWORK_ERR) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::NETWORK_ERR)) {
        os += (first ? "" : " | ");
        os += "NETWORK_ERR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::NETWORK_ERR;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::REQUEST_RATE_LIMITED) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::REQUEST_RATE_LIMITED)) {
        os += (first ? "" : " | ");
        os += "REQUEST_RATE_LIMITED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::REQUEST_RATE_LIMITED;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::SIM_BUSY) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::SIM_BUSY)) {
        os += (first ? "" : " | ");
        os += "SIM_BUSY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::SIM_BUSY;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::SIM_FULL) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::SIM_FULL)) {
        os += (first ? "" : " | ");
        os += "SIM_FULL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::SIM_FULL;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::NETWORK_REJECT) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::NETWORK_REJECT)) {
        os += (first ? "" : " | ");
        os += "NETWORK_REJECT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::NETWORK_REJECT;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OPERATION_NOT_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OPERATION_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "OPERATION_NOT_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OPERATION_NOT_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::EMPTY_RECORD) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::EMPTY_RECORD)) {
        os += (first ? "" : " | ");
        os += "EMPTY_RECORD";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::EMPTY_RECORD;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::INVALID_SMS_FORMAT) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::INVALID_SMS_FORMAT)) {
        os += (first ? "" : " | ");
        os += "INVALID_SMS_FORMAT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::INVALID_SMS_FORMAT;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::ENCODING_ERR) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::ENCODING_ERR)) {
        os += (first ? "" : " | ");
        os += "ENCODING_ERR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::ENCODING_ERR;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::INVALID_SMSC_ADDRESS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::INVALID_SMSC_ADDRESS)) {
        os += (first ? "" : " | ");
        os += "INVALID_SMSC_ADDRESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::INVALID_SMSC_ADDRESS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::NO_SUCH_ENTRY) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::NO_SUCH_ENTRY)) {
        os += (first ? "" : " | ");
        os += "NO_SUCH_ENTRY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::NO_SUCH_ENTRY;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::NETWORK_NOT_READY) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::NETWORK_NOT_READY)) {
        os += (first ? "" : " | ");
        os += "NETWORK_NOT_READY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::NETWORK_NOT_READY;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::NOT_PROVISIONED) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::NOT_PROVISIONED)) {
        os += (first ? "" : " | ");
        os += "NOT_PROVISIONED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::NOT_PROVISIONED;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::NO_SUBSCRIPTION) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::NO_SUBSCRIPTION)) {
        os += (first ? "" : " | ");
        os += "NO_SUBSCRIPTION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::NO_SUBSCRIPTION;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::NO_NETWORK_FOUND) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::NO_NETWORK_FOUND)) {
        os += (first ? "" : " | ");
        os += "NO_NETWORK_FOUND";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::NO_NETWORK_FOUND;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::DEVICE_IN_USE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::DEVICE_IN_USE)) {
        os += (first ? "" : " | ");
        os += "DEVICE_IN_USE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::DEVICE_IN_USE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::ABORTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::ABORTED)) {
        os += (first ? "" : " | ");
        os += "ABORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::ABORTED;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::INVALID_RESPONSE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::INVALID_RESPONSE)) {
        os += (first ? "" : " | ");
        os += "INVALID_RESPONSE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::INVALID_RESPONSE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_1) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_1)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_1";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_1;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_2) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_2)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_2";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_2;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_3) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_3)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_3";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_3;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_4) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_4)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_4";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_4;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_5) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_5)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_5";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_5;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_6) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_6)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_6";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_6;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_7) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_7)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_7";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_7;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_8) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_8)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_8";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_8;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_9) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_9)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_9";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_9;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_10) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_10)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_10";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_10;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_11) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_11)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_11";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_11;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_12) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_12)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_12";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_12;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_13) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_13)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_13";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_13;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_14) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_14)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_14";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_14;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_15) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_15)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_15";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_15;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_16) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_16)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_16";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_16;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_17) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_17)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_17";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_17;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_18) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_18)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_18";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_18;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_19) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_19)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_19";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_19;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_20) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_20)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_20";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_20;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_21) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_21)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_21";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_21;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_22) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_22)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_22";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_22;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_23) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_23)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_23";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_23;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_24) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_24)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_24";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_24;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_25) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioError::OEM_ERROR_25)) {
        os += (first ? "" : " | ");
        os += "OEM_ERROR_25";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_25;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::RadioError o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::RadioError::NONE) {
        return "NONE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::RADIO_NOT_AVAILABLE) {
        return "RADIO_NOT_AVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::GENERIC_FAILURE) {
        return "GENERIC_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::PASSWORD_INCORRECT) {
        return "PASSWORD_INCORRECT";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::SIM_PIN2) {
        return "SIM_PIN2";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::SIM_PUK2) {
        return "SIM_PUK2";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::REQUEST_NOT_SUPPORTED) {
        return "REQUEST_NOT_SUPPORTED";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::CANCELLED) {
        return "CANCELLED";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OP_NOT_ALLOWED_DURING_VOICE_CALL) {
        return "OP_NOT_ALLOWED_DURING_VOICE_CALL";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OP_NOT_ALLOWED_BEFORE_REG_TO_NW) {
        return "OP_NOT_ALLOWED_BEFORE_REG_TO_NW";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::SMS_SEND_FAIL_RETRY) {
        return "SMS_SEND_FAIL_RETRY";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::SIM_ABSENT) {
        return "SIM_ABSENT";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::SUBSCRIPTION_NOT_AVAILABLE) {
        return "SUBSCRIPTION_NOT_AVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::MODE_NOT_SUPPORTED) {
        return "MODE_NOT_SUPPORTED";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::FDN_CHECK_FAILURE) {
        return "FDN_CHECK_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::ILLEGAL_SIM_OR_ME) {
        return "ILLEGAL_SIM_OR_ME";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::MISSING_RESOURCE) {
        return "MISSING_RESOURCE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::NO_SUCH_ELEMENT) {
        return "NO_SUCH_ELEMENT";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::DIAL_MODIFIED_TO_USSD) {
        return "DIAL_MODIFIED_TO_USSD";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::DIAL_MODIFIED_TO_SS) {
        return "DIAL_MODIFIED_TO_SS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::DIAL_MODIFIED_TO_DIAL) {
        return "DIAL_MODIFIED_TO_DIAL";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::USSD_MODIFIED_TO_DIAL) {
        return "USSD_MODIFIED_TO_DIAL";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::USSD_MODIFIED_TO_SS) {
        return "USSD_MODIFIED_TO_SS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::USSD_MODIFIED_TO_USSD) {
        return "USSD_MODIFIED_TO_USSD";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::SS_MODIFIED_TO_DIAL) {
        return "SS_MODIFIED_TO_DIAL";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::SS_MODIFIED_TO_USSD) {
        return "SS_MODIFIED_TO_USSD";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::SUBSCRIPTION_NOT_SUPPORTED) {
        return "SUBSCRIPTION_NOT_SUPPORTED";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::SS_MODIFIED_TO_SS) {
        return "SS_MODIFIED_TO_SS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::LCE_NOT_SUPPORTED) {
        return "LCE_NOT_SUPPORTED";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::NO_MEMORY) {
        return "NO_MEMORY";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::INTERNAL_ERR) {
        return "INTERNAL_ERR";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::SYSTEM_ERR) {
        return "SYSTEM_ERR";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::MODEM_ERR) {
        return "MODEM_ERR";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::INVALID_STATE) {
        return "INVALID_STATE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::NO_RESOURCES) {
        return "NO_RESOURCES";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::SIM_ERR) {
        return "SIM_ERR";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::INVALID_ARGUMENTS) {
        return "INVALID_ARGUMENTS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::INVALID_SIM_STATE) {
        return "INVALID_SIM_STATE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::INVALID_MODEM_STATE) {
        return "INVALID_MODEM_STATE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::INVALID_CALL_ID) {
        return "INVALID_CALL_ID";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::NO_SMS_TO_ACK) {
        return "NO_SMS_TO_ACK";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::NETWORK_ERR) {
        return "NETWORK_ERR";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::REQUEST_RATE_LIMITED) {
        return "REQUEST_RATE_LIMITED";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::SIM_BUSY) {
        return "SIM_BUSY";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::SIM_FULL) {
        return "SIM_FULL";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::NETWORK_REJECT) {
        return "NETWORK_REJECT";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OPERATION_NOT_ALLOWED) {
        return "OPERATION_NOT_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::EMPTY_RECORD) {
        return "EMPTY_RECORD";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::INVALID_SMS_FORMAT) {
        return "INVALID_SMS_FORMAT";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::ENCODING_ERR) {
        return "ENCODING_ERR";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::INVALID_SMSC_ADDRESS) {
        return "INVALID_SMSC_ADDRESS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::NO_SUCH_ENTRY) {
        return "NO_SUCH_ENTRY";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::NETWORK_NOT_READY) {
        return "NETWORK_NOT_READY";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::NOT_PROVISIONED) {
        return "NOT_PROVISIONED";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::NO_SUBSCRIPTION) {
        return "NO_SUBSCRIPTION";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::NO_NETWORK_FOUND) {
        return "NO_NETWORK_FOUND";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::DEVICE_IN_USE) {
        return "DEVICE_IN_USE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::ABORTED) {
        return "ABORTED";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::INVALID_RESPONSE) {
        return "INVALID_RESPONSE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_1) {
        return "OEM_ERROR_1";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_2) {
        return "OEM_ERROR_2";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_3) {
        return "OEM_ERROR_3";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_4) {
        return "OEM_ERROR_4";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_5) {
        return "OEM_ERROR_5";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_6) {
        return "OEM_ERROR_6";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_7) {
        return "OEM_ERROR_7";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_8) {
        return "OEM_ERROR_8";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_9) {
        return "OEM_ERROR_9";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_10) {
        return "OEM_ERROR_10";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_11) {
        return "OEM_ERROR_11";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_12) {
        return "OEM_ERROR_12";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_13) {
        return "OEM_ERROR_13";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_14) {
        return "OEM_ERROR_14";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_15) {
        return "OEM_ERROR_15";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_16) {
        return "OEM_ERROR_16";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_17) {
        return "OEM_ERROR_17";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_18) {
        return "OEM_ERROR_18";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_19) {
        return "OEM_ERROR_19";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_20) {
        return "OEM_ERROR_20";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_21) {
        return "OEM_ERROR_21";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_22) {
        return "OEM_ERROR_22";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_23) {
        return "OEM_ERROR_23";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_24) {
        return "OEM_ERROR_24";
    }
    if (o == ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_25) {
        return "OEM_ERROR_25";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioResponseType lhs, const ::android::hardware::radio::V1_0::RadioResponseType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioResponseType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioResponseType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioResponseType lhs, const ::android::hardware::radio::V1_0::RadioResponseType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioResponseType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioResponseType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::RadioResponseType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::RadioResponseType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::RadioResponseType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::RadioResponseType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioResponseType::SOLICITED)) {
        os += (first ? "" : " | ");
        os += "SOLICITED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED_ACK) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioResponseType::SOLICITED_ACK)) {
        os += (first ? "" : " | ");
        os += "SOLICITED_ACK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED_ACK;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED_ACK_EXP) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioResponseType::SOLICITED_ACK_EXP)) {
        os += (first ? "" : " | ");
        os += "SOLICITED_ACK_EXP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED_ACK_EXP;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::RadioResponseType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED) {
        return "SOLICITED";
    }
    if (o == ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED_ACK) {
        return "SOLICITED_ACK";
    }
    if (o == ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED_ACK_EXP) {
        return "SOLICITED_ACK_EXP";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioIndicationType lhs, const ::android::hardware::radio::V1_0::RadioIndicationType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioIndicationType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioIndicationType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioIndicationType lhs, const ::android::hardware::radio::V1_0::RadioIndicationType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioIndicationType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioIndicationType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::RadioIndicationType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::RadioIndicationType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::RadioIndicationType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::RadioIndicationType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::RadioIndicationType::UNSOLICITED) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioIndicationType::UNSOLICITED)) {
        os += (first ? "" : " | ");
        os += "UNSOLICITED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioIndicationType::UNSOLICITED;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioIndicationType::UNSOLICITED_ACK_EXP) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioIndicationType::UNSOLICITED_ACK_EXP)) {
        os += (first ? "" : " | ");
        os += "UNSOLICITED_ACK_EXP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioIndicationType::UNSOLICITED_ACK_EXP;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::RadioIndicationType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::RadioIndicationType::UNSOLICITED) {
        return "UNSOLICITED";
    }
    if (o == ::android::hardware::radio::V1_0::RadioIndicationType::UNSOLICITED_ACK_EXP) {
        return "UNSOLICITED_ACK_EXP";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RestrictedState lhs, const ::android::hardware::radio::V1_0::RestrictedState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::RestrictedState rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RestrictedState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RestrictedState lhs, const ::android::hardware::radio::V1_0::RestrictedState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::RestrictedState rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RestrictedState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::RestrictedState e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::RestrictedState e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::RestrictedState>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::RestrictedState> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::RestrictedState::NONE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RestrictedState::NONE)) {
        os += (first ? "" : " | ");
        os += "NONE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RestrictedState::NONE;
    }
    if ((o & ::android::hardware::radio::V1_0::RestrictedState::CS_EMERGENCY) == static_cast<int32_t>(::android::hardware::radio::V1_0::RestrictedState::CS_EMERGENCY)) {
        os += (first ? "" : " | ");
        os += "CS_EMERGENCY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RestrictedState::CS_EMERGENCY;
    }
    if ((o & ::android::hardware::radio::V1_0::RestrictedState::CS_NORMAL) == static_cast<int32_t>(::android::hardware::radio::V1_0::RestrictedState::CS_NORMAL)) {
        os += (first ? "" : " | ");
        os += "CS_NORMAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RestrictedState::CS_NORMAL;
    }
    if ((o & ::android::hardware::radio::V1_0::RestrictedState::CS_ALL) == static_cast<int32_t>(::android::hardware::radio::V1_0::RestrictedState::CS_ALL)) {
        os += (first ? "" : " | ");
        os += "CS_ALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RestrictedState::CS_ALL;
    }
    if ((o & ::android::hardware::radio::V1_0::RestrictedState::PS_ALL) == static_cast<int32_t>(::android::hardware::radio::V1_0::RestrictedState::PS_ALL)) {
        os += (first ? "" : " | ");
        os += "PS_ALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RestrictedState::PS_ALL;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::RestrictedState o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::RestrictedState::NONE) {
        return "NONE";
    }
    if (o == ::android::hardware::radio::V1_0::RestrictedState::CS_EMERGENCY) {
        return "CS_EMERGENCY";
    }
    if (o == ::android::hardware::radio::V1_0::RestrictedState::CS_NORMAL) {
        return "CS_NORMAL";
    }
    if (o == ::android::hardware::radio::V1_0::RestrictedState::CS_ALL) {
        return "CS_ALL";
    }
    if (o == ::android::hardware::radio::V1_0::RestrictedState::PS_ALL) {
        return "PS_ALL";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CardState lhs, const ::android::hardware::radio::V1_0::CardState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CardState rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CardState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CardState lhs, const ::android::hardware::radio::V1_0::CardState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CardState rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CardState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CardState e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CardState e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CardState>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CardState> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CardState::ABSENT) == static_cast<int32_t>(::android::hardware::radio::V1_0::CardState::ABSENT)) {
        os += (first ? "" : " | ");
        os += "ABSENT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CardState::ABSENT;
    }
    if ((o & ::android::hardware::radio::V1_0::CardState::PRESENT) == static_cast<int32_t>(::android::hardware::radio::V1_0::CardState::PRESENT)) {
        os += (first ? "" : " | ");
        os += "PRESENT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CardState::PRESENT;
    }
    if ((o & ::android::hardware::radio::V1_0::CardState::ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_0::CardState::ERROR)) {
        os += (first ? "" : " | ");
        os += "ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CardState::ERROR;
    }
    if ((o & ::android::hardware::radio::V1_0::CardState::RESTRICTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CardState::RESTRICTED)) {
        os += (first ? "" : " | ");
        os += "RESTRICTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CardState::RESTRICTED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CardState o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CardState::ABSENT) {
        return "ABSENT";
    }
    if (o == ::android::hardware::radio::V1_0::CardState::PRESENT) {
        return "PRESENT";
    }
    if (o == ::android::hardware::radio::V1_0::CardState::ERROR) {
        return "ERROR";
    }
    if (o == ::android::hardware::radio::V1_0::CardState::RESTRICTED) {
        return "RESTRICTED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::PinState lhs, const ::android::hardware::radio::V1_0::PinState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::PinState rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::PinState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::PinState lhs, const ::android::hardware::radio::V1_0::PinState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::PinState rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::PinState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::PinState e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::PinState e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::PinState>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::PinState> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::PinState::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::PinState::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PinState::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_0::PinState::ENABLED_NOT_VERIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_0::PinState::ENABLED_NOT_VERIFIED)) {
        os += (first ? "" : " | ");
        os += "ENABLED_NOT_VERIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PinState::ENABLED_NOT_VERIFIED;
    }
    if ((o & ::android::hardware::radio::V1_0::PinState::ENABLED_VERIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_0::PinState::ENABLED_VERIFIED)) {
        os += (first ? "" : " | ");
        os += "ENABLED_VERIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PinState::ENABLED_VERIFIED;
    }
    if ((o & ::android::hardware::radio::V1_0::PinState::DISABLED) == static_cast<int32_t>(::android::hardware::radio::V1_0::PinState::DISABLED)) {
        os += (first ? "" : " | ");
        os += "DISABLED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PinState::DISABLED;
    }
    if ((o & ::android::hardware::radio::V1_0::PinState::ENABLED_BLOCKED) == static_cast<int32_t>(::android::hardware::radio::V1_0::PinState::ENABLED_BLOCKED)) {
        os += (first ? "" : " | ");
        os += "ENABLED_BLOCKED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PinState::ENABLED_BLOCKED;
    }
    if ((o & ::android::hardware::radio::V1_0::PinState::ENABLED_PERM_BLOCKED) == static_cast<int32_t>(::android::hardware::radio::V1_0::PinState::ENABLED_PERM_BLOCKED)) {
        os += (first ? "" : " | ");
        os += "ENABLED_PERM_BLOCKED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PinState::ENABLED_PERM_BLOCKED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::PinState o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::PinState::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_0::PinState::ENABLED_NOT_VERIFIED) {
        return "ENABLED_NOT_VERIFIED";
    }
    if (o == ::android::hardware::radio::V1_0::PinState::ENABLED_VERIFIED) {
        return "ENABLED_VERIFIED";
    }
    if (o == ::android::hardware::radio::V1_0::PinState::DISABLED) {
        return "DISABLED";
    }
    if (o == ::android::hardware::radio::V1_0::PinState::ENABLED_BLOCKED) {
        return "ENABLED_BLOCKED";
    }
    if (o == ::android::hardware::radio::V1_0::PinState::ENABLED_PERM_BLOCKED) {
        return "ENABLED_PERM_BLOCKED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::AppType lhs, const ::android::hardware::radio::V1_0::AppType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::AppType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::AppType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::AppType lhs, const ::android::hardware::radio::V1_0::AppType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::AppType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::AppType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::AppType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::AppType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::AppType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::AppType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::AppType::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::AppType::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::AppType::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_0::AppType::SIM) == static_cast<int32_t>(::android::hardware::radio::V1_0::AppType::SIM)) {
        os += (first ? "" : " | ");
        os += "SIM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::AppType::SIM;
    }
    if ((o & ::android::hardware::radio::V1_0::AppType::USIM) == static_cast<int32_t>(::android::hardware::radio::V1_0::AppType::USIM)) {
        os += (first ? "" : " | ");
        os += "USIM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::AppType::USIM;
    }
    if ((o & ::android::hardware::radio::V1_0::AppType::RUIM) == static_cast<int32_t>(::android::hardware::radio::V1_0::AppType::RUIM)) {
        os += (first ? "" : " | ");
        os += "RUIM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::AppType::RUIM;
    }
    if ((o & ::android::hardware::radio::V1_0::AppType::CSIM) == static_cast<int32_t>(::android::hardware::radio::V1_0::AppType::CSIM)) {
        os += (first ? "" : " | ");
        os += "CSIM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::AppType::CSIM;
    }
    if ((o & ::android::hardware::radio::V1_0::AppType::ISIM) == static_cast<int32_t>(::android::hardware::radio::V1_0::AppType::ISIM)) {
        os += (first ? "" : " | ");
        os += "ISIM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::AppType::ISIM;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::AppType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::AppType::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_0::AppType::SIM) {
        return "SIM";
    }
    if (o == ::android::hardware::radio::V1_0::AppType::USIM) {
        return "USIM";
    }
    if (o == ::android::hardware::radio::V1_0::AppType::RUIM) {
        return "RUIM";
    }
    if (o == ::android::hardware::radio::V1_0::AppType::CSIM) {
        return "CSIM";
    }
    if (o == ::android::hardware::radio::V1_0::AppType::ISIM) {
        return "ISIM";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::AppState lhs, const ::android::hardware::radio::V1_0::AppState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::AppState rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::AppState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::AppState lhs, const ::android::hardware::radio::V1_0::AppState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::AppState rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::AppState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::AppState e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::AppState e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::AppState>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::AppState> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::AppState::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::AppState::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::AppState::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_0::AppState::DETECTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::AppState::DETECTED)) {
        os += (first ? "" : " | ");
        os += "DETECTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::AppState::DETECTED;
    }
    if ((o & ::android::hardware::radio::V1_0::AppState::PIN) == static_cast<int32_t>(::android::hardware::radio::V1_0::AppState::PIN)) {
        os += (first ? "" : " | ");
        os += "PIN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::AppState::PIN;
    }
    if ((o & ::android::hardware::radio::V1_0::AppState::PUK) == static_cast<int32_t>(::android::hardware::radio::V1_0::AppState::PUK)) {
        os += (first ? "" : " | ");
        os += "PUK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::AppState::PUK;
    }
    if ((o & ::android::hardware::radio::V1_0::AppState::SUBSCRIPTION_PERSO) == static_cast<int32_t>(::android::hardware::radio::V1_0::AppState::SUBSCRIPTION_PERSO)) {
        os += (first ? "" : " | ");
        os += "SUBSCRIPTION_PERSO";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::AppState::SUBSCRIPTION_PERSO;
    }
    if ((o & ::android::hardware::radio::V1_0::AppState::READY) == static_cast<int32_t>(::android::hardware::radio::V1_0::AppState::READY)) {
        os += (first ? "" : " | ");
        os += "READY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::AppState::READY;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::AppState o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::AppState::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_0::AppState::DETECTED) {
        return "DETECTED";
    }
    if (o == ::android::hardware::radio::V1_0::AppState::PIN) {
        return "PIN";
    }
    if (o == ::android::hardware::radio::V1_0::AppState::PUK) {
        return "PUK";
    }
    if (o == ::android::hardware::radio::V1_0::AppState::SUBSCRIPTION_PERSO) {
        return "SUBSCRIPTION_PERSO";
    }
    if (o == ::android::hardware::radio::V1_0::AppState::READY) {
        return "READY";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::PersoSubstate lhs, const ::android::hardware::radio::V1_0::PersoSubstate rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::PersoSubstate rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::PersoSubstate lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::PersoSubstate lhs, const ::android::hardware::radio::V1_0::PersoSubstate rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::PersoSubstate rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::PersoSubstate lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::PersoSubstate e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::PersoSubstate e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::PersoSubstate>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::PersoSubstate> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::IN_PROGRESS) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::IN_PROGRESS)) {
        os += (first ? "" : " | ");
        os += "IN_PROGRESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::IN_PROGRESS;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::READY) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::READY)) {
        os += (first ? "" : " | ");
        os += "READY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::READY;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK)) {
        os += (first ? "" : " | ");
        os += "SIM_NETWORK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK_SUBSET) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK_SUBSET)) {
        os += (first ? "" : " | ");
        os += "SIM_NETWORK_SUBSET";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK_SUBSET;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::SIM_CORPORATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::SIM_CORPORATE)) {
        os += (first ? "" : " | ");
        os += "SIM_CORPORATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::SIM_CORPORATE;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::SIM_SERVICE_PROVIDER) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::SIM_SERVICE_PROVIDER)) {
        os += (first ? "" : " | ");
        os += "SIM_SERVICE_PROVIDER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::SIM_SERVICE_PROVIDER;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::SIM_SIM) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::SIM_SIM)) {
        os += (first ? "" : " | ");
        os += "SIM_SIM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::SIM_SIM;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK_PUK) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK_PUK)) {
        os += (first ? "" : " | ");
        os += "SIM_NETWORK_PUK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK_PUK;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK_SUBSET_PUK) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK_SUBSET_PUK)) {
        os += (first ? "" : " | ");
        os += "SIM_NETWORK_SUBSET_PUK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK_SUBSET_PUK;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::SIM_CORPORATE_PUK) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::SIM_CORPORATE_PUK)) {
        os += (first ? "" : " | ");
        os += "SIM_CORPORATE_PUK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::SIM_CORPORATE_PUK;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::SIM_SERVICE_PROVIDER_PUK) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::SIM_SERVICE_PROVIDER_PUK)) {
        os += (first ? "" : " | ");
        os += "SIM_SERVICE_PROVIDER_PUK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::SIM_SERVICE_PROVIDER_PUK;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::SIM_SIM_PUK) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::SIM_SIM_PUK)) {
        os += (first ? "" : " | ");
        os += "SIM_SIM_PUK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::SIM_SIM_PUK;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK1) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK1)) {
        os += (first ? "" : " | ");
        os += "RUIM_NETWORK1";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK1;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK2) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK2)) {
        os += (first ? "" : " | ");
        os += "RUIM_NETWORK2";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK2;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::RUIM_HRPD) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::RUIM_HRPD)) {
        os += (first ? "" : " | ");
        os += "RUIM_HRPD";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::RUIM_HRPD;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::RUIM_CORPORATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::RUIM_CORPORATE)) {
        os += (first ? "" : " | ");
        os += "RUIM_CORPORATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::RUIM_CORPORATE;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::RUIM_SERVICE_PROVIDER) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::RUIM_SERVICE_PROVIDER)) {
        os += (first ? "" : " | ");
        os += "RUIM_SERVICE_PROVIDER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::RUIM_SERVICE_PROVIDER;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::RUIM_RUIM) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::RUIM_RUIM)) {
        os += (first ? "" : " | ");
        os += "RUIM_RUIM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::RUIM_RUIM;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK1_PUK) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK1_PUK)) {
        os += (first ? "" : " | ");
        os += "RUIM_NETWORK1_PUK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK1_PUK;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK2_PUK) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK2_PUK)) {
        os += (first ? "" : " | ");
        os += "RUIM_NETWORK2_PUK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK2_PUK;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::RUIM_HRPD_PUK) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::RUIM_HRPD_PUK)) {
        os += (first ? "" : " | ");
        os += "RUIM_HRPD_PUK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::RUIM_HRPD_PUK;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::RUIM_CORPORATE_PUK) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::RUIM_CORPORATE_PUK)) {
        os += (first ? "" : " | ");
        os += "RUIM_CORPORATE_PUK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::RUIM_CORPORATE_PUK;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::RUIM_SERVICE_PROVIDER_PUK) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::RUIM_SERVICE_PROVIDER_PUK)) {
        os += (first ? "" : " | ");
        os += "RUIM_SERVICE_PROVIDER_PUK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::RUIM_SERVICE_PROVIDER_PUK;
    }
    if ((o & ::android::hardware::radio::V1_0::PersoSubstate::RUIM_RUIM_PUK) == static_cast<int32_t>(::android::hardware::radio::V1_0::PersoSubstate::RUIM_RUIM_PUK)) {
        os += (first ? "" : " | ");
        os += "RUIM_RUIM_PUK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PersoSubstate::RUIM_RUIM_PUK;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::PersoSubstate o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::IN_PROGRESS) {
        return "IN_PROGRESS";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::READY) {
        return "READY";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK) {
        return "SIM_NETWORK";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK_SUBSET) {
        return "SIM_NETWORK_SUBSET";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::SIM_CORPORATE) {
        return "SIM_CORPORATE";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::SIM_SERVICE_PROVIDER) {
        return "SIM_SERVICE_PROVIDER";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::SIM_SIM) {
        return "SIM_SIM";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK_PUK) {
        return "SIM_NETWORK_PUK";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK_SUBSET_PUK) {
        return "SIM_NETWORK_SUBSET_PUK";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::SIM_CORPORATE_PUK) {
        return "SIM_CORPORATE_PUK";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::SIM_SERVICE_PROVIDER_PUK) {
        return "SIM_SERVICE_PROVIDER_PUK";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::SIM_SIM_PUK) {
        return "SIM_SIM_PUK";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK1) {
        return "RUIM_NETWORK1";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK2) {
        return "RUIM_NETWORK2";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::RUIM_HRPD) {
        return "RUIM_HRPD";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::RUIM_CORPORATE) {
        return "RUIM_CORPORATE";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::RUIM_SERVICE_PROVIDER) {
        return "RUIM_SERVICE_PROVIDER";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::RUIM_RUIM) {
        return "RUIM_RUIM";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK1_PUK) {
        return "RUIM_NETWORK1_PUK";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK2_PUK) {
        return "RUIM_NETWORK2_PUK";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::RUIM_HRPD_PUK) {
        return "RUIM_HRPD_PUK";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::RUIM_CORPORATE_PUK) {
        return "RUIM_CORPORATE_PUK";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::RUIM_SERVICE_PROVIDER_PUK) {
        return "RUIM_SERVICE_PROVIDER_PUK";
    }
    if (o == ::android::hardware::radio::V1_0::PersoSubstate::RUIM_RUIM_PUK) {
        return "RUIM_RUIM_PUK";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioState lhs, const ::android::hardware::radio::V1_0::RadioState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioState rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioState lhs, const ::android::hardware::radio::V1_0::RadioState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioState rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::RadioState e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::RadioState e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::RadioState>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::RadioState> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::RadioState::OFF) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioState::OFF)) {
        os += (first ? "" : " | ");
        os += "OFF";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioState::OFF;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioState::UNAVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioState::UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "UNAVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioState::UNAVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioState::ON) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioState::ON)) {
        os += (first ? "" : " | ");
        os += "ON";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioState::ON;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::RadioState o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::RadioState::OFF) {
        return "OFF";
    }
    if (o == ::android::hardware::radio::V1_0::RadioState::UNAVAILABLE) {
        return "UNAVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioState::ON) {
        return "ON";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SapConnectRsp lhs, const ::android::hardware::radio::V1_0::SapConnectRsp rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::SapConnectRsp rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SapConnectRsp lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SapConnectRsp lhs, const ::android::hardware::radio::V1_0::SapConnectRsp rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::SapConnectRsp rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SapConnectRsp lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::SapConnectRsp e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::SapConnectRsp e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::SapConnectRsp>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::SapConnectRsp> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::SapConnectRsp::SUCCESS) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapConnectRsp::SUCCESS)) {
        os += (first ? "" : " | ");
        os += "SUCCESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapConnectRsp::SUCCESS;
    }
    if ((o & ::android::hardware::radio::V1_0::SapConnectRsp::CONNECT_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapConnectRsp::CONNECT_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CONNECT_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapConnectRsp::CONNECT_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_0::SapConnectRsp::MSG_SIZE_TOO_LARGE) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapConnectRsp::MSG_SIZE_TOO_LARGE)) {
        os += (first ? "" : " | ");
        os += "MSG_SIZE_TOO_LARGE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapConnectRsp::MSG_SIZE_TOO_LARGE;
    }
    if ((o & ::android::hardware::radio::V1_0::SapConnectRsp::MSG_SIZE_TOO_SMALL) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapConnectRsp::MSG_SIZE_TOO_SMALL)) {
        os += (first ? "" : " | ");
        os += "MSG_SIZE_TOO_SMALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapConnectRsp::MSG_SIZE_TOO_SMALL;
    }
    if ((o & ::android::hardware::radio::V1_0::SapConnectRsp::CONNECT_OK_CALL_ONGOING) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapConnectRsp::CONNECT_OK_CALL_ONGOING)) {
        os += (first ? "" : " | ");
        os += "CONNECT_OK_CALL_ONGOING";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapConnectRsp::CONNECT_OK_CALL_ONGOING;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::SapConnectRsp o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::SapConnectRsp::SUCCESS) {
        return "SUCCESS";
    }
    if (o == ::android::hardware::radio::V1_0::SapConnectRsp::CONNECT_FAILURE) {
        return "CONNECT_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_0::SapConnectRsp::MSG_SIZE_TOO_LARGE) {
        return "MSG_SIZE_TOO_LARGE";
    }
    if (o == ::android::hardware::radio::V1_0::SapConnectRsp::MSG_SIZE_TOO_SMALL) {
        return "MSG_SIZE_TOO_SMALL";
    }
    if (o == ::android::hardware::radio::V1_0::SapConnectRsp::CONNECT_OK_CALL_ONGOING) {
        return "CONNECT_OK_CALL_ONGOING";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SapDisconnectType lhs, const ::android::hardware::radio::V1_0::SapDisconnectType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::SapDisconnectType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SapDisconnectType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SapDisconnectType lhs, const ::android::hardware::radio::V1_0::SapDisconnectType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::SapDisconnectType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SapDisconnectType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::SapDisconnectType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::SapDisconnectType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::SapDisconnectType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::SapDisconnectType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::SapDisconnectType::GRACEFUL) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapDisconnectType::GRACEFUL)) {
        os += (first ? "" : " | ");
        os += "GRACEFUL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapDisconnectType::GRACEFUL;
    }
    if ((o & ::android::hardware::radio::V1_0::SapDisconnectType::IMMEDIATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapDisconnectType::IMMEDIATE)) {
        os += (first ? "" : " | ");
        os += "IMMEDIATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapDisconnectType::IMMEDIATE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::SapDisconnectType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::SapDisconnectType::GRACEFUL) {
        return "GRACEFUL";
    }
    if (o == ::android::hardware::radio::V1_0::SapDisconnectType::IMMEDIATE) {
        return "IMMEDIATE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SapApduType lhs, const ::android::hardware::radio::V1_0::SapApduType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::SapApduType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SapApduType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SapApduType lhs, const ::android::hardware::radio::V1_0::SapApduType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::SapApduType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SapApduType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::SapApduType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::SapApduType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::SapApduType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::SapApduType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::SapApduType::APDU) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapApduType::APDU)) {
        os += (first ? "" : " | ");
        os += "APDU";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapApduType::APDU;
    }
    if ((o & ::android::hardware::radio::V1_0::SapApduType::APDU7816) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapApduType::APDU7816)) {
        os += (first ? "" : " | ");
        os += "APDU7816";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapApduType::APDU7816;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::SapApduType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::SapApduType::APDU) {
        return "APDU";
    }
    if (o == ::android::hardware::radio::V1_0::SapApduType::APDU7816) {
        return "APDU7816";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SapResultCode lhs, const ::android::hardware::radio::V1_0::SapResultCode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::SapResultCode rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SapResultCode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SapResultCode lhs, const ::android::hardware::radio::V1_0::SapResultCode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::SapResultCode rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SapResultCode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::SapResultCode e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::SapResultCode e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::SapResultCode>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::SapResultCode> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::SapResultCode::SUCCESS) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapResultCode::SUCCESS)) {
        os += (first ? "" : " | ");
        os += "SUCCESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapResultCode::SUCCESS;
    }
    if ((o & ::android::hardware::radio::V1_0::SapResultCode::GENERIC_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapResultCode::GENERIC_FAILURE)) {
        os += (first ? "" : " | ");
        os += "GENERIC_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapResultCode::GENERIC_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_0::SapResultCode::CARD_NOT_ACCESSSIBLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapResultCode::CARD_NOT_ACCESSSIBLE)) {
        os += (first ? "" : " | ");
        os += "CARD_NOT_ACCESSSIBLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapResultCode::CARD_NOT_ACCESSSIBLE;
    }
    if ((o & ::android::hardware::radio::V1_0::SapResultCode::CARD_ALREADY_POWERED_OFF) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapResultCode::CARD_ALREADY_POWERED_OFF)) {
        os += (first ? "" : " | ");
        os += "CARD_ALREADY_POWERED_OFF";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapResultCode::CARD_ALREADY_POWERED_OFF;
    }
    if ((o & ::android::hardware::radio::V1_0::SapResultCode::CARD_REMOVED) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapResultCode::CARD_REMOVED)) {
        os += (first ? "" : " | ");
        os += "CARD_REMOVED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapResultCode::CARD_REMOVED;
    }
    if ((o & ::android::hardware::radio::V1_0::SapResultCode::CARD_ALREADY_POWERED_ON) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapResultCode::CARD_ALREADY_POWERED_ON)) {
        os += (first ? "" : " | ");
        os += "CARD_ALREADY_POWERED_ON";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapResultCode::CARD_ALREADY_POWERED_ON;
    }
    if ((o & ::android::hardware::radio::V1_0::SapResultCode::DATA_NOT_AVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapResultCode::DATA_NOT_AVAILABLE)) {
        os += (first ? "" : " | ");
        os += "DATA_NOT_AVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapResultCode::DATA_NOT_AVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_0::SapResultCode::NOT_SUPPORTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapResultCode::NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "NOT_SUPPORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapResultCode::NOT_SUPPORTED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::SapResultCode o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::SapResultCode::SUCCESS) {
        return "SUCCESS";
    }
    if (o == ::android::hardware::radio::V1_0::SapResultCode::GENERIC_FAILURE) {
        return "GENERIC_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_0::SapResultCode::CARD_NOT_ACCESSSIBLE) {
        return "CARD_NOT_ACCESSSIBLE";
    }
    if (o == ::android::hardware::radio::V1_0::SapResultCode::CARD_ALREADY_POWERED_OFF) {
        return "CARD_ALREADY_POWERED_OFF";
    }
    if (o == ::android::hardware::radio::V1_0::SapResultCode::CARD_REMOVED) {
        return "CARD_REMOVED";
    }
    if (o == ::android::hardware::radio::V1_0::SapResultCode::CARD_ALREADY_POWERED_ON) {
        return "CARD_ALREADY_POWERED_ON";
    }
    if (o == ::android::hardware::radio::V1_0::SapResultCode::DATA_NOT_AVAILABLE) {
        return "DATA_NOT_AVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_0::SapResultCode::NOT_SUPPORTED) {
        return "NOT_SUPPORTED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SapStatus lhs, const ::android::hardware::radio::V1_0::SapStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::SapStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SapStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SapStatus lhs, const ::android::hardware::radio::V1_0::SapStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::SapStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SapStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::SapStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::SapStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::SapStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::SapStatus> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::SapStatus::UNKNOWN_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapStatus::UNKNOWN_ERROR)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapStatus::UNKNOWN_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_0::SapStatus::CARD_RESET) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapStatus::CARD_RESET)) {
        os += (first ? "" : " | ");
        os += "CARD_RESET";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapStatus::CARD_RESET;
    }
    if ((o & ::android::hardware::radio::V1_0::SapStatus::CARD_NOT_ACCESSIBLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapStatus::CARD_NOT_ACCESSIBLE)) {
        os += (first ? "" : " | ");
        os += "CARD_NOT_ACCESSIBLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapStatus::CARD_NOT_ACCESSIBLE;
    }
    if ((o & ::android::hardware::radio::V1_0::SapStatus::CARD_REMOVED) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapStatus::CARD_REMOVED)) {
        os += (first ? "" : " | ");
        os += "CARD_REMOVED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapStatus::CARD_REMOVED;
    }
    if ((o & ::android::hardware::radio::V1_0::SapStatus::CARD_INSERTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapStatus::CARD_INSERTED)) {
        os += (first ? "" : " | ");
        os += "CARD_INSERTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapStatus::CARD_INSERTED;
    }
    if ((o & ::android::hardware::radio::V1_0::SapStatus::RECOVERED) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapStatus::RECOVERED)) {
        os += (first ? "" : " | ");
        os += "RECOVERED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapStatus::RECOVERED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::SapStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::SapStatus::UNKNOWN_ERROR) {
        return "UNKNOWN_ERROR";
    }
    if (o == ::android::hardware::radio::V1_0::SapStatus::CARD_RESET) {
        return "CARD_RESET";
    }
    if (o == ::android::hardware::radio::V1_0::SapStatus::CARD_NOT_ACCESSIBLE) {
        return "CARD_NOT_ACCESSIBLE";
    }
    if (o == ::android::hardware::radio::V1_0::SapStatus::CARD_REMOVED) {
        return "CARD_REMOVED";
    }
    if (o == ::android::hardware::radio::V1_0::SapStatus::CARD_INSERTED) {
        return "CARD_INSERTED";
    }
    if (o == ::android::hardware::radio::V1_0::SapStatus::RECOVERED) {
        return "RECOVERED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SapTransferProtocol lhs, const ::android::hardware::radio::V1_0::SapTransferProtocol rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::SapTransferProtocol rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SapTransferProtocol lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SapTransferProtocol lhs, const ::android::hardware::radio::V1_0::SapTransferProtocol rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::SapTransferProtocol rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SapTransferProtocol lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::SapTransferProtocol e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::SapTransferProtocol e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::SapTransferProtocol>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::SapTransferProtocol> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::SapTransferProtocol::T0) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapTransferProtocol::T0)) {
        os += (first ? "" : " | ");
        os += "T0";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapTransferProtocol::T0;
    }
    if ((o & ::android::hardware::radio::V1_0::SapTransferProtocol::T1) == static_cast<int32_t>(::android::hardware::radio::V1_0::SapTransferProtocol::T1)) {
        os += (first ? "" : " | ");
        os += "T1";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SapTransferProtocol::T1;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::SapTransferProtocol o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::SapTransferProtocol::T0) {
        return "T0";
    }
    if (o == ::android::hardware::radio::V1_0::SapTransferProtocol::T1) {
        return "T1";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CallState lhs, const ::android::hardware::radio::V1_0::CallState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CallState rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CallState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CallState lhs, const ::android::hardware::radio::V1_0::CallState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CallState rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CallState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CallState e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CallState e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CallState>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CallState> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CallState::ACTIVE) == static_cast<int32_t>(::android::hardware::radio::V1_0::CallState::ACTIVE)) {
        os += (first ? "" : " | ");
        os += "ACTIVE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CallState::ACTIVE;
    }
    if ((o & ::android::hardware::radio::V1_0::CallState::HOLDING) == static_cast<int32_t>(::android::hardware::radio::V1_0::CallState::HOLDING)) {
        os += (first ? "" : " | ");
        os += "HOLDING";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CallState::HOLDING;
    }
    if ((o & ::android::hardware::radio::V1_0::CallState::DIALING) == static_cast<int32_t>(::android::hardware::radio::V1_0::CallState::DIALING)) {
        os += (first ? "" : " | ");
        os += "DIALING";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CallState::DIALING;
    }
    if ((o & ::android::hardware::radio::V1_0::CallState::ALERTING) == static_cast<int32_t>(::android::hardware::radio::V1_0::CallState::ALERTING)) {
        os += (first ? "" : " | ");
        os += "ALERTING";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CallState::ALERTING;
    }
    if ((o & ::android::hardware::radio::V1_0::CallState::INCOMING) == static_cast<int32_t>(::android::hardware::radio::V1_0::CallState::INCOMING)) {
        os += (first ? "" : " | ");
        os += "INCOMING";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CallState::INCOMING;
    }
    if ((o & ::android::hardware::radio::V1_0::CallState::WAITING) == static_cast<int32_t>(::android::hardware::radio::V1_0::CallState::WAITING)) {
        os += (first ? "" : " | ");
        os += "WAITING";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CallState::WAITING;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CallState o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CallState::ACTIVE) {
        return "ACTIVE";
    }
    if (o == ::android::hardware::radio::V1_0::CallState::HOLDING) {
        return "HOLDING";
    }
    if (o == ::android::hardware::radio::V1_0::CallState::DIALING) {
        return "DIALING";
    }
    if (o == ::android::hardware::radio::V1_0::CallState::ALERTING) {
        return "ALERTING";
    }
    if (o == ::android::hardware::radio::V1_0::CallState::INCOMING) {
        return "INCOMING";
    }
    if (o == ::android::hardware::radio::V1_0::CallState::WAITING) {
        return "WAITING";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::UusType lhs, const ::android::hardware::radio::V1_0::UusType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::UusType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::UusType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::UusType lhs, const ::android::hardware::radio::V1_0::UusType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::UusType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::UusType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::UusType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::UusType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::UusType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::UusType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::UusType::TYPE1_IMPLICIT) == static_cast<int32_t>(::android::hardware::radio::V1_0::UusType::TYPE1_IMPLICIT)) {
        os += (first ? "" : " | ");
        os += "TYPE1_IMPLICIT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UusType::TYPE1_IMPLICIT;
    }
    if ((o & ::android::hardware::radio::V1_0::UusType::TYPE1_REQUIRED) == static_cast<int32_t>(::android::hardware::radio::V1_0::UusType::TYPE1_REQUIRED)) {
        os += (first ? "" : " | ");
        os += "TYPE1_REQUIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UusType::TYPE1_REQUIRED;
    }
    if ((o & ::android::hardware::radio::V1_0::UusType::TYPE1_NOT_REQUIRED) == static_cast<int32_t>(::android::hardware::radio::V1_0::UusType::TYPE1_NOT_REQUIRED)) {
        os += (first ? "" : " | ");
        os += "TYPE1_NOT_REQUIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UusType::TYPE1_NOT_REQUIRED;
    }
    if ((o & ::android::hardware::radio::V1_0::UusType::TYPE2_REQUIRED) == static_cast<int32_t>(::android::hardware::radio::V1_0::UusType::TYPE2_REQUIRED)) {
        os += (first ? "" : " | ");
        os += "TYPE2_REQUIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UusType::TYPE2_REQUIRED;
    }
    if ((o & ::android::hardware::radio::V1_0::UusType::TYPE2_NOT_REQUIRED) == static_cast<int32_t>(::android::hardware::radio::V1_0::UusType::TYPE2_NOT_REQUIRED)) {
        os += (first ? "" : " | ");
        os += "TYPE2_NOT_REQUIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UusType::TYPE2_NOT_REQUIRED;
    }
    if ((o & ::android::hardware::radio::V1_0::UusType::TYPE3_REQUIRED) == static_cast<int32_t>(::android::hardware::radio::V1_0::UusType::TYPE3_REQUIRED)) {
        os += (first ? "" : " | ");
        os += "TYPE3_REQUIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UusType::TYPE3_REQUIRED;
    }
    if ((o & ::android::hardware::radio::V1_0::UusType::TYPE3_NOT_REQUIRED) == static_cast<int32_t>(::android::hardware::radio::V1_0::UusType::TYPE3_NOT_REQUIRED)) {
        os += (first ? "" : " | ");
        os += "TYPE3_NOT_REQUIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UusType::TYPE3_NOT_REQUIRED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::UusType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::UusType::TYPE1_IMPLICIT) {
        return "TYPE1_IMPLICIT";
    }
    if (o == ::android::hardware::radio::V1_0::UusType::TYPE1_REQUIRED) {
        return "TYPE1_REQUIRED";
    }
    if (o == ::android::hardware::radio::V1_0::UusType::TYPE1_NOT_REQUIRED) {
        return "TYPE1_NOT_REQUIRED";
    }
    if (o == ::android::hardware::radio::V1_0::UusType::TYPE2_REQUIRED) {
        return "TYPE2_REQUIRED";
    }
    if (o == ::android::hardware::radio::V1_0::UusType::TYPE2_NOT_REQUIRED) {
        return "TYPE2_NOT_REQUIRED";
    }
    if (o == ::android::hardware::radio::V1_0::UusType::TYPE3_REQUIRED) {
        return "TYPE3_REQUIRED";
    }
    if (o == ::android::hardware::radio::V1_0::UusType::TYPE3_NOT_REQUIRED) {
        return "TYPE3_NOT_REQUIRED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::UusDcs lhs, const ::android::hardware::radio::V1_0::UusDcs rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::UusDcs rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::UusDcs lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::UusDcs lhs, const ::android::hardware::radio::V1_0::UusDcs rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::UusDcs rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::UusDcs lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::UusDcs e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::UusDcs e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::UusDcs>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::UusDcs> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::UusDcs::USP) == static_cast<int32_t>(::android::hardware::radio::V1_0::UusDcs::USP)) {
        os += (first ? "" : " | ");
        os += "USP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UusDcs::USP;
    }
    if ((o & ::android::hardware::radio::V1_0::UusDcs::OSIHLP) == static_cast<int32_t>(::android::hardware::radio::V1_0::UusDcs::OSIHLP)) {
        os += (first ? "" : " | ");
        os += "OSIHLP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UusDcs::OSIHLP;
    }
    if ((o & ::android::hardware::radio::V1_0::UusDcs::X244) == static_cast<int32_t>(::android::hardware::radio::V1_0::UusDcs::X244)) {
        os += (first ? "" : " | ");
        os += "X244";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UusDcs::X244;
    }
    if ((o & ::android::hardware::radio::V1_0::UusDcs::RMCF) == static_cast<int32_t>(::android::hardware::radio::V1_0::UusDcs::RMCF)) {
        os += (first ? "" : " | ");
        os += "RMCF";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UusDcs::RMCF;
    }
    if ((o & ::android::hardware::radio::V1_0::UusDcs::IA5C) == static_cast<int32_t>(::android::hardware::radio::V1_0::UusDcs::IA5C)) {
        os += (first ? "" : " | ");
        os += "IA5C";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UusDcs::IA5C;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::UusDcs o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::UusDcs::USP) {
        return "USP";
    }
    if (o == ::android::hardware::radio::V1_0::UusDcs::OSIHLP) {
        return "OSIHLP";
    }
    if (o == ::android::hardware::radio::V1_0::UusDcs::X244) {
        return "X244";
    }
    if (o == ::android::hardware::radio::V1_0::UusDcs::RMCF) {
        return "RMCF";
    }
    if (o == ::android::hardware::radio::V1_0::UusDcs::IA5C) {
        return "IA5C";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CallPresentation lhs, const ::android::hardware::radio::V1_0::CallPresentation rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CallPresentation rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CallPresentation lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CallPresentation lhs, const ::android::hardware::radio::V1_0::CallPresentation rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CallPresentation rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CallPresentation lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CallPresentation e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CallPresentation e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CallPresentation>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CallPresentation> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CallPresentation::ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CallPresentation::ALLOWED)) {
        os += (first ? "" : " | ");
        os += "ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CallPresentation::ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_0::CallPresentation::RESTRICTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CallPresentation::RESTRICTED)) {
        os += (first ? "" : " | ");
        os += "RESTRICTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CallPresentation::RESTRICTED;
    }
    if ((o & ::android::hardware::radio::V1_0::CallPresentation::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::CallPresentation::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CallPresentation::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_0::CallPresentation::PAYPHONE) == static_cast<int32_t>(::android::hardware::radio::V1_0::CallPresentation::PAYPHONE)) {
        os += (first ? "" : " | ");
        os += "PAYPHONE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CallPresentation::PAYPHONE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CallPresentation o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CallPresentation::ALLOWED) {
        return "ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_0::CallPresentation::RESTRICTED) {
        return "RESTRICTED";
    }
    if (o == ::android::hardware::radio::V1_0::CallPresentation::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_0::CallPresentation::PAYPHONE) {
        return "PAYPHONE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::Clir lhs, const ::android::hardware::radio::V1_0::Clir rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::Clir rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::Clir lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::Clir lhs, const ::android::hardware::radio::V1_0::Clir rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::Clir rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::Clir lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::Clir e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::Clir e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::Clir>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::Clir> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::Clir::DEFAULT) == static_cast<int32_t>(::android::hardware::radio::V1_0::Clir::DEFAULT)) {
        os += (first ? "" : " | ");
        os += "DEFAULT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::Clir::DEFAULT;
    }
    if ((o & ::android::hardware::radio::V1_0::Clir::INVOCATION) == static_cast<int32_t>(::android::hardware::radio::V1_0::Clir::INVOCATION)) {
        os += (first ? "" : " | ");
        os += "INVOCATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::Clir::INVOCATION;
    }
    if ((o & ::android::hardware::radio::V1_0::Clir::SUPPRESSION) == static_cast<int32_t>(::android::hardware::radio::V1_0::Clir::SUPPRESSION)) {
        os += (first ? "" : " | ");
        os += "SUPPRESSION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::Clir::SUPPRESSION;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::Clir o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::Clir::DEFAULT) {
        return "DEFAULT";
    }
    if (o == ::android::hardware::radio::V1_0::Clir::INVOCATION) {
        return "INVOCATION";
    }
    if (o == ::android::hardware::radio::V1_0::Clir::SUPPRESSION) {
        return "SUPPRESSION";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::LastCallFailCause lhs, const ::android::hardware::radio::V1_0::LastCallFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::LastCallFailCause rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::LastCallFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::LastCallFailCause lhs, const ::android::hardware::radio::V1_0::LastCallFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::LastCallFailCause rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::LastCallFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::LastCallFailCause e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::LastCallFailCause e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::LastCallFailCause>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::LastCallFailCause> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::UNOBTAINABLE_NUMBER) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::UNOBTAINABLE_NUMBER)) {
        os += (first ? "" : " | ");
        os += "UNOBTAINABLE_NUMBER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::UNOBTAINABLE_NUMBER;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::NO_ROUTE_TO_DESTINATION) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::NO_ROUTE_TO_DESTINATION)) {
        os += (first ? "" : " | ");
        os += "NO_ROUTE_TO_DESTINATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::NO_ROUTE_TO_DESTINATION;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::CHANNEL_UNACCEPTABLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::CHANNEL_UNACCEPTABLE)) {
        os += (first ? "" : " | ");
        os += "CHANNEL_UNACCEPTABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::CHANNEL_UNACCEPTABLE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OPERATOR_DETERMINED_BARRING) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OPERATOR_DETERMINED_BARRING)) {
        os += (first ? "" : " | ");
        os += "OPERATOR_DETERMINED_BARRING";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OPERATOR_DETERMINED_BARRING;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::NORMAL) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::NORMAL)) {
        os += (first ? "" : " | ");
        os += "NORMAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::NORMAL;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::BUSY) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::BUSY)) {
        os += (first ? "" : " | ");
        os += "BUSY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::BUSY;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::NO_USER_RESPONDING) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::NO_USER_RESPONDING)) {
        os += (first ? "" : " | ");
        os += "NO_USER_RESPONDING";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::NO_USER_RESPONDING;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::NO_ANSWER_FROM_USER) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::NO_ANSWER_FROM_USER)) {
        os += (first ? "" : " | ");
        os += "NO_ANSWER_FROM_USER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::NO_ANSWER_FROM_USER;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::CALL_REJECTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::CALL_REJECTED)) {
        os += (first ? "" : " | ");
        os += "CALL_REJECTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::CALL_REJECTED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::NUMBER_CHANGED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::NUMBER_CHANGED)) {
        os += (first ? "" : " | ");
        os += "NUMBER_CHANGED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::NUMBER_CHANGED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::PREEMPTION) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::PREEMPTION)) {
        os += (first ? "" : " | ");
        os += "PREEMPTION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::PREEMPTION;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::DESTINATION_OUT_OF_ORDER) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::DESTINATION_OUT_OF_ORDER)) {
        os += (first ? "" : " | ");
        os += "DESTINATION_OUT_OF_ORDER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::DESTINATION_OUT_OF_ORDER;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::INVALID_NUMBER_FORMAT) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::INVALID_NUMBER_FORMAT)) {
        os += (first ? "" : " | ");
        os += "INVALID_NUMBER_FORMAT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::INVALID_NUMBER_FORMAT;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::FACILITY_REJECTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::FACILITY_REJECTED)) {
        os += (first ? "" : " | ");
        os += "FACILITY_REJECTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::FACILITY_REJECTED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::RESP_TO_STATUS_ENQUIRY) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::RESP_TO_STATUS_ENQUIRY)) {
        os += (first ? "" : " | ");
        os += "RESP_TO_STATUS_ENQUIRY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::RESP_TO_STATUS_ENQUIRY;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::NORMAL_UNSPECIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::NORMAL_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "NORMAL_UNSPECIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::NORMAL_UNSPECIFIED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::CONGESTION) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::CONGESTION)) {
        os += (first ? "" : " | ");
        os += "CONGESTION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::CONGESTION;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_OUT_OF_ORDER) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_OUT_OF_ORDER)) {
        os += (first ? "" : " | ");
        os += "NETWORK_OUT_OF_ORDER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_OUT_OF_ORDER;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::TEMPORARY_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::TEMPORARY_FAILURE)) {
        os += (first ? "" : " | ");
        os += "TEMPORARY_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::TEMPORARY_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::SWITCHING_EQUIPMENT_CONGESTION) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::SWITCHING_EQUIPMENT_CONGESTION)) {
        os += (first ? "" : " | ");
        os += "SWITCHING_EQUIPMENT_CONGESTION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::SWITCHING_EQUIPMENT_CONGESTION;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::ACCESS_INFORMATION_DISCARDED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::ACCESS_INFORMATION_DISCARDED)) {
        os += (first ? "" : " | ");
        os += "ACCESS_INFORMATION_DISCARDED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::ACCESS_INFORMATION_DISCARDED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE)) {
        os += (first ? "" : " | ");
        os += "REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::RESOURCES_UNAVAILABLE_OR_UNSPECIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::RESOURCES_UNAVAILABLE_OR_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "RESOURCES_UNAVAILABLE_OR_UNSPECIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::RESOURCES_UNAVAILABLE_OR_UNSPECIFIED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::QOS_UNAVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::QOS_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "QOS_UNAVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::QOS_UNAVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::REQUESTED_FACILITY_NOT_SUBSCRIBED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::REQUESTED_FACILITY_NOT_SUBSCRIBED)) {
        os += (first ? "" : " | ");
        os += "REQUESTED_FACILITY_NOT_SUBSCRIBED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::REQUESTED_FACILITY_NOT_SUBSCRIBED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::INCOMING_CALLS_BARRED_WITHIN_CUG) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::INCOMING_CALLS_BARRED_WITHIN_CUG)) {
        os += (first ? "" : " | ");
        os += "INCOMING_CALLS_BARRED_WITHIN_CUG";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::INCOMING_CALLS_BARRED_WITHIN_CUG;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::BEARER_CAPABILITY_NOT_AUTHORIZED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::BEARER_CAPABILITY_NOT_AUTHORIZED)) {
        os += (first ? "" : " | ");
        os += "BEARER_CAPABILITY_NOT_AUTHORIZED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::BEARER_CAPABILITY_NOT_AUTHORIZED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::BEARER_CAPABILITY_UNAVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::BEARER_CAPABILITY_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "BEARER_CAPABILITY_UNAVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::BEARER_CAPABILITY_UNAVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::SERVICE_OPTION_NOT_AVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::SERVICE_OPTION_NOT_AVAILABLE)) {
        os += (first ? "" : " | ");
        os += "SERVICE_OPTION_NOT_AVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::SERVICE_OPTION_NOT_AVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::BEARER_SERVICE_NOT_IMPLEMENTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::BEARER_SERVICE_NOT_IMPLEMENTED)) {
        os += (first ? "" : " | ");
        os += "BEARER_SERVICE_NOT_IMPLEMENTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::BEARER_SERVICE_NOT_IMPLEMENTED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::ACM_LIMIT_EXCEEDED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::ACM_LIMIT_EXCEEDED)) {
        os += (first ? "" : " | ");
        os += "ACM_LIMIT_EXCEEDED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::ACM_LIMIT_EXCEEDED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::REQUESTED_FACILITY_NOT_IMPLEMENTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::REQUESTED_FACILITY_NOT_IMPLEMENTED)) {
        os += (first ? "" : " | ");
        os += "REQUESTED_FACILITY_NOT_IMPLEMENTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::REQUESTED_FACILITY_NOT_IMPLEMENTED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::ONLY_DIGITAL_INFORMATION_BEARER_AVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::ONLY_DIGITAL_INFORMATION_BEARER_AVAILABLE)) {
        os += (first ? "" : " | ");
        os += "ONLY_DIGITAL_INFORMATION_BEARER_AVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::ONLY_DIGITAL_INFORMATION_BEARER_AVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::SERVICE_OR_OPTION_NOT_IMPLEMENTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::SERVICE_OR_OPTION_NOT_IMPLEMENTED)) {
        os += (first ? "" : " | ");
        os += "SERVICE_OR_OPTION_NOT_IMPLEMENTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::SERVICE_OR_OPTION_NOT_IMPLEMENTED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::INVALID_TRANSACTION_IDENTIFIER) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::INVALID_TRANSACTION_IDENTIFIER)) {
        os += (first ? "" : " | ");
        os += "INVALID_TRANSACTION_IDENTIFIER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::INVALID_TRANSACTION_IDENTIFIER;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::USER_NOT_MEMBER_OF_CUG) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::USER_NOT_MEMBER_OF_CUG)) {
        os += (first ? "" : " | ");
        os += "USER_NOT_MEMBER_OF_CUG";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::USER_NOT_MEMBER_OF_CUG;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::INCOMPATIBLE_DESTINATION) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::INCOMPATIBLE_DESTINATION)) {
        os += (first ? "" : " | ");
        os += "INCOMPATIBLE_DESTINATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::INCOMPATIBLE_DESTINATION;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::INVALID_TRANSIT_NW_SELECTION) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::INVALID_TRANSIT_NW_SELECTION)) {
        os += (first ? "" : " | ");
        os += "INVALID_TRANSIT_NW_SELECTION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::INVALID_TRANSIT_NW_SELECTION;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::SEMANTICALLY_INCORRECT_MESSAGE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::SEMANTICALLY_INCORRECT_MESSAGE)) {
        os += (first ? "" : " | ");
        os += "SEMANTICALLY_INCORRECT_MESSAGE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::SEMANTICALLY_INCORRECT_MESSAGE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::INVALID_MANDATORY_INFORMATION) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::INVALID_MANDATORY_INFORMATION)) {
        os += (first ? "" : " | ");
        os += "INVALID_MANDATORY_INFORMATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::INVALID_MANDATORY_INFORMATION;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::MESSAGE_TYPE_NON_IMPLEMENTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::MESSAGE_TYPE_NON_IMPLEMENTED)) {
        os += (first ? "" : " | ");
        os += "MESSAGE_TYPE_NON_IMPLEMENTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::MESSAGE_TYPE_NON_IMPLEMENTED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE)) {
        os += (first ? "" : " | ");
        os += "MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::INFORMATION_ELEMENT_NON_EXISTENT) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::INFORMATION_ELEMENT_NON_EXISTENT)) {
        os += (first ? "" : " | ");
        os += "INFORMATION_ELEMENT_NON_EXISTENT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::INFORMATION_ELEMENT_NON_EXISTENT;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::CONDITIONAL_IE_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::CONDITIONAL_IE_ERROR)) {
        os += (first ? "" : " | ");
        os += "CONDITIONAL_IE_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::CONDITIONAL_IE_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE)) {
        os += (first ? "" : " | ");
        os += "MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::RECOVERY_ON_TIMER_EXPIRED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::RECOVERY_ON_TIMER_EXPIRED)) {
        os += (first ? "" : " | ");
        os += "RECOVERY_ON_TIMER_EXPIRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::RECOVERY_ON_TIMER_EXPIRED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::PROTOCOL_ERROR_UNSPECIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::PROTOCOL_ERROR_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "PROTOCOL_ERROR_UNSPECIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::PROTOCOL_ERROR_UNSPECIFIED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::INTERWORKING_UNSPECIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::INTERWORKING_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "INTERWORKING_UNSPECIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::INTERWORKING_UNSPECIFIED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::CALL_BARRED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::CALL_BARRED)) {
        os += (first ? "" : " | ");
        os += "CALL_BARRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::CALL_BARRED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::FDN_BLOCKED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::FDN_BLOCKED)) {
        os += (first ? "" : " | ");
        os += "FDN_BLOCKED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::FDN_BLOCKED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::IMSI_UNKNOWN_IN_VLR) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::IMSI_UNKNOWN_IN_VLR)) {
        os += (first ? "" : " | ");
        os += "IMSI_UNKNOWN_IN_VLR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::IMSI_UNKNOWN_IN_VLR;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::IMEI_NOT_ACCEPTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::IMEI_NOT_ACCEPTED)) {
        os += (first ? "" : " | ");
        os += "IMEI_NOT_ACCEPTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::IMEI_NOT_ACCEPTED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::DIAL_MODIFIED_TO_USSD) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::DIAL_MODIFIED_TO_USSD)) {
        os += (first ? "" : " | ");
        os += "DIAL_MODIFIED_TO_USSD";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::DIAL_MODIFIED_TO_USSD;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::DIAL_MODIFIED_TO_SS) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::DIAL_MODIFIED_TO_SS)) {
        os += (first ? "" : " | ");
        os += "DIAL_MODIFIED_TO_SS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::DIAL_MODIFIED_TO_SS;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::DIAL_MODIFIED_TO_DIAL) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::DIAL_MODIFIED_TO_DIAL)) {
        os += (first ? "" : " | ");
        os += "DIAL_MODIFIED_TO_DIAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::DIAL_MODIFIED_TO_DIAL;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_OFF) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::RADIO_OFF)) {
        os += (first ? "" : " | ");
        os += "RADIO_OFF";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_OFF;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OUT_OF_SERVICE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OUT_OF_SERVICE)) {
        os += (first ? "" : " | ");
        os += "OUT_OF_SERVICE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OUT_OF_SERVICE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::NO_VALID_SIM) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::NO_VALID_SIM)) {
        os += (first ? "" : " | ");
        os += "NO_VALID_SIM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::NO_VALID_SIM;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_INTERNAL_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::RADIO_INTERNAL_ERROR)) {
        os += (first ? "" : " | ");
        os += "RADIO_INTERNAL_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_INTERNAL_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_RESP_TIMEOUT) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_RESP_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "NETWORK_RESP_TIMEOUT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_RESP_TIMEOUT;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_REJECT) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_REJECT)) {
        os += (first ? "" : " | ");
        os += "NETWORK_REJECT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_REJECT;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_ACCESS_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::RADIO_ACCESS_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RADIO_ACCESS_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_ACCESS_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_LINK_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::RADIO_LINK_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RADIO_LINK_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_LINK_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_LINK_LOST) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::RADIO_LINK_LOST)) {
        os += (first ? "" : " | ");
        os += "RADIO_LINK_LOST";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_LINK_LOST;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_UPLINK_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::RADIO_UPLINK_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RADIO_UPLINK_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_UPLINK_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_SETUP_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::RADIO_SETUP_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RADIO_SETUP_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_SETUP_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_RELEASE_NORMAL) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::RADIO_RELEASE_NORMAL)) {
        os += (first ? "" : " | ");
        os += "RADIO_RELEASE_NORMAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_RELEASE_NORMAL;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_RELEASE_ABNORMAL) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::RADIO_RELEASE_ABNORMAL)) {
        os += (first ? "" : " | ");
        os += "RADIO_RELEASE_ABNORMAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_RELEASE_ABNORMAL;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::ACCESS_CLASS_BLOCKED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::ACCESS_CLASS_BLOCKED)) {
        os += (first ? "" : " | ");
        os += "ACCESS_CLASS_BLOCKED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::ACCESS_CLASS_BLOCKED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_DETACH) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_DETACH)) {
        os += (first ? "" : " | ");
        os += "NETWORK_DETACH";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_DETACH;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_LOCKED_UNTIL_POWER_CYCLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::CDMA_LOCKED_UNTIL_POWER_CYCLE)) {
        os += (first ? "" : " | ");
        os += "CDMA_LOCKED_UNTIL_POWER_CYCLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_LOCKED_UNTIL_POWER_CYCLE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_DROP) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::CDMA_DROP)) {
        os += (first ? "" : " | ");
        os += "CDMA_DROP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_DROP;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_INTERCEPT) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::CDMA_INTERCEPT)) {
        os += (first ? "" : " | ");
        os += "CDMA_INTERCEPT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_INTERCEPT;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_REORDER) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::CDMA_REORDER)) {
        os += (first ? "" : " | ");
        os += "CDMA_REORDER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_REORDER;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_SO_REJECT) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::CDMA_SO_REJECT)) {
        os += (first ? "" : " | ");
        os += "CDMA_SO_REJECT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_SO_REJECT;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_RETRY_ORDER) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::CDMA_RETRY_ORDER)) {
        os += (first ? "" : " | ");
        os += "CDMA_RETRY_ORDER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_RETRY_ORDER;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_ACCESS_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::CDMA_ACCESS_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CDMA_ACCESS_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_ACCESS_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_PREEMPTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::CDMA_PREEMPTED)) {
        os += (first ? "" : " | ");
        os += "CDMA_PREEMPTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_PREEMPTED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_NOT_EMERGENCY) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::CDMA_NOT_EMERGENCY)) {
        os += (first ? "" : " | ");
        os += "CDMA_NOT_EMERGENCY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_NOT_EMERGENCY;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_ACCESS_BLOCKED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::CDMA_ACCESS_BLOCKED)) {
        os += (first ? "" : " | ");
        os += "CDMA_ACCESS_BLOCKED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_ACCESS_BLOCKED;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_1) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_1)) {
        os += (first ? "" : " | ");
        os += "OEM_CAUSE_1";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_1;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_2) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_2)) {
        os += (first ? "" : " | ");
        os += "OEM_CAUSE_2";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_2;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_3) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_3)) {
        os += (first ? "" : " | ");
        os += "OEM_CAUSE_3";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_3;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_4) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_4)) {
        os += (first ? "" : " | ");
        os += "OEM_CAUSE_4";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_4;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_5) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_5)) {
        os += (first ? "" : " | ");
        os += "OEM_CAUSE_5";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_5;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_6) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_6)) {
        os += (first ? "" : " | ");
        os += "OEM_CAUSE_6";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_6;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_7) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_7)) {
        os += (first ? "" : " | ");
        os += "OEM_CAUSE_7";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_7;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_8) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_8)) {
        os += (first ? "" : " | ");
        os += "OEM_CAUSE_8";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_8;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_9) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_9)) {
        os += (first ? "" : " | ");
        os += "OEM_CAUSE_9";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_9;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_10) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_10)) {
        os += (first ? "" : " | ");
        os += "OEM_CAUSE_10";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_10;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_11) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_11)) {
        os += (first ? "" : " | ");
        os += "OEM_CAUSE_11";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_11;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_12) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_12)) {
        os += (first ? "" : " | ");
        os += "OEM_CAUSE_12";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_12;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_13) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_13)) {
        os += (first ? "" : " | ");
        os += "OEM_CAUSE_13";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_13;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_14) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_14)) {
        os += (first ? "" : " | ");
        os += "OEM_CAUSE_14";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_14;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_15) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_15)) {
        os += (first ? "" : " | ");
        os += "OEM_CAUSE_15";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_15;
    }
    if ((o & ::android::hardware::radio::V1_0::LastCallFailCause::ERROR_UNSPECIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LastCallFailCause::ERROR_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "ERROR_UNSPECIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LastCallFailCause::ERROR_UNSPECIFIED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::LastCallFailCause o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::UNOBTAINABLE_NUMBER) {
        return "UNOBTAINABLE_NUMBER";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::NO_ROUTE_TO_DESTINATION) {
        return "NO_ROUTE_TO_DESTINATION";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::CHANNEL_UNACCEPTABLE) {
        return "CHANNEL_UNACCEPTABLE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OPERATOR_DETERMINED_BARRING) {
        return "OPERATOR_DETERMINED_BARRING";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::NORMAL) {
        return "NORMAL";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::BUSY) {
        return "BUSY";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::NO_USER_RESPONDING) {
        return "NO_USER_RESPONDING";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::NO_ANSWER_FROM_USER) {
        return "NO_ANSWER_FROM_USER";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::CALL_REJECTED) {
        return "CALL_REJECTED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::NUMBER_CHANGED) {
        return "NUMBER_CHANGED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::PREEMPTION) {
        return "PREEMPTION";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::DESTINATION_OUT_OF_ORDER) {
        return "DESTINATION_OUT_OF_ORDER";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::INVALID_NUMBER_FORMAT) {
        return "INVALID_NUMBER_FORMAT";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::FACILITY_REJECTED) {
        return "FACILITY_REJECTED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::RESP_TO_STATUS_ENQUIRY) {
        return "RESP_TO_STATUS_ENQUIRY";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::NORMAL_UNSPECIFIED) {
        return "NORMAL_UNSPECIFIED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::CONGESTION) {
        return "CONGESTION";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_OUT_OF_ORDER) {
        return "NETWORK_OUT_OF_ORDER";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::TEMPORARY_FAILURE) {
        return "TEMPORARY_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::SWITCHING_EQUIPMENT_CONGESTION) {
        return "SWITCHING_EQUIPMENT_CONGESTION";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::ACCESS_INFORMATION_DISCARDED) {
        return "ACCESS_INFORMATION_DISCARDED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE) {
        return "REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::RESOURCES_UNAVAILABLE_OR_UNSPECIFIED) {
        return "RESOURCES_UNAVAILABLE_OR_UNSPECIFIED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::QOS_UNAVAILABLE) {
        return "QOS_UNAVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::REQUESTED_FACILITY_NOT_SUBSCRIBED) {
        return "REQUESTED_FACILITY_NOT_SUBSCRIBED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::INCOMING_CALLS_BARRED_WITHIN_CUG) {
        return "INCOMING_CALLS_BARRED_WITHIN_CUG";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::BEARER_CAPABILITY_NOT_AUTHORIZED) {
        return "BEARER_CAPABILITY_NOT_AUTHORIZED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::BEARER_CAPABILITY_UNAVAILABLE) {
        return "BEARER_CAPABILITY_UNAVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::SERVICE_OPTION_NOT_AVAILABLE) {
        return "SERVICE_OPTION_NOT_AVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::BEARER_SERVICE_NOT_IMPLEMENTED) {
        return "BEARER_SERVICE_NOT_IMPLEMENTED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::ACM_LIMIT_EXCEEDED) {
        return "ACM_LIMIT_EXCEEDED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::REQUESTED_FACILITY_NOT_IMPLEMENTED) {
        return "REQUESTED_FACILITY_NOT_IMPLEMENTED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::ONLY_DIGITAL_INFORMATION_BEARER_AVAILABLE) {
        return "ONLY_DIGITAL_INFORMATION_BEARER_AVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::SERVICE_OR_OPTION_NOT_IMPLEMENTED) {
        return "SERVICE_OR_OPTION_NOT_IMPLEMENTED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::INVALID_TRANSACTION_IDENTIFIER) {
        return "INVALID_TRANSACTION_IDENTIFIER";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::USER_NOT_MEMBER_OF_CUG) {
        return "USER_NOT_MEMBER_OF_CUG";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::INCOMPATIBLE_DESTINATION) {
        return "INCOMPATIBLE_DESTINATION";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::INVALID_TRANSIT_NW_SELECTION) {
        return "INVALID_TRANSIT_NW_SELECTION";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::SEMANTICALLY_INCORRECT_MESSAGE) {
        return "SEMANTICALLY_INCORRECT_MESSAGE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::INVALID_MANDATORY_INFORMATION) {
        return "INVALID_MANDATORY_INFORMATION";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::MESSAGE_TYPE_NON_IMPLEMENTED) {
        return "MESSAGE_TYPE_NON_IMPLEMENTED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE) {
        return "MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::INFORMATION_ELEMENT_NON_EXISTENT) {
        return "INFORMATION_ELEMENT_NON_EXISTENT";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::CONDITIONAL_IE_ERROR) {
        return "CONDITIONAL_IE_ERROR";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE) {
        return "MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::RECOVERY_ON_TIMER_EXPIRED) {
        return "RECOVERY_ON_TIMER_EXPIRED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::PROTOCOL_ERROR_UNSPECIFIED) {
        return "PROTOCOL_ERROR_UNSPECIFIED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::INTERWORKING_UNSPECIFIED) {
        return "INTERWORKING_UNSPECIFIED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::CALL_BARRED) {
        return "CALL_BARRED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::FDN_BLOCKED) {
        return "FDN_BLOCKED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::IMSI_UNKNOWN_IN_VLR) {
        return "IMSI_UNKNOWN_IN_VLR";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::IMEI_NOT_ACCEPTED) {
        return "IMEI_NOT_ACCEPTED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::DIAL_MODIFIED_TO_USSD) {
        return "DIAL_MODIFIED_TO_USSD";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::DIAL_MODIFIED_TO_SS) {
        return "DIAL_MODIFIED_TO_SS";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::DIAL_MODIFIED_TO_DIAL) {
        return "DIAL_MODIFIED_TO_DIAL";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_OFF) {
        return "RADIO_OFF";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OUT_OF_SERVICE) {
        return "OUT_OF_SERVICE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::NO_VALID_SIM) {
        return "NO_VALID_SIM";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_INTERNAL_ERROR) {
        return "RADIO_INTERNAL_ERROR";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_RESP_TIMEOUT) {
        return "NETWORK_RESP_TIMEOUT";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_REJECT) {
        return "NETWORK_REJECT";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_ACCESS_FAILURE) {
        return "RADIO_ACCESS_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_LINK_FAILURE) {
        return "RADIO_LINK_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_LINK_LOST) {
        return "RADIO_LINK_LOST";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_UPLINK_FAILURE) {
        return "RADIO_UPLINK_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_SETUP_FAILURE) {
        return "RADIO_SETUP_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_RELEASE_NORMAL) {
        return "RADIO_RELEASE_NORMAL";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_RELEASE_ABNORMAL) {
        return "RADIO_RELEASE_ABNORMAL";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::ACCESS_CLASS_BLOCKED) {
        return "ACCESS_CLASS_BLOCKED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_DETACH) {
        return "NETWORK_DETACH";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_LOCKED_UNTIL_POWER_CYCLE) {
        return "CDMA_LOCKED_UNTIL_POWER_CYCLE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_DROP) {
        return "CDMA_DROP";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_INTERCEPT) {
        return "CDMA_INTERCEPT";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_REORDER) {
        return "CDMA_REORDER";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_SO_REJECT) {
        return "CDMA_SO_REJECT";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_RETRY_ORDER) {
        return "CDMA_RETRY_ORDER";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_ACCESS_FAILURE) {
        return "CDMA_ACCESS_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_PREEMPTED) {
        return "CDMA_PREEMPTED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_NOT_EMERGENCY) {
        return "CDMA_NOT_EMERGENCY";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_ACCESS_BLOCKED) {
        return "CDMA_ACCESS_BLOCKED";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_1) {
        return "OEM_CAUSE_1";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_2) {
        return "OEM_CAUSE_2";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_3) {
        return "OEM_CAUSE_3";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_4) {
        return "OEM_CAUSE_4";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_5) {
        return "OEM_CAUSE_5";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_6) {
        return "OEM_CAUSE_6";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_7) {
        return "OEM_CAUSE_7";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_8) {
        return "OEM_CAUSE_8";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_9) {
        return "OEM_CAUSE_9";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_10) {
        return "OEM_CAUSE_10";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_11) {
        return "OEM_CAUSE_11";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_12) {
        return "OEM_CAUSE_12";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_13) {
        return "OEM_CAUSE_13";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_14) {
        return "OEM_CAUSE_14";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_15) {
        return "OEM_CAUSE_15";
    }
    if (o == ::android::hardware::radio::V1_0::LastCallFailCause::ERROR_UNSPECIFIED) {
        return "ERROR_UNSPECIFIED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::DataCallFailCause lhs, const ::android::hardware::radio::V1_0::DataCallFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::DataCallFailCause rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::DataCallFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::DataCallFailCause lhs, const ::android::hardware::radio::V1_0::DataCallFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::DataCallFailCause rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::DataCallFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::DataCallFailCause e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::DataCallFailCause e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::DataCallFailCause>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::DataCallFailCause> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::NONE) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::NONE)) {
        os += (first ? "" : " | ");
        os += "NONE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::NONE;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::OPERATOR_BARRED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::OPERATOR_BARRED)) {
        os += (first ? "" : " | ");
        os += "OPERATOR_BARRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::OPERATOR_BARRED;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::NAS_SIGNALLING) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::NAS_SIGNALLING)) {
        os += (first ? "" : " | ");
        os += "NAS_SIGNALLING";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::NAS_SIGNALLING;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::INSUFFICIENT_RESOURCES) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::INSUFFICIENT_RESOURCES)) {
        os += (first ? "" : " | ");
        os += "INSUFFICIENT_RESOURCES";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::INSUFFICIENT_RESOURCES;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::MISSING_UKNOWN_APN) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::MISSING_UKNOWN_APN)) {
        os += (first ? "" : " | ");
        os += "MISSING_UKNOWN_APN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::MISSING_UKNOWN_APN;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::UNKNOWN_PDP_ADDRESS_TYPE) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::UNKNOWN_PDP_ADDRESS_TYPE)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN_PDP_ADDRESS_TYPE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::UNKNOWN_PDP_ADDRESS_TYPE;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::USER_AUTHENTICATION) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::USER_AUTHENTICATION)) {
        os += (first ? "" : " | ");
        os += "USER_AUTHENTICATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::USER_AUTHENTICATION;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::ACTIVATION_REJECT_GGSN) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::ACTIVATION_REJECT_GGSN)) {
        os += (first ? "" : " | ");
        os += "ACTIVATION_REJECT_GGSN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::ACTIVATION_REJECT_GGSN;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::ACTIVATION_REJECT_UNSPECIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::ACTIVATION_REJECT_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "ACTIVATION_REJECT_UNSPECIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::ACTIVATION_REJECT_UNSPECIFIED;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::SERVICE_OPTION_NOT_SUPPORTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::SERVICE_OPTION_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "SERVICE_OPTION_NOT_SUPPORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::SERVICE_OPTION_NOT_SUPPORTED;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::SERVICE_OPTION_NOT_SUBSCRIBED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::SERVICE_OPTION_NOT_SUBSCRIBED)) {
        os += (first ? "" : " | ");
        os += "SERVICE_OPTION_NOT_SUBSCRIBED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::SERVICE_OPTION_NOT_SUBSCRIBED;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::SERVICE_OPTION_OUT_OF_ORDER) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::SERVICE_OPTION_OUT_OF_ORDER)) {
        os += (first ? "" : " | ");
        os += "SERVICE_OPTION_OUT_OF_ORDER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::SERVICE_OPTION_OUT_OF_ORDER;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::NSAPI_IN_USE) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::NSAPI_IN_USE)) {
        os += (first ? "" : " | ");
        os += "NSAPI_IN_USE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::NSAPI_IN_USE;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::REGULAR_DEACTIVATION) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::REGULAR_DEACTIVATION)) {
        os += (first ? "" : " | ");
        os += "REGULAR_DEACTIVATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::REGULAR_DEACTIVATION;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::QOS_NOT_ACCEPTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::QOS_NOT_ACCEPTED)) {
        os += (first ? "" : " | ");
        os += "QOS_NOT_ACCEPTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::QOS_NOT_ACCEPTED;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::NETWORK_FAILURE) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::NETWORK_FAILURE)) {
        os += (first ? "" : " | ");
        os += "NETWORK_FAILURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::NETWORK_FAILURE;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::UMTS_REACTIVATION_REQ) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::UMTS_REACTIVATION_REQ)) {
        os += (first ? "" : " | ");
        os += "UMTS_REACTIVATION_REQ";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::UMTS_REACTIVATION_REQ;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::FEATURE_NOT_SUPP) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::FEATURE_NOT_SUPP)) {
        os += (first ? "" : " | ");
        os += "FEATURE_NOT_SUPP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::FEATURE_NOT_SUPP;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::TFT_SEMANTIC_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::TFT_SEMANTIC_ERROR)) {
        os += (first ? "" : " | ");
        os += "TFT_SEMANTIC_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::TFT_SEMANTIC_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::TFT_SYTAX_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::TFT_SYTAX_ERROR)) {
        os += (first ? "" : " | ");
        os += "TFT_SYTAX_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::TFT_SYTAX_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::UNKNOWN_PDP_CONTEXT) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::UNKNOWN_PDP_CONTEXT)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN_PDP_CONTEXT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::UNKNOWN_PDP_CONTEXT;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::FILTER_SEMANTIC_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::FILTER_SEMANTIC_ERROR)) {
        os += (first ? "" : " | ");
        os += "FILTER_SEMANTIC_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::FILTER_SEMANTIC_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::FILTER_SYTAX_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::FILTER_SYTAX_ERROR)) {
        os += (first ? "" : " | ");
        os += "FILTER_SYTAX_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::FILTER_SYTAX_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::PDP_WITHOUT_ACTIVE_TFT) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::PDP_WITHOUT_ACTIVE_TFT)) {
        os += (first ? "" : " | ");
        os += "PDP_WITHOUT_ACTIVE_TFT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::PDP_WITHOUT_ACTIVE_TFT;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::ONLY_IPV4_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::ONLY_IPV4_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "ONLY_IPV4_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::ONLY_IPV4_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::ONLY_IPV6_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::ONLY_IPV6_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "ONLY_IPV6_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::ONLY_IPV6_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::ONLY_SINGLE_BEARER_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::ONLY_SINGLE_BEARER_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "ONLY_SINGLE_BEARER_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::ONLY_SINGLE_BEARER_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::ESM_INFO_NOT_RECEIVED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::ESM_INFO_NOT_RECEIVED)) {
        os += (first ? "" : " | ");
        os += "ESM_INFO_NOT_RECEIVED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::ESM_INFO_NOT_RECEIVED;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::PDN_CONN_DOES_NOT_EXIST) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::PDN_CONN_DOES_NOT_EXIST)) {
        os += (first ? "" : " | ");
        os += "PDN_CONN_DOES_NOT_EXIST";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::PDN_CONN_DOES_NOT_EXIST;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::MAX_ACTIVE_PDP_CONTEXT_REACHED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::MAX_ACTIVE_PDP_CONTEXT_REACHED)) {
        os += (first ? "" : " | ");
        os += "MAX_ACTIVE_PDP_CONTEXT_REACHED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::MAX_ACTIVE_PDP_CONTEXT_REACHED;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::UNSUPPORTED_APN_IN_CURRENT_PLMN) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::UNSUPPORTED_APN_IN_CURRENT_PLMN)) {
        os += (first ? "" : " | ");
        os += "UNSUPPORTED_APN_IN_CURRENT_PLMN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::UNSUPPORTED_APN_IN_CURRENT_PLMN;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::INVALID_TRANSACTION_ID) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::INVALID_TRANSACTION_ID)) {
        os += (first ? "" : " | ");
        os += "INVALID_TRANSACTION_ID";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::INVALID_TRANSACTION_ID;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::MESSAGE_INCORRECT_SEMANTIC) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::MESSAGE_INCORRECT_SEMANTIC)) {
        os += (first ? "" : " | ");
        os += "MESSAGE_INCORRECT_SEMANTIC";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::MESSAGE_INCORRECT_SEMANTIC;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::INVALID_MANDATORY_INFO) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::INVALID_MANDATORY_INFO)) {
        os += (first ? "" : " | ");
        os += "INVALID_MANDATORY_INFO";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::INVALID_MANDATORY_INFO;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::MESSAGE_TYPE_UNSUPPORTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::MESSAGE_TYPE_UNSUPPORTED)) {
        os += (first ? "" : " | ");
        os += "MESSAGE_TYPE_UNSUPPORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::MESSAGE_TYPE_UNSUPPORTED;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::MSG_TYPE_NONCOMPATIBLE_STATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::MSG_TYPE_NONCOMPATIBLE_STATE)) {
        os += (first ? "" : " | ");
        os += "MSG_TYPE_NONCOMPATIBLE_STATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::MSG_TYPE_NONCOMPATIBLE_STATE;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::UNKNOWN_INFO_ELEMENT) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::UNKNOWN_INFO_ELEMENT)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN_INFO_ELEMENT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::UNKNOWN_INFO_ELEMENT;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::CONDITIONAL_IE_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::CONDITIONAL_IE_ERROR)) {
        os += (first ? "" : " | ");
        os += "CONDITIONAL_IE_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::CONDITIONAL_IE_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE)) {
        os += (first ? "" : " | ");
        os += "MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::PROTOCOL_ERRORS) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::PROTOCOL_ERRORS)) {
        os += (first ? "" : " | ");
        os += "PROTOCOL_ERRORS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::PROTOCOL_ERRORS;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::APN_TYPE_CONFLICT) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::APN_TYPE_CONFLICT)) {
        os += (first ? "" : " | ");
        os += "APN_TYPE_CONFLICT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::APN_TYPE_CONFLICT;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::INVALID_PCSCF_ADDR) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::INVALID_PCSCF_ADDR)) {
        os += (first ? "" : " | ");
        os += "INVALID_PCSCF_ADDR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::INVALID_PCSCF_ADDR;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN)) {
        os += (first ? "" : " | ");
        os += "INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::EMM_ACCESS_BARRED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::EMM_ACCESS_BARRED)) {
        os += (first ? "" : " | ");
        os += "EMM_ACCESS_BARRED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::EMM_ACCESS_BARRED;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::EMERGENCY_IFACE_ONLY) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::EMERGENCY_IFACE_ONLY)) {
        os += (first ? "" : " | ");
        os += "EMERGENCY_IFACE_ONLY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::EMERGENCY_IFACE_ONLY;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::IFACE_MISMATCH) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::IFACE_MISMATCH)) {
        os += (first ? "" : " | ");
        os += "IFACE_MISMATCH";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::IFACE_MISMATCH;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::COMPANION_IFACE_IN_USE) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::COMPANION_IFACE_IN_USE)) {
        os += (first ? "" : " | ");
        os += "COMPANION_IFACE_IN_USE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::COMPANION_IFACE_IN_USE;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::IP_ADDRESS_MISMATCH) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::IP_ADDRESS_MISMATCH)) {
        os += (first ? "" : " | ");
        os += "IP_ADDRESS_MISMATCH";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::IP_ADDRESS_MISMATCH;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::IFACE_AND_POL_FAMILY_MISMATCH) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::IFACE_AND_POL_FAMILY_MISMATCH)) {
        os += (first ? "" : " | ");
        os += "IFACE_AND_POL_FAMILY_MISMATCH";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::IFACE_AND_POL_FAMILY_MISMATCH;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::EMM_ACCESS_BARRED_INFINITE_RETRY) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::EMM_ACCESS_BARRED_INFINITE_RETRY)) {
        os += (first ? "" : " | ");
        os += "EMM_ACCESS_BARRED_INFINITE_RETRY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::EMM_ACCESS_BARRED_INFINITE_RETRY;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::AUTH_FAILURE_ON_EMERGENCY_CALL) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::AUTH_FAILURE_ON_EMERGENCY_CALL)) {
        os += (first ? "" : " | ");
        os += "AUTH_FAILURE_ON_EMERGENCY_CALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::AUTH_FAILURE_ON_EMERGENCY_CALL;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_1) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_1)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_1";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_1;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_2) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_2)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_2";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_2;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_3) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_3)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_3";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_3;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_4) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_4)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_4";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_4;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_5) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_5)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_5";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_5;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_6) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_6)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_6";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_6;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_7) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_7)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_7";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_7;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_8) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_8)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_8";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_8;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_9) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_9)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_9";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_9;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_10) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_10)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_10";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_10;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_11) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_11)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_11";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_11;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_12) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_12)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_12";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_12;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_13) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_13)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_13";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_13;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_14) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_14)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_14";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_14;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_15) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_15)) {
        os += (first ? "" : " | ");
        os += "OEM_DCFAILCAUSE_15";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_15;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::VOICE_REGISTRATION_FAIL) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::VOICE_REGISTRATION_FAIL)) {
        os += (first ? "" : " | ");
        os += "VOICE_REGISTRATION_FAIL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::VOICE_REGISTRATION_FAIL;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::DATA_REGISTRATION_FAIL) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::DATA_REGISTRATION_FAIL)) {
        os += (first ? "" : " | ");
        os += "DATA_REGISTRATION_FAIL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::DATA_REGISTRATION_FAIL;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::SIGNAL_LOST) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::SIGNAL_LOST)) {
        os += (first ? "" : " | ");
        os += "SIGNAL_LOST";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::SIGNAL_LOST;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::PREF_RADIO_TECH_CHANGED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::PREF_RADIO_TECH_CHANGED)) {
        os += (first ? "" : " | ");
        os += "PREF_RADIO_TECH_CHANGED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::PREF_RADIO_TECH_CHANGED;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::RADIO_POWER_OFF) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::RADIO_POWER_OFF)) {
        os += (first ? "" : " | ");
        os += "RADIO_POWER_OFF";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::RADIO_POWER_OFF;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::TETHERED_CALL_ACTIVE) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::TETHERED_CALL_ACTIVE)) {
        os += (first ? "" : " | ");
        os += "TETHERED_CALL_ACTIVE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::TETHERED_CALL_ACTIVE;
    }
    if ((o & ::android::hardware::radio::V1_0::DataCallFailCause::ERROR_UNSPECIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataCallFailCause::ERROR_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "ERROR_UNSPECIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataCallFailCause::ERROR_UNSPECIFIED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::DataCallFailCause o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::NONE) {
        return "NONE";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::OPERATOR_BARRED) {
        return "OPERATOR_BARRED";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::NAS_SIGNALLING) {
        return "NAS_SIGNALLING";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::INSUFFICIENT_RESOURCES) {
        return "INSUFFICIENT_RESOURCES";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::MISSING_UKNOWN_APN) {
        return "MISSING_UKNOWN_APN";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::UNKNOWN_PDP_ADDRESS_TYPE) {
        return "UNKNOWN_PDP_ADDRESS_TYPE";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::USER_AUTHENTICATION) {
        return "USER_AUTHENTICATION";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::ACTIVATION_REJECT_GGSN) {
        return "ACTIVATION_REJECT_GGSN";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::ACTIVATION_REJECT_UNSPECIFIED) {
        return "ACTIVATION_REJECT_UNSPECIFIED";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::SERVICE_OPTION_NOT_SUPPORTED) {
        return "SERVICE_OPTION_NOT_SUPPORTED";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::SERVICE_OPTION_NOT_SUBSCRIBED) {
        return "SERVICE_OPTION_NOT_SUBSCRIBED";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::SERVICE_OPTION_OUT_OF_ORDER) {
        return "SERVICE_OPTION_OUT_OF_ORDER";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::NSAPI_IN_USE) {
        return "NSAPI_IN_USE";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::REGULAR_DEACTIVATION) {
        return "REGULAR_DEACTIVATION";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::QOS_NOT_ACCEPTED) {
        return "QOS_NOT_ACCEPTED";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::NETWORK_FAILURE) {
        return "NETWORK_FAILURE";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::UMTS_REACTIVATION_REQ) {
        return "UMTS_REACTIVATION_REQ";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::FEATURE_NOT_SUPP) {
        return "FEATURE_NOT_SUPP";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::TFT_SEMANTIC_ERROR) {
        return "TFT_SEMANTIC_ERROR";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::TFT_SYTAX_ERROR) {
        return "TFT_SYTAX_ERROR";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::UNKNOWN_PDP_CONTEXT) {
        return "UNKNOWN_PDP_CONTEXT";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::FILTER_SEMANTIC_ERROR) {
        return "FILTER_SEMANTIC_ERROR";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::FILTER_SYTAX_ERROR) {
        return "FILTER_SYTAX_ERROR";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::PDP_WITHOUT_ACTIVE_TFT) {
        return "PDP_WITHOUT_ACTIVE_TFT";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::ONLY_IPV4_ALLOWED) {
        return "ONLY_IPV4_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::ONLY_IPV6_ALLOWED) {
        return "ONLY_IPV6_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::ONLY_SINGLE_BEARER_ALLOWED) {
        return "ONLY_SINGLE_BEARER_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::ESM_INFO_NOT_RECEIVED) {
        return "ESM_INFO_NOT_RECEIVED";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::PDN_CONN_DOES_NOT_EXIST) {
        return "PDN_CONN_DOES_NOT_EXIST";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED) {
        return "MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::MAX_ACTIVE_PDP_CONTEXT_REACHED) {
        return "MAX_ACTIVE_PDP_CONTEXT_REACHED";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::UNSUPPORTED_APN_IN_CURRENT_PLMN) {
        return "UNSUPPORTED_APN_IN_CURRENT_PLMN";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::INVALID_TRANSACTION_ID) {
        return "INVALID_TRANSACTION_ID";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::MESSAGE_INCORRECT_SEMANTIC) {
        return "MESSAGE_INCORRECT_SEMANTIC";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::INVALID_MANDATORY_INFO) {
        return "INVALID_MANDATORY_INFO";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::MESSAGE_TYPE_UNSUPPORTED) {
        return "MESSAGE_TYPE_UNSUPPORTED";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::MSG_TYPE_NONCOMPATIBLE_STATE) {
        return "MSG_TYPE_NONCOMPATIBLE_STATE";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::UNKNOWN_INFO_ELEMENT) {
        return "UNKNOWN_INFO_ELEMENT";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::CONDITIONAL_IE_ERROR) {
        return "CONDITIONAL_IE_ERROR";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE) {
        return "MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::PROTOCOL_ERRORS) {
        return "PROTOCOL_ERRORS";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::APN_TYPE_CONFLICT) {
        return "APN_TYPE_CONFLICT";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::INVALID_PCSCF_ADDR) {
        return "INVALID_PCSCF_ADDR";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN) {
        return "INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::EMM_ACCESS_BARRED) {
        return "EMM_ACCESS_BARRED";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::EMERGENCY_IFACE_ONLY) {
        return "EMERGENCY_IFACE_ONLY";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::IFACE_MISMATCH) {
        return "IFACE_MISMATCH";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::COMPANION_IFACE_IN_USE) {
        return "COMPANION_IFACE_IN_USE";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::IP_ADDRESS_MISMATCH) {
        return "IP_ADDRESS_MISMATCH";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::IFACE_AND_POL_FAMILY_MISMATCH) {
        return "IFACE_AND_POL_FAMILY_MISMATCH";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::EMM_ACCESS_BARRED_INFINITE_RETRY) {
        return "EMM_ACCESS_BARRED_INFINITE_RETRY";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::AUTH_FAILURE_ON_EMERGENCY_CALL) {
        return "AUTH_FAILURE_ON_EMERGENCY_CALL";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_1) {
        return "OEM_DCFAILCAUSE_1";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_2) {
        return "OEM_DCFAILCAUSE_2";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_3) {
        return "OEM_DCFAILCAUSE_3";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_4) {
        return "OEM_DCFAILCAUSE_4";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_5) {
        return "OEM_DCFAILCAUSE_5";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_6) {
        return "OEM_DCFAILCAUSE_6";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_7) {
        return "OEM_DCFAILCAUSE_7";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_8) {
        return "OEM_DCFAILCAUSE_8";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_9) {
        return "OEM_DCFAILCAUSE_9";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_10) {
        return "OEM_DCFAILCAUSE_10";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_11) {
        return "OEM_DCFAILCAUSE_11";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_12) {
        return "OEM_DCFAILCAUSE_12";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_13) {
        return "OEM_DCFAILCAUSE_13";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_14) {
        return "OEM_DCFAILCAUSE_14";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_15) {
        return "OEM_DCFAILCAUSE_15";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::VOICE_REGISTRATION_FAIL) {
        return "VOICE_REGISTRATION_FAIL";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::DATA_REGISTRATION_FAIL) {
        return "DATA_REGISTRATION_FAIL";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::SIGNAL_LOST) {
        return "SIGNAL_LOST";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::PREF_RADIO_TECH_CHANGED) {
        return "PREF_RADIO_TECH_CHANGED";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::RADIO_POWER_OFF) {
        return "RADIO_POWER_OFF";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::TETHERED_CALL_ACTIVE) {
        return "TETHERED_CALL_ACTIVE";
    }
    if (o == ::android::hardware::radio::V1_0::DataCallFailCause::ERROR_UNSPECIFIED) {
        return "ERROR_UNSPECIFIED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RegState lhs, const ::android::hardware::radio::V1_0::RegState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::RegState rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RegState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RegState lhs, const ::android::hardware::radio::V1_0::RegState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::RegState rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RegState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::RegState e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::RegState e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::RegState>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::RegState> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::RegState::NOT_REG_MT_NOT_SEARCHING_OP) == static_cast<int32_t>(::android::hardware::radio::V1_0::RegState::NOT_REG_MT_NOT_SEARCHING_OP)) {
        os += (first ? "" : " | ");
        os += "NOT_REG_MT_NOT_SEARCHING_OP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RegState::NOT_REG_MT_NOT_SEARCHING_OP;
    }
    if ((o & ::android::hardware::radio::V1_0::RegState::REG_HOME) == static_cast<int32_t>(::android::hardware::radio::V1_0::RegState::REG_HOME)) {
        os += (first ? "" : " | ");
        os += "REG_HOME";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RegState::REG_HOME;
    }
    if ((o & ::android::hardware::radio::V1_0::RegState::NOT_REG_MT_SEARCHING_OP) == static_cast<int32_t>(::android::hardware::radio::V1_0::RegState::NOT_REG_MT_SEARCHING_OP)) {
        os += (first ? "" : " | ");
        os += "NOT_REG_MT_SEARCHING_OP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RegState::NOT_REG_MT_SEARCHING_OP;
    }
    if ((o & ::android::hardware::radio::V1_0::RegState::REG_DENIED) == static_cast<int32_t>(::android::hardware::radio::V1_0::RegState::REG_DENIED)) {
        os += (first ? "" : " | ");
        os += "REG_DENIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RegState::REG_DENIED;
    }
    if ((o & ::android::hardware::radio::V1_0::RegState::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::RegState::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RegState::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_0::RegState::REG_ROAMING) == static_cast<int32_t>(::android::hardware::radio::V1_0::RegState::REG_ROAMING)) {
        os += (first ? "" : " | ");
        os += "REG_ROAMING";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RegState::REG_ROAMING;
    }
    if ((o & ::android::hardware::radio::V1_0::RegState::NOT_REG_MT_NOT_SEARCHING_OP_EM) == static_cast<int32_t>(::android::hardware::radio::V1_0::RegState::NOT_REG_MT_NOT_SEARCHING_OP_EM)) {
        os += (first ? "" : " | ");
        os += "NOT_REG_MT_NOT_SEARCHING_OP_EM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RegState::NOT_REG_MT_NOT_SEARCHING_OP_EM;
    }
    if ((o & ::android::hardware::radio::V1_0::RegState::NOT_REG_MT_SEARCHING_OP_EM) == static_cast<int32_t>(::android::hardware::radio::V1_0::RegState::NOT_REG_MT_SEARCHING_OP_EM)) {
        os += (first ? "" : " | ");
        os += "NOT_REG_MT_SEARCHING_OP_EM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RegState::NOT_REG_MT_SEARCHING_OP_EM;
    }
    if ((o & ::android::hardware::radio::V1_0::RegState::REG_DENIED_EM) == static_cast<int32_t>(::android::hardware::radio::V1_0::RegState::REG_DENIED_EM)) {
        os += (first ? "" : " | ");
        os += "REG_DENIED_EM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RegState::REG_DENIED_EM;
    }
    if ((o & ::android::hardware::radio::V1_0::RegState::UNKNOWN_EM) == static_cast<int32_t>(::android::hardware::radio::V1_0::RegState::UNKNOWN_EM)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN_EM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RegState::UNKNOWN_EM;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::RegState o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::RegState::NOT_REG_MT_NOT_SEARCHING_OP) {
        return "NOT_REG_MT_NOT_SEARCHING_OP";
    }
    if (o == ::android::hardware::radio::V1_0::RegState::REG_HOME) {
        return "REG_HOME";
    }
    if (o == ::android::hardware::radio::V1_0::RegState::NOT_REG_MT_SEARCHING_OP) {
        return "NOT_REG_MT_SEARCHING_OP";
    }
    if (o == ::android::hardware::radio::V1_0::RegState::REG_DENIED) {
        return "REG_DENIED";
    }
    if (o == ::android::hardware::radio::V1_0::RegState::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_0::RegState::REG_ROAMING) {
        return "REG_ROAMING";
    }
    if (o == ::android::hardware::radio::V1_0::RegState::NOT_REG_MT_NOT_SEARCHING_OP_EM) {
        return "NOT_REG_MT_NOT_SEARCHING_OP_EM";
    }
    if (o == ::android::hardware::radio::V1_0::RegState::NOT_REG_MT_SEARCHING_OP_EM) {
        return "NOT_REG_MT_SEARCHING_OP_EM";
    }
    if (o == ::android::hardware::radio::V1_0::RegState::REG_DENIED_EM) {
        return "REG_DENIED_EM";
    }
    if (o == ::android::hardware::radio::V1_0::RegState::UNKNOWN_EM) {
        return "UNKNOWN_EM";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioTechnology lhs, const ::android::hardware::radio::V1_0::RadioTechnology rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioTechnology rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioTechnology lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioTechnology lhs, const ::android::hardware::radio::V1_0::RadioTechnology rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioTechnology rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioTechnology lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::RadioTechnology e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::RadioTechnology e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::RadioTechnology>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::RadioTechnology> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::GPRS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::GPRS)) {
        os += (first ? "" : " | ");
        os += "GPRS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::GPRS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::EDGE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::EDGE)) {
        os += (first ? "" : " | ");
        os += "EDGE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::EDGE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::UMTS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::UMTS)) {
        os += (first ? "" : " | ");
        os += "UMTS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::UMTS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::IS95A) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::IS95A)) {
        os += (first ? "" : " | ");
        os += "IS95A";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::IS95A;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::IS95B) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::IS95B)) {
        os += (first ? "" : " | ");
        os += "IS95B";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::IS95B;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::ONE_X_RTT) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::ONE_X_RTT)) {
        os += (first ? "" : " | ");
        os += "ONE_X_RTT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::ONE_X_RTT;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::EVDO_0) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::EVDO_0)) {
        os += (first ? "" : " | ");
        os += "EVDO_0";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::EVDO_0;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::EVDO_A) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::EVDO_A)) {
        os += (first ? "" : " | ");
        os += "EVDO_A";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::EVDO_A;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::HSDPA) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::HSDPA)) {
        os += (first ? "" : " | ");
        os += "HSDPA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::HSDPA;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::HSUPA) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::HSUPA)) {
        os += (first ? "" : " | ");
        os += "HSUPA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::HSUPA;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::HSPA) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::HSPA)) {
        os += (first ? "" : " | ");
        os += "HSPA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::HSPA;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::EVDO_B) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::EVDO_B)) {
        os += (first ? "" : " | ");
        os += "EVDO_B";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::EVDO_B;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::EHRPD) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::EHRPD)) {
        os += (first ? "" : " | ");
        os += "EHRPD";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::EHRPD;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::LTE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::LTE)) {
        os += (first ? "" : " | ");
        os += "LTE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::LTE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::HSPAP) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::HSPAP)) {
        os += (first ? "" : " | ");
        os += "HSPAP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::HSPAP;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::GSM) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::GSM)) {
        os += (first ? "" : " | ");
        os += "GSM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::GSM;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::TD_SCDMA) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::TD_SCDMA)) {
        os += (first ? "" : " | ");
        os += "TD_SCDMA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::TD_SCDMA;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::IWLAN) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::IWLAN)) {
        os += (first ? "" : " | ");
        os += "IWLAN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::IWLAN;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnology::LTE_CA) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnology::LTE_CA)) {
        os += (first ? "" : " | ");
        os += "LTE_CA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnology::LTE_CA;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::RadioTechnology o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::GPRS) {
        return "GPRS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::EDGE) {
        return "EDGE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::UMTS) {
        return "UMTS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::IS95A) {
        return "IS95A";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::IS95B) {
        return "IS95B";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::ONE_X_RTT) {
        return "ONE_X_RTT";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::EVDO_0) {
        return "EVDO_0";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::EVDO_A) {
        return "EVDO_A";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::HSDPA) {
        return "HSDPA";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::HSUPA) {
        return "HSUPA";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::HSPA) {
        return "HSPA";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::EVDO_B) {
        return "EVDO_B";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::EHRPD) {
        return "EHRPD";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::LTE) {
        return "LTE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::HSPAP) {
        return "HSPAP";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::GSM) {
        return "GSM";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::TD_SCDMA) {
        return "TD_SCDMA";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::IWLAN) {
        return "IWLAN";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnology::LTE_CA) {
        return "LTE_CA";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::DataProfileId lhs, const ::android::hardware::radio::V1_0::DataProfileId rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::DataProfileId rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::DataProfileId lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::DataProfileId lhs, const ::android::hardware::radio::V1_0::DataProfileId rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::DataProfileId rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::DataProfileId lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::DataProfileId e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::DataProfileId e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::DataProfileId>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::DataProfileId> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::DataProfileId::DEFAULT) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataProfileId::DEFAULT)) {
        os += (first ? "" : " | ");
        os += "DEFAULT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataProfileId::DEFAULT;
    }
    if ((o & ::android::hardware::radio::V1_0::DataProfileId::TETHERED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataProfileId::TETHERED)) {
        os += (first ? "" : " | ");
        os += "TETHERED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataProfileId::TETHERED;
    }
    if ((o & ::android::hardware::radio::V1_0::DataProfileId::IMS) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataProfileId::IMS)) {
        os += (first ? "" : " | ");
        os += "IMS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataProfileId::IMS;
    }
    if ((o & ::android::hardware::radio::V1_0::DataProfileId::FOTA) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataProfileId::FOTA)) {
        os += (first ? "" : " | ");
        os += "FOTA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataProfileId::FOTA;
    }
    if ((o & ::android::hardware::radio::V1_0::DataProfileId::CBS) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataProfileId::CBS)) {
        os += (first ? "" : " | ");
        os += "CBS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataProfileId::CBS;
    }
    if ((o & ::android::hardware::radio::V1_0::DataProfileId::OEM_BASE) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataProfileId::OEM_BASE)) {
        os += (first ? "" : " | ");
        os += "OEM_BASE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataProfileId::OEM_BASE;
    }
    if ((o & ::android::hardware::radio::V1_0::DataProfileId::INVALID) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataProfileId::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataProfileId::INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::DataProfileId o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::DataProfileId::DEFAULT) {
        return "DEFAULT";
    }
    if (o == ::android::hardware::radio::V1_0::DataProfileId::TETHERED) {
        return "TETHERED";
    }
    if (o == ::android::hardware::radio::V1_0::DataProfileId::IMS) {
        return "IMS";
    }
    if (o == ::android::hardware::radio::V1_0::DataProfileId::FOTA) {
        return "FOTA";
    }
    if (o == ::android::hardware::radio::V1_0::DataProfileId::CBS) {
        return "CBS";
    }
    if (o == ::android::hardware::radio::V1_0::DataProfileId::OEM_BASE) {
        return "OEM_BASE";
    }
    if (o == ::android::hardware::radio::V1_0::DataProfileId::INVALID) {
        return "INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause lhs, const ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause lhs, const ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::SmsAcknowledgeFailCause>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::SmsAcknowledgeFailCause> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause::MEMORY_CAPACITY_EXCEEDED) == static_cast<int32_t>(::android::hardware::radio::V1_0::SmsAcknowledgeFailCause::MEMORY_CAPACITY_EXCEEDED)) {
        os += (first ? "" : " | ");
        os += "MEMORY_CAPACITY_EXCEEDED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause::MEMORY_CAPACITY_EXCEEDED;
    }
    if ((o & ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause::UNSPECIFIED_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_0::SmsAcknowledgeFailCause::UNSPECIFIED_ERROR)) {
        os += (first ? "" : " | ");
        os += "UNSPECIFIED_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause::UNSPECIFIED_ERROR;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::SmsAcknowledgeFailCause o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause::MEMORY_CAPACITY_EXCEEDED) {
        return "MEMORY_CAPACITY_EXCEEDED";
    }
    if (o == ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause::UNSPECIFIED_ERROR) {
        return "UNSPECIFIED_ERROR";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CallForwardInfoStatus lhs, const ::android::hardware::radio::V1_0::CallForwardInfoStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CallForwardInfoStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CallForwardInfoStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CallForwardInfoStatus lhs, const ::android::hardware::radio::V1_0::CallForwardInfoStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CallForwardInfoStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CallForwardInfoStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CallForwardInfoStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CallForwardInfoStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CallForwardInfoStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CallForwardInfoStatus> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CallForwardInfoStatus::DISABLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::CallForwardInfoStatus::DISABLE)) {
        os += (first ? "" : " | ");
        os += "DISABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CallForwardInfoStatus::DISABLE;
    }
    if ((o & ::android::hardware::radio::V1_0::CallForwardInfoStatus::ENABLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::CallForwardInfoStatus::ENABLE)) {
        os += (first ? "" : " | ");
        os += "ENABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CallForwardInfoStatus::ENABLE;
    }
    if ((o & ::android::hardware::radio::V1_0::CallForwardInfoStatus::INTERROGATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::CallForwardInfoStatus::INTERROGATE)) {
        os += (first ? "" : " | ");
        os += "INTERROGATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CallForwardInfoStatus::INTERROGATE;
    }
    if ((o & ::android::hardware::radio::V1_0::CallForwardInfoStatus::REGISTRATION) == static_cast<int32_t>(::android::hardware::radio::V1_0::CallForwardInfoStatus::REGISTRATION)) {
        os += (first ? "" : " | ");
        os += "REGISTRATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CallForwardInfoStatus::REGISTRATION;
    }
    if ((o & ::android::hardware::radio::V1_0::CallForwardInfoStatus::ERASURE) == static_cast<int32_t>(::android::hardware::radio::V1_0::CallForwardInfoStatus::ERASURE)) {
        os += (first ? "" : " | ");
        os += "ERASURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CallForwardInfoStatus::ERASURE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CallForwardInfoStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CallForwardInfoStatus::DISABLE) {
        return "DISABLE";
    }
    if (o == ::android::hardware::radio::V1_0::CallForwardInfoStatus::ENABLE) {
        return "ENABLE";
    }
    if (o == ::android::hardware::radio::V1_0::CallForwardInfoStatus::INTERROGATE) {
        return "INTERROGATE";
    }
    if (o == ::android::hardware::radio::V1_0::CallForwardInfoStatus::REGISTRATION) {
        return "REGISTRATION";
    }
    if (o == ::android::hardware::radio::V1_0::CallForwardInfoStatus::ERASURE) {
        return "ERASURE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::ClipStatus lhs, const ::android::hardware::radio::V1_0::ClipStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::ClipStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::ClipStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::ClipStatus lhs, const ::android::hardware::radio::V1_0::ClipStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::ClipStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::ClipStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::ClipStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::ClipStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::ClipStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::ClipStatus> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::ClipStatus::CLIP_PROVISIONED) == static_cast<int32_t>(::android::hardware::radio::V1_0::ClipStatus::CLIP_PROVISIONED)) {
        os += (first ? "" : " | ");
        os += "CLIP_PROVISIONED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ClipStatus::CLIP_PROVISIONED;
    }
    if ((o & ::android::hardware::radio::V1_0::ClipStatus::CLIP_UNPROVISIONED) == static_cast<int32_t>(::android::hardware::radio::V1_0::ClipStatus::CLIP_UNPROVISIONED)) {
        os += (first ? "" : " | ");
        os += "CLIP_UNPROVISIONED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ClipStatus::CLIP_UNPROVISIONED;
    }
    if ((o & ::android::hardware::radio::V1_0::ClipStatus::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::ClipStatus::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ClipStatus::UNKNOWN;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::ClipStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::ClipStatus::CLIP_PROVISIONED) {
        return "CLIP_PROVISIONED";
    }
    if (o == ::android::hardware::radio::V1_0::ClipStatus::CLIP_UNPROVISIONED) {
        return "CLIP_UNPROVISIONED";
    }
    if (o == ::android::hardware::radio::V1_0::ClipStatus::UNKNOWN) {
        return "UNKNOWN";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SmsWriteArgsStatus lhs, const ::android::hardware::radio::V1_0::SmsWriteArgsStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::SmsWriteArgsStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SmsWriteArgsStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SmsWriteArgsStatus lhs, const ::android::hardware::radio::V1_0::SmsWriteArgsStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::SmsWriteArgsStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SmsWriteArgsStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::SmsWriteArgsStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::SmsWriteArgsStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::SmsWriteArgsStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::SmsWriteArgsStatus> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::SmsWriteArgsStatus::REC_UNREAD) == static_cast<int32_t>(::android::hardware::radio::V1_0::SmsWriteArgsStatus::REC_UNREAD)) {
        os += (first ? "" : " | ");
        os += "REC_UNREAD";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SmsWriteArgsStatus::REC_UNREAD;
    }
    if ((o & ::android::hardware::radio::V1_0::SmsWriteArgsStatus::REC_READ) == static_cast<int32_t>(::android::hardware::radio::V1_0::SmsWriteArgsStatus::REC_READ)) {
        os += (first ? "" : " | ");
        os += "REC_READ";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SmsWriteArgsStatus::REC_READ;
    }
    if ((o & ::android::hardware::radio::V1_0::SmsWriteArgsStatus::STO_UNSENT) == static_cast<int32_t>(::android::hardware::radio::V1_0::SmsWriteArgsStatus::STO_UNSENT)) {
        os += (first ? "" : " | ");
        os += "STO_UNSENT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SmsWriteArgsStatus::STO_UNSENT;
    }
    if ((o & ::android::hardware::radio::V1_0::SmsWriteArgsStatus::STO_SENT) == static_cast<int32_t>(::android::hardware::radio::V1_0::SmsWriteArgsStatus::STO_SENT)) {
        os += (first ? "" : " | ");
        os += "STO_SENT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SmsWriteArgsStatus::STO_SENT;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::SmsWriteArgsStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::SmsWriteArgsStatus::REC_UNREAD) {
        return "REC_UNREAD";
    }
    if (o == ::android::hardware::radio::V1_0::SmsWriteArgsStatus::REC_READ) {
        return "REC_READ";
    }
    if (o == ::android::hardware::radio::V1_0::SmsWriteArgsStatus::STO_UNSENT) {
        return "STO_UNSENT";
    }
    if (o == ::android::hardware::radio::V1_0::SmsWriteArgsStatus::STO_SENT) {
        return "STO_SENT";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioBandMode lhs, const ::android::hardware::radio::V1_0::RadioBandMode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioBandMode rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioBandMode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioBandMode lhs, const ::android::hardware::radio::V1_0::RadioBandMode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioBandMode rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioBandMode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::RadioBandMode e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::RadioBandMode e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::RadioBandMode>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::RadioBandMode> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_UNSPECIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_UNSPECIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_UNSPECIFIED;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_EURO) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_EURO)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_EURO";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_EURO;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_USA) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_USA)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_USA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_USA;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_JPN) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_JPN)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_JPN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_JPN;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_AUS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_AUS)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_AUS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_AUS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_AUS_2) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_AUS_2)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_AUS_2";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_AUS_2;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_CELL_800) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_CELL_800)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_CELL_800";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_CELL_800;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_PCS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_PCS)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_PCS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_PCS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_JTACS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_JTACS)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_JTACS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_JTACS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_KOREA_PCS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_KOREA_PCS)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_KOREA_PCS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_KOREA_PCS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_5_450M) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_5_450M)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_5_450M";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_5_450M;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_IMT2000) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_IMT2000)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_IMT2000";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_IMT2000;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_7_700M_2) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_7_700M_2)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_7_700M_2";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_7_700M_2;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_8_1800M) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_8_1800M)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_8_1800M";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_8_1800M;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_9_900M) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_9_900M)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_9_900M";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_9_900M;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_10_800M_2) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_10_800M_2)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_10_800M_2";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_10_800M_2;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_EURO_PAMR_400M) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_EURO_PAMR_400M)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_EURO_PAMR_400M";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_EURO_PAMR_400M;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_AWS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_AWS)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_AWS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_AWS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_USA_2500M) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_USA_2500M)) {
        os += (first ? "" : " | ");
        os += "BAND_MODE_USA_2500M";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_USA_2500M;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::RadioBandMode o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_UNSPECIFIED) {
        return "BAND_MODE_UNSPECIFIED";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_EURO) {
        return "BAND_MODE_EURO";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_USA) {
        return "BAND_MODE_USA";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_JPN) {
        return "BAND_MODE_JPN";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_AUS) {
        return "BAND_MODE_AUS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_AUS_2) {
        return "BAND_MODE_AUS_2";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_CELL_800) {
        return "BAND_MODE_CELL_800";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_PCS) {
        return "BAND_MODE_PCS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_JTACS) {
        return "BAND_MODE_JTACS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_KOREA_PCS) {
        return "BAND_MODE_KOREA_PCS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_5_450M) {
        return "BAND_MODE_5_450M";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_IMT2000) {
        return "BAND_MODE_IMT2000";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_7_700M_2) {
        return "BAND_MODE_7_700M_2";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_8_1800M) {
        return "BAND_MODE_8_1800M";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_9_900M) {
        return "BAND_MODE_9_900M";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_10_800M_2) {
        return "BAND_MODE_10_800M_2";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_EURO_PAMR_400M) {
        return "BAND_MODE_EURO_PAMR_400M";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_AWS) {
        return "BAND_MODE_AWS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_USA_2500M) {
        return "BAND_MODE_USA_2500M";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::OperatorStatus lhs, const ::android::hardware::radio::V1_0::OperatorStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::OperatorStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::OperatorStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::OperatorStatus lhs, const ::android::hardware::radio::V1_0::OperatorStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::OperatorStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::OperatorStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::OperatorStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::OperatorStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::OperatorStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::OperatorStatus> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::OperatorStatus::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::OperatorStatus::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::OperatorStatus::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_0::OperatorStatus::AVAILABLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::OperatorStatus::AVAILABLE)) {
        os += (first ? "" : " | ");
        os += "AVAILABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::OperatorStatus::AVAILABLE;
    }
    if ((o & ::android::hardware::radio::V1_0::OperatorStatus::CURRENT) == static_cast<int32_t>(::android::hardware::radio::V1_0::OperatorStatus::CURRENT)) {
        os += (first ? "" : " | ");
        os += "CURRENT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::OperatorStatus::CURRENT;
    }
    if ((o & ::android::hardware::radio::V1_0::OperatorStatus::FORBIDDEN) == static_cast<int32_t>(::android::hardware::radio::V1_0::OperatorStatus::FORBIDDEN)) {
        os += (first ? "" : " | ");
        os += "FORBIDDEN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::OperatorStatus::FORBIDDEN;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::OperatorStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::OperatorStatus::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_0::OperatorStatus::AVAILABLE) {
        return "AVAILABLE";
    }
    if (o == ::android::hardware::radio::V1_0::OperatorStatus::CURRENT) {
        return "CURRENT";
    }
    if (o == ::android::hardware::radio::V1_0::OperatorStatus::FORBIDDEN) {
        return "FORBIDDEN";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::PreferredNetworkType lhs, const ::android::hardware::radio::V1_0::PreferredNetworkType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::PreferredNetworkType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::PreferredNetworkType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::PreferredNetworkType lhs, const ::android::hardware::radio::V1_0::PreferredNetworkType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::PreferredNetworkType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::PreferredNetworkType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::PreferredNetworkType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::PreferredNetworkType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::PreferredNetworkType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::PreferredNetworkType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::GSM_WCDMA) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::GSM_WCDMA)) {
        os += (first ? "" : " | ");
        os += "GSM_WCDMA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::GSM_WCDMA;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::GSM_ONLY) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::GSM_ONLY)) {
        os += (first ? "" : " | ");
        os += "GSM_ONLY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::GSM_ONLY;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::WCDMA) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::WCDMA)) {
        os += (first ? "" : " | ");
        os += "WCDMA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::WCDMA;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::GSM_WCDMA_AUTO) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::GSM_WCDMA_AUTO)) {
        os += (first ? "" : " | ");
        os += "GSM_WCDMA_AUTO";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::GSM_WCDMA_AUTO;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::CDMA_EVDO_AUTO) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::CDMA_EVDO_AUTO)) {
        os += (first ? "" : " | ");
        os += "CDMA_EVDO_AUTO";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::CDMA_EVDO_AUTO;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::CDMA_ONLY) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::CDMA_ONLY)) {
        os += (first ? "" : " | ");
        os += "CDMA_ONLY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::CDMA_ONLY;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::EVDO_ONLY) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::EVDO_ONLY)) {
        os += (first ? "" : " | ");
        os += "EVDO_ONLY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::EVDO_ONLY;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::GSM_WCDMA_CDMA_EVDO_AUTO) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::GSM_WCDMA_CDMA_EVDO_AUTO)) {
        os += (first ? "" : " | ");
        os += "GSM_WCDMA_CDMA_EVDO_AUTO";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::GSM_WCDMA_CDMA_EVDO_AUTO;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_CDMA_EVDO) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::LTE_CDMA_EVDO)) {
        os += (first ? "" : " | ");
        os += "LTE_CDMA_EVDO";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_CDMA_EVDO;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_GSM_WCDMA) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::LTE_GSM_WCDMA)) {
        os += (first ? "" : " | ");
        os += "LTE_GSM_WCDMA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_GSM_WCDMA;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_CMDA_EVDO_GSM_WCDMA) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::LTE_CMDA_EVDO_GSM_WCDMA)) {
        os += (first ? "" : " | ");
        os += "LTE_CMDA_EVDO_GSM_WCDMA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_CMDA_EVDO_GSM_WCDMA;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_ONLY) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::LTE_ONLY)) {
        os += (first ? "" : " | ");
        os += "LTE_ONLY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_ONLY;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_WCDMA) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::LTE_WCDMA)) {
        os += (first ? "" : " | ");
        os += "LTE_WCDMA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_WCDMA;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_ONLY) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_ONLY)) {
        os += (first ? "" : " | ");
        os += "TD_SCDMA_ONLY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_ONLY;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_WCDMA) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_WCDMA)) {
        os += (first ? "" : " | ");
        os += "TD_SCDMA_WCDMA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_WCDMA;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_LTE) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_LTE)) {
        os += (first ? "" : " | ");
        os += "TD_SCDMA_LTE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_LTE;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM)) {
        os += (first ? "" : " | ");
        os += "TD_SCDMA_GSM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_LTE) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_LTE)) {
        os += (first ? "" : " | ");
        os += "TD_SCDMA_GSM_LTE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_LTE;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA)) {
        os += (first ? "" : " | ");
        os += "TD_SCDMA_GSM_WCDMA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_WCDMA_LTE) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_WCDMA_LTE)) {
        os += (first ? "" : " | ");
        os += "TD_SCDMA_WCDMA_LTE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_WCDMA_LTE;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA_LTE) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA_LTE)) {
        os += (first ? "" : " | ");
        os += "TD_SCDMA_GSM_WCDMA_LTE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA_LTE;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO)) {
        os += (first ? "" : " | ");
        os += "TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO;
    }
    if ((o & ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA) == static_cast<int32_t>(::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA)) {
        os += (first ? "" : " | ");
        os += "TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::PreferredNetworkType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::GSM_WCDMA) {
        return "GSM_WCDMA";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::GSM_ONLY) {
        return "GSM_ONLY";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::WCDMA) {
        return "WCDMA";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::GSM_WCDMA_AUTO) {
        return "GSM_WCDMA_AUTO";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::CDMA_EVDO_AUTO) {
        return "CDMA_EVDO_AUTO";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::CDMA_ONLY) {
        return "CDMA_ONLY";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::EVDO_ONLY) {
        return "EVDO_ONLY";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::GSM_WCDMA_CDMA_EVDO_AUTO) {
        return "GSM_WCDMA_CDMA_EVDO_AUTO";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_CDMA_EVDO) {
        return "LTE_CDMA_EVDO";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_GSM_WCDMA) {
        return "LTE_GSM_WCDMA";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_CMDA_EVDO_GSM_WCDMA) {
        return "LTE_CMDA_EVDO_GSM_WCDMA";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_ONLY) {
        return "LTE_ONLY";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_WCDMA) {
        return "LTE_WCDMA";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_ONLY) {
        return "TD_SCDMA_ONLY";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_WCDMA) {
        return "TD_SCDMA_WCDMA";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_LTE) {
        return "TD_SCDMA_LTE";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM) {
        return "TD_SCDMA_GSM";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_LTE) {
        return "TD_SCDMA_GSM_LTE";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA) {
        return "TD_SCDMA_GSM_WCDMA";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_WCDMA_LTE) {
        return "TD_SCDMA_WCDMA_LTE";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA_LTE) {
        return "TD_SCDMA_GSM_WCDMA_LTE";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO) {
        return "TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO";
    }
    if (o == ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA) {
        return "TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaSubscriptionSource lhs, const ::android::hardware::radio::V1_0::CdmaSubscriptionSource rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaSubscriptionSource rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaSubscriptionSource lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaSubscriptionSource lhs, const ::android::hardware::radio::V1_0::CdmaSubscriptionSource rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaSubscriptionSource rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaSubscriptionSource lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaSubscriptionSource e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaSubscriptionSource e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CdmaSubscriptionSource>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CdmaSubscriptionSource> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CdmaSubscriptionSource::RUIM_SIM) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSubscriptionSource::RUIM_SIM)) {
        os += (first ? "" : " | ");
        os += "RUIM_SIM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSubscriptionSource::RUIM_SIM;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSubscriptionSource::NV) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSubscriptionSource::NV)) {
        os += (first ? "" : " | ");
        os += "NV";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSubscriptionSource::NV;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CdmaSubscriptionSource o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CdmaSubscriptionSource::RUIM_SIM) {
        return "RUIM_SIM";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSubscriptionSource::NV) {
        return "NV";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaRoamingType lhs, const ::android::hardware::radio::V1_0::CdmaRoamingType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaRoamingType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaRoamingType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaRoamingType lhs, const ::android::hardware::radio::V1_0::CdmaRoamingType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaRoamingType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaRoamingType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaRoamingType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaRoamingType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CdmaRoamingType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CdmaRoamingType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CdmaRoamingType::HOME_NETWORK) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaRoamingType::HOME_NETWORK)) {
        os += (first ? "" : " | ");
        os += "HOME_NETWORK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaRoamingType::HOME_NETWORK;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaRoamingType::AFFILIATED_ROAM) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaRoamingType::AFFILIATED_ROAM)) {
        os += (first ? "" : " | ");
        os += "AFFILIATED_ROAM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaRoamingType::AFFILIATED_ROAM;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaRoamingType::ANY_ROAM) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaRoamingType::ANY_ROAM)) {
        os += (first ? "" : " | ");
        os += "ANY_ROAM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaRoamingType::ANY_ROAM;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CdmaRoamingType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CdmaRoamingType::HOME_NETWORK) {
        return "HOME_NETWORK";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaRoamingType::AFFILIATED_ROAM) {
        return "AFFILIATED_ROAM";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaRoamingType::ANY_ROAM) {
        return "ANY_ROAM";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::TtyMode lhs, const ::android::hardware::radio::V1_0::TtyMode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::TtyMode rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::TtyMode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::TtyMode lhs, const ::android::hardware::radio::V1_0::TtyMode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::TtyMode rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::TtyMode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::TtyMode e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::TtyMode e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::TtyMode>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::TtyMode> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::TtyMode::OFF) == static_cast<int32_t>(::android::hardware::radio::V1_0::TtyMode::OFF)) {
        os += (first ? "" : " | ");
        os += "OFF";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::TtyMode::OFF;
    }
    if ((o & ::android::hardware::radio::V1_0::TtyMode::FULL) == static_cast<int32_t>(::android::hardware::radio::V1_0::TtyMode::FULL)) {
        os += (first ? "" : " | ");
        os += "FULL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::TtyMode::FULL;
    }
    if ((o & ::android::hardware::radio::V1_0::TtyMode::HCO) == static_cast<int32_t>(::android::hardware::radio::V1_0::TtyMode::HCO)) {
        os += (first ? "" : " | ");
        os += "HCO";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::TtyMode::HCO;
    }
    if ((o & ::android::hardware::radio::V1_0::TtyMode::VCO) == static_cast<int32_t>(::android::hardware::radio::V1_0::TtyMode::VCO)) {
        os += (first ? "" : " | ");
        os += "VCO";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::TtyMode::VCO;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::TtyMode o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::TtyMode::OFF) {
        return "OFF";
    }
    if (o == ::android::hardware::radio::V1_0::TtyMode::FULL) {
        return "FULL";
    }
    if (o == ::android::hardware::radio::V1_0::TtyMode::HCO) {
        return "HCO";
    }
    if (o == ::android::hardware::radio::V1_0::TtyMode::VCO) {
        return "VCO";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::NvItem lhs, const ::android::hardware::radio::V1_0::NvItem rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::NvItem rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::NvItem lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::NvItem lhs, const ::android::hardware::radio::V1_0::NvItem rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::NvItem rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::NvItem lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::NvItem e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::NvItem e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::NvItem>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::NvItem> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::NvItem::CDMA_MEID) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::CDMA_MEID)) {
        os += (first ? "" : " | ");
        os += "CDMA_MEID";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::CDMA_MEID;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::CDMA_MIN) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::CDMA_MIN)) {
        os += (first ? "" : " | ");
        os += "CDMA_MIN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::CDMA_MIN;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::CDMA_MDN) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::CDMA_MDN)) {
        os += (first ? "" : " | ");
        os += "CDMA_MDN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::CDMA_MDN;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::CDMA_ACCOLC) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::CDMA_ACCOLC)) {
        os += (first ? "" : " | ");
        os += "CDMA_ACCOLC";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::CDMA_ACCOLC;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::DEVICE_MSL) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::DEVICE_MSL)) {
        os += (first ? "" : " | ");
        os += "DEVICE_MSL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::DEVICE_MSL;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::RTN_RECONDITIONED_STATUS) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::RTN_RECONDITIONED_STATUS)) {
        os += (first ? "" : " | ");
        os += "RTN_RECONDITIONED_STATUS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::RTN_RECONDITIONED_STATUS;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::RTN_ACTIVATION_DATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::RTN_ACTIVATION_DATE)) {
        os += (first ? "" : " | ");
        os += "RTN_ACTIVATION_DATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::RTN_ACTIVATION_DATE;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::RTN_LIFE_TIMER) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::RTN_LIFE_TIMER)) {
        os += (first ? "" : " | ");
        os += "RTN_LIFE_TIMER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::RTN_LIFE_TIMER;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::RTN_LIFE_CALLS) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::RTN_LIFE_CALLS)) {
        os += (first ? "" : " | ");
        os += "RTN_LIFE_CALLS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::RTN_LIFE_CALLS;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::RTN_LIFE_DATA_TX) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::RTN_LIFE_DATA_TX)) {
        os += (first ? "" : " | ");
        os += "RTN_LIFE_DATA_TX";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::RTN_LIFE_DATA_TX;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::RTN_LIFE_DATA_RX) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::RTN_LIFE_DATA_RX)) {
        os += (first ? "" : " | ");
        os += "RTN_LIFE_DATA_RX";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::RTN_LIFE_DATA_RX;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::OMADM_HFA_LEVEL) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::OMADM_HFA_LEVEL)) {
        os += (first ? "" : " | ");
        os += "OMADM_HFA_LEVEL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::OMADM_HFA_LEVEL;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_NAI) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_NAI)) {
        os += (first ? "" : " | ");
        os += "MIP_PROFILE_NAI";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_NAI;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_HOME_ADDRESS) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_HOME_ADDRESS)) {
        os += (first ? "" : " | ");
        os += "MIP_PROFILE_HOME_ADDRESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_HOME_ADDRESS;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_AAA_AUTH) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_AAA_AUTH)) {
        os += (first ? "" : " | ");
        os += "MIP_PROFILE_AAA_AUTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_AAA_AUTH;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_HA_AUTH) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_HA_AUTH)) {
        os += (first ? "" : " | ");
        os += "MIP_PROFILE_HA_AUTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_HA_AUTH;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_PRI_HA_ADDR) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_PRI_HA_ADDR)) {
        os += (first ? "" : " | ");
        os += "MIP_PROFILE_PRI_HA_ADDR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_PRI_HA_ADDR;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_SEC_HA_ADDR) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_SEC_HA_ADDR)) {
        os += (first ? "" : " | ");
        os += "MIP_PROFILE_SEC_HA_ADDR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_SEC_HA_ADDR;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_REV_TUN_PREF) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_REV_TUN_PREF)) {
        os += (first ? "" : " | ");
        os += "MIP_PROFILE_REV_TUN_PREF";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_REV_TUN_PREF;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_HA_SPI) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_HA_SPI)) {
        os += (first ? "" : " | ");
        os += "MIP_PROFILE_HA_SPI";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_HA_SPI;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_AAA_SPI) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_AAA_SPI)) {
        os += (first ? "" : " | ");
        os += "MIP_PROFILE_AAA_SPI";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_AAA_SPI;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_MN_HA_SS) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_MN_HA_SS)) {
        os += (first ? "" : " | ");
        os += "MIP_PROFILE_MN_HA_SS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_MN_HA_SS;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_MN_AAA_SS) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_MN_AAA_SS)) {
        os += (first ? "" : " | ");
        os += "MIP_PROFILE_MN_AAA_SS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_MN_AAA_SS;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::CDMA_PRL_VERSION) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::CDMA_PRL_VERSION)) {
        os += (first ? "" : " | ");
        os += "CDMA_PRL_VERSION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::CDMA_PRL_VERSION;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::CDMA_BC10) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::CDMA_BC10)) {
        os += (first ? "" : " | ");
        os += "CDMA_BC10";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::CDMA_BC10;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::CDMA_BC14) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::CDMA_BC14)) {
        os += (first ? "" : " | ");
        os += "CDMA_BC14";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::CDMA_BC14;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::CDMA_SO68) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::CDMA_SO68)) {
        os += (first ? "" : " | ");
        os += "CDMA_SO68";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::CDMA_SO68;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::CDMA_SO73_COP0) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::CDMA_SO73_COP0)) {
        os += (first ? "" : " | ");
        os += "CDMA_SO73_COP0";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::CDMA_SO73_COP0;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::CDMA_SO73_COP1TO7) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::CDMA_SO73_COP1TO7)) {
        os += (first ? "" : " | ");
        os += "CDMA_SO73_COP1TO7";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::CDMA_SO73_COP1TO7;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::CDMA_1X_ADVANCED_ENABLED) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::CDMA_1X_ADVANCED_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CDMA_1X_ADVANCED_ENABLED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::CDMA_1X_ADVANCED_ENABLED;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::CDMA_EHRPD_ENABLED) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::CDMA_EHRPD_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CDMA_EHRPD_ENABLED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::CDMA_EHRPD_ENABLED;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::CDMA_EHRPD_FORCED) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::CDMA_EHRPD_FORCED)) {
        os += (first ? "" : " | ");
        os += "CDMA_EHRPD_FORCED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::CDMA_EHRPD_FORCED;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::LTE_BAND_ENABLE_25) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::LTE_BAND_ENABLE_25)) {
        os += (first ? "" : " | ");
        os += "LTE_BAND_ENABLE_25";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::LTE_BAND_ENABLE_25;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::LTE_BAND_ENABLE_26) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::LTE_BAND_ENABLE_26)) {
        os += (first ? "" : " | ");
        os += "LTE_BAND_ENABLE_26";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::LTE_BAND_ENABLE_26;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::LTE_BAND_ENABLE_41) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::LTE_BAND_ENABLE_41)) {
        os += (first ? "" : " | ");
        os += "LTE_BAND_ENABLE_41";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::LTE_BAND_ENABLE_41;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::LTE_SCAN_PRIORITY_25) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::LTE_SCAN_PRIORITY_25)) {
        os += (first ? "" : " | ");
        os += "LTE_SCAN_PRIORITY_25";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::LTE_SCAN_PRIORITY_25;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::LTE_SCAN_PRIORITY_26) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::LTE_SCAN_PRIORITY_26)) {
        os += (first ? "" : " | ");
        os += "LTE_SCAN_PRIORITY_26";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::LTE_SCAN_PRIORITY_26;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::LTE_SCAN_PRIORITY_41) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::LTE_SCAN_PRIORITY_41)) {
        os += (first ? "" : " | ");
        os += "LTE_SCAN_PRIORITY_41";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::LTE_SCAN_PRIORITY_41;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::LTE_HIDDEN_BAND_PRIORITY_25) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::LTE_HIDDEN_BAND_PRIORITY_25)) {
        os += (first ? "" : " | ");
        os += "LTE_HIDDEN_BAND_PRIORITY_25";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::LTE_HIDDEN_BAND_PRIORITY_25;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::LTE_HIDDEN_BAND_PRIORITY_26) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::LTE_HIDDEN_BAND_PRIORITY_26)) {
        os += (first ? "" : " | ");
        os += "LTE_HIDDEN_BAND_PRIORITY_26";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::LTE_HIDDEN_BAND_PRIORITY_26;
    }
    if ((o & ::android::hardware::radio::V1_0::NvItem::LTE_HIDDEN_BAND_PRIORITY_41) == static_cast<int32_t>(::android::hardware::radio::V1_0::NvItem::LTE_HIDDEN_BAND_PRIORITY_41)) {
        os += (first ? "" : " | ");
        os += "LTE_HIDDEN_BAND_PRIORITY_41";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::NvItem::LTE_HIDDEN_BAND_PRIORITY_41;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::NvItem o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::NvItem::CDMA_MEID) {
        return "CDMA_MEID";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::CDMA_MIN) {
        return "CDMA_MIN";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::CDMA_MDN) {
        return "CDMA_MDN";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::CDMA_ACCOLC) {
        return "CDMA_ACCOLC";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::DEVICE_MSL) {
        return "DEVICE_MSL";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::RTN_RECONDITIONED_STATUS) {
        return "RTN_RECONDITIONED_STATUS";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::RTN_ACTIVATION_DATE) {
        return "RTN_ACTIVATION_DATE";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::RTN_LIFE_TIMER) {
        return "RTN_LIFE_TIMER";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::RTN_LIFE_CALLS) {
        return "RTN_LIFE_CALLS";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::RTN_LIFE_DATA_TX) {
        return "RTN_LIFE_DATA_TX";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::RTN_LIFE_DATA_RX) {
        return "RTN_LIFE_DATA_RX";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::OMADM_HFA_LEVEL) {
        return "OMADM_HFA_LEVEL";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_NAI) {
        return "MIP_PROFILE_NAI";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_HOME_ADDRESS) {
        return "MIP_PROFILE_HOME_ADDRESS";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_AAA_AUTH) {
        return "MIP_PROFILE_AAA_AUTH";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_HA_AUTH) {
        return "MIP_PROFILE_HA_AUTH";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_PRI_HA_ADDR) {
        return "MIP_PROFILE_PRI_HA_ADDR";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_SEC_HA_ADDR) {
        return "MIP_PROFILE_SEC_HA_ADDR";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_REV_TUN_PREF) {
        return "MIP_PROFILE_REV_TUN_PREF";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_HA_SPI) {
        return "MIP_PROFILE_HA_SPI";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_AAA_SPI) {
        return "MIP_PROFILE_AAA_SPI";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_MN_HA_SS) {
        return "MIP_PROFILE_MN_HA_SS";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_MN_AAA_SS) {
        return "MIP_PROFILE_MN_AAA_SS";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::CDMA_PRL_VERSION) {
        return "CDMA_PRL_VERSION";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::CDMA_BC10) {
        return "CDMA_BC10";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::CDMA_BC14) {
        return "CDMA_BC14";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::CDMA_SO68) {
        return "CDMA_SO68";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::CDMA_SO73_COP0) {
        return "CDMA_SO73_COP0";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::CDMA_SO73_COP1TO7) {
        return "CDMA_SO73_COP1TO7";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::CDMA_1X_ADVANCED_ENABLED) {
        return "CDMA_1X_ADVANCED_ENABLED";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::CDMA_EHRPD_ENABLED) {
        return "CDMA_EHRPD_ENABLED";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::CDMA_EHRPD_FORCED) {
        return "CDMA_EHRPD_FORCED";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::LTE_BAND_ENABLE_25) {
        return "LTE_BAND_ENABLE_25";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::LTE_BAND_ENABLE_26) {
        return "LTE_BAND_ENABLE_26";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::LTE_BAND_ENABLE_41) {
        return "LTE_BAND_ENABLE_41";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::LTE_SCAN_PRIORITY_25) {
        return "LTE_SCAN_PRIORITY_25";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::LTE_SCAN_PRIORITY_26) {
        return "LTE_SCAN_PRIORITY_26";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::LTE_SCAN_PRIORITY_41) {
        return "LTE_SCAN_PRIORITY_41";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::LTE_HIDDEN_BAND_PRIORITY_25) {
        return "LTE_HIDDEN_BAND_PRIORITY_25";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::LTE_HIDDEN_BAND_PRIORITY_26) {
        return "LTE_HIDDEN_BAND_PRIORITY_26";
    }
    if (o == ::android::hardware::radio::V1_0::NvItem::LTE_HIDDEN_BAND_PRIORITY_41) {
        return "LTE_HIDDEN_BAND_PRIORITY_41";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::ResetNvType lhs, const ::android::hardware::radio::V1_0::ResetNvType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::ResetNvType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::ResetNvType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::ResetNvType lhs, const ::android::hardware::radio::V1_0::ResetNvType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::ResetNvType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::ResetNvType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::ResetNvType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::ResetNvType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::ResetNvType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::ResetNvType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::ResetNvType::RELOAD) == static_cast<int32_t>(::android::hardware::radio::V1_0::ResetNvType::RELOAD)) {
        os += (first ? "" : " | ");
        os += "RELOAD";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ResetNvType::RELOAD;
    }
    if ((o & ::android::hardware::radio::V1_0::ResetNvType::ERASE) == static_cast<int32_t>(::android::hardware::radio::V1_0::ResetNvType::ERASE)) {
        os += (first ? "" : " | ");
        os += "ERASE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ResetNvType::ERASE;
    }
    if ((o & ::android::hardware::radio::V1_0::ResetNvType::FACTORY_RESET) == static_cast<int32_t>(::android::hardware::radio::V1_0::ResetNvType::FACTORY_RESET)) {
        os += (first ? "" : " | ");
        os += "FACTORY_RESET";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ResetNvType::FACTORY_RESET;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::ResetNvType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::ResetNvType::RELOAD) {
        return "RELOAD";
    }
    if (o == ::android::hardware::radio::V1_0::ResetNvType::ERASE) {
        return "ERASE";
    }
    if (o == ::android::hardware::radio::V1_0::ResetNvType::FACTORY_RESET) {
        return "FACTORY_RESET";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::HardwareConfigType lhs, const ::android::hardware::radio::V1_0::HardwareConfigType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::HardwareConfigType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::HardwareConfigType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::HardwareConfigType lhs, const ::android::hardware::radio::V1_0::HardwareConfigType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::HardwareConfigType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::HardwareConfigType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::HardwareConfigType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::HardwareConfigType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::HardwareConfigType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::HardwareConfigType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::HardwareConfigType::MODEM) == static_cast<int32_t>(::android::hardware::radio::V1_0::HardwareConfigType::MODEM)) {
        os += (first ? "" : " | ");
        os += "MODEM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::HardwareConfigType::MODEM;
    }
    if ((o & ::android::hardware::radio::V1_0::HardwareConfigType::SIM) == static_cast<int32_t>(::android::hardware::radio::V1_0::HardwareConfigType::SIM)) {
        os += (first ? "" : " | ");
        os += "SIM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::HardwareConfigType::SIM;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::HardwareConfigType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::HardwareConfigType::MODEM) {
        return "MODEM";
    }
    if (o == ::android::hardware::radio::V1_0::HardwareConfigType::SIM) {
        return "SIM";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::HardwareConfigState lhs, const ::android::hardware::radio::V1_0::HardwareConfigState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::HardwareConfigState rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::HardwareConfigState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::HardwareConfigState lhs, const ::android::hardware::radio::V1_0::HardwareConfigState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::HardwareConfigState rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::HardwareConfigState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::HardwareConfigState e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::HardwareConfigState e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::HardwareConfigState>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::HardwareConfigState> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::HardwareConfigState::ENABLED) == static_cast<int32_t>(::android::hardware::radio::V1_0::HardwareConfigState::ENABLED)) {
        os += (first ? "" : " | ");
        os += "ENABLED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::HardwareConfigState::ENABLED;
    }
    if ((o & ::android::hardware::radio::V1_0::HardwareConfigState::STANDBY) == static_cast<int32_t>(::android::hardware::radio::V1_0::HardwareConfigState::STANDBY)) {
        os += (first ? "" : " | ");
        os += "STANDBY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::HardwareConfigState::STANDBY;
    }
    if ((o & ::android::hardware::radio::V1_0::HardwareConfigState::DISABLED) == static_cast<int32_t>(::android::hardware::radio::V1_0::HardwareConfigState::DISABLED)) {
        os += (first ? "" : " | ");
        os += "DISABLED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::HardwareConfigState::DISABLED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::HardwareConfigState o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::HardwareConfigState::ENABLED) {
        return "ENABLED";
    }
    if (o == ::android::hardware::radio::V1_0::HardwareConfigState::STANDBY) {
        return "STANDBY";
    }
    if (o == ::android::hardware::radio::V1_0::HardwareConfigState::DISABLED) {
        return "DISABLED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::LceStatus lhs, const ::android::hardware::radio::V1_0::LceStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::LceStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::LceStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::LceStatus lhs, const ::android::hardware::radio::V1_0::LceStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::LceStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::LceStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::LceStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::LceStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::LceStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::LceStatus> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::LceStatus::NOT_SUPPORTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LceStatus::NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "NOT_SUPPORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LceStatus::NOT_SUPPORTED;
    }
    if ((o & ::android::hardware::radio::V1_0::LceStatus::STOPPED) == static_cast<int32_t>(::android::hardware::radio::V1_0::LceStatus::STOPPED)) {
        os += (first ? "" : " | ");
        os += "STOPPED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LceStatus::STOPPED;
    }
    if ((o & ::android::hardware::radio::V1_0::LceStatus::ACTIVE) == static_cast<int32_t>(::android::hardware::radio::V1_0::LceStatus::ACTIVE)) {
        os += (first ? "" : " | ");
        os += "ACTIVE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::LceStatus::ACTIVE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::LceStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::LceStatus::NOT_SUPPORTED) {
        return "NOT_SUPPORTED";
    }
    if (o == ::android::hardware::radio::V1_0::LceStatus::STOPPED) {
        return "STOPPED";
    }
    if (o == ::android::hardware::radio::V1_0::LceStatus::ACTIVE) {
        return "ACTIVE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CarrierMatchType lhs, const ::android::hardware::radio::V1_0::CarrierMatchType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CarrierMatchType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CarrierMatchType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CarrierMatchType lhs, const ::android::hardware::radio::V1_0::CarrierMatchType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CarrierMatchType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CarrierMatchType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CarrierMatchType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CarrierMatchType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CarrierMatchType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CarrierMatchType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CarrierMatchType::ALL) == static_cast<int32_t>(::android::hardware::radio::V1_0::CarrierMatchType::ALL)) {
        os += (first ? "" : " | ");
        os += "ALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CarrierMatchType::ALL;
    }
    if ((o & ::android::hardware::radio::V1_0::CarrierMatchType::SPN) == static_cast<int32_t>(::android::hardware::radio::V1_0::CarrierMatchType::SPN)) {
        os += (first ? "" : " | ");
        os += "SPN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CarrierMatchType::SPN;
    }
    if ((o & ::android::hardware::radio::V1_0::CarrierMatchType::IMSI_PREFIX) == static_cast<int32_t>(::android::hardware::radio::V1_0::CarrierMatchType::IMSI_PREFIX)) {
        os += (first ? "" : " | ");
        os += "IMSI_PREFIX";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CarrierMatchType::IMSI_PREFIX;
    }
    if ((o & ::android::hardware::radio::V1_0::CarrierMatchType::GID1) == static_cast<int32_t>(::android::hardware::radio::V1_0::CarrierMatchType::GID1)) {
        os += (first ? "" : " | ");
        os += "GID1";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CarrierMatchType::GID1;
    }
    if ((o & ::android::hardware::radio::V1_0::CarrierMatchType::GID2) == static_cast<int32_t>(::android::hardware::radio::V1_0::CarrierMatchType::GID2)) {
        os += (first ? "" : " | ");
        os += "GID2";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CarrierMatchType::GID2;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CarrierMatchType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CarrierMatchType::ALL) {
        return "ALL";
    }
    if (o == ::android::hardware::radio::V1_0::CarrierMatchType::SPN) {
        return "SPN";
    }
    if (o == ::android::hardware::radio::V1_0::CarrierMatchType::IMSI_PREFIX) {
        return "IMSI_PREFIX";
    }
    if (o == ::android::hardware::radio::V1_0::CarrierMatchType::GID1) {
        return "GID1";
    }
    if (o == ::android::hardware::radio::V1_0::CarrierMatchType::GID2) {
        return "GID2";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::android::hardware::radio::V1_0::NeighboringCell& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cid = ";
    os += ::android::hardware::toString(o.cid);
    os += ", .rssi = ";
    os += ::android::hardware::toString(o.rssi);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::NeighboringCell& lhs, const ::android::hardware::radio::V1_0::NeighboringCell& rhs) {
    if (lhs.cid != rhs.cid) {
        return false;
    }
    if (lhs.rssi != rhs.rssi) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::NeighboringCell& lhs,const ::android::hardware::radio::V1_0::NeighboringCell& rhs){
    return !(lhs == rhs);
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaSmsDigitMode lhs, const ::android::hardware::radio::V1_0::CdmaSmsDigitMode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaSmsDigitMode rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaSmsDigitMode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaSmsDigitMode lhs, const ::android::hardware::radio::V1_0::CdmaSmsDigitMode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaSmsDigitMode rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaSmsDigitMode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaSmsDigitMode e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaSmsDigitMode e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CdmaSmsDigitMode>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CdmaSmsDigitMode> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsDigitMode::FOUR_BIT) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsDigitMode::FOUR_BIT)) {
        os += (first ? "" : " | ");
        os += "FOUR_BIT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsDigitMode::FOUR_BIT;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsDigitMode::EIGHT_BIT) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsDigitMode::EIGHT_BIT)) {
        os += (first ? "" : " | ");
        os += "EIGHT_BIT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsDigitMode::EIGHT_BIT;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CdmaSmsDigitMode o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CdmaSmsDigitMode::FOUR_BIT) {
        return "FOUR_BIT";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsDigitMode::EIGHT_BIT) {
        return "EIGHT_BIT";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaSmsNumberMode lhs, const ::android::hardware::radio::V1_0::CdmaSmsNumberMode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaSmsNumberMode rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaSmsNumberMode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaSmsNumberMode lhs, const ::android::hardware::radio::V1_0::CdmaSmsNumberMode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaSmsNumberMode rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaSmsNumberMode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaSmsNumberMode e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaSmsNumberMode e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CdmaSmsNumberMode>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CdmaSmsNumberMode> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberMode::NOT_DATA_NETWORK) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberMode::NOT_DATA_NETWORK)) {
        os += (first ? "" : " | ");
        os += "NOT_DATA_NETWORK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberMode::NOT_DATA_NETWORK;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberMode::DATA_NETWORK) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberMode::DATA_NETWORK)) {
        os += (first ? "" : " | ");
        os += "DATA_NETWORK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberMode::DATA_NETWORK;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CdmaSmsNumberMode o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberMode::NOT_DATA_NETWORK) {
        return "NOT_DATA_NETWORK";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberMode::DATA_NETWORK) {
        return "DATA_NETWORK";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaSmsNumberType lhs, const ::android::hardware::radio::V1_0::CdmaSmsNumberType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaSmsNumberType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaSmsNumberType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaSmsNumberType lhs, const ::android::hardware::radio::V1_0::CdmaSmsNumberType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaSmsNumberType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaSmsNumberType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaSmsNumberType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaSmsNumberType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CdmaSmsNumberType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CdmaSmsNumberType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberType::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberType::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberType::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberType::INTERNATIONAL_OR_DATA_IP) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberType::INTERNATIONAL_OR_DATA_IP)) {
        os += (first ? "" : " | ");
        os += "INTERNATIONAL_OR_DATA_IP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberType::INTERNATIONAL_OR_DATA_IP;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberType::NATIONAL_OR_INTERNET_MAIL) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberType::NATIONAL_OR_INTERNET_MAIL)) {
        os += (first ? "" : " | ");
        os += "NATIONAL_OR_INTERNET_MAIL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberType::NATIONAL_OR_INTERNET_MAIL;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberType::NETWORK) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberType::NETWORK)) {
        os += (first ? "" : " | ");
        os += "NETWORK";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberType::NETWORK;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberType::SUBSCRIBER) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberType::SUBSCRIBER)) {
        os += (first ? "" : " | ");
        os += "SUBSCRIBER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberType::SUBSCRIBER;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberType::ALPHANUMERIC) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberType::ALPHANUMERIC)) {
        os += (first ? "" : " | ");
        os += "ALPHANUMERIC";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberType::ALPHANUMERIC;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberType::ABBREVIATED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberType::ABBREVIATED)) {
        os += (first ? "" : " | ");
        os += "ABBREVIATED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberType::ABBREVIATED;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberType::RESERVED_7) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberType::RESERVED_7)) {
        os += (first ? "" : " | ");
        os += "RESERVED_7";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberType::RESERVED_7;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CdmaSmsNumberType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberType::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberType::INTERNATIONAL_OR_DATA_IP) {
        return "INTERNATIONAL_OR_DATA_IP";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberType::NATIONAL_OR_INTERNET_MAIL) {
        return "NATIONAL_OR_INTERNET_MAIL";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberType::NETWORK) {
        return "NETWORK";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberType::SUBSCRIBER) {
        return "SUBSCRIBER";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberType::ALPHANUMERIC) {
        return "ALPHANUMERIC";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberType::ABBREVIATED) {
        return "ABBREVIATED";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberType::RESERVED_7) {
        return "RESERVED_7";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaSmsNumberPlan lhs, const ::android::hardware::radio::V1_0::CdmaSmsNumberPlan rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaSmsNumberPlan rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaSmsNumberPlan lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaSmsNumberPlan lhs, const ::android::hardware::radio::V1_0::CdmaSmsNumberPlan rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaSmsNumberPlan rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaSmsNumberPlan lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaSmsNumberPlan e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaSmsNumberPlan e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CdmaSmsNumberPlan>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CdmaSmsNumberPlan> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberPlan::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::TELEPHONY) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberPlan::TELEPHONY)) {
        os += (first ? "" : " | ");
        os += "TELEPHONY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::TELEPHONY;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_2) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_2)) {
        os += (first ? "" : " | ");
        os += "RESERVED_2";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_2;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::DATA) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberPlan::DATA)) {
        os += (first ? "" : " | ");
        os += "DATA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::DATA;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::TELEX) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberPlan::TELEX)) {
        os += (first ? "" : " | ");
        os += "TELEX";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::TELEX;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_5) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_5)) {
        os += (first ? "" : " | ");
        os += "RESERVED_5";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_5;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_6) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_6)) {
        os += (first ? "" : " | ");
        os += "RESERVED_6";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_6;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_7) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_7)) {
        os += (first ? "" : " | ");
        os += "RESERVED_7";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_7;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_8) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_8)) {
        os += (first ? "" : " | ");
        os += "RESERVED_8";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_8;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::PRIVATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberPlan::PRIVATE)) {
        os += (first ? "" : " | ");
        os += "PRIVATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::PRIVATE;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_10) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_10)) {
        os += (first ? "" : " | ");
        os += "RESERVED_10";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_10;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_11) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_11)) {
        os += (first ? "" : " | ");
        os += "RESERVED_11";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_11;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_12) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_12)) {
        os += (first ? "" : " | ");
        os += "RESERVED_12";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_12;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_13) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_13)) {
        os += (first ? "" : " | ");
        os += "RESERVED_13";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_13;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_14) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_14)) {
        os += (first ? "" : " | ");
        os += "RESERVED_14";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_14;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_15) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_15)) {
        os += (first ? "" : " | ");
        os += "RESERVED_15";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_15;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CdmaSmsNumberPlan o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::TELEPHONY) {
        return "TELEPHONY";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_2) {
        return "RESERVED_2";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::DATA) {
        return "DATA";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::TELEX) {
        return "TELEX";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_5) {
        return "RESERVED_5";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_6) {
        return "RESERVED_6";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_7) {
        return "RESERVED_7";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_8) {
        return "RESERVED_8";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::PRIVATE) {
        return "PRIVATE";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_10) {
        return "RESERVED_10";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_11) {
        return "RESERVED_11";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_12) {
        return "RESERVED_12";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_13) {
        return "RESERVED_13";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_14) {
        return "RESERVED_14";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_15) {
        return "RESERVED_15";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaSmsSubaddressType lhs, const ::android::hardware::radio::V1_0::CdmaSmsSubaddressType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaSmsSubaddressType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaSmsSubaddressType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaSmsSubaddressType lhs, const ::android::hardware::radio::V1_0::CdmaSmsSubaddressType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaSmsSubaddressType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaSmsSubaddressType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaSmsSubaddressType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaSmsSubaddressType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CdmaSmsSubaddressType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CdmaSmsSubaddressType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsSubaddressType::NSAP) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsSubaddressType::NSAP)) {
        os += (first ? "" : " | ");
        os += "NSAP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsSubaddressType::NSAP;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsSubaddressType::USER_SPECIFIED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsSubaddressType::USER_SPECIFIED)) {
        os += (first ? "" : " | ");
        os += "USER_SPECIFIED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsSubaddressType::USER_SPECIFIED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CdmaSmsSubaddressType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CdmaSmsSubaddressType::NSAP) {
        return "NSAP";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsSubaddressType::USER_SPECIFIED) {
        return "USER_SPECIFIED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaSmsErrorClass lhs, const ::android::hardware::radio::V1_0::CdmaSmsErrorClass rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaSmsErrorClass rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaSmsErrorClass lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaSmsErrorClass lhs, const ::android::hardware::radio::V1_0::CdmaSmsErrorClass rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaSmsErrorClass rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaSmsErrorClass lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaSmsErrorClass e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaSmsErrorClass e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CdmaSmsErrorClass>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CdmaSmsErrorClass> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsErrorClass::NO_ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsErrorClass::NO_ERROR)) {
        os += (first ? "" : " | ");
        os += "NO_ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsErrorClass::NO_ERROR;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsErrorClass::ERROR) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsErrorClass::ERROR)) {
        os += (first ? "" : " | ");
        os += "ERROR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsErrorClass::ERROR;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CdmaSmsErrorClass o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CdmaSmsErrorClass::NO_ERROR) {
        return "NO_ERROR";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsErrorClass::ERROR) {
        return "ERROR";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus lhs, const ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus lhs, const ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::REC_UNREAD) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::REC_UNREAD)) {
        os += (first ? "" : " | ");
        os += "REC_UNREAD";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::REC_UNREAD;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::REC_READ) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::REC_READ)) {
        os += (first ? "" : " | ");
        os += "REC_READ";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::REC_READ;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::STO_UNSENT) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::STO_UNSENT)) {
        os += (first ? "" : " | ");
        os += "STO_UNSENT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::STO_UNSENT;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::STO_SENT) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::STO_SENT)) {
        os += (first ? "" : " | ");
        os += "STO_SENT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::STO_SENT;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::REC_UNREAD) {
        return "REC_UNREAD";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::REC_READ) {
        return "REC_READ";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::STO_UNSENT) {
        return "STO_UNSENT";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::STO_SENT) {
        return "STO_SENT";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CellInfoType lhs, const ::android::hardware::radio::V1_0::CellInfoType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CellInfoType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CellInfoType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CellInfoType lhs, const ::android::hardware::radio::V1_0::CellInfoType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CellInfoType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CellInfoType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CellInfoType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CellInfoType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CellInfoType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CellInfoType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CellInfoType::NONE) == static_cast<int32_t>(::android::hardware::radio::V1_0::CellInfoType::NONE)) {
        os += (first ? "" : " | ");
        os += "NONE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CellInfoType::NONE;
    }
    if ((o & ::android::hardware::radio::V1_0::CellInfoType::GSM) == static_cast<int32_t>(::android::hardware::radio::V1_0::CellInfoType::GSM)) {
        os += (first ? "" : " | ");
        os += "GSM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CellInfoType::GSM;
    }
    if ((o & ::android::hardware::radio::V1_0::CellInfoType::CDMA) == static_cast<int32_t>(::android::hardware::radio::V1_0::CellInfoType::CDMA)) {
        os += (first ? "" : " | ");
        os += "CDMA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CellInfoType::CDMA;
    }
    if ((o & ::android::hardware::radio::V1_0::CellInfoType::LTE) == static_cast<int32_t>(::android::hardware::radio::V1_0::CellInfoType::LTE)) {
        os += (first ? "" : " | ");
        os += "LTE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CellInfoType::LTE;
    }
    if ((o & ::android::hardware::radio::V1_0::CellInfoType::WCDMA) == static_cast<int32_t>(::android::hardware::radio::V1_0::CellInfoType::WCDMA)) {
        os += (first ? "" : " | ");
        os += "WCDMA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CellInfoType::WCDMA;
    }
    if ((o & ::android::hardware::radio::V1_0::CellInfoType::TD_SCDMA) == static_cast<int32_t>(::android::hardware::radio::V1_0::CellInfoType::TD_SCDMA)) {
        os += (first ? "" : " | ");
        os += "TD_SCDMA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CellInfoType::TD_SCDMA;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CellInfoType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CellInfoType::NONE) {
        return "NONE";
    }
    if (o == ::android::hardware::radio::V1_0::CellInfoType::GSM) {
        return "GSM";
    }
    if (o == ::android::hardware::radio::V1_0::CellInfoType::CDMA) {
        return "CDMA";
    }
    if (o == ::android::hardware::radio::V1_0::CellInfoType::LTE) {
        return "LTE";
    }
    if (o == ::android::hardware::radio::V1_0::CellInfoType::WCDMA) {
        return "WCDMA";
    }
    if (o == ::android::hardware::radio::V1_0::CellInfoType::TD_SCDMA) {
        return "TD_SCDMA";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::TimeStampType lhs, const ::android::hardware::radio::V1_0::TimeStampType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::TimeStampType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::TimeStampType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::TimeStampType lhs, const ::android::hardware::radio::V1_0::TimeStampType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::TimeStampType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::TimeStampType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::TimeStampType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::TimeStampType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::TimeStampType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::TimeStampType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::TimeStampType::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::TimeStampType::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::TimeStampType::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_0::TimeStampType::ANTENNA) == static_cast<int32_t>(::android::hardware::radio::V1_0::TimeStampType::ANTENNA)) {
        os += (first ? "" : " | ");
        os += "ANTENNA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::TimeStampType::ANTENNA;
    }
    if ((o & ::android::hardware::radio::V1_0::TimeStampType::MODEM) == static_cast<int32_t>(::android::hardware::radio::V1_0::TimeStampType::MODEM)) {
        os += (first ? "" : " | ");
        os += "MODEM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::TimeStampType::MODEM;
    }
    if ((o & ::android::hardware::radio::V1_0::TimeStampType::OEM_RIL) == static_cast<int32_t>(::android::hardware::radio::V1_0::TimeStampType::OEM_RIL)) {
        os += (first ? "" : " | ");
        os += "OEM_RIL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::TimeStampType::OEM_RIL;
    }
    if ((o & ::android::hardware::radio::V1_0::TimeStampType::JAVA_RIL) == static_cast<int32_t>(::android::hardware::radio::V1_0::TimeStampType::JAVA_RIL)) {
        os += (first ? "" : " | ");
        os += "JAVA_RIL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::TimeStampType::JAVA_RIL;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::TimeStampType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::TimeStampType::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_0::TimeStampType::ANTENNA) {
        return "ANTENNA";
    }
    if (o == ::android::hardware::radio::V1_0::TimeStampType::MODEM) {
        return "MODEM";
    }
    if (o == ::android::hardware::radio::V1_0::TimeStampType::OEM_RIL) {
        return "OEM_RIL";
    }
    if (o == ::android::hardware::radio::V1_0::TimeStampType::JAVA_RIL) {
        return "JAVA_RIL";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::ApnAuthType lhs, const ::android::hardware::radio::V1_0::ApnAuthType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::ApnAuthType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::ApnAuthType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::ApnAuthType lhs, const ::android::hardware::radio::V1_0::ApnAuthType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::ApnAuthType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::ApnAuthType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::ApnAuthType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::ApnAuthType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::ApnAuthType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::ApnAuthType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::ApnAuthType::NO_PAP_NO_CHAP) == static_cast<int32_t>(::android::hardware::radio::V1_0::ApnAuthType::NO_PAP_NO_CHAP)) {
        os += (first ? "" : " | ");
        os += "NO_PAP_NO_CHAP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ApnAuthType::NO_PAP_NO_CHAP;
    }
    if ((o & ::android::hardware::radio::V1_0::ApnAuthType::PAP_NO_CHAP) == static_cast<int32_t>(::android::hardware::radio::V1_0::ApnAuthType::PAP_NO_CHAP)) {
        os += (first ? "" : " | ");
        os += "PAP_NO_CHAP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ApnAuthType::PAP_NO_CHAP;
    }
    if ((o & ::android::hardware::radio::V1_0::ApnAuthType::NO_PAP_CHAP) == static_cast<int32_t>(::android::hardware::radio::V1_0::ApnAuthType::NO_PAP_CHAP)) {
        os += (first ? "" : " | ");
        os += "NO_PAP_CHAP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ApnAuthType::NO_PAP_CHAP;
    }
    if ((o & ::android::hardware::radio::V1_0::ApnAuthType::PAP_CHAP) == static_cast<int32_t>(::android::hardware::radio::V1_0::ApnAuthType::PAP_CHAP)) {
        os += (first ? "" : " | ");
        os += "PAP_CHAP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ApnAuthType::PAP_CHAP;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::ApnAuthType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::ApnAuthType::NO_PAP_NO_CHAP) {
        return "NO_PAP_NO_CHAP";
    }
    if (o == ::android::hardware::radio::V1_0::ApnAuthType::PAP_NO_CHAP) {
        return "PAP_NO_CHAP";
    }
    if (o == ::android::hardware::radio::V1_0::ApnAuthType::NO_PAP_CHAP) {
        return "NO_PAP_CHAP";
    }
    if (o == ::android::hardware::radio::V1_0::ApnAuthType::PAP_CHAP) {
        return "PAP_CHAP";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioTechnologyFamily lhs, const ::android::hardware::radio::V1_0::RadioTechnologyFamily rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioTechnologyFamily rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioTechnologyFamily lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioTechnologyFamily lhs, const ::android::hardware::radio::V1_0::RadioTechnologyFamily rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioTechnologyFamily rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioTechnologyFamily lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::RadioTechnologyFamily e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::RadioTechnologyFamily e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::RadioTechnologyFamily>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::RadioTechnologyFamily> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::RadioTechnologyFamily::THREE_GPP) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnologyFamily::THREE_GPP)) {
        os += (first ? "" : " | ");
        os += "THREE_GPP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnologyFamily::THREE_GPP;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioTechnologyFamily::THREE_GPP2) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioTechnologyFamily::THREE_GPP2)) {
        os += (first ? "" : " | ");
        os += "THREE_GPP2";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioTechnologyFamily::THREE_GPP2;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::RadioTechnologyFamily o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::RadioTechnologyFamily::THREE_GPP) {
        return "THREE_GPP";
    }
    if (o == ::android::hardware::radio::V1_0::RadioTechnologyFamily::THREE_GPP2) {
        return "THREE_GPP2";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioCapabilityPhase lhs, const ::android::hardware::radio::V1_0::RadioCapabilityPhase rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioCapabilityPhase rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioCapabilityPhase lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioCapabilityPhase lhs, const ::android::hardware::radio::V1_0::RadioCapabilityPhase rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioCapabilityPhase rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioCapabilityPhase lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::RadioCapabilityPhase e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::RadioCapabilityPhase e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::RadioCapabilityPhase>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::RadioCapabilityPhase> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::RadioCapabilityPhase::CONFIGURED) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCapabilityPhase::CONFIGURED)) {
        os += (first ? "" : " | ");
        os += "CONFIGURED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCapabilityPhase::CONFIGURED;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCapabilityPhase::START) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCapabilityPhase::START)) {
        os += (first ? "" : " | ");
        os += "START";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCapabilityPhase::START;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCapabilityPhase::APPLY) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCapabilityPhase::APPLY)) {
        os += (first ? "" : " | ");
        os += "APPLY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCapabilityPhase::APPLY;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCapabilityPhase::UNSOL_RSP) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCapabilityPhase::UNSOL_RSP)) {
        os += (first ? "" : " | ");
        os += "UNSOL_RSP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCapabilityPhase::UNSOL_RSP;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCapabilityPhase::FINISH) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCapabilityPhase::FINISH)) {
        os += (first ? "" : " | ");
        os += "FINISH";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCapabilityPhase::FINISH;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::RadioCapabilityPhase o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::RadioCapabilityPhase::CONFIGURED) {
        return "CONFIGURED";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCapabilityPhase::START) {
        return "START";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCapabilityPhase::APPLY) {
        return "APPLY";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCapabilityPhase::UNSOL_RSP) {
        return "UNSOL_RSP";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCapabilityPhase::FINISH) {
        return "FINISH";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioCapabilityStatus lhs, const ::android::hardware::radio::V1_0::RadioCapabilityStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioCapabilityStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioCapabilityStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioCapabilityStatus lhs, const ::android::hardware::radio::V1_0::RadioCapabilityStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioCapabilityStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioCapabilityStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::RadioCapabilityStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::RadioCapabilityStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::RadioCapabilityStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::RadioCapabilityStatus> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::RadioCapabilityStatus::NONE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCapabilityStatus::NONE)) {
        os += (first ? "" : " | ");
        os += "NONE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCapabilityStatus::NONE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCapabilityStatus::SUCCESS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCapabilityStatus::SUCCESS)) {
        os += (first ? "" : " | ");
        os += "SUCCESS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCapabilityStatus::SUCCESS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioCapabilityStatus::FAIL) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioCapabilityStatus::FAIL)) {
        os += (first ? "" : " | ");
        os += "FAIL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioCapabilityStatus::FAIL;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::RadioCapabilityStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::RadioCapabilityStatus::NONE) {
        return "NONE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCapabilityStatus::SUCCESS) {
        return "SUCCESS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioCapabilityStatus::FAIL) {
        return "FAIL";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioAccessFamily lhs, const ::android::hardware::radio::V1_0::RadioAccessFamily rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioAccessFamily rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::RadioAccessFamily lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioAccessFamily lhs, const ::android::hardware::radio::V1_0::RadioAccessFamily rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::RadioAccessFamily rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::RadioAccessFamily lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::RadioAccessFamily e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::RadioAccessFamily e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::RadioAccessFamily>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::RadioAccessFamily> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::GPRS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::GPRS)) {
        os += (first ? "" : " | ");
        os += "GPRS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::GPRS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::EDGE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::EDGE)) {
        os += (first ? "" : " | ");
        os += "EDGE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::EDGE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::UMTS) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::UMTS)) {
        os += (first ? "" : " | ");
        os += "UMTS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::UMTS;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::IS95A) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::IS95A)) {
        os += (first ? "" : " | ");
        os += "IS95A";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::IS95A;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::IS95B) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::IS95B)) {
        os += (first ? "" : " | ");
        os += "IS95B";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::IS95B;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::ONE_X_RTT) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::ONE_X_RTT)) {
        os += (first ? "" : " | ");
        os += "ONE_X_RTT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::ONE_X_RTT;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::EVDO_0) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::EVDO_0)) {
        os += (first ? "" : " | ");
        os += "EVDO_0";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::EVDO_0;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::EVDO_A) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::EVDO_A)) {
        os += (first ? "" : " | ");
        os += "EVDO_A";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::EVDO_A;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::HSDPA) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::HSDPA)) {
        os += (first ? "" : " | ");
        os += "HSDPA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::HSDPA;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::HSUPA) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::HSUPA)) {
        os += (first ? "" : " | ");
        os += "HSUPA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::HSUPA;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::HSPA) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::HSPA)) {
        os += (first ? "" : " | ");
        os += "HSPA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::HSPA;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::EVDO_B) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::EVDO_B)) {
        os += (first ? "" : " | ");
        os += "EVDO_B";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::EVDO_B;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::EHRPD) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::EHRPD)) {
        os += (first ? "" : " | ");
        os += "EHRPD";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::EHRPD;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::LTE) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::LTE)) {
        os += (first ? "" : " | ");
        os += "LTE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::LTE;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::HSPAP) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::HSPAP)) {
        os += (first ? "" : " | ");
        os += "HSPAP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::HSPAP;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::GSM) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::GSM)) {
        os += (first ? "" : " | ");
        os += "GSM";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::GSM;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::TD_SCDMA) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::TD_SCDMA)) {
        os += (first ? "" : " | ");
        os += "TD_SCDMA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::TD_SCDMA;
    }
    if ((o & ::android::hardware::radio::V1_0::RadioAccessFamily::LTE_CA) == static_cast<int32_t>(::android::hardware::radio::V1_0::RadioAccessFamily::LTE_CA)) {
        os += (first ? "" : " | ");
        os += "LTE_CA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::RadioAccessFamily::LTE_CA;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::RadioAccessFamily o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::GPRS) {
        return "GPRS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::EDGE) {
        return "EDGE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::UMTS) {
        return "UMTS";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::IS95A) {
        return "IS95A";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::IS95B) {
        return "IS95B";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::ONE_X_RTT) {
        return "ONE_X_RTT";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::EVDO_0) {
        return "EVDO_0";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::EVDO_A) {
        return "EVDO_A";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::HSDPA) {
        return "HSDPA";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::HSUPA) {
        return "HSUPA";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::HSPA) {
        return "HSPA";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::EVDO_B) {
        return "EVDO_B";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::EHRPD) {
        return "EHRPD";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::LTE) {
        return "LTE";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::HSPAP) {
        return "HSPAP";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::GSM) {
        return "GSM";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::TD_SCDMA) {
        return "TD_SCDMA";
    }
    if (o == ::android::hardware::radio::V1_0::RadioAccessFamily::LTE_CA) {
        return "LTE_CA";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::UssdModeType lhs, const ::android::hardware::radio::V1_0::UssdModeType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::UssdModeType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::UssdModeType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::UssdModeType lhs, const ::android::hardware::radio::V1_0::UssdModeType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::UssdModeType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::UssdModeType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::UssdModeType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::UssdModeType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::UssdModeType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::UssdModeType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::UssdModeType::NOTIFY) == static_cast<int32_t>(::android::hardware::radio::V1_0::UssdModeType::NOTIFY)) {
        os += (first ? "" : " | ");
        os += "NOTIFY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UssdModeType::NOTIFY;
    }
    if ((o & ::android::hardware::radio::V1_0::UssdModeType::REQUEST) == static_cast<int32_t>(::android::hardware::radio::V1_0::UssdModeType::REQUEST)) {
        os += (first ? "" : " | ");
        os += "REQUEST";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UssdModeType::REQUEST;
    }
    if ((o & ::android::hardware::radio::V1_0::UssdModeType::NW_RELEASE) == static_cast<int32_t>(::android::hardware::radio::V1_0::UssdModeType::NW_RELEASE)) {
        os += (first ? "" : " | ");
        os += "NW_RELEASE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UssdModeType::NW_RELEASE;
    }
    if ((o & ::android::hardware::radio::V1_0::UssdModeType::LOCAL_CLIENT) == static_cast<int32_t>(::android::hardware::radio::V1_0::UssdModeType::LOCAL_CLIENT)) {
        os += (first ? "" : " | ");
        os += "LOCAL_CLIENT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UssdModeType::LOCAL_CLIENT;
    }
    if ((o & ::android::hardware::radio::V1_0::UssdModeType::NOT_SUPPORTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::UssdModeType::NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "NOT_SUPPORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UssdModeType::NOT_SUPPORTED;
    }
    if ((o & ::android::hardware::radio::V1_0::UssdModeType::NW_TIMEOUT) == static_cast<int32_t>(::android::hardware::radio::V1_0::UssdModeType::NW_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "NW_TIMEOUT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UssdModeType::NW_TIMEOUT;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::UssdModeType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::UssdModeType::NOTIFY) {
        return "NOTIFY";
    }
    if (o == ::android::hardware::radio::V1_0::UssdModeType::REQUEST) {
        return "REQUEST";
    }
    if (o == ::android::hardware::radio::V1_0::UssdModeType::NW_RELEASE) {
        return "NW_RELEASE";
    }
    if (o == ::android::hardware::radio::V1_0::UssdModeType::LOCAL_CLIENT) {
        return "LOCAL_CLIENT";
    }
    if (o == ::android::hardware::radio::V1_0::UssdModeType::NOT_SUPPORTED) {
        return "NOT_SUPPORTED";
    }
    if (o == ::android::hardware::radio::V1_0::UssdModeType::NW_TIMEOUT) {
        return "NW_TIMEOUT";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SimRefreshType lhs, const ::android::hardware::radio::V1_0::SimRefreshType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::SimRefreshType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SimRefreshType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SimRefreshType lhs, const ::android::hardware::radio::V1_0::SimRefreshType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::SimRefreshType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SimRefreshType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::SimRefreshType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::SimRefreshType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::SimRefreshType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::SimRefreshType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::SimRefreshType::SIM_FILE_UPDATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::SimRefreshType::SIM_FILE_UPDATE)) {
        os += (first ? "" : " | ");
        os += "SIM_FILE_UPDATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SimRefreshType::SIM_FILE_UPDATE;
    }
    if ((o & ::android::hardware::radio::V1_0::SimRefreshType::SIM_INIT) == static_cast<int32_t>(::android::hardware::radio::V1_0::SimRefreshType::SIM_INIT)) {
        os += (first ? "" : " | ");
        os += "SIM_INIT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SimRefreshType::SIM_INIT;
    }
    if ((o & ::android::hardware::radio::V1_0::SimRefreshType::SIM_RESET) == static_cast<int32_t>(::android::hardware::radio::V1_0::SimRefreshType::SIM_RESET)) {
        os += (first ? "" : " | ");
        os += "SIM_RESET";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SimRefreshType::SIM_RESET;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::SimRefreshType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::SimRefreshType::SIM_FILE_UPDATE) {
        return "SIM_FILE_UPDATE";
    }
    if (o == ::android::hardware::radio::V1_0::SimRefreshType::SIM_INIT) {
        return "SIM_INIT";
    }
    if (o == ::android::hardware::radio::V1_0::SimRefreshType::SIM_RESET) {
        return "SIM_RESET";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SrvccState lhs, const ::android::hardware::radio::V1_0::SrvccState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::SrvccState rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SrvccState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SrvccState lhs, const ::android::hardware::radio::V1_0::SrvccState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::SrvccState rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SrvccState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::SrvccState e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::SrvccState e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::SrvccState>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::SrvccState> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::SrvccState::HANDOVER_STARTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::SrvccState::HANDOVER_STARTED)) {
        os += (first ? "" : " | ");
        os += "HANDOVER_STARTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SrvccState::HANDOVER_STARTED;
    }
    if ((o & ::android::hardware::radio::V1_0::SrvccState::HANDOVER_COMPLETED) == static_cast<int32_t>(::android::hardware::radio::V1_0::SrvccState::HANDOVER_COMPLETED)) {
        os += (first ? "" : " | ");
        os += "HANDOVER_COMPLETED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SrvccState::HANDOVER_COMPLETED;
    }
    if ((o & ::android::hardware::radio::V1_0::SrvccState::HANDOVER_FAILED) == static_cast<int32_t>(::android::hardware::radio::V1_0::SrvccState::HANDOVER_FAILED)) {
        os += (first ? "" : " | ");
        os += "HANDOVER_FAILED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SrvccState::HANDOVER_FAILED;
    }
    if ((o & ::android::hardware::radio::V1_0::SrvccState::HANDOVER_CANCELED) == static_cast<int32_t>(::android::hardware::radio::V1_0::SrvccState::HANDOVER_CANCELED)) {
        os += (first ? "" : " | ");
        os += "HANDOVER_CANCELED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SrvccState::HANDOVER_CANCELED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::SrvccState o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::SrvccState::HANDOVER_STARTED) {
        return "HANDOVER_STARTED";
    }
    if (o == ::android::hardware::radio::V1_0::SrvccState::HANDOVER_COMPLETED) {
        return "HANDOVER_COMPLETED";
    }
    if (o == ::android::hardware::radio::V1_0::SrvccState::HANDOVER_FAILED) {
        return "HANDOVER_FAILED";
    }
    if (o == ::android::hardware::radio::V1_0::SrvccState::HANDOVER_CANCELED) {
        return "HANDOVER_CANCELED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::UiccSubActStatus lhs, const ::android::hardware::radio::V1_0::UiccSubActStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::UiccSubActStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::UiccSubActStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::UiccSubActStatus lhs, const ::android::hardware::radio::V1_0::UiccSubActStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::UiccSubActStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::UiccSubActStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::UiccSubActStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::UiccSubActStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::UiccSubActStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::UiccSubActStatus> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::UiccSubActStatus::DEACTIVATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::UiccSubActStatus::DEACTIVATE)) {
        os += (first ? "" : " | ");
        os += "DEACTIVATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UiccSubActStatus::DEACTIVATE;
    }
    if ((o & ::android::hardware::radio::V1_0::UiccSubActStatus::ACTIVATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::UiccSubActStatus::ACTIVATE)) {
        os += (first ? "" : " | ");
        os += "ACTIVATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::UiccSubActStatus::ACTIVATE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::UiccSubActStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::UiccSubActStatus::DEACTIVATE) {
        return "DEACTIVATE";
    }
    if (o == ::android::hardware::radio::V1_0::UiccSubActStatus::ACTIVATE) {
        return "ACTIVATE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SubscriptionType lhs, const ::android::hardware::radio::V1_0::SubscriptionType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::SubscriptionType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SubscriptionType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SubscriptionType lhs, const ::android::hardware::radio::V1_0::SubscriptionType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::SubscriptionType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SubscriptionType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::SubscriptionType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::SubscriptionType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::SubscriptionType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::SubscriptionType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::SubscriptionType::SUBSCRIPTION_1) == static_cast<int32_t>(::android::hardware::radio::V1_0::SubscriptionType::SUBSCRIPTION_1)) {
        os += (first ? "" : " | ");
        os += "SUBSCRIPTION_1";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SubscriptionType::SUBSCRIPTION_1;
    }
    if ((o & ::android::hardware::radio::V1_0::SubscriptionType::SUBSCRIPTION_2) == static_cast<int32_t>(::android::hardware::radio::V1_0::SubscriptionType::SUBSCRIPTION_2)) {
        os += (first ? "" : " | ");
        os += "SUBSCRIPTION_2";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SubscriptionType::SUBSCRIPTION_2;
    }
    if ((o & ::android::hardware::radio::V1_0::SubscriptionType::SUBSCRIPTION_3) == static_cast<int32_t>(::android::hardware::radio::V1_0::SubscriptionType::SUBSCRIPTION_3)) {
        os += (first ? "" : " | ");
        os += "SUBSCRIPTION_3";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SubscriptionType::SUBSCRIPTION_3;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::SubscriptionType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::SubscriptionType::SUBSCRIPTION_1) {
        return "SUBSCRIPTION_1";
    }
    if (o == ::android::hardware::radio::V1_0::SubscriptionType::SUBSCRIPTION_2) {
        return "SUBSCRIPTION_2";
    }
    if (o == ::android::hardware::radio::V1_0::SubscriptionType::SUBSCRIPTION_3) {
        return "SUBSCRIPTION_3";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::DataProfileInfoType lhs, const ::android::hardware::radio::V1_0::DataProfileInfoType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::DataProfileInfoType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::DataProfileInfoType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::DataProfileInfoType lhs, const ::android::hardware::radio::V1_0::DataProfileInfoType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::DataProfileInfoType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::DataProfileInfoType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::DataProfileInfoType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::DataProfileInfoType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::DataProfileInfoType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::DataProfileInfoType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::DataProfileInfoType::COMMON) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataProfileInfoType::COMMON)) {
        os += (first ? "" : " | ");
        os += "COMMON";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataProfileInfoType::COMMON;
    }
    if ((o & ::android::hardware::radio::V1_0::DataProfileInfoType::THREE_GPP) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataProfileInfoType::THREE_GPP)) {
        os += (first ? "" : " | ");
        os += "THREE_GPP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataProfileInfoType::THREE_GPP;
    }
    if ((o & ::android::hardware::radio::V1_0::DataProfileInfoType::THREE_GPP2) == static_cast<int32_t>(::android::hardware::radio::V1_0::DataProfileInfoType::THREE_GPP2)) {
        os += (first ? "" : " | ");
        os += "THREE_GPP2";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DataProfileInfoType::THREE_GPP2;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::DataProfileInfoType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::DataProfileInfoType::COMMON) {
        return "COMMON";
    }
    if (o == ::android::hardware::radio::V1_0::DataProfileInfoType::THREE_GPP) {
        return "THREE_GPP";
    }
    if (o == ::android::hardware::radio::V1_0::DataProfileInfoType::THREE_GPP2) {
        return "THREE_GPP2";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::PhoneRestrictedState lhs, const ::android::hardware::radio::V1_0::PhoneRestrictedState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::PhoneRestrictedState rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::PhoneRestrictedState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::PhoneRestrictedState lhs, const ::android::hardware::radio::V1_0::PhoneRestrictedState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::PhoneRestrictedState rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::PhoneRestrictedState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::PhoneRestrictedState e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::PhoneRestrictedState e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::PhoneRestrictedState>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::PhoneRestrictedState> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::PhoneRestrictedState::NONE) == static_cast<int32_t>(::android::hardware::radio::V1_0::PhoneRestrictedState::NONE)) {
        os += (first ? "" : " | ");
        os += "NONE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PhoneRestrictedState::NONE;
    }
    if ((o & ::android::hardware::radio::V1_0::PhoneRestrictedState::CS_EMERGENCY) == static_cast<int32_t>(::android::hardware::radio::V1_0::PhoneRestrictedState::CS_EMERGENCY)) {
        os += (first ? "" : " | ");
        os += "CS_EMERGENCY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PhoneRestrictedState::CS_EMERGENCY;
    }
    if ((o & ::android::hardware::radio::V1_0::PhoneRestrictedState::CS_NORMAL) == static_cast<int32_t>(::android::hardware::radio::V1_0::PhoneRestrictedState::CS_NORMAL)) {
        os += (first ? "" : " | ");
        os += "CS_NORMAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PhoneRestrictedState::CS_NORMAL;
    }
    if ((o & ::android::hardware::radio::V1_0::PhoneRestrictedState::CS_ALL) == static_cast<int32_t>(::android::hardware::radio::V1_0::PhoneRestrictedState::CS_ALL)) {
        os += (first ? "" : " | ");
        os += "CS_ALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PhoneRestrictedState::CS_ALL;
    }
    if ((o & ::android::hardware::radio::V1_0::PhoneRestrictedState::PS_ALL) == static_cast<int32_t>(::android::hardware::radio::V1_0::PhoneRestrictedState::PS_ALL)) {
        os += (first ? "" : " | ");
        os += "PS_ALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::PhoneRestrictedState::PS_ALL;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::PhoneRestrictedState o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::PhoneRestrictedState::NONE) {
        return "NONE";
    }
    if (o == ::android::hardware::radio::V1_0::PhoneRestrictedState::CS_EMERGENCY) {
        return "CS_EMERGENCY";
    }
    if (o == ::android::hardware::radio::V1_0::PhoneRestrictedState::CS_NORMAL) {
        return "CS_NORMAL";
    }
    if (o == ::android::hardware::radio::V1_0::PhoneRestrictedState::CS_ALL) {
        return "CS_ALL";
    }
    if (o == ::android::hardware::radio::V1_0::PhoneRestrictedState::PS_ALL) {
        return "PS_ALL";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation lhs, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation lhs, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation::ALLOWED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation::ALLOWED)) {
        os += (first ? "" : " | ");
        os += "ALLOWED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation::ALLOWED;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation::RESTRICTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation::RESTRICTED)) {
        os += (first ? "" : " | ");
        os += "RESTRICTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation::RESTRICTED;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation::UNKNOWN;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation::ALLOWED) {
        return "ALLOWED";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation::RESTRICTED) {
        return "RESTRICTED";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation::UNKNOWN) {
        return "UNKNOWN";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType lhs, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType lhs, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CdmaCallWaitingNumberType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CdmaCallWaitingNumberType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::INTERNATIONAL) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::INTERNATIONAL)) {
        os += (first ? "" : " | ");
        os += "INTERNATIONAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::INTERNATIONAL;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::NATIONAL) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::NATIONAL)) {
        os += (first ? "" : " | ");
        os += "NATIONAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::NATIONAL;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::NETWORK_SPECIFIC) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::NETWORK_SPECIFIC)) {
        os += (first ? "" : " | ");
        os += "NETWORK_SPECIFIC";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::NETWORK_SPECIFIC;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::SUBSCRIBER) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::SUBSCRIBER)) {
        os += (first ? "" : " | ");
        os += "SUBSCRIBER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::SUBSCRIBER;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CdmaCallWaitingNumberType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::INTERNATIONAL) {
        return "INTERNATIONAL";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::NATIONAL) {
        return "NATIONAL";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::NETWORK_SPECIFIC) {
        return "NETWORK_SPECIFIC";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::SUBSCRIBER) {
        return "SUBSCRIBER";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan lhs, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan lhs, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::ISDN) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::ISDN)) {
        os += (first ? "" : " | ");
        os += "ISDN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::ISDN;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::DATA) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::DATA)) {
        os += (first ? "" : " | ");
        os += "DATA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::DATA;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::TELEX) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::TELEX)) {
        os += (first ? "" : " | ");
        os += "TELEX";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::TELEX;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::NATIONAL) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::NATIONAL)) {
        os += (first ? "" : " | ");
        os += "NATIONAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::NATIONAL;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::PRIVATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::PRIVATE)) {
        os += (first ? "" : " | ");
        os += "PRIVATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::PRIVATE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::ISDN) {
        return "ISDN";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::DATA) {
        return "DATA";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::TELEX) {
        return "TELEX";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::NATIONAL) {
        return "NATIONAL";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::PRIVATE) {
        return "PRIVATE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus lhs, const ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus lhs, const ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CdmaOtaProvisionStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CdmaOtaProvisionStatus> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::SPL_UNLOCKED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::SPL_UNLOCKED)) {
        os += (first ? "" : " | ");
        os += "SPL_UNLOCKED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::SPL_UNLOCKED;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::SPC_RETRIES_EXCEEDED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::SPC_RETRIES_EXCEEDED)) {
        os += (first ? "" : " | ");
        os += "SPC_RETRIES_EXCEEDED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::SPC_RETRIES_EXCEEDED;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::A_KEY_EXCHANGED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::A_KEY_EXCHANGED)) {
        os += (first ? "" : " | ");
        os += "A_KEY_EXCHANGED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::A_KEY_EXCHANGED;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::SSD_UPDATED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::SSD_UPDATED)) {
        os += (first ? "" : " | ");
        os += "SSD_UPDATED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::SSD_UPDATED;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::NAM_DOWNLOADED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::NAM_DOWNLOADED)) {
        os += (first ? "" : " | ");
        os += "NAM_DOWNLOADED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::NAM_DOWNLOADED;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::MDN_DOWNLOADED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::MDN_DOWNLOADED)) {
        os += (first ? "" : " | ");
        os += "MDN_DOWNLOADED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::MDN_DOWNLOADED;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::IMSI_DOWNLOADED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::IMSI_DOWNLOADED)) {
        os += (first ? "" : " | ");
        os += "IMSI_DOWNLOADED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::IMSI_DOWNLOADED;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::PRL_DOWNLOADED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::PRL_DOWNLOADED)) {
        os += (first ? "" : " | ");
        os += "PRL_DOWNLOADED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::PRL_DOWNLOADED;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::COMMITTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::COMMITTED)) {
        os += (first ? "" : " | ");
        os += "COMMITTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::COMMITTED;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::OTAPA_STARTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::OTAPA_STARTED)) {
        os += (first ? "" : " | ");
        os += "OTAPA_STARTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::OTAPA_STARTED;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::OTAPA_STOPPED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::OTAPA_STOPPED)) {
        os += (first ? "" : " | ");
        os += "OTAPA_STOPPED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::OTAPA_STOPPED;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::OTAPA_ABORTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::OTAPA_ABORTED)) {
        os += (first ? "" : " | ");
        os += "OTAPA_ABORTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::OTAPA_ABORTED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CdmaOtaProvisionStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::SPL_UNLOCKED) {
        return "SPL_UNLOCKED";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::SPC_RETRIES_EXCEEDED) {
        return "SPC_RETRIES_EXCEEDED";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::A_KEY_EXCHANGED) {
        return "A_KEY_EXCHANGED";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::SSD_UPDATED) {
        return "SSD_UPDATED";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::NAM_DOWNLOADED) {
        return "NAM_DOWNLOADED";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::MDN_DOWNLOADED) {
        return "MDN_DOWNLOADED";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::IMSI_DOWNLOADED) {
        return "IMSI_DOWNLOADED";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::PRL_DOWNLOADED) {
        return "PRL_DOWNLOADED";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::COMMITTED) {
        return "COMMITTED";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::OTAPA_STARTED) {
        return "OTAPA_STARTED";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::OTAPA_STOPPED) {
        return "OTAPA_STOPPED";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::OTAPA_ABORTED) {
        return "OTAPA_ABORTED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaInfoRecName lhs, const ::android::hardware::radio::V1_0::CdmaInfoRecName rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaInfoRecName rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaInfoRecName lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaInfoRecName lhs, const ::android::hardware::radio::V1_0::CdmaInfoRecName rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaInfoRecName rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaInfoRecName lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaInfoRecName e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaInfoRecName e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CdmaInfoRecName>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CdmaInfoRecName> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CdmaInfoRecName::DISPLAY) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaInfoRecName::DISPLAY)) {
        os += (first ? "" : " | ");
        os += "DISPLAY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaInfoRecName::DISPLAY;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaInfoRecName::CALLED_PARTY_NUMBER) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaInfoRecName::CALLED_PARTY_NUMBER)) {
        os += (first ? "" : " | ");
        os += "CALLED_PARTY_NUMBER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaInfoRecName::CALLED_PARTY_NUMBER;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaInfoRecName::CALLING_PARTY_NUMBER) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaInfoRecName::CALLING_PARTY_NUMBER)) {
        os += (first ? "" : " | ");
        os += "CALLING_PARTY_NUMBER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaInfoRecName::CALLING_PARTY_NUMBER;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaInfoRecName::CONNECTED_NUMBER) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaInfoRecName::CONNECTED_NUMBER)) {
        os += (first ? "" : " | ");
        os += "CONNECTED_NUMBER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaInfoRecName::CONNECTED_NUMBER;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaInfoRecName::SIGNAL) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaInfoRecName::SIGNAL)) {
        os += (first ? "" : " | ");
        os += "SIGNAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaInfoRecName::SIGNAL;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaInfoRecName::REDIRECTING_NUMBER) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaInfoRecName::REDIRECTING_NUMBER)) {
        os += (first ? "" : " | ");
        os += "REDIRECTING_NUMBER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaInfoRecName::REDIRECTING_NUMBER;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaInfoRecName::LINE_CONTROL) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaInfoRecName::LINE_CONTROL)) {
        os += (first ? "" : " | ");
        os += "LINE_CONTROL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaInfoRecName::LINE_CONTROL;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaInfoRecName::EXTENDED_DISPLAY) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaInfoRecName::EXTENDED_DISPLAY)) {
        os += (first ? "" : " | ");
        os += "EXTENDED_DISPLAY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaInfoRecName::EXTENDED_DISPLAY;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaInfoRecName::T53_CLIR) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaInfoRecName::T53_CLIR)) {
        os += (first ? "" : " | ");
        os += "T53_CLIR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaInfoRecName::T53_CLIR;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaInfoRecName::T53_RELEASE) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaInfoRecName::T53_RELEASE)) {
        os += (first ? "" : " | ");
        os += "T53_RELEASE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaInfoRecName::T53_RELEASE;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaInfoRecName::T53_AUDIO_CONTROL) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaInfoRecName::T53_AUDIO_CONTROL)) {
        os += (first ? "" : " | ");
        os += "T53_AUDIO_CONTROL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaInfoRecName::T53_AUDIO_CONTROL;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CdmaInfoRecName o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CdmaInfoRecName::DISPLAY) {
        return "DISPLAY";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaInfoRecName::CALLED_PARTY_NUMBER) {
        return "CALLED_PARTY_NUMBER";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaInfoRecName::CALLING_PARTY_NUMBER) {
        return "CALLING_PARTY_NUMBER";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaInfoRecName::CONNECTED_NUMBER) {
        return "CONNECTED_NUMBER";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaInfoRecName::SIGNAL) {
        return "SIGNAL";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaInfoRecName::REDIRECTING_NUMBER) {
        return "REDIRECTING_NUMBER";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaInfoRecName::LINE_CONTROL) {
        return "LINE_CONTROL";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaInfoRecName::EXTENDED_DISPLAY) {
        return "EXTENDED_DISPLAY";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaInfoRecName::T53_CLIR) {
        return "T53_CLIR";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaInfoRecName::T53_RELEASE) {
        return "T53_RELEASE";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaInfoRecName::T53_AUDIO_CONTROL) {
        return "T53_AUDIO_CONTROL";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaRedirectingReason lhs, const ::android::hardware::radio::V1_0::CdmaRedirectingReason rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaRedirectingReason rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::CdmaRedirectingReason lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaRedirectingReason lhs, const ::android::hardware::radio::V1_0::CdmaRedirectingReason rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::CdmaRedirectingReason rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::CdmaRedirectingReason lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaRedirectingReason e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::CdmaRedirectingReason e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::CdmaRedirectingReason>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::CdmaRedirectingReason> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::CdmaRedirectingReason::UNKNOWN) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaRedirectingReason::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaRedirectingReason::UNKNOWN;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_BUSY) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_BUSY)) {
        os += (first ? "" : " | ");
        os += "CALL_FORWARDING_BUSY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_BUSY;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_NO_REPLY) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_NO_REPLY)) {
        os += (first ? "" : " | ");
        os += "CALL_FORWARDING_NO_REPLY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_NO_REPLY;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALLED_DTE_OUT_OF_ORDER) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaRedirectingReason::CALLED_DTE_OUT_OF_ORDER)) {
        os += (first ? "" : " | ");
        os += "CALLED_DTE_OUT_OF_ORDER";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALLED_DTE_OUT_OF_ORDER;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_BY_THE_CALLED_DTE) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_BY_THE_CALLED_DTE)) {
        os += (first ? "" : " | ");
        os += "CALL_FORWARDING_BY_THE_CALLED_DTE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_BY_THE_CALLED_DTE;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_UNCONDITIONAL) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_UNCONDITIONAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FORWARDING_UNCONDITIONAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_UNCONDITIONAL;
    }
    if ((o & ::android::hardware::radio::V1_0::CdmaRedirectingReason::RESERVED) == static_cast<int32_t>(::android::hardware::radio::V1_0::CdmaRedirectingReason::RESERVED)) {
        os += (first ? "" : " | ");
        os += "RESERVED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::CdmaRedirectingReason::RESERVED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::CdmaRedirectingReason o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::CdmaRedirectingReason::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_BUSY) {
        return "CALL_FORWARDING_BUSY";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_NO_REPLY) {
        return "CALL_FORWARDING_NO_REPLY";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALLED_DTE_OUT_OF_ORDER) {
        return "CALLED_DTE_OUT_OF_ORDER";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_BY_THE_CALLED_DTE) {
        return "CALL_FORWARDING_BY_THE_CALLED_DTE";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_UNCONDITIONAL) {
        return "CALL_FORWARDING_UNCONDITIONAL";
    }
    if (o == ::android::hardware::radio::V1_0::CdmaRedirectingReason::RESERVED) {
        return "RESERVED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SsServiceType lhs, const ::android::hardware::radio::V1_0::SsServiceType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::SsServiceType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SsServiceType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SsServiceType lhs, const ::android::hardware::radio::V1_0::SsServiceType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::SsServiceType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SsServiceType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::SsServiceType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::SsServiceType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::SsServiceType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::SsServiceType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::CFU) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::CFU)) {
        os += (first ? "" : " | ");
        os += "CFU";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::CFU;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::CF_BUSY) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::CF_BUSY)) {
        os += (first ? "" : " | ");
        os += "CF_BUSY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::CF_BUSY;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::CF_NO_REPLY) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::CF_NO_REPLY)) {
        os += (first ? "" : " | ");
        os += "CF_NO_REPLY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::CF_NO_REPLY;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::CF_NOT_REACHABLE) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::CF_NOT_REACHABLE)) {
        os += (first ? "" : " | ");
        os += "CF_NOT_REACHABLE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::CF_NOT_REACHABLE;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::CF_ALL) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::CF_ALL)) {
        os += (first ? "" : " | ");
        os += "CF_ALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::CF_ALL;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::CF_ALL_CONDITIONAL) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::CF_ALL_CONDITIONAL)) {
        os += (first ? "" : " | ");
        os += "CF_ALL_CONDITIONAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::CF_ALL_CONDITIONAL;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::CLIP) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::CLIP)) {
        os += (first ? "" : " | ");
        os += "CLIP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::CLIP;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::CLIR) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::CLIR)) {
        os += (first ? "" : " | ");
        os += "CLIR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::CLIR;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::COLP) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::COLP)) {
        os += (first ? "" : " | ");
        os += "COLP";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::COLP;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::COLR) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::COLR)) {
        os += (first ? "" : " | ");
        os += "COLR";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::COLR;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::WAIT) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::WAIT)) {
        os += (first ? "" : " | ");
        os += "WAIT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::WAIT;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::BAOC) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::BAOC)) {
        os += (first ? "" : " | ");
        os += "BAOC";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::BAOC;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::BAOIC) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::BAOIC)) {
        os += (first ? "" : " | ");
        os += "BAOIC";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::BAOIC;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::BAOIC_EXC_HOME) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::BAOIC_EXC_HOME)) {
        os += (first ? "" : " | ");
        os += "BAOIC_EXC_HOME";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::BAOIC_EXC_HOME;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::BAIC) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::BAIC)) {
        os += (first ? "" : " | ");
        os += "BAIC";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::BAIC;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::BAIC_ROAMING) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::BAIC_ROAMING)) {
        os += (first ? "" : " | ");
        os += "BAIC_ROAMING";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::BAIC_ROAMING;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::ALL_BARRING) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::ALL_BARRING)) {
        os += (first ? "" : " | ");
        os += "ALL_BARRING";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::ALL_BARRING;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::OUTGOING_BARRING) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::OUTGOING_BARRING)) {
        os += (first ? "" : " | ");
        os += "OUTGOING_BARRING";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::OUTGOING_BARRING;
    }
    if ((o & ::android::hardware::radio::V1_0::SsServiceType::INCOMING_BARRING) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsServiceType::INCOMING_BARRING)) {
        os += (first ? "" : " | ");
        os += "INCOMING_BARRING";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsServiceType::INCOMING_BARRING;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::SsServiceType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::SsServiceType::CFU) {
        return "CFU";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::CF_BUSY) {
        return "CF_BUSY";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::CF_NO_REPLY) {
        return "CF_NO_REPLY";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::CF_NOT_REACHABLE) {
        return "CF_NOT_REACHABLE";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::CF_ALL) {
        return "CF_ALL";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::CF_ALL_CONDITIONAL) {
        return "CF_ALL_CONDITIONAL";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::CLIP) {
        return "CLIP";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::CLIR) {
        return "CLIR";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::COLP) {
        return "COLP";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::COLR) {
        return "COLR";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::WAIT) {
        return "WAIT";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::BAOC) {
        return "BAOC";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::BAOIC) {
        return "BAOIC";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::BAOIC_EXC_HOME) {
        return "BAOIC_EXC_HOME";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::BAIC) {
        return "BAIC";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::BAIC_ROAMING) {
        return "BAIC_ROAMING";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::ALL_BARRING) {
        return "ALL_BARRING";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::OUTGOING_BARRING) {
        return "OUTGOING_BARRING";
    }
    if (o == ::android::hardware::radio::V1_0::SsServiceType::INCOMING_BARRING) {
        return "INCOMING_BARRING";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SsRequestType lhs, const ::android::hardware::radio::V1_0::SsRequestType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::SsRequestType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SsRequestType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SsRequestType lhs, const ::android::hardware::radio::V1_0::SsRequestType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::SsRequestType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SsRequestType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::SsRequestType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::SsRequestType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::SsRequestType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::SsRequestType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::SsRequestType::ACTIVATION) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsRequestType::ACTIVATION)) {
        os += (first ? "" : " | ");
        os += "ACTIVATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsRequestType::ACTIVATION;
    }
    if ((o & ::android::hardware::radio::V1_0::SsRequestType::DEACTIVATION) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsRequestType::DEACTIVATION)) {
        os += (first ? "" : " | ");
        os += "DEACTIVATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsRequestType::DEACTIVATION;
    }
    if ((o & ::android::hardware::radio::V1_0::SsRequestType::INTERROGATION) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsRequestType::INTERROGATION)) {
        os += (first ? "" : " | ");
        os += "INTERROGATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsRequestType::INTERROGATION;
    }
    if ((o & ::android::hardware::radio::V1_0::SsRequestType::REGISTRATION) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsRequestType::REGISTRATION)) {
        os += (first ? "" : " | ");
        os += "REGISTRATION";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsRequestType::REGISTRATION;
    }
    if ((o & ::android::hardware::radio::V1_0::SsRequestType::ERASURE) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsRequestType::ERASURE)) {
        os += (first ? "" : " | ");
        os += "ERASURE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsRequestType::ERASURE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::SsRequestType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::SsRequestType::ACTIVATION) {
        return "ACTIVATION";
    }
    if (o == ::android::hardware::radio::V1_0::SsRequestType::DEACTIVATION) {
        return "DEACTIVATION";
    }
    if (o == ::android::hardware::radio::V1_0::SsRequestType::INTERROGATION) {
        return "INTERROGATION";
    }
    if (o == ::android::hardware::radio::V1_0::SsRequestType::REGISTRATION) {
        return "REGISTRATION";
    }
    if (o == ::android::hardware::radio::V1_0::SsRequestType::ERASURE) {
        return "ERASURE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SsTeleserviceType lhs, const ::android::hardware::radio::V1_0::SsTeleserviceType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::SsTeleserviceType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SsTeleserviceType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SsTeleserviceType lhs, const ::android::hardware::radio::V1_0::SsTeleserviceType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::SsTeleserviceType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SsTeleserviceType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::SsTeleserviceType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::SsTeleserviceType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::SsTeleserviceType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::SsTeleserviceType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::SsTeleserviceType::ALL_TELE_AND_BEARER_SERVICES) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsTeleserviceType::ALL_TELE_AND_BEARER_SERVICES)) {
        os += (first ? "" : " | ");
        os += "ALL_TELE_AND_BEARER_SERVICES";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsTeleserviceType::ALL_TELE_AND_BEARER_SERVICES;
    }
    if ((o & ::android::hardware::radio::V1_0::SsTeleserviceType::ALL_TELESEVICES) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsTeleserviceType::ALL_TELESEVICES)) {
        os += (first ? "" : " | ");
        os += "ALL_TELESEVICES";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsTeleserviceType::ALL_TELESEVICES;
    }
    if ((o & ::android::hardware::radio::V1_0::SsTeleserviceType::TELEPHONY) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsTeleserviceType::TELEPHONY)) {
        os += (first ? "" : " | ");
        os += "TELEPHONY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsTeleserviceType::TELEPHONY;
    }
    if ((o & ::android::hardware::radio::V1_0::SsTeleserviceType::ALL_DATA_TELESERVICES) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsTeleserviceType::ALL_DATA_TELESERVICES)) {
        os += (first ? "" : " | ");
        os += "ALL_DATA_TELESERVICES";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsTeleserviceType::ALL_DATA_TELESERVICES;
    }
    if ((o & ::android::hardware::radio::V1_0::SsTeleserviceType::SMS_SERVICES) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsTeleserviceType::SMS_SERVICES)) {
        os += (first ? "" : " | ");
        os += "SMS_SERVICES";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsTeleserviceType::SMS_SERVICES;
    }
    if ((o & ::android::hardware::radio::V1_0::SsTeleserviceType::ALL_TELESERVICES_EXCEPT_SMS) == static_cast<int32_t>(::android::hardware::radio::V1_0::SsTeleserviceType::ALL_TELESERVICES_EXCEPT_SMS)) {
        os += (first ? "" : " | ");
        os += "ALL_TELESERVICES_EXCEPT_SMS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SsTeleserviceType::ALL_TELESERVICES_EXCEPT_SMS;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::SsTeleserviceType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::SsTeleserviceType::ALL_TELE_AND_BEARER_SERVICES) {
        return "ALL_TELE_AND_BEARER_SERVICES";
    }
    if (o == ::android::hardware::radio::V1_0::SsTeleserviceType::ALL_TELESEVICES) {
        return "ALL_TELESEVICES";
    }
    if (o == ::android::hardware::radio::V1_0::SsTeleserviceType::TELEPHONY) {
        return "TELEPHONY";
    }
    if (o == ::android::hardware::radio::V1_0::SsTeleserviceType::ALL_DATA_TELESERVICES) {
        return "ALL_DATA_TELESERVICES";
    }
    if (o == ::android::hardware::radio::V1_0::SsTeleserviceType::SMS_SERVICES) {
        return "SMS_SERVICES";
    }
    if (o == ::android::hardware::radio::V1_0::SsTeleserviceType::ALL_TELESERVICES_EXCEPT_SMS) {
        return "ALL_TELESERVICES_EXCEPT_SMS";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SuppServiceClass lhs, const ::android::hardware::radio::V1_0::SuppServiceClass rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::SuppServiceClass rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::SuppServiceClass lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SuppServiceClass lhs, const ::android::hardware::radio::V1_0::SuppServiceClass rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::SuppServiceClass rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::SuppServiceClass lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::SuppServiceClass e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::SuppServiceClass e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::SuppServiceClass>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::SuppServiceClass> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::SuppServiceClass::NONE) == static_cast<int32_t>(::android::hardware::radio::V1_0::SuppServiceClass::NONE)) {
        os += (first ? "" : " | ");
        os += "NONE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SuppServiceClass::NONE;
    }
    if ((o & ::android::hardware::radio::V1_0::SuppServiceClass::VOICE) == static_cast<int32_t>(::android::hardware::radio::V1_0::SuppServiceClass::VOICE)) {
        os += (first ? "" : " | ");
        os += "VOICE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SuppServiceClass::VOICE;
    }
    if ((o & ::android::hardware::radio::V1_0::SuppServiceClass::DATA) == static_cast<int32_t>(::android::hardware::radio::V1_0::SuppServiceClass::DATA)) {
        os += (first ? "" : " | ");
        os += "DATA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SuppServiceClass::DATA;
    }
    if ((o & ::android::hardware::radio::V1_0::SuppServiceClass::FAX) == static_cast<int32_t>(::android::hardware::radio::V1_0::SuppServiceClass::FAX)) {
        os += (first ? "" : " | ");
        os += "FAX";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SuppServiceClass::FAX;
    }
    if ((o & ::android::hardware::radio::V1_0::SuppServiceClass::SMS) == static_cast<int32_t>(::android::hardware::radio::V1_0::SuppServiceClass::SMS)) {
        os += (first ? "" : " | ");
        os += "SMS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SuppServiceClass::SMS;
    }
    if ((o & ::android::hardware::radio::V1_0::SuppServiceClass::DATA_SYNC) == static_cast<int32_t>(::android::hardware::radio::V1_0::SuppServiceClass::DATA_SYNC)) {
        os += (first ? "" : " | ");
        os += "DATA_SYNC";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SuppServiceClass::DATA_SYNC;
    }
    if ((o & ::android::hardware::radio::V1_0::SuppServiceClass::DATA_ASYNC) == static_cast<int32_t>(::android::hardware::radio::V1_0::SuppServiceClass::DATA_ASYNC)) {
        os += (first ? "" : " | ");
        os += "DATA_ASYNC";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SuppServiceClass::DATA_ASYNC;
    }
    if ((o & ::android::hardware::radio::V1_0::SuppServiceClass::PACKET) == static_cast<int32_t>(::android::hardware::radio::V1_0::SuppServiceClass::PACKET)) {
        os += (first ? "" : " | ");
        os += "PACKET";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SuppServiceClass::PACKET;
    }
    if ((o & ::android::hardware::radio::V1_0::SuppServiceClass::PAD) == static_cast<int32_t>(::android::hardware::radio::V1_0::SuppServiceClass::PAD)) {
        os += (first ? "" : " | ");
        os += "PAD";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SuppServiceClass::PAD;
    }
    if ((o & ::android::hardware::radio::V1_0::SuppServiceClass::MAX) == static_cast<int32_t>(::android::hardware::radio::V1_0::SuppServiceClass::MAX)) {
        os += (first ? "" : " | ");
        os += "MAX";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::SuppServiceClass::MAX;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::SuppServiceClass o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::SuppServiceClass::NONE) {
        return "NONE";
    }
    if (o == ::android::hardware::radio::V1_0::SuppServiceClass::VOICE) {
        return "VOICE";
    }
    if (o == ::android::hardware::radio::V1_0::SuppServiceClass::DATA) {
        return "DATA";
    }
    if (o == ::android::hardware::radio::V1_0::SuppServiceClass::FAX) {
        return "FAX";
    }
    if (o == ::android::hardware::radio::V1_0::SuppServiceClass::SMS) {
        return "SMS";
    }
    if (o == ::android::hardware::radio::V1_0::SuppServiceClass::DATA_SYNC) {
        return "DATA_SYNC";
    }
    if (o == ::android::hardware::radio::V1_0::SuppServiceClass::DATA_ASYNC) {
        return "DATA_ASYNC";
    }
    if (o == ::android::hardware::radio::V1_0::SuppServiceClass::PACKET) {
        return "PACKET";
    }
    if (o == ::android::hardware::radio::V1_0::SuppServiceClass::PAD) {
        return "PAD";
    }
    if (o == ::android::hardware::radio::V1_0::SuppServiceClass::MAX) {
        return "MAX";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::ApnTypes lhs, const ::android::hardware::radio::V1_0::ApnTypes rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::ApnTypes rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::ApnTypes lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::ApnTypes lhs, const ::android::hardware::radio::V1_0::ApnTypes rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::ApnTypes rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::ApnTypes lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::ApnTypes e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::ApnTypes e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::ApnTypes>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::ApnTypes> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::ApnTypes::NONE) == static_cast<int32_t>(::android::hardware::radio::V1_0::ApnTypes::NONE)) {
        os += (first ? "" : " | ");
        os += "NONE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ApnTypes::NONE;
    }
    if ((o & ::android::hardware::radio::V1_0::ApnTypes::DEFAULT) == static_cast<int32_t>(::android::hardware::radio::V1_0::ApnTypes::DEFAULT)) {
        os += (first ? "" : " | ");
        os += "DEFAULT";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ApnTypes::DEFAULT;
    }
    if ((o & ::android::hardware::radio::V1_0::ApnTypes::MMS) == static_cast<int32_t>(::android::hardware::radio::V1_0::ApnTypes::MMS)) {
        os += (first ? "" : " | ");
        os += "MMS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ApnTypes::MMS;
    }
    if ((o & ::android::hardware::radio::V1_0::ApnTypes::SUPL) == static_cast<int32_t>(::android::hardware::radio::V1_0::ApnTypes::SUPL)) {
        os += (first ? "" : " | ");
        os += "SUPL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ApnTypes::SUPL;
    }
    if ((o & ::android::hardware::radio::V1_0::ApnTypes::DUN) == static_cast<int32_t>(::android::hardware::radio::V1_0::ApnTypes::DUN)) {
        os += (first ? "" : " | ");
        os += "DUN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ApnTypes::DUN;
    }
    if ((o & ::android::hardware::radio::V1_0::ApnTypes::HIPRI) == static_cast<int32_t>(::android::hardware::radio::V1_0::ApnTypes::HIPRI)) {
        os += (first ? "" : " | ");
        os += "HIPRI";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ApnTypes::HIPRI;
    }
    if ((o & ::android::hardware::radio::V1_0::ApnTypes::FOTA) == static_cast<int32_t>(::android::hardware::radio::V1_0::ApnTypes::FOTA)) {
        os += (first ? "" : " | ");
        os += "FOTA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ApnTypes::FOTA;
    }
    if ((o & ::android::hardware::radio::V1_0::ApnTypes::IMS) == static_cast<int32_t>(::android::hardware::radio::V1_0::ApnTypes::IMS)) {
        os += (first ? "" : " | ");
        os += "IMS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ApnTypes::IMS;
    }
    if ((o & ::android::hardware::radio::V1_0::ApnTypes::CBS) == static_cast<int32_t>(::android::hardware::radio::V1_0::ApnTypes::CBS)) {
        os += (first ? "" : " | ");
        os += "CBS";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ApnTypes::CBS;
    }
    if ((o & ::android::hardware::radio::V1_0::ApnTypes::IA) == static_cast<int32_t>(::android::hardware::radio::V1_0::ApnTypes::IA)) {
        os += (first ? "" : " | ");
        os += "IA";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ApnTypes::IA;
    }
    if ((o & ::android::hardware::radio::V1_0::ApnTypes::EMERGENCY) == static_cast<int32_t>(::android::hardware::radio::V1_0::ApnTypes::EMERGENCY)) {
        os += (first ? "" : " | ");
        os += "EMERGENCY";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ApnTypes::EMERGENCY;
    }
    if ((o & ::android::hardware::radio::V1_0::ApnTypes::ALL) == static_cast<int32_t>(::android::hardware::radio::V1_0::ApnTypes::ALL)) {
        os += (first ? "" : " | ");
        os += "ALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::ApnTypes::ALL;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::ApnTypes o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::ApnTypes::NONE) {
        return "NONE";
    }
    if (o == ::android::hardware::radio::V1_0::ApnTypes::DEFAULT) {
        return "DEFAULT";
    }
    if (o == ::android::hardware::radio::V1_0::ApnTypes::MMS) {
        return "MMS";
    }
    if (o == ::android::hardware::radio::V1_0::ApnTypes::SUPL) {
        return "SUPL";
    }
    if (o == ::android::hardware::radio::V1_0::ApnTypes::DUN) {
        return "DUN";
    }
    if (o == ::android::hardware::radio::V1_0::ApnTypes::HIPRI) {
        return "HIPRI";
    }
    if (o == ::android::hardware::radio::V1_0::ApnTypes::FOTA) {
        return "FOTA";
    }
    if (o == ::android::hardware::radio::V1_0::ApnTypes::IMS) {
        return "IMS";
    }
    if (o == ::android::hardware::radio::V1_0::ApnTypes::CBS) {
        return "CBS";
    }
    if (o == ::android::hardware::radio::V1_0::ApnTypes::IA) {
        return "IA";
    }
    if (o == ::android::hardware::radio::V1_0::ApnTypes::EMERGENCY) {
        return "EMERGENCY";
    }
    if (o == ::android::hardware::radio::V1_0::ApnTypes::ALL) {
        return "ALL";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::IndicationFilter lhs, const ::android::hardware::radio::V1_0::IndicationFilter rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::IndicationFilter rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::IndicationFilter lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::IndicationFilter lhs, const ::android::hardware::radio::V1_0::IndicationFilter rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::IndicationFilter rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::IndicationFilter lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::IndicationFilter e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::IndicationFilter e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::IndicationFilter>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::IndicationFilter> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::IndicationFilter::NONE) == static_cast<int32_t>(::android::hardware::radio::V1_0::IndicationFilter::NONE)) {
        os += (first ? "" : " | ");
        os += "NONE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::IndicationFilter::NONE;
    }
    if ((o & ::android::hardware::radio::V1_0::IndicationFilter::SIGNAL_STRENGTH) == static_cast<int32_t>(::android::hardware::radio::V1_0::IndicationFilter::SIGNAL_STRENGTH)) {
        os += (first ? "" : " | ");
        os += "SIGNAL_STRENGTH";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::IndicationFilter::SIGNAL_STRENGTH;
    }
    if ((o & ::android::hardware::radio::V1_0::IndicationFilter::FULL_NETWORK_STATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::IndicationFilter::FULL_NETWORK_STATE)) {
        os += (first ? "" : " | ");
        os += "FULL_NETWORK_STATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::IndicationFilter::FULL_NETWORK_STATE;
    }
    if ((o & ::android::hardware::radio::V1_0::IndicationFilter::DATA_CALL_DORMANCY_CHANGED) == static_cast<int32_t>(::android::hardware::radio::V1_0::IndicationFilter::DATA_CALL_DORMANCY_CHANGED)) {
        os += (first ? "" : " | ");
        os += "DATA_CALL_DORMANCY_CHANGED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::IndicationFilter::DATA_CALL_DORMANCY_CHANGED;
    }
    if ((o & ::android::hardware::radio::V1_0::IndicationFilter::ALL) == static_cast<int32_t>(::android::hardware::radio::V1_0::IndicationFilter::ALL)) {
        os += (first ? "" : " | ");
        os += "ALL";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::IndicationFilter::ALL;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::IndicationFilter o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::IndicationFilter::NONE) {
        return "NONE";
    }
    if (o == ::android::hardware::radio::V1_0::IndicationFilter::SIGNAL_STRENGTH) {
        return "SIGNAL_STRENGTH";
    }
    if (o == ::android::hardware::radio::V1_0::IndicationFilter::FULL_NETWORK_STATE) {
        return "FULL_NETWORK_STATE";
    }
    if (o == ::android::hardware::radio::V1_0::IndicationFilter::DATA_CALL_DORMANCY_CHANGED) {
        return "DATA_CALL_DORMANCY_CHANGED";
    }
    if (o == ::android::hardware::radio::V1_0::IndicationFilter::ALL) {
        return "ALL";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::MvnoType lhs, const ::android::hardware::radio::V1_0::MvnoType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::MvnoType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::MvnoType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::MvnoType lhs, const ::android::hardware::radio::V1_0::MvnoType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::MvnoType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::MvnoType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::MvnoType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::MvnoType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::MvnoType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::MvnoType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::MvnoType::NONE) == static_cast<int32_t>(::android::hardware::radio::V1_0::MvnoType::NONE)) {
        os += (first ? "" : " | ");
        os += "NONE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::MvnoType::NONE;
    }
    if ((o & ::android::hardware::radio::V1_0::MvnoType::IMSI) == static_cast<int32_t>(::android::hardware::radio::V1_0::MvnoType::IMSI)) {
        os += (first ? "" : " | ");
        os += "IMSI";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::MvnoType::IMSI;
    }
    if ((o & ::android::hardware::radio::V1_0::MvnoType::GID) == static_cast<int32_t>(::android::hardware::radio::V1_0::MvnoType::GID)) {
        os += (first ? "" : " | ");
        os += "GID";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::MvnoType::GID;
    }
    if ((o & ::android::hardware::radio::V1_0::MvnoType::SPN) == static_cast<int32_t>(::android::hardware::radio::V1_0::MvnoType::SPN)) {
        os += (first ? "" : " | ");
        os += "SPN";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::MvnoType::SPN;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::MvnoType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::MvnoType::NONE) {
        return "NONE";
    }
    if (o == ::android::hardware::radio::V1_0::MvnoType::IMSI) {
        return "IMSI";
    }
    if (o == ::android::hardware::radio::V1_0::MvnoType::GID) {
        return "GID";
    }
    if (o == ::android::hardware::radio::V1_0::MvnoType::SPN) {
        return "SPN";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::DeviceStateType lhs, const ::android::hardware::radio::V1_0::DeviceStateType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::DeviceStateType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::DeviceStateType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::DeviceStateType lhs, const ::android::hardware::radio::V1_0::DeviceStateType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::DeviceStateType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::DeviceStateType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::DeviceStateType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::DeviceStateType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::DeviceStateType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::DeviceStateType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::DeviceStateType::POWER_SAVE_MODE) == static_cast<int32_t>(::android::hardware::radio::V1_0::DeviceStateType::POWER_SAVE_MODE)) {
        os += (first ? "" : " | ");
        os += "POWER_SAVE_MODE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DeviceStateType::POWER_SAVE_MODE;
    }
    if ((o & ::android::hardware::radio::V1_0::DeviceStateType::CHARGING_STATE) == static_cast<int32_t>(::android::hardware::radio::V1_0::DeviceStateType::CHARGING_STATE)) {
        os += (first ? "" : " | ");
        os += "CHARGING_STATE";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DeviceStateType::CHARGING_STATE;
    }
    if ((o & ::android::hardware::radio::V1_0::DeviceStateType::LOW_DATA_EXPECTED) == static_cast<int32_t>(::android::hardware::radio::V1_0::DeviceStateType::LOW_DATA_EXPECTED)) {
        os += (first ? "" : " | ");
        os += "LOW_DATA_EXPECTED";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::DeviceStateType::LOW_DATA_EXPECTED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::DeviceStateType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::DeviceStateType::POWER_SAVE_MODE) {
        return "POWER_SAVE_MODE";
    }
    if (o == ::android::hardware::radio::V1_0::DeviceStateType::CHARGING_STATE) {
        return "CHARGING_STATE";
    }
    if (o == ::android::hardware::radio::V1_0::DeviceStateType::LOW_DATA_EXPECTED) {
        return "LOW_DATA_EXPECTED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::P2Constant lhs, const ::android::hardware::radio::V1_0::P2Constant rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_0::P2Constant rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_0::P2Constant lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::P2Constant lhs, const ::android::hardware::radio::V1_0::P2Constant rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_0::P2Constant rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_0::P2Constant lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_0::P2Constant e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_0::P2Constant e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_0::P2Constant>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::P2Constant> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_0::P2Constant::NO_P2) == static_cast<int32_t>(::android::hardware::radio::V1_0::P2Constant::NO_P2)) {
        os += (first ? "" : " | ");
        os += "NO_P2";
        first = false;
        flipped |= ::android::hardware::radio::V1_0::P2Constant::NO_P2;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_0::P2Constant o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_0::P2Constant::NO_P2) {
        return "NO_P2";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::android::hardware::radio::V1_0::RadioResponseInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".type = ";
    os += ::android::hardware::radio::V1_0::toString(o.type);
    os += ", .serial = ";
    os += ::android::hardware::toString(o.serial);
    os += ", .error = ";
    os += ::android::hardware::radio::V1_0::toString(o.error);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::RadioResponseInfo& lhs, const ::android::hardware::radio::V1_0::RadioResponseInfo& rhs) {
    if (lhs.type != rhs.type) {
        return false;
    }
    if (lhs.serial != rhs.serial) {
        return false;
    }
    if (lhs.error != rhs.error) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::RadioResponseInfo& lhs,const ::android::hardware::radio::V1_0::RadioResponseInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::AppStatus& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".appType = ";
    os += ::android::hardware::radio::V1_0::toString(o.appType);
    os += ", .appState = ";
    os += ::android::hardware::radio::V1_0::toString(o.appState);
    os += ", .persoSubstate = ";
    os += ::android::hardware::radio::V1_0::toString(o.persoSubstate);
    os += ", .aidPtr = ";
    os += ::android::hardware::toString(o.aidPtr);
    os += ", .appLabelPtr = ";
    os += ::android::hardware::toString(o.appLabelPtr);
    os += ", .pin1Replaced = ";
    os += ::android::hardware::toString(o.pin1Replaced);
    os += ", .pin1 = ";
    os += ::android::hardware::radio::V1_0::toString(o.pin1);
    os += ", .pin2 = ";
    os += ::android::hardware::radio::V1_0::toString(o.pin2);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::AppStatus& lhs, const ::android::hardware::radio::V1_0::AppStatus& rhs) {
    if (lhs.appType != rhs.appType) {
        return false;
    }
    if (lhs.appState != rhs.appState) {
        return false;
    }
    if (lhs.persoSubstate != rhs.persoSubstate) {
        return false;
    }
    if (lhs.aidPtr != rhs.aidPtr) {
        return false;
    }
    if (lhs.appLabelPtr != rhs.appLabelPtr) {
        return false;
    }
    if (lhs.pin1Replaced != rhs.pin1Replaced) {
        return false;
    }
    if (lhs.pin1 != rhs.pin1) {
        return false;
    }
    if (lhs.pin2 != rhs.pin2) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::AppStatus& lhs,const ::android::hardware::radio::V1_0::AppStatus& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CardStatus& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cardState = ";
    os += ::android::hardware::radio::V1_0::toString(o.cardState);
    os += ", .universalPinState = ";
    os += ::android::hardware::radio::V1_0::toString(o.universalPinState);
    os += ", .gsmUmtsSubscriptionAppIndex = ";
    os += ::android::hardware::toString(o.gsmUmtsSubscriptionAppIndex);
    os += ", .cdmaSubscriptionAppIndex = ";
    os += ::android::hardware::toString(o.cdmaSubscriptionAppIndex);
    os += ", .imsSubscriptionAppIndex = ";
    os += ::android::hardware::toString(o.imsSubscriptionAppIndex);
    os += ", .applications = ";
    os += ::android::hardware::toString(o.applications);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CardStatus& lhs, const ::android::hardware::radio::V1_0::CardStatus& rhs) {
    if (lhs.cardState != rhs.cardState) {
        return false;
    }
    if (lhs.universalPinState != rhs.universalPinState) {
        return false;
    }
    if (lhs.gsmUmtsSubscriptionAppIndex != rhs.gsmUmtsSubscriptionAppIndex) {
        return false;
    }
    if (lhs.cdmaSubscriptionAppIndex != rhs.cdmaSubscriptionAppIndex) {
        return false;
    }
    if (lhs.imsSubscriptionAppIndex != rhs.imsSubscriptionAppIndex) {
        return false;
    }
    if (lhs.applications != rhs.applications) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CardStatus& lhs,const ::android::hardware::radio::V1_0::CardStatus& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::UusInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".uusType = ";
    os += ::android::hardware::radio::V1_0::toString(o.uusType);
    os += ", .uusDcs = ";
    os += ::android::hardware::radio::V1_0::toString(o.uusDcs);
    os += ", .uusData = ";
    os += ::android::hardware::toString(o.uusData);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::UusInfo& lhs, const ::android::hardware::radio::V1_0::UusInfo& rhs) {
    if (lhs.uusType != rhs.uusType) {
        return false;
    }
    if (lhs.uusDcs != rhs.uusDcs) {
        return false;
    }
    if (lhs.uusData != rhs.uusData) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::UusInfo& lhs,const ::android::hardware::radio::V1_0::UusInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::Call& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".state = ";
    os += ::android::hardware::radio::V1_0::toString(o.state);
    os += ", .index = ";
    os += ::android::hardware::toString(o.index);
    os += ", .toa = ";
    os += ::android::hardware::toString(o.toa);
    os += ", .isMpty = ";
    os += ::android::hardware::toString(o.isMpty);
    os += ", .isMT = ";
    os += ::android::hardware::toString(o.isMT);
    os += ", .als = ";
    os += ::android::hardware::toString(o.als);
    os += ", .isVoice = ";
    os += ::android::hardware::toString(o.isVoice);
    os += ", .isVoicePrivacy = ";
    os += ::android::hardware::toString(o.isVoicePrivacy);
    os += ", .number = ";
    os += ::android::hardware::toString(o.number);
    os += ", .numberPresentation = ";
    os += ::android::hardware::radio::V1_0::toString(o.numberPresentation);
    os += ", .name = ";
    os += ::android::hardware::toString(o.name);
    os += ", .namePresentation = ";
    os += ::android::hardware::radio::V1_0::toString(o.namePresentation);
    os += ", .uusInfo = ";
    os += ::android::hardware::toString(o.uusInfo);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::Call& lhs, const ::android::hardware::radio::V1_0::Call& rhs) {
    if (lhs.state != rhs.state) {
        return false;
    }
    if (lhs.index != rhs.index) {
        return false;
    }
    if (lhs.toa != rhs.toa) {
        return false;
    }
    if (lhs.isMpty != rhs.isMpty) {
        return false;
    }
    if (lhs.isMT != rhs.isMT) {
        return false;
    }
    if (lhs.als != rhs.als) {
        return false;
    }
    if (lhs.isVoice != rhs.isVoice) {
        return false;
    }
    if (lhs.isVoicePrivacy != rhs.isVoicePrivacy) {
        return false;
    }
    if (lhs.number != rhs.number) {
        return false;
    }
    if (lhs.numberPresentation != rhs.numberPresentation) {
        return false;
    }
    if (lhs.name != rhs.name) {
        return false;
    }
    if (lhs.namePresentation != rhs.namePresentation) {
        return false;
    }
    if (lhs.uusInfo != rhs.uusInfo) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::Call& lhs,const ::android::hardware::radio::V1_0::Call& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::Dial& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".address = ";
    os += ::android::hardware::toString(o.address);
    os += ", .clir = ";
    os += ::android::hardware::radio::V1_0::toString(o.clir);
    os += ", .uusInfo = ";
    os += ::android::hardware::toString(o.uusInfo);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::Dial& lhs, const ::android::hardware::radio::V1_0::Dial& rhs) {
    if (lhs.address != rhs.address) {
        return false;
    }
    if (lhs.clir != rhs.clir) {
        return false;
    }
    if (lhs.uusInfo != rhs.uusInfo) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::Dial& lhs,const ::android::hardware::radio::V1_0::Dial& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::LastCallFailCauseInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".causeCode = ";
    os += ::android::hardware::radio::V1_0::toString(o.causeCode);
    os += ", .vendorCause = ";
    os += ::android::hardware::toString(o.vendorCause);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::LastCallFailCauseInfo& lhs, const ::android::hardware::radio::V1_0::LastCallFailCauseInfo& rhs) {
    if (lhs.causeCode != rhs.causeCode) {
        return false;
    }
    if (lhs.vendorCause != rhs.vendorCause) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::LastCallFailCauseInfo& lhs,const ::android::hardware::radio::V1_0::LastCallFailCauseInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::GsmSignalStrength& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".signalStrength = ";
    os += ::android::hardware::toString(o.signalStrength);
    os += ", .bitErrorRate = ";
    os += ::android::hardware::toString(o.bitErrorRate);
    os += ", .timingAdvance = ";
    os += ::android::hardware::toString(o.timingAdvance);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::GsmSignalStrength& lhs, const ::android::hardware::radio::V1_0::GsmSignalStrength& rhs) {
    if (lhs.signalStrength != rhs.signalStrength) {
        return false;
    }
    if (lhs.bitErrorRate != rhs.bitErrorRate) {
        return false;
    }
    if (lhs.timingAdvance != rhs.timingAdvance) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::GsmSignalStrength& lhs,const ::android::hardware::radio::V1_0::GsmSignalStrength& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::WcdmaSignalStrength& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".signalStrength = ";
    os += ::android::hardware::toString(o.signalStrength);
    os += ", .bitErrorRate = ";
    os += ::android::hardware::toString(o.bitErrorRate);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::WcdmaSignalStrength& lhs, const ::android::hardware::radio::V1_0::WcdmaSignalStrength& rhs) {
    if (lhs.signalStrength != rhs.signalStrength) {
        return false;
    }
    if (lhs.bitErrorRate != rhs.bitErrorRate) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::WcdmaSignalStrength& lhs,const ::android::hardware::radio::V1_0::WcdmaSignalStrength& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaSignalStrength& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".dbm = ";
    os += ::android::hardware::toString(o.dbm);
    os += ", .ecio = ";
    os += ::android::hardware::toString(o.ecio);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaSignalStrength& lhs, const ::android::hardware::radio::V1_0::CdmaSignalStrength& rhs) {
    if (lhs.dbm != rhs.dbm) {
        return false;
    }
    if (lhs.ecio != rhs.ecio) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaSignalStrength& lhs,const ::android::hardware::radio::V1_0::CdmaSignalStrength& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::EvdoSignalStrength& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".dbm = ";
    os += ::android::hardware::toString(o.dbm);
    os += ", .ecio = ";
    os += ::android::hardware::toString(o.ecio);
    os += ", .signalNoiseRatio = ";
    os += ::android::hardware::toString(o.signalNoiseRatio);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::EvdoSignalStrength& lhs, const ::android::hardware::radio::V1_0::EvdoSignalStrength& rhs) {
    if (lhs.dbm != rhs.dbm) {
        return false;
    }
    if (lhs.ecio != rhs.ecio) {
        return false;
    }
    if (lhs.signalNoiseRatio != rhs.signalNoiseRatio) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::EvdoSignalStrength& lhs,const ::android::hardware::radio::V1_0::EvdoSignalStrength& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::LteSignalStrength& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".signalStrength = ";
    os += ::android::hardware::toString(o.signalStrength);
    os += ", .rsrp = ";
    os += ::android::hardware::toString(o.rsrp);
    os += ", .rsrq = ";
    os += ::android::hardware::toString(o.rsrq);
    os += ", .rssnr = ";
    os += ::android::hardware::toString(o.rssnr);
    os += ", .cqi = ";
    os += ::android::hardware::toString(o.cqi);
    os += ", .timingAdvance = ";
    os += ::android::hardware::toString(o.timingAdvance);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::LteSignalStrength& lhs, const ::android::hardware::radio::V1_0::LteSignalStrength& rhs) {
    if (lhs.signalStrength != rhs.signalStrength) {
        return false;
    }
    if (lhs.rsrp != rhs.rsrp) {
        return false;
    }
    if (lhs.rsrq != rhs.rsrq) {
        return false;
    }
    if (lhs.rssnr != rhs.rssnr) {
        return false;
    }
    if (lhs.cqi != rhs.cqi) {
        return false;
    }
    if (lhs.timingAdvance != rhs.timingAdvance) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::LteSignalStrength& lhs,const ::android::hardware::radio::V1_0::LteSignalStrength& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::TdScdmaSignalStrength& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".rscp = ";
    os += ::android::hardware::toString(o.rscp);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::TdScdmaSignalStrength& lhs, const ::android::hardware::radio::V1_0::TdScdmaSignalStrength& rhs) {
    if (lhs.rscp != rhs.rscp) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::TdScdmaSignalStrength& lhs,const ::android::hardware::radio::V1_0::TdScdmaSignalStrength& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::SignalStrength& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".gw = ";
    os += ::android::hardware::radio::V1_0::toString(o.gw);
    os += ", .cdma = ";
    os += ::android::hardware::radio::V1_0::toString(o.cdma);
    os += ", .evdo = ";
    os += ::android::hardware::radio::V1_0::toString(o.evdo);
    os += ", .lte = ";
    os += ::android::hardware::radio::V1_0::toString(o.lte);
    os += ", .tdScdma = ";
    os += ::android::hardware::radio::V1_0::toString(o.tdScdma);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::SignalStrength& lhs, const ::android::hardware::radio::V1_0::SignalStrength& rhs) {
    if (lhs.gw != rhs.gw) {
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
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::SignalStrength& lhs,const ::android::hardware::radio::V1_0::SignalStrength& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::SendSmsResult& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".messageRef = ";
    os += ::android::hardware::toString(o.messageRef);
    os += ", .ackPDU = ";
    os += ::android::hardware::toString(o.ackPDU);
    os += ", .errorCode = ";
    os += ::android::hardware::toString(o.errorCode);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::SendSmsResult& lhs, const ::android::hardware::radio::V1_0::SendSmsResult& rhs) {
    if (lhs.messageRef != rhs.messageRef) {
        return false;
    }
    if (lhs.ackPDU != rhs.ackPDU) {
        return false;
    }
    if (lhs.errorCode != rhs.errorCode) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::SendSmsResult& lhs,const ::android::hardware::radio::V1_0::SendSmsResult& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::SetupDataCallResult& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".status = ";
    os += ::android::hardware::radio::V1_0::toString(o.status);
    os += ", .suggestedRetryTime = ";
    os += ::android::hardware::toString(o.suggestedRetryTime);
    os += ", .cid = ";
    os += ::android::hardware::toString(o.cid);
    os += ", .active = ";
    os += ::android::hardware::toString(o.active);
    os += ", .type = ";
    os += ::android::hardware::toString(o.type);
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

static inline bool operator==(const ::android::hardware::radio::V1_0::SetupDataCallResult& lhs, const ::android::hardware::radio::V1_0::SetupDataCallResult& rhs) {
    if (lhs.status != rhs.status) {
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

static inline bool operator!=(const ::android::hardware::radio::V1_0::SetupDataCallResult& lhs,const ::android::hardware::radio::V1_0::SetupDataCallResult& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::IccIo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".command = ";
    os += ::android::hardware::toString(o.command);
    os += ", .fileId = ";
    os += ::android::hardware::toString(o.fileId);
    os += ", .path = ";
    os += ::android::hardware::toString(o.path);
    os += ", .p1 = ";
    os += ::android::hardware::toString(o.p1);
    os += ", .p2 = ";
    os += ::android::hardware::toString(o.p2);
    os += ", .p3 = ";
    os += ::android::hardware::toString(o.p3);
    os += ", .data = ";
    os += ::android::hardware::toString(o.data);
    os += ", .pin2 = ";
    os += ::android::hardware::toString(o.pin2);
    os += ", .aid = ";
    os += ::android::hardware::toString(o.aid);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::IccIo& lhs, const ::android::hardware::radio::V1_0::IccIo& rhs) {
    if (lhs.command != rhs.command) {
        return false;
    }
    if (lhs.fileId != rhs.fileId) {
        return false;
    }
    if (lhs.path != rhs.path) {
        return false;
    }
    if (lhs.p1 != rhs.p1) {
        return false;
    }
    if (lhs.p2 != rhs.p2) {
        return false;
    }
    if (lhs.p3 != rhs.p3) {
        return false;
    }
    if (lhs.data != rhs.data) {
        return false;
    }
    if (lhs.pin2 != rhs.pin2) {
        return false;
    }
    if (lhs.aid != rhs.aid) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::IccIo& lhs,const ::android::hardware::radio::V1_0::IccIo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::IccIoResult& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".sw1 = ";
    os += ::android::hardware::toString(o.sw1);
    os += ", .sw2 = ";
    os += ::android::hardware::toString(o.sw2);
    os += ", .simResponse = ";
    os += ::android::hardware::toString(o.simResponse);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::IccIoResult& lhs, const ::android::hardware::radio::V1_0::IccIoResult& rhs) {
    if (lhs.sw1 != rhs.sw1) {
        return false;
    }
    if (lhs.sw2 != rhs.sw2) {
        return false;
    }
    if (lhs.simResponse != rhs.simResponse) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::IccIoResult& lhs,const ::android::hardware::radio::V1_0::IccIoResult& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CallForwardInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".status = ";
    os += ::android::hardware::radio::V1_0::toString(o.status);
    os += ", .reason = ";
    os += ::android::hardware::toString(o.reason);
    os += ", .serviceClass = ";
    os += ::android::hardware::toString(o.serviceClass);
    os += ", .toa = ";
    os += ::android::hardware::toString(o.toa);
    os += ", .number = ";
    os += ::android::hardware::toString(o.number);
    os += ", .timeSeconds = ";
    os += ::android::hardware::toString(o.timeSeconds);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CallForwardInfo& lhs, const ::android::hardware::radio::V1_0::CallForwardInfo& rhs) {
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.reason != rhs.reason) {
        return false;
    }
    if (lhs.serviceClass != rhs.serviceClass) {
        return false;
    }
    if (lhs.toa != rhs.toa) {
        return false;
    }
    if (lhs.number != rhs.number) {
        return false;
    }
    if (lhs.timeSeconds != rhs.timeSeconds) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CallForwardInfo& lhs,const ::android::hardware::radio::V1_0::CallForwardInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::OperatorInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".alphaLong = ";
    os += ::android::hardware::toString(o.alphaLong);
    os += ", .alphaShort = ";
    os += ::android::hardware::toString(o.alphaShort);
    os += ", .operatorNumeric = ";
    os += ::android::hardware::toString(o.operatorNumeric);
    os += ", .status = ";
    os += ::android::hardware::radio::V1_0::toString(o.status);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::OperatorInfo& lhs, const ::android::hardware::radio::V1_0::OperatorInfo& rhs) {
    if (lhs.alphaLong != rhs.alphaLong) {
        return false;
    }
    if (lhs.alphaShort != rhs.alphaShort) {
        return false;
    }
    if (lhs.operatorNumeric != rhs.operatorNumeric) {
        return false;
    }
    if (lhs.status != rhs.status) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::OperatorInfo& lhs,const ::android::hardware::radio::V1_0::OperatorInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::SmsWriteArgs& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".status = ";
    os += ::android::hardware::radio::V1_0::toString(o.status);
    os += ", .pdu = ";
    os += ::android::hardware::toString(o.pdu);
    os += ", .smsc = ";
    os += ::android::hardware::toString(o.smsc);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::SmsWriteArgs& lhs, const ::android::hardware::radio::V1_0::SmsWriteArgs& rhs) {
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.pdu != rhs.pdu) {
        return false;
    }
    if (lhs.smsc != rhs.smsc) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::SmsWriteArgs& lhs,const ::android::hardware::radio::V1_0::SmsWriteArgs& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaSmsAddress& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".digitMode = ";
    os += ::android::hardware::radio::V1_0::toString(o.digitMode);
    os += ", .numberMode = ";
    os += ::android::hardware::radio::V1_0::toString(o.numberMode);
    os += ", .numberType = ";
    os += ::android::hardware::radio::V1_0::toString(o.numberType);
    os += ", .numberPlan = ";
    os += ::android::hardware::radio::V1_0::toString(o.numberPlan);
    os += ", .digits = ";
    os += ::android::hardware::toString(o.digits);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaSmsAddress& lhs, const ::android::hardware::radio::V1_0::CdmaSmsAddress& rhs) {
    if (lhs.digitMode != rhs.digitMode) {
        return false;
    }
    if (lhs.numberMode != rhs.numberMode) {
        return false;
    }
    if (lhs.numberType != rhs.numberType) {
        return false;
    }
    if (lhs.numberPlan != rhs.numberPlan) {
        return false;
    }
    if (lhs.digits != rhs.digits) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaSmsAddress& lhs,const ::android::hardware::radio::V1_0::CdmaSmsAddress& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaSmsSubaddress& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".subaddressType = ";
    os += ::android::hardware::radio::V1_0::toString(o.subaddressType);
    os += ", .odd = ";
    os += ::android::hardware::toString(o.odd);
    os += ", .digits = ";
    os += ::android::hardware::toString(o.digits);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaSmsSubaddress& lhs, const ::android::hardware::radio::V1_0::CdmaSmsSubaddress& rhs) {
    if (lhs.subaddressType != rhs.subaddressType) {
        return false;
    }
    if (lhs.odd != rhs.odd) {
        return false;
    }
    if (lhs.digits != rhs.digits) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaSmsSubaddress& lhs,const ::android::hardware::radio::V1_0::CdmaSmsSubaddress& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaSmsMessage& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".teleserviceId = ";
    os += ::android::hardware::toString(o.teleserviceId);
    os += ", .isServicePresent = ";
    os += ::android::hardware::toString(o.isServicePresent);
    os += ", .serviceCategory = ";
    os += ::android::hardware::toString(o.serviceCategory);
    os += ", .address = ";
    os += ::android::hardware::radio::V1_0::toString(o.address);
    os += ", .subAddress = ";
    os += ::android::hardware::radio::V1_0::toString(o.subAddress);
    os += ", .bearerData = ";
    os += ::android::hardware::toString(o.bearerData);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaSmsMessage& lhs, const ::android::hardware::radio::V1_0::CdmaSmsMessage& rhs) {
    if (lhs.teleserviceId != rhs.teleserviceId) {
        return false;
    }
    if (lhs.isServicePresent != rhs.isServicePresent) {
        return false;
    }
    if (lhs.serviceCategory != rhs.serviceCategory) {
        return false;
    }
    if (lhs.address != rhs.address) {
        return false;
    }
    if (lhs.subAddress != rhs.subAddress) {
        return false;
    }
    if (lhs.bearerData != rhs.bearerData) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaSmsMessage& lhs,const ::android::hardware::radio::V1_0::CdmaSmsMessage& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaSmsAck& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".errorClass = ";
    os += ::android::hardware::radio::V1_0::toString(o.errorClass);
    os += ", .smsCauseCode = ";
    os += ::android::hardware::toString(o.smsCauseCode);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaSmsAck& lhs, const ::android::hardware::radio::V1_0::CdmaSmsAck& rhs) {
    if (lhs.errorClass != rhs.errorClass) {
        return false;
    }
    if (lhs.smsCauseCode != rhs.smsCauseCode) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaSmsAck& lhs,const ::android::hardware::radio::V1_0::CdmaSmsAck& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaBroadcastSmsConfigInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".serviceCategory = ";
    os += ::android::hardware::toString(o.serviceCategory);
    os += ", .language = ";
    os += ::android::hardware::toString(o.language);
    os += ", .selected = ";
    os += ::android::hardware::toString(o.selected);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaBroadcastSmsConfigInfo& lhs, const ::android::hardware::radio::V1_0::CdmaBroadcastSmsConfigInfo& rhs) {
    if (lhs.serviceCategory != rhs.serviceCategory) {
        return false;
    }
    if (lhs.language != rhs.language) {
        return false;
    }
    if (lhs.selected != rhs.selected) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaBroadcastSmsConfigInfo& lhs,const ::android::hardware::radio::V1_0::CdmaBroadcastSmsConfigInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaSmsWriteArgs& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".status = ";
    os += ::android::hardware::radio::V1_0::toString(o.status);
    os += ", .message = ";
    os += ::android::hardware::radio::V1_0::toString(o.message);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaSmsWriteArgs& lhs, const ::android::hardware::radio::V1_0::CdmaSmsWriteArgs& rhs) {
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.message != rhs.message) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaSmsWriteArgs& lhs,const ::android::hardware::radio::V1_0::CdmaSmsWriteArgs& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".fromServiceId = ";
    os += ::android::hardware::toString(o.fromServiceId);
    os += ", .toServiceId = ";
    os += ::android::hardware::toString(o.toServiceId);
    os += ", .fromCodeScheme = ";
    os += ::android::hardware::toString(o.fromCodeScheme);
    os += ", .toCodeScheme = ";
    os += ::android::hardware::toString(o.toCodeScheme);
    os += ", .selected = ";
    os += ::android::hardware::toString(o.selected);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo& lhs, const ::android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo& rhs) {
    if (lhs.fromServiceId != rhs.fromServiceId) {
        return false;
    }
    if (lhs.toServiceId != rhs.toServiceId) {
        return false;
    }
    if (lhs.fromCodeScheme != rhs.fromCodeScheme) {
        return false;
    }
    if (lhs.toCodeScheme != rhs.toCodeScheme) {
        return false;
    }
    if (lhs.selected != rhs.selected) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo& lhs,const ::android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CellIdentityGsm& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".mcc = ";
    os += ::android::hardware::toString(o.mcc);
    os += ", .mnc = ";
    os += ::android::hardware::toString(o.mnc);
    os += ", .lac = ";
    os += ::android::hardware::toString(o.lac);
    os += ", .cid = ";
    os += ::android::hardware::toString(o.cid);
    os += ", .arfcn = ";
    os += ::android::hardware::toString(o.arfcn);
    os += ", .bsic = ";
    os += ::android::hardware::toString(o.bsic);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CellIdentityGsm& lhs, const ::android::hardware::radio::V1_0::CellIdentityGsm& rhs) {
    if (lhs.mcc != rhs.mcc) {
        return false;
    }
    if (lhs.mnc != rhs.mnc) {
        return false;
    }
    if (lhs.lac != rhs.lac) {
        return false;
    }
    if (lhs.cid != rhs.cid) {
        return false;
    }
    if (lhs.arfcn != rhs.arfcn) {
        return false;
    }
    if (lhs.bsic != rhs.bsic) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CellIdentityGsm& lhs,const ::android::hardware::radio::V1_0::CellIdentityGsm& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CellIdentityWcdma& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".mcc = ";
    os += ::android::hardware::toString(o.mcc);
    os += ", .mnc = ";
    os += ::android::hardware::toString(o.mnc);
    os += ", .lac = ";
    os += ::android::hardware::toString(o.lac);
    os += ", .cid = ";
    os += ::android::hardware::toString(o.cid);
    os += ", .psc = ";
    os += ::android::hardware::toString(o.psc);
    os += ", .uarfcn = ";
    os += ::android::hardware::toString(o.uarfcn);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CellIdentityWcdma& lhs, const ::android::hardware::radio::V1_0::CellIdentityWcdma& rhs) {
    if (lhs.mcc != rhs.mcc) {
        return false;
    }
    if (lhs.mnc != rhs.mnc) {
        return false;
    }
    if (lhs.lac != rhs.lac) {
        return false;
    }
    if (lhs.cid != rhs.cid) {
        return false;
    }
    if (lhs.psc != rhs.psc) {
        return false;
    }
    if (lhs.uarfcn != rhs.uarfcn) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CellIdentityWcdma& lhs,const ::android::hardware::radio::V1_0::CellIdentityWcdma& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CellIdentityCdma& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".networkId = ";
    os += ::android::hardware::toString(o.networkId);
    os += ", .systemId = ";
    os += ::android::hardware::toString(o.systemId);
    os += ", .baseStationId = ";
    os += ::android::hardware::toString(o.baseStationId);
    os += ", .longitude = ";
    os += ::android::hardware::toString(o.longitude);
    os += ", .latitude = ";
    os += ::android::hardware::toString(o.latitude);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CellIdentityCdma& lhs, const ::android::hardware::radio::V1_0::CellIdentityCdma& rhs) {
    if (lhs.networkId != rhs.networkId) {
        return false;
    }
    if (lhs.systemId != rhs.systemId) {
        return false;
    }
    if (lhs.baseStationId != rhs.baseStationId) {
        return false;
    }
    if (lhs.longitude != rhs.longitude) {
        return false;
    }
    if (lhs.latitude != rhs.latitude) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CellIdentityCdma& lhs,const ::android::hardware::radio::V1_0::CellIdentityCdma& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CellIdentityLte& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".mcc = ";
    os += ::android::hardware::toString(o.mcc);
    os += ", .mnc = ";
    os += ::android::hardware::toString(o.mnc);
    os += ", .ci = ";
    os += ::android::hardware::toString(o.ci);
    os += ", .pci = ";
    os += ::android::hardware::toString(o.pci);
    os += ", .tac = ";
    os += ::android::hardware::toString(o.tac);
    os += ", .earfcn = ";
    os += ::android::hardware::toString(o.earfcn);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CellIdentityLte& lhs, const ::android::hardware::radio::V1_0::CellIdentityLte& rhs) {
    if (lhs.mcc != rhs.mcc) {
        return false;
    }
    if (lhs.mnc != rhs.mnc) {
        return false;
    }
    if (lhs.ci != rhs.ci) {
        return false;
    }
    if (lhs.pci != rhs.pci) {
        return false;
    }
    if (lhs.tac != rhs.tac) {
        return false;
    }
    if (lhs.earfcn != rhs.earfcn) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CellIdentityLte& lhs,const ::android::hardware::radio::V1_0::CellIdentityLte& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CellIdentityTdscdma& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".mcc = ";
    os += ::android::hardware::toString(o.mcc);
    os += ", .mnc = ";
    os += ::android::hardware::toString(o.mnc);
    os += ", .lac = ";
    os += ::android::hardware::toString(o.lac);
    os += ", .cid = ";
    os += ::android::hardware::toString(o.cid);
    os += ", .cpid = ";
    os += ::android::hardware::toString(o.cpid);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CellIdentityTdscdma& lhs, const ::android::hardware::radio::V1_0::CellIdentityTdscdma& rhs) {
    if (lhs.mcc != rhs.mcc) {
        return false;
    }
    if (lhs.mnc != rhs.mnc) {
        return false;
    }
    if (lhs.lac != rhs.lac) {
        return false;
    }
    if (lhs.cid != rhs.cid) {
        return false;
    }
    if (lhs.cpid != rhs.cpid) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CellIdentityTdscdma& lhs,const ::android::hardware::radio::V1_0::CellIdentityTdscdma& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CellInfoGsm& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cellIdentityGsm = ";
    os += ::android::hardware::radio::V1_0::toString(o.cellIdentityGsm);
    os += ", .signalStrengthGsm = ";
    os += ::android::hardware::radio::V1_0::toString(o.signalStrengthGsm);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CellInfoGsm& lhs, const ::android::hardware::radio::V1_0::CellInfoGsm& rhs) {
    if (lhs.cellIdentityGsm != rhs.cellIdentityGsm) {
        return false;
    }
    if (lhs.signalStrengthGsm != rhs.signalStrengthGsm) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CellInfoGsm& lhs,const ::android::hardware::radio::V1_0::CellInfoGsm& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CellInfoWcdma& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cellIdentityWcdma = ";
    os += ::android::hardware::radio::V1_0::toString(o.cellIdentityWcdma);
    os += ", .signalStrengthWcdma = ";
    os += ::android::hardware::radio::V1_0::toString(o.signalStrengthWcdma);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CellInfoWcdma& lhs, const ::android::hardware::radio::V1_0::CellInfoWcdma& rhs) {
    if (lhs.cellIdentityWcdma != rhs.cellIdentityWcdma) {
        return false;
    }
    if (lhs.signalStrengthWcdma != rhs.signalStrengthWcdma) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CellInfoWcdma& lhs,const ::android::hardware::radio::V1_0::CellInfoWcdma& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CellInfoCdma& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cellIdentityCdma = ";
    os += ::android::hardware::radio::V1_0::toString(o.cellIdentityCdma);
    os += ", .signalStrengthCdma = ";
    os += ::android::hardware::radio::V1_0::toString(o.signalStrengthCdma);
    os += ", .signalStrengthEvdo = ";
    os += ::android::hardware::radio::V1_0::toString(o.signalStrengthEvdo);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CellInfoCdma& lhs, const ::android::hardware::radio::V1_0::CellInfoCdma& rhs) {
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

static inline bool operator!=(const ::android::hardware::radio::V1_0::CellInfoCdma& lhs,const ::android::hardware::radio::V1_0::CellInfoCdma& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CellInfoLte& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cellIdentityLte = ";
    os += ::android::hardware::radio::V1_0::toString(o.cellIdentityLte);
    os += ", .signalStrengthLte = ";
    os += ::android::hardware::radio::V1_0::toString(o.signalStrengthLte);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CellInfoLte& lhs, const ::android::hardware::radio::V1_0::CellInfoLte& rhs) {
    if (lhs.cellIdentityLte != rhs.cellIdentityLte) {
        return false;
    }
    if (lhs.signalStrengthLte != rhs.signalStrengthLte) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CellInfoLte& lhs,const ::android::hardware::radio::V1_0::CellInfoLte& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CellInfoTdscdma& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cellIdentityTdscdma = ";
    os += ::android::hardware::radio::V1_0::toString(o.cellIdentityTdscdma);
    os += ", .signalStrengthTdscdma = ";
    os += ::android::hardware::radio::V1_0::toString(o.signalStrengthTdscdma);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CellInfoTdscdma& lhs, const ::android::hardware::radio::V1_0::CellInfoTdscdma& rhs) {
    if (lhs.cellIdentityTdscdma != rhs.cellIdentityTdscdma) {
        return false;
    }
    if (lhs.signalStrengthTdscdma != rhs.signalStrengthTdscdma) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CellInfoTdscdma& lhs,const ::android::hardware::radio::V1_0::CellInfoTdscdma& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CellInfo& o) {
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
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CellInfo& lhs, const ::android::hardware::radio::V1_0::CellInfo& rhs) {
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
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CellInfo& lhs,const ::android::hardware::radio::V1_0::CellInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CellIdentity& o) {
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

static inline bool operator==(const ::android::hardware::radio::V1_0::CellIdentity& lhs, const ::android::hardware::radio::V1_0::CellIdentity& rhs) {
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

static inline bool operator!=(const ::android::hardware::radio::V1_0::CellIdentity& lhs,const ::android::hardware::radio::V1_0::CellIdentity& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::VoiceRegStateResult& o) {
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
    os += ::android::hardware::radio::V1_0::toString(o.cellIdentity);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::VoiceRegStateResult& lhs, const ::android::hardware::radio::V1_0::VoiceRegStateResult& rhs) {
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

static inline bool operator!=(const ::android::hardware::radio::V1_0::VoiceRegStateResult& lhs,const ::android::hardware::radio::V1_0::VoiceRegStateResult& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::DataRegStateResult& o) {
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
    os += ::android::hardware::radio::V1_0::toString(o.cellIdentity);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::DataRegStateResult& lhs, const ::android::hardware::radio::V1_0::DataRegStateResult& rhs) {
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

static inline bool operator!=(const ::android::hardware::radio::V1_0::DataRegStateResult& lhs,const ::android::hardware::radio::V1_0::DataRegStateResult& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::GsmSmsMessage& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".smscPdu = ";
    os += ::android::hardware::toString(o.smscPdu);
    os += ", .pdu = ";
    os += ::android::hardware::toString(o.pdu);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::GsmSmsMessage& lhs, const ::android::hardware::radio::V1_0::GsmSmsMessage& rhs) {
    if (lhs.smscPdu != rhs.smscPdu) {
        return false;
    }
    if (lhs.pdu != rhs.pdu) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::GsmSmsMessage& lhs,const ::android::hardware::radio::V1_0::GsmSmsMessage& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::ImsSmsMessage& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".tech = ";
    os += ::android::hardware::radio::V1_0::toString(o.tech);
    os += ", .retry = ";
    os += ::android::hardware::toString(o.retry);
    os += ", .messageRef = ";
    os += ::android::hardware::toString(o.messageRef);
    os += ", .cdmaMessage = ";
    os += ::android::hardware::toString(o.cdmaMessage);
    os += ", .gsmMessage = ";
    os += ::android::hardware::toString(o.gsmMessage);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::ImsSmsMessage& lhs, const ::android::hardware::radio::V1_0::ImsSmsMessage& rhs) {
    if (lhs.tech != rhs.tech) {
        return false;
    }
    if (lhs.retry != rhs.retry) {
        return false;
    }
    if (lhs.messageRef != rhs.messageRef) {
        return false;
    }
    if (lhs.cdmaMessage != rhs.cdmaMessage) {
        return false;
    }
    if (lhs.gsmMessage != rhs.gsmMessage) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::ImsSmsMessage& lhs,const ::android::hardware::radio::V1_0::ImsSmsMessage& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::SimApdu& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".sessionId = ";
    os += ::android::hardware::toString(o.sessionId);
    os += ", .cla = ";
    os += ::android::hardware::toString(o.cla);
    os += ", .instruction = ";
    os += ::android::hardware::toString(o.instruction);
    os += ", .p1 = ";
    os += ::android::hardware::toString(o.p1);
    os += ", .p2 = ";
    os += ::android::hardware::toString(o.p2);
    os += ", .p3 = ";
    os += ::android::hardware::toString(o.p3);
    os += ", .data = ";
    os += ::android::hardware::toString(o.data);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::SimApdu& lhs, const ::android::hardware::radio::V1_0::SimApdu& rhs) {
    if (lhs.sessionId != rhs.sessionId) {
        return false;
    }
    if (lhs.cla != rhs.cla) {
        return false;
    }
    if (lhs.instruction != rhs.instruction) {
        return false;
    }
    if (lhs.p1 != rhs.p1) {
        return false;
    }
    if (lhs.p2 != rhs.p2) {
        return false;
    }
    if (lhs.p3 != rhs.p3) {
        return false;
    }
    if (lhs.data != rhs.data) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::SimApdu& lhs,const ::android::hardware::radio::V1_0::SimApdu& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::NvWriteItem& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".itemId = ";
    os += ::android::hardware::radio::V1_0::toString(o.itemId);
    os += ", .value = ";
    os += ::android::hardware::toString(o.value);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::NvWriteItem& lhs, const ::android::hardware::radio::V1_0::NvWriteItem& rhs) {
    if (lhs.itemId != rhs.itemId) {
        return false;
    }
    if (lhs.value != rhs.value) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::NvWriteItem& lhs,const ::android::hardware::radio::V1_0::NvWriteItem& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::SelectUiccSub& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".slot = ";
    os += ::android::hardware::toString(o.slot);
    os += ", .appIndex = ";
    os += ::android::hardware::toString(o.appIndex);
    os += ", .subType = ";
    os += ::android::hardware::radio::V1_0::toString(o.subType);
    os += ", .actStatus = ";
    os += ::android::hardware::radio::V1_0::toString(o.actStatus);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::SelectUiccSub& lhs, const ::android::hardware::radio::V1_0::SelectUiccSub& rhs) {
    if (lhs.slot != rhs.slot) {
        return false;
    }
    if (lhs.appIndex != rhs.appIndex) {
        return false;
    }
    if (lhs.subType != rhs.subType) {
        return false;
    }
    if (lhs.actStatus != rhs.actStatus) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::SelectUiccSub& lhs,const ::android::hardware::radio::V1_0::SelectUiccSub& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::HardwareConfigModem& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".rilModel = ";
    os += ::android::hardware::toString(o.rilModel);
    os += ", .rat = ";
    os += ::android::hardware::toString(o.rat);
    os += ", .maxVoice = ";
    os += ::android::hardware::toString(o.maxVoice);
    os += ", .maxData = ";
    os += ::android::hardware::toString(o.maxData);
    os += ", .maxStandby = ";
    os += ::android::hardware::toString(o.maxStandby);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::HardwareConfigModem& lhs, const ::android::hardware::radio::V1_0::HardwareConfigModem& rhs) {
    if (lhs.rilModel != rhs.rilModel) {
        return false;
    }
    if (lhs.rat != rhs.rat) {
        return false;
    }
    if (lhs.maxVoice != rhs.maxVoice) {
        return false;
    }
    if (lhs.maxData != rhs.maxData) {
        return false;
    }
    if (lhs.maxStandby != rhs.maxStandby) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::HardwareConfigModem& lhs,const ::android::hardware::radio::V1_0::HardwareConfigModem& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::HardwareConfigSim& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".modemUuid = ";
    os += ::android::hardware::toString(o.modemUuid);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::HardwareConfigSim& lhs, const ::android::hardware::radio::V1_0::HardwareConfigSim& rhs) {
    if (lhs.modemUuid != rhs.modemUuid) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::HardwareConfigSim& lhs,const ::android::hardware::radio::V1_0::HardwareConfigSim& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::HardwareConfig& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".type = ";
    os += ::android::hardware::radio::V1_0::toString(o.type);
    os += ", .uuid = ";
    os += ::android::hardware::toString(o.uuid);
    os += ", .state = ";
    os += ::android::hardware::radio::V1_0::toString(o.state);
    os += ", .modem = ";
    os += ::android::hardware::toString(o.modem);
    os += ", .sim = ";
    os += ::android::hardware::toString(o.sim);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::HardwareConfig& lhs, const ::android::hardware::radio::V1_0::HardwareConfig& rhs) {
    if (lhs.type != rhs.type) {
        return false;
    }
    if (lhs.uuid != rhs.uuid) {
        return false;
    }
    if (lhs.state != rhs.state) {
        return false;
    }
    if (lhs.modem != rhs.modem) {
        return false;
    }
    if (lhs.sim != rhs.sim) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::HardwareConfig& lhs,const ::android::hardware::radio::V1_0::HardwareConfig& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::DataProfileInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".profileId = ";
    os += ::android::hardware::radio::V1_0::toString(o.profileId);
    os += ", .apn = ";
    os += ::android::hardware::toString(o.apn);
    os += ", .protocol = ";
    os += ::android::hardware::toString(o.protocol);
    os += ", .roamingProtocol = ";
    os += ::android::hardware::toString(o.roamingProtocol);
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
    os += ::android::hardware::radio::V1_0::toString<::android::hardware::radio::V1_0::ApnTypes>(o.supportedApnTypesBitmap);
    os += ", .bearerBitmap = ";
    os += ::android::hardware::radio::V1_0::toString<::android::hardware::radio::V1_0::RadioAccessFamily>(o.bearerBitmap);
    os += ", .mtu = ";
    os += ::android::hardware::toString(o.mtu);
    os += ", .mvnoType = ";
    os += ::android::hardware::radio::V1_0::toString(o.mvnoType);
    os += ", .mvnoMatchData = ";
    os += ::android::hardware::toString(o.mvnoMatchData);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::DataProfileInfo& lhs, const ::android::hardware::radio::V1_0::DataProfileInfo& rhs) {
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
    if (lhs.mvnoType != rhs.mvnoType) {
        return false;
    }
    if (lhs.mvnoMatchData != rhs.mvnoMatchData) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::DataProfileInfo& lhs,const ::android::hardware::radio::V1_0::DataProfileInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::RadioCapability& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".session = ";
    os += ::android::hardware::toString(o.session);
    os += ", .phase = ";
    os += ::android::hardware::radio::V1_0::toString(o.phase);
    os += ", .raf = ";
    os += ::android::hardware::radio::V1_0::toString<::android::hardware::radio::V1_0::RadioAccessFamily>(o.raf);
    os += ", .logicalModemUuid = ";
    os += ::android::hardware::toString(o.logicalModemUuid);
    os += ", .status = ";
    os += ::android::hardware::radio::V1_0::toString(o.status);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::RadioCapability& lhs, const ::android::hardware::radio::V1_0::RadioCapability& rhs) {
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

static inline bool operator!=(const ::android::hardware::radio::V1_0::RadioCapability& lhs,const ::android::hardware::radio::V1_0::RadioCapability& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::LceStatusInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".lceStatus = ";
    os += ::android::hardware::radio::V1_0::toString(o.lceStatus);
    os += ", .actualIntervalMs = ";
    os += ::android::hardware::toString(o.actualIntervalMs);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::LceStatusInfo& lhs, const ::android::hardware::radio::V1_0::LceStatusInfo& rhs) {
    if (lhs.lceStatus != rhs.lceStatus) {
        return false;
    }
    if (lhs.actualIntervalMs != rhs.actualIntervalMs) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::LceStatusInfo& lhs,const ::android::hardware::radio::V1_0::LceStatusInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::LceDataInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".lastHopCapacityKbps = ";
    os += ::android::hardware::toString(o.lastHopCapacityKbps);
    os += ", .confidenceLevel = ";
    os += ::android::hardware::toString(o.confidenceLevel);
    os += ", .lceSuspended = ";
    os += ::android::hardware::toString(o.lceSuspended);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::LceDataInfo& lhs, const ::android::hardware::radio::V1_0::LceDataInfo& rhs) {
    if (lhs.lastHopCapacityKbps != rhs.lastHopCapacityKbps) {
        return false;
    }
    if (lhs.confidenceLevel != rhs.confidenceLevel) {
        return false;
    }
    if (lhs.lceSuspended != rhs.lceSuspended) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::LceDataInfo& lhs,const ::android::hardware::radio::V1_0::LceDataInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::ActivityStatsInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".sleepModeTimeMs = ";
    os += ::android::hardware::toString(o.sleepModeTimeMs);
    os += ", .idleModeTimeMs = ";
    os += ::android::hardware::toString(o.idleModeTimeMs);
    os += ", .txmModetimeMs = ";
    os += ::android::hardware::toString(o.txmModetimeMs);
    os += ", .rxModeTimeMs = ";
    os += ::android::hardware::toString(o.rxModeTimeMs);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::ActivityStatsInfo& lhs, const ::android::hardware::radio::V1_0::ActivityStatsInfo& rhs) {
    if (lhs.sleepModeTimeMs != rhs.sleepModeTimeMs) {
        return false;
    }
    if (lhs.idleModeTimeMs != rhs.idleModeTimeMs) {
        return false;
    }
    if (lhs.txmModetimeMs != rhs.txmModetimeMs) {
        return false;
    }
    if (lhs.rxModeTimeMs != rhs.rxModeTimeMs) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::ActivityStatsInfo& lhs,const ::android::hardware::radio::V1_0::ActivityStatsInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::Carrier& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".mcc = ";
    os += ::android::hardware::toString(o.mcc);
    os += ", .mnc = ";
    os += ::android::hardware::toString(o.mnc);
    os += ", .matchType = ";
    os += ::android::hardware::radio::V1_0::toString(o.matchType);
    os += ", .matchData = ";
    os += ::android::hardware::toString(o.matchData);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::Carrier& lhs, const ::android::hardware::radio::V1_0::Carrier& rhs) {
    if (lhs.mcc != rhs.mcc) {
        return false;
    }
    if (lhs.mnc != rhs.mnc) {
        return false;
    }
    if (lhs.matchType != rhs.matchType) {
        return false;
    }
    if (lhs.matchData != rhs.matchData) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::Carrier& lhs,const ::android::hardware::radio::V1_0::Carrier& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CarrierRestrictions& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".allowedCarriers = ";
    os += ::android::hardware::toString(o.allowedCarriers);
    os += ", .excludedCarriers = ";
    os += ::android::hardware::toString(o.excludedCarriers);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CarrierRestrictions& lhs, const ::android::hardware::radio::V1_0::CarrierRestrictions& rhs) {
    if (lhs.allowedCarriers != rhs.allowedCarriers) {
        return false;
    }
    if (lhs.excludedCarriers != rhs.excludedCarriers) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CarrierRestrictions& lhs,const ::android::hardware::radio::V1_0::CarrierRestrictions& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::SuppSvcNotification& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".isMT = ";
    os += ::android::hardware::toString(o.isMT);
    os += ", .code = ";
    os += ::android::hardware::toString(o.code);
    os += ", .index = ";
    os += ::android::hardware::toString(o.index);
    os += ", .type = ";
    os += ::android::hardware::toString(o.type);
    os += ", .number = ";
    os += ::android::hardware::toString(o.number);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::SuppSvcNotification& lhs, const ::android::hardware::radio::V1_0::SuppSvcNotification& rhs) {
    if (lhs.isMT != rhs.isMT) {
        return false;
    }
    if (lhs.code != rhs.code) {
        return false;
    }
    if (lhs.index != rhs.index) {
        return false;
    }
    if (lhs.type != rhs.type) {
        return false;
    }
    if (lhs.number != rhs.number) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::SuppSvcNotification& lhs,const ::android::hardware::radio::V1_0::SuppSvcNotification& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::SimRefreshResult& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".type = ";
    os += ::android::hardware::radio::V1_0::toString(o.type);
    os += ", .efId = ";
    os += ::android::hardware::toString(o.efId);
    os += ", .aid = ";
    os += ::android::hardware::toString(o.aid);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::SimRefreshResult& lhs, const ::android::hardware::radio::V1_0::SimRefreshResult& rhs) {
    if (lhs.type != rhs.type) {
        return false;
    }
    if (lhs.efId != rhs.efId) {
        return false;
    }
    if (lhs.aid != rhs.aid) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::SimRefreshResult& lhs,const ::android::hardware::radio::V1_0::SimRefreshResult& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaSignalInfoRecord& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".isPresent = ";
    os += ::android::hardware::toString(o.isPresent);
    os += ", .signalType = ";
    os += ::android::hardware::toString(o.signalType);
    os += ", .alertPitch = ";
    os += ::android::hardware::toString(o.alertPitch);
    os += ", .signal = ";
    os += ::android::hardware::toString(o.signal);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaSignalInfoRecord& lhs, const ::android::hardware::radio::V1_0::CdmaSignalInfoRecord& rhs) {
    if (lhs.isPresent != rhs.isPresent) {
        return false;
    }
    if (lhs.signalType != rhs.signalType) {
        return false;
    }
    if (lhs.alertPitch != rhs.alertPitch) {
        return false;
    }
    if (lhs.signal != rhs.signal) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaSignalInfoRecord& lhs,const ::android::hardware::radio::V1_0::CdmaSignalInfoRecord& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaCallWaiting& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".number = ";
    os += ::android::hardware::toString(o.number);
    os += ", .numberPresentation = ";
    os += ::android::hardware::radio::V1_0::toString(o.numberPresentation);
    os += ", .name = ";
    os += ::android::hardware::toString(o.name);
    os += ", .signalInfoRecord = ";
    os += ::android::hardware::radio::V1_0::toString(o.signalInfoRecord);
    os += ", .numberType = ";
    os += ::android::hardware::radio::V1_0::toString(o.numberType);
    os += ", .numberPlan = ";
    os += ::android::hardware::radio::V1_0::toString(o.numberPlan);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaCallWaiting& lhs, const ::android::hardware::radio::V1_0::CdmaCallWaiting& rhs) {
    if (lhs.number != rhs.number) {
        return false;
    }
    if (lhs.numberPresentation != rhs.numberPresentation) {
        return false;
    }
    if (lhs.name != rhs.name) {
        return false;
    }
    if (lhs.signalInfoRecord != rhs.signalInfoRecord) {
        return false;
    }
    if (lhs.numberType != rhs.numberType) {
        return false;
    }
    if (lhs.numberPlan != rhs.numberPlan) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaCallWaiting& lhs,const ::android::hardware::radio::V1_0::CdmaCallWaiting& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaDisplayInfoRecord& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".alphaBuf = ";
    os += ::android::hardware::toString(o.alphaBuf);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaDisplayInfoRecord& lhs, const ::android::hardware::radio::V1_0::CdmaDisplayInfoRecord& rhs) {
    if (lhs.alphaBuf != rhs.alphaBuf) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaDisplayInfoRecord& lhs,const ::android::hardware::radio::V1_0::CdmaDisplayInfoRecord& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaNumberInfoRecord& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".number = ";
    os += ::android::hardware::toString(o.number);
    os += ", .numberType = ";
    os += ::android::hardware::toString(o.numberType);
    os += ", .numberPlan = ";
    os += ::android::hardware::toString(o.numberPlan);
    os += ", .pi = ";
    os += ::android::hardware::toString(o.pi);
    os += ", .si = ";
    os += ::android::hardware::toString(o.si);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaNumberInfoRecord& lhs, const ::android::hardware::radio::V1_0::CdmaNumberInfoRecord& rhs) {
    if (lhs.number != rhs.number) {
        return false;
    }
    if (lhs.numberType != rhs.numberType) {
        return false;
    }
    if (lhs.numberPlan != rhs.numberPlan) {
        return false;
    }
    if (lhs.pi != rhs.pi) {
        return false;
    }
    if (lhs.si != rhs.si) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaNumberInfoRecord& lhs,const ::android::hardware::radio::V1_0::CdmaNumberInfoRecord& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaRedirectingNumberInfoRecord& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".redirectingNumber = ";
    os += ::android::hardware::radio::V1_0::toString(o.redirectingNumber);
    os += ", .redirectingReason = ";
    os += ::android::hardware::radio::V1_0::toString(o.redirectingReason);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaRedirectingNumberInfoRecord& lhs, const ::android::hardware::radio::V1_0::CdmaRedirectingNumberInfoRecord& rhs) {
    if (lhs.redirectingNumber != rhs.redirectingNumber) {
        return false;
    }
    if (lhs.redirectingReason != rhs.redirectingReason) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaRedirectingNumberInfoRecord& lhs,const ::android::hardware::radio::V1_0::CdmaRedirectingNumberInfoRecord& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaLineControlInfoRecord& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".lineCtrlPolarityIncluded = ";
    os += ::android::hardware::toString(o.lineCtrlPolarityIncluded);
    os += ", .lineCtrlToggle = ";
    os += ::android::hardware::toString(o.lineCtrlToggle);
    os += ", .lineCtrlReverse = ";
    os += ::android::hardware::toString(o.lineCtrlReverse);
    os += ", .lineCtrlPowerDenial = ";
    os += ::android::hardware::toString(o.lineCtrlPowerDenial);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaLineControlInfoRecord& lhs, const ::android::hardware::radio::V1_0::CdmaLineControlInfoRecord& rhs) {
    if (lhs.lineCtrlPolarityIncluded != rhs.lineCtrlPolarityIncluded) {
        return false;
    }
    if (lhs.lineCtrlToggle != rhs.lineCtrlToggle) {
        return false;
    }
    if (lhs.lineCtrlReverse != rhs.lineCtrlReverse) {
        return false;
    }
    if (lhs.lineCtrlPowerDenial != rhs.lineCtrlPowerDenial) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaLineControlInfoRecord& lhs,const ::android::hardware::radio::V1_0::CdmaLineControlInfoRecord& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaT53ClirInfoRecord& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cause = ";
    os += ::android::hardware::toString(o.cause);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaT53ClirInfoRecord& lhs, const ::android::hardware::radio::V1_0::CdmaT53ClirInfoRecord& rhs) {
    if (lhs.cause != rhs.cause) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaT53ClirInfoRecord& lhs,const ::android::hardware::radio::V1_0::CdmaT53ClirInfoRecord& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaT53AudioControlInfoRecord& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".upLink = ";
    os += ::android::hardware::toString(o.upLink);
    os += ", .downLink = ";
    os += ::android::hardware::toString(o.downLink);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaT53AudioControlInfoRecord& lhs, const ::android::hardware::radio::V1_0::CdmaT53AudioControlInfoRecord& rhs) {
    if (lhs.upLink != rhs.upLink) {
        return false;
    }
    if (lhs.downLink != rhs.downLink) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaT53AudioControlInfoRecord& lhs,const ::android::hardware::radio::V1_0::CdmaT53AudioControlInfoRecord& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaInformationRecord& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".name = ";
    os += ::android::hardware::radio::V1_0::toString(o.name);
    os += ", .display = ";
    os += ::android::hardware::toString(o.display);
    os += ", .number = ";
    os += ::android::hardware::toString(o.number);
    os += ", .signal = ";
    os += ::android::hardware::toString(o.signal);
    os += ", .redir = ";
    os += ::android::hardware::toString(o.redir);
    os += ", .lineCtrl = ";
    os += ::android::hardware::toString(o.lineCtrl);
    os += ", .clir = ";
    os += ::android::hardware::toString(o.clir);
    os += ", .audioCtrl = ";
    os += ::android::hardware::toString(o.audioCtrl);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaInformationRecord& lhs, const ::android::hardware::radio::V1_0::CdmaInformationRecord& rhs) {
    if (lhs.name != rhs.name) {
        return false;
    }
    if (lhs.display != rhs.display) {
        return false;
    }
    if (lhs.number != rhs.number) {
        return false;
    }
    if (lhs.signal != rhs.signal) {
        return false;
    }
    if (lhs.redir != rhs.redir) {
        return false;
    }
    if (lhs.lineCtrl != rhs.lineCtrl) {
        return false;
    }
    if (lhs.clir != rhs.clir) {
        return false;
    }
    if (lhs.audioCtrl != rhs.audioCtrl) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaInformationRecord& lhs,const ::android::hardware::radio::V1_0::CdmaInformationRecord& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CdmaInformationRecords& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".infoRec = ";
    os += ::android::hardware::toString(o.infoRec);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CdmaInformationRecords& lhs, const ::android::hardware::radio::V1_0::CdmaInformationRecords& rhs) {
    if (lhs.infoRec != rhs.infoRec) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CdmaInformationRecords& lhs,const ::android::hardware::radio::V1_0::CdmaInformationRecords& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::CfData& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cfInfo = ";
    os += ::android::hardware::toString(o.cfInfo);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::CfData& lhs, const ::android::hardware::radio::V1_0::CfData& rhs) {
    if (lhs.cfInfo != rhs.cfInfo) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::CfData& lhs,const ::android::hardware::radio::V1_0::CfData& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::SsInfoData& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".ssInfo = ";
    os += ::android::hardware::toString(o.ssInfo);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::SsInfoData& lhs, const ::android::hardware::radio::V1_0::SsInfoData& rhs) {
    if (lhs.ssInfo != rhs.ssInfo) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::SsInfoData& lhs,const ::android::hardware::radio::V1_0::SsInfoData& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::StkCcUnsolSsResult& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".serviceType = ";
    os += ::android::hardware::radio::V1_0::toString(o.serviceType);
    os += ", .requestType = ";
    os += ::android::hardware::radio::V1_0::toString(o.requestType);
    os += ", .teleserviceType = ";
    os += ::android::hardware::radio::V1_0::toString(o.teleserviceType);
    os += ", .serviceClass = ";
    os += ::android::hardware::radio::V1_0::toString<::android::hardware::radio::V1_0::SuppServiceClass>(o.serviceClass);
    os += ", .result = ";
    os += ::android::hardware::radio::V1_0::toString(o.result);
    os += ", .ssInfo = ";
    os += ::android::hardware::toString(o.ssInfo);
    os += ", .cfData = ";
    os += ::android::hardware::toString(o.cfData);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::StkCcUnsolSsResult& lhs, const ::android::hardware::radio::V1_0::StkCcUnsolSsResult& rhs) {
    if (lhs.serviceType != rhs.serviceType) {
        return false;
    }
    if (lhs.requestType != rhs.requestType) {
        return false;
    }
    if (lhs.teleserviceType != rhs.teleserviceType) {
        return false;
    }
    if (lhs.serviceClass != rhs.serviceClass) {
        return false;
    }
    if (lhs.result != rhs.result) {
        return false;
    }
    if (lhs.ssInfo != rhs.ssInfo) {
        return false;
    }
    if (lhs.cfData != rhs.cfData) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::StkCcUnsolSsResult& lhs,const ::android::hardware::radio::V1_0::StkCcUnsolSsResult& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_0::PcoDataInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cid = ";
    os += ::android::hardware::toString(o.cid);
    os += ", .bearerProto = ";
    os += ::android::hardware::toString(o.bearerProto);
    os += ", .pcoId = ";
    os += ::android::hardware::toString(o.pcoId);
    os += ", .contents = ";
    os += ::android::hardware::toString(o.contents);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_0::PcoDataInfo& lhs, const ::android::hardware::radio::V1_0::PcoDataInfo& rhs) {
    if (lhs.cid != rhs.cid) {
        return false;
    }
    if (lhs.bearerProto != rhs.bearerProto) {
        return false;
    }
    if (lhs.pcoId != rhs.pcoId) {
        return false;
    }
    if (lhs.contents != rhs.contents) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_0::PcoDataInfo& lhs,const ::android::hardware::radio::V1_0::PcoDataInfo& rhs){
    return !(lhs == rhs);
}


}  // namespace V1_0
}  // namespace radio
}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::RadioConst>
{
    const ::android::hardware::radio::V1_0::RadioConst* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::RadioConst* end() { return begin() + 13; }
    private:
    static const ::android::hardware::radio::V1_0::RadioConst* static_begin() {
        static const ::android::hardware::radio::V1_0::RadioConst kVals[13] {
            ::android::hardware::radio::V1_0::RadioConst::CDMA_ALPHA_INFO_BUFFER_LENGTH,
            ::android::hardware::radio::V1_0::RadioConst::CDMA_NUMBER_INFO_BUFFER_LENGTH,
            ::android::hardware::radio::V1_0::RadioConst::MAX_RILDS,
            ::android::hardware::radio::V1_0::RadioConst::MAX_SOCKET_NAME_LENGTH,
            ::android::hardware::radio::V1_0::RadioConst::MAX_CLIENT_ID_LENGTH,
            ::android::hardware::radio::V1_0::RadioConst::MAX_DEBUG_SOCKET_NAME_LENGTH,
            ::android::hardware::radio::V1_0::RadioConst::MAX_QEMU_PIPE_NAME_LENGTH,
            ::android::hardware::radio::V1_0::RadioConst::MAX_UUID_LENGTH,
            ::android::hardware::radio::V1_0::RadioConst::CARD_MAX_APPS,
            ::android::hardware::radio::V1_0::RadioConst::CDMA_MAX_NUMBER_OF_INFO_RECS,
            ::android::hardware::radio::V1_0::RadioConst::SS_INFO_MAX,
            ::android::hardware::radio::V1_0::RadioConst::NUM_SERVICE_CLASSES,
            ::android::hardware::radio::V1_0::RadioConst::NUM_TX_POWER_LEVELS,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::RadioCdmaSmsConst>
{
    const ::android::hardware::radio::V1_0::RadioCdmaSmsConst* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::RadioCdmaSmsConst* end() { return begin() + 15; }
    private:
    static const ::android::hardware::radio::V1_0::RadioCdmaSmsConst* static_begin() {
        static const ::android::hardware::radio::V1_0::RadioCdmaSmsConst kVals[15] {
            ::android::hardware::radio::V1_0::RadioCdmaSmsConst::ADDRESS_MAX,
            ::android::hardware::radio::V1_0::RadioCdmaSmsConst::SUBADDRESS_MAX,
            ::android::hardware::radio::V1_0::RadioCdmaSmsConst::BEARER_DATA_MAX,
            ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_MAX_SND_SIZE,
            ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_EO_DATA_SEGMENT_MAX,
            ::android::hardware::radio::V1_0::RadioCdmaSmsConst::MAX_UD_HEADERS,
            ::android::hardware::radio::V1_0::RadioCdmaSmsConst::USER_DATA_MAX,
            ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_LARGE_PIC_SIZE,
            ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_SMALL_PIC_SIZE,
            ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_VAR_PIC_SIZE,
            ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_ANIM_NUM_BITMAPS,
            ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_LARGE_BITMAP_SIZE,
            ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_SMALL_BITMAP_SIZE,
            ::android::hardware::radio::V1_0::RadioCdmaSmsConst::UDH_OTHER_SIZE,
            ::android::hardware::radio::V1_0::RadioCdmaSmsConst::IP_ADDRESS_SIZE,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::RadioError>
{
    const ::android::hardware::radio::V1_0::RadioError* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::RadioError* end() { return begin() + 84; }
    private:
    static const ::android::hardware::radio::V1_0::RadioError* static_begin() {
        static const ::android::hardware::radio::V1_0::RadioError kVals[84] {
            ::android::hardware::radio::V1_0::RadioError::NONE,
            ::android::hardware::radio::V1_0::RadioError::RADIO_NOT_AVAILABLE,
            ::android::hardware::radio::V1_0::RadioError::GENERIC_FAILURE,
            ::android::hardware::radio::V1_0::RadioError::PASSWORD_INCORRECT,
            ::android::hardware::radio::V1_0::RadioError::SIM_PIN2,
            ::android::hardware::radio::V1_0::RadioError::SIM_PUK2,
            ::android::hardware::radio::V1_0::RadioError::REQUEST_NOT_SUPPORTED,
            ::android::hardware::radio::V1_0::RadioError::CANCELLED,
            ::android::hardware::radio::V1_0::RadioError::OP_NOT_ALLOWED_DURING_VOICE_CALL,
            ::android::hardware::radio::V1_0::RadioError::OP_NOT_ALLOWED_BEFORE_REG_TO_NW,
            ::android::hardware::radio::V1_0::RadioError::SMS_SEND_FAIL_RETRY,
            ::android::hardware::radio::V1_0::RadioError::SIM_ABSENT,
            ::android::hardware::radio::V1_0::RadioError::SUBSCRIPTION_NOT_AVAILABLE,
            ::android::hardware::radio::V1_0::RadioError::MODE_NOT_SUPPORTED,
            ::android::hardware::radio::V1_0::RadioError::FDN_CHECK_FAILURE,
            ::android::hardware::radio::V1_0::RadioError::ILLEGAL_SIM_OR_ME,
            ::android::hardware::radio::V1_0::RadioError::MISSING_RESOURCE,
            ::android::hardware::radio::V1_0::RadioError::NO_SUCH_ELEMENT,
            ::android::hardware::radio::V1_0::RadioError::DIAL_MODIFIED_TO_USSD,
            ::android::hardware::radio::V1_0::RadioError::DIAL_MODIFIED_TO_SS,
            ::android::hardware::radio::V1_0::RadioError::DIAL_MODIFIED_TO_DIAL,
            ::android::hardware::radio::V1_0::RadioError::USSD_MODIFIED_TO_DIAL,
            ::android::hardware::radio::V1_0::RadioError::USSD_MODIFIED_TO_SS,
            ::android::hardware::radio::V1_0::RadioError::USSD_MODIFIED_TO_USSD,
            ::android::hardware::radio::V1_0::RadioError::SS_MODIFIED_TO_DIAL,
            ::android::hardware::radio::V1_0::RadioError::SS_MODIFIED_TO_USSD,
            ::android::hardware::radio::V1_0::RadioError::SUBSCRIPTION_NOT_SUPPORTED,
            ::android::hardware::radio::V1_0::RadioError::SS_MODIFIED_TO_SS,
            ::android::hardware::radio::V1_0::RadioError::LCE_NOT_SUPPORTED,
            ::android::hardware::radio::V1_0::RadioError::NO_MEMORY,
            ::android::hardware::radio::V1_0::RadioError::INTERNAL_ERR,
            ::android::hardware::radio::V1_0::RadioError::SYSTEM_ERR,
            ::android::hardware::radio::V1_0::RadioError::MODEM_ERR,
            ::android::hardware::radio::V1_0::RadioError::INVALID_STATE,
            ::android::hardware::radio::V1_0::RadioError::NO_RESOURCES,
            ::android::hardware::radio::V1_0::RadioError::SIM_ERR,
            ::android::hardware::radio::V1_0::RadioError::INVALID_ARGUMENTS,
            ::android::hardware::radio::V1_0::RadioError::INVALID_SIM_STATE,
            ::android::hardware::radio::V1_0::RadioError::INVALID_MODEM_STATE,
            ::android::hardware::radio::V1_0::RadioError::INVALID_CALL_ID,
            ::android::hardware::radio::V1_0::RadioError::NO_SMS_TO_ACK,
            ::android::hardware::radio::V1_0::RadioError::NETWORK_ERR,
            ::android::hardware::radio::V1_0::RadioError::REQUEST_RATE_LIMITED,
            ::android::hardware::radio::V1_0::RadioError::SIM_BUSY,
            ::android::hardware::radio::V1_0::RadioError::SIM_FULL,
            ::android::hardware::radio::V1_0::RadioError::NETWORK_REJECT,
            ::android::hardware::radio::V1_0::RadioError::OPERATION_NOT_ALLOWED,
            ::android::hardware::radio::V1_0::RadioError::EMPTY_RECORD,
            ::android::hardware::radio::V1_0::RadioError::INVALID_SMS_FORMAT,
            ::android::hardware::radio::V1_0::RadioError::ENCODING_ERR,
            ::android::hardware::radio::V1_0::RadioError::INVALID_SMSC_ADDRESS,
            ::android::hardware::radio::V1_0::RadioError::NO_SUCH_ENTRY,
            ::android::hardware::radio::V1_0::RadioError::NETWORK_NOT_READY,
            ::android::hardware::radio::V1_0::RadioError::NOT_PROVISIONED,
            ::android::hardware::radio::V1_0::RadioError::NO_SUBSCRIPTION,
            ::android::hardware::radio::V1_0::RadioError::NO_NETWORK_FOUND,
            ::android::hardware::radio::V1_0::RadioError::DEVICE_IN_USE,
            ::android::hardware::radio::V1_0::RadioError::ABORTED,
            ::android::hardware::radio::V1_0::RadioError::INVALID_RESPONSE,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_1,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_2,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_3,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_4,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_5,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_6,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_7,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_8,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_9,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_10,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_11,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_12,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_13,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_14,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_15,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_16,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_17,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_18,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_19,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_20,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_21,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_22,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_23,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_24,
            ::android::hardware::radio::V1_0::RadioError::OEM_ERROR_25,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::RadioResponseType>
{
    const ::android::hardware::radio::V1_0::RadioResponseType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::RadioResponseType* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_0::RadioResponseType* static_begin() {
        static const ::android::hardware::radio::V1_0::RadioResponseType kVals[3] {
            ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED,
            ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED_ACK,
            ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED_ACK_EXP,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::RadioIndicationType>
{
    const ::android::hardware::radio::V1_0::RadioIndicationType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::RadioIndicationType* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_0::RadioIndicationType* static_begin() {
        static const ::android::hardware::radio::V1_0::RadioIndicationType kVals[2] {
            ::android::hardware::radio::V1_0::RadioIndicationType::UNSOLICITED,
            ::android::hardware::radio::V1_0::RadioIndicationType::UNSOLICITED_ACK_EXP,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::RestrictedState>
{
    const ::android::hardware::radio::V1_0::RestrictedState* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::RestrictedState* end() { return begin() + 5; }
    private:
    static const ::android::hardware::radio::V1_0::RestrictedState* static_begin() {
        static const ::android::hardware::radio::V1_0::RestrictedState kVals[5] {
            ::android::hardware::radio::V1_0::RestrictedState::NONE,
            ::android::hardware::radio::V1_0::RestrictedState::CS_EMERGENCY,
            ::android::hardware::radio::V1_0::RestrictedState::CS_NORMAL,
            ::android::hardware::radio::V1_0::RestrictedState::CS_ALL,
            ::android::hardware::radio::V1_0::RestrictedState::PS_ALL,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CardState>
{
    const ::android::hardware::radio::V1_0::CardState* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CardState* end() { return begin() + 4; }
    private:
    static const ::android::hardware::radio::V1_0::CardState* static_begin() {
        static const ::android::hardware::radio::V1_0::CardState kVals[4] {
            ::android::hardware::radio::V1_0::CardState::ABSENT,
            ::android::hardware::radio::V1_0::CardState::PRESENT,
            ::android::hardware::radio::V1_0::CardState::ERROR,
            ::android::hardware::radio::V1_0::CardState::RESTRICTED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::PinState>
{
    const ::android::hardware::radio::V1_0::PinState* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::PinState* end() { return begin() + 6; }
    private:
    static const ::android::hardware::radio::V1_0::PinState* static_begin() {
        static const ::android::hardware::radio::V1_0::PinState kVals[6] {
            ::android::hardware::radio::V1_0::PinState::UNKNOWN,
            ::android::hardware::radio::V1_0::PinState::ENABLED_NOT_VERIFIED,
            ::android::hardware::radio::V1_0::PinState::ENABLED_VERIFIED,
            ::android::hardware::radio::V1_0::PinState::DISABLED,
            ::android::hardware::radio::V1_0::PinState::ENABLED_BLOCKED,
            ::android::hardware::radio::V1_0::PinState::ENABLED_PERM_BLOCKED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::AppType>
{
    const ::android::hardware::radio::V1_0::AppType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::AppType* end() { return begin() + 6; }
    private:
    static const ::android::hardware::radio::V1_0::AppType* static_begin() {
        static const ::android::hardware::radio::V1_0::AppType kVals[6] {
            ::android::hardware::radio::V1_0::AppType::UNKNOWN,
            ::android::hardware::radio::V1_0::AppType::SIM,
            ::android::hardware::radio::V1_0::AppType::USIM,
            ::android::hardware::radio::V1_0::AppType::RUIM,
            ::android::hardware::radio::V1_0::AppType::CSIM,
            ::android::hardware::radio::V1_0::AppType::ISIM,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::AppState>
{
    const ::android::hardware::radio::V1_0::AppState* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::AppState* end() { return begin() + 6; }
    private:
    static const ::android::hardware::radio::V1_0::AppState* static_begin() {
        static const ::android::hardware::radio::V1_0::AppState kVals[6] {
            ::android::hardware::radio::V1_0::AppState::UNKNOWN,
            ::android::hardware::radio::V1_0::AppState::DETECTED,
            ::android::hardware::radio::V1_0::AppState::PIN,
            ::android::hardware::radio::V1_0::AppState::PUK,
            ::android::hardware::radio::V1_0::AppState::SUBSCRIPTION_PERSO,
            ::android::hardware::radio::V1_0::AppState::READY,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::PersoSubstate>
{
    const ::android::hardware::radio::V1_0::PersoSubstate* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::PersoSubstate* end() { return begin() + 25; }
    private:
    static const ::android::hardware::radio::V1_0::PersoSubstate* static_begin() {
        static const ::android::hardware::radio::V1_0::PersoSubstate kVals[25] {
            ::android::hardware::radio::V1_0::PersoSubstate::UNKNOWN,
            ::android::hardware::radio::V1_0::PersoSubstate::IN_PROGRESS,
            ::android::hardware::radio::V1_0::PersoSubstate::READY,
            ::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK,
            ::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK_SUBSET,
            ::android::hardware::radio::V1_0::PersoSubstate::SIM_CORPORATE,
            ::android::hardware::radio::V1_0::PersoSubstate::SIM_SERVICE_PROVIDER,
            ::android::hardware::radio::V1_0::PersoSubstate::SIM_SIM,
            ::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK_PUK,
            ::android::hardware::radio::V1_0::PersoSubstate::SIM_NETWORK_SUBSET_PUK,
            ::android::hardware::radio::V1_0::PersoSubstate::SIM_CORPORATE_PUK,
            ::android::hardware::radio::V1_0::PersoSubstate::SIM_SERVICE_PROVIDER_PUK,
            ::android::hardware::radio::V1_0::PersoSubstate::SIM_SIM_PUK,
            ::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK1,
            ::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK2,
            ::android::hardware::radio::V1_0::PersoSubstate::RUIM_HRPD,
            ::android::hardware::radio::V1_0::PersoSubstate::RUIM_CORPORATE,
            ::android::hardware::radio::V1_0::PersoSubstate::RUIM_SERVICE_PROVIDER,
            ::android::hardware::radio::V1_0::PersoSubstate::RUIM_RUIM,
            ::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK1_PUK,
            ::android::hardware::radio::V1_0::PersoSubstate::RUIM_NETWORK2_PUK,
            ::android::hardware::radio::V1_0::PersoSubstate::RUIM_HRPD_PUK,
            ::android::hardware::radio::V1_0::PersoSubstate::RUIM_CORPORATE_PUK,
            ::android::hardware::radio::V1_0::PersoSubstate::RUIM_SERVICE_PROVIDER_PUK,
            ::android::hardware::radio::V1_0::PersoSubstate::RUIM_RUIM_PUK,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::RadioState>
{
    const ::android::hardware::radio::V1_0::RadioState* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::RadioState* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_0::RadioState* static_begin() {
        static const ::android::hardware::radio::V1_0::RadioState kVals[3] {
            ::android::hardware::radio::V1_0::RadioState::OFF,
            ::android::hardware::radio::V1_0::RadioState::UNAVAILABLE,
            ::android::hardware::radio::V1_0::RadioState::ON,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::SapConnectRsp>
{
    const ::android::hardware::radio::V1_0::SapConnectRsp* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::SapConnectRsp* end() { return begin() + 5; }
    private:
    static const ::android::hardware::radio::V1_0::SapConnectRsp* static_begin() {
        static const ::android::hardware::radio::V1_0::SapConnectRsp kVals[5] {
            ::android::hardware::radio::V1_0::SapConnectRsp::SUCCESS,
            ::android::hardware::radio::V1_0::SapConnectRsp::CONNECT_FAILURE,
            ::android::hardware::radio::V1_0::SapConnectRsp::MSG_SIZE_TOO_LARGE,
            ::android::hardware::radio::V1_0::SapConnectRsp::MSG_SIZE_TOO_SMALL,
            ::android::hardware::radio::V1_0::SapConnectRsp::CONNECT_OK_CALL_ONGOING,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::SapDisconnectType>
{
    const ::android::hardware::radio::V1_0::SapDisconnectType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::SapDisconnectType* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_0::SapDisconnectType* static_begin() {
        static const ::android::hardware::radio::V1_0::SapDisconnectType kVals[2] {
            ::android::hardware::radio::V1_0::SapDisconnectType::GRACEFUL,
            ::android::hardware::radio::V1_0::SapDisconnectType::IMMEDIATE,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::SapApduType>
{
    const ::android::hardware::radio::V1_0::SapApduType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::SapApduType* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_0::SapApduType* static_begin() {
        static const ::android::hardware::radio::V1_0::SapApduType kVals[2] {
            ::android::hardware::radio::V1_0::SapApduType::APDU,
            ::android::hardware::radio::V1_0::SapApduType::APDU7816,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::SapResultCode>
{
    const ::android::hardware::radio::V1_0::SapResultCode* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::SapResultCode* end() { return begin() + 8; }
    private:
    static const ::android::hardware::radio::V1_0::SapResultCode* static_begin() {
        static const ::android::hardware::radio::V1_0::SapResultCode kVals[8] {
            ::android::hardware::radio::V1_0::SapResultCode::SUCCESS,
            ::android::hardware::radio::V1_0::SapResultCode::GENERIC_FAILURE,
            ::android::hardware::radio::V1_0::SapResultCode::CARD_NOT_ACCESSSIBLE,
            ::android::hardware::radio::V1_0::SapResultCode::CARD_ALREADY_POWERED_OFF,
            ::android::hardware::radio::V1_0::SapResultCode::CARD_REMOVED,
            ::android::hardware::radio::V1_0::SapResultCode::CARD_ALREADY_POWERED_ON,
            ::android::hardware::radio::V1_0::SapResultCode::DATA_NOT_AVAILABLE,
            ::android::hardware::radio::V1_0::SapResultCode::NOT_SUPPORTED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::SapStatus>
{
    const ::android::hardware::radio::V1_0::SapStatus* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::SapStatus* end() { return begin() + 6; }
    private:
    static const ::android::hardware::radio::V1_0::SapStatus* static_begin() {
        static const ::android::hardware::radio::V1_0::SapStatus kVals[6] {
            ::android::hardware::radio::V1_0::SapStatus::UNKNOWN_ERROR,
            ::android::hardware::radio::V1_0::SapStatus::CARD_RESET,
            ::android::hardware::radio::V1_0::SapStatus::CARD_NOT_ACCESSIBLE,
            ::android::hardware::radio::V1_0::SapStatus::CARD_REMOVED,
            ::android::hardware::radio::V1_0::SapStatus::CARD_INSERTED,
            ::android::hardware::radio::V1_0::SapStatus::RECOVERED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::SapTransferProtocol>
{
    const ::android::hardware::radio::V1_0::SapTransferProtocol* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::SapTransferProtocol* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_0::SapTransferProtocol* static_begin() {
        static const ::android::hardware::radio::V1_0::SapTransferProtocol kVals[2] {
            ::android::hardware::radio::V1_0::SapTransferProtocol::T0,
            ::android::hardware::radio::V1_0::SapTransferProtocol::T1,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CallState>
{
    const ::android::hardware::radio::V1_0::CallState* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CallState* end() { return begin() + 6; }
    private:
    static const ::android::hardware::radio::V1_0::CallState* static_begin() {
        static const ::android::hardware::radio::V1_0::CallState kVals[6] {
            ::android::hardware::radio::V1_0::CallState::ACTIVE,
            ::android::hardware::radio::V1_0::CallState::HOLDING,
            ::android::hardware::radio::V1_0::CallState::DIALING,
            ::android::hardware::radio::V1_0::CallState::ALERTING,
            ::android::hardware::radio::V1_0::CallState::INCOMING,
            ::android::hardware::radio::V1_0::CallState::WAITING,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::UusType>
{
    const ::android::hardware::radio::V1_0::UusType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::UusType* end() { return begin() + 7; }
    private:
    static const ::android::hardware::radio::V1_0::UusType* static_begin() {
        static const ::android::hardware::radio::V1_0::UusType kVals[7] {
            ::android::hardware::radio::V1_0::UusType::TYPE1_IMPLICIT,
            ::android::hardware::radio::V1_0::UusType::TYPE1_REQUIRED,
            ::android::hardware::radio::V1_0::UusType::TYPE1_NOT_REQUIRED,
            ::android::hardware::radio::V1_0::UusType::TYPE2_REQUIRED,
            ::android::hardware::radio::V1_0::UusType::TYPE2_NOT_REQUIRED,
            ::android::hardware::radio::V1_0::UusType::TYPE3_REQUIRED,
            ::android::hardware::radio::V1_0::UusType::TYPE3_NOT_REQUIRED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::UusDcs>
{
    const ::android::hardware::radio::V1_0::UusDcs* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::UusDcs* end() { return begin() + 5; }
    private:
    static const ::android::hardware::radio::V1_0::UusDcs* static_begin() {
        static const ::android::hardware::radio::V1_0::UusDcs kVals[5] {
            ::android::hardware::radio::V1_0::UusDcs::USP,
            ::android::hardware::radio::V1_0::UusDcs::OSIHLP,
            ::android::hardware::radio::V1_0::UusDcs::X244,
            ::android::hardware::radio::V1_0::UusDcs::RMCF,
            ::android::hardware::radio::V1_0::UusDcs::IA5C,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CallPresentation>
{
    const ::android::hardware::radio::V1_0::CallPresentation* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CallPresentation* end() { return begin() + 4; }
    private:
    static const ::android::hardware::radio::V1_0::CallPresentation* static_begin() {
        static const ::android::hardware::radio::V1_0::CallPresentation kVals[4] {
            ::android::hardware::radio::V1_0::CallPresentation::ALLOWED,
            ::android::hardware::radio::V1_0::CallPresentation::RESTRICTED,
            ::android::hardware::radio::V1_0::CallPresentation::UNKNOWN,
            ::android::hardware::radio::V1_0::CallPresentation::PAYPHONE,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::Clir>
{
    const ::android::hardware::radio::V1_0::Clir* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::Clir* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_0::Clir* static_begin() {
        static const ::android::hardware::radio::V1_0::Clir kVals[3] {
            ::android::hardware::radio::V1_0::Clir::DEFAULT,
            ::android::hardware::radio::V1_0::Clir::INVOCATION,
            ::android::hardware::radio::V1_0::Clir::SUPPRESSION,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::LastCallFailCause>
{
    const ::android::hardware::radio::V1_0::LastCallFailCause* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::LastCallFailCause* end() { return begin() + 96; }
    private:
    static const ::android::hardware::radio::V1_0::LastCallFailCause* static_begin() {
        static const ::android::hardware::radio::V1_0::LastCallFailCause kVals[96] {
            ::android::hardware::radio::V1_0::LastCallFailCause::UNOBTAINABLE_NUMBER,
            ::android::hardware::radio::V1_0::LastCallFailCause::NO_ROUTE_TO_DESTINATION,
            ::android::hardware::radio::V1_0::LastCallFailCause::CHANNEL_UNACCEPTABLE,
            ::android::hardware::radio::V1_0::LastCallFailCause::OPERATOR_DETERMINED_BARRING,
            ::android::hardware::radio::V1_0::LastCallFailCause::NORMAL,
            ::android::hardware::radio::V1_0::LastCallFailCause::BUSY,
            ::android::hardware::radio::V1_0::LastCallFailCause::NO_USER_RESPONDING,
            ::android::hardware::radio::V1_0::LastCallFailCause::NO_ANSWER_FROM_USER,
            ::android::hardware::radio::V1_0::LastCallFailCause::CALL_REJECTED,
            ::android::hardware::radio::V1_0::LastCallFailCause::NUMBER_CHANGED,
            ::android::hardware::radio::V1_0::LastCallFailCause::PREEMPTION,
            ::android::hardware::radio::V1_0::LastCallFailCause::DESTINATION_OUT_OF_ORDER,
            ::android::hardware::radio::V1_0::LastCallFailCause::INVALID_NUMBER_FORMAT,
            ::android::hardware::radio::V1_0::LastCallFailCause::FACILITY_REJECTED,
            ::android::hardware::radio::V1_0::LastCallFailCause::RESP_TO_STATUS_ENQUIRY,
            ::android::hardware::radio::V1_0::LastCallFailCause::NORMAL_UNSPECIFIED,
            ::android::hardware::radio::V1_0::LastCallFailCause::CONGESTION,
            ::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_OUT_OF_ORDER,
            ::android::hardware::radio::V1_0::LastCallFailCause::TEMPORARY_FAILURE,
            ::android::hardware::radio::V1_0::LastCallFailCause::SWITCHING_EQUIPMENT_CONGESTION,
            ::android::hardware::radio::V1_0::LastCallFailCause::ACCESS_INFORMATION_DISCARDED,
            ::android::hardware::radio::V1_0::LastCallFailCause::REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE,
            ::android::hardware::radio::V1_0::LastCallFailCause::RESOURCES_UNAVAILABLE_OR_UNSPECIFIED,
            ::android::hardware::radio::V1_0::LastCallFailCause::QOS_UNAVAILABLE,
            ::android::hardware::radio::V1_0::LastCallFailCause::REQUESTED_FACILITY_NOT_SUBSCRIBED,
            ::android::hardware::radio::V1_0::LastCallFailCause::INCOMING_CALLS_BARRED_WITHIN_CUG,
            ::android::hardware::radio::V1_0::LastCallFailCause::BEARER_CAPABILITY_NOT_AUTHORIZED,
            ::android::hardware::radio::V1_0::LastCallFailCause::BEARER_CAPABILITY_UNAVAILABLE,
            ::android::hardware::radio::V1_0::LastCallFailCause::SERVICE_OPTION_NOT_AVAILABLE,
            ::android::hardware::radio::V1_0::LastCallFailCause::BEARER_SERVICE_NOT_IMPLEMENTED,
            ::android::hardware::radio::V1_0::LastCallFailCause::ACM_LIMIT_EXCEEDED,
            ::android::hardware::radio::V1_0::LastCallFailCause::REQUESTED_FACILITY_NOT_IMPLEMENTED,
            ::android::hardware::radio::V1_0::LastCallFailCause::ONLY_DIGITAL_INFORMATION_BEARER_AVAILABLE,
            ::android::hardware::radio::V1_0::LastCallFailCause::SERVICE_OR_OPTION_NOT_IMPLEMENTED,
            ::android::hardware::radio::V1_0::LastCallFailCause::INVALID_TRANSACTION_IDENTIFIER,
            ::android::hardware::radio::V1_0::LastCallFailCause::USER_NOT_MEMBER_OF_CUG,
            ::android::hardware::radio::V1_0::LastCallFailCause::INCOMPATIBLE_DESTINATION,
            ::android::hardware::radio::V1_0::LastCallFailCause::INVALID_TRANSIT_NW_SELECTION,
            ::android::hardware::radio::V1_0::LastCallFailCause::SEMANTICALLY_INCORRECT_MESSAGE,
            ::android::hardware::radio::V1_0::LastCallFailCause::INVALID_MANDATORY_INFORMATION,
            ::android::hardware::radio::V1_0::LastCallFailCause::MESSAGE_TYPE_NON_IMPLEMENTED,
            ::android::hardware::radio::V1_0::LastCallFailCause::MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE,
            ::android::hardware::radio::V1_0::LastCallFailCause::INFORMATION_ELEMENT_NON_EXISTENT,
            ::android::hardware::radio::V1_0::LastCallFailCause::CONDITIONAL_IE_ERROR,
            ::android::hardware::radio::V1_0::LastCallFailCause::MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE,
            ::android::hardware::radio::V1_0::LastCallFailCause::RECOVERY_ON_TIMER_EXPIRED,
            ::android::hardware::radio::V1_0::LastCallFailCause::PROTOCOL_ERROR_UNSPECIFIED,
            ::android::hardware::radio::V1_0::LastCallFailCause::INTERWORKING_UNSPECIFIED,
            ::android::hardware::radio::V1_0::LastCallFailCause::CALL_BARRED,
            ::android::hardware::radio::V1_0::LastCallFailCause::FDN_BLOCKED,
            ::android::hardware::radio::V1_0::LastCallFailCause::IMSI_UNKNOWN_IN_VLR,
            ::android::hardware::radio::V1_0::LastCallFailCause::IMEI_NOT_ACCEPTED,
            ::android::hardware::radio::V1_0::LastCallFailCause::DIAL_MODIFIED_TO_USSD,
            ::android::hardware::radio::V1_0::LastCallFailCause::DIAL_MODIFIED_TO_SS,
            ::android::hardware::radio::V1_0::LastCallFailCause::DIAL_MODIFIED_TO_DIAL,
            ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_OFF,
            ::android::hardware::radio::V1_0::LastCallFailCause::OUT_OF_SERVICE,
            ::android::hardware::radio::V1_0::LastCallFailCause::NO_VALID_SIM,
            ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_INTERNAL_ERROR,
            ::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_RESP_TIMEOUT,
            ::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_REJECT,
            ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_ACCESS_FAILURE,
            ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_LINK_FAILURE,
            ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_LINK_LOST,
            ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_UPLINK_FAILURE,
            ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_SETUP_FAILURE,
            ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_RELEASE_NORMAL,
            ::android::hardware::radio::V1_0::LastCallFailCause::RADIO_RELEASE_ABNORMAL,
            ::android::hardware::radio::V1_0::LastCallFailCause::ACCESS_CLASS_BLOCKED,
            ::android::hardware::radio::V1_0::LastCallFailCause::NETWORK_DETACH,
            ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_LOCKED_UNTIL_POWER_CYCLE,
            ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_DROP,
            ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_INTERCEPT,
            ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_REORDER,
            ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_SO_REJECT,
            ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_RETRY_ORDER,
            ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_ACCESS_FAILURE,
            ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_PREEMPTED,
            ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_NOT_EMERGENCY,
            ::android::hardware::radio::V1_0::LastCallFailCause::CDMA_ACCESS_BLOCKED,
            ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_1,
            ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_2,
            ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_3,
            ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_4,
            ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_5,
            ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_6,
            ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_7,
            ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_8,
            ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_9,
            ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_10,
            ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_11,
            ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_12,
            ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_13,
            ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_14,
            ::android::hardware::radio::V1_0::LastCallFailCause::OEM_CAUSE_15,
            ::android::hardware::radio::V1_0::LastCallFailCause::ERROR_UNSPECIFIED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::DataCallFailCause>
{
    const ::android::hardware::radio::V1_0::DataCallFailCause* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::DataCallFailCause* end() { return begin() + 74; }
    private:
    static const ::android::hardware::radio::V1_0::DataCallFailCause* static_begin() {
        static const ::android::hardware::radio::V1_0::DataCallFailCause kVals[74] {
            ::android::hardware::radio::V1_0::DataCallFailCause::NONE,
            ::android::hardware::radio::V1_0::DataCallFailCause::OPERATOR_BARRED,
            ::android::hardware::radio::V1_0::DataCallFailCause::NAS_SIGNALLING,
            ::android::hardware::radio::V1_0::DataCallFailCause::INSUFFICIENT_RESOURCES,
            ::android::hardware::radio::V1_0::DataCallFailCause::MISSING_UKNOWN_APN,
            ::android::hardware::radio::V1_0::DataCallFailCause::UNKNOWN_PDP_ADDRESS_TYPE,
            ::android::hardware::radio::V1_0::DataCallFailCause::USER_AUTHENTICATION,
            ::android::hardware::radio::V1_0::DataCallFailCause::ACTIVATION_REJECT_GGSN,
            ::android::hardware::radio::V1_0::DataCallFailCause::ACTIVATION_REJECT_UNSPECIFIED,
            ::android::hardware::radio::V1_0::DataCallFailCause::SERVICE_OPTION_NOT_SUPPORTED,
            ::android::hardware::radio::V1_0::DataCallFailCause::SERVICE_OPTION_NOT_SUBSCRIBED,
            ::android::hardware::radio::V1_0::DataCallFailCause::SERVICE_OPTION_OUT_OF_ORDER,
            ::android::hardware::radio::V1_0::DataCallFailCause::NSAPI_IN_USE,
            ::android::hardware::radio::V1_0::DataCallFailCause::REGULAR_DEACTIVATION,
            ::android::hardware::radio::V1_0::DataCallFailCause::QOS_NOT_ACCEPTED,
            ::android::hardware::radio::V1_0::DataCallFailCause::NETWORK_FAILURE,
            ::android::hardware::radio::V1_0::DataCallFailCause::UMTS_REACTIVATION_REQ,
            ::android::hardware::radio::V1_0::DataCallFailCause::FEATURE_NOT_SUPP,
            ::android::hardware::radio::V1_0::DataCallFailCause::TFT_SEMANTIC_ERROR,
            ::android::hardware::radio::V1_0::DataCallFailCause::TFT_SYTAX_ERROR,
            ::android::hardware::radio::V1_0::DataCallFailCause::UNKNOWN_PDP_CONTEXT,
            ::android::hardware::radio::V1_0::DataCallFailCause::FILTER_SEMANTIC_ERROR,
            ::android::hardware::radio::V1_0::DataCallFailCause::FILTER_SYTAX_ERROR,
            ::android::hardware::radio::V1_0::DataCallFailCause::PDP_WITHOUT_ACTIVE_TFT,
            ::android::hardware::radio::V1_0::DataCallFailCause::ONLY_IPV4_ALLOWED,
            ::android::hardware::radio::V1_0::DataCallFailCause::ONLY_IPV6_ALLOWED,
            ::android::hardware::radio::V1_0::DataCallFailCause::ONLY_SINGLE_BEARER_ALLOWED,
            ::android::hardware::radio::V1_0::DataCallFailCause::ESM_INFO_NOT_RECEIVED,
            ::android::hardware::radio::V1_0::DataCallFailCause::PDN_CONN_DOES_NOT_EXIST,
            ::android::hardware::radio::V1_0::DataCallFailCause::MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED,
            ::android::hardware::radio::V1_0::DataCallFailCause::MAX_ACTIVE_PDP_CONTEXT_REACHED,
            ::android::hardware::radio::V1_0::DataCallFailCause::UNSUPPORTED_APN_IN_CURRENT_PLMN,
            ::android::hardware::radio::V1_0::DataCallFailCause::INVALID_TRANSACTION_ID,
            ::android::hardware::radio::V1_0::DataCallFailCause::MESSAGE_INCORRECT_SEMANTIC,
            ::android::hardware::radio::V1_0::DataCallFailCause::INVALID_MANDATORY_INFO,
            ::android::hardware::radio::V1_0::DataCallFailCause::MESSAGE_TYPE_UNSUPPORTED,
            ::android::hardware::radio::V1_0::DataCallFailCause::MSG_TYPE_NONCOMPATIBLE_STATE,
            ::android::hardware::radio::V1_0::DataCallFailCause::UNKNOWN_INFO_ELEMENT,
            ::android::hardware::radio::V1_0::DataCallFailCause::CONDITIONAL_IE_ERROR,
            ::android::hardware::radio::V1_0::DataCallFailCause::MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE,
            ::android::hardware::radio::V1_0::DataCallFailCause::PROTOCOL_ERRORS,
            ::android::hardware::radio::V1_0::DataCallFailCause::APN_TYPE_CONFLICT,
            ::android::hardware::radio::V1_0::DataCallFailCause::INVALID_PCSCF_ADDR,
            ::android::hardware::radio::V1_0::DataCallFailCause::INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN,
            ::android::hardware::radio::V1_0::DataCallFailCause::EMM_ACCESS_BARRED,
            ::android::hardware::radio::V1_0::DataCallFailCause::EMERGENCY_IFACE_ONLY,
            ::android::hardware::radio::V1_0::DataCallFailCause::IFACE_MISMATCH,
            ::android::hardware::radio::V1_0::DataCallFailCause::COMPANION_IFACE_IN_USE,
            ::android::hardware::radio::V1_0::DataCallFailCause::IP_ADDRESS_MISMATCH,
            ::android::hardware::radio::V1_0::DataCallFailCause::IFACE_AND_POL_FAMILY_MISMATCH,
            ::android::hardware::radio::V1_0::DataCallFailCause::EMM_ACCESS_BARRED_INFINITE_RETRY,
            ::android::hardware::radio::V1_0::DataCallFailCause::AUTH_FAILURE_ON_EMERGENCY_CALL,
            ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_1,
            ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_2,
            ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_3,
            ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_4,
            ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_5,
            ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_6,
            ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_7,
            ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_8,
            ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_9,
            ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_10,
            ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_11,
            ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_12,
            ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_13,
            ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_14,
            ::android::hardware::radio::V1_0::DataCallFailCause::OEM_DCFAILCAUSE_15,
            ::android::hardware::radio::V1_0::DataCallFailCause::VOICE_REGISTRATION_FAIL,
            ::android::hardware::radio::V1_0::DataCallFailCause::DATA_REGISTRATION_FAIL,
            ::android::hardware::radio::V1_0::DataCallFailCause::SIGNAL_LOST,
            ::android::hardware::radio::V1_0::DataCallFailCause::PREF_RADIO_TECH_CHANGED,
            ::android::hardware::radio::V1_0::DataCallFailCause::RADIO_POWER_OFF,
            ::android::hardware::radio::V1_0::DataCallFailCause::TETHERED_CALL_ACTIVE,
            ::android::hardware::radio::V1_0::DataCallFailCause::ERROR_UNSPECIFIED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::RegState>
{
    const ::android::hardware::radio::V1_0::RegState* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::RegState* end() { return begin() + 10; }
    private:
    static const ::android::hardware::radio::V1_0::RegState* static_begin() {
        static const ::android::hardware::radio::V1_0::RegState kVals[10] {
            ::android::hardware::radio::V1_0::RegState::NOT_REG_MT_NOT_SEARCHING_OP,
            ::android::hardware::radio::V1_0::RegState::REG_HOME,
            ::android::hardware::radio::V1_0::RegState::NOT_REG_MT_SEARCHING_OP,
            ::android::hardware::radio::V1_0::RegState::REG_DENIED,
            ::android::hardware::radio::V1_0::RegState::UNKNOWN,
            ::android::hardware::radio::V1_0::RegState::REG_ROAMING,
            ::android::hardware::radio::V1_0::RegState::NOT_REG_MT_NOT_SEARCHING_OP_EM,
            ::android::hardware::radio::V1_0::RegState::NOT_REG_MT_SEARCHING_OP_EM,
            ::android::hardware::radio::V1_0::RegState::REG_DENIED_EM,
            ::android::hardware::radio::V1_0::RegState::UNKNOWN_EM,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::RadioTechnology>
{
    const ::android::hardware::radio::V1_0::RadioTechnology* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::RadioTechnology* end() { return begin() + 20; }
    private:
    static const ::android::hardware::radio::V1_0::RadioTechnology* static_begin() {
        static const ::android::hardware::radio::V1_0::RadioTechnology kVals[20] {
            ::android::hardware::radio::V1_0::RadioTechnology::UNKNOWN,
            ::android::hardware::radio::V1_0::RadioTechnology::GPRS,
            ::android::hardware::radio::V1_0::RadioTechnology::EDGE,
            ::android::hardware::radio::V1_0::RadioTechnology::UMTS,
            ::android::hardware::radio::V1_0::RadioTechnology::IS95A,
            ::android::hardware::radio::V1_0::RadioTechnology::IS95B,
            ::android::hardware::radio::V1_0::RadioTechnology::ONE_X_RTT,
            ::android::hardware::radio::V1_0::RadioTechnology::EVDO_0,
            ::android::hardware::radio::V1_0::RadioTechnology::EVDO_A,
            ::android::hardware::radio::V1_0::RadioTechnology::HSDPA,
            ::android::hardware::radio::V1_0::RadioTechnology::HSUPA,
            ::android::hardware::radio::V1_0::RadioTechnology::HSPA,
            ::android::hardware::radio::V1_0::RadioTechnology::EVDO_B,
            ::android::hardware::radio::V1_0::RadioTechnology::EHRPD,
            ::android::hardware::radio::V1_0::RadioTechnology::LTE,
            ::android::hardware::radio::V1_0::RadioTechnology::HSPAP,
            ::android::hardware::radio::V1_0::RadioTechnology::GSM,
            ::android::hardware::radio::V1_0::RadioTechnology::TD_SCDMA,
            ::android::hardware::radio::V1_0::RadioTechnology::IWLAN,
            ::android::hardware::radio::V1_0::RadioTechnology::LTE_CA,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::DataProfileId>
{
    const ::android::hardware::radio::V1_0::DataProfileId* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::DataProfileId* end() { return begin() + 7; }
    private:
    static const ::android::hardware::radio::V1_0::DataProfileId* static_begin() {
        static const ::android::hardware::radio::V1_0::DataProfileId kVals[7] {
            ::android::hardware::radio::V1_0::DataProfileId::DEFAULT,
            ::android::hardware::radio::V1_0::DataProfileId::TETHERED,
            ::android::hardware::radio::V1_0::DataProfileId::IMS,
            ::android::hardware::radio::V1_0::DataProfileId::FOTA,
            ::android::hardware::radio::V1_0::DataProfileId::CBS,
            ::android::hardware::radio::V1_0::DataProfileId::OEM_BASE,
            ::android::hardware::radio::V1_0::DataProfileId::INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::SmsAcknowledgeFailCause>
{
    const ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause* static_begin() {
        static const ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause kVals[2] {
            ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause::MEMORY_CAPACITY_EXCEEDED,
            ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause::UNSPECIFIED_ERROR,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CallForwardInfoStatus>
{
    const ::android::hardware::radio::V1_0::CallForwardInfoStatus* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CallForwardInfoStatus* end() { return begin() + 5; }
    private:
    static const ::android::hardware::radio::V1_0::CallForwardInfoStatus* static_begin() {
        static const ::android::hardware::radio::V1_0::CallForwardInfoStatus kVals[5] {
            ::android::hardware::radio::V1_0::CallForwardInfoStatus::DISABLE,
            ::android::hardware::radio::V1_0::CallForwardInfoStatus::ENABLE,
            ::android::hardware::radio::V1_0::CallForwardInfoStatus::INTERROGATE,
            ::android::hardware::radio::V1_0::CallForwardInfoStatus::REGISTRATION,
            ::android::hardware::radio::V1_0::CallForwardInfoStatus::ERASURE,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::ClipStatus>
{
    const ::android::hardware::radio::V1_0::ClipStatus* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::ClipStatus* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_0::ClipStatus* static_begin() {
        static const ::android::hardware::radio::V1_0::ClipStatus kVals[3] {
            ::android::hardware::radio::V1_0::ClipStatus::CLIP_PROVISIONED,
            ::android::hardware::radio::V1_0::ClipStatus::CLIP_UNPROVISIONED,
            ::android::hardware::radio::V1_0::ClipStatus::UNKNOWN,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::SmsWriteArgsStatus>
{
    const ::android::hardware::radio::V1_0::SmsWriteArgsStatus* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::SmsWriteArgsStatus* end() { return begin() + 4; }
    private:
    static const ::android::hardware::radio::V1_0::SmsWriteArgsStatus* static_begin() {
        static const ::android::hardware::radio::V1_0::SmsWriteArgsStatus kVals[4] {
            ::android::hardware::radio::V1_0::SmsWriteArgsStatus::REC_UNREAD,
            ::android::hardware::radio::V1_0::SmsWriteArgsStatus::REC_READ,
            ::android::hardware::radio::V1_0::SmsWriteArgsStatus::STO_UNSENT,
            ::android::hardware::radio::V1_0::SmsWriteArgsStatus::STO_SENT,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::RadioBandMode>
{
    const ::android::hardware::radio::V1_0::RadioBandMode* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::RadioBandMode* end() { return begin() + 19; }
    private:
    static const ::android::hardware::radio::V1_0::RadioBandMode* static_begin() {
        static const ::android::hardware::radio::V1_0::RadioBandMode kVals[19] {
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_UNSPECIFIED,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_EURO,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_USA,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_JPN,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_AUS,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_AUS_2,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_CELL_800,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_PCS,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_JTACS,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_KOREA_PCS,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_5_450M,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_IMT2000,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_7_700M_2,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_8_1800M,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_9_900M,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_10_800M_2,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_EURO_PAMR_400M,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_AWS,
            ::android::hardware::radio::V1_0::RadioBandMode::BAND_MODE_USA_2500M,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::OperatorStatus>
{
    const ::android::hardware::radio::V1_0::OperatorStatus* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::OperatorStatus* end() { return begin() + 4; }
    private:
    static const ::android::hardware::radio::V1_0::OperatorStatus* static_begin() {
        static const ::android::hardware::radio::V1_0::OperatorStatus kVals[4] {
            ::android::hardware::radio::V1_0::OperatorStatus::UNKNOWN,
            ::android::hardware::radio::V1_0::OperatorStatus::AVAILABLE,
            ::android::hardware::radio::V1_0::OperatorStatus::CURRENT,
            ::android::hardware::radio::V1_0::OperatorStatus::FORBIDDEN,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::PreferredNetworkType>
{
    const ::android::hardware::radio::V1_0::PreferredNetworkType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::PreferredNetworkType* end() { return begin() + 23; }
    private:
    static const ::android::hardware::radio::V1_0::PreferredNetworkType* static_begin() {
        static const ::android::hardware::radio::V1_0::PreferredNetworkType kVals[23] {
            ::android::hardware::radio::V1_0::PreferredNetworkType::GSM_WCDMA,
            ::android::hardware::radio::V1_0::PreferredNetworkType::GSM_ONLY,
            ::android::hardware::radio::V1_0::PreferredNetworkType::WCDMA,
            ::android::hardware::radio::V1_0::PreferredNetworkType::GSM_WCDMA_AUTO,
            ::android::hardware::radio::V1_0::PreferredNetworkType::CDMA_EVDO_AUTO,
            ::android::hardware::radio::V1_0::PreferredNetworkType::CDMA_ONLY,
            ::android::hardware::radio::V1_0::PreferredNetworkType::EVDO_ONLY,
            ::android::hardware::radio::V1_0::PreferredNetworkType::GSM_WCDMA_CDMA_EVDO_AUTO,
            ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_CDMA_EVDO,
            ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_GSM_WCDMA,
            ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_CMDA_EVDO_GSM_WCDMA,
            ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_ONLY,
            ::android::hardware::radio::V1_0::PreferredNetworkType::LTE_WCDMA,
            ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_ONLY,
            ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_WCDMA,
            ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_LTE,
            ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM,
            ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_LTE,
            ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA,
            ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_WCDMA_LTE,
            ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA_LTE,
            ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO,
            ::android::hardware::radio::V1_0::PreferredNetworkType::TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CdmaSubscriptionSource>
{
    const ::android::hardware::radio::V1_0::CdmaSubscriptionSource* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CdmaSubscriptionSource* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_0::CdmaSubscriptionSource* static_begin() {
        static const ::android::hardware::radio::V1_0::CdmaSubscriptionSource kVals[2] {
            ::android::hardware::radio::V1_0::CdmaSubscriptionSource::RUIM_SIM,
            ::android::hardware::radio::V1_0::CdmaSubscriptionSource::NV,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CdmaRoamingType>
{
    const ::android::hardware::radio::V1_0::CdmaRoamingType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CdmaRoamingType* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_0::CdmaRoamingType* static_begin() {
        static const ::android::hardware::radio::V1_0::CdmaRoamingType kVals[3] {
            ::android::hardware::radio::V1_0::CdmaRoamingType::HOME_NETWORK,
            ::android::hardware::radio::V1_0::CdmaRoamingType::AFFILIATED_ROAM,
            ::android::hardware::radio::V1_0::CdmaRoamingType::ANY_ROAM,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::TtyMode>
{
    const ::android::hardware::radio::V1_0::TtyMode* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::TtyMode* end() { return begin() + 4; }
    private:
    static const ::android::hardware::radio::V1_0::TtyMode* static_begin() {
        static const ::android::hardware::radio::V1_0::TtyMode kVals[4] {
            ::android::hardware::radio::V1_0::TtyMode::OFF,
            ::android::hardware::radio::V1_0::TtyMode::FULL,
            ::android::hardware::radio::V1_0::TtyMode::HCO,
            ::android::hardware::radio::V1_0::TtyMode::VCO,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::NvItem>
{
    const ::android::hardware::radio::V1_0::NvItem* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::NvItem* end() { return begin() + 41; }
    private:
    static const ::android::hardware::radio::V1_0::NvItem* static_begin() {
        static const ::android::hardware::radio::V1_0::NvItem kVals[41] {
            ::android::hardware::radio::V1_0::NvItem::CDMA_MEID,
            ::android::hardware::radio::V1_0::NvItem::CDMA_MIN,
            ::android::hardware::radio::V1_0::NvItem::CDMA_MDN,
            ::android::hardware::radio::V1_0::NvItem::CDMA_ACCOLC,
            ::android::hardware::radio::V1_0::NvItem::DEVICE_MSL,
            ::android::hardware::radio::V1_0::NvItem::RTN_RECONDITIONED_STATUS,
            ::android::hardware::radio::V1_0::NvItem::RTN_ACTIVATION_DATE,
            ::android::hardware::radio::V1_0::NvItem::RTN_LIFE_TIMER,
            ::android::hardware::radio::V1_0::NvItem::RTN_LIFE_CALLS,
            ::android::hardware::radio::V1_0::NvItem::RTN_LIFE_DATA_TX,
            ::android::hardware::radio::V1_0::NvItem::RTN_LIFE_DATA_RX,
            ::android::hardware::radio::V1_0::NvItem::OMADM_HFA_LEVEL,
            ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_NAI,
            ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_HOME_ADDRESS,
            ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_AAA_AUTH,
            ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_HA_AUTH,
            ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_PRI_HA_ADDR,
            ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_SEC_HA_ADDR,
            ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_REV_TUN_PREF,
            ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_HA_SPI,
            ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_AAA_SPI,
            ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_MN_HA_SS,
            ::android::hardware::radio::V1_0::NvItem::MIP_PROFILE_MN_AAA_SS,
            ::android::hardware::radio::V1_0::NvItem::CDMA_PRL_VERSION,
            ::android::hardware::radio::V1_0::NvItem::CDMA_BC10,
            ::android::hardware::radio::V1_0::NvItem::CDMA_BC14,
            ::android::hardware::radio::V1_0::NvItem::CDMA_SO68,
            ::android::hardware::radio::V1_0::NvItem::CDMA_SO73_COP0,
            ::android::hardware::radio::V1_0::NvItem::CDMA_SO73_COP1TO7,
            ::android::hardware::radio::V1_0::NvItem::CDMA_1X_ADVANCED_ENABLED,
            ::android::hardware::radio::V1_0::NvItem::CDMA_EHRPD_ENABLED,
            ::android::hardware::radio::V1_0::NvItem::CDMA_EHRPD_FORCED,
            ::android::hardware::radio::V1_0::NvItem::LTE_BAND_ENABLE_25,
            ::android::hardware::radio::V1_0::NvItem::LTE_BAND_ENABLE_26,
            ::android::hardware::radio::V1_0::NvItem::LTE_BAND_ENABLE_41,
            ::android::hardware::radio::V1_0::NvItem::LTE_SCAN_PRIORITY_25,
            ::android::hardware::radio::V1_0::NvItem::LTE_SCAN_PRIORITY_26,
            ::android::hardware::radio::V1_0::NvItem::LTE_SCAN_PRIORITY_41,
            ::android::hardware::radio::V1_0::NvItem::LTE_HIDDEN_BAND_PRIORITY_25,
            ::android::hardware::radio::V1_0::NvItem::LTE_HIDDEN_BAND_PRIORITY_26,
            ::android::hardware::radio::V1_0::NvItem::LTE_HIDDEN_BAND_PRIORITY_41,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::ResetNvType>
{
    const ::android::hardware::radio::V1_0::ResetNvType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::ResetNvType* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_0::ResetNvType* static_begin() {
        static const ::android::hardware::radio::V1_0::ResetNvType kVals[3] {
            ::android::hardware::radio::V1_0::ResetNvType::RELOAD,
            ::android::hardware::radio::V1_0::ResetNvType::ERASE,
            ::android::hardware::radio::V1_0::ResetNvType::FACTORY_RESET,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::HardwareConfigType>
{
    const ::android::hardware::radio::V1_0::HardwareConfigType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::HardwareConfigType* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_0::HardwareConfigType* static_begin() {
        static const ::android::hardware::radio::V1_0::HardwareConfigType kVals[2] {
            ::android::hardware::radio::V1_0::HardwareConfigType::MODEM,
            ::android::hardware::radio::V1_0::HardwareConfigType::SIM,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::HardwareConfigState>
{
    const ::android::hardware::radio::V1_0::HardwareConfigState* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::HardwareConfigState* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_0::HardwareConfigState* static_begin() {
        static const ::android::hardware::radio::V1_0::HardwareConfigState kVals[3] {
            ::android::hardware::radio::V1_0::HardwareConfigState::ENABLED,
            ::android::hardware::radio::V1_0::HardwareConfigState::STANDBY,
            ::android::hardware::radio::V1_0::HardwareConfigState::DISABLED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::LceStatus>
{
    const ::android::hardware::radio::V1_0::LceStatus* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::LceStatus* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_0::LceStatus* static_begin() {
        static const ::android::hardware::radio::V1_0::LceStatus kVals[3] {
            ::android::hardware::radio::V1_0::LceStatus::NOT_SUPPORTED,
            ::android::hardware::radio::V1_0::LceStatus::STOPPED,
            ::android::hardware::radio::V1_0::LceStatus::ACTIVE,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CarrierMatchType>
{
    const ::android::hardware::radio::V1_0::CarrierMatchType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CarrierMatchType* end() { return begin() + 5; }
    private:
    static const ::android::hardware::radio::V1_0::CarrierMatchType* static_begin() {
        static const ::android::hardware::radio::V1_0::CarrierMatchType kVals[5] {
            ::android::hardware::radio::V1_0::CarrierMatchType::ALL,
            ::android::hardware::radio::V1_0::CarrierMatchType::SPN,
            ::android::hardware::radio::V1_0::CarrierMatchType::IMSI_PREFIX,
            ::android::hardware::radio::V1_0::CarrierMatchType::GID1,
            ::android::hardware::radio::V1_0::CarrierMatchType::GID2,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CdmaSmsDigitMode>
{
    const ::android::hardware::radio::V1_0::CdmaSmsDigitMode* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CdmaSmsDigitMode* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_0::CdmaSmsDigitMode* static_begin() {
        static const ::android::hardware::radio::V1_0::CdmaSmsDigitMode kVals[2] {
            ::android::hardware::radio::V1_0::CdmaSmsDigitMode::FOUR_BIT,
            ::android::hardware::radio::V1_0::CdmaSmsDigitMode::EIGHT_BIT,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CdmaSmsNumberMode>
{
    const ::android::hardware::radio::V1_0::CdmaSmsNumberMode* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CdmaSmsNumberMode* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_0::CdmaSmsNumberMode* static_begin() {
        static const ::android::hardware::radio::V1_0::CdmaSmsNumberMode kVals[2] {
            ::android::hardware::radio::V1_0::CdmaSmsNumberMode::NOT_DATA_NETWORK,
            ::android::hardware::radio::V1_0::CdmaSmsNumberMode::DATA_NETWORK,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CdmaSmsNumberType>
{
    const ::android::hardware::radio::V1_0::CdmaSmsNumberType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CdmaSmsNumberType* end() { return begin() + 8; }
    private:
    static const ::android::hardware::radio::V1_0::CdmaSmsNumberType* static_begin() {
        static const ::android::hardware::radio::V1_0::CdmaSmsNumberType kVals[8] {
            ::android::hardware::radio::V1_0::CdmaSmsNumberType::UNKNOWN,
            ::android::hardware::radio::V1_0::CdmaSmsNumberType::INTERNATIONAL_OR_DATA_IP,
            ::android::hardware::radio::V1_0::CdmaSmsNumberType::NATIONAL_OR_INTERNET_MAIL,
            ::android::hardware::radio::V1_0::CdmaSmsNumberType::NETWORK,
            ::android::hardware::radio::V1_0::CdmaSmsNumberType::SUBSCRIBER,
            ::android::hardware::radio::V1_0::CdmaSmsNumberType::ALPHANUMERIC,
            ::android::hardware::radio::V1_0::CdmaSmsNumberType::ABBREVIATED,
            ::android::hardware::radio::V1_0::CdmaSmsNumberType::RESERVED_7,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CdmaSmsNumberPlan>
{
    const ::android::hardware::radio::V1_0::CdmaSmsNumberPlan* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CdmaSmsNumberPlan* end() { return begin() + 16; }
    private:
    static const ::android::hardware::radio::V1_0::CdmaSmsNumberPlan* static_begin() {
        static const ::android::hardware::radio::V1_0::CdmaSmsNumberPlan kVals[16] {
            ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::UNKNOWN,
            ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::TELEPHONY,
            ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_2,
            ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::DATA,
            ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::TELEX,
            ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_5,
            ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_6,
            ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_7,
            ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_8,
            ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::PRIVATE,
            ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_10,
            ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_11,
            ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_12,
            ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_13,
            ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_14,
            ::android::hardware::radio::V1_0::CdmaSmsNumberPlan::RESERVED_15,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CdmaSmsSubaddressType>
{
    const ::android::hardware::radio::V1_0::CdmaSmsSubaddressType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CdmaSmsSubaddressType* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_0::CdmaSmsSubaddressType* static_begin() {
        static const ::android::hardware::radio::V1_0::CdmaSmsSubaddressType kVals[2] {
            ::android::hardware::radio::V1_0::CdmaSmsSubaddressType::NSAP,
            ::android::hardware::radio::V1_0::CdmaSmsSubaddressType::USER_SPECIFIED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CdmaSmsErrorClass>
{
    const ::android::hardware::radio::V1_0::CdmaSmsErrorClass* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CdmaSmsErrorClass* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_0::CdmaSmsErrorClass* static_begin() {
        static const ::android::hardware::radio::V1_0::CdmaSmsErrorClass kVals[2] {
            ::android::hardware::radio::V1_0::CdmaSmsErrorClass::NO_ERROR,
            ::android::hardware::radio::V1_0::CdmaSmsErrorClass::ERROR,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus>
{
    const ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus* end() { return begin() + 4; }
    private:
    static const ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus* static_begin() {
        static const ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus kVals[4] {
            ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::REC_UNREAD,
            ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::REC_READ,
            ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::STO_UNSENT,
            ::android::hardware::radio::V1_0::CdmaSmsWriteArgsStatus::STO_SENT,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CellInfoType>
{
    const ::android::hardware::radio::V1_0::CellInfoType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CellInfoType* end() { return begin() + 6; }
    private:
    static const ::android::hardware::radio::V1_0::CellInfoType* static_begin() {
        static const ::android::hardware::radio::V1_0::CellInfoType kVals[6] {
            ::android::hardware::radio::V1_0::CellInfoType::NONE,
            ::android::hardware::radio::V1_0::CellInfoType::GSM,
            ::android::hardware::radio::V1_0::CellInfoType::CDMA,
            ::android::hardware::radio::V1_0::CellInfoType::LTE,
            ::android::hardware::radio::V1_0::CellInfoType::WCDMA,
            ::android::hardware::radio::V1_0::CellInfoType::TD_SCDMA,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::TimeStampType>
{
    const ::android::hardware::radio::V1_0::TimeStampType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::TimeStampType* end() { return begin() + 5; }
    private:
    static const ::android::hardware::radio::V1_0::TimeStampType* static_begin() {
        static const ::android::hardware::radio::V1_0::TimeStampType kVals[5] {
            ::android::hardware::radio::V1_0::TimeStampType::UNKNOWN,
            ::android::hardware::radio::V1_0::TimeStampType::ANTENNA,
            ::android::hardware::radio::V1_0::TimeStampType::MODEM,
            ::android::hardware::radio::V1_0::TimeStampType::OEM_RIL,
            ::android::hardware::radio::V1_0::TimeStampType::JAVA_RIL,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::ApnAuthType>
{
    const ::android::hardware::radio::V1_0::ApnAuthType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::ApnAuthType* end() { return begin() + 4; }
    private:
    static const ::android::hardware::radio::V1_0::ApnAuthType* static_begin() {
        static const ::android::hardware::radio::V1_0::ApnAuthType kVals[4] {
            ::android::hardware::radio::V1_0::ApnAuthType::NO_PAP_NO_CHAP,
            ::android::hardware::radio::V1_0::ApnAuthType::PAP_NO_CHAP,
            ::android::hardware::radio::V1_0::ApnAuthType::NO_PAP_CHAP,
            ::android::hardware::radio::V1_0::ApnAuthType::PAP_CHAP,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::RadioTechnologyFamily>
{
    const ::android::hardware::radio::V1_0::RadioTechnologyFamily* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::RadioTechnologyFamily* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_0::RadioTechnologyFamily* static_begin() {
        static const ::android::hardware::radio::V1_0::RadioTechnologyFamily kVals[2] {
            ::android::hardware::radio::V1_0::RadioTechnologyFamily::THREE_GPP,
            ::android::hardware::radio::V1_0::RadioTechnologyFamily::THREE_GPP2,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::RadioCapabilityPhase>
{
    const ::android::hardware::radio::V1_0::RadioCapabilityPhase* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::RadioCapabilityPhase* end() { return begin() + 5; }
    private:
    static const ::android::hardware::radio::V1_0::RadioCapabilityPhase* static_begin() {
        static const ::android::hardware::radio::V1_0::RadioCapabilityPhase kVals[5] {
            ::android::hardware::radio::V1_0::RadioCapabilityPhase::CONFIGURED,
            ::android::hardware::radio::V1_0::RadioCapabilityPhase::START,
            ::android::hardware::radio::V1_0::RadioCapabilityPhase::APPLY,
            ::android::hardware::radio::V1_0::RadioCapabilityPhase::UNSOL_RSP,
            ::android::hardware::radio::V1_0::RadioCapabilityPhase::FINISH,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::RadioCapabilityStatus>
{
    const ::android::hardware::radio::V1_0::RadioCapabilityStatus* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::RadioCapabilityStatus* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_0::RadioCapabilityStatus* static_begin() {
        static const ::android::hardware::radio::V1_0::RadioCapabilityStatus kVals[3] {
            ::android::hardware::radio::V1_0::RadioCapabilityStatus::NONE,
            ::android::hardware::radio::V1_0::RadioCapabilityStatus::SUCCESS,
            ::android::hardware::radio::V1_0::RadioCapabilityStatus::FAIL,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::RadioAccessFamily>
{
    const ::android::hardware::radio::V1_0::RadioAccessFamily* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::RadioAccessFamily* end() { return begin() + 19; }
    private:
    static const ::android::hardware::radio::V1_0::RadioAccessFamily* static_begin() {
        static const ::android::hardware::radio::V1_0::RadioAccessFamily kVals[19] {
            ::android::hardware::radio::V1_0::RadioAccessFamily::UNKNOWN,
            ::android::hardware::radio::V1_0::RadioAccessFamily::GPRS,
            ::android::hardware::radio::V1_0::RadioAccessFamily::EDGE,
            ::android::hardware::radio::V1_0::RadioAccessFamily::UMTS,
            ::android::hardware::radio::V1_0::RadioAccessFamily::IS95A,
            ::android::hardware::radio::V1_0::RadioAccessFamily::IS95B,
            ::android::hardware::radio::V1_0::RadioAccessFamily::ONE_X_RTT,
            ::android::hardware::radio::V1_0::RadioAccessFamily::EVDO_0,
            ::android::hardware::radio::V1_0::RadioAccessFamily::EVDO_A,
            ::android::hardware::radio::V1_0::RadioAccessFamily::HSDPA,
            ::android::hardware::radio::V1_0::RadioAccessFamily::HSUPA,
            ::android::hardware::radio::V1_0::RadioAccessFamily::HSPA,
            ::android::hardware::radio::V1_0::RadioAccessFamily::EVDO_B,
            ::android::hardware::radio::V1_0::RadioAccessFamily::EHRPD,
            ::android::hardware::radio::V1_0::RadioAccessFamily::LTE,
            ::android::hardware::radio::V1_0::RadioAccessFamily::HSPAP,
            ::android::hardware::radio::V1_0::RadioAccessFamily::GSM,
            ::android::hardware::radio::V1_0::RadioAccessFamily::TD_SCDMA,
            ::android::hardware::radio::V1_0::RadioAccessFamily::LTE_CA,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::UssdModeType>
{
    const ::android::hardware::radio::V1_0::UssdModeType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::UssdModeType* end() { return begin() + 6; }
    private:
    static const ::android::hardware::radio::V1_0::UssdModeType* static_begin() {
        static const ::android::hardware::radio::V1_0::UssdModeType kVals[6] {
            ::android::hardware::radio::V1_0::UssdModeType::NOTIFY,
            ::android::hardware::radio::V1_0::UssdModeType::REQUEST,
            ::android::hardware::radio::V1_0::UssdModeType::NW_RELEASE,
            ::android::hardware::radio::V1_0::UssdModeType::LOCAL_CLIENT,
            ::android::hardware::radio::V1_0::UssdModeType::NOT_SUPPORTED,
            ::android::hardware::radio::V1_0::UssdModeType::NW_TIMEOUT,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::SimRefreshType>
{
    const ::android::hardware::radio::V1_0::SimRefreshType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::SimRefreshType* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_0::SimRefreshType* static_begin() {
        static const ::android::hardware::radio::V1_0::SimRefreshType kVals[3] {
            ::android::hardware::radio::V1_0::SimRefreshType::SIM_FILE_UPDATE,
            ::android::hardware::radio::V1_0::SimRefreshType::SIM_INIT,
            ::android::hardware::radio::V1_0::SimRefreshType::SIM_RESET,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::SrvccState>
{
    const ::android::hardware::radio::V1_0::SrvccState* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::SrvccState* end() { return begin() + 4; }
    private:
    static const ::android::hardware::radio::V1_0::SrvccState* static_begin() {
        static const ::android::hardware::radio::V1_0::SrvccState kVals[4] {
            ::android::hardware::radio::V1_0::SrvccState::HANDOVER_STARTED,
            ::android::hardware::radio::V1_0::SrvccState::HANDOVER_COMPLETED,
            ::android::hardware::radio::V1_0::SrvccState::HANDOVER_FAILED,
            ::android::hardware::radio::V1_0::SrvccState::HANDOVER_CANCELED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::UiccSubActStatus>
{
    const ::android::hardware::radio::V1_0::UiccSubActStatus* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::UiccSubActStatus* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_0::UiccSubActStatus* static_begin() {
        static const ::android::hardware::radio::V1_0::UiccSubActStatus kVals[2] {
            ::android::hardware::radio::V1_0::UiccSubActStatus::DEACTIVATE,
            ::android::hardware::radio::V1_0::UiccSubActStatus::ACTIVATE,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::SubscriptionType>
{
    const ::android::hardware::radio::V1_0::SubscriptionType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::SubscriptionType* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_0::SubscriptionType* static_begin() {
        static const ::android::hardware::radio::V1_0::SubscriptionType kVals[3] {
            ::android::hardware::radio::V1_0::SubscriptionType::SUBSCRIPTION_1,
            ::android::hardware::radio::V1_0::SubscriptionType::SUBSCRIPTION_2,
            ::android::hardware::radio::V1_0::SubscriptionType::SUBSCRIPTION_3,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::DataProfileInfoType>
{
    const ::android::hardware::radio::V1_0::DataProfileInfoType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::DataProfileInfoType* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_0::DataProfileInfoType* static_begin() {
        static const ::android::hardware::radio::V1_0::DataProfileInfoType kVals[3] {
            ::android::hardware::radio::V1_0::DataProfileInfoType::COMMON,
            ::android::hardware::radio::V1_0::DataProfileInfoType::THREE_GPP,
            ::android::hardware::radio::V1_0::DataProfileInfoType::THREE_GPP2,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::PhoneRestrictedState>
{
    const ::android::hardware::radio::V1_0::PhoneRestrictedState* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::PhoneRestrictedState* end() { return begin() + 5; }
    private:
    static const ::android::hardware::radio::V1_0::PhoneRestrictedState* static_begin() {
        static const ::android::hardware::radio::V1_0::PhoneRestrictedState kVals[5] {
            ::android::hardware::radio::V1_0::PhoneRestrictedState::NONE,
            ::android::hardware::radio::V1_0::PhoneRestrictedState::CS_EMERGENCY,
            ::android::hardware::radio::V1_0::PhoneRestrictedState::CS_NORMAL,
            ::android::hardware::radio::V1_0::PhoneRestrictedState::CS_ALL,
            ::android::hardware::radio::V1_0::PhoneRestrictedState::PS_ALL,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation>
{
    const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation* static_begin() {
        static const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation kVals[3] {
            ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation::ALLOWED,
            ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation::RESTRICTED,
            ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPresentation::UNKNOWN,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CdmaCallWaitingNumberType>
{
    const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType* end() { return begin() + 5; }
    private:
    static const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType* static_begin() {
        static const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType kVals[5] {
            ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::UNKNOWN,
            ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::INTERNATIONAL,
            ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::NATIONAL,
            ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::NETWORK_SPECIFIC,
            ::android::hardware::radio::V1_0::CdmaCallWaitingNumberType::SUBSCRIBER,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan>
{
    const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan* end() { return begin() + 6; }
    private:
    static const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan* static_begin() {
        static const ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan kVals[6] {
            ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::UNKNOWN,
            ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::ISDN,
            ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::DATA,
            ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::TELEX,
            ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::NATIONAL,
            ::android::hardware::radio::V1_0::CdmaCallWaitingNumberPlan::PRIVATE,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CdmaOtaProvisionStatus>
{
    const ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus* end() { return begin() + 12; }
    private:
    static const ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus* static_begin() {
        static const ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus kVals[12] {
            ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::SPL_UNLOCKED,
            ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::SPC_RETRIES_EXCEEDED,
            ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::A_KEY_EXCHANGED,
            ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::SSD_UPDATED,
            ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::NAM_DOWNLOADED,
            ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::MDN_DOWNLOADED,
            ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::IMSI_DOWNLOADED,
            ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::PRL_DOWNLOADED,
            ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::COMMITTED,
            ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::OTAPA_STARTED,
            ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::OTAPA_STOPPED,
            ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus::OTAPA_ABORTED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CdmaInfoRecName>
{
    const ::android::hardware::radio::V1_0::CdmaInfoRecName* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CdmaInfoRecName* end() { return begin() + 11; }
    private:
    static const ::android::hardware::radio::V1_0::CdmaInfoRecName* static_begin() {
        static const ::android::hardware::radio::V1_0::CdmaInfoRecName kVals[11] {
            ::android::hardware::radio::V1_0::CdmaInfoRecName::DISPLAY,
            ::android::hardware::radio::V1_0::CdmaInfoRecName::CALLED_PARTY_NUMBER,
            ::android::hardware::radio::V1_0::CdmaInfoRecName::CALLING_PARTY_NUMBER,
            ::android::hardware::radio::V1_0::CdmaInfoRecName::CONNECTED_NUMBER,
            ::android::hardware::radio::V1_0::CdmaInfoRecName::SIGNAL,
            ::android::hardware::radio::V1_0::CdmaInfoRecName::REDIRECTING_NUMBER,
            ::android::hardware::radio::V1_0::CdmaInfoRecName::LINE_CONTROL,
            ::android::hardware::radio::V1_0::CdmaInfoRecName::EXTENDED_DISPLAY,
            ::android::hardware::radio::V1_0::CdmaInfoRecName::T53_CLIR,
            ::android::hardware::radio::V1_0::CdmaInfoRecName::T53_RELEASE,
            ::android::hardware::radio::V1_0::CdmaInfoRecName::T53_AUDIO_CONTROL,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::CdmaRedirectingReason>
{
    const ::android::hardware::radio::V1_0::CdmaRedirectingReason* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::CdmaRedirectingReason* end() { return begin() + 7; }
    private:
    static const ::android::hardware::radio::V1_0::CdmaRedirectingReason* static_begin() {
        static const ::android::hardware::radio::V1_0::CdmaRedirectingReason kVals[7] {
            ::android::hardware::radio::V1_0::CdmaRedirectingReason::UNKNOWN,
            ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_BUSY,
            ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_NO_REPLY,
            ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALLED_DTE_OUT_OF_ORDER,
            ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_BY_THE_CALLED_DTE,
            ::android::hardware::radio::V1_0::CdmaRedirectingReason::CALL_FORWARDING_UNCONDITIONAL,
            ::android::hardware::radio::V1_0::CdmaRedirectingReason::RESERVED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::SsServiceType>
{
    const ::android::hardware::radio::V1_0::SsServiceType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::SsServiceType* end() { return begin() + 19; }
    private:
    static const ::android::hardware::radio::V1_0::SsServiceType* static_begin() {
        static const ::android::hardware::radio::V1_0::SsServiceType kVals[19] {
            ::android::hardware::radio::V1_0::SsServiceType::CFU,
            ::android::hardware::radio::V1_0::SsServiceType::CF_BUSY,
            ::android::hardware::radio::V1_0::SsServiceType::CF_NO_REPLY,
            ::android::hardware::radio::V1_0::SsServiceType::CF_NOT_REACHABLE,
            ::android::hardware::radio::V1_0::SsServiceType::CF_ALL,
            ::android::hardware::radio::V1_0::SsServiceType::CF_ALL_CONDITIONAL,
            ::android::hardware::radio::V1_0::SsServiceType::CLIP,
            ::android::hardware::radio::V1_0::SsServiceType::CLIR,
            ::android::hardware::radio::V1_0::SsServiceType::COLP,
            ::android::hardware::radio::V1_0::SsServiceType::COLR,
            ::android::hardware::radio::V1_0::SsServiceType::WAIT,
            ::android::hardware::radio::V1_0::SsServiceType::BAOC,
            ::android::hardware::radio::V1_0::SsServiceType::BAOIC,
            ::android::hardware::radio::V1_0::SsServiceType::BAOIC_EXC_HOME,
            ::android::hardware::radio::V1_0::SsServiceType::BAIC,
            ::android::hardware::radio::V1_0::SsServiceType::BAIC_ROAMING,
            ::android::hardware::radio::V1_0::SsServiceType::ALL_BARRING,
            ::android::hardware::radio::V1_0::SsServiceType::OUTGOING_BARRING,
            ::android::hardware::radio::V1_0::SsServiceType::INCOMING_BARRING,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::SsRequestType>
{
    const ::android::hardware::radio::V1_0::SsRequestType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::SsRequestType* end() { return begin() + 5; }
    private:
    static const ::android::hardware::radio::V1_0::SsRequestType* static_begin() {
        static const ::android::hardware::radio::V1_0::SsRequestType kVals[5] {
            ::android::hardware::radio::V1_0::SsRequestType::ACTIVATION,
            ::android::hardware::radio::V1_0::SsRequestType::DEACTIVATION,
            ::android::hardware::radio::V1_0::SsRequestType::INTERROGATION,
            ::android::hardware::radio::V1_0::SsRequestType::REGISTRATION,
            ::android::hardware::radio::V1_0::SsRequestType::ERASURE,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::SsTeleserviceType>
{
    const ::android::hardware::radio::V1_0::SsTeleserviceType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::SsTeleserviceType* end() { return begin() + 6; }
    private:
    static const ::android::hardware::radio::V1_0::SsTeleserviceType* static_begin() {
        static const ::android::hardware::radio::V1_0::SsTeleserviceType kVals[6] {
            ::android::hardware::radio::V1_0::SsTeleserviceType::ALL_TELE_AND_BEARER_SERVICES,
            ::android::hardware::radio::V1_0::SsTeleserviceType::ALL_TELESEVICES,
            ::android::hardware::radio::V1_0::SsTeleserviceType::TELEPHONY,
            ::android::hardware::radio::V1_0::SsTeleserviceType::ALL_DATA_TELESERVICES,
            ::android::hardware::radio::V1_0::SsTeleserviceType::SMS_SERVICES,
            ::android::hardware::radio::V1_0::SsTeleserviceType::ALL_TELESERVICES_EXCEPT_SMS,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::SuppServiceClass>
{
    const ::android::hardware::radio::V1_0::SuppServiceClass* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::SuppServiceClass* end() { return begin() + 10; }
    private:
    static const ::android::hardware::radio::V1_0::SuppServiceClass* static_begin() {
        static const ::android::hardware::radio::V1_0::SuppServiceClass kVals[10] {
            ::android::hardware::radio::V1_0::SuppServiceClass::NONE,
            ::android::hardware::radio::V1_0::SuppServiceClass::VOICE,
            ::android::hardware::radio::V1_0::SuppServiceClass::DATA,
            ::android::hardware::radio::V1_0::SuppServiceClass::FAX,
            ::android::hardware::radio::V1_0::SuppServiceClass::SMS,
            ::android::hardware::radio::V1_0::SuppServiceClass::DATA_SYNC,
            ::android::hardware::radio::V1_0::SuppServiceClass::DATA_ASYNC,
            ::android::hardware::radio::V1_0::SuppServiceClass::PACKET,
            ::android::hardware::radio::V1_0::SuppServiceClass::PAD,
            ::android::hardware::radio::V1_0::SuppServiceClass::MAX,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::ApnTypes>
{
    const ::android::hardware::radio::V1_0::ApnTypes* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::ApnTypes* end() { return begin() + 12; }
    private:
    static const ::android::hardware::radio::V1_0::ApnTypes* static_begin() {
        static const ::android::hardware::radio::V1_0::ApnTypes kVals[12] {
            ::android::hardware::radio::V1_0::ApnTypes::NONE,
            ::android::hardware::radio::V1_0::ApnTypes::DEFAULT,
            ::android::hardware::radio::V1_0::ApnTypes::MMS,
            ::android::hardware::radio::V1_0::ApnTypes::SUPL,
            ::android::hardware::radio::V1_0::ApnTypes::DUN,
            ::android::hardware::radio::V1_0::ApnTypes::HIPRI,
            ::android::hardware::radio::V1_0::ApnTypes::FOTA,
            ::android::hardware::radio::V1_0::ApnTypes::IMS,
            ::android::hardware::radio::V1_0::ApnTypes::CBS,
            ::android::hardware::radio::V1_0::ApnTypes::IA,
            ::android::hardware::radio::V1_0::ApnTypes::EMERGENCY,
            ::android::hardware::radio::V1_0::ApnTypes::ALL,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::IndicationFilter>
{
    const ::android::hardware::radio::V1_0::IndicationFilter* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::IndicationFilter* end() { return begin() + 5; }
    private:
    static const ::android::hardware::radio::V1_0::IndicationFilter* static_begin() {
        static const ::android::hardware::radio::V1_0::IndicationFilter kVals[5] {
            ::android::hardware::radio::V1_0::IndicationFilter::NONE,
            ::android::hardware::radio::V1_0::IndicationFilter::SIGNAL_STRENGTH,
            ::android::hardware::radio::V1_0::IndicationFilter::FULL_NETWORK_STATE,
            ::android::hardware::radio::V1_0::IndicationFilter::DATA_CALL_DORMANCY_CHANGED,
            ::android::hardware::radio::V1_0::IndicationFilter::ALL,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::MvnoType>
{
    const ::android::hardware::radio::V1_0::MvnoType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::MvnoType* end() { return begin() + 4; }
    private:
    static const ::android::hardware::radio::V1_0::MvnoType* static_begin() {
        static const ::android::hardware::radio::V1_0::MvnoType kVals[4] {
            ::android::hardware::radio::V1_0::MvnoType::NONE,
            ::android::hardware::radio::V1_0::MvnoType::IMSI,
            ::android::hardware::radio::V1_0::MvnoType::GID,
            ::android::hardware::radio::V1_0::MvnoType::SPN,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::DeviceStateType>
{
    const ::android::hardware::radio::V1_0::DeviceStateType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::DeviceStateType* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_0::DeviceStateType* static_begin() {
        static const ::android::hardware::radio::V1_0::DeviceStateType kVals[3] {
            ::android::hardware::radio::V1_0::DeviceStateType::POWER_SAVE_MODE,
            ::android::hardware::radio::V1_0::DeviceStateType::CHARGING_STATE,
            ::android::hardware::radio::V1_0::DeviceStateType::LOW_DATA_EXPECTED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_0::P2Constant>
{
    const ::android::hardware::radio::V1_0::P2Constant* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_0::P2Constant* end() { return begin() + 1; }
    private:
    static const ::android::hardware::radio::V1_0::P2Constant* static_begin() {
        static const ::android::hardware::radio::V1_0::P2Constant kVals[1] {
            ::android::hardware::radio::V1_0::P2Constant::NO_P2,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HARDWARE_RADIO_V1_0_TYPES_H
