/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef RIL_UTF_ANDROIDTEL_IF_H_
#define RIL_UTF_ANDROIDTEL_IF_H_

#include "ril_utf_if.h"

//RIL interfaces extracted from ril_utf_if.h
typedef class ril_utf_androidTel_if : public ril_utf_node_base_t {
public:
~ril_utf_androidTel_if() {
}

//--------------------------------------------------------------------------------
//==================================================
// RIL Interfaces
//==================================================
void update_default_ril_unsol_resp_radio_state_changed(void);
void set_ril_unsol_resp_radio_state_changed_state(RIL_RadioState state);


/*
  RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
*/
void update_default_ril_unsol_voice_radio_tech_changed(void);
void set_ril_unsol_voice_radio_tech_changed_radio_tech(RIL_RadioTechnology radio_tech);

/*
  RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
*/
void update_default_ril_unsol_response_call_state_changed(void);

/*
  RIL_UNSOL_RINGBACK_TONE
*/
void update_default_ril_unsol_ringback_tone(bool start);

/*
  RIL_UNSOL_SIGNAL_STRENGTH
*/
void update_default_ril_unsol_signal_strength(void);
void set_ril_unsol_signal_strength_gw_signal_strength(int signal_strength, int bit_error_rate);
void set_ril_unsol_signal_strength_cdma_signal_strength(int dbm, int ecio);
void set_ril_unsol_signal_strength_evdo_signal_strength(int dbm, int ecio, int snr);
void set_ril_unsol_signal_strength_lte_signal_strength
            (int signal_strength, int rsrp, int rsrq, int rssnr, int cqi, int timing_advance);
void set_ril_unsol_signal_strength_td_scdma_signal_strength(int recp);

/*
  RIL_UNSOL_STK_CC_ALPHA_NOTIFY
*/
uint8_t* update_default_ril_unsol_stk_cc_alpha_notify(void);
uint8_t* malloc_ril_request_get_alpha_text_resp();
void free_ril_request_get_alpha_text_resp();

/*
  RIL_UNSOL_ON_USSD
*/
void update_default_ril_unsol_on_ussd(char a, const char *str, int str_len);

/*
  RIL_UNSOL_ON_SS
*/
RIL_StkCcUnsolSsResponse* update_default_ril_unsol_on_ss(void);
RIL_StkCcUnsolSsResponse* malloc_ril_request_unsol_ss_resp();

/*
  RIL_UNSOL_SUPP_SVC_NOTIFICATION
*/
RIL_SuppSvcNotification *update_default_ril_unsol_supp_svc_notification();

/*
  RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
*/
void update_default_ril_unsol_response_voice_network_state_changed(void);

/*
  RIL_REQUEST_VOICE_REGISTRATION_STATE
*/
void update_default_ril_request_voice_registration_state(void);
char **update_default_ril_request_voice_registration_state_resp(void);

/*
  RIL_UNSOL_RESPONSE_NEW_SMS
*/
void update_default_ril_unsol_response_new_sms(char *ascii_hex_string);

/*
  RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT
*/
void update_default_ril_unsol_response_new_sms_status_report(char *ascii_hex_string);

/*
  RIL_UNSOL_SIM_SMS_STORAGE_FULL
*/
void update_default_ril_unsol_sim_sms_storage_full(void);

/*
  RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL
*/
void update_default_ril_unsol_ruim_sms_storage_full(void);

/*
  RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS
*/
void update_default_ril_unsol_response_new_broadcast_sms(unsigned char ascii_hex_string[],
                                                         size_t len);

/*
  RIL_UNSOL_CALL_RING
*/
void update_default_ril_unsol_call_ring(void);

/*
  RIL_UNSOL_OEM_HOOK_RAW
*/
void update_default_ril_unsol_oem_hook_raw(void);

/*
  RIL_REQUEST_RADIO_POWER
*/
void update_default_ril_request_radio_power(void);
void update_default_ril_request_radio_power_resp(void);


/*
  RIL_REQUEST_SEND_SMS
*/
char** update_default_ril_request_send_sms(void);
RIL_SMS_Response* update_default_ril_request_send_sms_resp(void);
char** malloc_ril_request_send_sms(void);
void free_ril_request_send_sms();
RIL_SMS_Response* malloc_ril_request_send_sms_resp();
void free_ril_request_send_sms_resp();
int compare_ril_request_send_sms(ril_request_send_sms_t *ptr1,ril_request_send_sms_t *ptr2);
int compare_ril_request_send_sms_resp(RIL_SMS_Response *ptr1, RIL_SMS_Response *ptr2);


/*
  RIL_REQUEST_SEND_SMS_EXPECT_MORE
*/
ril_request_send_sms_expect_more_t* update_default_ril_request_send_sms_expect_more(void);
RIL_SMS_Response* update_default_ril_request_send_sms_expect_more_resp(void);
ril_request_send_sms_expect_more_t* malloc_ril_request_send_sms_expect_more(void);
void free_ril_request_send_sms_expect_more();
RIL_SMS_Response* malloc_ril_request_send_sms_expect_more_resp(void);
void free_ril_request_send_sms_expect_more_resp();
int compare_ril_request_send_sms_expect_more(ril_request_send_sms_expect_more_t *ptr1,ril_request_send_sms_expect_more_t *ptr2);
int compare_ril_request_send_sms_expect_more_resp(RIL_SMS_Response *ptr1, RIL_SMS_Response *ptr2);

/*
  RIL_REQUEST_SMS_ACKNOWLEDGE
*/

int* update_default_ril_request_sms_acknowledge(void);
void update_default_ril_request_sms_acknowledge_resp();
ril_request_sms_acknowledge_t* malloc_ril_request_sms_acknowledge();
void free_ril_request_sms_acknowledge();
int compare_ril_request_sms_acknowledge(ril_request_sms_acknowledge_t *ptr1, ril_request_sms_acknowledge_t* ptr2);

/*
  RIL_REQUEST_WRITE_SMS_TO_SIM
*/
RIL_SMS_WriteArgs* update_default_ril_request_write_sms_to_sim(void);
ril_request_write_sms_to_sim_resp_t* update_default_ril_request_write_sms_to_sim_resp();
RIL_SMS_WriteArgs* malloc_ril_request_write_sms_to_sim();
void free_ril_request_write_sms_to_sim();
ril_request_write_sms_to_sim_resp_t* malloc_ril_request_write_sms_to_sim_resp();
void free_ril_request_write_sms_to_sim_resp();
int compare_ril_request_write_sms_to_sim(RIL_SMS_WriteArgs *ptr1, RIL_SMS_WriteArgs *ptr2);
int compare_ril_request_write_sms_to_sim_resp(ril_request_write_sms_to_sim_resp_t *ptr1, ril_request_write_sms_to_sim_resp_t *ptr2);

/*
    RIL_REQUEST_DELETE_SMS_ON_SIM
*/
ril_request_delete_sms_on_sim_t* update_default_ril_request_delete_sms_on_sim(int index);
void update_default_ril_request_delete_sms_on_sim_resp(RIL_Errno err);
void free_ril_request_delete_sms_on_sim();

/*
    RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM
*/
ril_request_delete_sms_on_sim_t* update_default_ril_request_delete_cdma_sms_on_ruim(int index);
void update_default_ril_request_delete_cdma_sms_on_ruim_resp(RIL_Errno err);
void free_ril_request_delete_cdma_sms_on_ruim();
/*
    RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
*/
void update_default_ril_unsol_response_sim_status_changed();
/*
  RIL_REQUEST_GET_SIM_STATUS
*/
void update_default_ril_request_get_sim_status();
RIL_CardStatus_v6* update_default_ril_request_get_sim_status_resp();

/*
   RIL_REQUEST_ENTER_SIM_PIN
*/
char **update_default_ril_request_enter_sim_pin();
void free_ril_request_enter_sim_pin();
int *update_default_ril_request_enter_sim_pin_resp();

/*
   RIL_REQUEST_ENTER_SIM_PUK
*/
char **update_default_ril_request_enter_sim_puk();
void free_ril_request_enter_sim_puk();
int *update_default_ril_request_enter_sim_puk_resp();

/*
   QCRIL_EVT_HOOK_ENTER_DEPERSONALIZATION_REQ
*/
char **update_default_ril_request_enter_depersonalization_code();
void free_ril_request_enter_depersonalization_code();
int *update_default_ril_request_enter_depersonalization_code_resp();

/*
    RIL_REQUEST_GET_SMSC_ADDRESS
*/
void update_default_ril_request_get_smsc_address();
ril_request_get_smsc_address_resp_t* update_default_ril_request_get_smsc_address_resp(char *smsc);
ril_request_get_smsc_address_resp_t* malloc_ril_request_get_smsc_address_resp();
void free_ril_request_get_smsc_address_resp();
int compare_ril_request_get_smsc_address_resp(ril_request_get_smsc_address_resp_t *ptr1, ril_request_get_smsc_address_resp_t *ptr2);
/*
    RIL_REQUEST_SET_SMSC_ADDRESS
*/
void update_default_ril_request_set_smsc_address(char *val);
void update_default_ril_request_set_smsc_address_resp();

/*
    RIL_REQUEST_REPORT_SMS_MEMORY_STATUS
*/
ril_request_report_sms_memory_status_t* update_default_ril_request_report_sms_memory_status();
void update_default_ril_request_report_sms_memory_status_resp();
ril_request_report_sms_memory_status_t* malloc_ril_request_report_sms_memory_status();
void free_ril_request_report_sms_memory_status();
int compare_ril_request_report_sms_memory_status(ril_request_report_sms_memory_status_t *ptr1, ril_request_report_sms_memory_status_t *ptr2);

/*
    RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG
*/
void update_default_ril_request_gsm_get_broadcast_sms_config();
RIL_GSM_BroadcastSmsConfigInfo** update_default_ril_request_gsm_get_broadcast_sms_config_resp(int num);
RIL_GSM_BroadcastSmsConfigInfo** malloc_ril_request_gsm_get_broadcast_sms_config_resp(int num);
void free_ril_request_gsm_get_broadcast_sms_config_resp();
int compare_ril_request_gsm_get_broadcast_sms_config_resp(RIL_GSM_BroadcastSmsConfigInfo **ptr1, RIL_GSM_BroadcastSmsConfigInfo **ptr2 );

/*
    RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG
*/
RIL_GSM_BroadcastSmsConfigInfo** update_default_ril_request_gsm_set_broadcast_sms_config(int num);
void update_default_ril_request_gsm_set_broadcast_sms_config_resp();
RIL_GSM_BroadcastSmsConfigInfo** malloc_ril_request_gsm_set_broadcast_sms_config(int num);
void free_ril_request_gsm_set_broadcast_sms_config();
int compare_ril_request_gsm_set_broadcast_sms_config(RIL_GSM_BroadcastSmsConfigInfo **ptr1, RIL_GSM_BroadcastSmsConfigInfo **ptr2);
/*
    RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION
*/
ril_request_gsm_sms_broadcast_activation_t* update_default_ril_request_gsm_sms_broadcast_activation();
void update_default_ril_request_gsm_sms_broadcast_activation_resp();
ril_request_gsm_sms_broadcast_activation_t* malloc_ril_request_gsm_sms_broadcast_activation();
void free_ril_request_gsm_sms_broadcast_activation();
int compare_ril_request_gsm_sms_broadcast_activation(ril_request_gsm_sms_broadcast_activation_t *ptr1, ril_request_gsm_sms_broadcast_activation_t *ptr2);
/*
    RIL_REQUEST_CDMA_SEND_SMS
*/
RIL_CDMA_SMS_Message* update_default_ril_request_cdma_send_sms();
RIL_SMS_Response* update_default_ril_request_cdma_send_sms_resp();
RIL_CDMA_SMS_Message* malloc_ril_request_cdma_send_sms();
void free_ril_request_cdma_send_sms();
RIL_SMS_Response* malloc_ril_request_cdma_send_sms_resp();
void free_ril_request_cdma_send_sms_resp();
int compare_ril_request_cdma_send_sms(RIL_CDMA_SMS_Message *ptr1, RIL_CDMA_SMS_Message *ptr2);
int compare_ril_request_cdma_send_sms_resp(RIL_SMS_Response *ptr1, RIL_SMS_Response *ptr2);
/*
    RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE
*/
RIL_CDMA_SMS_Ack* update_default_ril_request_cdma_sms_acknowledge();
void update_default_ril_request_cdma_sms_acknowledge_resp();
RIL_CDMA_SMS_Ack* malloc_ril_request_cdma_sms_acknowledge();
void free_ril_request_cdma_sms_acknowledge();
int compare_ril_request_cdma_sms_acknowledge(RIL_CDMA_SMS_Ack *ptr1, RIL_CDMA_SMS_Ack *ptr2);
/*
    RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM
*/
RIL_CDMA_SMS_WriteArgs* update_default_ril_request_cdma_write_sms_to_ruim();
void update_default_ril_request_cdma_write_sms_to_ruim_resp();
RIL_CDMA_SMS_WriteArgs* malloc_ril_request_cdma_write_sms_to_ruim();
void free_ril_request_cdma_write_sms_to_ruim();
int compare_ril_request_cdma_write_sms_to_ruim(RIL_CDMA_SMS_WriteArgs *ptr1, RIL_CDMA_SMS_WriteArgs *ptr2);

/*
    RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM
*/
ril_request_cdma_delete_sms_on_ruim_t* update_default_ril_request_cdma_delete_sms_on_ruim_t();
void update_default_ril_request_cdma_delete_sms_on_ruim_resp();
ril_request_cdma_delete_sms_on_ruim_t* malloc_ril_request_cdma_delete_sms_on_ruim();
void free_ril_request_cdma_delete_sms_on_ruim();
int compare_ril_request_cdma_delete_sms_on_ruim(ril_request_cdma_delete_sms_on_ruim_t *ptr1, ril_request_cdma_delete_sms_on_ruim_t *ptr2);
/*
    RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG
*/
void update_default_ril_request_cdma_get_broadcast_sms_config();
RIL_CDMA_BroadcastSmsConfigInfo** update_default_ril_request_cdma_get_broadcast_sms_config_resp(int num);
RIL_CDMA_BroadcastSmsConfigInfo** malloc_ril_request_cdma_get_broadcast_sms_config_resp(int num);
void free_ril_request_cdma_get_broadcast_sms_config_resp();
int compare_ril_request_cdma_get_broadcast_sms_config_resp(RIL_CDMA_BroadcastSmsConfigInfo **ptr1, RIL_CDMA_BroadcastSmsConfigInfo **ptr2);

/*
    RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG
*/
RIL_CDMA_BroadcastSmsConfigInfo** update_default_ril_request_cdma_set_broadcast_sms_config(int num);
void update_default_ril_request_cdma_set_broadcast_sms_config_resp();
RIL_CDMA_BroadcastSmsConfigInfo** malloc_ril_request_cdma_set_broadcast_sms_config(int num);
void free_ril_request_cdma_set_broadcast_sms_config();
int compare_ril_request_cdma_set_broadcast_sms_config(RIL_CDMA_BroadcastSmsConfigInfo **ptr1, RIL_CDMA_BroadcastSmsConfigInfo **ptr2);

/*
    RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION
*/

ril_request_cdma_sms_broadcast_activation_t* update_default_ril_request_cdma_sms_broadcast_activation();
void update_default_ril_request_cdma_sms_broadcast_activation_resp();
ril_request_cdma_sms_broadcast_activation_t* malloc_ril_request_cdma_sms_broadcast_activation();
void free_ril_request_cdma_sms_broadcast_activation();
int compare_ril_request_cdma_sms_broadcast_activation(ril_request_cdma_sms_broadcast_activation_t *ptr1, ril_request_cdma_sms_broadcast_activation_t *ptr2);

/*
    RIL_REQUEST_GET_CURRENT_CALLS
*/
void update_default_ril_request_get_current_calls();
RIL_Call** update_default_ril_request_get_current_calls_resp(int num_calls);
void set_ril_request_get_current_calls_resp_number(int call_slot, char *number);
void set_ril_request_get_current_calls_resp_name(int call_slot, char *name);
RIL_Call* malloc_ril_request_get_current_calls_resp();

/*
    RIL_REQUEST_DIAL
*/
RIL_Dial* update_default_ril_request_dial(char *address);
void update_default_ril_request_dial_resp();
RIL_Dial* malloc_ril_request_dial();
void free_ril_request_dial();
int compare_ril_requst_dial(RIL_Dial *ptr1, RIL_Dial *ptr2);

/*
    RIL_REQUEST_HANGUP
*/
ril_request_hangup_t* update_default_ril_request_hangup(int call_id);
void update_default_ril_request_hangup_resp();
ril_request_hangup_t* malloc_ril_request_hangup();
void free_ril_request_hangup();
int compare_ril_request_hangup(ril_request_hangup_t *ptr1, ril_request_hangup_t *ptr2);
/*
    RIL_REQUEST_LAST_CALL_FAIL_CAUSE
*/
void update_default_ril_request_last_call_fail_cause();
RIL_LastCallFailCauseInfo* update_default_ril_request_last_call_fail_cause_resp();
/*
ril_request_last_call_fail_cause_resp_t* malloc_ril_request_last_call_fail_cause_resp();
void free_ril_request_last_call_fail_cause_resp();
int compare_ril_request_last_call_fail_cause_resp(ril_request_last_call_fail_cause_resp_t *ptr1, ril_request_last_call_fail_cause_resp_t *ptr2);
*/

/*
    RIL_REQUEST_ANSWER
*/
void update_default_ril_request_answer();
void update_default_ril_request_answer_resp();

/*
    RIL_REQUEST_DTMF
*/
ril_request_dtmf_t* update_default_ril_request_dtmf();
void update_default_ril_request_dtmf_resp();
ril_request_dtmf_t* maloc_ril_request_dtmf();
void free_ril_request_dtmf();
int compare_ril_request_dtmf(ril_request_dtmf_t *ptr1, ril_request_dtmf_t *ptr2);

/*
    RIL_REQUEST_DTMF_START
*/
ril_request_dtmf_t* update_default_ril_request_dtmf_start();
void update_default_ril_request_dtmf_start_resp();
ril_request_dtmf_t* malloc_ril_request_dtmf_start();
void free_ril_request_dtmf_start();
int compare_ril_request_dtmf_start(ril_request_dtmf_t *ptr1, ril_request_dtmf_t *ptr2);

/*
    RIL_REQUEST_DTMF_STOP
*/
void update_default_ril_request_dtmf_stop();
void update_default_ril_request_dtmf_stop_resp();
/*
    RIL_REQUEST_CDMA_BURST_DTMF
*/
ril_request_cdma_burst_dtmf_t* update_default_ril_request_cdma_burst_dtmf();
void update_default_ril_request_cdma_burst_dtmf_resp();
ril_request_cdma_burst_dtmf_t* malloc_ril_request_cdma_burst_dtmf();
void free_ril_request_cdma_burst_dtmf();
int compare_ril_request_cdma_burst_dtmf(ril_request_cdma_burst_dtmf_t **ptr1, ril_request_cdma_burst_dtmf_t **ptr2);

/*
    RIL_REQUEST_CDMA_FLASH
*/
ril_request_cdma_flash_t* update_default_ril_request_cdma_flash();
void update_default_ril_request_cdma_flash_resp();
ril_request_cdma_flash_t* malloc_ril_request_cdma_flash();
void free_ril_request_cdma_flash();
int compare_ril_request_cdma_flash(ril_request_cdma_flash_t *ptr1, ril_request_cdma_flash_t *ptr2);

/*
    RIL_REQUEST_SET_TTY_MODE
*/
ril_request_set_tty_mode_t* update_default_ril_request_set_tty_mode();
void update_default_ril_request_set_tty_mode_resp();
ril_request_set_tty_mode_t* malloc_ril_request_set_tty_mode();
void free_ril_request_set_tty_mode();
int compare_ril_request_set_tty_mode(ril_request_set_tty_mode_t *ptr1, ril_request_set_tty_mode_t *ptr2);

/*
    RIL_REQUEST_QUERY_TTY_MODE
*/
void update_default_ril_request_get_tty_mode();
ril_request_get_tty_mode_t *update_default_ril_request_get_tty_mode_resp();
ril_request_get_tty_mode_t* malloc_ril_request_get_tty_mode();
void free_ril_request_get_tty_mode();
int compare_ril_request_get_tty_mode(ril_request_get_tty_mode_t *ptr1, ril_request_get_tty_mode_t *ptr2);

/*
    RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE
*/
ril_request_cdma_set_preferred_voice_privacy_mode_t* update_default_ril_request_cdma_set_preferred_voice_privacy_mode();
void update_default_ril_request_cdma_set_preferred_voice_privacy_mode_resp();
ril_request_cdma_set_preferred_voice_privacy_mode_t* malloc_ril_request_cdma_set_preferred_voice_privacy_mode();
void free_ril_request_cdma_set_preferred_voice_privacy_mode();
int compare_ril_request_cdma_set_preferred_voice_privacy_mode(ril_request_cdma_set_preferred_voice_privacy_mode_t *ptr1, ril_request_cdma_set_preferred_voice_privacy_mode_t *ptr2);
/*
    RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE
*/
void update_default_ril_request_cdma_query_preferred_voice_privacy_mode();
ril_request_cdma_query_preferred_voice_privacy_mode_t*
update_default_ril_request_cdma_query_preferred_voice_privacy_mode_resp();
ril_request_cdma_query_preferred_voice_privacy_mode_t*
malloc_ril_request_cdma_query_preferred_voice_privacy_mode();
void free_ril_request_cdma_query_preferred_voice_privacy_mode();
int compare_ril_request_cdma_query_preferred_voice_privacy_mode(ril_request_cdma_query_preferred_voice_privacy_mode_t *ptr1, ril_request_cdma_query_preferred_voice_privacy_mode_t *ptr2);

/*
    RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND
*/
void update_default_ril_request_hangup_waiting_or_background();
void update_default_ril_request_hangup_waiting_or_background_resp();
/*
    RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
*/
void update_default_ril_request_hangup_foreground_resume_background();
void update_default_ril_request_hangup_foreground_resume_background_resp();

/*
    RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE
*/
void update_default_ril_request_switch_waiting_or_holding_and_active();
void update_default_ril_request_switch_waiting_or_holding_and_active_resp();

/*
    RIL_REQUEST_CONFERENCE
*/
void update_default_ril_request_conference();
void update_default_ril_request_conference_resp();
/*
    RIL_REQUEST_UDUB
*/
void update_default_ril_request_udub();
void update_default_ril_request_udub_resp();

/*
    RIL_REQUEST_SEND_USSD
*/
ril_request_send_ussd_t* update_default_ril_request_send_ussd(char *);
void update_default_ril_request_send_ussd_resp();
ril_request_send_ussd_t* malloc_ril_request_send_ussd();
void free_ril_request_send_ussd();
int compare_ril_request_send_ussd(ril_request_send_ussd_t *ptr1, ril_request_send_ussd_t *ptr2);

/*
    RIL_REQUEST_CANCEL_USSD
*/
void update_default_ril_request_cancel_ussd();
void update_default_ril_request_cancel_ussd_resp();

/*
    RIL_REQUEST_GET_CLIR
*/
void update_default_ril_request_get_clir();
ril_request_get_clir_resp_t* update_default_ril_request_get_clir_resp();
ril_request_get_clir_resp_t* malloc_ril_request_get_clir_resp();
void free_ril_request_get_clir_resp();
int compare_ril_request_get_clir_resp(ril_request_get_clir_resp_t *ptr1, ril_request_get_clir_resp_t *ptr2);

/*
    RIL_REQUEST_SET_CLIR
*/
ril_request_set_clir_t* update_default_ril_request_set_clir();
void update_default_ril_request_set_clir_resp();
ril_request_set_clir_t* malloc_ril_request_set_clir();
void free_ril_request_set_clir();
int compare_ril_request_set_clir(ril_request_set_clir_t *ptr1, ril_request_set_clir_t *ptr2);

/*
    RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE
*/
ril_request_unsol_cell_info_rate_t *update_default_ril_request_set_unsol_cell_info_list_rate();
void update_default_ril_request_set_unsol_cell_info_list_rate_resp();

/*
    RIL_REQUEST_SET_MUTE
*/
ril_request_mute_t *update_default_ril_request_set_mute();
void update_default_ril_request_set_mute_resp();

/*
    RIL_REQUEST_GET_MUTE
*/
void update_default_ril_request_get_mute();
ril_request_mute_t *update_default_ril_request_get_mute_resp();

/*
    RIL_REQUEST_QUERY_CALL_FORWARD_STATUS
*/
RIL_CallForwardInfo* update_default_ril_request_query_call_forward_status();
RIL_CallForwardInfo** update_default_ril_request_query_call_forward_status_resp(int num_info);
void set_query_call_forward_status_resp_number(int slot, char *number);
RIL_CallForwardInfo* malloc_ril_request_query_call_forward_status();
void free_ril_request_query_call_forward_status();
RIL_CallForwardInfo* malloc_ril_request_query_call_forward_status_resp();
void free_ril_request_query_call_forward_status_resp();
int compare_ril_request_query_call_forward_status(RIL_CallForwardInfo *ptr1, RIL_CallForwardInfo *ptr2);
int compare_ril_request_query_call_forward_status_resp(RIL_CallForwardInfo **ptr1, RIL_CallForwardInfo **ptr2);

/*
    RIL_REQUEST_SET_CALL_FORWARD
*/
RIL_CallForwardInfo* update_default_ril_request_set_call_forward();
void update_default_ril_request_set_call_forward_resp();
RIL_CallForwardInfo* malloc_ril_request_set_call_forward();
void free_ril_request_set_call_forward();
int compare_ril_request_set_call_forward(RIL_CallForwardInfo *ptr1, RIL_CallForwardInfo *ptr2);

/*
    RIL_REQUEST_QUERY_CALL_WAITING
*/
ril_request_query_call_waiting_t* update_default_ril_request_query_call_waiting();
ril_request_query_call_waiting_resp_t* update_default_ril_request_query_call_waiting_resp();
ril_request_query_call_waiting_t* malloc_ril_request_query_call_waiting();
void free_ril_request_query_call_waiting();
ril_request_query_call_waiting_resp_t* malloc_ril_request_query_call_waiting_resp();
void free_ril_request_query_call_waiting_resp();
int compare_ril_request_query_call_waiting(ril_request_query_call_waiting_t *ptr1, ril_request_query_call_waiting_t *ptr2);
int compare_ril_request_query_call_waiting_resp(ril_request_query_call_waiting_resp_t *ptr1, ril_request_query_call_waiting_resp_t *ptr2);

/*
    RIL_REQUEST_SET_CALL_WAITING
*/
ril_request_set_call_waiting_t* update_default_ril_request_set_call_waiting();
void update_default_ril_request_set_call_waiting_resp();
ril_request_set_call_waiting_t* malloc_ril_request_set_call_waiting();
void free_ril_request_set_call_waiting();
int compare_ril_request_set_call_waiting(ril_request_set_call_waiting_t *ptr1, ril_request_set_call_waiting_t *ptr2);

/*
    RIL_REQUEST_QUERY_FACILITY_LOCK
*/
char** update_default_ril_request_query_facility_lock();
void update_default_ril_request_query_facility_lock_resp(int service_class);
char** malloc_ril_request_query_facility_lock();
void free_ril_request_query_facility_lock();
int compare_ril_request_query_facility_lock(ril_request_query_facility_lock_t **ptr1, ril_request_query_facility_lock_t **ptr2);
/*
    RIL_REQUEST_SET_FACILITY_LOCK
*/
char** update_default_ril_request_set_facility_lock();
void update_default_ril_request_set_facility_lock_resp();
char** malloc_ril_request_set_facility_lock();
void free_ril_request_set_facility_lock();
ril_request_set_facility_lock_resp_t* malloc_ril_request_set_facility_lock_resp();
void free_ril_request_set_facility_lock_resp();
int compare_ril_request_set_facility_lock(ril_request_set_facility_lock_t **ptr1, ril_request_set_facility_lock_t **ptr2);
int compare_ril_request_set_facility_lock_resp(ril_request_set_facility_lock_resp_t *ptr1, ril_request_set_facility_lock_resp_t *ptr2);

/*
    RIL_REQUEST_CHANGE_BARRING_PASSWORD
*/
ril_request_change_barring_password_t* update_default_ril_request_change_barring_password();
void update_default_ril_request_change_barring_password_resp();
ril_request_change_barring_password_t* malloc_ril_request_change_barring_password();
void free_ril_request_change_barring_password();
int compare_ril_request_change_barring_password(ril_request_change_barring_password_t **ptr1, ril_request_change_barring_password_t **ptr2);

/*
    RIL_REQUEST_SEPARATE_CONNECTION
*/
ril_request_separate_connection_t* update_default_ril_request_separate_connection(int call_id);
void update_default_ril_request_separate_connection_resp();
ril_request_separate_connection_t* malloc_ril_request_separate_connection();
void free_ril_request_separate_connection();
int compare_ril_request_separate_connection(ril_request_separate_connection_t *ptr1, ril_request_separate_connection_t *ptr2);

/*
    RIL_REQUEST_QUERY_CLIP
*/
void update_default_ril_request_query_clip();
void update_default_ril_request_query_clip_resp(int clip_status);

/*
    RIL_REQUEST_GET_CELL_INFO_LIST
*/
void update_default_ril_request_get_cell_info_list();
RIL_CellInfo_v12 *update_default_ril_request_get_cell_info_list_resp(size_t len);

/*
    RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION
*/
ril_request_set_supp_svc_notification_t* update_default_ril_request_set_supp_svc_notification();
void update_default_ril_request_set_supp_svc_notification_resp();
ril_request_set_supp_svc_notification_t* malloc_ril_request_set_supp_svc_notification();
void free_ril_request_set_supp_svc_notification();
int compare_ril_request_set_supp_svc_notification(ril_request_set_supp_svc_notification_t *ptr1, ril_request_set_supp_svc_notification_t *ptr2);

/*
    RIL_REQUEST_EXPLICIT_CALL_TRANSFER
*/
void update_default_ril_request_explicit_call_transfer();
void update_default_ril_request_explicit_call_transfer_resp();

/*
    RIL_REQUEST_RADIO_POWER
*/
/*
    RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE
*/
void update_default_ril_request_query_network_selection_mode();
ril_request_query_network_selection_mode_resp_t* update_default_ril_request_query_network_selection_mode_resp();
ril_request_query_network_selection_mode_resp_t* malloc_ril_request_query_network_selection_mode_resp();
void free_ril_request_query_network_selection_mode_resp();
int compare_ril_request_query_network_selection_mode_resp(ril_request_query_network_selection_mode_resp_t *ptr1, ril_request_query_network_selection_mode_resp_t *ptr2);

/*
    RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC
*/
void update_default_ril_request_set_network_selection_automatic();
void update_default_ril_request_set_network_selection_automatic_resp();

/*
    RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL
*/
void update_default_ril_request_set_network_selection_manual(int is_legacy, int is_combine);
void set_ril_request_set_network_selection_manual(int is_legacy, int is_combine, char *mccmnc, char *rat);
void update_default_ril_request_set_network_selection_manual_resp();
ril_request_set_network_selection_manual_t* malloc_ril_request_set_network_selection_manual_in_legacy_case();
char *malloc_ril_request_set_network_selection_manual(int is_combine);
void free_ril_request_set_network_selection_manual();
int compare_ril_request_set_network_selection_manual(ril_request_set_network_selection_manual_t *ptr1, ril_request_set_network_selection_manual_t *ptr2);

/*
    RIL_REQUEST_QUERY_AVAILABLE_NETWORKS
*/
/*
    RIL_REQUEST_RESET_RADIO
*/
void update_default_ril_request_reset_radio();
void update_default_ril_request_reset_radio_resp();

/*
    RIL_REQUEST_DEVICE_IDENTITY
*/
void update_default_ril_request_device_identity_req();
ril_request_device_identity_type_t* update_default_ril_request_device_identity_resp();

/*
    RIL_REQUEST_BASEBAND_VERSION
*/
void update_default_ril_request_baseband_version_req();
ril_request_get_baseband_version_type_t* update_default_ril_request_baseband_version_resp();

/*
    RIL_REQUEST_SET_BAND_MODE
*/
ril_request_set_band_mode_t* update_default_ril_request_set_band_mode();
void update_default_ril_request_set_band_mode_resp();
ril_request_set_band_mode_t* malloc_ril_request_set_band_mode();
void free_ril_request_set_band_mode();
int compare_ril_request_set_band_mode(ril_request_set_band_mode_t *ptr1, ril_request_set_band_mode_t *ptr2);

/*
    RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE
*/
void update_default_ril_request_query_available_band_mode();
int* update_Default_ril_request_query_available_band_mode_resp();
int* malloc_ril_request_query_available_band_mode_resp();
void free_ril_request_query_available_band_mode_resp();
int compare_ril_request_query_available_band_mode_resp(ril_request_query_available_band_mode_resp_t *ptr1, ril_request_query_available_band_mode_resp_t *ptr2);
/*
    RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE
*/
RIL_PreferredNetworkType* update_default_ril_request_set_preferred_network_type();
void update_default_ril_request_set_preferred_network_type_resp();

/*
    RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE
*/
void update_default_ril_request_get_preferred_network_type();
RIL_PreferredNetworkType* update_default_ril_request_get_preferred_network_type_resp();
void update_default_ril_request_get_preferred_network_type_resp_failure();
RIL_PreferredNetworkType* malloc_ril_request_get_preferred_network_type_resp();
void free_ril_request_get_preferred_network_type_resp();
int compare_ril_request_get_preferred_network_type_resp(RIL_PreferredNetworkType *ptr1, RIL_PreferredNetworkType *ptr2);

/*
    RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE
*/
RIL_CdmaSubscriptionSource* update_default_ril_request_cdma_set_subscription_source();
void update_default_ril_request_cdma_set_subscription_source_resp();
RIL_CdmaSubscriptionSource* malloc_ril_request_cdma_set_subscription_source();
void free_ril_request_cdma_set_subscription_source();
int compare_ril_request_cdma_set_subscription_source(RIL_CdmaSubscriptionSource *ptr1, RIL_CdmaSubscriptionSource *ptr2);
/*
    RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE
*/
void update_default_ril_request_cdma_get_subscription_source();
RIL_CdmaSubscriptionSource* update_default_ril_request_cdma_get_subscription_source_resp();
void update_default_ril_request_cdma_get_subscription_source_resp(int);
void update_default_ril_request_cdma_get_subscription_source_resp_failure();
RIL_CdmaSubscriptionSource* malloc_ril_request_cdma_get_subscription_source_resp();
void free_ril_request_cdma_get_subscription_source_resp();
int compare_ril_request_cdma_get_subscription_source_resp(RIL_CdmaSubscriptionSource *ptr1, RIL_CdmaSubscriptionSource *ptr2);

/*
    RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE
*/
ril_request_cdma_set_roaming_preference_t* update_default_ril_request_cdma_set_roaming_preference();
void update_default_ril_request_cdma_set_roaming_preference_resp();
ril_request_cdma_set_roaming_preference_t* malloc_ril_request_cdma_set_roaming_preference();
void free_ril_request_cdma_set_roaming_preference();
int compare_ril_request_cdma_set_roaming_preference(ril_request_cdma_set_roaming_preference_t *ptr1, ril_request_cdma_set_roaming_preference_t *ptr2);

/*
    RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE
*/
void update_default_ril_request_cdma_query_roaming_preference();
ril_request_cdma_query_roaming_preference_resp_t* update_default_ril_request_cdma_query_roaming_preference_resp();
void update_default_ril_request_cdma_query_roaming_preference_resp(ril_cdma_roaming_preference_enum_type cdma_roam_pref);
void update_default_ril_request_cdma_query_roaming_preference_resp_failure();
ril_request_cdma_query_roaming_preference_resp_t* malloc_ril_request_cdma_query_roaming_preference_resp();
void free_ril_request_cdma_query_roaming_preference_resp();
int compare_ril_request_cdma_query_roaming_preference_resp(ril_request_cdma_query_roaming_preference_resp_t *ptr1, ril_request_cdma_query_roaming_preference_resp_t *ptr2);
/*
    RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE
*/
void update_default_exit_emergency_callback_mode();
void update_default_exit_emergency_callback_mode_resp();
/*
    RIL_REQUEST_SET_UICC_SUBSCRIPTION
*/
RIL_SelectUiccSub* update_default_ril_request_set_uicc_subscription();
void update_default_ril_request_set_uicc_subscription_resp();
RIL_SelectUiccSub* malloc_ril_request_set_uicc_subscription();
void set_ril_request_set_uicc_subscription_app_index(int app_index);
void set_ril_request_set_uicc_subscription_slot(int slot);
void set_ril_request_set_uicc_subscription_act_status(RIL_UiccSubActStatus act_status);
void set_ril_request_set_uicc_subscription_sub_type(RIL_SubscriptionType sub_type);
void free_ril_request_set_uicc_subscription();
int compare_ril_request_set_uicc_subscription(RIL_SelectUiccSub *ptr1, RIL_SelectUiccSub *ptr2);

/*
    RIL_REQUEST_SET_DATA_SUBSCRIPTION
*/
void update_default_ril_request_set_data_subscription();
void update_default_ril_request_set_data_subscription_resp();

/*
    RIL_REQUEST_GET_UICC_SUBSCRIPTION
*/
void update_default_ril_request_get_uicc_subscription();
RIL_SelectUiccSub* update_default_ril_request_get_uicc_subscription_resp();
RIL_SelectUiccSub* malloc_ril_request_get_uicc_subscription_resp();
void free_ril_request_get_uicc_subscription_resp();
int compare_ril_request_get_uicc_subscription_resp(RIL_SelectUiccSub *ptr1, RIL_SelectUiccSub *ptr2);

/*
    RIL_REQUEST_GET_DATA_SUBSCRIPTION
*/
void update_default_ril_request_get_data_subscription();
ril_request_get_data_subscription_resp_t* update_default_ril_request_get_data_subscription_resp();
ril_request_get_data_subscription_resp_t* malloc_ril_request_get_data_subscription_resp();
void free_ril_request_get_data_subscription_resp();
int compare_ril_request_get_data_subscription_resp(ril_request_get_data_subscription_resp_t *ptr1, ril_request_get_data_subscription_resp_t *ptr2);

/*
    RIL_REQUEST_SET_SUBSCRIPTION_MODE
*/
ril_request_set_subscription_mode_t* update_default_ril_request_set_subscription_mode();
void update_default_ril_request_set_subscription_mode_resp();
ril_request_set_subscription_mode_t* malloc_ril_request_set_subscription_mode();
void free_ril_request_set_subscription_mode();
int compare_ril_request_set_subscription_mode(ril_request_set_subscription_mode_t *ptr1, ril_request_set_subscription_mode_t *ptr2);

/*
    RIL_REQUEST_SET_TRANSMIT_POWER
*/
void update_default_ril_request_set_transmit_power();
void update_default_ril_request_set_transmit_power_resp();

/*
    RIL_REQUEST_VOICE_RADIO_TECH
*/
void update_default_ril_request_voice_radio_tech();
RIL_RadioTechnology* update_default_ril_request_voice_radio_tech_resp();
RIL_RadioTechnology* malloc_ril_request_voice_radio_tech_resp();
void free_ril_request_voice_radio_tech_resp();
int compare_ril_request_voice_radio_tech_resp(RIL_RadioTechnology *ptr1, RIL_RadioTechnology *ptr2);

/*
    RIL_REQUEST_GET_DC_RT_INFO
*/
void update_default_ril_request_get_dc_rt_info();
void update_default_ril_request_get_dc_rt_info_resp();
/*
    RIL_REQUEST_SIGNAL_STRENGTH
*/
void update_default_ril_request_signal_strength();
RIL_SignalStrength_v6* update_default_ril_request_signal_strength_resp();
RIL_SignalStrength_v6* malloc_ril_request_signal_strength_resp();
void free_ril_request_signal_strength_resp();
int compare_ril_request_signal_strength_resp(RIL_SignalStrength_v6 *ptr1, RIL_SignalStrength_v6 *ptr2);

/*
    RIL_REQUEST_REGISTRATION_STATE
*/
void update_default_ril_request_registration_state();
void update_default_ril_request_registration_state_resp_in_error_case();
RIL_VoiceRegistrationStateResponse* update_default_ril_request_registration_state_resp();

/*
    RIL_REQUEST_DATA_REGISTRATION_STATE
*/

void update_default_ril_request_data_registration_state();
void update_default_ril_request_data_registration_state_in_error_case();
RIL_DataRegistrationStateResponse* update_default_ril_request_data_registration_state_resp();

/*
    RIL_REQUEST_OPERATOR
*/
void update_default_ril_request_operator();
ril_request_operator_resp_from_qcril_t* update_default_ril_request_operator_resp();
ril_request_operator_resp_from_qcril_t* malloc_ril_request_operator_resp();

/*
    RIL_REQUEST_SCREEN_STATE
*/
ril_request_screen_state_t* update_default_ril_request_screen_state();
void update_default_ril_request_screen_state_resp();
ril_request_screen_state_t* malloc_ril_request_screen_state();
void free_ril_request_screen_state();
int compare_ril_request_screen_state(ril_request_screen_state_t *tpr1, ril_request_screen_state_t *ptr2);

/*
    RIL_REQUEST_GET_NEIGHBORING_CELL_IDS
*/
void update_default_ril_request_get_neighboring_cell_ids();
RIL_NeighboringCell** update_default_ril_request_get_neighboring_cell_ids_resp(int num_cells);
RIL_NeighboringCell** malloc_ril_request_get_neighboring_cell_ids_resp();
void free_ril_request_get_neighboring_cell_ids_resp();
int compare_ril_request_get_neighboring_cell_ids_resp(RIL_NeighboringCell **ptr1, RIL_NeighboringCell **ptr2);

/*
    RIL_REQUEST_SET_LOCATION_UPDATES
*/
ril_request_set_location_updates_t* update_default_ril_request_set_location_updates();
void update_default_ril_request_set_location_updates_resp();
ril_request_set_location_updates_t* malloc_ril_request_set_location_updates();
void free_ril_request_set_location_updates();
int compare_ril_request_set_location_updates(ril_request_set_location_updates_t *ptr1, ril_request_set_location_updates_t *ptr2);


/*========================================================
  RIL_REQUEST_QUERY_AVAILABLE_NETWORKS
========================================================*/
void update_default_ril_request_query_available_networks();
void update_default_ril_request_query_available_networks_resp_failure();
ril_request_query_available_networks_resp_t* update_default_ril_request_query_available_networks_resp(int nw_cnt, int is_legacy);
ril_request_query_available_networks_resp_t* malloc_ril_request_query_available_networks_resp(int nw_cnt, int is_legacy);

/*========================================================
  RIL_UNSOL_EMERGENCY_NUMBERS_LIST
========================================================*/
ril_unsol_emergency_list_t* update_default_ril_unsol_emergency_number_list(size_t len);

/*========================================================
  RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED
========================================================*/
void update_default_ril_unsol_resp_ims_network_state_changed(void);

/*========================================================
  RIL_REQUEST_IMS_REGISTRATION_STATE
========================================================*/
void update_default_ril_request_ims_reg_state(void);

/*========================================================
  RIL_REQUEST_IMS_REGISTRATION_STATE_RESP
========================================================*/
void update_default_ril_request_ims_reg_state_resp(uint8_t is_empty, int reg_status, int rat);

/*========================================================
  RIL_REQUEST_IMS_SEND_SMS
========================================================*/
void update_default_ril_request_ims_send_sms(uint8_t is_3gpp2);
ims_send_sms_3gpp2_payload* malloc_ril_request_ims_send_sms_3gpp2();
void set_ril_request_ims_send_sms_3gpp2_uTeleserviceID(int uTeleserviceID);
void set_ril_request_ims_send_sms_3gpp2_bIsServicePresent(unsigned char bIsServicePresent);
void set_ril_request_ims_send_sms_3gpp2_uServicecategory(int uServicecategory);
ims_send_sms_3gpp_payload* malloc_ril_request_ims_send_sms_3gpp();
ims_send_sms_3gpp_payload* set_ril_request_ims_send_sms_3gpp_tech_retry_msgref(RIL_RadioTechnologyFamily tech, unsigned char retry, int messageRef);
ims_send_sms_3gpp2_payload* set_ril_request_ims_send_sms_3gpp2_tech_retry_msgref(RIL_RadioTechnologyFamily tech, unsigned char retry, int messageRef);

/*========================================================
  RIL_REQUEST_IMS_SEND_SMS_RESP
========================================================*/
RIL_SMS_Response* update_default_ril_request_ims_send_sms_resp(void);
/*========================================================
  RIL_REQUEST_IMS_SEND_SMS_RESP_EMPTY
========================================================*/
void update_default_ril_request_ims_send_sms_resp_empty();

/*========================================================
  RIL_UNSOL_SRVCC_STATE_NOTIFY
========================================================*/
#ifdef RIL_UTF_L_MASTER
void update_default_ril_unsol_srvcc_state_notify(RIL_SrvccState srvcc_state);
#endif

/*========================================================
  RIL_REQUEST_SET_INITIAL_ATTACH_APN
========================================================*/
void update_default_ril_request_set_initial_attach_apn(void);
void update_default_ril_request_set_initial_attach_apn_resp(RIL_Errno err);
void update_ril_request_set_initial_attach_apn_address_and_protocol(const char* address, const char* protocol);

/*========================================================
 RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED
========================================================*/
void update_default_ril_unsol_uicc_subscription_status_changed(int data);

/*========================================================
  RIL_REQUEST_ALLOW_DATA
========================================================*/
#ifdef RIL_UTF_L_MASTER
void update_default_ril_request_allow_data_req(int allow_data);
void update_default_ril_request_allow_data_resp(void);
void update_default_ril_request_allow_data_resp(RIL_Errno err);

/*========================================================
  RIL_REQUEST_SET_RADIO_CAPABILITY
========================================================*/
void update_default_ril_request_set_radio_capability_req
                                                    (int version,
                                                     int session,
                                                     int phase,
                                                     int rat,
                                                     char *logicalModemUuid_data,
                                                     unsigned int logicalModemUuid_len,
                                                     int status);
void update_default_ril_request_set_radio_capability_resp
                                                    (RIL_Errno err,
                                                     int version,
                                                     int session,
                                                     int phase,
                                                     int rat,
                                                     char *logicalModemUuid_data,
                                                     unsigned int logicalModemUuid_len,
                                                     int status);

/*========================================================
  RIL_REQUEST_GET_RADIO_CAPABILITY
========================================================*/
void update_default_ril_request_get_radio_capability_req
                                                    (int version,
                                                     int session,
                                                     int phase,
                                                     int rat,
                                                     char *logicalModemUuid_data,
                                                     unsigned int logicalModemUuid_len,
                                                     int status);
void update_default_ril_request_get_radio_capability_resp
                                                    (RIL_Errno err,
                                                     int version,
                                                     int session,
                                                     int phase,
                                                     int rat,
                                                     char *logicalModemUuid_data,
                                                     unsigned int logicalModemUuid_len,
                                                     int status);

/*========================================================
  RIL_UNSOL_RADIO_CAPABILITY
========================================================*/
void update_default_ril_unsol_radio_capability
                                            (int version,
                                             int session,
                                             int phase,
                                             int rat,
                                             char *logicalModemUuid_data,
                                             unsigned int logicalModemUuid_len,
                                             int status);

#endif
/*========================================================
  RIL_REQUEST_OEM_HOOK_RAW
========================================================*/
void update_default_ril_request_oem_hook_raw_req(int message_id, void *data, uint32_t data_len);
void update_default_ril_request_oem_hook_raw_resp(RIL_Errno);

/*========================================================
 RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED
========================================================*/
void update_default_ril_unsol_cdma_subscription_source_changed(int data);

/*========================================================
  RIL_UNSOL_RESTRICTED_STATE_CHANGED
========================================================*/
void update_default_ril_unsol_restricted_state_changed(int data);

/*========================================================
  RIL_UNSOL_CDMA_CALL_WAITING
========================================================*/
RIL_CDMA_CallWaiting_v6 *update_default_ril_unsol_cdma_call_waiting(void);

/*========================================================
  RIL_UNSOL_CDMA_OTA_PROVISION_STATUS
========================================================*/
void update_default_ril_unsol_cdma_ota_provision_status(RIL_CDMA_OTA_ProvisionStatus data);

/*========================================================
  RIL_UNSOL_CDMA_INFO_REC
========================================================*/
RIL_CDMA_InformationRecords *update_default_ril_unsol_cdma_info_rec(void);

/*========================================================
  RIL_REQUEST_GET_ACTIVITY_INFO
========================================================*/
void update_default_ril_request_get_activity_info_req(void);
void update_default_ril_request_get_activity_info_resp(void);

/*========================================================
  RIL_REQUEST_SET_LTE_BAND_PREF
========================================================*/
void update_default_ril_request_set_lte_band_pref_req(int* data);
void update_default_ril_request_set_lte_band_pref_resp(void);
void update_default_ril_request_set_lte_band_pref_resp(RIL_Errno err);

/*
    RIL_REQUEST_NV_RESET_CONFIG
*/
void update_default_ril_request_nv_reset_config();
void update_default_ril_request_nv_reset_config_resp();

/*
    RIL_REQUEST_SHUTDOWN
*/
void update_default_ril_request_shutdown();
void update_default_ril_request_shutdown_resp();

/*
    RIL_REQUEST_START_NETWORK_SCAN
*/
RIL_NetworkScanRequest *update_default_ril_request_start_network_scan(RIL_NetworkScanRequest *req = nullptr);
void update_default_ril_request_start_network_scan_resp();

/*
    RIL_REQUEST_STOP_NETWORK_SCAN
*/
void update_default_ril_request_stop_network_scan();
void update_default_ril_request_stop_network_scan_resp();

/*
    RIL_UNSOL_NETWORK_SCAN_RESULT
*/
RIL_NetworkScanResult *update_default_ril_unsol_network_scan_result();

/* RIL_REQUEST_SIM_IO */

RIL_SIM_IO_v6*       update_default_ril_request_sim_io();
RIL_SIM_IO_Response* update_default_ril_request_sim_io_resp();

/* RIL_REQUEST_ENABLE_5G */
void update_default_enable_5g();
void update_default_enable_5g_resp();

/* RIL_REQUEST_ENABLE_MODEM */
int* update_default_enable_modem(void);
void fill_enable_modem(int *, bool);
void update_default_enable_modem_resp(RIL_Errno err);

/* RIL_REQUEST_ENABLE_5G */
void update_default_get_modem_stack_status(void);
void update_default_get_modem_stack_status_resp(bool on);

/* RIL_REQUEST_SIM_OPEN_CHANNEL */
RIL_OpenChannelParams* update_default_ril_request_open_logical_channel();
uint8_t* update_default_ril_request_open_logical_channel_resp(int payload_length);

} AndroidTelTestnode;

#endif /* RIL_UTF_ANDROIDTEL_IF_H_ */
