/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_msim_if.h
  @brief   MSIM API's for test case dev

  DESCRIPTION

  API's for modifying MSIM messages used in RIL UTF tests.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef RIL_UTF_MSIM_IF_H_
#define RIL_UTF_MSIM_IF_H_

#include "ril_utf_if.h"
#include "ril_utf_msim_rild.h"
#include "qcril_log.h"

//RIL interfaces extracted from ril_utf_if.h
typedef class ril_utf_msim_if : public ril_utf_node_base_t {
public:
//==================================================
// MSIM Interfaces
//==================================================

/*
  IPC_MESSAGE_RADIO_POWER (send)
*/
void update_default_ipc_message_radio_power_send(int is_genuine_signal);
void set_ipc_message_radio_power_send_rild_instance(int tmp_instance_id);
/*
  IPC_MESSAGE_RADIO_POWER (recv)
*/
void update_default_ipc_message_radio_power_recv(int is_genuine_signal);
void set_ipc_message_radio_power_recv_rild_instance(int tmp_instance_id);

} MSIMTestnode;

#endif /* RIL_UTF_MSIM_IF_H_*/
