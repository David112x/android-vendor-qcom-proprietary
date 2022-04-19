#ifndef _MMDEFS_H
#define _MMDEFS_H

/* ========================================================================*/
/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
/*===========================================================================*/
/*
 *                   S T A N D A R D    D E C L A R A T I O N S
 *
 *  Defines common types used within Multimedia Subsystem. Attempt is made to
 *  align to C99 standard and intention is to keep the list of commonly used
 *  types small.
*/
/*===========================================================================*/

#if /* Supported Compilers */ \
    defined(__ARMCC_VERSION) || \
    defined(__GNUC__)

/* If we're hosted, fall back to the system's stdint.h, which might have
 * additional definitions.
 */

#include "stdint.h"

#else /* Unsupported Compilers */

/* The following definitions are the same accross platforms.  This first
 * group are the sanctioned types.
 */

typedef unsigned long long uint64_t;  /**< Unsigned 64-bit integer type. */
typedef unsigned long int uint32_t;  /**< Unsigned 32-bit integer type. */
typedef unsigned short uint16_t;  /**< Unsigned 16-bit integer type. */
typedef unsigned char uint8_t;   /**< Unsigned  8-bit integer type. */

typedef signed long long int64_t;   /**< Signed 64-bit integer type. */
typedef signed long int int32_t;   /**< Signed 32-bit integer type. */
typedef signed short int16_t;   /**< Signed 16-bit integer type. */
typedef signed char int8_t;    /**< Signed  8-bit integer type. */

#endif /* Supported Compilers */

/** @} */ /* end_name Standard Integer Types */
/** @} */ /* end_addtogroup apr_core_data_types */

/** @addtogroup apr_core_constants_macros
@{ */
/* ------------------------------------------------------------------------
** Constants
** ------------------------------------------------------------------------ */
#undef TRUE
#undef FALSE

#define TRUE   (1)  /**< Boolean value for TRUE. */
#define FALSE  (0)  /**< Boolean value for FALSE. */

#ifndef NULL
#define NULL (0)  /**< NULL value. */
#endif

/** @} */ /* end_addtogroup apr_core_constants_macros */

/** @addtogroup apr_core_data_types
@{ */
/* ------------------------------------------------------------------------*/
/** @name Character and Boolean Types
@{ */
/* ------------------------------------------------------------------------ */
typedef char char_t;           /**< Eight-bit character type. */
typedef unsigned char bool_t;  /**< Eight-bit Boolean type. */

/** @} */ /* end_name Character and Boolean */
/** @} */ /* end_addtogroup apr_core_data_types */

#endif /* _MMDEFS_H */
