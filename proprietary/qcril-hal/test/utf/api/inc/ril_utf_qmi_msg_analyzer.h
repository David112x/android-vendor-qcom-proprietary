/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef RIL_UTF_MSG_ANALYZER_H_
#define RIL_UTF_MSG_ANALYZER_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <algorithm>
#include "qmi_idl_lib.h"
#include "qmi.h"
#include "voice_service_v02.h"
#include "network_access_service_v01.h"
#include "wireless_messaging_service_v01.h"
#include "wireless_data_service_v01.h"
#include "device_management_service_v01.h"
#include "user_identity_module_v01.h"
#include "phonebook_manager_service_v01.h"
#include "sar_vs_service_v01.h"
#include "ip_multimedia_subsystem_video_telephony_v01.h"
#include "ip_multimedia_subsystem_presence_v01.h"
#include "ip_multimedia_subsystem_application_v01.h"
#include "ip_multimedia_subsystem_settings_v01.h"
#include "radio_frequency_radiated_performance_enhancement_v01.h"
#include "circuit_switched_video_telephony_v01.h"


// Structure to store the hex_message
struct hex_node {
  /* QMI Message */
  uint8_t if_type; // byte 1: Identifies the type of interface

  /* QMUX PDU */
  uint16_t length_0; // byte 2, 3: Total length of QMUX PDU (including length itself)
  uint8_t ctl_0; // byte 4: Flags specify sender type
  uint8_t qmux_type; //byte 5: Identifies the QMI service to which this message belongs
  uint8_t client_id; // byte 6: Identifies the ctl_1 point to which the message pertains

  /* QMUX SDU */
  uint8_t ctl_1; // byte 7: Control flags for SDU
  uint16_t tx_id; //byte 8, 9: Identifier to correlate a response with a command
  uint16_t qmi_xx_type; // byte 10, 11: Identifies message type - unique with service type
  uint16_t length_1; // byte 12, 13: Length of TLV
  void* value; // void pointer for byte 14-last, all TLV
  std::string value_str; // hex string for byte 14-last, all TLV
};

void decode_qmi_hex_msg(std::string msg, void* payload, int payload_len);

void print_node (hex_node n);

#endif /* RIL_UTF_MSG_ANALYZER_H_ */
