/* af_module.h
 *
 * Copyright (c) 2013, 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef __AF_MODULE_H__
#define __AF_MODULE_H__

#include "q3a_module.h"
#include "af.h"

#define AF_OBJECT_CAST(obj) ((af_object_t *)(obj))

#define AF_LOCK(obj) \
  pthread_mutex_lock(&(AF_OBJECT_CAST(obj)->obj_lock))

#define AF_UNLOCK(obj) \
  pthread_mutex_unlock(&(AF_OBJECT_CAST(obj)->obj_lock))

#define AF_INITIALIZE_LOCK(obj) \
  pthread_mutex_init(&(AF_OBJECT_CAST(obj)->obj_lock), NULL)

#define AF_DESTROY_LOCK(obj) \
  pthread_mutex_destroy(&(AF_OBJECT_CAST(obj)->obj_lock));

/** _af_object:
 *    @obj_lock:    synchronization mechanism to protect concurrent access
 *                  to the af object
 *    @af:          pointer to the af module
 *    @output:      the data to be sent upstream
 *    @af_ops:      structure with pointers to the api library functions
 *
 * This structure describes the AF object that will be used to handle the
 * AF operations
 **/
typedef struct _af_object {
  pthread_mutex_t  obj_lock;

  void             *af;
  af_output_data_t output;
  af_ops_t         af_ops;
} af_object_t;

mct_module_t *af_module_init(const char *name);
void       af_module_deinit(mct_module_t *module);
mct_port_t *af_module_get_port(mct_module_t *af_module, unsigned int sessionid);

#endif /* __AF_MODULE_H__ */
