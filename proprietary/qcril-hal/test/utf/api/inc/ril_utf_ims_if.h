/******************************************************************************
#  Copyright (c) 2013, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_ims_if.h
  @brief   ims API's for test case dev

  DESCRIPTION

  API's for modifying IMS messages used in RIL UTF tests.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef RIL_UTF_IMS_IF_H_
#define RIL_UTF_IMS_IF_H_

#include "ril_utf_if.h"

struct utf_ims_mt_sms_t {
  RIL_RadioTechnologyFamily tech;
  uint8_t* pdu;
  size_t size;
  //qcril::interfaces::VerificationStatus ver_status;
};

struct utf_ims_ack_sms_t {
  uint32_t message_id;
  int delivery_result;
};

typedef struct ims_ril_ss_ind_type {
  RIL_StkCcUnsolSsResponse *ril_ss_response;
  int                       cb_num_list_len;
  ims_CbNumList            *cb_num_list;
} ims_ril_ss_ind_type;

//RIL interfaces extracted from ril_utf_if.h
typedef class ril_utf_ims_if : public ril_utf_node_base_t {
public:
//==================================================
// IMS Interfaces
//==================================================
  /*
    ims_MsgId_REQUEST_IMS_REGISTRATION_STATE
  */
  void update_default_ims_request_registration_state(void);
  ims_Registration *update_default_ims_request_registration_state_resp(void);
  void set_ims_request_registration_state_resp_state(ims_Registration_RegState state);
  void set_ims_request_registration_state_resp_radioTech(ims_RadioTechType  val);
  void set_ims_request_registration_state_resp_pAssociatedUris(const char *val);
  void set_ims_request_registration_state_resp_errorcode(int has_errorcode, uint32_t errorcode, char *errormessage);

  /*
    ims_MsgId_REQUEST_DIAL
  */
  ims_Dial* update_default_ims_request_dial(char *address);
  ims_CallDetails *set_ims_request_dial_call_details();
  void update_default_ims_request_dial_resp(void);
  void set_ims_request_dial_isconferenceuri();
  void set_ims_request_dial_isencrypted(bool isEncrypted);

  /*
    ims_MsgId_REQUEST_ADD_PARTICIPANT
  */
  ims_Dial* update_default_ims_request_add_participant(char *address);
  ims_CallDetails *set_ims_request_add_participant_call_details();
  void update_default_ims_request_add_participant_resp(void);

  /*
    ims_MsgId_REQUEST_ANSWER
  */
  void update_default_ims_request_answer(void);
  void set_ims_request_answer_call_type(ims_CallType call_type);
  void set_ims_request_answer_presentation(ims_IpPresentation pres);
  void update_default_ims_request_answer_resp(void);

  /*
    ims_MsgId_REQUEST_HANGUP
  */
  ims_Hangup *update_default_ims_request_hangup(void);
  void set_ims_request_hangup_conn_index(uint32_t conn_index);
  void set_ims_request_hangup_conn_uri(char *uri);
  void set_ims_request_hangup_multi_party(bool multi_party);
  void set_ims_request_hangup_conf_id(uint32_t conf_id);
  void set_ims_request_hangup_callfailcauseresponse_errorinfo(char *val);
  void set_ims_request_hangup_callfailcauseresponse_failcause(ims_CallFailCause val);
  void update_default_ims_request_hangup_resp(void);

  /**/
#ifdef RIL_UTF_L_MASTER
  ims_CallFailCauseResponse* set_ims_callfailcauseresponse (ims_CallFailCause failcause, uint8_t val);
  ims_CallFailCauseResponse* set_ims_callfailcauseresponse_no_cause ();
#endif
  /*
    ims_MsgId_REQUEST_LAST_CALL_FAIL_CAUSE
  */
  void update_default_ims_request_last_call_fail_cause(void);
  void update_default_ims_request_last_call_fail_cause_resp(void);
  void set_ims_request_last_call_fail_cause_failcause(ims_CallFailCause failcause);
  void set_ims_request_last_call_fail_cause_networkerrorstring(char *networkerrorstring);

  /*
    ims_MsgId_REQUEST_GET_CURRENT_CALLS
  */
  void update_default_ims_request_get_current_calls(void);
  ims_CallList_Call** update_default_ims_request_get_current_calls_resp(int num_calls);
#ifdef RIL_UTF_L_MASTER
  ims_CallList_Call** update_default_ims_unsol_response_call_state_changed_L(int num_calls);
#endif
  /*
    ims_MsgId_REQUEST_HANGUP_WAITING_OR_BACKGROUND
  */
  void update_default_ims_request_hangup_waiting_or_background(void);
  void update_default_ims_request_hangup_waiting_or_background_resp(void);

  /*
    ims_MsgId_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
  */
  void update_default_ims_request_hangup_foreground_resume_background(void);
  void update_default_ims_request_hangup_foreground_resume_background_resp(void);

  /*
    ims_MsgId_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE
  */
  void update_default_ims_request_switch_waiting_or_holding_and_active(void);
  void set_ims_request_switch_waiting_or_holding_and_active_call_type(ims_CallType call_type);
  void update_default_ims_request_switch_waiting_or_holding_and_active_resp(void);

  /*
    ims_MsgId_REQUEST_CONFERENCE
  */
  void update_default_ims_request_conference(void);
  ims_SipErrorInfo *update_default_ims_request_conference_resp(void);
  void update_sip_error_info(uint32_t code, char* error_string);

  /*
    ims_MsgId_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE
  */
  void update_default_ims_request_exit_emergency_callback_mode(void);
  void update_default_ims_request_exit_emergency_callback_mode_resp(void);

  /*
    ims_MsgId_REQUEST_EXPLICIT_CALL_TRANSFER
  */
  ims_ExplicitCallTransfer *update_default_ims_request_explicit_call_transfer(void);
  void update_default_ims_request_explicit_call_transfer_resp(void);

  /*
    ims_MsgId_REQUEST_DEFLECT_CALL
  */
  ims_DeflectCall *update_default_ims_request_deflect_call(void);
  void update_default_ims_request_deflect_call_resp(void);

  /*
    ims_MsgId_REQUEST_DTMF
  */
  void update_default_ims_request_dtmf(char *dtmf);
  void update_default_ims_request_dtmf_resp(void);

  /*
    ims_MsgId_REQUEST_DTMF_START
  */
  void update_default_ims_request_dtmf_start(char *dtmf);
  void update_default_ims_request_dtmf_start_resp(void);

  /*
    ims_MsgId_REQUEST_DTMF_STOP
  */
  void update_default_ims_request_dtmf_stop(void);
  void update_default_ims_request_dtmf_stop_resp(void);

  /*
    ims_MsgId_REQUEST_MODIFY_CALL_INITIATE
  */
  void update_default_ims_request_modify_call_initiate(void);
  void set_ims_request_modify_call_initiate_call_index(uint32_t callindex);
  void set_ims_request_modify_call_initiate_error(ims_Error error);
  void set_ims_request_modify_call_initiate_call_details(ims_CallType calltype, ims_CallDomain calldomain);
  void set_ims_request_modify_call_initiate_rtt_mode(ims_RttMode rttMode);
  void update_default_ims_request_modify_call_initiate_resp(void);

  /*
    ims_MsgId_REQUEST_MODIFY_CALL_CONFIRM
  */
  void update_default_ims_request_modify_call_confirm(void);
  void set_ims_request_modify_call_confirm_call_index(uint32_t callindex);
  void set_ims_request_modify_call_confirm_error(ims_Error error);
  void set_ims_request_modify_call_confirm_call_details(ims_CallType calltype, ims_CallDomain calldomain);
  void update_default_ims_request_modify_call_confirm_resp(void);

  /*
    ims_MsgId_REQUEST_CANCEL_MODIFY_CALL
  */
  void update_default_ims_request_cancel_modify_call(void);
  void set_ims_request_cancel_modify_call_call_id(uint32_t callid);
  void update_default_ims_request_cancel_modify_call_resp(void);

  /*
    ims_MsgId_REQUEST_QUERY_CLIP
  */
  void update_default_ims_request_query_clip(void);
  void update_default_ims_request_query_clip_resp(ims_ClipStatus clip_status);
  ims_ClipProvisionStatus *update_default_ims_request_query_clip_resp_no_status(void);

  /*
    ims_MsgId_REQUEST_GET_CLIR
  */
  void update_default_ims_request_get_clir(void);
  void update_default_ims_request_get_clir_resp(void);
  void set_ims_request_get_clir_resp_param_m(uint32_t param_m);
  void set_ims_request_get_clir_resp_param_n(uint32_t param_n);

  /*
    ims_MsgId_REQUEST_SET_CLIR
  */
  void update_default_ims_request_set_clir(void);
  void set_ims_request_set_clir_param_m(uint32_t param_m);
  void set_ims_request_set_clir_param_n(uint32_t param_n);
  void update_default_ims_request_set_clir_resp(void);

  /*
    ims_MsgId_REQUEST_QUERY_CALL_FORWARD_STATUS
  */
  ims_CallForwardInfoList_CallForwardInfo**
  update_default_ims_request_query_call_forward_status(int num_entries);
  ims_CallForwardInfoList_CallForwardInfo**
  update_default_ims_request_query_call_forward_status_resp(int num_entries);
  ims_SipErrorInfo *set_ims_request_query_call_forward_status_resp_error_details();
  void set_ims_request_query_call_forward_status_callfwdtimerstart(
        uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute,
        uint32_t second, uint32_t timezone, uint32_t index = 0);
  void set_ims_request_query_call_forward_status_callfwdtimerend(
        uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute,
        uint32_t second, uint32_t timezone, uint32_t index = 0);

  /*
    ims_MsgId_REQUEST_SET_CALL_FORWARD_STATUS
  */
  ims_CallForwardInfoList_CallForwardInfo**
  update_default_ims_request_set_call_forward_status(int num_entries);
  void set_ims_request_set_call_forward_status_callfwdtimerstart(
        uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute,
        uint32_t second, uint32_t timezone);
  void set_ims_request_set_call_forward_status_callfwdtimerend(
        uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute,
        uint32_t second, uint32_t timezone);
  ims_CallForwardStatusInfo *update_default_ims_request_set_call_forward_status_resp(void);

  /*
    ims_MsgId_REQUEST_QUERY_CALL_WAITING
  */
  void update_default_ims_request_query_call_waiting(void);
  void set_ims_request_query_call_waiting_service_class(uint32_t service_class);
  void update_default_ims_request_query_call_waiting_resp(void);
  void set_ims_request_query_call_waiting_resp_service_status(ims_ServiceClassStatus service_status);
  void set_ims_request_query_call_waiting_resp_service_class(uint32_t service_class);
  ims_SipErrorInfo *set_ims_request_query_call_waiting_resp_error_details();

  /*
    ims_MsgId_REQUEST_SET_CALL_WAITING
  */
  ims_CallWaitingInfo *update_default_ims_request_set_call_waiting(void);
  void set_ims_request_set_call_waiting_service_status(ims_ServiceClassStatus service_status);
  ims_SuppSvcResponse *update_default_ims_request_set_call_waiting_resp(void);

  /*
    ims_MsgId_REQUEST_IMS_REG_STATE_CHANGE
  */
  void update_default_ims_request_ims_reg_state_change(void);
  void set_ims_request_ims_reg_state_change_state(ims_Registration_RegState state);
  void update_default_ims_request_ims_reg_state_change_resp(void);

  /*
    ims_MsgId_REQUEST_SET_SUPP_SVC_NOTIFICATION
  */
  void update_default_ims_request_set_supp_svc_notification(void);
  void set_ims_request_set_supp_svc_notification_status(ims_ServiceClassStatus status);
  void update_default_ims_request_set_supp_svc_notification_resp(void);
  void set_ims_request_set_supp_svc_notification_resp_status(ims_ServiceClassStatus status);
  void set_sups_sip_error_info(uint32_t code, char* message);

  /*
    ims_MsgId_REQUEST_SET_SERVICE_STATUS
  */
  void update_default_ims_request_set_service_status (void);
  void set_ims_request_set_service_status_volte_enabled(bool val);
  void set_ims_request_set_service_status_video_telephony_enabled(bool val);
  void set_ims_request_set_service_status_wifi_calling_enabled(bool val);
  void set_ims_request_set_service_status_ut_enabled(bool val);
  void update_default_ims_request_set_service_status_resp (void);

  /*
    ims_MsgId_REQUEST_SET_TTY_MODE
  */
  void update_default_ims_request_set_tty_mode (ims_Tty_Mode_Type mode);
  void update_default_ims_request_set_tty_mode_resp(void);

  /*
    ims_MsgId_UNSOL_RSP_BASE
  */
  void update_default_ims_unsol_rsp_base(void);

  /*
    ims_MsgId_UNSOL_CALL_RING
  */
  void update_default_ims_unsol_call_ring(void);

  /*
    ims_MsgId_UNSOL_RINGBACK_TONE
  */
  void update_default_ims_unsol_ringback_tone(void);
  void set_ims_unsol_ringback_tone_flag(ims_RingBackTone_ToneFlag flag);

  /*
    ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  */
  void update_default_ims_unsol_response_call_state_changed(void);

  /*
    ims_MsgId_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED
  */
  void update_default_ims_unsol_response_ims_network_state_changed(void);
  void set_ims_unsol_response_ims_network_state_changed_reg_state(int has_state, ims_Registration_RegState state);
  void set_ims_unsol_response_ims_network_state_changed_reg_radioTech(ims_RadioTechType  val);
  void set_ims_unsol_response_ims_network_state_changed_reg_pAssociatedUris(const char *val);
  void set_ims_unsol_response_ims_network_state_changed_errorcode(int has_errorcode, uint32_t errorcode, char *errormessage);

  /*
    ims_MsgId_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE
  */
  void update_default_ims_unsol_enter_emergency_callback_mode(void);

  /*
    ims_MsgId_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE
  */
  void update_default_ims_unsol_exit_emergency_callback_mode(void);

  /*
    ims_MsgId_UNSOL_MODIFY_CALL
  */
  void update_default_ims_unsol_modify_call(void);
  void set_ims_unsol_modify_call_index(uint32_t callindex);
  void set_ims_unsol_modify_call_error(ims_Error error);
  void set_ims_unsol_modify_call_call_details(ims_CallType calltype, ims_CallDomain calldomain);

  /*
    ims_MsgId_UNSOL_RESPONSE_HANDOVER
  */
  void update_default_ims_unsol_response_handover(void);
  void set_ims_unsol_response_handover_type(ims_Handover_Msg_Type type);
  void set_ims_unsol_response_srctech(ims_RadioTechType srctech);
  void set_ims_unsol_response_targettech(ims_RadioTechType targettech);

  /*
    ims_MsgId_UNSOL_TTY_NOTIFICATION
  */
  void update_default_ims_unsol_response_tty(ims_Tty_Mode_Type tty_mode_type);

  /*
    ims_MsgId_UNSOL_REFRESH_CONF_INFO
  */
  void update_default_ims_unsol_refresh_conf_info(void);
  void set_ims_unsol_refresh_conf_info_conf_info_uri(pb_callback_t conf_info_uri);
  void update_ims_unsol_refresh_conf_info_conf_info_uri_data(size_t len, uint8_t* data);
  void set_ims_unsol_refresh_conf_info_callstate(ims_ConfCallState conf_call_state);
  /*
    ims_MsgId_UNSOL_REFRESH_VICE_INFO
  */
  void update_ims_unsol_refresh_vice_info_conf_info_uri_data(size_t len, uint8_t* data);
  /*
    ims_MsgId_UNSOL_AUTO_CALL_REJECTION_IND
  */
  void update_ims_unsol_auto_call_rejection_ind(ims_CallType callType, ims_CallFailCause autoRejectionCause, uint16_t sipErrorCode);
  void set_ims_unsol_auto_call_rejection_ind_number(char *number);

  /*
    ims_MsgId_UNSOL_SRV_STATUS_UPDATE
  */
  void update_default_ims_unsol_srv_status_update(int num_entries);
  void set_ims_unsol_srv_status_update_sms_service(int array_idx, uint8_t sms_val, boolean rat_valid, uint8_t rat_val);
  void set_ims_unsol_srv_status_update_voip_service(int array_idx, uint8_t voip_val, boolean rat_valid, uint8_t rat_val);
  void set_ims_unsol_srv_status_update_vt_service(int array_idx, uint8_t vt_val, boolean rat_valid, uint8_t rat_val);
#ifdef RIL_UTF_L_MASTER
  void set_ims_unsol_srv_status_update_ut_service(int array_idx, uint8_t ut_val, boolean rat_valid, uint8_t rat_val);
#endif

  /*
    ims_MsgId_UNSOL_SUPP_SVC_NOTIFICATION
  */
  void update_default_ims_unsol_supp_svc_notification(void);
  void set_ims_unsol_supp_svc_notification_number(char *number);
  void set_ims_unsol_supp_svc_notification_code(uint32_t code);
  void set_ims_unsol_supp_svc_notification_index(uint32_t index);
  void set_ims_unsol_supp_svc_notification_type(uint32_t type);
  void set_ims_unsol_supp_svc_notification_connid(uint32_t connid);
  void set_ims_unsol_supp_svc_notification_notificationtype(ims_NotificationType notificationtype);
  void set_ims_unsol_supp_svc_notification_history_info(char *history_info);
  void set_ims_unsol_supp_svc_notification_hold_tone(bool hold_tone);

  /*
    ims_MsgId_UNSOL_VOWIFI_CALL_QUALITY
  */
  void update_default_ims_unsol_vowifi_call_quality(ims_voWiFiQuality quality);

  /*
    ims_MsgId_REQUEST_QUERY_SERVICE_STATUS
  */
  void update_default_ims_request_query_service_status(void);
  void update_default_ims_request_query_service_status_resp(int num_entries);
  void set_ims_request_query_service_status_setup_resp_sms_service(int array_idx, uint8_t sms_val, boolean rat_valid, uint8_t rat_val);
  void set_ims_request_query_service_status_setup_resp_voip_service(int array_idx, uint8_t voip_val, boolean rat_valid, uint8_t rat_val);
  void set_ims_request_query_service_status_setup_resp_vt_service(int array_idx, uint8_t vt_val, boolean rat_valid, uint8_t rat_val);
#ifdef RIL_UTF_L_MASTER
  void set_ims_request_query_service_status_setup_resp_ut_service(int array_idx, uint8_t ut_val, boolean rat_valid, uint8_t rat_val);
#endif

  /*
    ims_MsgId_REQUEST_SUPP_SVC_STATUS
  */
void update_default_ims_request_supp_svc_status( void );
void set_ims_request_supp_svc_status(ims_SuppSvcOperationType operationType, ims_SuppSvcFacilityType facilityType);
void set_ims_request_supp_svc_status_service_class(uint32_t service_Class);
ims_CbNumList **set_ims_request_supp_svc_status_cb_num_list(uint32_t num_list_len);
void set_ims_request_supp_svc_status_password(char *passwd);
ims_SuppSvcResponse *update_default_ims_request_supp_svc_status_resp( void );
void set_ims_request_set_supp_svc_response_resp_status(ims_ServiceClassStatus status, uint32 facility);
void set_ims_request_set_supp_svc_response_resp_provisionStatus(ims_ServiceClassProvisionStatus status);

#ifndef UTF_TARGET_BUILD
  /*
    ims_MsgId_REQUEST_GET_COLR
  */
  void update_default_ims_request_get_colr( void );
  void update_default_ims_request_get_colr_resp( void );
  void set_ims_request_get_colr_resp_colr(ims_IpPresentation colr);

  /*
    ims_MsgId_REQUEST_SET_COLR
  */
  void update_default_ims_request_set_colr( void );
  void set_ims_request_set_colr_req_colr(ims_IpPresentation colr);
  void update_default_ims_request_set_colr_resp( void );
#endif


  /*
    ims_MsgId_UNSOL_RADIO_STATE_CHANGED
  */
  void update_default_ims_unsol_radio_state_changed(ims_RadioState state);

  /*
    ims_MsgId_UNSOL_WFC_ROAMING_MODE_CONFIG_SUPPORT
  */
  void update_default_ims_unsol_wfc_roaming_config_support(boolean wfcRoamingConfigSupport);

  /*
    ims_MsgId_UNSOL_USSD_FAILED
  */
  void update_default_ims_unsol_ussd_failed(ims_UssdModeType type, int has_errorcode, uint32_t code, char* message);

#ifndef UTF_TARGET_BUILD
  /*
    ims_MsgId_REQUEST_QUERY_VT_CALL_QUALITY
  */
  void update_default_ims_request_query_vt_call_quality(void);
  void update_default_ims_request_query_vt_call_quality_resp(void);
  void set_ims_request_query_vt_call_quality_resp_video_quality(ims_Quality quality);

  /*
    ims_MsgId_REQUEST_SET_VT_CALL_QUALITY
  */
  void update_default_ims_request_set_vt_call_quality(void);
  void set_ims_request_set_vt_call_quality_resp_video_quality(ims_Quality quality);
  void update_default_ims_request_set_vt_call_quality_resp(void);

  /*
    ims_MsgId_UNSOL_MWI
  */
  void update_default_ims_unsol_mwi(int n_mwimsgsummary, int n_mwimsgdetail);
  void set_ims_unsol_mwi_message_summary(
    int array_idx,
    ims_MwiMessageType messagetype,
    uint32_t newmessage,
    uint32_t oldmessage,
    uint32_t newurgent,
    uint32_t oldurgent);
  void set_ims_unsol_mwi_ueaddress(char *ueaddress);
  void set_ims_unsol_mwi_message_details(
    int array_idx,
    char *toaddress,
    char *fromaddress,
    char *subject,
    char *date,
    ims_MwiPriority priority,
    char *messageid,
    ims_MwiMessageType messagetype);
#endif


#ifdef RIL_UTF_L_MASTER
  /*
    ims_MsgId_REQUEST_HOLD
  */
  ims_Hold* update_default_ims_request_hold(int callId);
  ims_SipErrorInfo *update_default_ims_request_hold_resp();

  /*
    ims_MsgId_REQUEST_RESUME
  */
  ims_Resume* update_default_ims_request_resume(int callId);
  void update_default_ims_request_resume_resp();

  /*
    ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS
  */
  void update_default_ims_request_get_wifi_calling_status(void);
  ims_WifiCallingInfo *update_default_ims_request_get_wifi_calling_status_resp(void);
  void set_ims_request_get_wifi_calling_status_resp_wificallinginfo(
          ims_WifiCallingStatus status,
          ims_WifiCallingPreference preference);

  /*
    ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS
  */
  ims_WifiCallingInfo *update_default_ims_request_set_wifi_calling_status(void);
  void set_ims_request_set_wifi_calling_status_wificallinginfo(
          ims_WifiCallingStatus status,
          ims_WifiCallingPreference preference);
  void update_default_ims_request_set_wifi_calling_status_resp(void);

  /*
    ims_MsgId_REQUEST_SET_IMS_CONFIG
  */

  //Need to use set_ims_request_set_config to
  //set the config item for request(mandatory)
  ims_ConfigMsg *update_default_ims_request_set_config(ims_ConfigMsg *req=nullptr);
  ims_ConfigMsg *update_default_ims_request_set_config_resp();
  ims_ConfigMsg *set_ims_request_set_config();

  /*
    ims_MsgId_REQUEST_GET_IMS_CONFIG
  */

  //Need to use set_ims_request_set_config to
  //set the config item for request(mandatory)
  ims_ConfigMsg *update_default_ims_request_get_config();
  ims_ConfigMsg *update_default_ims_request_get_config_resp();
  ims_ConfigMsg *set_ims_request_get_config();

  void update_default_ims_unsol_request_geolocation();
  void update_default_ims_request_send_geolocation_info();
  void set_ims_geolocation_info_lat(double lat);
  void set_ims_geolocation_info_lon(double lon);

  void set_ims_geolocation_info_countryCode(const char *contryCode);
  void set_ims_geolocation_info_country(const char *country);
  void set_ims_geolocation_info_state(const char *state);
  void set_ims_geolocation_info_city(const char *city);
  void set_ims_geolocation_info_postalCode(const char *postalCode);
  void set_ims_geolocation_info_street(const char *street);
  void set_ims_geolocation_info_houseNumber(const char *houseNumber);
  void update_default_ims_request_send_geolocation_info_resp();

  /*
    ims_MsgId_REQUEST_GET_IMS_SUB_CONFIG
   */
  void update_default_ims_request_get_ims_sub_config();
  ims_ImsSubConfig *update_default_ims_request_get_ims_sub_config_resp();
  void set_ims_request_get_ims_sub_config_resp_simultStackCount(uint32_t val);
  void set_ims_request_get_ims_sub_config_resp_imsStackEnabled(uint64_t *val, size_t len);

  /*
    ims_MsgId_UNSOL_IMS_SUB_CONFIG_CHANGED
  */
  ims_ImsSubConfig *update_default_ims_unsol_ims_sub_config_changed();
  void set_ims_unsol_ims_sub_config_changed_simultStackCount(uint32_t val);
  void set_ims_unsol_ims_sub_config_changed_imsStackEnabled(uint64_t *val, size_t len);

  /*
    ims_MsgId_REQUEST_REGISTER_MULTI_IDENTITY_LINES
  */
  ims_MultiIdentityLineInfo *update_default_ims_request_register_multi_identity_lines(uint32_t lineInfo_len);
  void update_default_ims_request_register_multi_identity_lines_resp();

  /*
    ims_MsgId_REQUEST_QUERY_VIRTUAL_LINE_INFO
  */
  void update_default_ims_request_query_virtual_line_info(const char *msisdn);
  void update_default_ims_request_query_virtual_line_info_resp(const char *msisdn, uint32_t lines_len);
  void set_ims_request_query_virtual_line_info_resp_lines(uint32_t i, char *line);

  /*
    ims_MsgId_UNSOL_MULTI_IDENTITY_PENDING_INFO
  */
  void update_default_ims_unsol_multi_identity_pending_info(void);

  /*
    ims_MsgId_UNSOL_VOICE_INFO
  */
  void update_default_ims_unsol_voice_info(ims_VoiceInfo voiceInfo);
  /*
    ims_MsgId_UNSOL_MULTI_IDENTITY_REGISTRATION_STATUS
  */
  ims_MultiIdentityLineInfo *update_default_ims_unsol_multi_identity_registration_status(uint32_t lineInfo_len);

  /*
    ims_MsgId_REQUEST_SEND_RTT_MSG
  */
  void update_default_ims_request_send_rtt_message();
  void set_ims_rtt_message_text(const char *msg);
  void update_default_ims_request_send_rtt_message_resp (void);
  void update_default_ims_unsol_rtt_message();

  /*
    ims_MsgId_REQUEST_GET_VOWIFI_CALL_QUALITY
  */
  void update_default_ims_request_get_vowifi_call_quality();

  ims_VoWiFiCallQuality *update_default_ims_request_get_vowifi_call_quality_resp();
  void set_ims_request_get_vowifi_call_quality_resp_quality(ims_voWiFiQuality quality);
  void update_default_ims_unsol_vops_changed(bool);

  /*
    ims_MsgId_REQUEST_SEND_SMS
  */
  RIL_IMS_SMS_Message* update_default_ims_request_send_sms();
  void set_ims_sms_gsm_payload(const char* smsc, const char* content);
  void update_default_ims_send_sms_response();

  /*
    ims_MsgId_UNSOL_INCOMING_IMS_SMS
  */
  utf_ims_mt_sms_t* update_default_ims_mt_sms();
  void set_ims_mt_sms_payload(const std::initializer_list<uint8_t> &list);
  utf_ims_ack_sms_t* update_default_ims_ack_sms();

  /*
    ims_MsgId_UNSOL_PARTICIPANT_STATUS_INFO
  */
  ims_ParticipantStatusInfo* update_default_ims_unsol_participant_status_info();
  void set_ims_unsol_participant_status_info_call_id(uint32_t callId);
  void set_ims_unsol_participant_status_info_operation(ims_ConfParticipantOperation operation);
  void set_ims_unsol_participant_status_info_sip_status(uint32_t sipStatus);
  void set_ims_unsol_participant_status_info_participant_uri(char *participantUri);
  void set_ims_unsol_participant_status_info_is_ect(bool isEct);

  /*
     ims_MsgId_UNSOL_ON_SS
  */
  ims_ril_ss_ind_type* update_default_ims_unsol_on_ss();

  /*
     ims_MsgId_UNSOL_RESPONSE_REGISTRATION_BLOCK_STATUS
  */
  ims_RegistrationBlockStatus* update_default_ims_unsol_registration_block_status();
  ims_BlockStatus *set_ims_unsol_registration_block_status_BlockStatusOnWwan(
      ims_BlockReasonType block, ims_RegFailureReasonType regFailure, uint32_t regFailureCode);
  ims_BlockStatus *set_ims_unsol_registration_block_status_BlockStatusOnWlan(
      ims_BlockReasonType block, ims_RegFailureReasonType regFailure, uint32_t regFailureCode);

#endif

  ims_Info *create_ims_Info(
      bool has_isValid,
      bool isValid,
      bool has_type,
      ims_SrvType type,
      bool has_callType,
      ims_CallType callType,
      bool has_status,
      ims_StatusType status,
      void *userdata,
      bool has_restrictCause,
      uint32_t restrictCause,
      ims_StatusForAccessTech **accTechStatus);
  ims_StatusForAccessTech *create_ims_StatusForAccessTech(
      bool has_networkMode,
      ims_RadioTechType networkMode,
      bool has_status,
      ims_StatusType status,
      bool has_restrictionCause,
      uint32_t restrictionCause,
      bool has_registered,
      ims_Registration registered);

} IMSTestnode;

#endif /* RIL_UTF_IMS_IF_H_*/
