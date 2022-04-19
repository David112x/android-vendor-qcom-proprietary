/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef QCRIL_UIM_TYPES_H
#define QCRIL_UIM_TYPES_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include <stdint.h>
#include <string>
#include <vector>


/*===========================================================================

                           DEFINES

===========================================================================*/
#define RIL_UIM_CARD_MAX_APPS          8
#define RIL_UIM_MAX_SELECT_RSP_LEN  1024
#define RIL_UIM_MAX_CARD_COUNT         3


/*===========================================================================

                           ENUMS

===========================================================================*/
/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_Errno

   DESCRIPTION:
     RIL Errors applicable to UIM and values same as RIL_Errno
-------------------------------------------------------------------------------*/
typedef enum {
    RIL_UIM_E_SUCCESS = 0,
    RIL_UIM_E_RADIO_NOT_AVAILABLE = 1,              /* If radio did not start or is resetting */
    RIL_UIM_E_GENERIC_FAILURE = 2,
    RIL_UIM_E_PASSWORD_INCORRECT = 3,               /* for PIN/PIN2 methods only! */
    RIL_UIM_E_SIM_PIN2 = 4,                         /* Operation requires SIM PIN2 to be entered */
    RIL_UIM_E_SIM_PUK2 = 5,                         /* Operation requires SIM PIN2 to be entered */
    RIL_UIM_E_REQUEST_NOT_SUPPORTED = 6,
    RIL_UIM_E_CANCELLED = 7,
    RIL_UIM_E_SIM_ABSENT = 11,
    RIL_UIM_E_SUBSCRIPTION_NOT_AVAILABLE = 12,      /* fail to find CDMA subscription from specified
                                                       location */
    RIL_UIM_E_ILLEGAL_SIM_OR_ME = 15,               /* network selection failed due to
                                                       illegal SIM or ME */
    RIL_UIM_E_MISSING_RESOURCE = 16,                /* no logical channel available */
    RIL_UIM_E_NO_SUCH_ELEMENT = 17,                 /* application not found on SIM */
    RIL_UIM_E_SUBSCRIPTION_NOT_SUPPORTED = 26,      /* Subscription not supported by RIL */
    RIL_UIM_E_NO_MEMORY = 37,                       /* Not sufficient memory to process the request */
    RIL_UIM_E_INTERNAL_ERR = 38,                    /* Modem hit unexpected error scenario while handling
                                                       this request */
    RIL_UIM_E_SYSTEM_ERR = 39,                      /* Hit platform or system error */
    RIL_UIM_E_MODEM_ERR = 40,                       /* Vendor RIL got unexpected or incorrect response
                                                       from modem for this request */
    RIL_UIM_E_INVALID_STATE = 41,                   /* Unexpected request for the current state */
    RIL_UIM_E_NO_RESOURCES = 42,                    /* Not sufficient resource to process the request */
    RIL_UIM_E_SIM_ERR = 43,                         /* Received error from SIM card */
    RIL_UIM_E_INVALID_ARGUMENTS = 44,               /* Received invalid arguments in request */
    RIL_UIM_E_INVALID_SIM_STATE = 45,               /* Can not process the request in current SIM state */
    RIL_UIM_E_INVALID_MODEM_STATE = 46,             /* Can not process the request in current Modem state */
    RIL_UIM_E_OPERATION_NOT_ALLOWED = 54,           /* Not allowed the request now */
} RIL_UIM_Errno;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_CardState

   DESCRIPTION:
     CARD states
-------------------------------------------------------------------------------*/
typedef enum {
    RIL_UIM_CARDSTATE_ABSENT     = 0,
    RIL_UIM_CARDSTATE_PRESENT    = 1,
    RIL_UIM_CARDSTATE_ERROR      = 2
} RIL_UIM_CardState;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_PinState

   DESCRIPTION:
     CARD pin states
-------------------------------------------------------------------------------*/
typedef enum {
    RIL_UIM_PINSTATE_UNKNOWN              = 0,
    RIL_UIM_PINSTATE_ENABLED_NOT_VERIFIED = 1,
    RIL_UIM_PINSTATE_ENABLED_VERIFIED     = 2,
    RIL_UIM_PINSTATE_DISABLED             = 3,
    RIL_UIM_PINSTATE_ENABLED_BLOCKED      = 4,
    RIL_UIM_PINSTATE_ENABLED_PERM_BLOCKED = 5
} RIL_UIM_PinState;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_AppType

   DESCRIPTION:
     CARD App types
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_APPTYPE_UNKNOWN = 0,
  RIL_UIM_APPTYPE_SIM     = 1,
  RIL_UIM_APPTYPE_USIM    = 2,
  RIL_UIM_APPTYPE_RUIM    = 3,
  RIL_UIM_APPTYPE_CSIM    = 4,
  RIL_UIM_APPTYPE_ISIM    = 5
} RIL_UIM_AppType;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_AppState

   DESCRIPTION:
     CARD App states
-------------------------------------------------------------------------------*/
typedef enum {
    RIL_UIM_APPSTATE_UNKNOWN               = 0,
    RIL_UIM_APPSTATE_DETECTED              = 1,
    RIL_UIM_APPSTATE_PIN                   = 2, /* If PIN1 or UPin is required */
    RIL_UIM_APPSTATE_PUK                   = 3, /* If PUK1 or Puk for UPin is required */
    RIL_UIM_APPSTATE_SUBSCRIPTION_PERSO    = 4, /* perso_substate should be look at
                                               when app_state is assigned to this value */
    RIL_UIM_APPSTATE_READY                 = 5
} RIL_UIM_AppState;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_PersoSubstate

   DESCRIPTION:
     CARD perso states
-------------------------------------------------------------------------------*/
typedef enum {
    RIL_UIM_PERSOSUBSTATE_UNKNOWN                   = 0, /* initial state */
    RIL_UIM_PERSOSUBSTATE_IN_PROGRESS               = 1, /* in between each lock transition */
    RIL_UIM_PERSOSUBSTATE_READY                     = 2, /* when either SIM or RUIM Perso is finished
                                                            since each app can only have 1 active perso
                                                            involved */
    RIL_UIM_PERSOSUBSTATE_SIM_NETWORK               = 3,
    RIL_UIM_PERSOSUBSTATE_SIM_NETWORK_SUBSET        = 4,
    RIL_UIM_PERSOSUBSTATE_SIM_CORPORATE             = 5,
    RIL_UIM_PERSOSUBSTATE_SIM_SERVICE_PROVIDER      = 6,
    RIL_UIM_PERSOSUBSTATE_SIM_SIM                   = 7,
    RIL_UIM_PERSOSUBSTATE_SIM_NETWORK_PUK           = 8, /* The corresponding perso lock is blocked */
    RIL_UIM_PERSOSUBSTATE_SIM_NETWORK_SUBSET_PUK    = 9,
    RIL_UIM_PERSOSUBSTATE_SIM_CORPORATE_PUK         = 10,
    RIL_UIM_PERSOSUBSTATE_SIM_SERVICE_PROVIDER_PUK  = 11,
    RIL_UIM_PERSOSUBSTATE_SIM_SIM_PUK               = 12,
    RIL_UIM_PERSOSUBSTATE_RUIM_NETWORK1             = 13,
    RIL_UIM_PERSOSUBSTATE_RUIM_NETWORK2             = 14,
    RIL_UIM_PERSOSUBSTATE_RUIM_HRPD                 = 15,
    RIL_UIM_PERSOSUBSTATE_RUIM_CORPORATE            = 16,
    RIL_UIM_PERSOSUBSTATE_RUIM_SERVICE_PROVIDER     = 17,
    RIL_UIM_PERSOSUBSTATE_RUIM_RUIM                 = 18,
    RIL_UIM_PERSOSUBSTATE_RUIM_NETWORK1_PUK         = 19, /* The corresponding perso lock is blocked */
    RIL_UIM_PERSOSUBSTATE_RUIM_NETWORK2_PUK         = 20,
    RIL_UIM_PERSOSUBSTATE_RUIM_HRPD_PUK             = 21,
    RIL_UIM_PERSOSUBSTATE_RUIM_CORPORATE_PUK        = 22,
    RIL_UIM_PERSOSUBSTATE_RUIM_SERVICE_PROVIDER_PUK = 23,
    RIL_UIM_PERSOSUBSTATE_RUIM_RUIM_PUK             = 24,

    RIL_UIM_PERSOSUBSTATE_SIM_SPN                   = 100,
    RIL_UIM_PERSOSUBSTATE_SIM_SPN_PUK               = 101,
    RIL_UIM_PERSOSUBSTATE_SIM_SP_EHPLMN             = 102,
    RIL_UIM_PERSOSUBSTATE_SIM_SP_EHPLMN_PUK         = 103,
    RIL_UIM_PERSOSUBSTATE_SIM_ICCID                 = 104,
    RIL_UIM_PERSOSUBSTATE_SIM_ICCID_PUK             = 105,
    RIL_UIM_PERSOSUBSTATE_SIM_IMPI                  = 106,
    RIL_UIM_PERSOSUBSTATE_SIM_IMPI_PUK              = 107,
    RIL_UIM_PERSOSUBSTATE_SIM_NS_SP                 = 108,
    RIL_UIM_PERSOSUBSTATE_SIM_NS_SP_PUK             = 109,

    RIL_UIM_PERSOSUBSTATE_SIM_RSU_NO_CK             = 200
} RIL_UIM_PersoSubstate;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_SimRefreshResult

   DESCRIPTION:
     CARD Refresh type
-------------------------------------------------------------------------------*/
typedef enum {
    /* A file on SIM has been updated.  data[1] contains the EFID. */
    RIL_UIM_SIM_FILE_UPDATE = 0,
    /* SIM initialized.  All files should be re-read. */
    RIL_UIM_SIM_INIT = 1,
    /* SIM reset.  SIM power required, SIM may be locked and all files should be re-read. */
    RIL_UIM_SIM_RESET = 2
} RIL_UIM_SIM_RefreshResult;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_CardPowerState

   DESCRIPTION:
     CARD Power request types
-------------------------------------------------------------------------------*/
typedef enum {
    RIL_UIM_CARD_POWER_DOWN = 0,
    RIL_UIM_CARD_POWER_UP = 1,
    RIL_UIM_CARD_POWER_UP_PASS_THROUGH = 2
} RIL_UIM_CardPowerState;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_UiccVoltageStatus
   DESCRIPTION:
     Used to convey the SIM card's voltage status
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_UICC_VOLTAGE_START_ACTIVATION    = 0,
  RIL_UIM_UICC_VOLTAGE_ACTIVATED,
  RIL_UIM_UICC_VOLTAGE_START_DEACTIVATION,
  RIL_UIM_UICC_VOLTAGE_DEACTIVATED
} RIL_UIM_UiccVoltageStatus;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_APDUBehavior
   DESCRIPTION:
     Used to convey APDU behavior
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_UICC_APDU_BEHAVIOR_NO_GET_RESPONSE_FOR_WARNING = 0,
  RIL_UIM_UICC_APDU_BEHAVIOR_AUTOMATIC_GET_RESPONSE_FOR_WARNING
} RIL_UIM_APDU_Behavior;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_PIN_Type
   DESCRIPTION:
     SIM PIN type
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_SIM_PIN1,
  RIL_UIM_SIM_PIN2,
  RIL_UIM_SIM_PUK1,
  RIL_UIM_SIM_PUK2
} RIL_UIM_PIN_Type;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_FacilityLockType
   DESCRIPTION:
     Facility lock type
-------------------------------------------------------------------------------*/
typedef enum {
  FDN,
  PIN1
} RIL_UIM_FacilityLockType;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_SlotState
   DESCRIPTION:
    Used to convey slot activity status
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_PHYSICAL_SLOT_STATE_INACTIVE  = 0x00,
  RIL_UIM_PHYSICAL_SLOT_STATE_ACTIVE    = 0x01
} RIL_UIM_SlotState;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_GBA_KsNafType
   DESCRIPTION:
    Used to convey slot activity status
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_GBA_KS_NAF_TYPE_PLAIN        = 0x00,
  RIL_UIM_GBA_KS_NAF_TYPE_ENCRYPTED    = 0x01
} RIL_UIM_GBA_KsNafType;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_RemoteSimlockOperationType
   DESCRIPTION:
    Remote Simlock operation type
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_REMOTE_SIMLOCK_GENERATE_ENCRYPTED_KEY,
  RIL_UIM_REMOTE_SIMLOCK_PROCESS_SIMLOCK_DATA,
  RIL_UIM_REMOTE_SIMLOCK_GENERATE_HMAC,
  RIL_UIM_REMOTE_SIMLOCK_GET_MAX_SUPPORTED_VERSION,
  RIL_UIM_REMOTE_SIMLOCK_GET_STATUS,
  RIL_UIM_REMOTE_SIMLOCK_GENERATE_BLOB,
  RIL_UIM_REMOTE_SIMLOCK_START_UNLOCK_TIMER,
  RIL_UIM_REMOTE_SIMLOCK_STOP_UNLOCK_TIMER
} RIL_UIM_RemoteSimlockOperationType;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_RemoteSimlockResponseType
   DESCRIPTION:
    Remote Simlock response type
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_REMOTE_SIMLOCK_RESP_SUCCESS,
  RIL_UIM_REMOTE_SIMLOCK_RESP_FAILURE,
  RIL_UIM_REMOTE_SIMLOCK_RESP_GET_TIME_FAILED,
  RIL_UIM_REMOTE_SIMLOCK_RESP_TIMER_EXPIRED
} RIL_UIM_RemoteSimlockResponseType;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_RemoteSimlockStateType
   DESCRIPTION:
    Remote Simlock state
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_REMOTE_SIMLOCK_STATE_LOCKED,
  RIL_UIM_REMOTE_SIMLOCK_STATE_TEMPERORY_UNLOCK,
  RIL_UIM_REMOTE_SIMLOCK_STATE_PERMANENT_UNLOCK
} RIL_UIM_RemoteSimlockStateType;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_SAP_StatusType

   DESCRIPTION:
     APDU behavior
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_SAP_STATUS_UNKNOWN,
  RIL_UIM_SAP_DISCONNECT_GRACEFUL,
  RIL_UIM_SAP_DISCONNECT_IMMEDIATE,
  RIL_UIM_SAP_STATUS_CARD_RESET,
  RIL_UIM_SAP_STATUS_NOT_ACCESSIABLE,
  RIL_UIM_SAP_STATUS_CARD_REMOVED,
  RIL_UIM_SAP_STATUS_CARD_INSERTED,
  RIL_UIM_SAP_STATUS_RECOVERED
} RIL_UIM_SAP_StatusType;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_SAP_ConnectionResponse

   DESCRIPTION:
     SAP connection response
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_SAP_CONNECTION_SUCCESS,
  RIL_UIM_SAP_CONNECTION_FAILURE
} RIL_UIM_SAP_ConnectionResponse;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_SAP_ConnectionType

   DESCRIPTION:
     SAP Connection type
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_SAP_CONNECT,
  RIL_UIM_SAP_DISCONNECT,
} RIL_UIM_SAP_ConnectionType;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_SAP_RequestType

   DESCRIPTION:
     SAP Request Type
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_SAP_APDU_REQ,
  RIL_UIM_SAP_TRANSFER_ATR_REQ,
  RIL_UIM_SAP_POWER_REQ,
  RIL_UIM_SAP_RESET_SIM_REQ,
  RIL_UIM_SAP_TRANSFER_CARD_READER_STATUS_REQ
} RIL_UIM_SAP_RequestType;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_SAP_ResultCode

   DESCRIPTION:
     SAP request result type
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_SAP_SUCCESS                  = 0,
  RIL_UIM_SAP_GENERIC_FAILURE          = 1,
  RIL_UIM_SAP_CARD_NOT_ACCESSSIBLE     = 2,
  RIL_UIM_SAP_CARD_ALREADY_POWERED_OFF = 3,
  RIL_UIM_SAP_CARD_REMOVED             = 4,
  RIL_UIM_SAP_CARD_ALREADY_POWERED_ON  = 5,
  RIL_UIM_SAP_DATA_NOT_AVAILABLE       = 6,
  RIL_UIM_SAP_NOT_SUPPORTED            = 7
} RIL_UIM_SAP_ResultCode;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_SAP_CardState

   DESCRIPTION:
     SAP card state
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_SAP_CARD_STATE_NOT_ACCESSSIBLE,
  RIL_UIM_SAP_CARD_STATE_INSERTED,
  RIL_UIM_SAP_CARD_STATE_REMOVED,
  RIL_UIM_SAP_CARD_STATE_POWER_DOWN,
} RIL_UIM_SAP_CardState;


/*===========================================================================

                           STRUCTURES

===========================================================================*/
/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_AppStatus

   DESCRIPTION:
     Structure that holds card app status data
-------------------------------------------------------------------------------*/
typedef struct
{
  RIL_UIM_AppType        app_type;
  RIL_UIM_AppState       app_state;
  RIL_UIM_PersoSubstate  perso_substate; /* applicable only if app_state ==
                                            RIL_APPSTATE_SUBSCRIPTION_PERSO */
  std::string            aid_ptr;        /* null terminated string, e.g., from 0xA0, 0x00 -> 0x41,
                                           0x30, 0x30, 0x30 */
  std::string            app_label_ptr;  /* null terminated string */
  int32_t                pin1_replaced;  /* applicable to USIM, CSIM & ISIM */
  RIL_UIM_PinState       pin1;
  RIL_UIM_PinState       pin2;
} RIL_UIM_AppStatus;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_CardStatus

   DESCRIPTION:
     Structure that holds card status response data
-------------------------------------------------------------------------------*/
typedef struct
{
  RIL_UIM_Errno     err;
  RIL_UIM_CardState card_state;
  int32_t           physical_slot_id;
  std::string       atr;
  std::string       iccid;
  std::string       eid;
  RIL_UIM_PinState  universal_pin_state;             /* applicable to USIM and CSIM: RIL_PINSTATE_xxx */
  int32_t           gsm_umts_subscription_app_index; /* value < RIL_CARD_MAX_APPS, -1 if none */
  int32_t           cdma_subscription_app_index;     /* value < RIL_CARD_MAX_APPS, -1 if none */
  int32_t           ims_subscription_app_index;      /* value < RIL_CARD_MAX_APPS, -1 if none */
  int32_t           num_applications;                /* value <= RIL_CARD_MAX_APPS */
  RIL_UIM_AppStatus applications[RIL_UIM_CARD_MAX_APPS];
} RIL_UIM_CardStatus;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_SIM_PIN_Response

   DESCRIPTION:
     Structure that holds SIM PIN response data
-------------------------------------------------------------------------------*/
typedef struct {
  RIL_UIM_Errno    err;
  int32_t          no_of_retries;
} RIL_UIM_SIM_PIN_Response;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_PersoResponse

   DESCRIPTION:
     Structure that holds Network deperso response data
-------------------------------------------------------------------------------*/
typedef struct {
  RIL_UIM_Errno    err;
  int32_t          no_of_retries;
} RIL_UIM_PersoResponse;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_IMSI_Response

   DESCRIPTION:
     Structure that holds IMSI response data
-------------------------------------------------------------------------------*/
typedef struct {
  RIL_UIM_Errno    err;
  std::string      IMSI;
} RIL_UIM_IMSI_Response;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_SIM_IO

   DESCRIPTION:
     Structure that holds SIM IO request data
-------------------------------------------------------------------------------*/
typedef struct {
    int32_t      command;    /* one of the commands listed for TS 27.007 +CRSM*/
    int32_t      fileid;     /* EF id */
    std::string  path;       /* "pathid" from TS 27.007 +CRSM command.
                                 Path is in hex asciii format eg "7f205f70"
                                 Path must always be provided.
                             */
    int32_t      p1;
    int32_t      p2;
    int32_t      p3;
    std::string  data;       /* May be NULL*/
    std::string  pin2;       /* May be NULL*/
    std::string  aidPtr;     /* AID value, See ETSI 102.221 8.1 and 101.220 4, NULL if no value. */
} RIL_UIM_SIM_IO;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_SIM_IO_Response

   DESCRIPTION:
     Structure that holds SIM IO response data
-------------------------------------------------------------------------------*/
typedef struct {
    uint32_t          slot_id;
    RIL_UIM_Errno     err;
    int32_t           sw1;
    int32_t           sw2;
    std::string       simResponse;  /* In hex string format ([a-fA-F0-9]*), except for SIM_AUTHENTICATION
                                       response for which it is in Base64 format, see 3GPP TS 31.102 7.1.2 */
} RIL_UIM_SIM_IO_Response;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_OpenChannelResponse

   DESCRIPTION:
     Structure that holds SIM open channel response data
-------------------------------------------------------------------------------*/
typedef struct {
    RIL_UIM_Errno        err;
    int32_t              channel_id;
    std::vector<uint8_t> selectResponse;
} RIL_UIM_OpenChannelResponse;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_SIM_APDU

   DESCRIPTION:
     Structure that holds SIM SEND APDU request data
-------------------------------------------------------------------------------*/
typedef struct {
    int32_t      sessionid;  /* "sessionid" from TS 27.007 +CGLA command. Should be
                                ignored for +CSIM command. */

    /* Following fields are used to derive the APDU ("command" and "length"
       values in TS 27.007 +CSIM and +CGLA commands). */
    int32_t      cla;
    int32_t      instruction;
    int32_t      p1;
    int32_t      p2;
    int32_t      p3;         /* A negative P3 implies a 4 byte APDU. */
    std::string  data;
} RIL_UIM_SIM_APDU;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_SimRefreshIndication

   DESCRIPTION:
     Structure that holds SIM Refresh indication data
-------------------------------------------------------------------------------*/
typedef struct {
    RIL_UIM_SIM_RefreshResult result;
    int32_t                   ef_id; /* is the EFID of the updated file if the result is */
                                     /* SIM_FILE_UPDATE or 0 for any other result. */
    RIL_UIM_AppType           app_type;
    std::string               aid;   /* is AID(application ID) of the card application */
                                     /* See ETSI 102.221 8.1 and 101.220 4 */
                                     /*     For SIM_FILE_UPDATE result it can be set to AID of */
                                     /*         application in which updated EF resides or it can be */
                                     /*         NULL if EF is outside of an application. */
                                     /*     For SIM_INIT result this field is set to AID of */
                                     /*         application that caused REFRESH */
                                     /*     For SIM_RESET result it is NULL. */
} RIL_UIM_SIM_RefreshIndication;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_PersonalizationStatus

   DESCRIPTION:
     Used to get personaliztion status
-------------------------------------------------------------------------------*/
typedef struct {
    int verifyAttempts;
} RIL_UIM_PersonalizationStatus;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_PersonalizationReq

   DESCRIPTION:
     Used to activate & set the device's personalization data
-------------------------------------------------------------------------------*/
typedef struct {
    std::string controlKey;
    RIL_UIM_PersoSubstate persoType;
} RIL_UIM_PersonalizationReq;

/* -----------------------------------------------------------------------------
   STRUCT:     RIL_UIM_PersonalizationStatusResp

   DESCRIPTION:
     Used to convey the device's personalization status
-------------------------------------------------------------------------------*/
typedef struct {
    RIL_UIM_Errno  err;
    bool has_gwNWPersoStatus;
    RIL_UIM_PersonalizationStatus gwNWPersoStatus;
    bool has_gwNWSubsetPersoStatus;
    RIL_UIM_PersonalizationStatus gwNWSubsetPersoStatus;
    bool has_gwSPPersoStatus;
    RIL_UIM_PersonalizationStatus gwSPPersoStatus;
    bool has_gwCPPersoStatus;
    RIL_UIM_PersonalizationStatus gwCPPersoStatus;
    bool has_gwSIMPersoStatus;
    RIL_UIM_PersonalizationStatus gwSIMPersoStatus;
    bool has_gw1xNWType1PersoStatus;
    RIL_UIM_PersonalizationStatus gw1xNWType1PersoStatus;
    bool has_gw1xNWType2PersoStatus;
    RIL_UIM_PersonalizationStatus gw1xNWType2PersoStatus;
    bool has_gw1xRUIMPersoStatus;
    RIL_UIM_PersonalizationStatus gw1xRUIMPersoStatus;
    bool has_gwSPNPersoStatus;
    RIL_UIM_PersonalizationStatus gwSPNPersoStatus;
    bool has_gwSPEhplmnPersoStatus;
    RIL_UIM_PersonalizationStatus gwSPEhplmnPersoStatus;
    bool has_gwICCIDPersoStatus;
    RIL_UIM_PersonalizationStatus gwICCIDPersoStatus;
    bool has_gwIMPIPersoStatus;
    RIL_UIM_PersonalizationStatus gwIMPIPersoStatus;
    bool has_gwNSSPPersoStatus;
    RIL_UIM_PersonalizationStatus gwNSSPPersoStatus;
} RIL_UIM_PersonalizationStatusResp;

/* -----------------------------------------------------------------------------
   STRUCT:     RIL_UIM_SetAPDUBehavior

   DESCRIPTION:
     Struct for OEMHook set APDU behavior request
-------------------------------------------------------------------------------*/
typedef struct {
  uint32_t                  channel_id;
  RIL_UIM_APDU_Behavior     apdu_behavior;
} RIL_UIM_SetAPDUBehavior;

/* -----------------------------------------------------------------------------
   STRUCT:     RIL_UIM_TerminalCapabilityLPATLV

   DESCRIPTION:
     Struct to set the LPA terminal capability TLV
-------------------------------------------------------------------------------*/
typedef struct {
  RIL_UIM_Errno err;
  bool          tlv_present;
  uint8_t       value;
} RIL_UIM_TerminalCapabilityLPATLV;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_GIDResponse

   DESCRIPTION:
     Structure that holds SIM GID response data
-------------------------------------------------------------------------------*/
typedef struct {
    RIL_UIM_Errno        err;
    std::vector<uint8_t> GIDResponse;
} RIL_UIM_GIDResponse;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_PhySlotMap

   DESCRIPTION:
     Structure that holds device logical slot to physical slot mapping
-------------------------------------------------------------------------------*/
typedef struct {
    RIL_UIM_Errno        err;
    std::vector<uint8_t> slotMap;
} RIL_UIM_PhySlotMap;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_SlotStatus

   DESCRIPTION:
     Structure that holds device physical slot information
-------------------------------------------------------------------------------*/
typedef struct {
  RIL_UIM_CardState    card_state;
  RIL_UIM_SlotState    slot_state;
/* Logical slot is valid only when the slot state is ACTIVE */
  uint8_t              logical_slot;
  std::vector<uint8_t> iccid;
  std::vector<uint8_t> atr;
  std::vector<uint8_t> eid;
} RIL_UIM_SlotStatus;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_SlotStatusInfo

   DESCRIPTION:
     Structure that holds device physical slot information
-------------------------------------------------------------------------------*/
typedef struct {
  RIL_UIM_Errno                   err;
  std::vector<RIL_UIM_SlotStatus> slot_status;
} RIL_UIM_SlotsStatusInfo;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_GBA_IMPI_Params

   DESCRIPTION:
     Structure that holds GBA IMPI parameters
-------------------------------------------------------------------------------*/
typedef struct {
  uint8_t         slot_id;
  RIL_UIM_AppType app_type;
  bool            secure;
} RIL_UIM_GBA_IMPI_Params;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_GBA_IMPI_Response

   DESCRIPTION:
     Structure that holds GBA IMPI Respones
-------------------------------------------------------------------------------*/
typedef struct {
  RIL_UIM_Errno        err;
  bool                 is_impi_encrypted;
  std::vector<uint8_t> impi;
} RIL_UIM_GBA_IMPI_Response;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_GBA_InitParams

   DESCRIPTION:
     Structure that holds GBA init parameters
-------------------------------------------------------------------------------*/
typedef struct {
  uint8_t              slot_id;
  RIL_UIM_AppType      app_type;
  bool                 force_bootstrapping;
  std::vector<uint8_t> protocol;
  std::string          domain_name;
  std::string          apn;
} RIL_UIM_GBA_InitParams;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_GBA_InitResponse

   DESCRIPTION:
     Structure that holds GBA init response
-------------------------------------------------------------------------------*/
typedef struct {
  RIL_UIM_Errno          err;
  RIL_UIM_GBA_KsNafType  naf_type;
  std::vector<uint8_t>   naf_response;
  std::string            life_time;
  std::string            bootstrap_tid;
} RIL_UIM_GBA_InitResponse;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_RemoteSimlockVersion

   DESCRIPTION:
     Structure that holds Remote Simlock Version
-------------------------------------------------------------------------------*/
typedef struct {
  uint8_t  majorVersion;
  uint8_t  minorVersion;
} RIL_UIM_RemoteSimlockVersion;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_RemoteSimlockStatus

   DESCRIPTION:
     Structure that holds Remote Simlock Status
-------------------------------------------------------------------------------*/
typedef struct uimRemoteSimlockStatus {
  RIL_UIM_RemoteSimlockStateType   state;
  uint32_t                         unlockTime;
} RIL_UIM_RemoteSimlockStatus;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_RemoteSimlockResponse

   DESCRIPTION:
     Structure that holds Remote Simlock Response
-------------------------------------------------------------------------------*/
typedef struct RIL_UIM_RemoteSimlockResponse {
  RIL_UIM_RemoteSimlockResponseType  rsp_type;
  RIL_UIM_RemoteSimlockOperationType op_type;
  std::vector<uint8_t>               rsp_data;
  RIL_UIM_RemoteSimlockVersion       version;
  RIL_UIM_RemoteSimlockStatus        status;
  int32_t                            timer_value;
} RIL_UIM_RemoteSimlockResponse;

/* -----------------------------------------------------------------------------
   STRUCT:      RIL_UIM_SAP_Response

   DESCRIPTION:
     Structure that holds SAP Response
-------------------------------------------------------------------------------*/
typedef struct RIL_UIM_SAP_Response {
  RIL_UIM_SAP_ResultCode  result;
  std::vector<uint8_t>    atr_apdu_data;
  int32_t                 reader_status;
} RIL_UIM_SAP_Response;

#endif /* QCRIL_UIM_TYPES_H */
