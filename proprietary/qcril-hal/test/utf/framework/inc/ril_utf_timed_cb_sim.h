/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_timed_cb_sim.h
  @brief   RIL UTF timed callback simulation

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UF_TIMED_CB_H
#define _RIL_UF_TIMED_CB_H
#include "ril_utf_common_includes.h"

struct timed_cb_t* create_timed_cb_list_node ( RIL_TimedCallback callback, void *param, struct timespec timeout);

void add_to_timed_cb_list( struct timed_cb_t *node );

void start_callback_timer(RIL_TimedCallback callback, void *param, const struct timeval *relativeTime) ;

void* timed_cb_thread(void *param);
void enqueue_timed_cb(struct timed_cb_t *node);
void empty_timed_cb_queue();
struct timed_cb_t* pop_top_node();
void init_timed_cb_emulation();
void ril_utf_timed_cb_thread_shutdown();

#endif //_RIL_UF_TIMED_CB_H
