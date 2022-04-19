/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "ril_utf_qmi_msg_analyzer.h"
using namespace std;

/*========================================================
  FUNCTION: print_node
  print the node information stored in hex_node data structure
  including a string of the hex and a real void pointer
  for testing purpose only
========================================================*/
void print_node(hex_node n)
{
  cout << (int)n.if_type << " " << n.length_0  << " " << (int)n.ctl_0  << " " << \
    (int)n.qmux_type  << " " << (int)n.client_id << " " << (int)n.ctl_1<< " " << n.qmi_xx_type \
    << " " << n.length_1 << endl;
  if (n.value_str != "") {
    cout << "        Original Hex in str: " << n.value_str << endl;
  }
  if (n.value != NULL) {
    cout << "        New Hex in void ptr: ";
    uint8_t* p = (uint8_t*)n.value;
    for (int i = 0; i < n.length_1; ++i) {
      printf("%02X", p[i]);
    }
    cout << endl;
  }
  cout << endl;
}


/*========================================================
  FUNCTION: clear node
  free the memory used to store the void pointer
========================================================*/
void clear_node(hex_node n)
{
  free(n.value);
}

/*========================================================
  FUNCTION: hex2dec
  convert a QMI message hex string to decimal, and reverse the order
  ex. 0800 will be changed to 0008 first, then converted to 8 in decimal
========================================================*/
uint16_t hex2dec (string s) {
  string newstring = "";
  for (string::iterator it = s.end(); it != s.begin(); it-=2) {
    newstring.append(it-2, it);
  }
  uint ret;
  sscanf(newstring.c_str(), "%x", &ret);
  return (uint16_t)ret;
}


/*========================================================
  FUNCTION: get_p_service_type
  return the service type for different QMI services
========================================================*/
qmi_idl_service_object_type get_p_service_type (uint8_t service_type) {
  switch (service_type) {
  case QMI_VOICE_SERVICE:
    return voice_get_service_object_v02();
  case QMI_NAS_SERVICE:
    return nas_get_service_object_v01();
  case QMI_WMS_SERVICE:
    return wms_get_service_object_v01();
  case QMI_WDS_SERVICE:
    return wds_get_service_object_v01();
  case QMI_DMS_SERVICE:
    return dms_get_service_object_v01();
  case QMI_UIM_SERVICE:
    return uim_get_service_object_v01();
  case QMI_PBM_SERVICE:
    return pbm_get_service_object_v01();
  case QMI_RF_SAR_SERVICE:
    return svs_get_service_object_v01();
  case QMI_IMS_VT_SERVICE:
    return imsvt_get_service_object_v01();
  case QMI_IMS_PRESENCE_SERVICE:
    return imsp_get_service_object_v01();
  // case QMI IMSA:
  case 0x21:
    return imsa_get_service_object_v01();
  case QMI_RFPE_SERVICE:
    return rfrpe_get_service_object_v01();
  // case QMI IMSS:
  case 0x12:
    return imss_get_service_object_v01();
  case QMI_CSVT_SERVICE:
    return csvt_get_service_object_v01();
  default:
    cout << "\nError! Unknown Service Type " << (int)service_type << endl;
    return NULL;
  }
}

/*========================================================
  FUNCTION: get_message_type
  return the message type for different QMI messages
========================================================*/
qmi_idl_type_of_message_type get_message_type (uint8_t message_type) {
   switch (message_type) {
   case 0x00:
     return QMI_IDL_REQUEST;
   case 0x02:
     return QMI_IDL_RESPONSE;
   case 0x04:
     return QMI_IDL_INDICATION;
   default:
     cout << "Error! Unknown Message Type" << (int)message_type << endl;
     exit(0);
   }
}


/*========================================================
  FUNCTION: read_hex_msg
  read the hex message from a string, and store that in the hex_node struct
========================================================*/
hex_node read_hex_msg(string msg)
{
  hex_node n;
  if (msg.length() >= 26)
  {
    n.if_type = hex2dec(msg.substr(0, 2));
    n.length_0 = hex2dec(msg.substr(2, 4));
    n.ctl_0 = hex2dec(msg.substr(6, 2));
    n.qmux_type = hex2dec(msg.substr(8, 2));
    n.client_id = hex2dec(msg.substr(10, 2));
    n.ctl_1 = hex2dec(msg.substr(12, 2));
    n.tx_id = hex2dec(msg.substr(14, 4));
    n.qmi_xx_type = hex2dec(msg.substr(18, 4));
    n.length_1 = hex2dec(msg.substr(22, 4));
    if (msg.length() > 26)
      n.value_str = msg.substr(26).c_str();
    else
      n.value_str = "";
    n.value = NULL;
    if ((int) n.length_1 * 2 != (int) n.value_str.length())
        cout << "Error. Length doesn't match\n";
    else
    {
      char* buf = (char*) malloc(n.length_1 * sizeof(char));
      for (int i = 0; i < n.length_1; ++i)
      {
        buf[i] = (char) hex2dec(msg.substr(26 + 2 * i, 2));
        n.value = buf;
      }
    }
  }
  else
    cout << "Error. Length less than 13 bytes\n";
  return n;
}

/*========================================================
  FUNCTION: decode_hex_msg
  decode the hex message stored in hex_node, and store the decoded payload in the void* payload pointer
========================================================*/
void decode_hex_msg(hex_node n, void* payload, int payload_len)
{
  qmi_idl_service_object_type p_service = get_p_service_type (n.qmux_type);
  qmi_idl_type_of_message_type message_type = get_message_type (n.ctl_1);
  uint16_t message_id = n.qmi_xx_type;
  const void *p_src = n.value;
  uint32_t src_len = n.length_1;

  if (p_service == NULL)
  {
    cout << "NULL Service Type\n";
    return;
  }
  if (p_src == NULL)
  {
    cout << "NULL p_src, no need for decode\n";
    return;
  }

  int32_t ret = qmi_idl_message_decode(p_service, message_type, message_id, p_src, src_len, payload, payload_len);
  if (ret != QMI_NO_ERR)
  {
    cout << "Decode Error! " << (int)ret << endl;;
    cout << "Service ID: " << (int)n.qmux_type << "\nService Type: " << (int)n.ctl_1 << "\nMessage ID: " << (int)n.qmi_xx_type << "\nTLV Length: " << (int)n.length_1 << endl;
    print_node (n);
  }
  //else
  //  cout << "Decode Successful\n";
}

/*========================================================
  FUNCTION: decode_qmi_hex_msg
  this is the only function called by test case
  it will read the hex message from a string, decode that and store the decoded payload
  in the void* payload pointer
========================================================*/
void decode_qmi_hex_msg(string msg, void* payload, int payload_len)
{
  hex_node n = read_hex_msg(msg);
  //print_node(n);
  decode_hex_msg(n, payload, payload_len);
  clear_node(n);
}
