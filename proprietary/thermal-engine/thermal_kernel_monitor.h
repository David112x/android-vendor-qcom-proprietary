/*=============================================================/
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * =============================================================*/
#ifndef __THERMAL_KERNEL_MONITOR_H__
#define __THERMAL_KERNEL_MONITOR_H__

#include <stdio.h>
#include <stdlib.h>
#include "inc/thermal.h"
#include "inc/thermal_config.h"
#include "server/thermal_lib_common.h"

#ifdef USE_THERMAL_FRAMEWORK
void step_wise_init_data(struct thermal_setting_t *setting);
void step_wise_algo_init(struct thermal_setting_t *setting);
#else
static inline void step_wise_init_data(struct thermal_setting_t *setting) {};
static inline void step_wise_algo_init(struct thermal_setting_t *setting) {};
#endif  /* USE_THERMAL_FRAMEWORK */

#endif  /* __THERMAL_KERNEL_MONITOR_H__ */
