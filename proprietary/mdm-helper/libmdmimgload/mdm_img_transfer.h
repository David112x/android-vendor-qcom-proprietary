/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 */

#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <mdm_detect.h>
#define MDM_IMAGE_PATH_PROP "persist.mdm_image_path"
#define MHI_PIPE_SBL_LOGGER "_pipe_24"
#define MHI_PIPE_EFS_SYNC "_pipe_10"
#define MHI_PIPE_SAHARA "_pipe_2"

/* mhi pipe for image transfer - for first 5 seconds poll
 * every 20 ms,later every 100 ms. Max timeout = 100 seconds */
#define INITIAL_DELAY_BETWEEN_RETRIES_MS 20
#define INITIAL_RETRIES_FOR_MHI_BUS_IMG 250
#define FINAL_DELAY_BETWEEN_RETRIES_MS 100
#define FINAL_RETRIES_FOR_MHI_BUS_IMG 950

/* mhi pipe for efs sync. Max timeout = 100 seconds */
#define MAX_RETRIES_FOR_MHI_BUS_EFS 200

typedef enum {
  MODE_DEFAULT,
  MODE_FORCE_RAM_DUMPS,
  MODE_IGNORE_RAM_DUMPS,
  MODE_NO_TIMEOUT
} img_transfer_mode;

int mdm_img_transfer(char* mdm_name,
                      char* mdm_link,
                      char* mdm_link_extra_data,
                      char* ram_dump_path,
                      char* saved_file_prefix,
                      img_transfer_mode mode);



