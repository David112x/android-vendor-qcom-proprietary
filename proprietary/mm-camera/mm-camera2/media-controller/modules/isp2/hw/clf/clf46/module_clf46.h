/* module_clf46.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */


#ifndef __MODULE_CLF46_H__
#define __MODULE_CLF46_H__

/* std headers */
#include "pthread.h"

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_common.h"
#include "module_clf46.h"
#include "clf46_reg.h"
#include "chromatix.h"

#define MAP_NZ(value)  ((value == 0) ? 1 : ((value == 1) ? 0 : 2))
/** clf46_t:
 *
 *  @reg_cmd: reg cmd to configure clf hw module
 **/

typedef struct {
  ISP_CLF_CmdType reg_cmd;
} clf46_t;

#endif
