/* q3a_thread_ext.h
 *
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __Q3A_THREAD_EXT_H__
#define __Q3A_THREAD_EXT_H__

#include "aec_port.h"
#include "af_port.h"

/* AECAWB  ext */
int q3a_thread_ext_aecawb_initialize_register();
boolean q3a_thread_ext_aecawb_get_thread_obj(mct_port_t *mct_port);
boolean q3a_thread_ext_aecawb_set_thread_obj(mct_port_t *mct_port);
void q3a_thread_ext_aecawb_release_aecawb_obj_ref(mct_port_t *mct_port);
boolean q3a_thread_ext_aecawb_create_single_thread(mct_port_t *mct_port);
void q3a_thread_ext_aecawb_destroy_single_thread();

/* AF ext */
int q3a_thread_ext_af_initialize_register();
boolean q3a_thread_ext_af_get_thread_obj(mct_port_t *mct_port);
boolean q3a_thread_ext_af_set_thread_obj(mct_port_t *mct_port);
void q3a_thread_ext_af_release_af_obj_ref(mct_port_t *mct_port);
boolean q3a_thread_ext_af_create_single_thread(mct_port_t *mct_port);
void q3a_thread_ext_af_destroy_single_thread();

#endif /* __Q3A_THREAD_EXT_H__ */
