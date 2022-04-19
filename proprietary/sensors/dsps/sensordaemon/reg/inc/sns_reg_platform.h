#ifndef _SNS_REG_PLATFORM_H_
#define _SNS_REG_PLATFORM_H_
/*============================================================================
  @file sns_reg_platform.h

  @brief
  Header file for the Linux Android specific Sensors Registry definitions.

  <br><br>

  DEPENDENCIES: None.

  Copyright (c) 2013, 2017-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
  ============================================================================*/


#define SNS_REG_DATA_FILENAME "/mnt/vendor/persist/sensors/sns.reg"
#define SNS_REG_DATA_FILENAME_LEGACY "/persist/sensors/sns.reg"
#define SNS_REG_DEFAULT_CONF_PATH "/vendor/etc/sensors/"
#define SNS_REG_DEFAULT_CONF_PATH_LEGACY "/etc/sensors/"

#define SNS_REG_FILE_MASK S_IRWXU

#endif /* _SNS_REG_PLATFORM_H_ */
