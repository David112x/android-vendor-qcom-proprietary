/******************************************************************************
#  Copyright (c) 2012-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/******************************************************************************
  @file    qcrili.h
  @brief   qcril qmi - internal interfaces

  DESCRIPTION

******************************************************************************/


#ifndef QCRILI_H
#define QCRILI_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#ifdef QMI_RIL_UTF
#define __STDC_LIMIT_MACROS
#endif

#include "telephony/ril.h"
#include "telephony/ril_cdma_sms.h"
#include "IxErrno.h"
#ifndef __cplusplus
#include "qcril_log.h"
#endif
#include "qmi_client_instance_defs.h"
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <limits.h>
#include <framework/legacy.h>
#include <framework/qcril_event.h>
#include <modules/android/version.h>

#ifndef QCRIL_DATA_OFFTARGET

#include "qcril_features_def.h"

#endif /* QCRIL_DATA_OFFTARGET */

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */
/*===========================================================================

                   EXTERNAL DEFINITIONS AND TYPES

===========================================================================*/

#define QMI_RIL_SYS_PROP_NAME_BASEBAND    "ro.baseband"

extern int qmi_ril_is_qcom_ril_version_supported(int version);

#define QCRIL_NW_SCAN_SELECTION_RADIO_SUPPORT() \
    (qmi_ril_is_feature_supported(QMI_RIL_FEATURE_LEGACY_RAT) ||\
    qmi_ril_is_feature_supported(QMI_RIL_FEATURE_COMBINE_RAT))

#ifdef _MSC_VER
#define QCRIL_SNPRINTF _snprintf
#else
#define QCRIL_SNPRINTF snprintf
#endif

#define QCRIL_HOOK_HEADER_SIZE           16
#define QCRIL_HOOK_OEM_NAME              "QOEMHOOK"

/* This is not present in ril.h, so adding here */
#define PREF_NET_TYPE_LTE_CDMA_EVDO_GSM ( 23 )

/* Emergency number ECC list System Property */
#define QCRIL_ECC_LIST                   "ril.ecclist"
#define QCRIL_ECC_LIST_1                 "ril1.ecclist"
#define QCRIL_TRACK_HEAP_MEM             "persist.vendor.radio.track_heap_mem"

#define QMI_RIL_QMI_RILD_THREAD_NAME                    "rild"
#define QMI_RIL_QMI_MAIN_THREAD_NAME                    "main"
#define QMI_RIL_QMI_CALLBACK_THREAD_NAME                "qmi_cb"
#define QMI_RIL_EVENT_THREAD_NAME                       "event"
#define QMI_RIL_CORE_INIT_KICKER_THREAD_NAME            "core_init"
#define QMI_RIL_RESUMER_DEFERRED_ACTION_THREAD_NAME     "resumer_def"
#define QMI_RIL_IPC_RECEIVER_THREAD_NAME                "ipc_receiver"
#define QMI_RIL_LTE_COEX_RIVA_THREAD_NAME               "coex_opener"
#define QMI_RIL_UIM_REMOTE_CLIENT_SOCKET_THREAD_NAME    "uim_remote_client_socket"
#define QMI_RIL_UIM_REMOTE_SERVER_SOCKET_THREAD_NAME    "uim_remote_server_socket"
#define QMI_RIL_RADIO_CONFIG_SOCKET_THREAD_NAME         "radio_config_so"

#define QMI_RIL_IMS_PIL_MONITOR_THREAD_NAME             "pil_monitor"
#define QMI_RIL_IFC_THREAD_NAME                         "ims_flow_control"

#define QMI_RIL_NUM_DUAL_SUB              2
#define QMI_RIL_NUM_TRIPLE_SUB            3
#define QMI_RIL_NUM_SINGLE_ACTIVE         1
#define QMI_RIL_NUM_DUAL_ACTIVE           2

#define QCRIL_RESTORE_MODE_PREF_SUPPORT                 "persist.vendor.radio.restore_mode_pref"

#define QCRIL_ARR_SIZE( a ) ( sizeof( ( a ) ) / sizeof( ( a[ 0 ] ) ) )

/* Most Significant 16 bits are the Instance ID + Modem ID and Least Significant 16 bits are the user data */
#define QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, user_data )                               \
  ( ( ( instance_id & 0xFF ) << 24 ) | ( ( modem_id & 0xFF ) << 16 ) | ( user_data & 0xFFFF ) )

#define QCRIL_EXTRACT_INSTANCE_ID_FROM_USER_DATA( user_data )                                     \
  ( ( user_data & 0xFF000000 ) >> 24 )

#define QCRIL_EXTRACT_MODEM_ID_FROM_USER_DATA( user_data )                                        \
  ( ( user_data & 0xFF0000 ) >> 16 )

#define QCRIL_EXTRACT_USER_ID_FROM_USER_DATA( user_data )                                         \
  ( user_data & 0xFFFF )

/* Internal Token ID */
#define QCRIL_TOKEN_ID_INTERNAL         0xFFFF

/* Internal Request ID */
#define QCRIL_REQ_ID_INTERNAL           0xFFFF

/* Unsolicited notification status */
typedef enum
{
  QCRIL_UNSOL_NOTIFICATION_STATUS_NO_CHANGE, /* Indicates that no change in unsolicited notification status */
  QCRIL_UNSOL_NOTIFICATION_STATUS_ON,        /* Indicates that enabled unsolicited notification */
  QCRIL_UNSOL_NOTIFICATION_STATUS_OFF        /* Indicates that disabled unsolicited notification */
} qcril_unsol_notification_status_e_type;

/* Serving system info changed notification status */
typedef enum
{
  QCRIL_SSIC_NOTIFICATION_STATUS_NO_CHANGE,   /* Indicates that no need to send unsolicited network state changed notification */
  QCRIL_SSIC_NOTIFICATION_STATUS_INFO,        /* SS_INFO, may need to send unsolicited network state changed notification */
  QCRIL_SSIC_NOTIFICATION_STATUS_RSSI,        /* RSSI, may need to send unsolicited network state changed notification */
  QCRIL_SSIC_NOTIFICATION_STATUS_SRV_CHANGED, /* SRV_CHANGED, may need to send unsolicited network state changed notification */
  QCRIL_SSIC_NOTIFICATION_STATUS_REG_REJECT   /* REG-REJECT, may need to send unsolicited network state changed notification */
} qcril_ssic_notification_status_e_type;

typedef enum
{
    QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_NONE,
    QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_VT,
    QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS,
    QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_PRESENCE,
    QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_QTUNER
} qmi_ril_oem_hook_qmi_tunneling_service_id_type;

// TODO: check if we need to put this under some condition
typedef struct {
    int act_status;   // User SIM provisioning preference 0 - Deactivate, 1 - Activate
    int slot_id;       // slot id
} RIL_SetSubProvPreference;

typedef struct {
    int  user_preference;      // User SIM provisioning preference 0 - Deactivate, 1 - Activate
    int  current_sub_preference;   // Current SIM provisioning status  0 - Deactivate, 1 - Activate
} RIL_SubProvStatus;


QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_ANDROID_RIL_REQ);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_DEFAULT);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_GSTK);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_MBN_HW);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_MBN_SW);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_NAS);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_NONE);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_OEM);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_OEM_HOOK);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_PBM);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_SMS);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_IMS);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_UIM);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_UIM_GSDI);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_UIM_MMGSDI);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_RC_UIM);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_VOICE);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_DATA);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_PROV);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_CM);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_CELL_INFO);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_EMBMS);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_LTED);
QCRIL_DECLARE_LEGACY_EVENT_GROUP(QCRIL_GRP_ANDROID_RIL_UNSOL);

#define QCRIL_LEGACY_EVENT QCRIL_DECLARE_LEGACY_EVENT
#define QCRIL_LEGACY_INDICATION QCRIL_DECLARE_LEGACY_INDICATION
#include "qcril_eventlist.h"

/* Timed Callback information */
struct qcril_timed_callback_info;
typedef struct qcril_timed_callback_info qcril_timed_callback_info;
struct qcril_timed_callback_info
{
  uint32 timer_id;
  void*                     callback;
  void*                     extra_params;
  boolean                   need_free; // indicates if needs to free extra_params when canceling the callback
  qcril_timed_callback_info *next;
  qcril_timed_callback_info *prev;
};

typedef struct
{
    qcril_evt_e_type evt_originator;
    RIL_Errno        result;
} qcril_modem_restart_con_type;


typedef struct
{
    int32 dbg_trace_id;
} qcril_embms_enable_request_payload_type;

/* This is an internally used error and it does not match any of the errors in
   RIL_Errno(ril.h) */
#define RIL_E_SKIP_LTE_REATTACH  (101)

typedef void (*qcril_timed_callback_type) (qcril_timed_callback_handler_params_type* handler_params);

/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/

errno_enum_type qcril_cm_init( void );
boolean qcril_cm_screen_is_off( qcril_instance_id_e_type instance_id );
void qcril_cm_get_modem_capability( void );
void qcril_cm_process_network_info( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id,
                                    qcril_ssic_notification_status_e_type ssic_notification_status );

void qcril_data_qmi_wds_init(boolean from_ssr);
void qcril_data_qmi_wds_release(void);
void qcril_data_process_stack_switch(qcril_modem_stack_id_e_type old_stack_id, qcril_modem_stack_id_e_type new_stack_id, qcril_instance_id_e_type instance_id);
void qcril_data_request_set_data_profile
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);
void qcril_other_init( void );
void qcril_other_mute( qcril_instance_id_e_type instance_id, boolean mic_mute, boolean ear_mute );
void qcril_event_start( void );
void qcril_event_suspend();
void qcril_event_resume();


int qcril_setup_timed_callback_ex_params(     qcril_instance_id_e_type      instance_id,
                                              qcril_modem_id_e_type         modem_id,
                                              qcril_timed_callback_type     callback,
                                              void*                         extra_params,
                                              const struct timeval *        relativeTime,
                                              uint32 *                      timer_id
                                         );

int qcril_setup_timed_callback_ex_params_adv( qcril_instance_id_e_type      instance_id,
                                              qcril_modem_id_e_type         modem_id,
                                              qcril_timed_callback_type     callback,
                                              void*                         extra_params,
                                              boolean                       need_free,
                                              const struct timeval *        relativeTime,
                                              uint32 *                      timer_id
                                            );

int qcril_cancel_timed_callback( void *param );

void qcril_send_empty_payload_request_response(qcril_instance_id_e_type instance_id, RIL_Token t, qcril_evt_e_type request_id, RIL_Errno ril_err_no);

void qcril_send_request_ack(qcril_instance_id_e_type instance_id, RIL_Token token);

void qmi_ril_fw_android_request_flow_control_abandon_requests_family_main_thrd( int android_request_id, RIL_Errno cause );
void qmi_ril_fw_android_request_flow_control_abandon_all_requests_main_thrd( RIL_Errno cause, int is_unbind_cleanup );
void qmi_ril_fw_android_request_flow_control_drop_legacy_book_records( int voice_calls_related_only, int is_unbind_cleanup  );

pthread_t qmi_ril_fw_get_main_thread_id();

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */
void qcril_default_unsol_resp_params( qcril_instance_id_e_type instance_id, int response_id, qcril_unsol_resp_params_type *param );
void qcril_send_unsol_response( qcril_unsol_resp_params_type *param_ptr );
void qcril_hook_unsol_response( qcril_instance_id_e_type instance_id, qcril_evt_e_type unsol_event, void *data, uint32 data_len );

#ifdef __cplusplus
}
#endif  /* __cplusplus */


#ifdef QMI_RIL_UTF
int utf_pthread_create_handler( pthread_t *thread, const pthread_attr_t *attr,
                                void *(*start_routine) (void *), void *arg);
#endif

void qcril_release( void );
#define QCRIL_EXTERN( xxx_request ) \
    void qcril_##xxx_request ( const qcril_request_params_type *const params_ptr, \
                             qcril_request_return_type *const ret_ptr)
#define QCRIL_QMI_EXTERN( xxx_request ) \
    void qcril_##xxx_request ( const qcril_request_params_type *const params_ptr)

void qcril_qmi_mgr_modem_state_updated(qcril_instance_id_e_type instance_id, qcril_modem_state_e_type new_modem_state);
void qcril_qmi_mgr_voice_technology_updated(qcril_radio_tech_e_type new_voice_technology);
qcril_radio_tech_e_type qcril_qmi_mgr_get_known_voice_technology(void);

int  qmi_ril_ssr_in_progress();
/// returns ril instance
qcril_instance_id_e_type qmi_ril_get_process_instance_id(void);

/// get ecclist property name by instance_id
extern void get_ecc_property_name(char *ecc_prop_name);

/// returns sim card slot index for current RIL instance
uint32_t qmi_ril_get_sim_slot(void);

qcril_modem_stack_id_e_type qmi_ril_get_stack_id( qcril_instance_id_e_type instance_id );
void qcril_unsol_oem_socket_connected();

extern int ril_to_uim_is_dsds_enabled(void);
extern int ril_to_uim_is_tsts_enabled(void);

void qcril_modem_restart_confirm_suspend_resume_step( qcril_evt_e_type confirmation_evt, qcril_modem_restart_con_type * details );
void qmi_ril_core_init_enter_warp(void);

void qmi_ril_clear_thread_name(pthread_t thread_id);

void qmi_ril_enter_critical_section(void);
void qmi_ril_leave_critical_section(void);

void qmi_ril_clear_timed_callback_list(void);

void qmi_ril_reset_android_unsol_resp_dispatchable_table();
void qmi_ril_reset_unsol_resp_pending_list();

void qmi_ril_suppress_android_unsol_resp(int resp_id);
void qmi_ril_unsuppress_android_unsol_resp(int resp_id);

boolean qcril_request_check_if_suppressed
(
    uint32_t event_id,
    qcril_req_handler_type **event_handler
);

int qcril_request_suppress_request
(
    uint32_t                     event_id,
    qcril_req_handler_type *event_handler
);

int qcril_request_unsuppress_request
(
    uint32_t                     event_id,
    qcril_req_handler_type *event_handler
);

void qcril_request_clean_up_suppress_list();

int qmi_ril_retrieve_number_of_rilds();
qmi_client_qmux_instance_type qmi_ril_client_get_master_port(void);

// function to reset all globals and release shared data
int qmi_ril_reboot_cleanup();
int qmi_ril_threads_shutdown();

bool qcril_qmi_is_secondary_modem_present(void);
bool qmi_ril_peripheral_mng_is_secondary_modem_online(void);
void qcril_qmi_modem_power_set_voting_state_primary_modem(int state);
void qcril_qmi_modem_power_set_voting_state_secondary_modem(int state);
int qcril_qmi_modem_power_voting_state_primary_modem(void);
int qcril_qmi_modem_power_voting_state_secondary_modem(void);
boolean qcril_qmi_modem_power_is_voting_feature_supported_for_primary_modem(void);
void qcril_qmi_modem_power_process_apm_off();
void qcril_qmi_vote_up_secondary_modem(void);
void qcril_qmi_vote_down_primary_modem(void);
void qcril_qmi_vote_down_secondary_modem(void);

void qmi_ril_reset_multi_sim_ftr_info( void );

void qcril_update_event_table_entry(qcril_evt_e_type event_id, qcril_req_handler_type *handler);

/* Other RIL services */
QCRIL_EXTERN (other_request_set_mute);
QCRIL_EXTERN (other_request_get_mute);
QCRIL_EXTERN (other_request_oem_hook_strings);
QCRIL_EXTERN (other_request_oem_hook_nv_read);
QCRIL_EXTERN (other_request_oem_hook_nv_write);
QCRIL_EXTERN (other_request_oem_hook_go_dormant);
QCRIL_EXTERN (other_request_oem_hook_neighboring_cells_info);
QCRIL_EXTERN (qmi_ims_vt_dial_call);
QCRIL_EXTERN (qmi_ims_vt_end_call);
QCRIL_EXTERN (qmi_ims_vt_answer_call);
QCRIL_EXTERN (qmi_ims_vt_get_call_info);
QCRIL_EXTERN (qmi_ims_presence_enabler_state_req);
QCRIL_EXTERN (qmi_ims_presence_send_publish_req);
QCRIL_EXTERN (qmi_ims_presence_send_publish_xml_req);
QCRIL_EXTERN (qmi_ims_presence_send_unpublish_req);
QCRIL_EXTERN (qmi_ims_presence_send_subscribe_req);
QCRIL_EXTERN (qmi_ims_presence_send_subscribe_xml_req);
QCRIL_EXTERN (qmi_ims_presence_set_notify_fmt_req);
QCRIL_EXTERN (qmi_ims_presence_get_notify_fmt_req);
QCRIL_EXTERN (qmi_ims_presence_send_unsubscribe_req);
QCRIL_EXTERN (qmi_ims_presence_get_event_report_req);
QCRIL_EXTERN (qmi_ims_presence_set_event_report_req);

QCRIL_EXTERN (qmi_nas_request_query_roaming_preference);

#if (QCRIL_RIL_VERSION > 6)
//Google added this struct with version 7 whereas we introduced it in v6
#define RIL_SimRefreshResponse_v6 RIL_SimRefreshResponse_v7

#endif

#if (QCRIL_RIL_VERSION >= 6)

// ICS support
#undef RIL_CDMA_CallWaiting
#define RIL_CDMA_CallWaiting    RIL_CDMA_CallWaiting_v6

// These messages have been renamed on ICS to VOICE_REGISTRATION_STATE and
// VOICE_NETWORK_STATE_CHANGED. But RIL can just use the old names
//#define RIL_REQUEST_REGISTRATION_STATE              RIL_REQUEST_VOICE_REGISTRATION_STATE
#define RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED    RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED

// Added by Qualcomm to identify illegal SIM apps.
#define RIL_APPSTATE_ILLEGAL    RIL_APPSTATE_READY

#if (QCRIL_RIL_VERSION < 9)
#define RADIO_TECH_TD_SCDMA     17
#define RADIO_TECH_MAX          (RADIO_TECH_TD_SCDMA)

typedef struct {
  int rscp; /* The Received Signal Code Power in dBm multipled by -1.
             * Range : 25 to 120
             * INT_MAX: 0x7FFFFFFF denotes invalid value.
             * Reference: 3GPP TS 25.123, section 9.1.1.1 */
} RIL_TD_SCDMA_SignalStrength;

#else /* (QCRIL_RIL_VERSION < 9) */

#if (QCRIL_RIL_VERSION >= 10)
#  define RADIO_TECH_MAX (RADIO_TECH_5G)
#endif


#endif

#if (QCRIL_RIL_VERSION >= 9)
#define RIL_LTE_SignalStrength_qc RIL_LTE_SignalStrength_v8
#else /* (QCRIL_RIL_VERSION >= 9) */
#define RIL_LTE_SignalStrength_qc RIL_LTE_SignalStrength
#endif /* (QCRIL_RIL_VERSION >= 9) */

#if defined(QCRIL_DATA_OFFTARGET)
#define RADIO_TECH_TD_SCDMA  117
#define RADIO_TECH_LTE_CA (19)
#endif
typedef struct {
  RIL_GW_SignalStrength GW_SignalStrength;
  RIL_WCDMA_SignalStrength WCDMA_SignalStrength;
  RIL_CDMA_SignalStrength CDMA_SignalStrength;
  RIL_EVDO_SignalStrength EVDO_SignalStrength;
  RIL_LTE_SignalStrength_qc LTE_SignalStrength;
#ifndef QMI_RIL_UTF
  RIL_TD_SCDMA_SignalStrength TD_SCDMA_SignalStrength;
#endif
} RIL_SignalStrength_qc_internal;

#endif /* (QCRIL_RIL_VERSION >= 6) */

// SMS over IMS support using IMS stack in the modem is added by QCOM
// So if RIL_REQUEST_IMS_REGISTRATION_STATE is not defined, then these messages will
// not be defined in ril.h. Add these in here for compilation
// and to avoid conditionally compiled code in QMI RIL
#ifdef RIL_REQUEST_IMS_REGISTRATION_STATE
#define FEATURE_QCRIL_IMS
#else
#define RIL_REQUEST_IMS_REGISTRATION_STATE 10009
#define RIL_REQUEST_IMS_SEND_SMS 10010
#define RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED 11009

typedef enum {
    RADIO_TECH_FAMILY_UNKNOWN = 0,
    RADIO_TECH_3GPP = 1, /* 3GPP Technologies - GSM, WCDMA */
    RADIO_TECH_3GPP2 = 2 /* 3GPP2 Technologies - CDMA */
} RIL_RadioTechnologyFamily;

typedef struct {
    RIL_RadioTechnologyFamily tech;
    unsigned char             retry;       /* 0 == not retry, nonzero == retry */
    int                       messageRef;  /* Valid field if retry is set to nonzero.
                                              Contains messageRef from RIL_SMS_Response
                                              corresponding to failed MO SMS.
                                            */

    union {
        /* Valid field if tech is RADIO_TECH_3GPP2. See RIL_REQUEST_CDMA_SEND_SMS */
        RIL_CDMA_SMS_Message* cdmaMessage;

        /* Valid field if tech is RADIO_TECH_3GPP. See RIL_REQUEST_SEND_SMS */
        char**                gsmMessage;
    } message;
} RIL_IMS_SMS_Message;

#endif /* RIL_REQUEST_IMS_REGISTRATION_STATE */


#ifndef RIL_REQUEST_SETUP_QOS

#define RIL_REQUEST_SETUP_QOS      10012
#define RIL_REQUEST_RELEASE_QOS    10013
#define RIL_REQUEST_GET_QOS_STATUS 10014
#define RIL_REQUEST_MODIFY_QOS     10015
#define RIL_REQUEST_SUSPEND_QOS    10016
#define RIL_REQUEST_RESUME_QOS     10017
#define RIL_UNSOL_QOS_STATE_CHANGED_IND 11010

typedef enum {
    RIL_QOS_CONVERSATIONAL,
    RIL_QOS_STREAMING,
    RIL_QOS_INTERACTIVE,
    RIL_QOS_BACKGROUND
} RIL_QosClass;

typedef enum {
    RIL_QOS_TX,
    RIL_QOS_RX
} RIL_QosDirection;

/* QoS status */
typedef enum {
    RIL_QOS_STATUS_NONE,        /* Qos not active */
    RIL_QOS_STATUS_ACTIVATED,   /* Qos currently active */
    RIL_QOS_STATUS_SUSPENDED    /* Qos Suspended */
} RIL_QosStatus;

/* Enum for status of the QoS flows */
typedef enum {
    RIL_QOS_ACTIVATED,           /* QoS activation completed or QoS Resumed) */
    RIL_QOS_ACTIVATED_NETWORK,   /* QoS activation (from network) complete */
    RIL_QOS_USER_RELEASE,        /* QoS released by the user */
    RIL_QOS_NETWORK_RELEASE,     /* QoS released by the network */
    RIL_QOS_SUSPENDED,           /* QoS was suspended */
    RIL_QOS_MODIFIED,            /* QoS modified */
    RIL_QOS_ERROR_UNKNOWN        /* Any other error */
} RIL_QosIndStates;

/* Keys the QoS spec along with the description of their values.
 *
 * Each QoS Spec will begin with a unique SPEC_INDEX. Within each spec there can
 * be multiple filter sets, each of which will start with a unique FILTER_INDEX
 */
typedef enum {
    RIL_QOS_SPEC_INDEX,                         /* Positive numerical value */

    RIL_QOS_FLOW_DIRECTION,                     /* RIL_QosDirection */
    RIL_QOS_FLOW_TRAFFIC_CLASS,                 /* RIL_QosClass */
    RIL_QOS_FLOW_DATA_RATE_MIN,                 /* Positive number in bps */
    RIL_QOS_FLOW_DATA_RATE_MAX,                 /* Positive number in bps */
    RIL_QOS_FLOW_LATENCY,                       /* Positive number in milliseconds */

    RIL_QOS_FLOW_3GPP2_PROFILE_ID,              /* Positive numerical value */
    RIL_QOS_FLOW_3GPP2_PRIORITY,                /* Positive numerical value */

    RIL_QOS_FILTER_INDEX,                       /* Mandatory. Positive numerical value */
    RIL_QOS_FILTER_IPVERSION,                   /* Mandatory. Values must be "IP" or "IPV6" */
    RIL_QOS_FILTER_DIRECTION,                   /* RIL_QosDirection */
    RIL_QOS_FILTER_IPV4_SOURCE_ADDR,            /* Format: xxx.xxx.xxx.xxx/yy */
    RIL_QOS_FILTER_IPV4_DESTINATION_ADDR,       /* Format: xxx.xxx.xxx.xxx/yy */
    RIL_QOS_FILTER_IPV4_TOS,                    /* Positive numerical Value (max 6-bit number) */
    RIL_QOS_FILTER_IPV4_TOS_MASK,               /* Mask for the 6 bit TOS value */

    /**
     * *PORT_START is the starting port number,
     * *PORT_RANGE is the number of continuous ports from *PORT_START key
     */
    RIL_QOS_FILTER_TCP_SOURCE_PORT_START,
    RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE,
    RIL_QOS_FILTER_TCP_DESTINATION_PORT_START,
    RIL_QOS_FILTER_TCP_DESTINATION_PORT_RANGE,
    RIL_QOS_FILTER_UDP_SOURCE_PORT_START,
    RIL_QOS_FILTER_UDP_SOURCE_PORT_RANGE,
    RIL_QOS_FILTER_UDP_DESTINATION_PORT_START,
    RIL_QOS_FILTER_UDP_DESTINATION_PORT_RANGE,

    RIL_QOS_FILTER_IPV6_SOURCE_ADDR,        /* Format: xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx/yyy */
    RIL_QOS_FILTER_IPV6_DESTINATION_ADDR,   /* Format: xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx/yyy */
    RIL_QOS_FILTER_IPV6_TRAFFIC_CLASS,
    RIL_QOS_FILTER_IPV6_FLOW_LABEL
} RIL_QosSpecKeys;

#endif /* RIL_REQUEST_SETUP_QOS */

#define QMI_RIL_UNDER_PLAIN_ANDROID

#ifndef RIL_UNSOL_RESPONSE_TETHERED_MODE_STATE_CHANGED
#define RIL_UNSOL_RESPONSE_TETHERED_MODE_STATE_CHANGED 11037
#endif

/* QCRIL definition for RIL_RadioTechnology for get/setPreferredVoice/Data */

/* GSM/WCDMA (WCDMA preferred) */
#define QCRIL_PREF_NET_TYPE_GSM_WCDMA                            PREF_NET_TYPE_GSM_WCDMA

/* GSM only */
#define QCRIL_PREF_NET_TYPE_GSM_ONLY                             PREF_NET_TYPE_GSM_ONLY

/* WCDMA  */
#define QCRIL_PREF_NET_TYPE_WCDMA                                PREF_NET_TYPE_WCDMA

/* GSM/WCDMA (auto mode, according to PRL) */
#define QCRIL_PREF_NET_TYPE_GSM_WCDMA_AUTO                       PREF_NET_TYPE_GSM_WCDMA_AUTO

/* CDMA and EvDo (auto mode, according to PRL) */
#define QCRIL_PREF_NET_TYPE_CDMA_EVDO_AUTO                       PREF_NET_TYPE_CDMA_EVDO_AUTO

/* CDMA only */
#define QCRIL_PREF_NET_TYPE_CDMA_ONLY                            PREF_NET_TYPE_CDMA_ONLY

/* EvDo only */
#define QCRIL_PREF_NET_TYPE_EVDO_ONLY                            PREF_NET_TYPE_EVDO_ONLY

/* GSM/WCDMA, CDMA, and EvDo (auto mode, according to PRL) */
#define QCRIL_PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO             PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO

/* LTE, CDMA and EvDo */
#define QCRIL_PREF_NET_TYPE_LTE_CDMA_EVDO                        PREF_NET_TYPE_LTE_CDMA_EVDO

/* LTE, CDMA, EvDo and GSM */
#define QCRIL_PREF_NET_TYPE_LTE_CDMA_EVDO_GSM                    PREF_NET_TYPE_LTE_CDMA_EVDO_GSM

/* LTE, GSM/WCDMA */
#define QCRIL_PREF_NET_TYPE_LTE_GSM_WCDMA                        PREF_NET_TYPE_LTE_GSM_WCDMA

/* LTE, CDMA, EvDo, GSM/WCDMA */
#define QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA              PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA

/* LTE only */
#define QCRIL_PREF_NET_TYPE_LTE_ONLY                             PREF_NET_TYPE_LTE_ONLY

#define QCRIL_PREF_NET_TYPE_LTE_WCDMA                            PREF_NET_TYPE_LTE_WCDMA
#define QCRIL_PREF_NET_TYPE_TD_SCDMA_ONLY                        PREF_NET_TYPE_TD_SCDMA_ONLY
#define QCRIL_PREF_NET_TYPE_TD_SCDMA_WCDMA                       PREF_NET_TYPE_TD_SCDMA_WCDMA
#define QCRIL_PREF_NET_TYPE_TD_SCDMA_LTE                         PREF_NET_TYPE_TD_SCDMA_LTE
#define QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM                         PREF_NET_TYPE_TD_SCDMA_GSM
#define QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_LTE                     PREF_NET_TYPE_TD_SCDMA_GSM_LTE
#define QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA                   PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA
#define QCRIL_PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE                   PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE
#define QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE               PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE
#define QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO    PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO
#define QCRIL_PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA     PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA

#define RIL_REQUEST_UNKOWN  998
#define RIL_UNSOL_UNKOWN 1101

#ifndef RIL_REQUEST_GET_DC_RT_INFO
#define RIL_REQUEST_GET_DC_RT_INFO 10117

typedef enum {
    RIL_DC_POWER_STATE_UNKNOWN            = 0,    // Unknown state
    RIL_DC_POWER_STATE_INACTIVE           = 1,    // Data connection is in a inactive state
    RIL_DC_POWER_STATE_DORMANT            = 2,    // Data connection is in a low power state
    RIL_DC_POWER_STATE_NOT_DORMANT        = 3,    // Data connection is in a high power state
} RIL_DcPowerStates;

/**
 * Data connection real time info
 */
typedef struct {
    int                         call_id;    //Call identifier
    RIL_RadioTechnology         radioTech;  // Radio technology
    RIL_DcPowerStates           powerState; // Current power state
} RIL_DcRtInfo;

#endif

#ifndef RIL_UNSOL_DC_RT_INFO_CHANGED
#define RIL_UNSOL_DC_RT_INFO_CHANGED 11036
#endif

#ifndef MAX_RIL_MCC_MNC_SIZE
#define MAX_RIL_MCC_MNC_SIZE 4
#endif

#ifndef RIL_REQUEST_GET_CELL_INFO_LIST

#  define RIL_REQUEST_GET_CELL_INFO_LIST 10123
#  define RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE 10124
#  define RIL_UNSOL_CELL_INFO_LIST 11042

typedef struct {
    int signalStrength;  /* Valid values are (0-31, 99) as defined in TS 27.007 8.5 */
    int rsrp;            /* The current Reference Signal Receive Power in dBm multipled by -1.
                          * Range: 44 to 140 dBm
                          * INT_MAX: 0x7FFFFFFF denotes invalid value.
                          * Reference: 3GPP TS 36.133 9.1.4 */
    int rsrq;            /* The current Reference Signal Receive Quality in dB multiplied by -1.
                          * Range: 20 to 3 dB.
                          * INT_MAX: 0x7FFFFFFF denotes invalid value.
                          * Reference: 3GPP TS 36.133 9.1.7 */
    int rssnr;           /* The current reference signal signal-to-noise ratio in 0.1 dB units.
                          * Range: -200 to +300 (-200 = -20.0 dB, +300 = 30dB).
                          * INT_MAX : 0x7FFFFFFF denotes invalid value.
                          * Reference: 3GPP TS 36.101 8.1.1 */
    int cqi;             /* The current Channel Quality Indicator.
                          * Range: 0 to 15.
                          * INT_MAX : 0x7FFFFFFF denotes invalid value.
                          * Reference: 3GPP TS 36.101 9.2, 9.3, A.4 */
    int timingAdvance;   /* timing advance in micro seconds for a one way trip from cell to device.
                          * Approximate distance can be calculated using 300m/us * timingAdvance.
                          * Range: 0 to 0x7FFFFFFE
                          * INT_MAX : 0x7FFFFFFF denotes invalid value.
                          * Reference: 3GPP 36.321 section 6.1.3.5
                          * also: http://www.cellular-planningoptimization.com/2010/02/timing-advance-with-calculation.html */
} RIL_LTE_SignalStrength_v8;

typedef RIL_LTE_SignalStrength RIL_LTE_SignalStrength_v8;

typedef struct {
    RIL_GW_SignalStrength       GW_SignalStrength;
    RIL_CDMA_SignalStrength     CDMA_SignalStrength;
    RIL_EVDO_SignalStrength     EVDO_SignalStrength;
    RIL_LTE_SignalStrength_v8   LTE_SignalStrength;
} RIL_SignalStrength_v8;

/** RIL_CellIdentityGsm */
typedef struct {
    char mcc[MAX_RIL_MCC_MNC_SIZE];    /* 3-digit Mobile Country Code, 0..999, INT_MAX if unknown */
    char mnc[MAX_RIL_MCC_MNC_SIZE];    /* 2 or 3-digit Mobile Network Code, 0..999, INT_MAX if unknown */
    int lac;    /* 16-bit Location Area Code, 0..65535, INT_MAX if unknown  */
    int cid;    /* 16-bit GSM Cell Identity described in TS 27.007, 0..65535, INT_MAX if unknown  */
} RIL_CellIdentityGsm;

/** RIL_CellIdentityWcdma */
typedef struct {
    char mcc[MAX_RIL_MCC_MNC_SIZE];    /* 3-digit Mobile Country Code, 0..999, INT_MAX if unknown  */
    char mnc[MAX_RIL_MCC_MNC_SIZE];    /* 2 or 3-digit Mobile Network Code, 0..999, INT_MAX if unknown  */
    int lac;    /* 16-bit Location Area Code, 0..65535, INT_MAX if unknown  */
    int cid;    /* 28-bit UMTS Cell Identity described in TS 25.331, 0..268435455, INT_MAX if unknow
n  */
    int psc;    /* 9-bit UMTS Primary Scrambling Code described in TS 25.331, 0..511, INT_MAX if unk
nown */
} RIL_CellIdentityWcdma;

/** RIL_CellIdentityCdma */
typedef struct {
    int networkId;      /* Network Id 0..65535, INT_MAX if unknown */
    int systemId;       /* CDMA System Id 0..32767, INT_MAX if unknown  */
    int basestationId;  /* Base Station Id 0..65535, INT_MAX if unknown  */
    int longitude;      /* Longitude is a decimal number as specified in 3GPP2 C.S0005-A v6.0.
                         * It is represented in units of 0.25 seconds and ranges from -2592000
                         * to 2592000, both values inclusive (corresponding to a range of -180
                         * to +180 degrees). INT_MAX if unknown */

    int latitude;       /* Latitude is a decimal number as specified in 3GPP2 C.S0005-A v6.0.
                         * It is represented in units of 0.25 seconds and ranges from -1296000
                         * to 1296000, both values inclusive (corresponding to a range of -90
                         * to +90 degrees). INT_MAX if unknown */
} RIL_CellIdentityCdma;

/** RIL_CellIdentityLte */
typedef struct {
    char mcc[MAX_RIL_MCC_MNC_SIZE];    /* 3-digit Mobile Country Code, 0..999, INT_MAX if unknown  */
    char mnc[MAX_RIL_MCC_MNC_SIZE];    /* 2 or 3-digit Mobile Network Code, 0..999, INT_MAX if unknown  */
    int ci;     /* 28-bit Cell Identity described in TS ???, INT_MAX if unknown */
    int pci;    /* physical cell id 0..503, INT_MAX if unknown  */
    int tac;    /* 16-bit tracking area code, INT_MAX if unknown  */
} RIL_CellIdentityLte;

/** RIL_CellInfoGsm */
typedef struct {
  RIL_CellIdentityGsm   cellIdentityGsm;
  RIL_GW_SignalStrength signalStrengthGsm;
} RIL_CellInfoGsm;

/** RIL_CellInfoWcdma */
typedef struct {
  RIL_CellIdentityWcdma cellIdentityWcdma;
  RIL_WCDMA_SignalStrength signalStrengthWcdma;
} RIL_CellInfoWcdma;

/** RIL_CellInfoCdma */
typedef struct {
  RIL_CellIdentityCdma      cellIdentityCdma;
  RIL_CDMA_SignalStrength   signalStrengthCdma;
  RIL_EVDO_SignalStrength   signalStrengthEvdo;
} RIL_CellInfoCdma;

/** RIL_CellInfoLte */
typedef struct {
  RIL_CellIdentityLte        cellIdentityLte;
  RIL_LTE_SignalStrength_v8  signalStrengthLte;
} RIL_CellInfoLte;

// Must be the same as CellInfo.TYPE_XXX
typedef enum {
  RIL_CELL_INFO_TYPE_GSM    = 1,
  RIL_CELL_INFO_TYPE_CDMA   = 2,
  RIL_CELL_INFO_TYPE_LTE    = 3,
  RIL_CELL_INFO_TYPE_WCDMA  = 4,
} RIL_CellInfoType;

// Must be the same as CellInfo.TIMESTAMP_TYPE_XXX
typedef enum {
    RIL_TIMESTAMP_TYPE_UNKNOWN = 0,
    RIL_TIMESTAMP_TYPE_ANTENNA = 1,
    RIL_TIMESTAMP_TYPE_MODEM = 2,
    RIL_TIMESTAMP_TYPE_OEM_RIL = 3,
    RIL_TIMESTAMP_TYPE_JAVA_RIL = 4,
} RIL_TimeStampType;

typedef struct {
  RIL_CellInfoType  cellInfoType;   /* cell type for selecting from union CellInfo */
  int               registered;     /* !0 if this cell is registered 0 if not registered */
  RIL_TimeStampType timeStampType;  /* type of time stamp represented by timeStamp */
  uint64_t          timeStamp;      /* Time in nanos as returned by ril_nano_time */
  union {
    RIL_CellInfoGsm     gsm;
    RIL_CellInfoCdma    cdma;
    RIL_CellInfoLte     lte;
    RIL_CellInfoWcdma   wcdma;
  } CellInfo;
} RIL_CellInfo;

#endif // RIL_REQUEST_GET_CELL_INFO_LIST

#ifndef RIL_REQUEST_GET_CELL_INFO_LIST

#  define RIL_REQUEST_GET_CELL_INFO_LIST 10123
#  define RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE 10124
#  define RIL_UNSOL_CELL_INFO_LIST 11042

/** RIL_CellIdentityGsm */
typedef struct {
    char mcc[MAX_RIL_MCC_MNC_SIZE];    /* 3-digit Mobile Country Code, 0..999 */
    char mnc[MAX_RIL_MCC_MNC_SIZE];    /* 2 or 3-digit Mobile Network Code, 0..999 */
    int lac;    /* 16-bit Location Area Code, 0..65535 */
    int cid;    /* 16-bit GSM Cell Identity described in TS 27.007, 0..65535
                 * 28-bit UMTS Cell Identity described in TS 25.331, 0..268435455 */
    int psc;    /* 9-bit UMTS Primary Scrambling Code described in TS 25.331, 0..511 */
} RIL_CellIdentityGsm;

/** RIL_CellIdentityCdma */
typedef struct {
    int networkId;      /* Network Id 0..65535 */
    int systemId;       /* CDMA System Id 0..32767 */
    int basestationId;  /* Base Station Id 0..65535 */
    int longitude;      /* Longitude is a decimal number as specified in 3GPP2 C.S0005-A v6.0.
                         * It is represented in units of 0.25 seconds and ranges from -2592000
                         * to 2592000, both values inclusive (corresponding to a range of -180
                         * to +180 degrees). */

    int latitude;       /* Latitude is a decimal number as specified in 3GPP2 C.S0005-A v6.0.
                         * It is represented in units of 0.25 seconds and ranges from -1296000
                         * to 1296000, both values inclusive (corresponding to a range of -90
                         * to +90 degrees). */
} RIL_CellIdentityCdma;

/** RIL_CellIdentityLte */
typedef struct {
    char mcc[MAX_RIL_MCC_MNC_SIZE];    /* 3-digit Mobile Country Code, 0..999 */
    char mnc[MAX_RIL_MCC_MNC_SIZE];    /* 2 or 3-digit Mobile Network Code, 0..999 */
    int ci;     /* 28-bit Cell Identity described in TS ??? */
    int pci;    /* physical cell id 0..503 */
    int tac;    /* 16-bit tracking area code */
} RIL_CellIdentityLte;

/** RIL_CellInfoGsm */
typedef struct {
  RIL_CellIdentityGsm   cellIdentityGsm;
  RIL_GW_SignalStrength signalStrengthGsm;
} RIL_CellInfoGsm;

/** RIL_CellInfoCdma */
typedef struct {
  RIL_CellIdentityCdma      cellIdentityCdma;
  RIL_CDMA_SignalStrength   signalStrengthCdma;
  RIL_EVDO_SignalStrength   signalStrengthEvdo;
} RIL_CellInfoCdma;

/** RIL_CellInfoLte */
typedef struct {
  RIL_CellIdentityLte        cellIdentityLte;
  RIL_LTE_SignalStrength_v7  signalStrengthLte;
} RIL_CellInfoLte;

typedef enum {
  RIL_CELL_INFO_TYPE_GSM    = 1,
  RIL_CELL_INFO_TYPE_CDMA   = 2,
  RIL_CELL_INFO_TYPE_LTE    = 3,
} RIL_CellInfoType;

typedef struct {
  RIL_CellInfoType  cellInfoType;   /* cell type for selecting from union CellInfo */
  int               registered;     /* !0 if this cell is registered 0 if not registered */
  uint64_t          time;           /* Time in nanos as returned by ril_nano_time */
  union {
    RIL_CellInfoGsm     gsm;
    RIL_CellInfoCdma    cdma;
    RIL_CellInfoLte     lte;
  } CellInfo;
} RIL_CellInfo;

#endif // RIL_REQUEST_GET_CELL_INFO_LIST

#ifndef RIL_REQUEST_GET_DATA_CALL_PROFILE
#define RIL_REQUEST_GET_DATA_CALL_PROFILE 10111
#endif // RIL_REQUEST_GET_DATA_CALL_PROFILE

boolean qcril_is_binary_data_empty(qcril_binary_data_type bin_data);
qcril_binary_data_type qcril_get_empty_binary_data_type();
qcril_binary_data_type qcril_find_pattern(qcril_binary_data_type bin_data, const char *pattern);

#ifndef RIL_REQUEST_SET_LTE_BAND_PREF

/* LTE Band Pref */
typedef enum {
    E_UTRA_Operating_Band_1,
    E_UTRA_Operating_Band_2,
    E_UTRA_Operating_Band_3,
    E_UTRA_Operating_Band_4,
    E_UTRA_Operating_Band_5,
    E_UTRA_Operating_Band_6,
    E_UTRA_Operating_Band_7,
    E_UTRA_Operating_Band_8,
    E_UTRA_Operating_Band_9,
    E_UTRA_Operating_Band_10,
    E_UTRA_Operating_Band_11,
    E_UTRA_Operating_Band_12,
    E_UTRA_Operating_Band_13,
    E_UTRA_Operating_Band_14,
    E_UTRA_Operating_Band_17,
    E_UTRA_Operating_Band_33,
    E_UTRA_Operating_Band_34,
    E_UTRA_Operating_Band_35,
    E_UTRA_Operating_Band_36,
    E_UTRA_Operating_Band_37,
    E_UTRA_Operating_Band_38,
    E_UTRA_Operating_Band_39,
    E_UTRA_Operating_Band_40,
    E_UTRA_Operating_Band_18,
    E_UTRA_Operating_Band_19,
    E_UTRA_Operating_Band_20,
    E_UTRA_Operating_Band_21,
    E_UTRA_Operating_Band_24,
    E_UTRA_Operating_Band_25,
    E_UTRA_Operating_Band_41,
    E_UTRA_Operating_Band_42,
    E_UTRA_Operating_Band_43,
    E_UTRA_Operating_Band_23,
    E_UTRA_Operating_Band_26,
    E_UTRA_Operating_Band_32,
    E_UTRA_Operating_Band_125,
    E_UTRA_Operating_Band_126,
    E_UTRA_Operating_Band_127
} RIL_LTE_band_pref;

#define RIL_REQUEST_SET_LTE_BAND_PREF 234

#endif // RIL_REQUEST_SET_LTE_BAND_PREF

/* Tx Power Levels */
typedef enum {
   RIL_TX_POWER_LEVEL_ONE =  0,
   RIL_TX_POWER_LEVEL_TWO = 1,
   RIL_TX_POWER_LEVEL_THREE = 2,
   RIL_TX_POWER_LEVEL_FOUR = 3,
   RIL_TX_POWER_LEVEL_FIVE = 4,
   RIL_TX_POWER_LEVEL_MAX = 5
} RIL_TxPowerLevel;

#ifndef RIL_REQUEST_GET_ACTIVITY_INFO

typedef struct {

  /* period (in ms) when modem is power collapsed */
  uint32_t sleep_mode_time_ms;

  /* period (in ms) when modem is awake and in idle mode*/
  uint32_t idle_mode_time_ms;

  /* period (in ms) for which Tx is active */
  uint32_t tx_mode_time_ms[RIL_NUM_TX_POWER_LEVELS];

  /* period (in ms) for which Rx is active */
  uint32_t rx_mode_time_ms;
} RIL_ActivityStatsInfo;

#ifndef QMI_RIL_UTF
#define RIL_REQUEST_GET_ACTIVITY_INFO 10135
#else
#define RIL_REQUEST_GET_ACTIVITY_INFO 235
#endif

#endif // RIL_REQUEST_GET_ACTIVITY_INFO

#define QCRIL_INTERNAL_REQ_TOKEN_END        ((uintptr_t)-1)
#define QCRIL_INTERNAL_REQ_TOKEN_BASE       (QCRIL_INTERNAL_REQ_TOKEN_END - 0xFF)
#define QCRIL_SUB_PROVISION_INTERNAL_TOKEN  (QCRIL_INTERNAL_REQ_TOKEN_BASE + 1)
#define QCRIL_MBN_INTERNAL_TOKEN_ID         (QCRIL_INTERNAL_REQ_TOKEN_BASE + 2)
#define QCRIL_SHUTDOWN_INTERNAL_TOKEN       (QCRIL_INTERNAL_REQ_TOKEN_BASE + 3)
#define QCRIL_CUSTOM_ECC_INTERNAL_TOKEN     (QCRIL_INTERNAL_REQ_TOKEN_BASE + 4)
#define QCRIL_RADIO_POWER_INTERNAL_TOKEN    (QCRIL_INTERNAL_REQ_TOKEN_BASE + 5)

/* -----------------------------------------------------------------------------
-------------------------------------------------------------------------------*/

#if QCRIL_RIL_VERSION < 12
#define RIL_CELL_INFO_LIST_TYPE RIL_CellInfo
#else
#define RIL_CELL_INFO_LIST_TYPE RIL_CellInfo_v12
#endif

#define CELL_INFO_NULL ((RIL_CELL_INFO_LIST_TYPE *)NULL)
#define CELL_INFO_TYPEOF(um, sm, stem) __typeof__(CELL_INFO_NULL->CellInfo.um.sm##stem)
#define CELL_INFO_TYPEOF_ID(um, stem)  CELL_INFO_TYPEOF(um, cellIdentity, stem)
#define CELL_INFO_TYPEOF_SS(um, stem)  CELL_INFO_TYPEOF(um, signalStrength, stem)

#define RIL_CELL_INFO_LIST_GSM_ID_TYPE   CELL_INFO_TYPEOF_ID(gsm,   Gsm)
#define RIL_CELL_INFO_LIST_GSM_SS_TYPE   CELL_INFO_TYPEOF_SS(gsm,   Gsm)
#define RIL_CELL_INFO_LIST_WCDMA_ID_TYPE CELL_INFO_TYPEOF_ID(wcdma, Wcdma)
#define RIL_CELL_INFO_LIST_WCDMA_SS_TYPE CELL_INFO_TYPEOF_SS(wcdma, Wcdma)
#define RIL_CELL_INFO_LIST_LTE_ID_TYPE   CELL_INFO_TYPEOF_ID(lte,   Lte)
#define RIL_CELL_INFO_LIST_LTE_SS_TYPE   CELL_INFO_TYPEOF_SS(lte,   Lte)

#define DATA_PARTITION_ROOT "/data/vendor/"

#define PROPERTY_NAME_MAX    80

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* QCRILI_H */
