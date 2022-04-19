/* awb_module.h
 *
 * Copyright (c) 2013, 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AWB_MODULE_H__
#define __AWB_MODULE_H__

#include "mct_list.h"
#include "mct_module.h"
#include "mct_port.h"
#include "stats_module.h"
#include "awb.h"

/**
 *
 **/
typedef struct _awb_object {
  pthread_mutex_t          obj_lock;
  void                     *awb;
  q3a_custom_data_t        awb_custom_param;
  awb_output_data_t        output;
  awb_ops_t                awb_ops;
} awb_object_t;

#define AWB_OBJECT_CAST(obj) ((awb_object_t *)(obj))

#define AWB_LOCK(obj) \
  pthread_mutex_lock(&(AWB_OBJECT_CAST(obj)->obj_lock))

#define AWB_UNLOCK(obj) \
  pthread_mutex_unlock(AWB_OBJECT_CAST(obj)->obj_lock)

#define AWB_INITIALIZE_LOCK(obj) \
  pthread_mutex_init(&(AWB_OBJECT_CAST(obj)->obj_lock), NULL)

#define AWB_DESTROY_LOCK(obj) \
  pthread_mutex_destroy(&(obj->obj_lock));


mct_module_t* awb_module_init(const char *name);
void awb_module_deinit(mct_module_t *mod);
mct_port_t *awb_module_get_port(mct_module_t *awb_module, unsigned int sessionid);

#endif /* __AWB_MODULE_H__ */
