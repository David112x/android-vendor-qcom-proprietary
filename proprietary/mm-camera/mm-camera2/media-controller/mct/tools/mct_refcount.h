/* Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef MCT_REFCOUNT_H_INCLUDED
#define MCT_REFCOUNT_H_INCLUDED

#define MCT_REFCOUNT_FATAL_ON_ERROR

/* Uncomment to enable debug logs */
//#define MCT_REFCOUNT_DEBUG

#define MCT_REFCOUNT_MAX 20

#include <stdatomic.h>

typedef struct
{
  atomic_int refc;
  void (*destroyer)(void *);
} mct_refcount_t;

#define MCT_REFCOUNT_INIT_VAL(var, type, val, fptr)    \
  atomic_init(&((var)->type.refc), val);  \
  (var)->type.destroyer = fptr;

#define MCT_REFCOUNT_PRINT(var, type)   \
  MCT_REFCOUNT_PRINT_STR(&((var)->type.refc), "")

#define MCT_REFCOUNT_PRINT_STR(refc, str)             \
  CLOGI(CAM_MCT_MODULE, "refc: " str " %p : %d\n", (refc), atomic_load(refc));

/* Increase refcount of object */
#define MCT_REFCOUNT_GET(var, type)                         \
  ({                                                        \
    int ret;                                                \
    ret = mct_refcount_get_object(&(var)->type);            \
    ret;                                                    \
  })

/* Decrease refcount. If refcount reaches to 0 then invoke
destroyer function */
#define MCT_REFCOUNT_PUT(var, type)                         \
  ({                                                        \
    int ret;                                                \
    ret = mct_refcount_put_object(&(var)->type);            \
    if (ret == 0) {                                         \
      mct_refcount_destroy(&(var)->type, (var));           \
    }                                                       \
    ret;                                                    \
  })

extern void mct_refcount_destroy(mct_refcount_t *refc, void *p);
/* Should be called only from MCT_REFCOUNT_GET*/
extern int mct_refcount_get_object(mct_refcount_t *refc);
/* Should be called only from MCT_REFCOUNT_PUT*/
extern int mct_refcount_put_object(mct_refcount_t *refc);

#endif // ATOMIC_OBJECT_H_INCLUDED
