/*===========================================================================
  Copyright (c) 2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/


#ifndef __COOLING_DEVICES_H__
#define __COOLING_DEVICES_H__


#ifdef USE_THERMAL_FRAMEWORK
int cooling_devices_init(void);
void cooling_devices_release(void);
#else
static inline int cooling_devices_init(void) {return 0;}
static inline void cooling_devices_release(void) {}
#endif /* USE_THERMAL_FRAMEWORK */

#endif  /* __COOLING_DEVICES_H__ */
