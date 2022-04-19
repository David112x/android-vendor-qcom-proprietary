/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "chromatix_lru.h"
#include "sensor_common.h"

/** lru_create
 *    @obj: lru type
 *    @node_count: maximum number of that LRU can have
 *
 *  Create the list for LRU tracking.
 *  Initialize the lru type.
 *
 *  Return: TRUE/FALSE
 **/
boolean lru_create(chromatix_lru_type *obj, uint32_t node_count)
{
  RETURN_ON_NULL(obj);

  if (node_count == 0) {
    SERR("wrong node count");
    return FALSE;
  }

  obj->list_node = (LIST_NODE *)malloc(sizeof(LIST_NODE) * node_count);
  if (obj->list_node == NULL) {
    SERR("failed : memory allocation");
    return FALSE;
  }

  /* create list structure */
  list_create(&obj->list);

  obj->node_count = node_count;

  return TRUE;
}

/** lru_destroy
 *    @obj: lru type
 *
 *  Free the list.
 *
 *  Return: TRUE/FALSE
 **/
void lru_destroy(chromatix_lru_type *obj)
{
  if (!obj) {
    SERR("failed NULL pointer detected");
    return;
  }

  list_destroy(&obj->list);
}

/** lru_add
 *    @obj: hash type
 *    @key: hash key (chromatix file name)
 *    @cIndex: (call-by-value)LRU table index
 *
 *  Add the LRU node.
 *  If LRU is full, then delete the least used node and add.
 *
 *  Return: TRUE/FALSE
 **/
boolean lru_add(chromatix_lru_type *obj, const char *key, uint32_t *cIndex)
{
  LIST_NODE node;
  uint32_t  count;

  RETURN_ON_NULL(obj);
  RETURN_ON_NULL(cIndex);

  count = list_count(&obj->list);

  /* If cache has space, then use an empty slot.
     Otherwise, remove the least recent slot and return the slot */
  if (count < obj->node_count) {
    *cIndex = count;
    obj->list_node[*cIndex] = list_pushBack(&obj->list, *cIndex, key);
  } else {
    node = list_back(&obj->list);
    if (NULL == node)
    {
      return FALSE;
    }
    *cIndex = node->cIndex;

    list_popBack(&obj->list);
    obj->list_node[*cIndex] = list_pushFront(&obj->list, *cIndex, key);
  }

  return TRUE;
}

/** lru_update
 *    @obj: hash type
 *    @cIndex: LRU table index
 *
 *  Update the LRU node.
 *  (Move the node into the front of the list.)
 *
 *  Return: TRUE/FALSE
 **/
boolean lru_update(chromatix_lru_type *obj, uint32_t cIndex)
{
  LIST_NODE data;
  char      key[256];

  RETURN_ON_NULL(obj);

  data = obj->list_node[cIndex];
  RETURN_ON_NULL(data);

  snprintf(key, 256, "%s", data->key);
  SLOW("%s updated", data->key);

  list_erase(&obj->list, obj->list_node[cIndex]);
  obj->list_node[cIndex] = list_pushFront(&obj->list, cIndex, key);

  return TRUE;
}

/** lru_getRecent
 *    @obj: hash type
 *    @key: (call-by-value)hash key (chromatix file name)
 *    @cIndex: (call-by-value)LRU table index
 *
 *  Return key/cIndex of the node at the front of the list.
 *
 *  Return: TRUE/FALSE
 **/
boolean lru_getRecent(chromatix_lru_type* obj, char **key, uint32_t *cIndex)
{
  LIST_NODE data;

  RETURN_ON_NULL(obj);
  RETURN_ON_NULL(key);
  RETURN_ON_NULL(cIndex);

  data = list_front(&obj->list);
  RETURN_ON_NULL(data);

  *key = data->key;
  *cIndex = data->cIndex;
  SLOW("recently used : %s", data->key);

  return TRUE;
}

/** lru_getLeastRecent
 *    @obj: hash type
 *    @key: (call-by-value)hash key (chromatix file name)
 *    @cIndex: (call-by-value)LRU table index
 *
 *  Return key/cIndex of the node at the end of the list.
 *
 *  Return: TRUE/FALSE
 **/
boolean lru_getLeastRecent(chromatix_lru_type* obj, char **key, uint32_t *cIndex)
{
  LIST_NODE data;

  RETURN_ON_NULL(obj);
  RETURN_ON_NULL(key);
  RETURN_ON_NULL(cIndex);

  data = list_back(&obj->list);
  RETURN_ON_NULL(data);

  *key = data->key;
  *cIndex = data->cIndex;
  SLOW("least used : %s", data->key);

  return TRUE;
}

/** lru_count
 *    @obj: hash type
 *
 *  Return: the number of lru nodes
 **/
uint32_t lru_count(chromatix_lru_type* obj)
{
  if (!obj) {
    SERR("failed NULL pointer detected");
    return 0;
  }

  return list_count(&obj->list);
}

void lru_traverse(chromatix_lru_type* obj)
{
  list_traverse(&obj->list);
}
