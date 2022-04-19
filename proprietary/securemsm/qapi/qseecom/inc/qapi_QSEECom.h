/**
 * @file qapi_QSEECom.h
 *
 * @brief HLOS QSEEComAPI Library Functions
 *
 * Contains the library functions for accessing the QSEECom driver.
 */
/*
 * Copyright (c) 2010-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __QAPI_QSEECOM_H_
#define __QAPI_QSEECOM_H_

#include <stdint.h>
#include <stdbool.h>
#include "qapi_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup qapi_qsee
@{ */

/**
 * @brief Handle to the loaded trusted application (TA).
 *
 * This handle is returned by qapi_QSEECom_start_app().
 */
typedef struct qapi_QSEECom_handle {
    unsigned char *mem_sbuffer;     /**< Not used -- set to NULL. */
} qapi_QSEECom_handle_t;
  
/**
 * @brief Starts a trusted application.
 *
 * This API is deprecated. Please refer to APIs provided by "IAppLoader.h" and 
 * "IAppController.h" to load a TA. 
 * 
 * See qapi_QTEEInvoke_GetOpener() for TA loading and shutdown usage example.
 *
 * @return QAPI_ERR_API_DEPRACATED.
 */
static int qapi_QSEECom_start_app(qapi_QSEECom_handle_t **clnt_handle,
        const char *path, const char *fname, uint32_t sb_size)
{
    return QAPI_ERR_API_DEPRACATED;
}

/**
 * @brief Starts a trusted application from a HLOS provided buffer.
 * 
 * This API is deprecated. Please refer to APIs provided by "IAppLoader.h" and 
 * "IAppController.h" to load a TA. 
 * 
 * See qapi_QTEEInvoke_GetOpener() for TA loading and shutdown usage example.
 *
 * @return QAPI_ERR_API_DEPRACATED.
 */

static int qapi_QSEECom_start_app_from_buffer(
        qapi_QSEECom_handle_t **clnt_handle,
        const char *name,
        const void *appElf_ptr,
        size_t appElf_len)
{
    return QAPI_ERR_API_DEPRACATED;
}

/**
 * @brief Shuts down a started trusted application.
 *
 * This API is deprecated. See qapi_QTEEInvoke_GetOpener() for TA loading and
 * shutdown usage example.
 *
 * @return QAPI_ERR_API_DEPRACATED.
 */
static int qapi_QSEECom_shutdown_app(qapi_QSEECom_handle_t **handle)
{
    return QAPI_ERR_API_DEPRACATED;
}

/**
 * @brief Dumps QSEE logs to a file on target EFS.
 *
 * This API is deprecated. Please use QXDM for TA logs. TA logs will be streamed
 * to QXDM if logging is enabled.
 *
 * @return QAPI_ERR_API_DEPRACATED.
 */
static int qapi_get_qsee_log(void)
{
    return QAPI_ERR_API_DEPRACATED;
}

/** @} */ /* end_addtogroup qapi_qsee */

#ifdef __cplusplus
}
#endif

#endif
