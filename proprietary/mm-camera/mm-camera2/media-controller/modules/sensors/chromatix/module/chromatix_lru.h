/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef __CHROMATIX_LRU_H__
#define __CHROMATIX_LRU_H__

#include "mtype.h"
#include "chromatix_list.h"

typedef struct {
  LIST_NODE *list_node;
  uint32_t   node_count;  /* max count of list item */

  list_type  list;
} chromatix_lru_type;

boolean lru_create(chromatix_lru_type *obj, uint32_t node_count);
void lru_destroy(chromatix_lru_type *obj);

boolean lru_add(chromatix_lru_type *obj, const char *key, uint32_t *cIndex);
boolean lru_update(chromatix_lru_type *obj, uint32_t cIndex);

boolean lru_getRecent(chromatix_lru_type* obj, char **key, uint32_t *cIndex);
boolean lru_getLeastRecent(chromatix_lru_type* obj, char **key, uint32_t *cIndex);

uint32_t lru_count(chromatix_lru_type* obj);

/* Print list items
 * debugging purpose */
void lru_traverse(chromatix_lru_type* obj);

#endif
