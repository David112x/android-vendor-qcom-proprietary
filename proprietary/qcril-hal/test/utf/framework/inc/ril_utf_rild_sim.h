/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_rild_sim.h
  @brief   RIL UTF RIL daemon simulation

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_RILD_SIM_H
#define _RIL_UTF_RILD_SIM_H

#include "ril_utf_common_includes.h"
#include "ril_utf_core.h"
#include "ril_utf_ril_api.h"
#include "ril_utf_timed_cb_sim.h"
#include "ril_utf_androidTel_payload.h"


void *rild_start(void *param);

void start_rild_thread();

void RIL_register (const RIL_RadioFunctions *callbacks);

void RIL_onRequestComplete(RIL_Token t, RIL_Errno e, void *response, size_t responselen);

void RIL_onUnsolicitedResponse(int unsolResponse, const void *data, size_t datalen);

void RIL_requestTimedCallback (RIL_TimedCallback callback, void *param, const struct timeval *relativeTime);

void process_callbacks();

void* main_thread_ev_loop( void *param );

void start_main_thread();
void ril_utf_rild_thread_shutdown();
void ril_utf_main_thread_shutdown();

void ril_utf_set_primary_rild();
void ril_utf_set_secondary_rild();

#ifdef __cplusplus
extern "C" {
#endif

int acquire_expectation_table();
void release_expectation_table(int expect_slot);
void enqueue_ril_expect( void  *payload, int payload_len, unsigned long msg_id,
                     int token_id, RIL_Errno e, enum ril_utf_q_element_t type );

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif
