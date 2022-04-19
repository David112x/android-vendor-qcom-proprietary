/********************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
*********************************************************************/

#ifndef _TZCOM_H_
#define _TZCOM_H_

#include "object.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@brief get root object
* This is used to create a root IClientEnv Obj;
* it supports default 16 callback threads and 4K callback request buffer.
*
* @ param[out] rootobj: root IClientEnv Obj
*
* return value:  Object_OK - success; Object_ERROR - failure
*/
int TZCom_getRootEnvObject (Object *obj);

/**@brief get root object with configurable callback threads and/or callback buffer size
* This is used by client to create a root IClientEnv Obj when number of callback threads
* and/or callback buffer size are different from default values
*
* @ param[in] cbthread_cnt:    cb thread count
* @ param[in] cbbuf_len:       cb request buffer length
* @ param[out] rootobj:        root IClientEnv Obj
*
* return value:  Object_OK - success; Object_ERROR - failure
*/
int TZCom_getRootEnvObjectWithCB (size_t cbthread_cnt, size_t cbbuf_len, Object *obj);

/**@brief Client get a new IClientEnv obj
* This interface provides an IClientEnv obj that is registered with TZ with client's credentials
* @param[out] obj: client's IClientEnv object
*
* return value:  0- success; -1 - failure
*/
int TZCom_getClientEnvObject (Object *obj);

/**@brief Client get a fd obj
*
* @ param[in] fd: the fd to be wrapped into an obj.
* @ param[out] obj: fd object that takes ownership of fd
*                   i.e.release of obj would close fd.
*
* return value:  0 - success; -1 - failure
*/
int TZCom_getFdObject (int fd, Object *obj);

#ifdef __cplusplus
}
#endif

#endif

