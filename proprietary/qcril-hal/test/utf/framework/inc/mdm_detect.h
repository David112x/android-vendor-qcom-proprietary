/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#define ESOC_ROOT_DIR "/sys/bus/esoc/devices"
#define RET_SUCCESS 0
#define RET_FAILED 1
#define MAX_SUPPORTED_MDM 4
#define MAX_NAME_LEN 32
#define MAX_PATH_LEN 255
typedef enum MdmType {
        MDM_TYPE_EXTERNAL = 0,
        MDM_TYPE_INTERNAL,
} MdmType;
struct mdm_info {
        MdmType type;
        char mdm_name[MAX_NAME_LEN];
        char mdm_link[MAX_NAME_LEN];
        char powerup_node[MAX_PATH_LEN];
        char drv_port[MAX_PATH_LEN];
        char ram_dump_path[MAX_PATH_LEN];
        char esoc_node[MAX_NAME_LEN];
};
struct dev_info {
        int num_modems;
        struct mdm_info mdm_list[MAX_SUPPORTED_MDM];
};
char *get_soc_name(char *esoc_dev_node);
char *get_soc_link(char *esoc_dev_node);
char *get_soc_ramdump_path(char *esoc_dev_node);
char *get_soc_port(char *esoc_dev_node);
int esoc_supported(char *esoc_node);
int esoc_framework_supported();
struct dev_info dev = {1, {{MDM_TYPE_EXTERNAL, "mdm2", "pcie"}}};
int get_system_info(struct dev_info *dev)
{
    return -1;
}
