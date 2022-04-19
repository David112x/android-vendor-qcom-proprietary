#ifndef __TH_ERROR_H__
#define __TH_ERROR_H__

/*========================================================================

*//** @file thread_error.h

FILE SERVICES:
      Common error handling code to be used by Thread Services Library.

      Detailed description 1

Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                      Edit History

when       who    what, where, why
--------   ---    -------------------------------------------------------
11/10/14   rg     Initial Version.

========================================================================== */

/*========================================================================*/
/* INCLUDE FILES */
/*==  =====================================================================*/
/* std headers */

/* local headers */

/*========================================================================*/
/* DEFINITIONS AND DECLARATIONS */
/*========================================================================*/
#if defined( __cplusplus )
extern "C"
{
#endif /* end of macro __cplusplus */

typedef unsigned int th_err_t; /* 32 bit error code */

/*========================================================================*/
/* xxxx xxxx xxxx xxxx     xxxx xxxx      xxxx xxxx   */
/* -------------------     ---------     -----------  */
/*     (unused)            (layer)       (error code) */
/* if the modude return code is succes, the last 8 bits should be kept zero */
/*========================================================================*/

#define TH_COMMON_ERROR_BASE   0x100

/* global library success and failure codes */
#define TH_SUCCESS  0
#define TH_FAILURE -1

#define TH_ERROR_FATAL              TH_COMMON_ERROR_BASE + 1
#define TH_ERROR_NULLPTR            TH_COMMON_ERROR_BASE + 2
#define TH_ERROR_SESSION_EXISTS     TH_COMMON_ERROR_BASE + 3
#define TH_ERROR_SESSION_INVALID    TH_COMMON_ERROR_BASE + 4
#define TH_ERROR_JOBQ_FULL          TH_COMMON_ERROR_BASE + 5
#define TH_ERROR_TIMEOUT            TH_COMMON_ERROR_BASE + 6
#define TH_ERROR_STOPREQ            TH_COMMON_ERROR_BASE + 7
#define TH_ERROR_JOB_HOLD           TH_COMMON_ERROR_BASE + 8
#define TH_ERROR_JOB_STOPPED        TH_COMMON_ERROR_BASE + 9
#define TH_ERROR_JOBLIST_FULL       TH_COMMON_ERROR_BASE + 10
#define TH_ERROR_JOB_UNREGISTERED   TH_COMMON_ERROR_BASE + 11


#if defined( __cplusplus )
}
#endif /* end of macro __cplusplus */

#endif /* end of macro __TH_LOG_H__ */
