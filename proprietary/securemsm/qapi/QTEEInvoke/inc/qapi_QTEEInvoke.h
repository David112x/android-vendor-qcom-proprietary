// ============================================================================
// Copyright (c) 2016-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// ============================================================================

/** @file qapi_QTEEInvoke.h
 *
 * @brief Client API used to get the opener object.
 */

#ifndef __QAPI_QTEE_INVOKE_H__
#define __QAPI_QTEE_INVOKE_H__

#include "object.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup qapi_qtee_invoke
@{ */

/**
 * Function used by clients to create an IOpener object. The call flow is as follows:
 *
 * @code{.c}
 * #include <object.h>
 * #include <stdint.h>
 *
 * Object openerObj = Object_NULL;
 * Object appLoaderAppObj   = Object_NULL;
 * Object appControllerObj  = Object_NULL;
 * Object myTAObj = Object_NULL;
 * size_t size              = 0;
 * uint8_t *buffer          = NULL;
 * int32_t err = Object_OK;
 * int32_t out = 0;
 *
 * // Get the opener object to open TA services with
 * err = qapi_QTEEInvoke_GetOpener(&openerObj);
 *
 * // Get AppLoader object to load the TA
 * err = qapi_IOpener_open(openerObj, CAppLoader_UID, &appLoaderAppObj);
 *
 * // Load the TA. "buffer" must contain the TA mbn bytes and "size" must be set
 * // to the number of bytes in "buffer".
 * err = IAppLoader_loadFromBuffer(appLoaderAppObj, buffer, size, appControllerObj);
 *
 * // Get the TA object instance for communicating with the TA
 * err = IAppController_getAppObject(appControllerObj, &myTAObj);
 *
 * // Call TA method
 * err = IMyTA_add(myTAObj, 5, 10, &out);
 *
 * // Unload TA
 * Object_ASSIGN_NULL(myTAObj);
 *
 * // Clean up
 * Object_ASSIGN_NULL(appControllerObj);
 * Object_ASSIGN_NULL(appLoaderAppObj);
 * Object_ASSIGN_NULL(openerObj);
 * @endcode
 *
 * @param[out] ClientOpenerObj  Object to be used with IOpener.
 *
 * @return 0 on success; -1 on failure.
 */

/**@file
 * @brief client get root object
 * This is used by client to create a root IOpener Obj when it starts up
 *
 * @ param[out] rootobj: root IClientEnv Obj
 * @return Object_OK: success, Object_ERROR: failure
 **/
int qapi_QTEEInvoke_GetOpener(Object *ClientOpenerObj);

/** @} */ /* end_addtogroup qapi_qtee_invoke */

#ifdef __cplusplus
}
#endif

#endif
