/* aec_ext_test_wrapper.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AEC_EXT_TEST_WRAPPER_H__
#define __AEC_EXT_TEST_WRAPPER_H__

#include "aec.h"

void * aec_ext_test_wrapper_load_function(aec_object_t *aec_object);
void aec_ext_test_wrapper_unload_function(aec_object_t *aec_object, void *lib_handler);

#endif /*__AEC_EXT_TEST_WRAPPER_H__ */