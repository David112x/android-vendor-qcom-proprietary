/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "chromatix_list.h"
#include "sensor_common.h"

static LIST_NODE createNode(uint32_t cIndex, const char *key);

/** list_create
 *    @list: list type
 *
 *  Initialize the list type.
 *
 *  Return: TRUE/FALSE
 **/
boolean list_create(list_type *list)
{
  RETURN_ON_NULL(list);

  list->count = 0;
  list->front = NULL;
  list->back = NULL;

  return TRUE;
}

/** list_destroy
 *    @list: list type
 *
 *  Free all list nodes created.
 *
 *  Return: TRUE/FALSE
 **/
void list_destroy(list_type *list)
{
  LIST_NODE node, d_node;

  if (!list) {
    SERR("failed NULL pointer detected");
    return;
  }

  if (list->count == 0)
    return;

  node = list->front;
  while (node) {
    d_node = node;
    node = node->next;
    free(d_node);
  }
}

/** list_count
 *    @list: list type
 *
 *  Return: the number of list nodes
 **/
uint32_t list_count(list_type *list)
{
  if (!list) {
    SERR("failed NULL pointer detected");
    return 0;
  }

  return list->count;
}

/** list_pushFront
 *    @list: list type
 *    @cIndex: index of LRU
 *    @key: hash key (chromatix file name)
 *
 *  Insert the node to the front of the list.
 *
 *  Return: indersted node
 **/
LIST_NODE list_pushFront(list_type *list, uint32_t cIndex, const char *key)
{
  LIST_NODE node = NULL;

  if (!list || !key) {
    SERR("failed NULL pointer detected");
    return NULL;
  }

  node = createNode(cIndex, key);
  if (!node) {
    SERR("failed: createNode");
    return NULL;
  }

  node->prev = NULL;
  node->next = list->front;

  if (list->front)
    list->front->prev = node;

  list->front = node;
  if (!list->back)
    list->back = node;

  ++list->count;

  return node;
}

/** list_pushBack
 *    @list: list type
 *    @cIndex: index of LRU
 *    @key: hash key (chromatix file name)
 *
 *  Insert the node to the end of list.
 *
 *  Return: indersted node
 **/
LIST_NODE list_pushBack(list_type *list, uint32_t cIndex, const char *key)
{
  LIST_NODE node = NULL;

  if (!list || !key) {
    SERR("failed NULL pointer detected");
    return NULL;
  }

  node = createNode(cIndex, key);
  if (!node) {
    SERR("failed: createNode");
    return NULL;
  }

  node->next = NULL;
  node->prev = list->back;

  if (list->back)
    list->back->next = node;

  list->back = node;
  if (!list->front)
    list->front = node;

  ++list->count;

  return node;
}

/** list_popFront
 *    @list: list type
 *
 *  Delete the node of the front of list.
 **/
void list_popFront(list_type *list)
{
  LIST_NODE node;

  if (!list) {
    SERR("failed NULL pointer detected");
    return;
  }

  if (list->count == 0)
    return;

  node = list->front;
  if (list->front == list->back)
    list->front = list->back = NULL;
  else {
    list->front = list->front->next;
    list->front->prev = NULL;
  }

  --list->count;
  free(node);
}

/** list_popBack
 *    @list: list type
 *
 *  Delete the node of the end of list.
 **/
void list_popBack(list_type *list)
{
  LIST_NODE node;

  if (!list) {
    SERR("failed NULL pointer detected");
    return;
  }

  if (list->count == 0)
    return;

  node = list->back;
  if (list->front == list->back)
    list->front = list->back = NULL;
  else {
    list->back = list->back->prev;
    list->back->next = NULL;
  }

  --list->count;
  free(node);
}

/** list_erase
 *    @list: list type
 *    @node: list node pointer
 *
 *  Delete the node from the list.
 **/
void list_erase(list_type *list, LIST_NODE node)
{
  if (!list || !node) {
    SERR("failed NULL pointer detected");
    return;
  }

  if (list->count == 0)
    return;

  if (node->prev == NULL)
    list->front = node->next;
  else
    node->prev->next = node->next;

  if (node->next == NULL)
    list->back = node->prev;
  else
    node->next->prev = node->prev;

  --list->count;
  free(node);
}

/** list_front
 *    @list: list type
 *
 *  Return: the front node of the list
 **/
LIST_NODE list_front(list_type *list)
{
  if (!list) {
    SERR("failed NULL pointer detected");
    return NULL;
  }

  if (list->count == 0) {
    SERR("list is empty");
    return NULL;
  }

  return list->front;
}

/** list_back
 *    @list: list type
 *
 *  Return: the end node of the list
 **/
LIST_NODE list_back(list_type *list)
{
  if (!list) {
    SERR("failed NULL pointer detected");
    return NULL;
  }

  if (list->count == 0) {
    SERR("list is empty");
    return NULL;
  }

  return list->back;
}

/** list_traverse
 *    @list: list type
 *
 *  Print all nodes in the list.
 *  (for debugging)
 **/
void list_traverse(list_type *list)
{
  LIST_NODE node;

  node = list->front;
  SLOW("-----------------------------");
  while (node) {
    SLOW("(%d:%s)", node->cIndex, node->key);
    node = node->next;
  }
  SLOW("-----------------------------");
}

static LIST_NODE createNode(uint32_t cIndex, const char *key)
{
  LIST_NODE node;

  node = (list_node_type *)malloc(sizeof(list_node_type));
  if (!node) {
    SERR("failed to alloc list node");
    return NULL;
  }

  node->cIndex = cIndex;
  snprintf(node->key, 256, "%s", key);

  return node;
}
