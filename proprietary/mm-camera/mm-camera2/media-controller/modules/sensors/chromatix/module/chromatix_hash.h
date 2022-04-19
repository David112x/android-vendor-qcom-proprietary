/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef __CHROMATIX_HASH_H__
#define __CHROMATIX_HASH_H__

#include "mtype.h"
#include "chromatix_lru.h"

enum {
  DEFAULT_HASH_SIZE = 37,
};

typedef struct hash_node_tag {
  struct hash_node_tag* prev;
  struct hash_node_tag* next;

  uint32_t              cIndex;      /* cache index (lru) */

  char                  key[256];    /* debugging purpose */
  uint32_t              hash_key;    /* crc # from the key */
  void                 *data_hdl;
  void                 *data_sym;
} hash_node_type, *HASH_NODE;

typedef struct {
  uint32_t        count;       /* total data count */

  hash_node_type *table;       /* hash bucket */
  uint32_t        table_size;  /* hash table bucket count */

  /*
   * Double HASH - separate chaining
   *
   * keytype  : string
   * hashkey  : uint32
   * datatype : pointer
   * datasize : sizeof(void*)
  */
} hash_type;

typedef int32_t (*hash_userFunction)(void*, void*);

/* Constructor/Destructor
 *   hash_size : the size of hash table
 *   (default value : 0) */
boolean hash_create(hash_type* obj, uint32_t hashSize);
void hash_destroy(hash_type* obj);

/* Add (key:data) into the hash table */
boolean hash_add(hash_type* obj, const char *key,
  void *data_hdl, void *data_sym, uint32_t cIndex);
/* Delete (key:data) from the hash table,
 * return the data */
boolean hash_delete(hash_type* obj, const char *key, void **data_hdl,
  void **data_symbol);
/* Find (key:data) from the hash table with a given key,
 * return the data */
boolean hash_find(hash_type* obj, const char *key,
  void **data_sym, uint32_t *cIndex);

void hash_traverseFuction(hash_type* obj, hash_userFunction func);

/* Print hash items
 * debugging purpose */
void hash_traverse(hash_type* obj);

#endif
