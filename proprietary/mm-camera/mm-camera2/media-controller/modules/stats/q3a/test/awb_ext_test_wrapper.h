/* awb_ext_test_wrapper.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AWB_EXT_TEST_WRAPPER_H__
#define __AWB_EXT_TEST_WRAPPER_H__

#include "awb.h"

void * awb_ext_test_wrapper_load_function(awb_ops_t *awb_ops);
void awb_ext_test_wrapper_unload_function(awb_ops_t *awb_ops, void *lib_handler);

#endif /*__AWB_EXT_TEST_WRAPPER_H__ */