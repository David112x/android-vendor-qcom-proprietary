/* af_ext_test_wrapper.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AF_EXT_TEST_WRAPPER_H__
#define __AF_EXT_TEST_WRAPPER_H__

#include "af.h"


void * af_ext_test_wrapper_load_function(af_ops_t *af_ops);
void af_ext_test_wrapper_unload_function(af_ops_t *af_ops, void *lib_handler);

#endif /*__AF_EXT_TEST_WRAPPER_H__*/
