/* q3a_version.h
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __Q3A_VERSION_H__
#define __Q3A_VERSION_H__

#include "q3a_platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
  uint16 major_version;
  uint16 minor_version;
  uint16 patch_version;
  uint16 new_feature_des;
}Q3a_version_t;

/* Q3a version*/
void get_3A_version(void *q3a_version);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __Q3A_VERSION_H__ */
