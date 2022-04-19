/*============================================================================

  Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __PPROC_MODULE_H__
#define __PPROC_MODULE_H__

#include "mct_module.h"

/* macros for unpacking identity */
#define PPROC_GET_STREAM_ID(identity) ((identity) & 0xFFFF)
#define PPROC_GET_SESSION_ID(identity) (((identity) & 0xFFFF0000) >> 16)

extern volatile uint32_t gcam_pproc_loglevel;
mct_module_t* pproc_module_get_sub_mod(mct_module_t *module, const char *name);
#endif
