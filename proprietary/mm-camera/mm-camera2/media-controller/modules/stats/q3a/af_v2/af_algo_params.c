/* af_algo_params.c
*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/


#include "stats_chromatix_wrapper.h"

static af_algo_params af_algo_param =
#include "af_algo_params.h"

void* af_get_algo_params(void)
{
  return &af_algo_param;
}
