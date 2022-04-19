/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "ril_utf_message_printer.h"
#include "LteDirectDiscovery.pb.h"
#include "qcril_qmi_lte_direct_disc_msg_meta.h"
#ifdef QMI_RIL_HAL_UTF
#include "qcril_qmi_oem_eventlist.h"

#endif
#ifndef RIL_UTF_IOE
const char* lookup_ims_message_name(int msg_id)
{
  int i;
  for (i = 0; i < IMS_MESSAGE_SIZE; ++i)
  {
    if (ims_service_msg_string_table[i].number == msg_id)
    {
      return ims_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}

const char* lookup_ril_message_name(int msg_id)
{
  int i;
  for (i = 0; i < RIL_MESSAGE_SIZE; ++i)
  {
    if (ril_service_msg_string_table[i].number == msg_id)
    {
      return ril_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}
const char* lookup_oemhook_message_name(int msg_id)
{
  int i;
  for (i = 0; i < RIL_MESSAGE_SIZE; ++i)
  {
    if (oemhook_service_msg_string_table[i].number == msg_id)
    {
      return oemhook_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}
#endif

const char* lookup_dms_message_name(int msg_id)
{
  int i;
  for (i = 0; i < DMS_MESSAGE_SIZE; ++i)
  {
    if (device_management_service_msg_string_table[i].number == msg_id)
    {
      return device_management_service_msg_string_table[i].name;
    }
  }

//TODO: recursive calls of RIL_UTF_ESSENTIAL cause deadlock DO NOT USE
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}

const char* lookup_voice_message_name(int msg_id)
{
  int i;
  for (i = 0; i < VOICE_MESSAGE_SIZE; ++i)
  {
    if (voice_service_msg_string_table[i].number == msg_id)
    {
      return voice_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}

const char* lookup_lte_message_name(int msg_id)
{
  int i;
  for (i = 0; i < LTE_MESSAGE_SIZE; ++i)
  {
    if (lte_service_msg_string_table[i].number == msg_id)
    {
      return lte_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}

const char* lookup_pbm_message_name(int msg_id)
{
  int i;
  for (i = 0; i < PBM_MESSAGE_SIZE; ++i)
  {
    if (phonebook_manager_service_msg_string_table[i].number == msg_id)
    {
      return phonebook_manager_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}

const char* lookup_nas_message_name(int msg_id)
{
  int i;
  for (i = 0; i < NAS_MESSAGE_SIZE; ++i)
  {
    if (network_access_service_msg_string_table[i].number == msg_id)
    {
      return network_access_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}

const char* lookup_wms_message_name(int msg_id)
{
  int i;
  for (i = 0; i < WMS_MESSAGE_SIZE; ++i)
  {
    if (wireless_messaging_service_msg_string_table[i].number == msg_id)
    {
      return wireless_messaging_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}


const char* lookup_wds_message_name(int msg_id)
{
  int i;
  for (i = 0; i < WDS_MESSAGE_SIZE; ++i)
  {
    if (wireless_data_service_msg_string_table[i].number == msg_id)
    {
      return wireless_data_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}

const char* lookup_dsd_message_name(int msg_id)
{
  int i;
  for (i = 0; i < DSD_MESSAGE_SIZE; ++i)
  {
    if (data_system_determination_msg_string_table[i].number == msg_id)
    {
      return data_system_determination_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}

const char* lookup_auth_message_name(int msg_id)
{
  int i;
  for (i = 0; i < AUTH_MESSAGE_SIZE; ++i)
  {
    if (authentication_service_msg_string_table[i].number == msg_id)
    {
      return authentication_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}

const char* lookup_imsp_message_name(int msg_id)
{
  int i;
  for (i = 0; i < IMSP_MESSAGE_SIZE; ++i)
  {
    if (ip_multimedia_subsystem_presence_service_msg_string_table[i].number == msg_id)
    {
      return ip_multimedia_subsystem_presence_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}

const char* lookup_imss_message_name(int msg_id)
{
  int i;
  for (i = 0; i < IMSS_MESSAGE_SIZE; ++i)
  {
    if (ip_multimedia_subsystem_settings_service_msg_string_table[i].number == msg_id)
    {
      return ip_multimedia_subsystem_settings_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}

const char* lookup_imsa_message_name(int msg_id)
{
  int i;
  for (i = 0; i < IMSA_MESSAGE_SIZE; ++i)
  {
    if (ip_multimedia_subsystem_application_service_msg_string_table[i].number == msg_id)
    {
      return ip_multimedia_subsystem_application_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}
#ifdef RIL_UTF_IOE
const char* lookup_ioe_message_name(int msg_id)
{
  int i;
  for(i = 0; i < IOE_MESSAGE_SIZE; i++)
  {
      if(ioe_client_msg_table[i].number == msg_id)
      {
          return ioe_client_msg_table[i].name;
      }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}

#else

const char* lookup_cat_message_name(int msg_id)
{
  int i;
  for (i = 0; i < CAT_MESSAGE_SIZE; ++i)
  {
    if (card_application_toolkit_msg_string_table[i].number == msg_id)
    {
      return card_application_toolkit_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}
const char* lookup_uim_message_name(int msg_id)
{
  int i;
  for (i = 0; i < UIM_MESSAGE_SIZE; ++i)
  {
    if (user_identity_module_service_msg_string_table[i].number == msg_id)
    {
      return user_identity_module_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}
const char* lookup_uim_http_message_name(int msg_id)
{
  int i;
  for (i = 0; i < UIM_HTTP_MESSAGE_SIZE; ++i)
  {
    if (user_identity_module_http_service_msg_string_table[i].number == msg_id)
    {
      return user_identity_module_http_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}
const char* lookup_embms_message_name(int msg_id)
{
  int i;
  for (i = 0; i < EMBMS_MESSAGE_SIZE; ++i)
  {
    if (embms_service_msg_string_table[i].number == msg_id)
    {
      return embms_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}
const char* lookup_msim_message_name(int msg_id)
{
  int i;
  for (i = 0; i < MSIM_MESSAGE_NUM; ++i)
  {
    if (msim_service_msg_string_table[i].number == msg_id)
    {
      return msim_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}
const char* lookup_pdc_message_name(int msg_id)
{
  int i;
  for (i = 0; i < PDC_MESSAGE_SIZE; ++i)
  {
    if (pdc_service_msg_string_table[i].number == msg_id)
    {
      return pdc_service_msg_string_table[i].name;
    }
  }
  RIL_UTF_ESSENTIAL("\n@@@@@@     ERROR! NO MATCHING MESSAGE!!!\n");
  return NULL;
}
#endif

void print_ril_message_name(int msg_id)
{
#ifndef RIL_UTF_IOE
  RIL_UTF_ESSENTIAL("\n@@@@@@     Service Android Telephony");
  const char* str = lookup_ril_message_name(msg_id);
  RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s", str);
#endif
}
void print_ims_message_name(int msg_id)
{
  #ifndef RIL_UTF_IOE
  RIL_UTF_ESSENTIAL("\n@@@@@@     IMS Service interface");
  const char* str = lookup_ims_message_name(msg_id);
  RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s", str);
  #endif
}

void print_oem_message_name(int service_id, int msg_id, int oem_command_id, enum ril_utf_q_element_t type)
{
  const char* tmp;

#if 0
  if (oem_command_id == QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ ||
          oem_command_id == QCRIL_EVT_HOOK_UNSOL_LTE_DIRECT_DISC)
  {
    LteDirectDiscovery_MsgType msg_type;
    if (type == ril_utf_oem_request)
    {
      msg_type = LteDirectDiscovery_MsgType_REQUEST;
    }
    else if (type == ril_utf_oem_response)
    {
      msg_type = LteDirectDiscovery_MsgType_RESPONSE;
    }
    else
    {
      msg_type = LteDirectDiscovery_MsgType_UNSOL_RESPONSE;
    }
    //RIL_UTF_ESSENTIAL("\n@@@@@@     Event %s", (const char*)qcril_log_lookup_event_name((qcril_evt_e_type)oem_command_id));
    RIL_UTF_ESSENTIAL("\n@@@@@@     Event %s", "Invalid");
    RIL_UTF_ESSENTIAL("\n@@@@@@     msg_id %d, msg_type = %d\n", msg_id, msg_type);
    const char *msg = qcril_qmi_lte_direct_disc_get_msg_log_str(msg_id, msg_type);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s", msg);
  }
  else
#endif
  {
    switch (service_id)
    {
    #ifndef RIL_UTF_IOE
    case QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_NONE:
      RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_NONE");
      RIL_UTF_ESSENTIAL("\n@@@@@@     Event %s", lookup_oemhook_message_name(oem_command_id));
      break;

    case QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS:
      RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS");
      tmp = lookup_embms_message_name(msg_id);
      RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s", tmp);
    #endif
      break;

    default:
      RIL_UTF_ESSENTIAL("\n@@@@@@     Error unknown OEM Service Type %\n", service_id);
    }
  }
}

void print_msim_message_name(int msg_id)
{
  const char* tmp;
  #ifndef RIL_UTF_IOE
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service IPC RILD SOCKET");
    tmp = lookup_msim_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s", tmp);
  #endif

}

void print_qmi_message_name(int qmi_srv_id, int msg_id, enum ril_utf_q_element_t type)
{
  const char* tmp;
  char suffix[5];

  if ( type == ril_utf_qmi_request )
  {
    strlcpy(suffix, "REQ", 5);
  }
  else if (type == ril_utf_qmi_response)
  {
    strlcpy(suffix, "RESP", 5);
  }
  else
  {
   strlcpy(suffix, "IND", 5);
  }

  switch (qmi_srv_id)
  {
  case QMI_CTL:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_CTL_SERVICE");
    //TBD: update CTL in table
    //tmp = lookup_voice_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %d", msg_id);
    break;

  case QMI_VOICE:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_VOICE_SERVICE");
    tmp = lookup_voice_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s%s", tmp, suffix);
    break;

  case QMI_DMS:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_DMS_SERVICE");
    tmp = lookup_dms_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s%s", tmp, suffix);
    break;

  case QMI_PBM:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_PBM_SERVICE");
    tmp = lookup_pbm_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s%s", tmp, suffix);
    break;

  case QMI_NAS:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_NAS_SERVICE");
    tmp = lookup_nas_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s%s", tmp, suffix);
    break;

  case QMI_WMS:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_WMS_SERVICE");
    tmp = lookup_wms_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s%s", tmp, suffix);
    break;

  case QMI_IMSP:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_IMSP_SERVICE");
    tmp = lookup_imsp_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s%s", tmp, suffix);
    break;

  case QMI_IMSS:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_IMSS_SERVICE");
    tmp = lookup_imss_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s%s", tmp, suffix);
    break;

  case QMI_IMSA:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_IMSA_SERVICE");
    tmp = lookup_imsa_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s%s", tmp, suffix);
    break;

  case QMI_UIM:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_UIM_SERVICE");
    tmp = lookup_uim_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s%s", tmp, suffix);
    break;

  case QMI_CAT:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_CAT_SERVICE");
    tmp = lookup_cat_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s%s", tmp, suffix);
    break;

  case QMI_PDC:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_PDC_SERVICE");
    tmp = lookup_pdc_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s%s", tmp, suffix);
    break;

  case QMI_LTE:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_LTE_SERVICE");
    tmp = lookup_lte_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s%s", tmp, suffix);
    break;

  case QMI_UIM_HTTP:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_UIM_HTTP_SERVICE");
    tmp = lookup_uim_http_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s%s", tmp, suffix);
    break;

  case QMI_DSD:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_DSD_SERVICE");
    tmp = lookup_dsd_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s%s", tmp, suffix);
    break;

  case QMI_WDS:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_WDS_SERVICE");
    tmp = lookup_wds_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s%s", tmp, suffix);
    break;

  case QMI_AUTH:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Service QMI_AUTH_SERVICE");
    tmp = lookup_auth_message_name(msg_id);
    RIL_UTF_ESSENTIAL("\n@@@@@@     Message %s%s", tmp, suffix);
    break;

  default:
    RIL_UTF_ESSENTIAL("\n@@@@@@     Error unknown QMI Service Type");
    RIL_UTF_ESSENTIAL(" %d\n", qmi_srv_id);
  }
}

#ifdef RIL_UTF_IOE
char* get_ioe_message_name(int msg_id)
{
    char *str = lookup_ioe_message_name(msg_id);
    return str;
}
#else
const char* get_ril_message_name(int msg_id)
{
  const char* str = lookup_ril_message_name(msg_id);
  return str;
}

const char* get_ims_message_name(int msg_id)
{
  const char* str = lookup_ims_message_name(msg_id);
  return str;
}

const char* get_oem_message_name(int service_id, int msg_id)
{
  const char* tmp = NULL;
  switch (service_id)
  {
  case QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS:
    tmp = lookup_embms_message_name(msg_id);
    break;
  }
  return tmp;
}
#endif
const char* get_qmi_message_name(int qmi_srv_id, int msg_id)
{
  const char* tmp = NULL;
  switch (qmi_srv_id)
  {
  case QMI_VOICE:
    tmp = lookup_voice_message_name(msg_id);
    break;

  case QMI_DMS:
    tmp = lookup_dms_message_name(msg_id);
    break;

  case QMI_PBM:
    tmp = lookup_pbm_message_name(msg_id);
    break;

  case QMI_NAS:
    tmp = lookup_nas_message_name(msg_id);
    break;

  case QMI_WMS:
    tmp = lookup_wms_message_name(msg_id);
    break;

  case QMI_IMSP:
      #ifndef RIL_UTF_IOE
    tmp = lookup_imsp_message_name(msg_id);
    #endif
    break;

  case QMI_IMSS:
      #ifndef RIL_UTF_IOE
    tmp = lookup_imss_message_name(msg_id);
    #endif
    break;

  case QMI_IMSA:
      #ifndef RIL_UTF_IOE
    tmp = lookup_imsa_message_name(msg_id);
    #endif
    break;
  case QMI_PDC:
    tmp = lookup_pdc_message_name(msg_id);
    break;

  case QMI_LTE:
    tmp = lookup_lte_message_name(msg_id);
    break;
  }
  return tmp;
}

/*========================================================
  FUNCTION: ril_utf_print_hex
    - hex dump a buffer
  Critical section access:
    - none
========================================================*/
#define RIL_UTF_MAX_BUFFER_BYTES_PER_LINE                16
#define RIL_UTF_MAX_OUTPUT_BUF_SIZE                      ( ( RIL_UTF_MAX_BUFFER_BYTES_PER_LINE * 5 ) + 2 )
void ril_utf_print_hex
(
  unsigned char *msg,
  int            msg_len
)
{

  char output_buf[ RIL_UTF_MAX_OUTPUT_BUF_SIZE ];
  unsigned char val;
  char *p;
  int i;

  while ( msg_len > 0 )
  {
    p = output_buf;

    for ( i = 0; ( i < RIL_UTF_MAX_BUFFER_BYTES_PER_LINE ) && ( msg_len > 0 ); i++ )
    {
      /* First digit */
      val = ( *msg >> 4 ) & 0x0F;
      if ( val <= 9 )
      {
        *p++ = val + '0';
      }
      else
      {
        *p++ = ( val - 10 ) + 'A';
      }

      /* Second digit... ugly copied code */
      val = *msg & 0x0F;
      if ( val <= 9 )
      {
        *p++ = val + '0';
      }
      else
      {
        *p++ = ( val - 10 ) + 'A';
      }

      /* Add a space, and increment msg pointer */
      *p++ = ' ';
      msg++;
      msg_len--;
    }

    /* Add \n and NULL terminator and print out */
    *p++ = '\n';
    *p = '\0';

    RIL_UTF_ESSENTIAL("\nQMI_RIL_UTF: Hex dump: %s", output_buf );
  }
}
