#ifndef _MMDEFS_H
#define _MMDEFS_H

/*
 * This file contrains the functions for controlling the
 * internal circular buffer within the sound trigger HAL.
 * The purpose is to keep all the different read and write
 * commands from different threads synchronized.
 *
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
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
