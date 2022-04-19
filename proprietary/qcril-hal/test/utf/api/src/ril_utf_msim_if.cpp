/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_msim_if.cpp
  @brief   MSIM API's for test case dev

  DESCRIPTION

  API's for modifying RILD simulation for IPC communication
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_msim_if.h"
//---------------------------------------------------------------------------------------
//==================================================
// MSIM Interfaces
//==================================================

/*==================================================
  IPC_MESSAGE_RADIO_POWER (send)
===================================================*/
void ril_utf_msim_if::update_default_ipc_message_radio_power_send(int is_genuine_signal)
{
  payload = malloc(sizeof(int));
  payload_len = sizeof(int);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_msim_send_msg;
  msg_id = IPC_MESSAGE_RADIO_POWER;

  int *ptr = (int*) payload;
  *ptr = is_genuine_signal;
}
void ril_utf_msim_if::set_ipc_message_radio_power_send_rild_instance(int tmp_instance_id)
{
  rild_instance_id = tmp_instance_id;
}

/*==================================================
  IPC_MESSAGE_RADIO_POWER (recv)
===================================================*/
void ril_utf_msim_if::update_default_ipc_message_radio_power_recv(int is_genuine_signal)
{
  payload = malloc(sizeof(int));
  payload_len = sizeof(int);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_msim_recv_msg;
  msg_id = IPC_MESSAGE_RADIO_POWER;

  int *ptr = (int*) payload;
  *ptr = is_genuine_signal;
}
void ril_utf_msim_if::set_ipc_message_radio_power_recv_rild_instance(int tmp_instance_id)
{
  rild_instance_id = tmp_instance_id;
}

