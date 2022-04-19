/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "ril_utf_androidTel_if.h"
#ifdef QMI_RIL_HAL_UTF
#include "qcril_qmi_oem_reqlist.h"
#endif
using namespace std;
//---------------------------------------------------------------------------------------

/*========================================================
  **** RIL API definitions ***
========================================================*/

/*========================================================
  RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_resp_radio_state_changed(void)
{
  struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t* tmp =
    (struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t*) malloc(sizeof(struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t));
  tmp->state = RADIO_STATE_UNAVAILABLE;

  payload = NULL;
  //payload_len = sizeof(struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t);
  payload_len = 0;
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED;
}
void ril_utf_androidTel_if::set_ril_unsol_resp_radio_state_changed_state
  (RIL_RadioState state)
{
  struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t* ptr =
    (struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t*) payload;
  ptr->state = state;
}

/*========================================================
  RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_response_call_state_changed(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED;
}

/*========================================================
  RIL_UNSOL_RINGBACK_TONE
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_ringback_tone(bool start)
{
  struct PACKED_STRUCT ril_unsol_ringback_tone_t* tmp =
    (struct PACKED_STRUCT ril_unsol_ringback_tone_t*)
    malloc(sizeof(struct PACKED_STRUCT ril_unsol_ringback_tone_t));
  payload = tmp;
  payload_len = sizeof(struct PACKED_STRUCT ril_unsol_ringback_tone_t);
  memset(payload, 0x00, payload_len);
  tmp->start = start;
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_RINGBACK_TONE;
}


/*========================================================
  RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_voice_radio_tech_changed(void)
{
  struct PACKED_STRUCT ril_unsol_resp_voice_radio_tech_changed_t* tmp =
    (struct PACKED_STRUCT ril_unsol_resp_voice_radio_tech_changed_t*) malloc(sizeof(struct PACKED_STRUCT ril_unsol_resp_voice_radio_tech_changed_t));
  tmp->tech = RADIO_TECH_UMTS;

  payload = tmp;
  payload_len = sizeof(struct PACKED_STRUCT ril_unsol_resp_voice_radio_tech_changed_t);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_VOICE_RADIO_TECH_CHANGED;
}
void ril_utf_androidTel_if::set_ril_unsol_voice_radio_tech_changed_radio_tech(RIL_RadioTechnology radio_tech)
{
  struct PACKED_STRUCT ril_unsol_resp_voice_radio_tech_changed_t* tmp =
    (struct PACKED_STRUCT ril_unsol_resp_voice_radio_tech_changed_t*) payload;
  tmp->tech = radio_tech;
}

/*========================================================
  RIL_UNSOL_SIGNAL_STRENGTH
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_signal_strength(void)
{
  struct PACKED_STRUCT ril_unsol_signal_strength_t* tmp =
    (struct PACKED_STRUCT ril_unsol_signal_strength_t*) malloc(sizeof(struct PACKED_STRUCT ril_unsol_signal_strength_t));

  payload = tmp;
  payload_len = sizeof(struct PACKED_STRUCT ril_unsol_signal_strength_t);
  memset(payload, 0x00, payload_len);
  verify_payload = false;
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_SIGNAL_STRENGTH;
}
void ril_utf_androidTel_if::set_ril_unsol_signal_strength_gw_signal_strength(int signal_strength, int bit_error_rate)
{
  struct PACKED_STRUCT ril_unsol_signal_strength_t *ptr =
      (struct PACKED_STRUCT ril_unsol_signal_strength_t *) payload;
  ptr->signal_strength.GW_SignalStrength.signalStrength = signal_strength;
  ptr->signal_strength.GW_SignalStrength.bitErrorRate = bit_error_rate;
}
void ril_utf_androidTel_if::set_ril_unsol_signal_strength_cdma_signal_strength(int dbm, int ecio)
{
  struct PACKED_STRUCT ril_unsol_signal_strength_t *ptr =
      (struct PACKED_STRUCT ril_unsol_signal_strength_t *) payload;
  ptr->signal_strength.CDMA_SignalStrength.dbm = dbm;
  ptr->signal_strength.CDMA_SignalStrength.ecio = ecio;
}
void ril_utf_androidTel_if::set_ril_unsol_signal_strength_evdo_signal_strength(int dbm, int ecio, int snr)
{
  struct PACKED_STRUCT ril_unsol_signal_strength_t *ptr =
      (struct PACKED_STRUCT ril_unsol_signal_strength_t *) payload;
  ptr->signal_strength.EVDO_SignalStrength.dbm = dbm;
  ptr->signal_strength.EVDO_SignalStrength.ecio = ecio;
  ptr->signal_strength.EVDO_SignalStrength.signalNoiseRatio = snr;
}
void ril_utf_androidTel_if::set_ril_unsol_signal_strength_lte_signal_strength
            (int signal_strength, int rsrp, int rsrq, int rssnr, int cqi, int timing_advance)
{
  struct PACKED_STRUCT ril_unsol_signal_strength_t *ptr =
      (struct PACKED_STRUCT ril_unsol_signal_strength_t *) payload;
  ptr->signal_strength.LTE_SignalStrength.signalStrength = signal_strength;
  ptr->signal_strength.LTE_SignalStrength.rsrp = rsrp;
  ptr->signal_strength.LTE_SignalStrength.rsrq = rsrq;
  ptr->signal_strength.LTE_SignalStrength.rssnr = rssnr;
  ptr->signal_strength.LTE_SignalStrength.cqi = cqi;
  ptr->signal_strength.LTE_SignalStrength.timingAdvance = timing_advance;
}
void ril_utf_androidTel_if::set_ril_unsol_signal_strength_td_scdma_signal_strength(int recp)
{
  struct PACKED_STRUCT ril_unsol_signal_strength_t *ptr =
      (struct PACKED_STRUCT ril_unsol_signal_strength_t *) payload;
#ifndef RIL_UTF_L_MASTER
  ptr->signal_strength.TD_SCDMA_SignalStrength.rscp = recp;
#endif
}

/*========================================================
  RIL_UNSOL_STK_CC_ALPHA_NOTIFY
========================================================*/
uint8_t* ril_utf_androidTel_if::update_default_ril_unsol_stk_cc_alpha_notify(void)
{
  payload = malloc_ril_request_get_alpha_text_resp();
  payload_len = sizeof(uint8_t) * 257;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_STK_CC_ALPHA_NOTIFY;

  uint8_t *ptr = (uint8_t*) payload;
  return ptr;
}

uint8_t* ril_utf_androidTel_if::malloc_ril_request_get_alpha_text_resp()
{
  uint8_t *ptr= (uint8_t *) malloc(sizeof(uint8_t) * 257);
  return ptr;
}

void ril_utf_androidTel_if::free_ril_request_get_alpha_text_resp()
{
  struct PACKED_STRUCT ril_unsol_stk_cc_alpha* ptr = (struct PACKED_STRUCT ril_unsol_stk_cc_alpha*)payload;
  free(ptr);
}

/*========================================================
  RIL_UNSOL_ON_USSD
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_on_ussd(char a, const char *str, int str_len)
{
  char type_code [2];
  char *response_buff [2];
  char *ptr=NULL;
  memset( type_code, 0, sizeof( type_code ) );

  type_code[ 0 ] = a;
  type_code[ 1 ] = '\0';
  response_buff[ 0 ] = (char *)malloc(2*sizeof(char));
  memcpy(response_buff[ 0 ], type_code, 2);
  response_buff[ 1 ] = NULL;
  if( str != NULL)
  {
    response_buff[ 1 ] = (char *)malloc(str_len * sizeof(char));
    memset(response_buff[ 1 ], 0, str_len);
    memcpy(response_buff[ 1 ], str, str_len);
  }
  ptr = (char *)malloc(1* sizeof(response_buff));
  memcpy(ptr,response_buff, sizeof(response_buff) );
  payload = (void *)ptr;
  payload_len = sizeof( response_buff );
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_ON_USSD;
}

/*========================================================
  RIL_UNSOL_ON_SS
========================================================*/
RIL_StkCcUnsolSsResponse* ril_utf_androidTel_if::update_default_ril_unsol_on_ss(void)
{
  payload = malloc_ril_request_unsol_ss_resp();
  payload_len = sizeof(RIL_StkCcUnsolSsResponse);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_ON_SS;

  RIL_StkCcUnsolSsResponse *ptr = (RIL_StkCcUnsolSsResponse*) payload;
  return ptr;
}

RIL_StkCcUnsolSsResponse* ril_utf_androidTel_if::malloc_ril_request_unsol_ss_resp()
{
  RIL_StkCcUnsolSsResponse *ptr= (RIL_StkCcUnsolSsResponse *) malloc(sizeof(RIL_StkCcUnsolSsResponse) * 1);
  memset( ptr, 0, sizeof(RIL_StkCcUnsolSsResponse) );
  return ptr;
}

/*========================================================
  RIL_UNSOL_SUPP_SVC_NOTIFICATION
========================================================*/
RIL_SuppSvcNotification *ril_utf_androidTel_if::update_default_ril_unsol_supp_svc_notification()
{
  payload = qcril_malloc(sizeof(RIL_SuppSvcNotification) * 1);
  payload_len = sizeof(RIL_SuppSvcNotification);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_SUPP_SVC_NOTIFICATION;

  RIL_SuppSvcNotification *ptr = (RIL_SuppSvcNotification*) payload;
  return ptr;
}

/*========================================================
  RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_response_voice_network_state_changed(void)
{

  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED;
}

/*========================================================
  RIL_REQUEST_VOICE_REGISTRATION_STATE
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_voice_registration_state(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_VOICE_REGISTRATION_STATE;
}
char** ril_utf_androidTel_if::update_default_ril_request_voice_registration_state_resp(void)
{
  int x;
  payload = malloc(sizeof(char*) * 15);  // Defined in ril.h
  payload_len = (sizeof(char*) * 15);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_VOICE_REGISTRATION_STATE;

  char **ptr = (char**) payload;

  for (x = 0; x < 15; ++x)
  {
    ptr[x] = (char *)malloc(sizeof(char) * 30);
  }

  return ptr;
}

/*========================================================
  RIL_UNSOL_RESPONSE_NEW_SMS
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_response_new_sms(char *ascii_hex_string)
{
  payload = malloc(strlen(ascii_hex_string) +1);
  payload_len = strlen(ascii_hex_string)+1;
  memset(payload, 0, payload_len);
  strlcpy((char*)payload, ascii_hex_string, strlen(ascii_hex_string)+1);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_RESPONSE_NEW_SMS ;
}

/*========================================================
  RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_response_new_sms_status_report(char *ascii_hex_string)
{
  payload = malloc(strlen(ascii_hex_string) +1);
  payload_len = strlen(ascii_hex_string)+1;
  strlcpy((char*)payload, ascii_hex_string, strlen(ascii_hex_string));
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT;
}

/*========================================================
  RIL_UNSOL_SIM_SMS_STORAGE_FULL
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_sim_sms_storage_full(
 void)
{
  payload = malloc(payload_len);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_SIM_SMS_STORAGE_FULL;
}

/*========================================================
  RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_ruim_sms_storage_full(
 void)
{
  payload = malloc(payload_len);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL;
}

/*========================================================
  RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_response_new_broadcast_sms(
  unsigned char* hex_values, size_t len )
{
  payload_len = len;
  payload = malloc(payload_len);
  memset(payload, 0, payload_len);
  memcpy (payload, hex_values, len);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS;
}

/*========================================================
  RIL_UNSOL_CALL_RING
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_call_ring(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_CALL_RING;
}

/*========================================================
  RIL_UNSOL_OEM_HOOK_RAW
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_oem_hook_raw(void)
{
  payload = malloc(20);
  payload_len = 20;
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_OEM_HOOK_RAW;
}

/*========================================================
  RIL_REQUEST_RADIO_POWER
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_radio_power(void)
{
  struct PACKED_STRUCT ril_request_radio_power_t* tmp =
    (struct PACKED_STRUCT ril_request_radio_power_t*) malloc(sizeof(struct PACKED_STRUCT ril_request_radio_power_t));
  tmp->power = 1;

  payload = tmp;
  payload_len = sizeof(struct PACKED_STRUCT ril_request_radio_power_t);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_RADIO_POWER;
}
void ril_utf_androidTel_if::update_default_ril_request_radio_power_resp(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_RADIO_POWER;
}

/*========================================================
  RIL_REQUEST_SEND_SMS
========================================================*/

#define SMSC_address_size 30

char** ril_utf_androidTel_if::update_default_ril_request_send_sms(void)
{
  payload = malloc_ril_request_send_sms();
  payload_len = sizeof(char*)*2;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SEND_SMS;

  char** ptr = (char**) payload;
  return ptr;
}

RIL_SMS_Response* ril_utf_androidTel_if::update_default_ril_request_send_sms_resp(void)
{
  payload = malloc_ril_request_send_sms_resp();
  payload_len = sizeof(RIL_SMS_Response);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SEND_SMS;

  RIL_SMS_Response *ptr = (RIL_SMS_Response *)payload;
  ptr->errorCode = -1;
  return ptr;
}

char** ril_utf_androidTel_if::malloc_ril_request_send_sms(void)
{
  char** ptr = (char**)malloc(sizeof(char*) *2);
  ptr[0] = (char *)malloc(SMSC_address_size);
  ptr[1] = (char *)malloc(WMS_MESSAGE_LENGTH_MAX_V01);
  return ptr;
}

void ril_utf_androidTel_if::free_ril_request_send_sms(void)
{
  ril_request_send_sms_t* ptr = (ril_request_send_sms_t*) payload;
  free(ptr->SMSC_address);
  free(ptr->SMS_pdu_format);
  free(ptr);
}

RIL_SMS_Response* ril_utf_androidTel_if::malloc_ril_request_send_sms_resp()
{
  RIL_SMS_Response *ptr = (RIL_SMS_Response*) malloc(sizeof(RIL_SMS_Response));
  memset(ptr, 0, sizeof(RIL_SMS_Response));
  return ptr;
}

void ril_utf_androidTel_if::free_ril_request_send_sms_resp()
{
  RIL_SMS_Response* ptr = (RIL_SMS_Response*) payload;
  free(ptr->ackPDU);
  free(ptr);
}

int ril_utf_androidTel_if::compare_ril_request_send_sms(ril_request_send_sms_t *ptr1,ril_request_send_sms_t *ptr2)
{
   if(memcmp(ptr1->SMSC_address, ptr2->SMSC_address, sizeof(ptr1->SMSC_address))!=0)
      return FALSE;
   else if(memcmp(ptr1->SMS_pdu_format, ptr2->SMS_pdu_format, sizeof(ptr1->SMS_pdu_format))!=0)\
      return FALSE;
   else
      return TRUE;
}

int ril_utf_androidTel_if::compare_ril_request_send_sms_resp(RIL_SMS_Response *ptr1, RIL_SMS_Response *ptr2)
{
  if(ptr1->messageRef != ptr2->messageRef)
      return FALSE;
  else if(memcmp(ptr1->ackPDU, ptr2->ackPDU, sizeof(ptr1->ackPDU))!=0)
      return FALSE;
  else if(ptr1->errorCode != ptr2-> errorCode)
      return FALSE;
  else
      return TRUE;
}

/*========================================================
  RIL_REQUEST_SEND_SMS_EXPECT_MORE
========================================================*/
ril_request_send_sms_expect_more_t* ril_utf_androidTel_if::update_default_ril_request_send_sms_expect_more(void)
{
  payload = malloc_ril_request_send_sms_expect_more();
  payload_len = sizeof(ril_request_send_sms_t);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SEND_SMS_EXPECT_MORE;

  ril_request_send_sms_expect_more_t* ptr = (ril_request_send_sms_expect_more_t*) payload;
  return ptr;
}
RIL_SMS_Response* ril_utf_androidTel_if::update_default_ril_request_send_sms_expect_more_resp(void)
{
  payload = malloc_ril_request_send_sms_expect_more_resp();
  payload_len = sizeof(RIL_SMS_Response);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SEND_SMS_EXPECT_MORE;

  RIL_SMS_Response *ptr = (RIL_SMS_Response *)payload;
  return ptr;
}
ril_request_send_sms_expect_more_t* ril_utf_androidTel_if::malloc_ril_request_send_sms_expect_more(void)
{
  ril_request_send_sms_expect_more_t* ptr = (ril_request_send_sms_expect_more_t*) malloc(sizeof(ril_request_send_sms_expect_more_t));
  ptr->SMSC_address = (char *)malloc(sizeof(ptr->SMSC_address));
  ptr->SMS_pdu_format = (char *)malloc(sizeof(ptr->SMS_pdu_format));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_send_sms_expect_more()
{
  ril_request_send_sms_expect_more_t* ptr = (ril_request_send_sms_expect_more_t*) payload;
  free(ptr->SMSC_address);
  free(ptr->SMS_pdu_format);
  free(ptr);
}

RIL_SMS_Response* ril_utf_androidTel_if::malloc_ril_request_send_sms_expect_more_resp()
{
  RIL_SMS_Response *ptr = (RIL_SMS_Response*) malloc(sizeof(RIL_SMS_Response));
  ptr->ackPDU = (char *)malloc(sizeof(ptr->ackPDU));
  return ptr;
}

void ril_utf_androidTel_if::free_ril_request_send_sms_expect_more_resp()
{
  RIL_SMS_Response* ptr = (RIL_SMS_Response*) payload;
  free(ptr->ackPDU);
  free(ptr);
}

int ril_utf_androidTel_if::compare_ril_request_send_sms_expect_more(ril_request_send_sms_expect_more_t *ptr1,ril_request_send_sms_expect_more_t *ptr2)
{
   if(memcmp(ptr1->SMSC_address, ptr2->SMSC_address, sizeof(ptr1->SMSC_address))!=0)
      return FALSE;
   else if(memcmp(ptr1->SMS_pdu_format, ptr2->SMS_pdu_format, sizeof(ptr1->SMS_pdu_format))!=0)
      return FALSE;
   else
      return TRUE;
}

int ril_utf_androidTel_if::compare_ril_request_send_sms_expect_more_resp(RIL_SMS_Response *ptr1, RIL_SMS_Response *ptr2)
{
  if(ptr1->messageRef != ptr2->messageRef)
      return FALSE;
  else if(memcmp(ptr1->ackPDU, ptr2->ackPDU, sizeof(ptr1->ackPDU))!=0)
      return FALSE;
  else if(ptr1->errorCode != ptr2-> errorCode)
      return FALSE;
  else
      return TRUE;
}

/*========================================================
  RIL_REQUEST_SMS_ACKNOWLEDGE
========================================================*/
int* ril_utf_androidTel_if::update_default_ril_request_sms_acknowledge(void)
{
  payload = malloc(sizeof(int)*2);
  payload_len = sizeof(int)*2;
  memset(payload, 0, sizeof(int)*2);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SMS_ACKNOWLEDGE;

  int* ptr = (int*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_sms_acknowledge_resp()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SMS_ACKNOWLEDGE;
}
ril_request_sms_acknowledge_t* ril_utf_androidTel_if::malloc_ril_request_sms_acknowledge()
{
  ril_request_sms_acknowledge_t* ptr = (ril_request_sms_acknowledge_t*) malloc(sizeof(ril_request_sms_acknowledge_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_sms_acknowledge()
{
  ril_request_sms_acknowledge_t* ptr = (ril_request_sms_acknowledge_t*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_sms_acknowledge(ril_request_sms_acknowledge_t *ptr1, ril_request_sms_acknowledge_t* ptr2)
{
  if(ptr1->ack_recieved != ptr2->ack_recieved)
      return FALSE;
  else if(ptr1->failure_cause != ptr2-> failure_cause)
      return FALSE;
  else
      return TRUE;
}

/*========================================================
  RIL_REQUEST_WRITE_SMS_TO_SIM
========================================================*/
RIL_SMS_WriteArgs* ril_utf_androidTel_if::update_default_ril_request_write_sms_to_sim(void)
{
  payload = malloc_ril_request_write_sms_to_sim();
  payload_len = sizeof(RIL_SMS_WriteArgs);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_WRITE_SMS_TO_SIM;

  RIL_SMS_WriteArgs* ptr = (RIL_SMS_WriteArgs*) payload;
  return ptr;
}
ril_request_write_sms_to_sim_resp_t* ril_utf_androidTel_if::update_default_ril_request_write_sms_to_sim_resp()
{
  payload = malloc_ril_request_write_sms_to_sim();
  payload_len = sizeof(ril_request_write_sms_to_sim_resp_t);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_WRITE_SMS_TO_SIM;

  ril_request_write_sms_to_sim_resp_t* ptr = (ril_request_write_sms_to_sim_resp_t*) payload;
  return ptr;
}
RIL_SMS_WriteArgs* ril_utf_androidTel_if::malloc_ril_request_write_sms_to_sim()
{
  RIL_SMS_WriteArgs* ptr = (RIL_SMS_WriteArgs*) malloc(sizeof(RIL_SMS_WriteArgs));
  ptr->pdu = (char *)malloc(WMS_MESSAGE_LENGTH_MAX_V01);
  memset(ptr->pdu, 0, WMS_MESSAGE_LENGTH_MAX_V01);
  ptr->smsc = (char *)malloc(SMSC_address_size);
  memset(ptr->smsc, 0, SMSC_address_size);
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_write_sms_to_sim()
{
  RIL_SMS_WriteArgs* ptr = (RIL_SMS_WriteArgs*) payload;
  free(ptr->pdu);
  free(ptr->smsc);
  free(ptr);
}
ril_request_write_sms_to_sim_resp_t* ril_utf_androidTel_if::malloc_ril_request_write_sms_to_sim_resp()
{
  ril_request_write_sms_to_sim_resp_t* ptr = (ril_request_write_sms_to_sim_resp_t*) malloc(sizeof(ril_request_write_sms_to_sim_resp_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_write_sms_to_sim_resp()
{
  ril_request_write_sms_to_sim_resp_t* ptr = (ril_request_write_sms_to_sim_resp_t*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_write_sms_to_sim(RIL_SMS_WriteArgs *ptr1, RIL_SMS_WriteArgs *ptr2)
{
  if(ptr1->status != ptr2->status)
      return FALSE;
  else if(memcpy(ptr1->pdu,ptr2->pdu,sizeof(ptr1->pdu))!=0)
      return FALSE;
  else if(memcpy(ptr1->smsc, ptr2->smsc, sizeof(ptr1->smsc))!=0)
      return FALSE;
  else
      return TRUE;
}
int ril_utf_androidTel_if::compare_ril_request_write_sms_to_sim_resp(ril_request_write_sms_to_sim_resp_t *ptr1, ril_request_write_sms_to_sim_resp_t *ptr2)
{
  if(ptr1->response != ptr2->response)
      return FALSE;
  else
      return TRUE;
}


/*========================================================
  RIL_REQUEST_WRITE_SMS_TO_SIM
========================================================*/

ril_request_delete_sms_on_sim_t* ril_utf_androidTel_if::update_default_ril_request_delete_sms_on_sim(int index)
{
  payload = malloc(sizeof(ril_request_delete_sms_on_sim_t));
  payload_len = sizeof(ril_request_delete_sms_on_sim_t);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_DELETE_SMS_ON_SIM;

  ril_request_delete_sms_on_sim_t* ptr = (ril_request_delete_sms_on_sim_t*) payload;
  if (ptr != nullptr)  ptr->record_index = index;
  return ptr;
}

void ril_utf_androidTel_if::update_default_ril_request_delete_sms_on_sim_resp(RIL_Errno err)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_DELETE_SMS_ON_SIM;
  error = err;
}

void ril_utf_androidTel_if::free_ril_request_delete_sms_on_sim()
{
  ril_request_delete_sms_on_sim_t* ptr = (ril_request_delete_sms_on_sim_t*) payload;
  free(ptr);
}

/*========================================================
  RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM
========================================================*/
ril_request_delete_sms_on_sim_t* ril_utf_androidTel_if::update_default_ril_request_delete_cdma_sms_on_ruim(int index)
{
  payload = malloc(sizeof(ril_request_delete_sms_on_sim_t));
  payload_len = sizeof(ril_request_delete_sms_on_sim_t);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM;

  ril_request_delete_sms_on_sim_t* ptr = (ril_request_delete_sms_on_sim_t*) payload;
  if (ptr != nullptr)  ptr->record_index = index;
  return ptr;
}

void ril_utf_androidTel_if::update_default_ril_request_delete_cdma_sms_on_ruim_resp(RIL_Errno err)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM;
  error = err;
}

void ril_utf_androidTel_if::free_ril_request_delete_cdma_sms_on_ruim()
{
  ril_request_delete_sms_on_sim_t* ptr = (ril_request_delete_sms_on_sim_t*) payload;
  free(ptr);
}

/*========================================================
    RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_response_sim_status_changed()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED;
}

/*========================================================
  RIL_REQUEST_GET_SIM_STATUS
========================================================*/
RIL_SIM_IO_v6* ril_utf_androidTel_if::update_default_ril_request_sim_io()
{
  payload = malloc(sizeof(RIL_SIM_IO_v6));
  payload_len = sizeof(RIL_SIM_IO_v6);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SIM_IO;

  return (RIL_SIM_IO_v6 *) payload;
}

 RIL_SIM_IO_Response* ril_utf_androidTel_if::update_default_ril_request_sim_io_resp()
{
  payload = malloc(sizeof( RIL_SIM_IO_Response));
  payload_len = sizeof( RIL_SIM_IO_Response);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SIM_IO;

  return ( RIL_SIM_IO_Response*) payload;

}

/*========================================================
  RIL_REQUEST_GET_SIM_STATUS
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_get_sim_status()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_GET_SIM_STATUS;
}
RIL_CardStatus_v6* ril_utf_androidTel_if::update_default_ril_request_get_sim_status_resp()
{
  payload = malloc(sizeof(RIL_CardStatus_v6));
  payload_len = sizeof(RIL_CardStatus_v6);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_GET_SIM_STATUS;

  return (RIL_CardStatus_v6*) payload;
}

/*========================================================
  RIL_REQUEST_SIM_OPEN_CHANNEL
========================================================*/
RIL_OpenChannelParams* ril_utf_androidTel_if::update_default_ril_request_open_logical_channel()
{
  payload =  malloc(sizeof(RIL_OpenChannelParams));
  payload_len = sizeof(RIL_OpenChannelParams);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SIM_OPEN_CHANNEL;

  return (RIL_OpenChannelParams*)payload;
}

uint8_t* ril_utf_androidTel_if::update_default_ril_request_open_logical_channel_resp(int payload_length)
{
  payload = malloc(payload_length);
  payload_len = payload_length;
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SIM_OPEN_CHANNEL;

  return (uint8_t*)payload;
}

/*========================================================
  RIL_REQUEST_ENTER_SIM_PIN
========================================================*/
char **ril_utf_androidTel_if::update_default_ril_request_enter_sim_pin()
{
   payload = malloc(2 * sizeof(char*));
   char ** ret = (char **) payload;
   payload_len = 2 * sizeof(char*);
   memset(payload, 0x0, payload_len);
   payload_type = ril_utf_ril_request;
   ret[0] = (char *)malloc(QMI_UIM_PIN_MAX_V01 + 1);
   memset(ret[0], 0x0, QMI_UIM_PIN_MAX_V01 + 1);
   ret[1] = (char *)malloc(2 * QMI_UIM_AID_MAX_V01 + 1);
   memset(ret[1], 0x0, 2 * QMI_UIM_AID_MAX_V01 + 1);
   msg_id = RIL_REQUEST_ENTER_SIM_PIN;
   return ret;
}
void ril_utf_androidTel_if::free_ril_request_enter_sim_pin()
{
   char ** request_enter_sim_pin_ptr = (char **) payload;
   free(request_enter_sim_pin_ptr[0]);
   free(request_enter_sim_pin_ptr[1]);
}
int *ril_utf_androidTel_if::update_default_ril_request_enter_sim_pin_resp()
{
   //TODO: discuss "error" flowing up to the testcase
   payload = malloc(sizeof(int));
   payload_len = sizeof(int);
   memset(payload, 0x0, payload_len);
   payload_type = ril_utf_ril_response;
   msg_id = RIL_REQUEST_ENTER_SIM_PIN;
   return (int *)payload;
}

/*========================================================
  RIL_REQUEST_ENTER_SIM_PUK
========================================================*/
char **ril_utf_androidTel_if::update_default_ril_request_enter_sim_puk()
{
   payload = malloc(3 * sizeof(char*));
   char ** ret = (char **) payload;
   payload_len = 3 * sizeof(char*);
   memset(payload, 0x0, payload_len);
   payload_type = ril_utf_ril_request;
   ret[0] = (char *)malloc(QMI_UIM_PIN_MAX_V01 + 1);//puk max and pin max are same
   memset(ret[0], 0x0, QMI_UIM_PIN_MAX_V01 + 1);
   ret[1] = (char *)malloc(QMI_UIM_PIN_MAX_V01 + 1);
   memset(ret[1], 0x0, QMI_UIM_PIN_MAX_V01 + 1);
   ret[2] = (char *)malloc(2 * QMI_UIM_AID_MAX_V01 + 1);
   memset(ret[2], 0x0, 2 * QMI_UIM_AID_MAX_V01 + 1);
   msg_id = RIL_REQUEST_ENTER_SIM_PUK;
   return ret;
}
void ril_utf_androidTel_if::free_ril_request_enter_sim_puk()
{
   char ** request_enter_sim_puk_ptr = (char **) payload;
   free(request_enter_sim_puk_ptr[0]);
   free(request_enter_sim_puk_ptr[1]);
   free(request_enter_sim_puk_ptr[2]);
}
int *ril_utf_androidTel_if::update_default_ril_request_enter_sim_puk_resp()
{
   //TODO: discuss "error" flowing up to the testcase
   payload = malloc(sizeof(int));
   payload_len = sizeof(int);
   memset(payload, 0x0, payload_len);
   payload_type = ril_utf_ril_response;
   msg_id = RIL_REQUEST_ENTER_SIM_PUK;
   return (int *)payload;
}


/*========================================================
  QCRIL_EVT_HOOK_ENTER_DEPERSONALIZATION_REQ
========================================================*/
char **ril_utf_androidTel_if::update_default_ril_request_enter_depersonalization_code()
{
   payload = malloc(2 * sizeof(char*));
   char ** ret = (char **) payload;
   payload_len = 2 * sizeof(char*);
   memset(payload, 0x0, payload_len);
   payload_type = ril_utf_ril_request;
   ret[0] = (char *)malloc(1);
   memset(ret[0], 0x0, 1);
   ret[1] = (char *)malloc(17); //max depersonalization code length is 16,
                        //and the message must be null-terminated -> 17
   memset(ret[1], 0x0, 17);
#ifdef QMI_RIL_HAL_UTF
  msg_id = QCRIL_REQ_HOOK_ENTER_DEPERSONALIZATION_REQ;
#else
  msg_id = QCRIL_EVT_HOOK_ENTER_DEPERSONALIZATION_REQ;
#endif
   return ret;
}
void ril_utf_androidTel_if::free_ril_request_enter_depersonalization_code()
{
   char ** request_enter_deperso_code_ptr = (char **) payload;
   free(request_enter_deperso_code_ptr[0]);
   free(request_enter_deperso_code_ptr[1]);
}
int *ril_utf_androidTel_if::update_default_ril_request_enter_depersonalization_code_resp()
{
   //TODO: discuss "error" flowing up to the testcase
   payload = malloc(sizeof(int));
   payload_len = sizeof(int);
   memset(payload, 0x0, payload_len);
   payload_type = ril_utf_ril_response;
#ifdef QMI_RIL_HAL_UTF
  msg_id = QCRIL_REQ_HOOK_ENTER_DEPERSONALIZATION_REQ;
#else
  msg_id = QCRIL_EVT_HOOK_ENTER_DEPERSONALIZATION_REQ;
#endif
   return (int *) payload;
}

/*========================================================
  RIL_REQUEST_GET_SMSC_ADDRESS
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_get_smsc_address()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_GET_SMSC_ADDRESS;

}
ril_request_get_smsc_address_resp_t* ril_utf_androidTel_if::update_default_ril_request_get_smsc_address_resp(char *smsc)
{
  payload = malloc_ril_request_get_smsc_address_resp();
  payload_len = sizeof(ril_request_get_smsc_address_resp_t) ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_GET_SMSC_ADDRESS;

  ril_request_get_smsc_address_resp_t* ptr = (ril_request_get_smsc_address_resp_t*) payload;
  strlcpy (ptr->SMSC_address, smsc, std::min(sizeof(ptr->SMSC_address), strlen(smsc)+1));
  return ptr;
}
ril_request_get_smsc_address_resp_t* ril_utf_androidTel_if::malloc_ril_request_get_smsc_address_resp()
{
  ril_request_get_smsc_address_resp_t* ptr = (ril_request_get_smsc_address_resp_t*) malloc(sizeof(ril_request_get_smsc_address_resp_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_get_smsc_address_resp()
{
  ril_request_get_smsc_address_resp_t* ptr = (ril_request_get_smsc_address_resp_t*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_get_smsc_address_resp(ril_request_get_smsc_address_resp_t *ptr1, ril_request_get_smsc_address_resp_t *ptr2)
{
  if(memcmp(ptr1->SMSC_address, ptr2->SMSC_address, sizeof(ptr1->SMSC_address)))
      return FALSE;
  else
      return TRUE;
}

/*========================================================
  RIL_REQUEST_SET_SMSC_ADDRESS
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_set_smsc_address(char *val)
{
  payload = (char *) malloc(WMS_ADDRESS_DIGIT_MAX_V01 + WMS_ADDRESS_TYPE_MAX_V01 + 6);
  payload_len =  WMS_ADDRESS_DIGIT_MAX_V01 + WMS_ADDRESS_TYPE_MAX_V01 + 6;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_SMSC_ADDRESS;

  strlcpy ((char *)payload, val, strlen (val) + 1);
}
void ril_utf_androidTel_if::update_default_ril_request_set_smsc_address_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_SMSC_ADDRESS;
}

/*========================================================
  RIL_REQUEST_REPORT_SMS_MEMORY_STATUS
========================================================*/

ril_request_report_sms_memory_status_t* ril_utf_androidTel_if::update_default_ril_request_report_sms_memory_status()
{
  payload = malloc_ril_request_report_sms_memory_status();
  payload_len = sizeof(ril_request_report_sms_memory_status_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_REPORT_SMS_MEMORY_STATUS;

  ril_request_report_sms_memory_status_t* ptr = (ril_request_report_sms_memory_status_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_report_sms_memory_status_resp()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_REPORT_SMS_MEMORY_STATUS;
}
ril_request_report_sms_memory_status_t* ril_utf_androidTel_if::malloc_ril_request_report_sms_memory_status()
{
  ril_request_report_sms_memory_status_t* ptr = (ril_request_report_sms_memory_status_t*) malloc(sizeof(ril_request_report_sms_memory_status_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_report_sms_memory_status()
{
  ril_request_report_sms_memory_status_t* ptr = (ril_request_report_sms_memory_status_t*) payload;
  free(ptr);
}

int ril_utf_androidTel_if::compare_ril_request_report_sms_memory_status(ril_request_report_sms_memory_status_t *ptr1, ril_request_report_sms_memory_status_t *ptr2)
{
  if(ptr1->sms_memory_status != ptr2->sms_memory_status)
      return FALSE;
  else
      return TRUE;
}
/*========================================================
  RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_gsm_get_broadcast_sms_config()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG;
}
RIL_GSM_BroadcastSmsConfigInfo** ril_utf_androidTel_if::update_default_ril_request_gsm_get_broadcast_sms_config_resp(int num)
{
  payload = malloc_ril_request_gsm_get_broadcast_sms_config_resp(num);
  payload_len = num*sizeof(RIL_GSM_BroadcastSmsConfigInfo*) ;
  RIL_GSM_BroadcastSmsConfigInfo** ptr = (RIL_GSM_BroadcastSmsConfigInfo**) payload;
  for(int i=0; i<num; i++) {
    ptr[i] = (RIL_GSM_BroadcastSmsConfigInfo *) malloc(sizeof(RIL_GSM_BroadcastSmsConfigInfo));
    memset(ptr[i], 0, sizeof(RIL_GSM_BroadcastSmsConfigInfo));
  }
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG;

  return ptr;
}
RIL_GSM_BroadcastSmsConfigInfo** ril_utf_androidTel_if::malloc_ril_request_gsm_get_broadcast_sms_config_resp(int num)
{
  RIL_GSM_BroadcastSmsConfigInfo** dptr = (RIL_GSM_BroadcastSmsConfigInfo **) malloc(num*sizeof(RIL_GSM_BroadcastSmsConfigInfo*));
  return dptr;
}
void ril_utf_androidTel_if::free_ril_request_gsm_get_broadcast_sms_config_resp()
{
  RIL_GSM_BroadcastSmsConfigInfo** dptr = (RIL_GSM_BroadcastSmsConfigInfo**) payload;
  free((*dptr));
}
int compare_ril_request_gsm_get_broadcast_sms_config(RIL_GSM_BroadcastSmsConfigInfo **ptr1, RIL_GSM_BroadcastSmsConfigInfo **ptr2)
{
  if( (*ptr1)->fromServiceId != (*ptr2)->fromServiceId )
    return FALSE;
  else if( (*ptr1)->toServiceId != (*ptr2)->toServiceId )
    return FALSE;
  else if( (*ptr1)->fromCodeScheme != (*ptr2)->fromCodeScheme )
    return FALSE;
  else if( (*ptr1)->toCodeScheme != (*ptr2)->toCodeScheme )
    return FALSE;
  else if( (*ptr1)->selected != (*ptr2)->selected )
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG
========================================================*/

RIL_GSM_BroadcastSmsConfigInfo** ril_utf_androidTel_if::update_default_ril_request_gsm_set_broadcast_sms_config(int num)
{
  payload = malloc_ril_request_gsm_set_broadcast_sms_config(num);
  payload_len = num*sizeof(RIL_GSM_BroadcastSmsConfigInfo*);
  RIL_GSM_BroadcastSmsConfigInfo** ptr = (RIL_GSM_BroadcastSmsConfigInfo**) payload;
  for(int i=0; i<num; i++) {
    ptr[i] = (RIL_GSM_BroadcastSmsConfigInfo *) malloc(sizeof(RIL_GSM_BroadcastSmsConfigInfo));
    memset(ptr[i], 0, sizeof(RIL_GSM_BroadcastSmsConfigInfo));
  }
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG;

  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_gsm_set_broadcast_sms_config_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG;
}
RIL_GSM_BroadcastSmsConfigInfo** ril_utf_androidTel_if::malloc_ril_request_gsm_set_broadcast_sms_config(int num)
{
  RIL_GSM_BroadcastSmsConfigInfo** dptr = (RIL_GSM_BroadcastSmsConfigInfo **) malloc(num*sizeof(RIL_GSM_BroadcastSmsConfigInfo*));
  return dptr;
}
void ril_utf_androidTel_if::free_ril_request_gsm_set_broadcast_sms_config()
{
  RIL_GSM_BroadcastSmsConfigInfo** dptr = (RIL_GSM_BroadcastSmsConfigInfo**) payload;
  free((*dptr));
}
int compare_ril_request_gsm_set_broadcast_sms_config(RIL_GSM_BroadcastSmsConfigInfo **ptr1, RIL_GSM_BroadcastSmsConfigInfo **ptr2)
{
  if( (*ptr1)->fromServiceId != (*ptr2)->fromServiceId )
    return FALSE;
  else if( (*ptr1)->toServiceId != (*ptr2)->toServiceId )
    return FALSE;
  else if( (*ptr1)->fromCodeScheme != (*ptr2)->fromCodeScheme )
    return FALSE;
  else if( (*ptr1)->toCodeScheme != (*ptr2)->toCodeScheme )
    return FALSE;
  else if( (*ptr1)->selected != (*ptr2)->selected )
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION
========================================================*/

ril_request_gsm_sms_broadcast_activation_t* ril_utf_androidTel_if::update_default_ril_request_gsm_sms_broadcast_activation()
{
  payload = malloc_ril_request_gsm_sms_broadcast_activation();
  payload_len = sizeof(ril_request_gsm_sms_broadcast_activation_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION;

  ril_request_gsm_sms_broadcast_activation_t* ptr = (ril_request_gsm_sms_broadcast_activation_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_gsm_sms_broadcast_activation_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION;
}
ril_request_gsm_sms_broadcast_activation_t* ril_utf_androidTel_if::malloc_ril_request_gsm_sms_broadcast_activation()
{
  ril_request_gsm_sms_broadcast_activation_t* ptr = (ril_request_gsm_sms_broadcast_activation_t*) malloc(sizeof(ril_request_gsm_sms_broadcast_activation_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_gsm_sms_broadcast_activation()
{
  ril_request_gsm_sms_broadcast_activation_t* ptr = (ril_request_gsm_sms_broadcast_activation_t*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_gsm_sms_broadcast_activation(ril_request_gsm_sms_broadcast_activation_t *ptr1, ril_request_gsm_sms_broadcast_activation_t *ptr2)
{
  if(ptr1->activation_state != ptr2->activation_state)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_CDMA_SEND_SMS
========================================================*/
RIL_CDMA_SMS_Message* ril_utf_androidTel_if::update_default_ril_request_cdma_send_sms()
{
  payload = malloc_ril_request_cdma_send_sms();
  payload_len = sizeof(RIL_CDMA_SMS_Message) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CDMA_SEND_SMS;

  RIL_CDMA_SMS_Message* ptr = (RIL_CDMA_SMS_Message*) payload;
  return ptr;
}
RIL_SMS_Response* ril_utf_androidTel_if::update_default_ril_request_cdma_send_sms_resp()
{
  payload = malloc_ril_request_cdma_send_sms_resp();
  payload_len = sizeof(RIL_SMS_Response) ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_SEND_SMS;

  RIL_SMS_Response* ptr = (RIL_SMS_Response*) payload;
  return ptr;
}
RIL_CDMA_SMS_Message* ril_utf_androidTel_if::malloc_ril_request_cdma_send_sms()
{
  RIL_CDMA_SMS_Message* ptr = (RIL_CDMA_SMS_Message*) malloc(sizeof(RIL_CDMA_SMS_Message));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_cdma_send_sms()
{
  RIL_CDMA_SMS_Message* ptr = (RIL_CDMA_SMS_Message*) payload;
  free(ptr);
}
RIL_SMS_Response* ril_utf_androidTel_if::malloc_ril_request_cdma_send_sms_resp()
{
  RIL_SMS_Response* ptr = (RIL_SMS_Response*) malloc(sizeof(RIL_SMS_Response));
  ptr->ackPDU = (char *) malloc(sizeof(ptr->ackPDU));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_cdma_send_sms_resp()
{
  RIL_SMS_Response* ptr = (RIL_SMS_Response*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_cdma_send_sms(RIL_CDMA_SMS_Message *ptr1, RIL_CDMA_SMS_Message *ptr2)
{
  if(ptr1!=ptr2)
    return FALSE;
  else
    return TRUE;
}
int ril_utf_androidTel_if::compare_ril_request_cdma_send_sms_resp(RIL_SMS_Response *ptr1, RIL_SMS_Response *ptr2)
{
  if(ptr1->messageRef != ptr2->messageRef)
    return FALSE;
  else if(memcmp(ptr1->ackPDU, ptr2->ackPDU, sizeof(ptr1->ackPDU))!=0)
    return FALSE;
  else if(ptr1->errorCode != ptr2->errorCode)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE
========================================================*/
RIL_CDMA_SMS_Ack* ril_utf_androidTel_if::update_default_ril_request_cdma_sms_acknowledge()
{
  payload = malloc_ril_request_cdma_sms_acknowledge();
  payload_len = sizeof(RIL_CDMA_SMS_Ack) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE;

  RIL_CDMA_SMS_Ack* ptr = (RIL_CDMA_SMS_Ack*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_cdma_sms_acknowledge_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE;
}
RIL_CDMA_SMS_Ack* ril_utf_androidTel_if::malloc_ril_request_cdma_sms_acknowledge()
{
  RIL_CDMA_SMS_Ack* ptr = (RIL_CDMA_SMS_Ack*) malloc(sizeof(RIL_CDMA_SMS_Ack));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_cdma_sms_acknowledge()
{
  RIL_CDMA_SMS_Ack* ptr = (RIL_CDMA_SMS_Ack*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_cdma_sms_acknowledge(RIL_CDMA_SMS_Ack *ptr1, RIL_CDMA_SMS_Ack *ptr2)
{
  if(ptr1!=ptr2)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM
========================================================*/
RIL_CDMA_SMS_WriteArgs* ril_utf_androidTel_if::update_default_ril_request_cdma_write_sms_to_ruim()
{
  payload = malloc_ril_request_cdma_write_sms_to_ruim();
  payload_len = sizeof(RIL_CDMA_SMS_WriteArgs) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM;

  RIL_CDMA_SMS_WriteArgs* ptr = (RIL_CDMA_SMS_WriteArgs*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_cdma_write_sms_to_ruim_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM;
}
RIL_CDMA_SMS_WriteArgs* ril_utf_androidTel_if::malloc_ril_request_cdma_write_sms_to_ruim()
{
  RIL_CDMA_SMS_WriteArgs* ptr = (RIL_CDMA_SMS_WriteArgs*) malloc(sizeof(RIL_CDMA_SMS_WriteArgs));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_cdma_write_sms_to_ruim()
{
  RIL_CDMA_SMS_WriteArgs* ptr = (RIL_CDMA_SMS_WriteArgs*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_cdma_write_sms_to_ruim(RIL_CDMA_SMS_WriteArgs *ptr1, RIL_CDMA_SMS_WriteArgs *ptr2)
{
  if (ptr1!=ptr2)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM
========================================================*/
ril_request_cdma_delete_sms_on_ruim_t* ril_utf_androidTel_if::update_default_ril_request_cdma_delete_sms_on_ruim_t()
{
  payload = malloc_ril_request_cdma_delete_sms_on_ruim();
  payload_len = sizeof(ril_request_cdma_delete_sms_on_ruim_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM;

  ril_request_cdma_delete_sms_on_ruim_t* ptr = (ril_request_cdma_delete_sms_on_ruim_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_cdma_delete_sms_on_ruim_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM;
}
ril_request_cdma_delete_sms_on_ruim_t* ril_utf_androidTel_if::malloc_ril_request_cdma_delete_sms_on_ruim()
{
  ril_request_cdma_delete_sms_on_ruim_t* ptr = (ril_request_cdma_delete_sms_on_ruim_t*) malloc(sizeof(RIL_CDMA_SMS_WriteArgs));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_cdma_delete_sms_on_ruim()
{
  ril_request_cdma_delete_sms_on_ruim_t* ptr = (ril_request_cdma_delete_sms_on_ruim_t*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_cdma_delete_sms_on_ruim(ril_request_cdma_delete_sms_on_ruim_t *ptr1, ril_request_cdma_delete_sms_on_ruim_t *ptr2)
{
  if (ptr1!=ptr2)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_cdma_get_broadcast_sms_config()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG;
}
RIL_CDMA_BroadcastSmsConfigInfo** ril_utf_androidTel_if::update_default_ril_request_cdma_get_broadcast_sms_config_resp(int num)
{
  payload = malloc_ril_request_cdma_get_broadcast_sms_config_resp(num);
  payload_len = num*sizeof(RIL_CDMA_BroadcastSmsConfigInfo*) ;
  RIL_CDMA_BroadcastSmsConfigInfo** ptr = (RIL_CDMA_BroadcastSmsConfigInfo**) payload;

  for(int i=0; i<num; i++) {
    ptr[i] = (RIL_CDMA_BroadcastSmsConfigInfo *) malloc(sizeof(RIL_CDMA_BroadcastSmsConfigInfo));
    memset(ptr[i], 0, sizeof(RIL_CDMA_BroadcastSmsConfigInfo));
  }
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG;

  return ptr;
}
RIL_CDMA_BroadcastSmsConfigInfo** ril_utf_androidTel_if::malloc_ril_request_cdma_get_broadcast_sms_config_resp(int num)
{
  RIL_CDMA_BroadcastSmsConfigInfo **dptr = (RIL_CDMA_BroadcastSmsConfigInfo**) malloc(num*sizeof(RIL_CDMA_BroadcastSmsConfigInfo*));
  return dptr;
}
void ril_utf_androidTel_if::free_ril_request_cdma_get_broadcast_sms_config_resp()
{
  RIL_CDMA_BroadcastSmsConfigInfo** dptr = (RIL_CDMA_BroadcastSmsConfigInfo**) payload;
  free((*dptr));
}
int ril_utf_androidTel_if::compare_ril_request_cdma_get_broadcast_sms_config_resp(RIL_CDMA_BroadcastSmsConfigInfo **ptr1, RIL_CDMA_BroadcastSmsConfigInfo **ptr2)
{
  if ((*ptr1)->service_category != (*ptr2)->service_category)
    return FALSE;
  else if ((*ptr1)->language != (*ptr2)->language)
    return FALSE;
  else if ((*ptr1)->selected != (*ptr2)->selected)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG
========================================================*/

RIL_CDMA_BroadcastSmsConfigInfo** ril_utf_androidTel_if::update_default_ril_request_cdma_set_broadcast_sms_config(int num)
{
  payload = malloc_ril_request_cdma_set_broadcast_sms_config(num);
  payload_len = num*sizeof(RIL_CDMA_BroadcastSmsConfigInfo*) ;
  RIL_CDMA_BroadcastSmsConfigInfo** ptr = (RIL_CDMA_BroadcastSmsConfigInfo**) payload;

  for(int i=0; i<num; i++) {
    ptr[i] = (RIL_CDMA_BroadcastSmsConfigInfo *) malloc(sizeof(RIL_CDMA_BroadcastSmsConfigInfo));
    memset(ptr[i], 0, sizeof(RIL_CDMA_BroadcastSmsConfigInfo));
  }
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG;

  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_cdma_set_broadcast_sms_config_resp()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG;
}
RIL_CDMA_BroadcastSmsConfigInfo** ril_utf_androidTel_if::malloc_ril_request_cdma_set_broadcast_sms_config(int num)
{
  RIL_CDMA_BroadcastSmsConfigInfo** ptr = (RIL_CDMA_BroadcastSmsConfigInfo**) malloc(num*sizeof(RIL_CDMA_BroadcastSmsConfigInfo*));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_cdma_set_broadcast_sms_config()
{
  RIL_CDMA_BroadcastSmsConfigInfo** ptr = (RIL_CDMA_BroadcastSmsConfigInfo**) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_cdma_set_broadcast_sms_config(RIL_CDMA_BroadcastSmsConfigInfo **ptr1, RIL_CDMA_BroadcastSmsConfigInfo **ptr2)
{
  if (ptr1!=ptr2)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION
========================================================*/

ril_request_cdma_sms_broadcast_activation_t* ril_utf_androidTel_if::update_default_ril_request_cdma_sms_broadcast_activation()
{
  payload = malloc_ril_request_cdma_sms_broadcast_activation();
  payload_len = sizeof(ril_request_cdma_sms_broadcast_activation_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION;

  ril_request_cdma_sms_broadcast_activation_t* ptr = (ril_request_cdma_sms_broadcast_activation_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_cdma_sms_broadcast_activation_resp()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION;
}
ril_request_cdma_sms_broadcast_activation_t* ril_utf_androidTel_if::malloc_ril_request_cdma_sms_broadcast_activation()
{
  ril_request_cdma_sms_broadcast_activation_t* ptr = (ril_request_cdma_sms_broadcast_activation_t*) malloc(sizeof(ril_request_cdma_sms_broadcast_activation_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_cdma_sms_broadcast_activation()
{
  ril_request_cdma_sms_broadcast_activation_t* ptr = (ril_request_cdma_sms_broadcast_activation_t*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_cdma_sms_broadcast_activation(ril_request_cdma_sms_broadcast_activation_t *ptr1, ril_request_cdma_sms_broadcast_activation_t *ptr2)
{
  if (ptr1!=ptr2)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_GET_CURRENT_CALLS
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_get_current_calls()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_GET_CURRENT_CALLS;
}
RIL_Call** ril_utf_androidTel_if::update_default_ril_request_get_current_calls_resp(int num_calls)
{
  if ( num_calls == 0 )
  {
    payload = NULL;
    payload_len = 0;
  }
  else
  {
    payload = malloc(sizeof(RIL_Call*) * num_calls);
    payload_len = sizeof(RIL_Call*)*num_calls;
  }
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_GET_CURRENT_CALLS;

  RIL_Call** dptr = (RIL_Call**) payload;
  int i;
  for (i = 0; i < num_calls; ++i)
  {
    dptr[i] = malloc_ril_request_get_current_calls_resp();
  }
  return dptr;
}
void ril_utf_androidTel_if::set_ril_request_get_current_calls_resp_number(int call_slot, char *number)
{
  RIL_Call** ptr = (RIL_Call**) payload;
  ptr[call_slot]->number = (char *) malloc(strlen(number)+1);
  strlcpy(ptr[call_slot]->number, number, strlen(number)+1);
}
void ril_utf_androidTel_if::set_ril_request_get_current_calls_resp_name(int call_slot, char *name)
{
  RIL_Call** ptr = (RIL_Call**) payload;
  ptr[call_slot]->name = (char *) malloc(strlen(name)+1);
  strlcpy(ptr[call_slot]->name, name, strlen(name)+1);
}
RIL_Call* ril_utf_androidTel_if::malloc_ril_request_get_current_calls_resp()
{
  RIL_Call* ptr = (RIL_Call*)malloc(sizeof(RIL_Call));
  memset(ptr, 0, sizeof(RIL_Call));
  return ptr;
}

/*========================================================
  RIL_REQUEST_DIAL
========================================================*/
RIL_Dial* ril_utf_androidTel_if::update_default_ril_request_dial(char *address)
{
  payload = malloc_ril_request_dial();
  payload_len = sizeof(RIL_Dial) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_DIAL;

  RIL_Dial* ptr = (RIL_Dial*) payload;
  ptr->address = (char *) malloc(strlen(address)+1);
  strcpy(ptr->address, address);
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_dial_resp()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_DIAL;
}
RIL_Dial* ril_utf_androidTel_if::malloc_ril_request_dial()
{
  RIL_Dial* ptr = (RIL_Dial*)malloc(sizeof(RIL_Dial));
  memset(ptr, 0, sizeof(RIL_Dial));
//  ptr->address = (char *)malloc(10);
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_dial()
{
  RIL_Dial* ptr = (RIL_Dial*) payload;
  free(ptr->address);
  free((ptr->uusInfo)->uusData);
  free(ptr->uusInfo);
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_requst_dial(RIL_Dial *ptr1, RIL_Dial *ptr2)
{
  if(memcmp(ptr1->address, ptr2->address , sizeof(ptr1->address))!=0)
    return FALSE;
  else if(ptr1->clir != ptr2->clir)
    return FALSE;
  else if( (ptr1->uusInfo)->uusType != (ptr2->uusInfo)->uusType )
    return FALSE;
  else if((ptr1->uusInfo)->uusDcs != (ptr2->uusInfo)->uusDcs )
    return FALSE;
  else if((ptr1->uusInfo)->uusDcs != (ptr2->uusInfo)->uusDcs )
    return FALSE;
  else if((ptr1->uusInfo)->uusDcs != (ptr2->uusInfo)->uusDcs )
    return FALSE;
  else if(memcmp((ptr1->uusInfo)->uusData , (ptr2->uusInfo)->uusData, sizeof((ptr1->uusInfo)->uusData))!=0)
    return FALSE;

  return TRUE;
}

/*========================================================
  RIL_REQUEST_HANGUP
========================================================*/
ril_request_hangup_t* ril_utf_androidTel_if::update_default_ril_request_hangup(int call_id)
{
  payload = malloc_ril_request_hangup();
  payload_len = sizeof(ril_request_hangup_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_HANGUP;

  ril_request_hangup_t* ptr = (ril_request_hangup_t*) payload;
  ptr->connection_index = call_id;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_hangup_resp()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_HANGUP;
}
ril_request_hangup_t* ril_utf_androidTel_if::malloc_ril_request_hangup()
{
  ril_request_hangup_t* ptr = (ril_request_hangup_t*)malloc(sizeof(ril_request_hangup_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_hangup()
{
  ril_request_hangup_t* ptr = (ril_request_hangup_t*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_hangup(ril_request_hangup_t *ptr1, ril_request_hangup_t *ptr2)
{
  if(ptr1->connection_index != ptr2->connection_index)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_LAST_CALL_FAIL_CAUSE
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_last_call_fail_cause()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_LAST_CALL_FAIL_CAUSE;
}

RIL_LastCallFailCauseInfo* ril_utf_androidTel_if::update_default_ril_request_last_call_fail_cause_resp()
{
  payload = malloc(sizeof(RIL_LastCallFailCauseInfo));
  memset(payload, 0, sizeof(RIL_LastCallFailCauseInfo));
  payload_len = sizeof(RIL_LastCallFailCauseInfo) ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_LAST_CALL_FAIL_CAUSE;

  RIL_LastCallFailCauseInfo* ptr = (RIL_LastCallFailCauseInfo*) payload;
  return ptr;
}

/*
ril_request_last_call_fail_cause_resp_t* ril_utf_androidTel_if::update_default_ril_request_last_call_fail_cause_resp()
{
  payload = malloc_ril_request_last_call_fail_cause_resp();
  payload_len = sizeof(ril_request_last_call_fail_cause_resp_t) ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_LAST_CALL_FAIL_CAUSE;

  ril_request_last_call_fail_cause_resp_t* ptr = (ril_request_last_call_fail_cause_resp_t*) payload;
  return ptr;
}
ril_request_last_call_fail_cause_resp_t* ril_utf_androidTel_if::malloc_ril_request_last_call_fail_cause_resp()
{
  ril_request_last_call_fail_cause_resp_t *ptr = (ril_request_last_call_fail_cause_resp_t*)malloc(sizeof(ril_request_last_call_fail_cause_resp_t));
  memset(ptr, 0, sizeof(ril_request_last_call_fail_cause_resp_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_last_call_fail_cause_resp()
{
  ril_request_last_call_fail_cause_resp_t* ptr = (ril_request_last_call_fail_cause_resp_t*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_last_call_fail_cause_resp(ril_request_last_call_fail_cause_resp_t *ptr1, ril_request_last_call_fail_cause_resp_t *ptr2)
{
  if(ptr1->failure_reason != ptr2->failure_reason)
    return FALSE;
  else
    return TRUE;
}
*/

/*========================================================
  RIL_REQUEST_ANSWER
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_answer()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_ANSWER;
}
void ril_utf_androidTel_if::update_default_ril_request_answer_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_ANSWER;
}

/*========================================================
  RIL_REQUEST_DTMF
========================================================*/
ril_request_dtmf_t* ril_utf_androidTel_if::update_default_ril_request_dtmf()
{
  payload = maloc_ril_request_dtmf();
  payload_len = sizeof(ril_request_dtmf_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_DTMF;

  ril_request_dtmf_t* ptr = (ril_request_dtmf_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_dtmf_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_DTMF;
}
ril_request_dtmf_t* ril_utf_androidTel_if::maloc_ril_request_dtmf()
{
  ril_request_dtmf_t *ptr = (ril_request_dtmf_t*)malloc(sizeof(ril_request_dtmf_t));
  ptr->dtmf = (char *)malloc(sizeof(char));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_dtmf()
{
  ril_request_dtmf_t* ptr = (ril_request_dtmf_t*) payload;
  free(ptr->dtmf);
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_dtmf(ril_request_dtmf_t *ptr1, ril_request_dtmf_t *ptr2)
{
  if(*(ptr1->dtmf) != *(ptr2->dtmf))
    return FALSE;
  else
    return TRUE;
}


/*========================================================
  RIL_REQUEST_DTMF_START
========================================================*/
ril_request_dtmf_t* ril_utf_androidTel_if::update_default_ril_request_dtmf_start()
{
  payload = maloc_ril_request_dtmf();
  payload_len = sizeof(ril_request_dtmf_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_DTMF_START;

  ril_request_dtmf_t* ptr = (ril_request_dtmf_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_dtmf_start_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_DTMF_START;
}
ril_request_dtmf_t* ril_utf_androidTel_if::malloc_ril_request_dtmf_start()
{
  ril_request_dtmf_t *ptr = (ril_request_dtmf_t*)malloc(sizeof(ril_request_dtmf_t));
  ptr->dtmf = (char *)malloc(sizeof(char));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_dtmf_start()
{
  ril_request_dtmf_t* ptr = (ril_request_dtmf_t*) payload;
  free(ptr->dtmf);
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_dtmf_start(ril_request_dtmf_t *ptr1, ril_request_dtmf_t *ptr2)
{
  if(*(ptr1->dtmf) != *(ptr2->dtmf))
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_DTMF_STOP
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_dtmf_stop()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_DTMF_STOP;
}
void ril_utf_androidTel_if::update_default_ril_request_dtmf_stop_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_DTMF_STOP;
}

/*========================================================
  RIL_REQUEST_CDMA_FLASH
========================================================*/
ril_request_cdma_flash_t* ril_utf_androidTel_if::update_default_ril_request_cdma_flash()
{
  payload = malloc_ril_request_cdma_flash();
  payload_len = sizeof(ril_request_cdma_flash_t);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CDMA_FLASH;

  ril_request_cdma_flash_t* ptr = (ril_request_cdma_flash_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_cdma_flash_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_FLASH;
}
ril_request_cdma_flash_t* ril_utf_androidTel_if::malloc_ril_request_cdma_flash()
{
  ril_request_cdma_flash_t *ptr = (ril_request_cdma_flash_t*)malloc(sizeof(ril_request_cdma_flash_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_cdma_flash()
{
  ril_request_cdma_flash_t* ptr = (ril_request_cdma_flash_t*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_cdma_flash(ril_request_cdma_flash_t *ptr1, ril_request_cdma_flash_t *ptr2)
{
  if(memcmp(ptr1->flash_string, ptr2->flash_string, sizeof(ptr1->flash_string))!=0)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_CDMA_BURST_DTMF
========================================================*/
ril_request_cdma_burst_dtmf_t* ril_utf_androidTel_if::update_default_ril_request_cdma_burst_dtmf()
{
  payload = malloc_ril_request_cdma_burst_dtmf();
  payload_len = sizeof(ril_request_cdma_burst_dtmf_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CDMA_BURST_DTMF;

  ril_request_cdma_burst_dtmf_t* ptr = (ril_request_cdma_burst_dtmf_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_cdma_burst_dtmf_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_BURST_DTMF;
}
ril_request_cdma_burst_dtmf_t* ril_utf_androidTel_if::malloc_ril_request_cdma_burst_dtmf()
{
  ril_request_cdma_burst_dtmf_t *ptr = (ril_request_cdma_burst_dtmf_t*)malloc(sizeof(ril_request_cdma_burst_dtmf_t));
//  ptr->DTMF_string = (char *) malloc(sizeof(char));
//  ptr->DTMF_ON_length = (char *) malloc(sizeof(char));
//  ptr->DTMF_OFF_length = (char *) malloc(sizeof(char));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_cdma_burst_dtmf()
{
  ril_request_cdma_burst_dtmf_t* ptr = (ril_request_cdma_burst_dtmf_t*) payload;
  free((ptr)->DTMF_string);
  free((ptr)->DTMF_ON_length);
  free((ptr)->DTMF_OFF_length);
  free((ptr));
}
int ril_utf_androidTel_if::compare_ril_request_cdma_burst_dtmf(ril_request_cdma_burst_dtmf_t **ptr1, ril_request_cdma_burst_dtmf_t **ptr2)
{
  if(memcmp((*ptr1)->DTMF_string,(*ptr2)->DTMF_string, sizeof((*ptr1)->DTMF_string)) !=0)
    return FALSE;
  else if(memcmp((*ptr1)->DTMF_ON_length,(*ptr2)->DTMF_ON_length, sizeof((*ptr1)->DTMF_ON_length)) !=0)
    return FALSE;
  else if(memcmp((*ptr1)->DTMF_OFF_length,(*ptr2)->DTMF_OFF_length, sizeof((*ptr1)->DTMF_OFF_length)) !=0)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_SET_TTY_MODE
========================================================*/
ril_request_set_tty_mode_t* ril_utf_androidTel_if::update_default_ril_request_set_tty_mode()
{
  payload = malloc_ril_request_set_tty_mode();
  payload_len = sizeof(ril_request_set_tty_mode_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_TTY_MODE;

  ril_request_set_tty_mode_t* ptr = (ril_request_set_tty_mode_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_set_tty_mode_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_TTY_MODE;
}
ril_request_set_tty_mode_t* ril_utf_androidTel_if::malloc_ril_request_set_tty_mode()
{
  ril_request_set_tty_mode_t *ptr = (ril_request_set_tty_mode_t*)malloc(sizeof(ril_request_set_tty_mode_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_set_tty_mode()
{
  ril_request_set_tty_mode_t* ptr = (ril_request_set_tty_mode_t*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_set_tty_mode(ril_request_set_tty_mode_t *ptr1, ril_request_set_tty_mode_t *ptr2)
{
  if(ptr1->tty_mode != ptr2->tty_mode)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_QUERY_TTY_MODE
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_get_tty_mode()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_QUERY_TTY_MODE;
}
ril_request_get_tty_mode_t *ril_utf_androidTel_if::update_default_ril_request_get_tty_mode_resp()
{
  payload = malloc_ril_request_get_tty_mode();
  payload_len = sizeof(ril_request_get_tty_mode_t);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_QUERY_TTY_MODE;
  ril_request_get_tty_mode_t *ptr = (ril_request_get_tty_mode_t *)payload;
  return ptr;
}
ril_request_get_tty_mode_t* ril_utf_androidTel_if::malloc_ril_request_get_tty_mode()
{
  ril_request_get_tty_mode_t *ptr = (ril_request_get_tty_mode_t*)malloc(sizeof(ril_request_get_tty_mode_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_get_tty_mode()
{
  ril_request_get_tty_mode_t* ptr = (ril_request_get_tty_mode_t*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_get_tty_mode(ril_request_get_tty_mode_t *ptr1, ril_request_get_tty_mode_t *ptr2)
{
  if(ptr1->tty_mode != ptr2->tty_mode)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE
========================================================*/
ril_request_cdma_set_preferred_voice_privacy_mode_t* ril_utf_androidTel_if::update_default_ril_request_cdma_set_preferred_voice_privacy_mode()
{
  payload = malloc_ril_request_cdma_set_preferred_voice_privacy_mode();
  payload_len = sizeof(ril_request_cdma_set_preferred_voice_privacy_mode_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE;

  ril_request_cdma_set_preferred_voice_privacy_mode_t* ptr = (ril_request_cdma_set_preferred_voice_privacy_mode_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_cdma_set_preferred_voice_privacy_mode_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE;
}
ril_request_cdma_set_preferred_voice_privacy_mode_t* ril_utf_androidTel_if::malloc_ril_request_cdma_set_preferred_voice_privacy_mode()
{
  ril_request_cdma_set_preferred_voice_privacy_mode_t *ptr = (ril_request_cdma_set_preferred_voice_privacy_mode_t*)malloc(sizeof(ril_request_cdma_set_preferred_voice_privacy_mode_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_cdma_set_preferred_voice_privacy_mode()
{
  ril_request_cdma_set_preferred_voice_privacy_mode_t* ptr = (ril_request_cdma_set_preferred_voice_privacy_mode_t*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_cdma_set_preferred_voice_privacy_mode(ril_request_cdma_set_preferred_voice_privacy_mode_t *ptr1, ril_request_cdma_set_preferred_voice_privacy_mode_t *ptr2)
{
  if(ptr1->voice_privacy_mode != ptr2->voice_privacy_mode)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_cdma_query_preferred_voice_privacy_mode()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE;
}
ril_request_cdma_query_preferred_voice_privacy_mode_t*
ril_utf_androidTel_if::update_default_ril_request_cdma_query_preferred_voice_privacy_mode_resp()
{
  payload = malloc_ril_request_cdma_query_preferred_voice_privacy_mode();
  payload_len = sizeof(ril_request_cdma_query_preferred_voice_privacy_mode_t) ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE;
  ril_request_cdma_query_preferred_voice_privacy_mode_t* ptr =
    (ril_request_cdma_query_preferred_voice_privacy_mode_t*) payload;
  return ptr;
}
ril_request_cdma_query_preferred_voice_privacy_mode_t*
ril_utf_androidTel_if::malloc_ril_request_cdma_query_preferred_voice_privacy_mode()
{
  ril_request_cdma_query_preferred_voice_privacy_mode_t *ptr = (ril_request_cdma_query_preferred_voice_privacy_mode_t*)malloc(sizeof(ril_request_cdma_query_preferred_voice_privacy_mode_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_cdma_query_preferred_voice_privacy_mode()
{
  ril_request_cdma_query_preferred_voice_privacy_mode_t* ptr = (ril_request_cdma_query_preferred_voice_privacy_mode_t*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_cdma_query_preferred_voice_privacy_mode(ril_request_cdma_query_preferred_voice_privacy_mode_t *ptr1, ril_request_cdma_query_preferred_voice_privacy_mode_t *ptr2)
{
  if(ptr1->voice_privacy_mode != ptr2->voice_privacy_mode)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_hangup_waiting_or_background()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND;
}
void ril_utf_androidTel_if::update_default_ril_request_hangup_waiting_or_background_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND;
}
/*========================================================
  RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_hangup_foreground_resume_background()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND;
}
void ril_utf_androidTel_if::update_default_ril_request_hangup_foreground_resume_background_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND;
}

/*========================================================
  RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_switch_waiting_or_holding_and_active()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE;
}
void ril_utf_androidTel_if::update_default_ril_request_switch_waiting_or_holding_and_active_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE;
}

/*========================================================
  RIL_REQUEST_CONFERENCE
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_conference()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CONFERENCE;
}
void ril_utf_androidTel_if::update_default_ril_request_conference_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CONFERENCE;
}

/*========================================================
  RIL_REQUEST_UDUB
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_udub()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_UDUB;
}
void ril_utf_androidTel_if::update_default_ril_request_udub_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_UDUB;
}

/*========================================================
  RIL_REQUEST_SEND_USSD
========================================================*/
ril_request_send_ussd_t* ril_utf_androidTel_if::update_default_ril_request_send_ussd(char *ussd)
{
  int len = strlen(ussd)+1;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SEND_USSD;

  payload = malloc(sizeof(char)*len);
  strlcpy((char *)payload, ussd, len);
  payload_len = len;

  ril_request_send_ussd_t* ptr = malloc_ril_request_send_ussd();
  ptr->ussd_request = (char *)payload;

  return ptr;
}

void ril_utf_androidTel_if::update_default_ril_request_send_ussd_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SEND_USSD;
}
ril_request_send_ussd_t* ril_utf_androidTel_if::malloc_ril_request_send_ussd()
{
  ril_request_send_ussd_t *ptr = (ril_request_send_ussd_t*)malloc(sizeof(ril_request_send_ussd_t));
  ptr->ussd_request = (char *)malloc(sizeof(char));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_send_ussd()
{
  ril_request_send_ussd_t* ptr = (ril_request_send_ussd_t*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_send_ussd(ril_request_send_ussd_t *ptr1, ril_request_send_ussd_t *ptr2)
{
  if(memcmp(ptr1->ussd_request, ptr2->ussd_request, sizeof(ptr1->ussd_request)))
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_CANCEL_USSD
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_cancel_ussd()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CANCEL_USSD;
}
void ril_utf_androidTel_if::update_default_ril_request_cancel_ussd_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CANCEL_USSD;
}
/*========================================================
  RIL_REQUEST_GET_CLIR
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_get_clir()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_GET_CLIR;
}
ril_request_get_clir_resp_t* ril_utf_androidTel_if::update_default_ril_request_get_clir_resp()
{
  payload = malloc_ril_request_get_clir_resp();
  payload_len = sizeof(ril_request_get_clir_resp_t) ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_GET_CLIR;

  ril_request_get_clir_resp_t* ptr = (ril_request_get_clir_resp_t*) payload;
  return ptr;
}
ril_request_get_clir_resp_t* ril_utf_androidTel_if::malloc_ril_request_get_clir_resp()
{
  ril_request_get_clir_resp_t *ptr = (ril_request_get_clir_resp_t*)malloc(sizeof(ril_request_get_clir_resp_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_get_clir_resp()
{
  ril_request_get_clir_resp_t* ptr = (ril_request_get_clir_resp_t*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_get_clir_resp(ril_request_get_clir_resp_t *ptr1, ril_request_get_clir_resp_t *ptr2)
{
  if(ptr1->n_parameter != ptr2->n_parameter)
    return FALSE;
  else if(ptr1->m_parameter != ptr2->m_parameter)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_SET_CLIR
========================================================*/
ril_request_set_clir_t* ril_utf_androidTel_if::update_default_ril_request_set_clir()
{
  payload = malloc_ril_request_set_clir();
  payload_len = sizeof(ril_request_set_clir_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_CLIR;

  ril_request_set_clir_t* ptr = (ril_request_set_clir_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_set_clir_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_CLIR;
}
ril_request_set_clir_t* ril_utf_androidTel_if::malloc_ril_request_set_clir()
{
  ril_request_set_clir_t *ptr = (ril_request_set_clir_t*)malloc(sizeof(ril_request_set_clir_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_set_clir()
{
  ril_request_set_clir_t* ptr = (ril_request_set_clir_t*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_set_clir(ril_request_set_clir_t *ptr1, ril_request_set_clir_t *ptr2)
{
  if(ptr1->n_parameter != ptr2->n_parameter)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE
========================================================*/
ril_request_unsol_cell_info_rate_t* ril_utf_androidTel_if::update_default_ril_request_set_unsol_cell_info_list_rate()
{
  payload = malloc(sizeof(ril_request_unsol_cell_info_rate_t));
  payload_len = sizeof(ril_request_unsol_cell_info_rate_t);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE;

  ril_request_unsol_cell_info_rate_t* ptr = (ril_request_unsol_cell_info_rate_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_set_unsol_cell_info_list_rate_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE;
}

/*========================================================
  RIL_REQUEST_SET_MUTE
========================================================*/
ril_request_mute_t* ril_utf_androidTel_if::update_default_ril_request_set_mute()
{
  payload = malloc(sizeof(ril_request_mute_t));
  payload_len = sizeof(ril_request_mute_t);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_MUTE;

  ril_request_mute_t* ptr = (ril_request_mute_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_set_mute_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_MUTE;
}

/*========================================================
  RIL_REQUEST_GET_MUTE
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_get_mute()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_GET_MUTE;
}
ril_request_mute_t* ril_utf_androidTel_if::update_default_ril_request_get_mute_resp()
{
  payload = malloc(sizeof(ril_request_mute_t));
  payload_len = sizeof(ril_request_mute_t);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_GET_MUTE;

  ril_request_mute_t* ptr = (ril_request_mute_t*) payload;
  return ptr;
}

/*========================================================
  RIL_REQUEST_QUERY_CALL_FORWARD_STATUS
========================================================*/

RIL_CallForwardInfo* ril_utf_androidTel_if::update_default_ril_request_query_call_forward_status()
{
  payload = malloc_ril_request_query_call_forward_status();
  payload_len = sizeof(RIL_CallForwardInfo) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_QUERY_CALL_FORWARD_STATUS;

  RIL_CallForwardInfo* ptr = (RIL_CallForwardInfo*) payload;
  return ptr;
}
RIL_CallForwardInfo** ril_utf_androidTel_if::update_default_ril_request_query_call_forward_status_resp(int num_info)
{
  if ( num_info == 0 )
  {
    payload = NULL;
    payload_len = 0;
  }
  else
  {
    payload = malloc(sizeof(RIL_CallForwardInfo*) * num_info);
    payload_len = sizeof(RIL_CallForwardInfo*)*num_info;
  }
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_QUERY_CALL_FORWARD_STATUS;

  RIL_CallForwardInfo** dptr = (RIL_CallForwardInfo**) payload;
  int i;
  for (i = 0; i < num_info; ++i)
  {
    dptr[i] = malloc_ril_request_query_call_forward_status_resp();
  }

  return dptr;
}
void ril_utf_androidTel_if::set_query_call_forward_status_resp_number(int slot, char *number)
{
  RIL_CallForwardInfo** dptr = (RIL_CallForwardInfo**) payload;
  dptr[slot]->number = (char *) malloc(strlen(number)+1);
  strlcpy(dptr[slot]->number, number, strlen(number)+1);
}
RIL_CallForwardInfo* ril_utf_androidTel_if::malloc_ril_request_query_call_forward_status()
{
  RIL_CallForwardInfo *ptr = (RIL_CallForwardInfo*)malloc(sizeof(RIL_CallForwardInfo));
  ptr->number = (char *)malloc(sizeof(char));
  *ptr->number = 0;
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_query_call_forward_status()
{
  RIL_CallForwardInfo* ptr = (RIL_CallForwardInfo*) payload;
  free(ptr->number);
  free(ptr);
}
RIL_CallForwardInfo* ril_utf_androidTel_if::malloc_ril_request_query_call_forward_status_resp()
{
  RIL_CallForwardInfo *ptr = (RIL_CallForwardInfo*)malloc(sizeof(RIL_CallForwardInfo));
  memset(ptr, 0, sizeof(RIL_CallForwardInfo));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_query_call_forward_status_resp()
{
  RIL_CallForwardInfo **dptr = (RIL_CallForwardInfo **) payload;
  free((*dptr)->number);
  free((*dptr));
}
int ril_utf_androidTel_if::compare_ril_request_query_call_forward_status(RIL_CallForwardInfo *ptr1, RIL_CallForwardInfo *ptr2)
{
  if(ptr1->status != ptr2->status)
    return FALSE;
  else if(ptr1->reason != ptr2->reason)
    return FALSE;
  else if(ptr1->serviceClass != ptr2->serviceClass)
    return FALSE;
  else if(ptr1->toa != ptr2->toa)
    return FALSE;
  else if(memcmp(ptr1->number, ptr2->number, sizeof(ptr1->number)) !=0)
    return FALSE;
  else if(ptr1->timeSeconds != ptr2->timeSeconds)
    return FALSE;
  else
    return TRUE;
}
int ril_utf_androidTel_if::compare_ril_request_query_call_forward_status_resp(RIL_CallForwardInfo **ptr1, RIL_CallForwardInfo **ptr2)
{
  if((*ptr1)->status != (*ptr2)->status)
    return FALSE;
  else if((*ptr1)->reason != (*ptr2)->reason)
    return FALSE;
  else if((*ptr1)->serviceClass != (*ptr2)->serviceClass)
    return FALSE;
  else if((*ptr1)->toa != (*ptr2)->toa)
    return FALSE;
  else if(memcmp((*ptr1)->number, (*ptr2)->number, sizeof((*ptr1)->number)) !=0)
    return FALSE;
  else if((*ptr1)->timeSeconds != (*ptr2)->timeSeconds)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_SET_CALL_FORWARD
========================================================*/
RIL_CallForwardInfo* ril_utf_androidTel_if::update_default_ril_request_set_call_forward()
{
  payload = malloc_ril_request_set_call_forward();
  payload_len = sizeof(RIL_CallForwardInfo) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_CALL_FORWARD;

  RIL_CallForwardInfo* ptr = (RIL_CallForwardInfo*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_set_call_forward_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_CALL_FORWARD;
}
RIL_CallForwardInfo* ril_utf_androidTel_if::malloc_ril_request_set_call_forward()
{
  RIL_CallForwardInfo *ptr = (RIL_CallForwardInfo*)malloc(sizeof(RIL_CallForwardInfo));
  memset(ptr, 0, sizeof(RIL_CallForwardInfo));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_set_call_forward()
{
  RIL_CallForwardInfo* ptr = (RIL_CallForwardInfo*) payload;
  free(ptr->number);
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_set_call_forward(RIL_CallForwardInfo *ptr1, RIL_CallForwardInfo *ptr2)
{
  if(ptr1->status != ptr2->status)
    return FALSE;
  else if(ptr1->reason != ptr2->reason)
    return FALSE;
  else if(ptr1->serviceClass != ptr2->serviceClass)
    return FALSE;
  else if(ptr1->toa != ptr2->toa)
    return FALSE;
  else if(memcmp(ptr1->number, ptr2->number, sizeof(ptr1->number))!=0)
    return FALSE;
  else if(ptr1->timeSeconds != ptr2->timeSeconds)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_QUERY_CALL_WAITING
========================================================*/
ril_request_query_call_waiting_t* ril_utf_androidTel_if::update_default_ril_request_query_call_waiting()
{
  payload = malloc_ril_request_query_call_waiting();
  payload_len = sizeof(ril_request_query_call_waiting_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_QUERY_CALL_WAITING;

  ril_request_query_call_waiting_t* ptr = (ril_request_query_call_waiting_t*) payload;
  return ptr;
}
ril_request_query_call_waiting_resp_t* ril_utf_androidTel_if::update_default_ril_request_query_call_waiting_resp()
{
  payload = malloc_ril_request_query_call_waiting_resp();
  payload_len = sizeof(ril_request_query_call_waiting_resp_t) ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_QUERY_CALL_WAITING;

  ril_request_query_call_waiting_resp_t* ptr = (ril_request_query_call_waiting_resp_t*) payload;
  return ptr;
}
ril_request_query_call_waiting_t* ril_utf_androidTel_if::malloc_ril_request_query_call_waiting()
{
  ril_request_query_call_waiting_t *ptr = (ril_request_query_call_waiting_t*)malloc(sizeof(ril_request_query_call_waiting_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_query_call_waiting()
{
  ril_request_query_call_waiting_t *ptr = (ril_request_query_call_waiting_t *) payload;
  free(ptr);
}
ril_request_query_call_waiting_resp_t* ril_utf_androidTel_if::malloc_ril_request_query_call_waiting_resp()
{
  ril_request_query_call_waiting_resp_t *ptr = (ril_request_query_call_waiting_resp_t*)malloc(sizeof(ril_request_query_call_waiting_resp_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_query_call_waiting_resp()
{
  ril_request_query_call_waiting_resp_t *ptr = (ril_request_query_call_waiting_resp_t *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_query_call_waiting(ril_request_query_call_waiting_t *ptr1, ril_request_query_call_waiting_t *ptr2)
{
  if(ptr1->service_class != ptr2->service_class)
    return FALSE;
  else
    return TRUE;
}
int ril_utf_androidTel_if::compare_ril_request_query_call_waiting_resp(ril_request_query_call_waiting_resp_t *ptr1, ril_request_query_call_waiting_resp_t *ptr2)
{
  if(ptr1->status != ptr2->status)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_SET_CALL_WAITING
========================================================*/
ril_request_set_call_waiting_t* ril_utf_androidTel_if::update_default_ril_request_set_call_waiting()
{
  payload = malloc_ril_request_set_call_waiting();
  payload_len = sizeof(ril_request_set_call_waiting_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_CALL_WAITING;

  ril_request_set_call_waiting_t* ptr = (ril_request_set_call_waiting_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_set_call_waiting_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_CALL_WAITING;
}
ril_request_set_call_waiting_t* ril_utf_androidTel_if::malloc_ril_request_set_call_waiting()
{
  ril_request_set_call_waiting_t *ptr = (ril_request_set_call_waiting_t*)malloc(sizeof(ril_request_set_call_waiting_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_set_call_waiting()
{
  ril_request_set_call_waiting_t *ptr = (ril_request_set_call_waiting_t *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_set_call_waiting(ril_request_set_call_waiting_t *ptr1, ril_request_set_call_waiting_t *ptr2)
{
  if(ptr1->status != ptr2->status)
    return FALSE;
  else if(ptr1->service_class != ptr2->service_class)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_QUERY_FACILITY_LOCK
========================================================*/
char** ril_utf_androidTel_if::update_default_ril_request_query_facility_lock()
{
  payload = malloc_ril_request_query_facility_lock();
  payload_len = sizeof(char*)*4;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_QUERY_FACILITY_LOCK;

  char **ptr = (char**) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_query_facility_lock_resp(int service_class)
{
  payload = malloc(sizeof(int));
  payload_len = sizeof(int);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_QUERY_FACILITY_LOCK;

  int *ptr = (int*) payload;

  *ptr = service_class;

}
char** ril_utf_androidTel_if::malloc_ril_request_query_facility_lock()
{
  char **ptr = (char**)malloc(sizeof(char*)*4);
  ptr[0] = (char *)malloc(10);
  ptr[1] = (char *)malloc(10);
  ptr[2] = (char *)malloc(40);
  ptr[3] = NULL;
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_query_facility_lock()
{
  ril_request_query_facility_lock_t **dptr = (ril_request_query_facility_lock_t **) payload;
  free((*dptr)->facility_string_code);
  free((*dptr)->password);
  free((*dptr)->service_class_bit_vector);
  free((*dptr)->AID_value);
  free((*dptr));
}
int ril_utf_androidTel_if::compare_ril_request_query_facility_lock(ril_request_query_facility_lock_t **ptr1, ril_request_query_facility_lock_t **ptr2)
{
  if(memcmp((*ptr1)->facility_string_code,(*ptr2)->facility_string_code, sizeof((*ptr1)->facility_string_code)) !=0)
    return FALSE;
  else if(memcmp((*ptr1)->password,(*ptr2)->password, sizeof((*ptr1)->password)) !=0)
    return FALSE;
  else if(memcmp((*ptr1)->service_class_bit_vector,(*ptr2)->service_class_bit_vector, sizeof((*ptr1)->service_class_bit_vector)) !=0)
    return FALSE;
  else if(memcmp((*ptr1)->AID_value,(*ptr2)->AID_value, sizeof((*ptr1)->AID_value)) !=0)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_SET_FACILITY_LOCK
========================================================*/
char** ril_utf_androidTel_if::update_default_ril_request_set_facility_lock()
{
  payload = malloc_ril_request_set_facility_lock();
  payload_len = sizeof(char*)*5 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_FACILITY_LOCK;

  char** ptr = (char**) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_set_facility_lock_resp()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_FACILITY_LOCK;
}
char** ril_utf_androidTel_if::malloc_ril_request_set_facility_lock()
{
  char **ptr = (char**)malloc(sizeof(char*)*5);
  ptr[0] = (char *)malloc(10);
  ptr[1] = (char *)malloc(10);
  ptr[2] = (char *)malloc(40);
  ptr[3] = (char *)malloc(40);
  ptr[4] = NULL;
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_set_facility_lock()
{
  ril_request_set_facility_lock_t **dptr = (ril_request_set_facility_lock_t **) payload;
  free((*dptr)->facility_string_code);
  free((*dptr)->lock_status);
  free((*dptr)->password);
  free((*dptr)->service_class_bit_vector);
  free((*dptr)->AID_value);
  free((*dptr));
}
int ril_utf_androidTel_if::compare_ril_request_set_facility_lock(ril_request_set_facility_lock_t **ptr1, ril_request_set_facility_lock_t **ptr2)
{
  if(memcmp((*ptr1)->facility_string_code,(*ptr2)->facility_string_code, sizeof((*ptr1)->facility_string_code)) !=0)
    return FALSE;
  else if(memcmp((*ptr1)->lock_status,(*ptr2)->lock_status, sizeof((*ptr1)->lock_status)) !=0)
    return FALSE;
  else if(memcmp((*ptr1)->password,(*ptr2)->password, sizeof((*ptr1)->password)) !=0)
    return FALSE;
  else if(memcmp((*ptr1)->service_class_bit_vector,(*ptr2)->service_class_bit_vector, sizeof((*ptr1)->service_class_bit_vector)) !=0)
    return FALSE;
  else if(memcmp((*ptr1)->AID_value,(*ptr2)->AID_value, sizeof((*ptr1)->AID_value)) !=0)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_CHANGE_BARRING_PASSWORD
========================================================*/
ril_request_change_barring_password_t* ril_utf_androidTel_if::update_default_ril_request_change_barring_password()
{
  payload = malloc_ril_request_change_barring_password();
  payload_len = sizeof(ril_request_change_barring_password_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CHANGE_BARRING_PASSWORD;

  ril_request_change_barring_password_t* ptr = (ril_request_change_barring_password_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_change_barring_password_resp()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CHANGE_BARRING_PASSWORD;
}
ril_request_change_barring_password_t* ril_utf_androidTel_if::malloc_ril_request_change_barring_password()
{
  ril_request_change_barring_password_t *ptr = (ril_request_change_barring_password_t *)malloc(sizeof(ril_request_change_barring_password_t));
//  ptr->facility_string_code = (char *)malloc(sizeof(char));
//  ptr->old_password = (char *)malloc(sizeof(char));
//  ptr->new_password = (char *)malloc(sizeof(char));
//  ril_request_change_barring_password_t **dptr = &ptr;
//  return dptr;
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_change_barring_password()
{
  ril_request_change_barring_password_t *dptr = (ril_request_change_barring_password_t *) payload;
  free((dptr)->facility_string_code);
  free((dptr)->old_password);
  free((dptr)->new_password);
  free((dptr));
}
int ril_utf_androidTel_if::compare_ril_request_change_barring_password(ril_request_change_barring_password_t **ptr1, ril_request_change_barring_password_t **ptr2)
{
  if(memcmp((*ptr1)->facility_string_code,(*ptr2)->facility_string_code, sizeof((*ptr1)->facility_string_code)) !=0)
    return FALSE;
  else if(memcmp((*ptr1)->old_password,(*ptr2)->old_password, sizeof((*ptr1)->old_password)) !=0)
    return FALSE;
  else if(memcmp((*ptr1)->new_password,(*ptr2)->new_password, sizeof((*ptr1)->new_password)) !=0)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_SEPARATE_CONNECTION
========================================================*/
ril_request_separate_connection_t* ril_utf_androidTel_if::update_default_ril_request_separate_connection(int call_id)
{
  payload = malloc_ril_request_separate_connection();
  payload_len = sizeof(ril_request_separate_connection_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SEPARATE_CONNECTION;

  ril_request_separate_connection_t* ptr = (ril_request_separate_connection_t*) payload;
  ptr->connection_index = call_id;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_separate_connection_resp()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SEPARATE_CONNECTION;
}
ril_request_separate_connection_t* ril_utf_androidTel_if::malloc_ril_request_separate_connection()
{
  ril_request_separate_connection_t *ptr = (ril_request_separate_connection_t *)malloc(sizeof(ril_request_separate_connection_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_separate_connection()
{
  ril_request_separate_connection_t *ptr = (ril_request_separate_connection_t *) payload;
  free(ptr);
}
int compare_ril_request_separate_connection(ril_request_separate_connection_t *ptr1, ril_request_separate_connection_t *ptr2)
{
  if(ptr1->connection_index != ptr2->connection_index )
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_QUERY_CLIP
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_query_clip()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_QUERY_CLIP;
}
void ril_utf_androidTel_if::update_default_ril_request_query_clip_resp(int clip_status)
{
  payload = malloc(sizeof(int));
  payload_len = sizeof(int);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_QUERY_CLIP;

  int *ptr = (int*) payload;
  *ptr = clip_status;
}

/*========================================================
  RIL_REQUEST_GET_CELL_INFO_LIST
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_get_cell_info_list()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_GET_CELL_INFO_LIST;
}
RIL_CellInfo_v12 *ril_utf_androidTel_if::update_default_ril_request_get_cell_info_list_resp(size_t len)
{
  payload_len = sizeof(RIL_CellInfo_v12)*len;
  payload = malloc(payload_len);
  memset(payload, 0x0, payload_len);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_GET_CELL_INFO_LIST;

  RIL_CellInfo_v12 *ptr = (RIL_CellInfo_v12*) payload;
  return ptr;
}

/*========================================================
  RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION
========================================================*/
ril_request_set_supp_svc_notification_t* ril_utf_androidTel_if::update_default_ril_request_set_supp_svc_notification()
{
  payload = malloc_ril_request_set_supp_svc_notification();
  payload_len = sizeof(ril_request_set_supp_svc_notification_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION;

  ril_request_set_supp_svc_notification_t* ptr = (ril_request_set_supp_svc_notification_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_set_supp_svc_notification_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION;
}
ril_request_set_supp_svc_notification_t* ril_utf_androidTel_if::malloc_ril_request_set_supp_svc_notification()
{
  ril_request_set_supp_svc_notification_t *ptr = (ril_request_set_supp_svc_notification_t *)malloc(sizeof(ril_request_set_supp_svc_notification_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_set_supp_svc_notification()
{
  ril_request_set_supp_svc_notification_t *ptr = (ril_request_set_supp_svc_notification_t *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_set_supp_svc_notification(ril_request_set_supp_svc_notification_t *ptr1, ril_request_set_supp_svc_notification_t *ptr2)
{
  if(ptr1->notification_status != ptr2->notification_status )
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_EXPLICIT_CALL_TRANSFER
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_explicit_call_transfer()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_EXPLICIT_CALL_TRANSFER;
}
void ril_utf_androidTel_if::update_default_ril_request_explicit_call_transfer_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_EXPLICIT_CALL_TRANSFER;
}
/*========================================================
  RIL_REQUEST_RADIO_POWER
========================================================*/
/*========================================================
  RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_query_network_selection_mode()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE;
}
ril_request_query_network_selection_mode_resp_t* ril_utf_androidTel_if::update_default_ril_request_query_network_selection_mode_resp()
{
  payload = malloc_ril_request_query_network_selection_mode_resp();
  payload_len = sizeof(ril_request_query_network_selection_mode_resp_t) ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE;

  ril_request_query_network_selection_mode_resp_t* ptr = (ril_request_query_network_selection_mode_resp_t*) payload;
  return ptr;
}
ril_request_query_network_selection_mode_resp_t* ril_utf_androidTel_if::malloc_ril_request_query_network_selection_mode_resp()
{
  ril_request_query_network_selection_mode_resp_t *ptr = (ril_request_query_network_selection_mode_resp_t *)malloc(sizeof(ril_request_query_network_selection_mode_resp_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_query_network_selection_mode_resp()
{
  ril_request_query_network_selection_mode_resp_t *ptr = (ril_request_query_network_selection_mode_resp_t *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_query_network_selection_mode_resp(ril_request_query_network_selection_mode_resp_t *ptr1, ril_request_query_network_selection_mode_resp_t *ptr2)
{
  if(ptr1->network_selection_mode != ptr2->network_selection_mode)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_set_network_selection_automatic()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC;
}
void ril_utf_androidTel_if::update_default_ril_request_set_network_selection_automatic_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC;
}
/*========================================================
  RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL
========================================================*/
//ril_request_set_network_selection_manual_t* ril_utf_androidTel_if::update_default_ril_request_set_network_selection_manual();
void ril_utf_androidTel_if::update_default_ril_request_set_network_selection_manual(int is_legacy, int is_combine)
{
  if(is_legacy)
  {
    payload = malloc_ril_request_set_network_selection_manual_in_legacy_case();
    payload_len = sizeof(ril_request_set_network_selection_manual_t);
  }
  else
  {
    payload = malloc_ril_request_set_network_selection_manual(is_combine);
    payload_len = strlen((char *)payload);
  }
  //payload_len = sizeof(ril_request_set_network_selection_manual_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL;

  //ril_request_set_network_selection_manual_t* ptr = (ril_request_set_network_selection_manual_t*) payload;
  //return ptr;
}
void ril_utf_androidTel_if::set_ril_request_set_network_selection_manual(int is_legacy, int is_combine, char *mccmnc, char *rat)
{
  if(is_legacy)
  {
    ril_request_set_network_selection_manual_t* ptr = (ril_request_set_network_selection_manual_t*) payload;
    strcpy(ptr->MCCMNC, mccmnc);
    strcpy(ptr->rat, rat);
  }
  else
  {
    char *ptr = (char *) payload;
    strcpy(ptr,mccmnc);
    if(is_combine)
    {
       strcat(ptr,"+");
       strcat(ptr,rat);
    }
  }
}
void ril_utf_androidTel_if::update_default_ril_request_set_network_selection_manual_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL;
}
ril_request_set_network_selection_manual_t* ril_utf_androidTel_if::malloc_ril_request_set_network_selection_manual_in_legacy_case()
{
  ril_request_set_network_selection_manual_t *ptr = (ril_request_set_network_selection_manual_t *)malloc(sizeof(ril_request_set_network_selection_manual_t));
  bzero(ptr, sizeof(ril_request_set_network_selection_manual_t));
  ptr->MCCMNC = (char *)malloc(sizeof(char)*7);
  bzero(ptr->MCCMNC, (sizeof(char)*7));
  ptr->rat = (char *)malloc(sizeof(char)*4);
  bzero(ptr->rat, (sizeof(char)*4));
  return ptr;
}
char *ril_utf_androidTel_if::malloc_ril_request_set_network_selection_manual(int is_combine)
{
  ril_request_set_network_selection_manual_t *ptr = (ril_request_set_network_selection_manual_t *)malloc(sizeof(ril_request_set_network_selection_manual_t));
  bzero(ptr, sizeof(ril_request_set_network_selection_manual_t));

  if(is_combine)
  {
    ptr->MCCMNC = (char *)malloc(sizeof(char)*12);
    bzero(ptr->MCCMNC, (sizeof(char)*12));
  }
  else
  {
    ptr->MCCMNC = (char *)malloc(sizeof(char)*7);
    bzero(ptr->MCCMNC, (sizeof(char)*7));
  }
  return ptr->MCCMNC;
}
void ril_utf_androidTel_if::free_ril_request_set_network_selection_manual()
{
  ril_request_set_network_selection_manual_t *ptr = (ril_request_set_network_selection_manual_t *) payload;
  free(ptr->MCCMNC);
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_set_network_selection_manual(ril_request_set_network_selection_manual_t *ptr1, ril_request_set_network_selection_manual_t *ptr2)
{
  if(memcmp(ptr1->MCCMNC, ptr2->MCCMNC , sizeof(ptr1->MCCMNC))!=0)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_RESET_RADIO
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_reset_radio()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_RESET_RADIO;
}
void ril_utf_androidTel_if::update_default_ril_request_reset_radio_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_RESET_RADIO;
}

/*========================================================
  RIL_REQUEST_DEVICE_IDENTITY
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_device_identity_req()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_DEVICE_IDENTITY;
}
ril_request_device_identity_type_t* ril_utf_androidTel_if::update_default_ril_request_device_identity_resp()
{
  payload = malloc(sizeof(ril_request_device_identity_type_t));
  payload_len = sizeof(ril_request_device_identity_type_t);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_DEVICE_IDENTITY;
  return (ril_request_device_identity_type_t*)payload;
}

/*========================================================
  RIL_REQUEST_BASEBAND_VERSION
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_baseband_version_req()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_BASEBAND_VERSION;
}
ril_request_get_baseband_version_type_t*
    ril_utf_androidTel_if::update_default_ril_request_baseband_version_resp()
{
  payload = malloc(sizeof(ril_request_get_baseband_version_type_t));
  payload_len = sizeof(ril_request_get_baseband_version_type_t);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_BASEBAND_VERSION;
  return (ril_request_get_baseband_version_type_t*)payload;
}

/*========================================================
  RIL_REQUEST_SET_BAND_MODE
========================================================*/
ril_request_set_band_mode_t* ril_utf_androidTel_if::update_default_ril_request_set_band_mode()
{
  payload = malloc_ril_request_set_band_mode();
  payload_len = sizeof(ril_request_set_band_mode_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_BAND_MODE;

  ril_request_set_band_mode_t* ptr = (ril_request_set_band_mode_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_set_band_mode_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_BAND_MODE;
}
ril_request_set_band_mode_t* ril_utf_androidTel_if::malloc_ril_request_set_band_mode()
{
  ril_request_set_band_mode_t *ptr = (ril_request_set_band_mode_t *)malloc(sizeof(ril_request_set_band_mode_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_set_band_mode()
{
  ril_request_set_band_mode_t *ptr = (ril_request_set_band_mode_t *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_set_band_mode(ril_request_set_band_mode_t *ptr1, ril_request_set_band_mode_t *ptr2)
{
  if(ptr1->band_mode != ptr2->band_mode)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_query_available_band_mode()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE;
}
int* ril_utf_androidTel_if::update_Default_ril_request_query_available_band_mode_resp()
{
  payload = malloc_ril_request_query_available_band_mode_resp();
  payload_len =  sizeof(int) * MAX_AVAILABLE_BANDS;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE;

  int* ptr = (int*) payload;
  return ptr;
}
int * ril_utf_androidTel_if::malloc_ril_request_query_available_band_mode_resp()
{
  int *ptr = (int *)malloc(sizeof(int) * MAX_AVAILABLE_BANDS);
  memset(ptr, 0, MAX_AVAILABLE_BANDS);
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_query_available_band_mode_resp()
{
  int *ptr = (int*) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_query_available_band_mode_resp(ril_request_query_available_band_mode_resp_t *ptr1, ril_request_query_available_band_mode_resp_t *ptr2)
{
  if(ptr1->band_mode != ptr2->band_mode)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE
========================================================*/
RIL_PreferredNetworkType* ril_utf_androidTel_if::update_default_ril_request_set_preferred_network_type()
{
  payload = malloc(sizeof(int));
  payload_len = sizeof(int) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE;

  RIL_PreferredNetworkType* ptr = (RIL_PreferredNetworkType*) payload;

  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_set_preferred_network_type_resp()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE;
}

/*========================================================
  RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_get_preferred_network_type()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE;
}
RIL_PreferredNetworkType* ril_utf_androidTel_if::update_default_ril_request_get_preferred_network_type_resp()
{
  payload = malloc(sizeof(int));
  payload_len = sizeof(int) ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE;

  RIL_PreferredNetworkType* ptr = (RIL_PreferredNetworkType*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_get_preferred_network_type_resp_failure()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE;
}
RIL_PreferredNetworkType* ril_utf_androidTel_if::malloc_ril_request_get_preferred_network_type_resp()
{
  RIL_PreferredNetworkType *ptr = (RIL_PreferredNetworkType *)malloc(sizeof(RIL_PreferredNetworkType));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_get_preferred_network_type_resp()
{
  RIL_PreferredNetworkType *ptr = (RIL_PreferredNetworkType *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_get_preferred_network_type_resp(RIL_PreferredNetworkType *ptr1, RIL_PreferredNetworkType *ptr2)
{
  if(ptr1!=ptr2)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE
========================================================*/
RIL_CdmaSubscriptionSource* ril_utf_androidTel_if::update_default_ril_request_cdma_set_subscription_source()
{
  payload = malloc_ril_request_cdma_set_subscription_source();
  payload_len = sizeof(RIL_CdmaSubscriptionSource) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE;

  RIL_CdmaSubscriptionSource* ptr = (RIL_CdmaSubscriptionSource*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_cdma_set_subscription_source_resp()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE;
}
RIL_CdmaSubscriptionSource* ril_utf_androidTel_if::malloc_ril_request_cdma_set_subscription_source()
{
  RIL_CdmaSubscriptionSource *ptr = (RIL_CdmaSubscriptionSource *)malloc(sizeof(RIL_CdmaSubscriptionSource));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_cdma_set_subscription_source()
{
  RIL_CdmaSubscriptionSource *ptr = (RIL_CdmaSubscriptionSource *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_cdma_set_subscription_source(RIL_CdmaSubscriptionSource *ptr1, RIL_CdmaSubscriptionSource *ptr2)
{
  if(ptr1!=ptr2)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_cdma_get_subscription_source()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE;
}
RIL_CdmaSubscriptionSource* ril_utf_androidTel_if::update_default_ril_request_cdma_get_subscription_source_resp()
{
  payload = malloc_ril_request_cdma_get_subscription_source_resp();
  payload_len = sizeof(RIL_CdmaSubscriptionSource) ;
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE;

  RIL_CdmaSubscriptionSource* ptr = (RIL_CdmaSubscriptionSource*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_cdma_get_subscription_source_resp(int sub_source)
{
  payload = malloc_ril_request_cdma_get_subscription_source_resp();
  payload_len = sizeof(RIL_CdmaSubscriptionSource) ;
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE;
  RIL_CdmaSubscriptionSource* ptr = (RIL_CdmaSubscriptionSource*) payload;
  *ptr = (RIL_CdmaSubscriptionSource)sub_source;
}
RIL_CdmaSubscriptionSource* ril_utf_androidTel_if::malloc_ril_request_cdma_get_subscription_source_resp()
{
  RIL_CdmaSubscriptionSource *ptr = (RIL_CdmaSubscriptionSource *)malloc(sizeof(RIL_CdmaSubscriptionSource));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_cdma_get_subscription_source_resp()
{
  RIL_CdmaSubscriptionSource *ptr = (RIL_CdmaSubscriptionSource *) payload;
  free(ptr);
}
void ril_utf_androidTel_if::update_default_ril_request_cdma_get_subscription_source_resp_failure()
{
  payload = malloc_ril_request_cdma_get_subscription_source_resp();
  payload_len = sizeof(RIL_CdmaSubscriptionSource) ;
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE;
}
int ril_utf_androidTel_if::compare_ril_request_cdma_get_subscription_source_resp(RIL_CdmaSubscriptionSource *ptr1, RIL_CdmaSubscriptionSource *ptr2)
{
  if(ptr1!=ptr2)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE
========================================================*/
ril_request_cdma_set_roaming_preference_t* ril_utf_androidTel_if::update_default_ril_request_cdma_set_roaming_preference()
{
  payload = malloc_ril_request_cdma_set_roaming_preference();
  payload_len = sizeof(ril_request_cdma_set_roaming_preference_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE;

  ril_request_cdma_set_roaming_preference_t* ptr = (ril_request_cdma_set_roaming_preference_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_cdma_set_roaming_preference_resp()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE;
}
ril_request_cdma_set_roaming_preference_t* ril_utf_androidTel_if::malloc_ril_request_cdma_set_roaming_preference()
{
  ril_request_cdma_set_roaming_preference_t *ptr = (ril_request_cdma_set_roaming_preference_t *)malloc(sizeof(ril_request_cdma_set_roaming_preference_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_cdma_set_roaming_preference()
{
  ril_request_cdma_set_roaming_preference_t *ptr = (ril_request_cdma_set_roaming_preference_t *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_cdma_set_roaming_preference(ril_request_cdma_set_roaming_preference_t *ptr1, ril_request_cdma_set_roaming_preference_t *ptr2)
{
  if(ptr1->roaming_preference != ptr2->roaming_preference)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_cdma_query_roaming_preference()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE;
}
ril_request_cdma_query_roaming_preference_resp_t* ril_utf_androidTel_if::update_default_ril_request_cdma_query_roaming_preference_resp()
{
  payload = malloc_ril_request_cdma_query_roaming_preference_resp();
  payload_len = sizeof(ril_request_cdma_query_roaming_preference_resp_t) ;
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE;

  ril_request_cdma_query_roaming_preference_resp_t* ptr = (ril_request_cdma_query_roaming_preference_resp_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_cdma_query_roaming_preference_resp_failure()
{
  payload = malloc_ril_request_cdma_query_roaming_preference_resp();
  payload_len = sizeof(ril_request_cdma_query_roaming_preference_resp_t) ;
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE;
}

void ril_utf_androidTel_if::update_default_ril_request_cdma_query_roaming_preference_resp(ril_cdma_roaming_preference_enum_type cdma_roam_pref)
{
  payload = malloc_ril_request_cdma_query_roaming_preference_resp();
  payload_len = sizeof(ril_request_cdma_query_roaming_preference_resp_t) ;
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE;
  ril_request_cdma_query_roaming_preference_resp_t* ptr = (ril_request_cdma_query_roaming_preference_resp_t*) payload;
  ptr->roaming_preference = cdma_roam_pref;
}
ril_request_cdma_query_roaming_preference_resp_t* ril_utf_androidTel_if::malloc_ril_request_cdma_query_roaming_preference_resp()
{
  ril_request_cdma_query_roaming_preference_resp_t *ptr = (ril_request_cdma_query_roaming_preference_resp_t *)malloc(sizeof(ril_request_cdma_query_roaming_preference_resp_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_cdma_query_roaming_preference_resp()
{
  ril_request_cdma_query_roaming_preference_resp_t *ptr = (ril_request_cdma_query_roaming_preference_resp_t *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_cdma_query_roaming_preference_resp(ril_request_cdma_query_roaming_preference_resp_t *ptr1, ril_request_cdma_query_roaming_preference_resp_t *ptr2)
{
  if(ptr1->roaming_preference != ptr2->roaming_preference)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE
========================================================*/
void ril_utf_androidTel_if::update_default_exit_emergency_callback_mode()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE;
}
void ril_utf_androidTel_if::update_default_exit_emergency_callback_mode_resp()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE;
}
/*========================================================
  RIL_REQUEST_SET_UICC_SUBSCRIPTION
========================================================*/
RIL_SelectUiccSub* ril_utf_androidTel_if::update_default_ril_request_set_uicc_subscription()
{
  payload = malloc_ril_request_set_uicc_subscription();
  payload_len = sizeof(RIL_SelectUiccSub) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_UICC_SUBSCRIPTION;

  RIL_SelectUiccSub* ptr = (RIL_SelectUiccSub*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_set_uicc_subscription_resp()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_UICC_SUBSCRIPTION;
}
RIL_SelectUiccSub* ril_utf_androidTel_if::malloc_ril_request_set_uicc_subscription()
{
  RIL_SelectUiccSub *ptr = (RIL_SelectUiccSub *)malloc(sizeof(RIL_SelectUiccSub));
  return ptr;
}
void ril_utf_androidTel_if::set_ril_request_set_uicc_subscription_app_index(int app_index)
{
  RIL_SelectUiccSub* ptr = (RIL_SelectUiccSub*) payload;
  ptr->app_index = app_index;
}
void ril_utf_androidTel_if::set_ril_request_set_uicc_subscription_slot(int slot)
{
  RIL_SelectUiccSub* ptr = (RIL_SelectUiccSub*) payload;
  ptr->slot = slot;
}
void ril_utf_androidTel_if::set_ril_request_set_uicc_subscription_act_status(RIL_UiccSubActStatus act_status)
{
  RIL_SelectUiccSub* ptr = (RIL_SelectUiccSub*) payload;
  ptr->act_status = act_status;
}
void ril_utf_androidTel_if::set_ril_request_set_uicc_subscription_sub_type(RIL_SubscriptionType sub_type)
{
  RIL_SelectUiccSub* ptr = (RIL_SelectUiccSub*) payload;
  ptr->sub_type = sub_type;
}
void ril_utf_androidTel_if::free_ril_request_set_uicc_subscription()
{
  RIL_SelectUiccSub *ptr = (RIL_SelectUiccSub *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_set_uicc_subscription(RIL_SelectUiccSub *ptr1, RIL_SelectUiccSub *ptr2)
{
  if(ptr1->slot != ptr2->slot)
    return FALSE;
  else if(ptr1->app_index != ptr2->app_index)
    return FALSE;
  else if(ptr1->sub_type != ptr2->sub_type)
    return FALSE;
  else if(ptr1->act_status != ptr2->act_status)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_SET_DATA_SUBSCRIPTION
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_set_data_subscription()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  //msg_id = RIL_REQUEST_SET_DATA_SUBSCRIPTION;
}
void ril_utf_androidTel_if::update_default_ril_request_set_data_subscription_resp()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  //msg_id = RIL_REQUEST_SET_DATA_SUBSCRIPTION;
}

/*========================================================
  RIL_REQUEST_GET_UICC_SUBSCRIPTION
========================================================*/

void ril_utf_androidTel_if::update_default_ril_request_get_uicc_subscription()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  //msg_id = RIL_REQUEST_GET_UICC_SUBSCRIPTION;
}
RIL_SelectUiccSub* ril_utf_androidTel_if::update_default_ril_request_get_uicc_subscription_resp()
{
  payload = malloc_ril_request_get_uicc_subscription_resp();
  payload_len = sizeof(RIL_SelectUiccSub) ;
  payload_type = ril_utf_ril_response;
  //msg_id = RIL_REQUEST_GET_UICC_SUBSCRIPTION;

  RIL_SelectUiccSub* ptr = (RIL_SelectUiccSub*) payload;
  return ptr;
}
RIL_SelectUiccSub* ril_utf_androidTel_if::malloc_ril_request_get_uicc_subscription_resp()
{
  RIL_SelectUiccSub *ptr = (RIL_SelectUiccSub *)malloc(sizeof(RIL_SelectUiccSub));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_get_uicc_subscription_resp()
{
  RIL_SelectUiccSub *ptr = (RIL_SelectUiccSub *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_get_uicc_subscription_resp(RIL_SelectUiccSub *ptr1, RIL_SelectUiccSub *ptr2)
{
  if(ptr1->slot != ptr2->slot)
    return FALSE;
  else if(ptr1->app_index != ptr2->app_index)
    return FALSE;
  else if(ptr1->sub_type != ptr2->sub_type)
    return FALSE;
  else if(ptr1->act_status != ptr2->act_status)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_GET_DATA_SUBSCRIPTION
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_get_data_subscription()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  //msg_id = RIL_REQUEST_GET_DATA_SUBSCRIPTION;
}
ril_request_get_data_subscription_resp_t* ril_utf_androidTel_if::update_default_ril_request_get_data_subscription_resp()
{
  payload = malloc_ril_request_get_data_subscription_resp();
  payload_len = sizeof(ril_request_get_data_subscription_resp_t) ;
  payload_type = ril_utf_ril_response;
  //msg_id = RIL_REQUEST_GET_DATA_SUBSCRIPTION;

  ril_request_get_data_subscription_resp_t* ptr = (ril_request_get_data_subscription_resp_t*) payload;
  return ptr;
}
ril_request_get_data_subscription_resp_t* ril_utf_androidTel_if::malloc_ril_request_get_data_subscription_resp()
{
  ril_request_get_data_subscription_resp_t *ptr = (ril_request_get_data_subscription_resp_t *)malloc(sizeof(ril_request_get_data_subscription_resp_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_get_data_subscription_resp()
{
  ril_request_get_data_subscription_resp_t *ptr = (ril_request_get_data_subscription_resp_t *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_get_data_subscription_resp(ril_request_get_data_subscription_resp_t *ptr1, ril_request_get_data_subscription_resp_t *ptr2)
{
  if(ptr1->subscription_status != ptr2->subscription_status)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_SET_SUBSCRIPTION_MODE
========================================================*/
ril_request_set_subscription_mode_t* ril_utf_androidTel_if::update_default_ril_request_set_subscription_mode()
{
  payload = malloc_ril_request_set_subscription_mode();
  payload_len = sizeof(ril_request_set_subscription_mode_t) ;
  payload_type = ril_utf_ril_response;
  //msg_id = RIL_REQUEST_SET_SUBSCRIPTION_MODE;

  ril_request_set_subscription_mode_t* ptr = (ril_request_set_subscription_mode_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_set_subscription_mode_resp()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  //msg_id = RIL_REQUEST_SET_SUBSCRIPTION_MODE;
}
ril_request_set_subscription_mode_t* ril_utf_androidTel_if::malloc_ril_request_set_subscription_mode()
{
  ril_request_set_subscription_mode_t *ptr = (ril_request_set_subscription_mode_t *)malloc(sizeof(ril_request_set_subscription_mode_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_set_subscription_mode()
{
  ril_request_set_subscription_mode_t *ptr = (ril_request_set_subscription_mode_t *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_set_subscription_mode(ril_request_set_subscription_mode_t *ptr1, ril_request_set_subscription_mode_t *ptr2)
{
  if(ptr1->subscription_mode != ptr2->subscription_mode)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_SET_TRANSMIT_POWER
========================================================*/
/*
void ril_utf_androidTel_if::update_default_ril_request_set_transmit_power()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_TRANSMIT_POWER;
}
void ril_utf_androidTel_if::update_default_ril_request_set_transmit_power_resp()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_TRANSMIT_POWER;
}
*/
/*========================================================
  RIL_REQUEST_VOICE_RADIO_TECH
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_voice_radio_tech()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_VOICE_RADIO_TECH;
}
RIL_RadioTechnology* ril_utf_androidTel_if::update_default_ril_request_voice_radio_tech_resp()
{
  payload = malloc_ril_request_voice_radio_tech_resp();
  payload_len = sizeof(RIL_RadioTechnology) ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_VOICE_RADIO_TECH;

  RIL_RadioTechnology* ptr = (RIL_RadioTechnology*) payload;
  return ptr;
}
RIL_RadioTechnology* ril_utf_androidTel_if::malloc_ril_request_voice_radio_tech_resp()
{
  RIL_RadioTechnology *ptr = (RIL_RadioTechnology *)malloc(sizeof(RIL_RadioTechnology));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_voice_radio_tech_resp()
{
  RIL_RadioTechnology *ptr = (RIL_RadioTechnology *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_voice_radio_tech_resp(RIL_RadioTechnology *ptr1, RIL_RadioTechnology *ptr2)
{
  if(ptr1 != ptr2)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_GET_DC_RT_INFO
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_get_dc_rt_info()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_GET_DC_RT_INFO;
}
void ril_utf_androidTel_if::update_default_ril_request_get_dc_rt_info_resp()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_GET_DC_RT_INFO;
}
/*========================================================
  RIL_REQUEST_SIGNAL_STRENGTH
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_signal_strength()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SIGNAL_STRENGTH;
}
RIL_SignalStrength_v6* ril_utf_androidTel_if::update_default_ril_request_signal_strength_resp()
{
  payload = malloc_ril_request_signal_strength_resp();
  payload_len = sizeof(RIL_SignalStrength_v6) ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SIGNAL_STRENGTH;

  RIL_SignalStrength_v6* ptr = (RIL_SignalStrength_v6*) payload;
  return ptr;
}
RIL_SignalStrength_v6* ril_utf_androidTel_if::malloc_ril_request_signal_strength_resp()
{
  RIL_SignalStrength_v6 *ptr = (RIL_SignalStrength_v6 *)malloc(sizeof(RIL_SignalStrength_v6));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_signal_strength_resp()
{
  RIL_SignalStrength_v6 *ptr = (RIL_SignalStrength_v6 *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_signal_strength_resp(RIL_SignalStrength_v6 *ptr1, RIL_SignalStrength_v6 *ptr2)
{
  if(ptr1 != ptr2)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_VOICE_REGISTRATION_STATE
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_registration_state()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_VOICE_REGISTRATION_STATE;
}
void ril_utf_androidTel_if::update_default_ril_request_registration_state_resp_in_error_case()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_VOICE_REGISTRATION_STATE;
}
RIL_VoiceRegistrationStateResponse* ril_utf_androidTel_if::update_default_ril_request_registration_state_resp()
{
  payload = (RIL_VoiceRegistrationStateResponse*)malloc(sizeof(RIL_VoiceRegistrationStateResponse));
  payload_len = sizeof(RIL_VoiceRegistrationStateResponse);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_VOICE_REGISTRATION_STATE;

  RIL_VoiceRegistrationStateResponse* ptr = (RIL_VoiceRegistrationStateResponse *)payload;
  return ptr;
}

/*========================================================
  RIL_REQUEST_DATA_REGISTRATION_STATE
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_data_registration_state()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_DATA_REGISTRATION_STATE;
}
void ril_utf_androidTel_if::update_default_ril_request_data_registration_state_in_error_case()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_DATA_REGISTRATION_STATE;
}
RIL_DataRegistrationStateResponse* ril_utf_androidTel_if::update_default_ril_request_data_registration_state_resp()
{
  payload = (RIL_DataRegistrationStateResponse*)malloc(sizeof(RIL_DataRegistrationStateResponse));
  payload_len = sizeof(RIL_DataRegistrationStateResponse);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_DATA_REGISTRATION_STATE;

  RIL_DataRegistrationStateResponse* ptr = (RIL_DataRegistrationStateResponse *)payload;
  return ptr;
}
/*========================================================
  RIL_REQUEST_OPERATOR
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_operator()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_OPERATOR;
}
ril_request_operator_resp_from_qcril_t* ril_utf_androidTel_if::update_default_ril_request_operator_resp()
{
  payload = malloc_ril_request_operator_resp();
  payload_len = sizeof(ril_request_operator_resp_from_qcril_t) ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_OPERATOR;

  ril_request_operator_resp_from_qcril_t* ptr = (ril_request_operator_resp_from_qcril_t*) payload;
  return ptr;
}
ril_request_operator_resp_from_qcril_t* ril_utf_androidTel_if::malloc_ril_request_operator_resp()
{
  ril_request_operator_resp_from_qcril_t *ptr = (ril_request_operator_resp_from_qcril_t*)malloc(sizeof(ril_request_operator_resp_from_qcril_t));
  ril_request_operator_resp_t *operator_resp = (ril_request_operator_resp_t*)malloc(sizeof(ril_request_operator_resp_t));

  memset(operator_resp, '\0', sizeof(ril_request_operator_resp_t));
  ptr->operator_info_array[0] = operator_resp->long_eons;
  ptr->operator_info_array[1] = operator_resp->short_eons;
  ptr->operator_info_array[2] = operator_resp->mcc_mnc_ascii;

  return ptr;
}
/*========================================================
  RIL_REQUEST_SCREEN_STATE
========================================================*/
ril_request_screen_state_t* ril_utf_androidTel_if::update_default_ril_request_screen_state()
{
  payload = malloc_ril_request_screen_state();
  payload_len = sizeof(ril_request_screen_state_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SCREEN_STATE;

  ril_request_screen_state_t* ptr = (ril_request_screen_state_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_screen_state_resp()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SCREEN_STATE;
}
ril_request_screen_state_t* ril_utf_androidTel_if::malloc_ril_request_screen_state()
{
  ril_request_screen_state_t *ptr = (ril_request_screen_state_t*)malloc(sizeof(ril_request_screen_state_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_screen_state()
{
  ril_request_screen_state_t *ptr = (ril_request_screen_state_t *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_screen_state(ril_request_screen_state_t *ptr1, ril_request_screen_state_t *ptr2)
{
  if(ptr1->screen_state != ptr2->screen_state)
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_GET_NEIGHBORING_CELL_IDS
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_get_neighboring_cell_ids()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_GET_NEIGHBORING_CELL_IDS;
}
RIL_NeighboringCell** ril_utf_androidTel_if::update_default_ril_request_get_neighboring_cell_ids_resp(int num_cells)
{
  payload_len = sizeof(RIL_NeighboringCell*)*num_cells;
  payload = malloc(payload_len);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_GET_NEIGHBORING_CELL_IDS;

  RIL_NeighboringCell** ptr = (RIL_NeighboringCell**) payload;
  for(int i = 0; i < num_cells; i++) {
    ptr[i] = (RIL_NeighboringCell*)malloc(sizeof(RIL_NeighboringCell));
    memset(ptr[i], 0, sizeof(RIL_NeighboringCell));
  }
  return ptr;
}
RIL_NeighboringCell** ril_utf_androidTel_if::malloc_ril_request_get_neighboring_cell_ids_resp()
{
  RIL_NeighboringCell *ptr = (RIL_NeighboringCell*)malloc(sizeof(RIL_NeighboringCell));
  ptr->cid = (char *)malloc(sizeof(char));
  RIL_NeighboringCell **dptr = &ptr;
  return dptr;
}
void ril_utf_androidTel_if::free_ril_request_get_neighboring_cell_ids_resp()
{
  RIL_NeighboringCell **dptr = (RIL_NeighboringCell **) payload;
  free((*dptr)->cid);
  free((*dptr));
}
int ril_utf_androidTel_if::compare_ril_request_get_neighboring_cell_ids_resp(RIL_NeighboringCell **ptr1, RIL_NeighboringCell **ptr2)
{
  if(strncmp((*ptr1)->cid,(*ptr2)->cid, 9) !=0)
    return FALSE;
  else if((*ptr1)->rssi != (*ptr2)->rssi )
    return FALSE;
  else
    return TRUE;
}
/*========================================================
  RIL_REQUEST_SET_LOCATION_UPDATES
========================================================*/
ril_request_set_location_updates_t* ril_utf_androidTel_if::update_default_ril_request_set_location_updates()
{
  payload = malloc_ril_request_set_location_updates();
  payload_len = sizeof(ril_request_set_location_updates_t) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_LOCATION_UPDATES;

  ril_request_set_location_updates_t* ptr = (ril_request_set_location_updates_t*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_set_location_updates_resp()
{
  payload = NULL ;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_LOCATION_UPDATES;
}
ril_request_set_location_updates_t* ril_utf_androidTel_if::malloc_ril_request_set_location_updates()
{
  ril_request_set_location_updates_t *ptr = (ril_request_set_location_updates_t*)malloc(sizeof(ril_request_set_location_updates_t));
  return ptr;
}
void ril_utf_androidTel_if::free_ril_request_set_location_updates()
{
  ril_request_set_location_updates_t *ptr = (ril_request_set_location_updates_t *) payload;
  free(ptr);
}
int ril_utf_androidTel_if::compare_ril_request_set_location_updates(ril_request_set_location_updates_t *ptr1, ril_request_set_location_updates_t *ptr2)
{
  if(ptr1->updates_status != ptr2->updates_status)
    return FALSE;
  else
    return TRUE;
}

/*========================================================
  RIL_REQUEST_QUERY_AVAILABLE_NETWORKS
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_query_available_networks()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_QUERY_AVAILABLE_NETWORKS;
}

void ril_utf_androidTel_if::update_default_ril_request_query_available_networks_resp_failure()
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_QUERY_AVAILABLE_NETWORKS;
}

ril_request_query_available_networks_resp_t* ril_utf_androidTel_if::update_default_ril_request_query_available_networks_resp(int nw_cnt, int is_legacy)
{
  payload = malloc_ril_request_query_available_networks_resp(nw_cnt, is_legacy);
  payload_len = sizeof(ril_request_query_available_networks_resp_t) ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_QUERY_AVAILABLE_NETWORKS;

  ril_request_query_available_networks_resp_t* ptr = (ril_request_query_available_networks_resp_t*) payload;
  return ptr;
}

ril_request_query_available_networks_resp_t* ril_utf_androidTel_if::malloc_ril_request_query_available_networks_resp(int nw_cnt, int is_legacy)
{
  int i=0,j=0;

  ril_request_query_available_networks_resp_t *ptr = (ril_request_query_available_networks_resp_t *)malloc(sizeof(ril_request_query_available_networks_resp_t));
  memset(ptr,0,sizeof(ril_request_query_available_networks_resp_t));
  qcril_qmi_nw_scan_resp_helper_type *ril_resp_helper = (qcril_qmi_nw_scan_resp_helper_type *)malloc(nw_cnt * sizeof(qcril_qmi_nw_scan_resp_helper_type));
  memset(ril_resp_helper,0,sizeof(*ril_resp_helper));

  for(i=0;i<nw_cnt;i++)
  {
    ptr->nw_scan_info_array[j++] = ril_resp_helper[i].long_eons;
    ptr->nw_scan_info_array[j++] = ril_resp_helper[i].short_eons;
    ptr->nw_scan_info_array[j++] = ril_resp_helper[i].mccmnc_info;
    ptr->nw_scan_info_array[j++] = ril_resp_helper[i].network_status;
    if(is_legacy)
    {
       ptr->nw_scan_info_array[j++] = ril_resp_helper[i].rat;
    }
  }

  return ptr;
}

/*========================================================
  RIL_UNSOL_EMERGENCY_NUMBERS_LIST
========================================================*/
ril_unsol_emergency_list_t* ril_utf_androidTel_if::update_default_ril_unsol_emergency_number_list(
        size_t len)
{
  ril_unsol_emergency_list_t* ptr;
  payload = malloc(sizeof(ril_unsol_emergency_list_t));
  payload_len = sizeof(ril_unsol_emergency_list_t);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_EMERGENCY_NUMBERS_LIST;
  ptr = (ril_unsol_emergency_list_t*)payload;
  ptr->ecc_list.emergency_list_length = len;
  return ptr;
}

/*========================================================
  RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_resp_ims_network_state_changed(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED;
}

/*========================================================
  RIL_REQUEST_IMS_REGISTRATION_STATE
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_ims_reg_state(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_IMS_REGISTRATION_STATE;
}

/*========================================================
  RIL_REQUEST_IMS_REGISTRATION_STATE_RESP
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_ims_reg_state_resp(uint8_t is_empty, int reg_status, int rat)
{
  if(FALSE == is_empty)
  {
    payload = (int *)malloc(2*sizeof(int));
    memset(payload, 0, (2*sizeof(int)));
    payload_len = (2*sizeof(int));

    int *ptr = (int *)payload;
    ptr[0] = reg_status;
    ptr[1] = rat;
  }
  else
  {
    payload = NULL;
    payload_len = 0;
  }
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_IMS_REGISTRATION_STATE;
}

/*========================================================
  RIL_REQUEST_IMS_SEND_SMS
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_ims_send_sms(uint8_t is_3gpp2)
{
  if(is_3gpp2)
  {
    payload = (ims_send_sms_3gpp2_payload *)malloc_ril_request_ims_send_sms_3gpp2();
    payload_len = sizeof(ims_send_sms_3gpp2_payload);
  }
  else
  {
    payload = malloc_ril_request_ims_send_sms_3gpp();
    payload_len = sizeof(ims_send_sms_3gpp_payload);
  }

  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_IMS_SEND_SMS;
}

/*========================================================
  RIL_REQUEST_IMS_SEND_SMS_RESP
========================================================*/
RIL_SMS_Response* ril_utf_androidTel_if::update_default_ril_request_ims_send_sms_resp()
{
  payload = malloc_ril_request_send_sms_resp();
  payload_len = sizeof(RIL_SMS_Response);

  RIL_SMS_Response *ptr = (RIL_SMS_Response *)payload;
  ptr->messageRef = 0;
  ptr->ackPDU = NULL;
  ptr->errorCode = -1;

  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_IMS_SEND_SMS;
  return ptr;
}
/*========================================================
  RIL_REQUEST_IMS_SEND_SMS_RESP_EMPTY
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_ims_send_sms_resp_empty()
{
  payload = NULL;
  payload_len = 0;

  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_IMS_SEND_SMS;
}

#ifdef RIL_UTF_L_MASTER
/*========================================================
  RIL_UNSOL_SRVCC_STATE_NOTIFY
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_srvcc_state_notify(RIL_SrvccState srvcc_state)
{
  struct PACKED_STRUCT ril_unsol_srvcc_state_notify_t* tmp =
      (struct PACKED_STRUCT ril_unsol_srvcc_state_notify_t*) malloc(
              sizeof(struct PACKED_STRUCT ril_unsol_srvcc_state_notify_t));
  payload = tmp;
  payload_len = sizeof(struct PACKED_STRUCT ril_unsol_srvcc_state_notify_t);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_SRVCC_STATE_NOTIFY;
  tmp->srvcc_state = srvcc_state;
}
#endif

ims_send_sms_3gpp2_payload* ril_utf_androidTel_if::malloc_ril_request_ims_send_sms_3gpp2()
{
  ims_send_sms_3gpp2_payload* ptr = (ims_send_sms_3gpp2_payload*) malloc(sizeof(ims_send_sms_3gpp2_payload));
  memset(ptr, 0, sizeof(ims_send_sms_3gpp2_payload));
  ptr->cdmaMessage = (RIL_CDMA_SMS_Message*)malloc(1*sizeof(RIL_CDMA_SMS_Message));
  memset(ptr->cdmaMessage, 0, sizeof(RIL_CDMA_SMS_Message));
  return ptr;
}

ims_send_sms_3gpp_payload* ril_utf_androidTel_if::malloc_ril_request_ims_send_sms_3gpp()
{
  ims_send_sms_3gpp_payload* ptr = (ims_send_sms_3gpp_payload*) malloc(sizeof(ims_send_sms_3gpp_payload));
  memset(ptr, 0, sizeof(ims_send_sms_3gpp_payload));
  ptr->gsmMessage = (char**)malloc(sizeof(char*) *2);
  ptr->gsmMessage[0] = (char *)malloc(SMSC_address_size);
  ptr->gsmMessage[1] = (char *)malloc(WMS_MESSAGE_LENGTH_MAX_V01);
  memset(ptr->gsmMessage[0], 0, SMSC_address_size);
  memset(ptr->gsmMessage[1], 0, WMS_MESSAGE_LENGTH_MAX_V01);
  return ptr;
}

ims_send_sms_3gpp_payload* ril_utf_androidTel_if::set_ril_request_ims_send_sms_3gpp_tech_retry_msgref(RIL_RadioTechnologyFamily tech, unsigned char retry, int messageRef)
{
  ims_send_sms_3gpp_payload* ptr = (ims_send_sms_3gpp_payload*)payload;

  ptr->tech = tech;
  ptr->retry = retry;
  ptr->messageRef = messageRef;

  return ptr;
}

ims_send_sms_3gpp2_payload* ril_utf_androidTel_if::set_ril_request_ims_send_sms_3gpp2_tech_retry_msgref(RIL_RadioTechnologyFamily tech, unsigned char retry, int messageRef)
{
  ims_send_sms_3gpp2_payload* ptr = (ims_send_sms_3gpp2_payload*)payload;

  ptr->tech = tech;
  ptr->retry = retry;
  ptr->messageRef = messageRef;

  return ptr;
}

void ril_utf_androidTel_if::set_ril_request_ims_send_sms_3gpp2_uTeleserviceID(int uTeleserviceID)
{
  ims_send_sms_3gpp2_payload* ptr = (ims_send_sms_3gpp2_payload*)payload;

  ptr->cdmaMessage[0].uTeleserviceID = uTeleserviceID;
}

void ril_utf_androidTel_if::set_ril_request_ims_send_sms_3gpp2_bIsServicePresent(unsigned char bIsServicePresent)
{
  ims_send_sms_3gpp2_payload* ptr = (ims_send_sms_3gpp2_payload*)payload;

  ptr->cdmaMessage[0].bIsServicePresent = bIsServicePresent;
}

void ril_utf_androidTel_if::set_ril_request_ims_send_sms_3gpp2_uServicecategory(int uServicecategory)
{
  ims_send_sms_3gpp2_payload* ptr = (ims_send_sms_3gpp2_payload*)payload;

  ptr->cdmaMessage[0].uServicecategory = uServicecategory;
}

/*========================================================
  RIL_REQUEST_SET_INITIAL_ATTACH_APN
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_set_initial_attach_apn(void)
{
  payload = malloc(sizeof(RIL_InitialAttachApn));
  memset(payload, 0, sizeof(RIL_InitialAttachApn));
  payload_len = sizeof(RIL_InitialAttachApn);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_INITIAL_ATTACH_APN;
}

void ril_utf_androidTel_if::update_default_ril_request_set_initial_attach_apn_resp(RIL_Errno err)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_INITIAL_ATTACH_APN;
  error = err;
}

void ril_utf_androidTel_if::update_ril_request_set_initial_attach_apn_address_and_protocol(const char* address, const char* protocol)
{
  RIL_InitialAttachApn* ptr = (RIL_InitialAttachApn*) payload;
  if (address)
  {
    ptr->apn = (char *)malloc(strlen(address)+1);
    strcpy(ptr->apn, address);
  }
  if (protocol)
  {
    ptr->protocol = (char *)malloc(strlen(protocol)+1);
    strcpy(ptr->protocol, protocol);
  }
}

/*========================================================
 RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_uicc_subscription_status_changed(int data)
{
  payload = malloc(sizeof(int));
  payload_len = sizeof(int);
  memset(payload, 0x0, payload_len);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED;

  int *ptr = (int *)payload;
  *ptr = data;
}

#ifdef RIL_UTF_L_MASTER
/*========================================================
  RIL_REQUEST_ALLOW_DATA
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_allow_data_req(int allow_data)
{
  payload = malloc(sizeof(int));
  payload_len = sizeof(int);
  memset(payload, 0x0, payload_len);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_ALLOW_DATA;

  int *ptr = (int *)payload;
  *ptr = allow_data;
}

void ril_utf_androidTel_if::update_default_ril_request_allow_data_resp(void)
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_ALLOW_DATA;
}

void ril_utf_androidTel_if::update_default_ril_request_allow_data_resp(RIL_Errno err)
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_ALLOW_DATA;
  error = err;
}

/*========================================================
  RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_cdma_subscription_source_changed(int data)
{
  payload = malloc(sizeof(int));
  payload_len = sizeof(int);
  memset(payload, 0x0, payload_len);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED;

  int *ptr = (int *)payload;
  *ptr = data;
}

/*========================================================
  RIL_REQUEST_SET_RADIO_CAPABILITY
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_set_radio_capability_req
                                                    (int version,
                                                     int session,
                                                     int phase,
                                                     int rat,
                                                     char *logicalModemUuid_data,
                                                     unsigned int logicalModemUuid_len,
                                                     int status)
{
  payload = malloc(sizeof(RIL_RadioCapability));
  payload_len = sizeof(RIL_RadioCapability);
  memset(payload, 0x0, payload_len);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_RADIO_CAPABILITY;

  RIL_RadioCapability *ptr = (RIL_RadioCapability *)payload;
  ptr->version = version;
  ptr->session = session;
  ptr->phase = phase;
  ptr->rat = rat;
  if (logicalModemUuid_len > 0 && logicalModemUuid_len < MAX_UUID_LENGTH)
  {
      memset(ptr->logicalModemUuid, 0, MAX_UUID_LENGTH);
      memcpy (ptr->logicalModemUuid, logicalModemUuid_data, logicalModemUuid_len);
  }
  ptr->status = status;
}

void ril_utf_androidTel_if::update_default_ril_request_set_radio_capability_resp
                                                    (RIL_Errno err,
                                                     int version,
                                                     int session,
                                                     int phase,
                                                     int rat,
                                                     char *logicalModemUuid_data,
                                                     unsigned int logicalModemUuid_len,
                                                     int status)
{
  payload = malloc(sizeof(RIL_RadioCapability));
  payload_len = sizeof(RIL_RadioCapability);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_RADIO_CAPABILITY;

  error = err;
  RIL_RadioCapability *ptr = (RIL_RadioCapability *)payload;
  ptr->version = version;
  ptr->session = session;
  ptr->phase = phase;
  ptr->rat = rat;
  if (logicalModemUuid_len > 0 && logicalModemUuid_len < MAX_UUID_LENGTH)
  {
      memset(ptr->logicalModemUuid, 0, MAX_UUID_LENGTH);
      memcpy (ptr->logicalModemUuid, logicalModemUuid_data, logicalModemUuid_len);
  }
  ptr->status = status;
}

/*========================================================
  RIL_REQUEST_GET_RADIO_CAPABILITY
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_get_radio_capability_req
                                                    (int version,
                                                     int session,
                                                     int phase,
                                                     int rat,
                                                     char *logicalModemUuid_data,
                                                     unsigned int logicalModemUuid_len,
                                                     int status)
{
  payload = malloc(sizeof(RIL_RadioCapability));
  payload_len = sizeof(RIL_RadioCapability);
  memset(payload, 0x0, payload_len);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_GET_RADIO_CAPABILITY;

  RIL_RadioCapability *ptr = (RIL_RadioCapability *)payload;
  ptr->version = version;
  ptr->session = session;
  ptr->phase = phase;
  ptr->rat = rat;
  if (logicalModemUuid_len > 0 && logicalModemUuid_len < MAX_UUID_LENGTH)
  {
      memset(ptr->logicalModemUuid, 0, MAX_UUID_LENGTH);
      memcpy (ptr->logicalModemUuid, logicalModemUuid_data, logicalModemUuid_len);
  }
  ptr->status = status;
}

void ril_utf_androidTel_if::update_default_ril_request_get_radio_capability_resp
                                                    (RIL_Errno err,
                                                     int version,
                                                     int session,
                                                     int phase,
                                                     int rat,
                                                     char *logicalModemUuid_data,
                                                     unsigned int logicalModemUuid_len,
                                                     int status)
{
  payload = malloc(sizeof(RIL_RadioCapability));
  payload_len = sizeof(RIL_RadioCapability);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_GET_RADIO_CAPABILITY;

  error = err;
  RIL_RadioCapability *ptr = (RIL_RadioCapability *)payload;
  ptr->version = version;
  ptr->session = session;
  ptr->phase = phase;
  ptr->rat = rat;
  if (logicalModemUuid_len > 0 && logicalModemUuid_len < MAX_UUID_LENGTH)
  {
      memset(ptr->logicalModemUuid, 0, MAX_UUID_LENGTH);
      memcpy (ptr->logicalModemUuid, logicalModemUuid_data, logicalModemUuid_len);
  }
  ptr->status = status;
}
/*========================================================
  RIL_UNSOL_RADIO_CAPABILITY
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_radio_capability
                                                    (int version,
                                                     int session,
                                                     int phase,
                                                     int rat,
                                                     char *logicalModemUuid_data,
                                                     unsigned int logicalModemUuid_len,
                                                     int status)
{
  payload = malloc(sizeof(RIL_RadioCapability));
  payload_len = sizeof(RIL_RadioCapability);
  memset(payload, 0x0, payload_len);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_RADIO_CAPABILITY;

  RIL_RadioCapability *ptr = (RIL_RadioCapability *)payload;
  ptr->version = version;
  ptr->session = session;
  ptr->phase = phase;
  ptr->rat = rat;
  if (logicalModemUuid_len > 0 && logicalModemUuid_len < MAX_UUID_LENGTH)
  {
      memcpy (ptr->logicalModemUuid, logicalModemUuid_data, logicalModemUuid_len);
  }
  ptr->status = status;
}
#endif

/*========================================================
  RIL_REQUEST_OEM_HOOK_RAW
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_oem_hook_raw_req(int message_id, void *data, uint32_t data_len)
{
  unsigned char header[] = {0x51, 0x4F, 0x45, 0x4D, 0x48, 0x4F, 0x4F, 0x4B};
  unsigned char *ptr;
  payload_len = 8 + 4 + 4 + data_len;
  payload = malloc (payload_len);
  memset(payload, 0x0, payload_len);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_OEM_HOOK_RAW;

  ptr = (unsigned char *) payload;
  memcpy (payload, header, sizeof (header));
  ptr[11] = (message_id >> 24) & 0xFF;
  ptr[10] = (message_id >> 16) & 0xFF;
  ptr[9]  = (message_id >> 8)  & 0xFF;
  ptr[8]  = (message_id)       & 0xFF;
  if (data_len)
  {
    ptr[15] = (data_len >> 24) & 0xFF;
    ptr[14] = (data_len >> 16) & 0xFF;
    ptr[13] = (data_len >> 8)  & 0xFF;
    ptr[12] = (data_len)        & 0xFF;
    memcpy (ptr + 16, data, data_len);
  }
}

void ril_utf_androidTel_if::update_default_ril_request_oem_hook_raw_resp(RIL_Errno err)
{
  payload = NULL;
  payload_len = 0 ;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_OEM_HOOK_RAW;
  error = err;
}

/*========================================================
  RIL_UNSOL_RESTRICTED_STATE_CHANGED
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_restricted_state_changed(int data)
{
  payload = malloc(sizeof(int));
  payload_len = sizeof(int);
  memset(payload, 0x0, payload_len);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_RESTRICTED_STATE_CHANGED;

  int *ptr = (int *)payload;
  *ptr = data;
}

/*========================================================
  RIL_UNSOL_CDMA_CALL_WAITING
========================================================*/
RIL_CDMA_CallWaiting_v6 *ril_utf_androidTel_if::update_default_ril_unsol_cdma_call_waiting(void)
{
  payload = qcril_malloc(sizeof(RIL_CDMA_CallWaiting_v6) * 1);
  payload_len = sizeof(RIL_CDMA_CallWaiting_v6);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_CDMA_CALL_WAITING;

  RIL_CDMA_CallWaiting_v6 *ptr = (RIL_CDMA_CallWaiting_v6*) payload;
  return ptr;
}
#if 0
RIL_CDMA_CallWaiting_v6* ril_utf_androidTel_if::malloc_ril_request_unsol_ss_resp()
{
  RIL_CDMA_CallWaiting_v6 *ptr= (RIL_CDMA_CallWaiting_v6 *) malloc(sizeof(RIL_CDMA_CallWaiting_v6) * 1);
  memset(ptr, 0, sizeof(RIL_CDMA_CallWaiting_v6));
  return ptr;
}
#endif
/*========================================================
  RIL_UNSOL_CDMA_OTA_PROVISION_STATUS
========================================================*/
void ril_utf_androidTel_if::update_default_ril_unsol_cdma_ota_provision_status(RIL_CDMA_OTA_ProvisionStatus data)
{
  payload = qcril_malloc(sizeof(RIL_CDMA_OTA_ProvisionStatus) * 1);
  payload_len = sizeof(RIL_CDMA_OTA_ProvisionStatus);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_CDMA_OTA_PROVISION_STATUS;

  RIL_CDMA_OTA_ProvisionStatus *ptr = (RIL_CDMA_OTA_ProvisionStatus*) payload;
  *ptr = data;
}
/*========================================================
  RIL_UNSOL_CDMA_INFO_REC
========================================================*/
RIL_CDMA_InformationRecords *ril_utf_androidTel_if::update_default_ril_unsol_cdma_info_rec(void)
{
  payload = qcril_malloc(sizeof(RIL_CDMA_InformationRecords) * 1);
  payload_len = sizeof(RIL_CDMA_InformationRecords);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_CDMA_INFO_REC;

  RIL_CDMA_InformationRecords *ptr = (RIL_CDMA_InformationRecords*) payload;
  return ptr;
}

/*========================================================
  RIL_REQUEST_GET_ACTIVITY_INFO
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_get_activity_info_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_GET_ACTIVITY_INFO;
}

void ril_utf_androidTel_if::update_default_ril_request_get_activity_info_resp(void)
{
  payload = malloc(sizeof(RIL_ActivityStatsInfo));
  payload_len = sizeof(RIL_ActivityStatsInfo);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_GET_ACTIVITY_INFO;
  error = RIL_E_SUCCESS;
}

/*========================================================
  RIL_REQUEST_SET_LTE_BAND_PREF
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_set_lte_band_pref_req(int* data)
{
  payload_len = (data[0]+1)*sizeof(int);
  payload = malloc(payload_len);
  memcpy(payload, (void*)data, payload_len);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SET_LTE_BAND_PREF;
}

void ril_utf_androidTel_if::update_default_ril_request_set_lte_band_pref_resp(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_LTE_BAND_PREF;
  error = RIL_E_SUCCESS;
}

void ril_utf_androidTel_if::update_default_ril_request_set_lte_band_pref_resp(RIL_Errno err)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SET_LTE_BAND_PREF;
  error = err;
}

/*========================================================
  RIL_REQUEST_NV_RESET_CONFIG
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_nv_reset_config()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_NV_RESET_CONFIG;
}
void ril_utf_androidTel_if::update_default_ril_request_nv_reset_config_resp()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_NV_RESET_CONFIG;
}

/*========================================================
  RIL_REQUEST_SHUTDOWN
========================================================*/
void ril_utf_androidTel_if::update_default_ril_request_shutdown()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_SHUTDOWN;
}
void ril_utf_androidTel_if::update_default_ril_request_shutdown_resp()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_SHUTDOWN;
}

/*
    RIL_REQUEST_START_NETWORK_SCAN
*/
RIL_NetworkScanRequest *ril_utf_androidTel_if::update_default_ril_request_start_network_scan(RIL_NetworkScanRequest *req)
{
  payload = malloc(sizeof(RIL_NetworkScanRequest));
  payload_len = sizeof(RIL_NetworkScanRequest);
  memset(payload, 0x0, payload_len);
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_START_NETWORK_SCAN;
  if (req) {
    memcpy(payload, req, payload_len);
  }
  RIL_NetworkScanRequest *ptr = (RIL_NetworkScanRequest*) payload;
  return ptr;
}
void ril_utf_androidTel_if::update_default_ril_request_start_network_scan_resp()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_START_NETWORK_SCAN;
}

/*
    RIL_REQUEST_STOP_NETWORK_SCAN
*/
void ril_utf_androidTel_if::update_default_ril_request_stop_network_scan()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_STOP_NETWORK_SCAN;
}
void ril_utf_androidTel_if::update_default_ril_request_stop_network_scan_resp()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_STOP_NETWORK_SCAN;
}

/*
    RIL_UNSOL_NETWORK_SCAN_RESULT
*/
RIL_NetworkScanResult *ril_utf_androidTel_if::update_default_ril_unsol_network_scan_result(void)
{
  payload = qcril_malloc(sizeof(RIL_NetworkScanResult) * 1);
  payload_len = sizeof(RIL_NetworkScanResult);
  payload_type = ril_utf_ril_unsol_response;
  msg_id = RIL_UNSOL_NETWORK_SCAN_RESULT;

  RIL_NetworkScanResult *ptr = (RIL_NetworkScanResult*) payload;
  return ptr;
}

/*========================================================
  RIL_REQUEST_ENABLE_5G
========================================================*/
void ril_utf_androidTel_if::update_default_enable_5g()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_ENABLE_5G;
}
void ril_utf_androidTel_if::update_default_enable_5g_resp()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_ENABLE_5G;
}

/*========================================================
  RIL_REQUEST_ENABLE_MODEM
========================================================*/
int* ril_utf_androidTel_if::update_default_enable_modem()
{
  payload = malloc(sizeof(int));
  payload_len = sizeof(int) ;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_ENABLE_MODEM;

  int* ptr = (int*) payload;
  return ptr;
}
void ril_utf_androidTel_if::fill_enable_modem(int *ptr, bool on)
{
    *ptr = on ? 1 : 0;
}
void ril_utf_androidTel_if::update_default_enable_modem_resp(RIL_Errno err)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_ENABLE_MODEM;
  error = err;
}

/*========================================================
  RIL_REQUEST_GET_MODEM_STACK_STATUS
========================================================*/
void ril_utf_androidTel_if::update_default_get_modem_stack_status()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ril_request;
  msg_id = RIL_REQUEST_GET_MODEM_STACK_STATUS;
}
void ril_utf_androidTel_if::update_default_get_modem_stack_status_resp(bool on)
{
  payload = malloc(sizeof(int));
  payload_len = sizeof(int);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_ril_response;
  msg_id = RIL_REQUEST_GET_MODEM_STACK_STATUS;
  int *ptr = (int*)payload;
  *ptr = on ? 1 : 0;
  error = RIL_E_SUCCESS;
}
