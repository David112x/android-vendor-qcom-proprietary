/* af_module.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __AF_MODULE_H__
#define __AF_MODULE_H__

#include "q3a_module.h"
#include "af.h"

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
  void             *af;
  af_output_data_t output;
  af_ops_t         af_ops;
} af_object_t;

mct_module_t *af_module_init(const char *name);
void       af_module_deinit(mct_module_t *module);
mct_port_t *af_module_get_port(mct_module_t *af_module, unsigned int sessionid);

#endif /* __AF_MODULE_H__ */
