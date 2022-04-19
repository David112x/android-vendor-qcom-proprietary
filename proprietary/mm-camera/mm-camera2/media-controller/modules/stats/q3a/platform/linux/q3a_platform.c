/* q3a_platform.c
 *
 *Copyright (c) 2016 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "q3a_platform.h"
int q3a_prop_get(char* propName, char *defVal) {

#ifdef __ANDROID__
  char value[PROPERTY_VALUE_MAX]="0";
  property_get(propName, value, defVal);
  return atoi(value);
#else
  return atoi(defVal);
#endif
}
