/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_ril_api.h
  @brief   RIL API's for test case dev

  DESCRIPTION

  API's for modifying RIL messages used in RIL UTF tests.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _H_QCRIL_UNITTEST_RIL_DEFS_
#define _H_QCRIL_UNITTEST_RIL_DEFS_

#include "ril.h"
//#include "ril_utf_defs.h"
#include "ril_utf_core.h"

#define PACKED_STRUCT __attribute__ ((__packed__))

#define NAS_REG_STATE_RESP_MAX_ARR_SIZE 15
#define NAS_REG_DATA_STATE_RESP_MAX_ARR_SIZE (11)
#define NAS_OPERATOR_RESP_MAX_ARR_SIZE (3)
#define NAS_OPERATOR_RESP_MAX_EONS_LEN (512)
#define NAS_OPERATOR_RESP_MAX_MCC_MNC_LEN (16)
#define NAS_NW_SCAN_RESP_ENTRY_SIZE     (5)
#define NAS_NW_SCAN_RESP_MCCMNC_SIZE    (12)
#define NAS_NW_SCAN_RESP_RAT_SIZE       (3)
#define NAS_3GPP_NETWORK_INFO_LIST_MAX_V01 40

#define MAX_AVAILABLE_BANDS (20)

#define IMEI_MAX_STR_SIZE         (17)
#define IMEISV_MAX_STR_SIZE       (3)
#define ESN_MAX_STR_SIZE          (9)
#define MEID_MAX_STR_SIZE         (15)

#define SW_VERSION_MAX (32)

/*
  messages with no structure ( due to 0 len payload)
*/

/*
   Although RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED does not have
   a payload, this is a special instance where, RilD upon receiving
   the message issues the RIL_RadioStateRequest to query the
   radio state from QMI RIL (from within the context of the thread
   that is issuing the RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED ind.
   So tests can specify the return value of the RIL_RadioStateRequest
   call in the "state" in below struct.
*/
struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t
{
  RIL_RadioState state;
};

struct PACKED_STRUCT ril_unsol_resp_voice_radio_tech_changed_t
{
  RIL_RadioTechnology tech;
};

struct PACKED_STRUCT ril_request_radio_power_t
{
  int power;
};

struct PACKED_STRUCT ril_unsol_signal_strength_t
{
  RIL_SignalStrength signal_strength; // no longer in use
};

typedef struct PACKED_STRUCT ril_unsol_ringback_tone_t
{
  bool start;
} ril_unsol_ringback_tone_t;

typedef struct PACKED_STRUCT
{
  char* SMSC_address;
  char* SMS_pdu_format;
}ril_request_send_sms_t;

typedef struct PACKED_STRUCT
{
  char* SMSC_address;
  char* SMS_pdu_format;
}ril_request_send_sms_expect_more_t;

typedef struct PACKED_STRUCT
{
  int ack_recieved;
  int failure_cause;
}ril_request_sms_acknowledge_t;

typedef struct PACKED_STRUCT
{
  int response;
}ril_request_write_sms_to_sim_resp_t;

typedef struct PACKED_STRUCT
{
  int record_index;
}ril_request_delete_sms_on_sim_t;

typedef struct PACKED_STRUCT
{
  char SMSC_address[28];  // 21+3+3+1 ie., '"<SMSC(21chars)>",<SMSCAddressType(3chars)>'
}ril_request_get_smsc_address_resp_t ;

typedef struct PACKED_STRUCT
{
  char* SMSC_address;
}ril_request_set_smsc_address_t;

typedef enum {
RIL_SMS_MEMORY_STATUS_AVAILABLE = 1,
RIL_SMS_MEMORY_STATUS_EXCEEDED = 0
}ril_sms_memory_status_enum_type;

typedef struct PACKED_STRUCT
{

  ril_sms_memory_status_enum_type sms_memory_status;

}ril_request_report_sms_memory_status_t;

typedef enum
{
  RIL_GSM_SMS_BROADCAST_ACTIVATE = 1,
  RIL_GSM_SMS_BROADCAST_TURN_OFF = 0
}ril_gsm_sms_broadcast_activation_enum_type;

typedef struct PACKED_STRUCT
{
  ril_gsm_sms_broadcast_activation_enum_type activation_state;
}ril_request_gsm_sms_broadcast_activation_t;

typedef struct PACKED_STRUCT
{
  int record_index;
}ril_request_cdma_delete_sms_on_ruim_t;

typedef enum
{
  RIL_CDMA_SMS_BROADCAST_ACTIVATE = 1,
  RIL_CDMA_SMS_BROADCAST_TURN_OFF = 0
}ril_cdma_sms_broadcast_activation_enum_type;

typedef struct PACKED_STRUCT
{
  ril_cdma_sms_broadcast_activation_enum_type activation_state;

}ril_request_cdma_sms_broadcast_activation_t;

typedef struct PACKED_STRUCT
{
  int connection_index;
}ril_request_hangup_t;

/*
typedef struct PACKED_STRUCT
{
  RIL_LastCallFailCause failure_reason;
  char *                vendor_cause;
}ril_request_last_call_fail_cause_resp_t;
*/

typedef struct PACKED_STRUCT
{
  char* dtmf; // a single character with one of 12 values: 0-9,*,#
}ril_request_dtmf_t;

typedef struct PACKED_STRUCT
{
  char flash_string[QMI_VOICE_FLASH_PAYLOAD_MAX_V02+1];
}ril_request_cdma_flash_t;

typedef struct PACKED_STRUCT
{
  char *DTMF_string;
  char *DTMF_ON_length;
  char *DTMF_OFF_length;
}ril_request_cdma_burst_dtmf_t;

typedef enum
{
  TTY_Off = 0,
  TTY_Full= 1,
  TTY_HCO = 2,
  TTY_VCO = 3
}ril_tty_mode_enum_type;

typedef struct PACKED_STRUCT
{
  ril_tty_mode_enum_type tty_mode;
}ril_request_set_tty_mode_t;

typedef struct PACKED_STRUCT
{
  ril_tty_mode_enum_type tty_mode;
}ril_request_get_tty_mode_t;

typedef enum
{
  STANDARD_PRIVACY_MODE = 0 ,
  ENHANCED_PRIVACY_MODE = 1
}ril_voice_privacy_mode_enum_type;

typedef struct PACKED_STRUCT
{
  ril_voice_privacy_mode_enum_type voice_privacy_mode;
}ril_request_cdma_set_preferred_voice_privacy_mode_t;

typedef struct PACKED_STRUCT
{
  ril_voice_privacy_mode_enum_type voice_privacy_mode;
}ril_request_cdma_query_preferred_voice_privacy_mode_t;

typedef struct PACKED_STRUCT
{
  char *ussd_request;
}ril_request_send_ussd_t;

typedef struct PACKED_STRUCT
{
  int n_parameter;
  int m_parameter;
}ril_request_get_clir_resp_t;

typedef struct PACKED_STRUCT
{
  int n_parameter;
}ril_request_set_clir_t;

typedef struct PACKED_STRUCT
{
  int32_t rate;
}ril_request_unsol_cell_info_rate_t;

typedef struct PACKED_STRUCT
{
  bool enable;
}ril_request_mute_t;

typedef struct PACKED_STRUCT
{
  int service_class;
}ril_request_query_call_waiting_t;

typedef enum
{
  DISABLED = 0 ,
  ENABLED = 1
}ril_call_waiting_status_enum_type;

typedef struct PACKED_STRUCT
{
  ril_call_waiting_status_enum_type status;
}ril_request_query_call_waiting_resp_t;

typedef struct PACKED_STRUCT
{
  ril_call_waiting_status_enum_type status;
  int service_class;
}ril_request_set_call_waiting_t;

typedef struct PACKED_STRUCT
{
  char *facility_string_code;
  char *password;
  char *service_class_bit_vector;
  char *AID_value;
}ril_request_query_facility_lock_t;

typedef struct PACKED_STRUCT
{
  int service_class;
}ril_request_query_facility_lock_resp_t;

typedef struct PACKED_STRUCT
{
  char *facility_string_code;
  char *lock_status;
  char *password;
  char *service_class_bit_vector;
  char *AID_value;
}ril_request_set_facility_lock_t;

typedef struct PACKED_STRUCT
{
  int no_of_retries_remaining;
}ril_request_set_facility_lock_resp_t;

typedef struct PACKED_STRUCT
{
  char *facility_string_code;
  char *old_password;
  char *new_password;
}ril_request_change_barring_password_t;

typedef struct PACKED_STRUCT
{
  int connection_index;
}ril_request_separate_connection_t;

typedef enum
{
  CLIP_provisioned = 1 ,
  CLIP_Not_provisioned = 0 ,
  Unkown = 2
}ril_clip_status_enum_type;

typedef struct PACKED_STRUCT
{
  ril_clip_status_enum_type clip_status;
}ril_request_query_clip_resp_t;
typedef enum
{
  NOTIFICATIONS_ENABLED = 1,
  NOTIFICATIONS_DISABLED = 0
}ril_supp_svc_notification_status_enum_type;
typedef struct PACKED_STRUCT
{
  ril_supp_svc_notification_status_enum_type notification_status;
}ril_request_set_supp_svc_notification_t;

typedef enum
{
  AUTOMATIC_SELECTION = 0,
  MANUAL_SELECTION = 1
}ril_network_selection_mode_enum_type;

typedef struct PACKED_STRUCT
{
   ril_network_selection_mode_enum_type network_selection_mode;
}ril_request_query_network_selection_mode_resp_t;

typedef struct PACKED_STRUCT
{
  char *MCCMNC;
  char *rat;
}ril_request_set_network_selection_manual_t;

typedef enum
{
  unspecified = 0, // (selected by baseband automatically)
  EURO_band = 1,  // (GSM-900 / DCS-1800 / WCDMA-IMT-2000)
  US_band = 2,  // (GSM-850 / PCS-1900 / WCDMA-850 / WCDMA-PCS-1900)
  JPN_band = 3, // (WCDMA-800 / WCDMA-IMT-2000)
  AUS_band = 4, // (GSM-900 / DCS-1800 / WCDMA-850 / WCDMA-IMT-2000)
  AUS_band_2 = 5, // (GSM-900 / DCS-1800 / WCDMA-850)
  Cellular  = 6, // (800-MHz Band)
  PCS = 7, // (1900-MHz Band)
  Band_Class_3 = 8 ,  // (JTACS Band)
  Band_Class_4  = 9, // (Korean PCS Band)
  Band_Class_5 = 10, // (450-MHz Band)
  Band_Class_6  = 11, // (2-GMHz IMT2000 Band)
  Band_Class_7 = 12, // (Upper 700-MHz Band)
  Band_Class_8 = 13, // (1800-MHz Band)
  Band_Class_9 = 14, // (900-MHz Band)
  Band_Class_10 = 15, // (Secondary 800-MHz Band)
  Band_Class_11 = 16, // (400-MHz European PAMR Band)
  Band_Class_15 = 17, // (AWS Band)
  Band_Class_16 = 18 // (US 2.5-GHz Band)
}ril_band_mode_enum_type;

typedef struct  PACKED_STRUCT
{
  ril_band_mode_enum_type band_mode;
}ril_request_set_band_mode_t;

typedef struct  PACKED_STRUCT
{
  ril_band_mode_enum_type band_mode;
}ril_request_query_available_band_mode_resp_t;

typedef enum
{
  Home_Networks_only= 0,
  Roaming_on_Affiliated_networks = 1 ,
  Roaming_on_Any_Network = 2
}ril_cdma_roaming_preference_enum_type;

typedef struct PACKED_STRUCT
{
  ril_cdma_roaming_preference_enum_type roaming_preference;
}ril_request_cdma_set_roaming_preference_t;

typedef struct  PACKED_STRUCT
{
  ril_cdma_roaming_preference_enum_type roaming_preference;
}ril_request_cdma_query_roaming_preference_resp_t;

typedef enum
{
  DATA_ACTIVE_ON_SUBSCRIPTION_0 = 0,
  DATA_ACTIVE_ON_SUBSCRIPTION_1 = 1
}ril_data_subscription_enum_type;

typedef struct PACKED_STRUCT
{
  ril_data_subscription_enum_type subscription_status;
}ril_request_get_data_subscription_resp_t;

typedef enum
{
  SingleStandBy_Mode = 1 ,
  DualStandBy_Mode = 2
}ril_subscription_mode_enum_type;

typedef struct PACKED_STRUCT
{
  ril_subscription_mode_enum_type subscription_mode;
}ril_request_set_subscription_mode_t;

typedef struct
{
  char long_eons[ NAS_OPERATOR_RESP_MAX_EONS_LEN ];
  char short_eons[ NAS_OPERATOR_RESP_MAX_EONS_LEN ];
  char mcc_mnc_ascii[ NAS_OPERATOR_RESP_MAX_MCC_MNC_LEN ];
} ril_request_operator_resp_t;

typedef struct
{
  char *operator_info_array[ NAS_OPERATOR_RESP_MAX_ARR_SIZE ];
} ril_request_operator_resp_from_qcril_t;

typedef struct
{
    char long_eons[ NAS_OPERATOR_RESP_MAX_EONS_LEN ];
    char short_eons[ NAS_OPERATOR_RESP_MAX_EONS_LEN ];
    char mccmnc_info[ NAS_NW_SCAN_RESP_MCCMNC_SIZE ] ;
    char network_status[10];
    char rat[ NAS_NW_SCAN_RESP_RAT_SIZE ] ;
} qcril_qmi_nw_scan_resp_helper_type;

typedef struct
{
 char *nw_scan_info_array[ NAS_NW_SCAN_RESP_ENTRY_SIZE * NAS_3GPP_NETWORK_INFO_LIST_MAX_V01 ];
} ril_request_query_available_networks_resp_t;

typedef struct
{
    RIL_EmergencyList ecc_list;
} ril_unsol_emergency_list_t;

typedef enum
{
  Screen_on = 1,
  Scree_off = 0
}ril_screen_state_enum_type;

typedef struct PACKED_STRUCT
{
  ril_screen_state_enum_type screen_state;
}ril_request_screen_state_t;

typedef struct
{
  char imei[ IMEI_MAX_STR_SIZE ];
  char imeisv[ IMEISV_MAX_STR_SIZE ];
  char esn[ ESN_MAX_STR_SIZE ];
  char meid[ MEID_MAX_STR_SIZE ];
} ril_request_device_identity_type_t;

typedef struct
{
    char swver[SW_VERSION_MAX];
} ril_request_get_baseband_version_type_t;

typedef struct
{
    char alpha[QMI_VOICE_ALPHA_TEXT_MAX_V02+2];
} ril_unsol_stk_cc_alpha_notify_t;

typedef enum
{
  UPDATES_ENABLED = 1 ,
  UPDATES_DISABLED = 0
}ril_location_updates_enum_type;
typedef struct  PACKED_STRUCT
{
  ril_location_updates_enum_type updates_status;
}ril_request_set_location_updates_t;

void update_default_ril_unsol_resp_radio_state_changed(elnptr node);
#ifndef QCRIL_QMI_NAS_H
typedef enum
{
    SUBSCRIPTION_SOURCE_RUIM_SIM = 0,
    SUBSCRIPTION_SOURCE_NV,
    SUBSCRIPTION_SOURCE_RUIM_IF_AVAIL
}oem_hook_cdma_subscription_source_e_type;
#endif

typedef struct
{
    RIL_RadioTechnologyFamily tech;
    unsigned char             retry;
    int                       messageRef;
    char**                gsmMessage;
}ims_send_sms_3gpp_payload;

typedef struct
{
    RIL_RadioTechnologyFamily tech;
    unsigned char             retry;
    int                       messageRef;
    RIL_CDMA_SMS_Message*     cdmaMessage;
}ims_send_sms_3gpp2_payload;

#ifdef RIL_UTF_L_MASTER
struct PACKED_STRUCT ril_unsol_srvcc_state_notify_t
{
  RIL_SrvccState srvcc_state;
};
#endif

#endif
