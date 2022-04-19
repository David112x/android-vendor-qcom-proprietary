/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef __CHROMATIX_LIST_H__
#define __CHROMATIX_LIST_H__

#include "mtype.h"

typedef struct list_node_tag {
  struct list_node_tag *prev;
  struct list_node_tag *next;

  uint32_t cIndex;

  char     key[256];
} list_node_type, *LIST_NODE;

typedef struct {
  uint32_t count;   /* number of nodes */

  LIST_NODE front;  /* the first item in list */
  LIST_NODE back;   /* the last item in list */
} list_type;


/* Constructor/Destructor */
boolean list_create(list_type *list);
void list_destroy(list_type *list);

uint32_t list_count(list_type *list);

/* Insert item to list */
LIST_NODE list_pushFront(list_type *list, uint32_t cIndex, const char *key);
LIST_NODE list_pushBack(list_type *list, uint32_t cIndex, const char *key);
/* delete item to list */
void list_popFront(list_type *list);
void list_popBack(list_type *list);

void list_erase(list_type *list, LIST_NODE node);

LIST_NODE list_front(list_type *list);
LIST_NODE list_back(list_type *list);

/* Print list items
 * debugging purpose */
void list_traverse(list_type *list);

#endif
