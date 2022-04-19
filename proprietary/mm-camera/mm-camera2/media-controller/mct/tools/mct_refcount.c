/* Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <signal.h>

#include "camera_dbg.h"
#include "mct_refcount.h"

int mct_refcount_get_object(mct_refcount_t *refc)
{
  int old_val, new_val;
  atomic_int *var = &refc->refc;

  #ifdef MCT_REFCOUNT_DEBUG
  MCT_REFCOUNT_PRINT_STR(&refc->refc, "get");
  #endif

  do
  {
    old_val = atomic_load(var);
    if (old_val <= 0)
    {
      CLOGI(CAM_MCT_MODULE, "ref count is already 0. Object is freed.\n");
      return 0;
    }

    #ifdef MCT_REFCOUNT_FATAL_ON_ERROR
    LOG_ALWAYS_FATAL_IF(old_val >= MCT_REFCOUNT_MAX, "refc is MCT_REFCOUNT_MAX. Did you forget to put?");
    #else
    if (old_val >= MCT_REFCOUNT_MAX)
    {
      CLOGI(CAM_MCT_MODULE, "refc is MCT_REFCOUNT_MAX. Did you forget to put?");
    }
    #endif

    new_val = old_val + 1;
  }
  while(!atomic_compare_exchange_weak(var, &old_val, new_val));

  return new_val;
}

int mct_refcount_put_object(mct_refcount_t *refc)
{
  int old_val, new_val;
  atomic_int *var = &refc->refc;

  #ifdef MCT_REFCOUNT_DEBUG
  MCT_REFCOUNT_PRINT_STR(&refc->refc, "put");
  #endif

  do
  {
    old_val = atomic_load(var);

    #ifdef MCT_REFCOUNT_FATAL_ON_ERROR
    LOG_ALWAYS_FATAL_IF(old_val <= 0, "refc is 0. Do you forget to get?");
    #else
    if (old_val <= 0)
    {
      CLOGI(CAM_MCT_MODULE, "refc is 0. Do you forget to get?");
      return -1;
    }
    #endif

    new_val = old_val - 1;
  }
  while(!atomic_compare_exchange_weak(var, &old_val, new_val));

  return new_val;
}

void mct_refcount_destroy(mct_refcount_t *refc, void *p)
{
  if (refc->destroyer) {
    #ifdef MCT_REFCOUNT_DEBUG
    CLOGI(CAM_MCT_MODULE, "refc: Destroying %p", refc);
    #endif
    refc->destroyer(p);
  }
}

