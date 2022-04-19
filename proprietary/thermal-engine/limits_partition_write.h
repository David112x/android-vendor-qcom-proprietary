/*===========================================================================

  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef __LIMITS_PARTITION_WRITE_H__
#define __LIMITS_PARTITION_WRITE_H__

#ifdef ENABLE_LIMITS_PARTITION
void limits_partition_init(void);
#else
static inline void limits_partition_init(void){}
#endif /* ENABLE_PARTITION */

#endif /* __LIMITS_PARTITION_WRITE_H__ */
