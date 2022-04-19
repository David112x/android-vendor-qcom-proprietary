#ifndef _TH_COMMON_H__
#define _TH_COMMON_H__

/*========================================================================

*//** @file common.h

FILE SERVICES:
     Common include headers for all internal library implementaions to use.

      Detailed description

Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                      Edit History

when       who    what, where, why
--------   ---    -------------------------------------------------------
11/7/14   rg     Initial Version.

========================================================================== */


/*========================================================================*/
/* INCLUDE FILES */
/*==  =====================================================================*/
/* std headers */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <utils/Log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>

/* local headers */
#include "thread_error.h"
#include "debuglog.h"
#include "types.h"

/*========================================================================*/
/* MACRO DEFINITIONS */
/*========================================================================*/
#define IS_NULL(ptr) (NULL == ptr)

#endif /* end of macro _TH_COMMON_H__ */
