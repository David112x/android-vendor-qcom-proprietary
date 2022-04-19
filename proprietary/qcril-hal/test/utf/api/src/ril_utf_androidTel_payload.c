/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_androidTel_payload.c
  @brief   RIL.h allocation functions for test case dev

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_androidTel_payload.h"
#include <string.h>


void free_ril_request_get_sim_status_resp
(
   RIL_CardStatus_v6 *cardstatus
);
/*
  RIL_REQUEST_GET_CURRENT_CALLS (resp)
*/
RIL_Call** copy_ril_request_get_current_calls_resp(RIL_Call** call_list, int len)
{
  if (call_list == NULL)
  {
    // No active calls
    return NULL;
  }

  int x;

  int num_calls = (len / sizeof(RIL_Call*));

  // Malloc out new array
  RIL_Call** ret = (RIL_Call**) malloc(sizeof(RIL_Call*) * num_calls);

  for (x = 0; x < num_calls; ++x)
  {
    ret[x] = (RIL_Call*)malloc(sizeof(RIL_Call));

    memcpy(ret[x], call_list[x], sizeof(RIL_Call));

    if (call_list[x]->number != NULL)
    {
      int num_len = strlen(call_list[x]->number) + 1;
      ret[x]->number = (char*) malloc(num_len);
      strlcpy(ret[x]->number, call_list[x]->number, num_len);
    }

    if (call_list[x]->name != NULL)
    {
      int name_len = strlen(call_list[x]->name) + 1;
      ret[x]->name = (char*) malloc(name_len);
      strlcpy(ret[x]->name, call_list[x]->name, name_len);
    }

    if (call_list[x]->uusInfo != NULL)
    {
      ret[x]->uusInfo = (RIL_UUS_Info*) malloc(sizeof(RIL_UUS_Info));
      memcpy(ret[x]->uusInfo, call_list[x]->uusInfo, sizeof(RIL_UUS_Info));

      if (call_list[x]->uusInfo->uusData != NULL)
      {
        int data_len = strlen(call_list[x]->uusInfo->uusData);
        ret[x]->uusInfo->uusData = (char*) malloc(data_len);
        strlcpy(ret[x]->uusInfo->uusData, call_list[x]->uusInfo->uusData, data_len); // the pointer equivalent of a tongue twister
      }
    }

  } // end for

  return ret;
}
void free_ril_request_get_current_calls_resp(RIL_Call** call_list, int len)
{
  if (call_list == NULL)
  {
    // No active calls
    return;
  }

  int x;
  int num_calls = (len / sizeof(RIL_Call*));


  for (x = 0; x < num_calls; ++x)
  {
    if (call_list[x]->number != NULL)
    {
      free(call_list[x]->number);
    }

    if (call_list[x]->name != NULL)
    {
      free(call_list[x]->name);
    }

    if (call_list[x]->uusInfo != NULL)
    {
      if (call_list[x]->uusInfo->uusData != NULL)
      {
        free(call_list[x]->uusInfo->uusData);
      }
      free(call_list[x]->uusInfo);
    }

    free(call_list[x]);
  } // end for
}

RIL_LastCallFailCauseInfo* copy_ril_request_last_call_fail_cause_resp(RIL_LastCallFailCauseInfo* causeInfo)
{
  RIL_LastCallFailCauseInfo* ret = (RIL_LastCallFailCauseInfo*) malloc(sizeof(RIL_LastCallFailCauseInfo));
  ret->cause_code = causeInfo->cause_code;
  RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: causeInfo->vendor_cause = %p\n", causeInfo->vendor_cause);
  if (causeInfo->vendor_cause)
  {
    ret->vendor_cause = malloc(strlen(causeInfo->vendor_cause)+1);
    strlcpy(ret->vendor_cause, causeInfo->vendor_cause, strlen(causeInfo->vendor_cause)+1);
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: causeInfo->vendor_cause = %s\n", causeInfo->vendor_cause);
  }
  return ret;
}

enum ril_utf_expectation_t verify_ril_last_call_fail_cause(void *exp_node, int exp_len, void *data, int data_len)
{
  enum ril_utf_expectation_t result = met;
  RIL_LastCallFailCauseInfo* expect = (RIL_LastCallFailCauseInfo*) exp_node;
  RIL_LastCallFailCauseInfo* recv = (RIL_LastCallFailCauseInfo*) data;

  if (expect->cause_code != recv->cause_code)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: expected cause_code = %d, received cause_code = %d\n",
            expect->cause_code, recv->cause_code);
    printf("\n QMI_RIL_UTF: expected cause_code = %d, received cause_code = %d\n",
            expect->cause_code, recv->cause_code);
    result = not_met;
  }
  if (recv->vendor_cause)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: recv->vendor_cause = %s\n", recv->vendor_cause);
    if (expect->vendor_cause)
    {
      if ((strcmp(expect->vendor_cause, recv->vendor_cause) != 0))
      {
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: expected vendor_cause = %s\n", expect->vendor_cause);
        result = not_met;
      }
      else
      {
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: vendor_cause matches :-)\n");
      }
    }
  }

  return result;
}

enum ril_utf_expectation_t verify_ril_get_mute(void *exp_node, int exp_len, void *data, int data_len)
{
  enum ril_utf_expectation_t result = met;
  ril_request_mute_t* expect = (ril_request_mute_t*) exp_node;
  ril_request_mute_t* recv = (ril_request_mute_t*) data;

  if (expect->enable != recv->enable)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: expected enable = %d, received enable = %d\n",
            expect->enable, recv->enable);
    result = not_met;
  }
  return result;
}

enum ril_utf_expectation_t verify_ril_cdma_query_roaming_preference(void *exp_node, int exp_len, void *data, int data_len)
{
  enum ril_utf_expectation_t result = met;
  ril_request_cdma_query_roaming_preference_resp_t* expect =
      (ril_request_cdma_query_roaming_preference_resp_t*) exp_node;
  ril_request_cdma_query_roaming_preference_resp_t* recv =
      (ril_request_cdma_query_roaming_preference_resp_t*) data;

  if (expect->roaming_preference != recv->roaming_preference)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: expected roaming_preference = %d, received roaming_preference = %d\n",
            expect->roaming_preference, recv->roaming_preference);
    result = not_met;
  }
  return result;
}

enum ril_utf_expectation_t verify_ril_cdma_get_subscription_source(void *exp_node, int exp_len, void *data, int data_len)
{
  enum ril_utf_expectation_t result = met;
  RIL_CdmaSubscriptionSource* expect = (RIL_CdmaSubscriptionSource*) exp_node;
  RIL_CdmaSubscriptionSource* recv = (RIL_CdmaSubscriptionSource*) data;

  if (*expect != *recv)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: expected source = %d, received source = %d\n",
            *expect, *recv);
    result = not_met;
  }
  return result;
}

void print_RIL_Call(const RIL_Call *c)
{
  RIL_UTF_ESSENTIAL("\nstate = %d, index = %d, als = %d, isMpty = %d, isMT = %d, isVoice = %d,"
                    " isVoicePrivacy = %d, numberPresentation = %d, number = %s,"
                    " namePresentation = %d, name = %s, toa = %d",
                    c->state, c->index, c->als, c->isMpty, c->isMT, c->isVoice,
                    c->isVoicePrivacy, c->numberPresentation, (c->number ? c->number : "<empty>"),
                    c->namePresentation, (c->name ? c->name : "<empty>"), c->toa);
}

enum ril_utf_expectation_t verify_ril_get_current_calls(void *exp_node, int exp_len, void *data, int data_len)
{
  int i;
  enum ril_utf_expectation_t result = met;
  RIL_Call** expect = (RIL_Call**) exp_node;
  RIL_Call** recv = (RIL_Call**) data;

  if (exp_len != data_len)
  {
    result = not_met;
    return result;
  }

  // For each call
  int num_calls = (exp_len / sizeof(RIL_Call*));
#ifdef UTF_TARGET_BUILD
  for (i = 0; i < num_calls; ++i)
  {
    if ( expect[i]->state != recv[i]->state ||
         expect[i]->index != recv[i]->index ||
         expect[i]->isMT != recv[i]->isMT ||
         expect[i]->isVoice != recv[i]->isVoice )
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: For Call id %d", expect[i]->index);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: For Received Call id %d", recv[i]->index);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Expected");
      print_RIL_Call(expect[i]);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Recvd");
      print_RIL_Call(recv[i]);
      printf("\n QMI_RIL_UTF: Expected call state %d, received %d", expect[i]->state, recv[i]->state);
      result = not_met;
      break;
    }
  }

#else
  for (i = 0; i < num_calls; ++i)
  {
    if ( expect[i]->state != recv[i]->state ||
         expect[i]->index != recv[i]->index ||
         expect[i]->als != recv[i]->als ||
         expect[i]->isMpty != recv[i]->isMpty ||
         expect[i]->isMT != recv[i]->isMT ||
         expect[i]->isVoice != recv[i]->isVoice ||
         expect[i]->isVoicePrivacy != recv[i]->isVoicePrivacy ||
         expect[i]->numberPresentation != recv[i]->numberPresentation ||
         expect[i]->namePresentation != recv[i]->namePresentation ||
         expect[i]->toa != recv[i]->toa )
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: For Call id %d", expect[i]->index);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: For Received Call id %d", recv[i]->index);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Expected");
      print_RIL_Call(expect[i]);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Recvd");
      print_RIL_Call(recv[i]);
      result = not_met;
      break;
    }
    if ( (expect[i]->number != NULL) &&
         (strcmp(expect[i]->number, recv[i]->number) != 0) )
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: For Call id %d", expect[i]->index);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: For Received Call id %d", recv[i]->index);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Expected number: %s", expect[i]->number);
      print_msg_in_hex( expect[i]->number, strlen(expect[i]->number));
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Recvd number: %s", recv[i]->number);
      print_msg_in_hex( recv[i]->number, strlen(recv[i]->number));
      result = not_met;
      break;
    }
  }

  free_ril_request_get_current_calls_resp(expect, exp_len);
#endif
  free_ril_request_get_current_calls_resp(recv, exp_len);

  return result;
}

/*
  RIL_REQUEST_OPERATOR
*/
ril_request_operator_resp_from_qcril_t* copy_ril_request_operator_resp
  (char** opr)
{
  ril_request_operator_resp_from_qcril_t *ptr = (ril_request_operator_resp_from_qcril_t*)malloc(sizeof(ril_request_operator_resp_from_qcril_t));

  memset(ptr->operator_info_array, 0, NAS_OPERATOR_RESP_MAX_ARR_SIZE);

  if (opr == NULL)
  {
    return ptr;
  }

  ril_request_operator_resp_t *operator_resp = (ril_request_operator_resp_t*)malloc(sizeof(ril_request_operator_resp_t));
  memset(operator_resp, '\0', sizeof(ril_request_operator_resp_t));

  if (opr[0] != NULL)
    memcpy(operator_resp->long_eons, opr[0], sizeof(operator_resp->long_eons));
  if (opr[1] != NULL)
    memcpy(operator_resp->short_eons, opr[1], sizeof(operator_resp->short_eons));
  if (opr[2] != NULL)
    memcpy(operator_resp->mcc_mnc_ascii, opr[2], sizeof(operator_resp->mcc_mnc_ascii));

  ptr->operator_info_array[0] = operator_resp->long_eons;
  ptr->operator_info_array[1] = operator_resp->short_eons;
  ptr->operator_info_array[2] = operator_resp->mcc_mnc_ascii;

  return ptr;
}

void free_ril_request_operator_resp(ril_request_operator_resp_from_qcril_t* ptr)
{
  // the char** array is malloced with a hack and must be freed the same way
  // (see above deep copy function and related function in ril_utf_androidTel_if.cpp)
  ril_request_operator_resp_t *tmp_free_ptr = (ril_request_operator_resp_t*)ptr->operator_info_array[0];

  free(tmp_free_ptr);
}

enum ril_utf_expectation_t verify_ril_operator_name(void *exp_node, void *data)
{
  enum ril_utf_expectation_t result = met; // start with met and mark unmet if failure

  ril_request_operator_resp_from_qcril_t *expect = (ril_request_operator_resp_from_qcril_t *) exp_node;
  ril_request_operator_resp_from_qcril_t *recv = (ril_request_operator_resp_from_qcril_t *) data;

  if( (NULL == expect) || (NULL == recv) )
    return result;

  if ( (NULL != expect->operator_info_array[0]) && (NULL != recv->operator_info_array[0]) && ( strcmp(expect->operator_info_array[0], recv->operator_info_array[0]) != 0 ) )
  {
    result = not_met;
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Expected Long EONS %s received Long EONS %s", expect->operator_info_array[0], recv->operator_info_array[0]);
  }

  if ( (NULL != expect->operator_info_array[1]) && (NULL != recv->operator_info_array[1]) && (strcmp(expect->operator_info_array[1], recv->operator_info_array[1]) != 0) )
  {
    result = not_met;
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Expected Short EONS %d received Short EONS %d", expect->operator_info_array[1], recv->operator_info_array[1]);
  }

  if ( (NULL != expect->operator_info_array[2]) && (NULL != recv->operator_info_array[2]) && (strcmp(expect->operator_info_array[2], recv->operator_info_array[2]) != 0) )
  {
    result = not_met;
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Expected MCC_MNC %d received MCC_MNC %d", expect->operator_info_array[2], recv->operator_info_array[2]);
  }

  free_ril_request_operator_resp(exp_node);
  free_ril_request_operator_resp(data);

  return result;
}


/*
  RIL_REQUEST_QUERY_AVAILABLE_NETWORKS
*/
ril_request_query_available_networks_resp_t* copy_ril_request_query_available_networks_resp(char** networks, int len)
{
  int i=0,j=0;
  int nw_cnt,param=0;
  int x=0;
  char prop_str[ QCRIL_TEST_PROPERTY_VALUE_LEN ];

  ril_request_query_available_networks_resp_t *ptr =
    (ril_request_query_available_networks_resp_t *)malloc(sizeof(ril_request_query_available_networks_resp_t));
  memset(ptr,0,sizeof(ril_request_query_available_networks_resp_t));

  if (networks == NULL)
  {
    return ptr;
  }

  memset(prop_str, '\0', QCRIL_TEST_PROPERTY_VALUE_LEN);
  property_get( "persist.vendor.radio.rat_on", prop_str, "" );
  if(!strcmp(prop_str,"legacy"))
    param = 5;
  else
    param = 4;

  // NW count
  nw_cnt = len / sizeof(char*) / param;

  qcril_qmi_nw_scan_resp_helper_type *ril_resp_helper =
    (qcril_qmi_nw_scan_resp_helper_type *)malloc(nw_cnt * sizeof(qcril_qmi_nw_scan_resp_helper_type));

  memset(ril_resp_helper,0,sizeof(*ril_resp_helper));

  for(i=0;i<nw_cnt;i++)
    {
      ptr->nw_scan_info_array[j++] = ril_resp_helper[i].long_eons;
      strlcpy(ril_resp_helper[i].long_eons, networks[x++], sizeof(ril_resp_helper[i].long_eons));
      ptr->nw_scan_info_array[j++] = ril_resp_helper[i].short_eons;
      strlcpy(ril_resp_helper[i].short_eons, networks[x++], sizeof(ril_resp_helper[i].short_eons));
      ptr->nw_scan_info_array[j++] = ril_resp_helper[i].mccmnc_info;
      strlcpy(ril_resp_helper[i].mccmnc_info, networks[x++], sizeof(ril_resp_helper[i].mccmnc_info));
      ptr->nw_scan_info_array[j++] = ril_resp_helper[i].network_status;
      strlcpy(ril_resp_helper[i].network_status, networks[x++], sizeof(ril_resp_helper[i].network_status));

      if(5 == param)
      {
        ptr->nw_scan_info_array[j++] = ril_resp_helper[i].rat;
        strlcpy(ril_resp_helper[i].rat, networks[x++], sizeof(ril_resp_helper[i].rat));
      }
    }


  return ptr;
}

void free_ril_request_query_available_networks_resp(ril_request_query_available_networks_resp_t* ptr)
{
  // the char** array is malloced with a hack and must be freed the same way
  // (see above deep copy function and related function in ril_utf_androidTel_if.cpp)
  qcril_qmi_nw_scan_resp_helper_type *tmp_free_ptr = (qcril_qmi_nw_scan_resp_helper_type *)ptr->nw_scan_info_array[0];

  free(tmp_free_ptr);
}
enum ril_utf_expectation_t verify_ril_query_available_network(void *exp_node, void *data)
{
  int i = 0;
  enum ril_utf_expectation_t result = met; // start with met and mark unmet if failure

  ril_request_query_available_networks_resp_t *expect = (ril_request_query_available_networks_resp_t *) exp_node;
  ril_request_query_available_networks_resp_t *recv = (ril_request_query_available_networks_resp_t *) data;

  if((NULL == expect) || (NULL == recv))
  {
    return result;
  }

  for( i = 0; i< (NAS_NW_SCAN_RESP_ENTRY_SIZE * NAS_3GPP_NETWORK_INFO_LIST_MAX_V01); i++)
  {
    if( (NULL == expect->nw_scan_info_array[i]) && (NULL == recv->nw_scan_info_array[i]) )
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Data matched");
      break;
    }
    else if ( ( (NULL == expect->nw_scan_info_array[i]) && (NULL != recv->nw_scan_info_array[i])) ||
              ( (NULL != expect->nw_scan_info_array[i]) && (NULL == recv->nw_scan_info_array[i]) ) )
    {
        result = not_met;
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Expected nw_scan info and received nw_scan NOT matched");
        break;
    }

    if ( (NULL != expect->nw_scan_info_array[i]) && (NULL != recv->nw_scan_info_array[i]) && ( strcmp(expect->nw_scan_info_array[i], recv->nw_scan_info_array[i]) != 0 ) )
    {
      result = not_met;
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Expected nw_scan info%s received nw_scan info %s", expect->nw_scan_info_array[i], recv->nw_scan_info_array[i]);
      break;
    }
  }
  free_ril_request_query_available_networks_resp(expect);
  free_ril_request_query_available_networks_resp(recv);

  return result;
}

char** copy_ril_unsol_on_ussd(char** data, int datalen)
{
    char **unsol_data = (char **)malloc(sizeof(char*)*2);

    if (unsol_data)
    {
        memset (unsol_data, 0, 2);

        if (data[0])
        {
            int len = strlen(data[0])+1;
            unsol_data[0] = malloc(len);
            if (unsol_data[0])
            {
                strlcpy(unsol_data[0], data[0], len);
            }
        }
        if (data[1])
        {
            int len = strlen(data[1])+1;
            unsol_data[1] = malloc(len);
            if (unsol_data[1])
            {
                strlcpy(unsol_data[1], data[1], len);
            }
        }
    }

    return unsol_data;
}

enum ril_utf_expectation_t verify_ril_unsol_on_ussd(void *exp_node, void *data)
{
    enum ril_utf_expectation_t result = met;

    char** expect = (char**) exp_node;
    char** recv = (char**) data;

    if (expect && recv)
    {
       char *exp_code = expect[0];
       char *recv_code = recv[0];
       RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: exp_code = %s\n", exp_code);
       RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: recv_code = %s\n", recv_code);
       if (exp_code && recv_code &&
               (strcmp(exp_code, recv_code) != 0))
       {
          result = not_met;
       }
       char *exp_msg = expect[1];
       char *recv_msg = recv[1];
       if (exp_msg && recv_msg)
       {
         RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: exp_msg = %s\n", exp_msg);
         RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: recv_msg = %s\n", recv_msg);
         if (strcmp(exp_msg, recv_msg) != 0)
         {
           result = not_met;
         }
       }
    }

    return result;
}

enum ril_utf_expectation_t verify_ril_unsol_on_ss(void *exp_node, void *data)
{
    enum ril_utf_expectation_t result = met;

    RIL_StkCcUnsolSsResponse* expect = (RIL_StkCcUnsolSsResponse*) exp_node;
    RIL_StkCcUnsolSsResponse* recv = (RIL_StkCcUnsolSsResponse*) data;

    if( (expect->serviceType != recv->serviceType ) ||
        (expect->requestType != recv->requestType ) ||
        (expect->teleserviceType != recv->teleserviceType ) ||
        (expect->serviceClass != recv->serviceClass ) ||
        (expect->result != recv->result )
      )
    {
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Recvd and Expected mismatch unsol on ss");
        result = not_met;
    }

    return result;
}

enum ril_utf_expectation_t verify_ril_unsol_supp_svc_notification(void *exp_node, void *data)
{
  enum ril_utf_expectation_t result = met;

  RIL_SuppSvcNotification* expect = (RIL_SuppSvcNotification*) exp_node;
  RIL_SuppSvcNotification* recv = (RIL_SuppSvcNotification*) data;

  do {
    if (expect->notificationType != recv->notificationType)
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: mismatch notificationType: exp: %d, recv: %d",
          expect->notificationType, recv->notificationType);
      result = not_met;
      break;
    }
    if (expect->code != recv->code)
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: mismatch code: exp: %d, recv: %d",
          expect->code, recv->code);
      result = not_met;
      break;
    }
    if (expect->index != recv->index)
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: mismatch index: exp: %d, recv: %d",
          expect->index, recv->index);
      result = not_met;
      break;
    }
    if (expect->type != recv->type)
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: mismatch type: exp: %d, recv: %d",
          expect->type, recv->type);
      result = not_met;
      break;
    }
    if ((expect->number && recv->number) &&
        (strcmp(expect->number, recv->number) != 0))
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: mismatch number: exp: %s, recv: %s",
          expect->number, recv->number);
      result = not_met;
      break;
    }
  } while (0);

  return result;
}

enum ril_utf_expectation_t verify_ril_unsol_cdma_call_waiting(void *exp_node, void *data)
{
  enum ril_utf_expectation_t result = met;

  RIL_CDMA_CallWaiting_v6* expect = (RIL_CDMA_CallWaiting_v6*) exp_node;
  RIL_CDMA_CallWaiting_v6* recv = (RIL_CDMA_CallWaiting_v6*) data;

  if((expect->numberPresentation != recv->numberPresentation) ||
      (expect->number_type != recv->number_type) ||
      (expect->number_plan != recv->number_plan))
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Recvd and Expected mismatch");
    result = not_met;
  }
  // TODO: check number, name & signalInfoRecord

  return result;
}

enum ril_utf_expectation_t verify_ril_unsol_cdma_ota_provision_status(void *exp_node, void *data)
{
  enum ril_utf_expectation_t result = met;

  RIL_CDMA_OTA_ProvisionStatus* expect = (RIL_CDMA_OTA_ProvisionStatus*) exp_node;
  RIL_CDMA_OTA_ProvisionStatus* recv = (RIL_CDMA_OTA_ProvisionStatus*) data;

  if (expect && recv)
  {
    if (*expect != *recv)
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: mismatch Recvd %d, and Expected %d", *recv, *expect);
      result = not_met;
    }
  }

  return result;
}

enum ril_utf_expectation_t verify_ril_unsol_cdma_info_rec(void *exp_node, void *data)
{
  enum ril_utf_expectation_t result = met;

  RIL_CDMA_InformationRecords* expect = (RIL_CDMA_InformationRecords*) exp_node;
  RIL_CDMA_InformationRecords* recv = (RIL_CDMA_InformationRecords*) data;

  if((expect->numberOfInfoRecs != recv->numberOfInfoRecs))
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Recvd and Expected mismatch");
    result = not_met;
  }
  // TODO: check infoRec

  return result;
}

enum ril_utf_expectation_t verify_ril_unsol_ringback_tone(void *exp_node, void *data)
{
  enum ril_utf_expectation_t result = met;

  ril_unsol_ringback_tone_t* expect = (ril_unsol_ringback_tone_t*) exp_node;
  ril_unsol_ringback_tone_t* recv = (ril_unsol_ringback_tone_t*) data;

  if((expect->start != recv->start))
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: mismatch start: exp: %d, recv: %d",
        expect->start, recv->start);
    result = not_met;
  }

  return result;
}

enum ril_utf_expectation_t verify_ril_unsol_network_scan_result(void *exp_node, void *data)
{
  enum ril_utf_expectation_t result = met;

  RIL_NetworkScanResult* expect = (RIL_NetworkScanResult*) exp_node;
  RIL_NetworkScanResult* recv = (RIL_NetworkScanResult*) data;

  if((expect->status != recv->status))
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: mismatch status: exp: %d, recv: %d",
        expect->status, recv->status);
    result = not_met;
  }
  // TODO: Validate other parameters

  return result;
}

enum ril_utf_expectation_t verify_ril_get_neighboring_cell_ids(
 void *exp_node, int exp_len, void *data, int data_len)
{
  enum ril_utf_expectation_t result = met;

  RIL_NeighboringCell **expect = (RIL_NeighboringCell **) exp_node;
  RIL_NeighboringCell **recv = (RIL_NeighboringCell **) data;

  if (exp_len != data_len)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Failed, Expected len = %d, Received len = %d\n",
        exp_len, data_len);
    result = not_met;
    return result;
  }

  // For each entry
  int num_info = (exp_len / sizeof(RIL_NeighboringCell*));

  for (int i = 0; i < num_info; ++i)
  {
    if (expect[i]->rssi != recv[i]->rssi)
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: For index %d, mismatch rssi %d, %d",
                    i, expect[i]->rssi, recv[i]->rssi);
      result = not_met;
      break;
    }
    if ( (expect[i]->cid != NULL) &&
         (strcmp(expect[i]->cid, recv[i]->cid) != 0) )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: For index %d, mismatch cid %s, %s",
                    i, expect[i]->cid, recv[i]->cid);
      result = not_met;
      break;
    }
  }

  return result;
}


enum ril_utf_expectation_t verify_ril_send_sms(void *exp_node, void *data)
{
  enum ril_utf_expectation_t result = met;
  RIL_SMS_Response *expect = (RIL_SMS_Response *) exp_node;
  RIL_SMS_Response *recv = (RIL_SMS_Response  *) data;

  if (expect->ackPDU != recv->ackPDU)
  {
    result = not_met;
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Expected ackPDU %d received ackPDU %d", expect->ackPDU, recv->ackPDU);
  }

  if (expect->errorCode != recv->errorCode)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Expected errorCode %d received errorCode %d", expect->errorCode, recv->errorCode);
    result = not_met;
  }

  return result;
}


/*
  RIL_REQUEST_QUERY_CALL_FORWARD_STATUS
*/
RIL_CallForwardInfo **copy_ril_request_query_call_forward_status_resp
(
 RIL_CallForwardInfo **call_fwd_info_list, int len
)
{
  int i = 0;
  int num_info = 0;
  RIL_CallForwardInfo** ret = NULL;

  if (call_fwd_info_list == NULL)
  {
    return NULL;
  }


  num_info = (len / sizeof(RIL_CallForwardInfo *));

  ret = (RIL_CallForwardInfo**) malloc(sizeof(RIL_CallForwardInfo*) * num_info);

  for (i = 0; i < num_info; i++)
  {
    ret[i] = (RIL_CallForwardInfo *)malloc(sizeof(RIL_CallForwardInfo));
    memcpy(ret[i], call_fwd_info_list[i], sizeof (RIL_CallForwardInfo));
    if (call_fwd_info_list[i]->number != NULL)
    {
      int num_len = strlen(call_fwd_info_list[i]->number) + 1;
      ret[i]->number = (char*) malloc(num_len);
      strlcpy(ret[i]->number, call_fwd_info_list[i]->number, num_len);
    }
  }

  return ret;
}

void free_ril_request_query_call_forward_status_resp
(
 RIL_CallForwardInfo **call_fwd_info_list, int len
)
{
  int i = 0;
  int num_info = 0;

  if (call_fwd_info_list == NULL)
  {
    return;
  }

  num_info = (len / sizeof(RIL_CallForwardInfo*));

  for (i = 0; i < num_info; ++i)
  {
    if (call_fwd_info_list[i]->number != NULL)
    {
      free(call_fwd_info_list[i]->number);
    }
    free(call_fwd_info_list[i]);
  }
}

enum ril_utf_expectation_t verify_ril_query_call_forward_status
(
 void *exp_node, int exp_len, void *data, int data_len
)
{
  int i;
  enum ril_utf_expectation_t result = met;

  RIL_CallForwardInfo** expect = (RIL_CallForwardInfo**) exp_node;
  RIL_CallForwardInfo** recv = (RIL_CallForwardInfo**) data;

  if (exp_len != data_len)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Failed, Expected len = %d, Received len = %d\n",
        exp_len, data_len);
    result = not_met;
    return result;
  }

  // For each entry
  int num_info = (exp_len / sizeof(RIL_CallForwardInfo*));

  for (i = 0; i < num_info; ++i)
  {
    if ( expect[i]->status != recv[i]->status ||
         expect[i]->reason != recv[i]->reason ||
         expect[i]->serviceClass != recv[i]->serviceClass ||
         expect[i]->toa != recv[i]->toa ||
         expect[i]->timeSeconds != recv[i]->timeSeconds)
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: For index %d", i);
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected");
      print_msg_in_hex( expect[i], sizeof(RIL_Call));
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: Recvd");
      print_msg_in_hex( recv[i], sizeof(RIL_Call));
      result = not_met;
      break;
    }
    if ( (expect[i]->number != NULL) &&
         (strcmp(expect[i]->number, recv[i]->number) != 0) )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: For index %d", i);
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected number");
      print_msg_in_hex( expect[i]->number, strlen(expect[i]->number));
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: Recvd number");
      print_msg_in_hex( recv[i]->number, strlen(recv[i]->number));
      result = not_met;
      break;
    }
  }

  free_ril_request_query_call_forward_status_resp(expect, exp_len);

  return result;
}

enum ril_utf_expectation_t verify_ril_get_sim_status(void *exp_node, int exp_len, void *data, int data_len)
{
  int i;
  enum ril_utf_expectation_t result = met;
  RIL_CardStatus_v6* expect = (RIL_CardStatus_v6*) exp_node;
  RIL_CardStatus_v6* recv = (RIL_CardStatus_v6*) data;

  if (exp_len != data_len)
  {
    result = not_met;
    return result;
  }
  if (expect->card_state != recv->card_state)
  {
    result = not_met;
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected card state %d, received card state %d",
                  expect->card_state, recv->card_state);
    return result;
  }
  if( (expect->universal_pin_state != recv->universal_pin_state) ||
       (expect->gsm_umts_subscription_app_index != recv->gsm_umts_subscription_app_index) ||
       (expect->cdma_subscription_app_index != recv->cdma_subscription_app_index) ||
       (expect->ims_subscription_app_index != recv->ims_subscription_app_index) )
  {
     result = not_met;
     RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected univ_pin_state %d, received pin state %d",
                   expect->universal_pin_state, recv->universal_pin_state);
     RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected gsm_umts_subscription_app_index %d, received %d",
                   expect->gsm_umts_subscription_app_index, recv->gsm_umts_subscription_app_index);
     RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected cdma_subscription_app_index %d, received %d",
                   expect->cdma_subscription_app_index, recv->cdma_subscription_app_index);
     RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected ims_subscription_app_index %d, received %d",
                   expect->ims_subscription_app_index, recv->ims_subscription_app_index);
     /*
     RIL_UTF_HTML_LOG("\n QMI_RIL_UTF: Expected univ_pin_state %d, received pin state %d",
                   expect->universal_pin_state, recv->universal_pin_state);
     RIL_UTF_HTML_LOG("\n QMI_RIL_UTF: Expected gsm_umts_subscription_app_index %d, received %d",
                   expect->gsm_umts_subscription_app_index, recv->gsm_umts_subscription_app_index);
     RIL_UTF_HTML_LOG("\n QMI_RIL_UTF: Expected cdma_subscription_app_index %d, received %d",
                   expect->cdma_subscription_app_index, recv->cdma_subscription_app_index);
     RIL_UTF_HTML_LOG("\n QMI_RIL_UTF: Expected ims_subscription_app_index %d, received %d",
                   expect->ims_subscription_app_index, recv->ims_subscription_app_index);
     */
  }
  for (i = 0; i < RIL_CARD_MAX_APPS; i++) //check each app in the card
  {
      if ( (expect->applications[i].app_state != recv->applications[i].app_state ||
            expect->applications[i].app_type != recv->applications[i].app_type ||
            expect->applications[i].perso_substate != recv->applications[i].perso_substate ||
            expect->applications[i].pin1 != recv->applications[i].pin1 ||
            expect->applications[i].pin1_replaced != recv->applications[i].pin1_replaced ||
            expect->applications[i].pin2 != recv->applications[i].pin2))
      {
         result = not_met;
         RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected app[%d] app_state %d, received %d",
                       i, expect->applications[i].app_state, recv->applications[i].app_state);
         RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected app[%d] app_type %d, received %d",
                       i, expect->applications[i].app_type, recv->applications[i].app_type);
         RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected app[%d] perso_substate %d, received %d",
                       i, expect->applications[i].perso_substate, recv->applications[i].perso_substate);
         RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected app[%d] pin1 %d, received %d",
                       i, expect->applications[i].pin1, recv->applications[i].pin1);
         RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected app[%d] pin1_replaced %d, received %d",
                       i, expect->applications[i].pin1_replaced, recv->applications[i].pin1_replaced);
         RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected app[%d] pin2 %d, received %d",
                       i, expect->applications[i].pin2, recv->applications[i].pin2);
         /*
         RIL_UTF_HTML_LOG("\n QMI_RIL_UTF: Expected app[%d] app_state %d, received %d",
                          i, expect->applications[i].app_state, recv->applications[i].app_state);
         RIL_UTF_HTML_LOG("\n QMI_RIL_UTF: Expected app[%d] app_type %d, received %d",
                          i, expect->applications[i].app_type, recv->applications[i].app_type);
         RIL_UTF_HTML_LOG("\n QMI_RIL_UTF: Expected app[%d] perso_substate %d, received %d",
                          i, expect->applications[i].perso_substate, recv->applications[i].perso_substate);
         RIL_UTF_HTML_LOG("\n QMI_RIL_UTF: Expected app[%d] pin1 %d, received %d",
                          i, expect->applications[i].pin1, recv->applications[i].pin1);
         RIL_UTF_HTML_LOG("\n QMI_RIL_UTF: Expected app[%d] pin1_replaced %d, received %d",
                          i, expect->applications[i].pin1_replaced, recv->applications[i].pin1_replaced);
         RIL_UTF_HTML_LOG("\n QMI_RIL_UTF: Expected app[%d] pin2 %d, received %d",
                          i, expect->applications[i].pin2, recv->applications[i].pin2);
         */
      }
      if ( (expect->applications[i].aid_ptr != NULL) ^ (recv->applications[i].aid_ptr != NULL))
      {
         result = not_met;
         RIL_UTF_DEBUG("\n QMI_RIL_UTF: App[%d] aid_ptr mismatch - expected %d, received %d",
                       i, expect->applications[i].aid_ptr != NULL, recv->applications[i].aid_ptr != NULL);
         RIL_UTF_HTML_LOG("\n QMI_RIL_UTF: App[%d] aid_ptr mismatch - expected %d, received %d",
                          i, expect->applications[i].aid_ptr != NULL, recv->applications[i].aid_ptr != NULL);
      }
      else if ( (expect->applications[i].aid_ptr != NULL) &&
           (recv->applications[i].aid_ptr != NULL) &&
           (strcmp(expect->applications[i].aid_ptr, recv->applications[i].aid_ptr) != 0))
      {
         result = not_met;
         RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected app[%d] AID did not match received\n Expected:\n", i);
         RIL_UTF_HTML_LOG("\n QMI_RIL_UTF: Expected app[%d] AID did not match received\n Expected:\n", i);
         print_msg_in_hex(expect->applications[i].aid_ptr, strlen(expect->applications[i].aid_ptr));
         RIL_UTF_DEBUG("Received:\n");
         RIL_UTF_HTML_LOG("Received:\n");
         print_msg_in_hex(recv->applications[i].aid_ptr, 32);//strlen(recv->applications[i].aid_ptr));
      }
      /*
      THIS HAS BEEN EXCLUDED DUE TO A MEMORY ALLOCATION ISSUE WITH HANDLING APP LABEL MESSAGES. THE VALUE DOES NOT
      CHANGE AFTER A GET_LABEL REQUEST/RESP WITH THE UTF IN ITS CURRENT STATE (as of July 2014), SO THE VALIDATION
      STEP IS SKIPPED.

      if ( (expect->applications[i].app_label_ptr != NULL) ^ (recv->applications[i].app_label_ptr != NULL))
      {
         result = not_met;
         RIL_UTF_DEBUG("\n QMI_RIL_UTF: app_label_ptr mismatch - expected %d, received %d",
                       expect->applications[i].app_label_ptr != NULL, recv->applications[i].app_label_ptr != NULL);
         RIL_UTF_HTML_LOG("\n QMI_RIL_UTF: app_label_ptr mismatch - expected %d, received %d",
                          expect->applications[i].app_label_ptr != NULL, recv->applications[i].app_label_ptr != NULL);
      }
      else if ( (expect->applications[i].app_label_ptr != NULL) &&
           (recv->applications[i].app_label_ptr != NULL) &&
           (strcmp(expect->applications[i].app_label_ptr, recv->applications[i].app_label_ptr) != 0))
      {
         result = not_met;
         RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected app[%d] app_label did not match received\n Expected:\n", i);
         RIL_UTF_HTML_LOG("\n QMI_RIL_UTF: Expected app[%d] app_label did not match received\n Expected:\n", i);
         print_msg_in_hex(expect->applications[i].app_label_ptr, strlen(expect->applications[i].app_label_ptr));
         RIL_UTF_DEBUG("Received:\n");
         RIL_UTF_HTML_LOG("Received:\n");
         print_msg_in_hex(recv->applications[i].app_label_ptr, strlen(recv->applications[i].app_label_ptr));
      } */
  }
  //free_ril_request_get_sim_status_resp(expect);
  //free_ril_request_get_sim_status_resp(recv);
  return result;
}
RIL_CardStatus_v6 *copy_ril_request_get_sim_status_resp
(
   RIL_CardStatus_v6 *cardstatus, int len
)
{
   if (cardstatus == NULL)
   {
      return NULL;
   }
   int i;
   RIL_CardStatus_v6 *ret = malloc(len);
   memcpy(ret, cardstatus, len);
   for (i = 0; i < RIL_CARD_MAX_APPS; i++)
   {
      if (cardstatus->applications[i].aid_ptr != NULL)
      {
         int size = strlen(cardstatus->applications[i].aid_ptr) + 1;
         ret->applications[i].aid_ptr = malloc(size);
         memcpy(ret->applications[i].aid_ptr, cardstatus->applications[i].aid_ptr, size);
      }
      if (cardstatus->applications[i].app_label_ptr != NULL)
      {
         int size = strlen(cardstatus->applications[i].app_label_ptr) + 1;
         ret->applications[i].app_label_ptr = malloc(size);
         memcpy(ret->applications[i].app_label_ptr, cardstatus->applications[i].app_label_ptr, size);
      }
   }
   return ret;
}
void free_ril_request_get_sim_status_resp
(
   RIL_CardStatus_v6 *cardstatus
)
{
   if (cardstatus != NULL)
   {
      int i;
      for (i = 0; i < RIL_CARD_MAX_APPS; i++)
      {
         if (cardstatus->applications[i].aid_ptr != NULL)
         {
            free(cardstatus->applications[i].aid_ptr);
         }
         if (cardstatus->applications[i].app_label_ptr != NULL)
         {
            free(cardstatus->applications[i].app_label_ptr);
         }
      }
   }
}
enum ril_utf_expectation_t verify_ril_query_tty_mode
(
 void *exp_node,
 int exp_len,
 void *data,
 int data_len
)
{
  enum ril_utf_expectation_t result = met;

  ril_request_get_tty_mode_t* expect = (ril_request_get_tty_mode_t*) exp_node;
  ril_request_get_tty_mode_t* recv = (ril_request_get_tty_mode_t*) data;

  if (expect->tty_mode != recv->tty_mode)
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: expected tty_mode = %d, received tty_mode = %d\n",
            expect->tty_mode, recv->tty_mode);
    printf("\n QMI_RIL_UTF: expected tty_mode = %d, received tty_mode = %d\n",
            expect->tty_mode, recv->tty_mode);
    result = not_met;
  }

  return result;
}
enum ril_utf_expectation_t verify_ril_query_preferred_voice_privacy_mode
(
 void *exp_node,
 int exp_len,
 void *data,
 int data_len
)
{
  enum ril_utf_expectation_t result = met;

  ril_request_cdma_query_preferred_voice_privacy_mode_t* expect =
      (ril_request_cdma_query_preferred_voice_privacy_mode_t*) exp_node;
  ril_request_cdma_query_preferred_voice_privacy_mode_t* recv =
      (ril_request_cdma_query_preferred_voice_privacy_mode_t*) data;

  if (expect->voice_privacy_mode != recv->voice_privacy_mode)
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: expected voice_privacy_mode = %d, received voice_privacy_mode = %d\n",
            expect->voice_privacy_mode, recv->voice_privacy_mode);
    printf("\n QMI_RIL_UTF: expected voice_privacy_mode = %d, received voice_privacy_mode = %d\n",
            expect->voice_privacy_mode, recv->voice_privacy_mode);
    result = not_met;
  }

  return result;
}

enum ril_utf_expectation_t verify_ril_open_channel_response(void *exp_node, int exp_len,
                             void *data, int data_len)
{

  enum ril_utf_expectation_t result = met;
  uint8_t*  exp_node_stream         = (uint8_t*)exp_node;
  uint8_t*  data_node_stream        = (uint8_t*)data;

  if(exp_len != data_len || memcmp(exp_node_stream, data_node_stream, data_len) != 0)
  {
    result = not_met;
  }
  return result;
}

