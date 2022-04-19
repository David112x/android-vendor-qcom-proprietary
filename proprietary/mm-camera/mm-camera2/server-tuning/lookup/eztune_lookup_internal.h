/***************************************************************************
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ***************************************************************************/
#ifndef __EZTUNE_LOOKUP_INTERNAL_H__
#define __EZTUNE_LOOKUP_INTERNAL_H__

#include "eztune.h"

#if defined(__cplusplus)
extern "C" {
#endif

eztune_item_t eztune_get_item(
  int i);

int32_t eztune_get_item_value(
  eztune_item_t *item,
  uint32_t table_index,
  char *output_buf);

void eztune_set_item_value(
    eztune_item_t *item,
    eztune_set_val_t *item_data);

#if defined(__cplusplus)
}
#endif

#endif  /* __EZTUNE_LOOKUP_INTERNAL_H__ */
