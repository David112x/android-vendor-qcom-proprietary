/**
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#ifndef __QCRIL_DATA_MUTEX_H
#define __QCRIL_DATA_MUTEX_H

#include "qcril_data_defs.h"
#include "qcril_data.h"

#define INFO_TBL_MUTEX    1
#define DSI_CB_TBL_MUTEX  2
#define LQE_CB_TBL_MUTEX  3
#define LQE_MUTEX         4
#define LQE_CLIENT_MUTEX  5

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void qcril_data_mutex_lock(unsigned int);
void qcril_data_mutex_unlock(unsigned int);
boolean qcril_data_mutex_trylock(unsigned int);

#define  QCRIL_DATA_MUTEX_LOCK( mutex_id ) \
do { \
   QCRIL_LOG_ADDITIONAL(">>>>>> LOCK QCRIL_DATA MUTEX %d", mutex_id); \
   qcril_data_mutex_lock( mutex_id ); \
  } while (0)

#define  QCRIL_DATA_MUTEX_UNLOCK( mutex_id ) \
do { \
   QCRIL_LOG_ADDITIONAL("<<<<<< UNLOCK QCRIL_DATA MUTEX %d", mutex_id); \
   qcril_data_mutex_unlock( mutex_id ); \
  } while (0)

#define QCRIL_DATA_MUTEX_TRYLOCK( mutex_id, result ) \
do { \
   QCRIL_LOG_ADDITIONAL(">>>>>> TRY LOCK QCRIL_DATA MUTEX %d", mutex_id); \
   result = qcril_data_mutex_trylock( mutex_id ); \
   QCRIL_LOG_ADDITIONAL("TRY LOCK QCRIL_DATA MUTEX %d result %d", \
       mutex_id, result); \
  } while (0)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __QCRIL_DATA_MUTEX_H */
