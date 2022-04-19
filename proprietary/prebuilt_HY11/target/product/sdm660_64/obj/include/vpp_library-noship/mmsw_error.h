/*========================================================================*//**
@file 
    mmsw_error.h

@par
    Error codes for multi-media software architecture (MMSW)

@par
    Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*//*=========================================================================*/

#ifndef MMSW_ERROR_H
#define MMSW_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/*========================================================================
 Data Types
 ========================================================================*/ 

/** @enum mmsw_error_t
 *  @brief Error codes/statuses returned by functions.
 */
typedef enum _mmsw_error_t
{
    mmsw_error_success         = 0, /**< Error code is: success         */
    mmsw_error_fail            = 1, /**< Error code is: generic failure */
    mmsw_error_out_of_memory   = 2, /**< Error code is: out of memory   */
    mmsw_error_invalid_value   = 3, /**< Error code is: invalid value   */
    mmsw_error_invalid_context = 4, /**< Error code is: invalid context */
} mmsw_error_t;

#ifdef __cplusplus
}
#endif

#endif // MMSW_ERROR_H
