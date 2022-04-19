/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
//#include <linux/esoc_ctrl.h>
//#include <linux/ioctl.h>
#include <cutils/properties.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
//#include <mdm_helper.h>
#include "mdm_img_transfer.h"
#define LOG_TAG "libmdmimgload"
#include <log/log.h>
#define LOG_NIDEBUG 0

#define MAX_TEMP_BUFFER_LENGTH 256
#define MAX_EXEC_ARGS 64
#define EXEC_STRING_LENGTH 1024
#define LINK_TYPE_HSIC "HSIC"
#define LINK_TYPE_PCIE "PCIe"

#define KS_BINARY_PATH "/vendor/bin/ks"
#define RAM_DUMP_IMAGE_SBL "21"
#define RAM_DUMP_IMAGE_NONE "-1"
#define HSIC_PORT_NAME "/dev/ks_hsic_bridge"
#define PCIE_PORT_NAME "/dev/mhi_pipe_2"
#define SBL_IMAGE_NAME "sbl1.mbn"
#define MAX_IMAGES 20
#define BLK_DEV_PATH "/dev/block/bootdevice/by-name/"
#define MDMDDR  "9:mdmddr"

struct sahara_map {
	char mdm_name[MAX_NAME_LEN];
	char mdm_image_root_dir[PATH_MAX];
	//MAX_IMAGES * 2 since there are two entries in the arr
	//for each image
	char *mappings[MAX_IMAGES * 2];
};

struct sahara_map sahara_map_arr[] = {
	{
		.mdm_name = "MDM9x55",
		.mdm_image_root_dir = "/firmware/image/mdm9x55",
		.mappings = {
			"-s", "21:/firmware/image/mdm9x55/sbl1.mbn",
			"-s", "25:/firmware/image/mdm9x55/tz.mbn",
			"-s", "23:/firmware/image/mdm9x55/rpm.mbn",
			"-s", "30:/firmware/image/mdm9x55/sdi.mbn",
			"-s", "29:/firmware/image/mdm9x55/acdb.mbn",
			"-s", "31:/firmware/image/mdm9x55/mba.mbn",
			"-s", "8:/firmware/image/mdm9x55/qdsp6sw.mbn",
			"-s", "28:/firmware/image/mdm9x55/dsp2.mbn",
			"-s", "6:/firmware/image/mdm9x55/apps.mbn",
			"-s", "16:/dev/block/bootdevice/by-name/mdm1m9kefs1",
			"-s", "17:/dev/block/bootdevice/by-name/mdm1m9kefs2",
			"-s", "20:/dev/block/bootdevice/by-name/mdm1m9kefs3",
			"-s", "34:/dev/block/bootdevice/by-name/mdmddr",
			NULL,
		},
	},
	{
		.mdm_name = "SDX50M",
		.mdm_image_root_dir = "/vendor/firmware_mnt/image/sdx50m",
		.mappings = {
			"-s", "21:/vendor/firmware_mnt/image/sdx50m/sbl1.mbn",
			"-s", "25:/vendor/firmware_mnt/image/sdx50m/tz.mbn",
			"-s", "23:/vendor/firmware_mnt/image/sdx50m/rpm.mbn",
			"-s", "30:/vendor/firmware_mnt/image/sdx50m/sdi.mbn",
			"-s", "29:/vendor/firmware_mnt/image/sdx50m/acdb.mbn",
			"-s", "31:/vendor/firmware_mnt/image/sdx50m/mba.mbn",
			"-s", "8:/vendor/firmware_mnt/image/sdx50m/qdsp6sw.mbn",
			"-s", "28:/vendor/firmware_mnt/image/sdx50m/dsp2.mbn",
			"-s", "6:/vendor/firmware_mnt/image/sdx50m/apps.mbn",
			"-s", "16:/dev/block/bootdevice/by-name/mdm1m9kefs1",
			"-s", "17:/dev/block/bootdevice/by-name/mdm1m9kefs2",
			"-s", "20:/dev/block/bootdevice/by-name/mdm1m9kefs3",
			"-s", "34:/dev/block/bootdevice/by-name/mdmddr",
			NULL,
		},
	},
	{
		.mdm_name = "SDX55M",
		.mdm_image_root_dir = "/vendor/firmware_mnt/image/sdx55m",
		.mappings = {
			"-s", "21:/vendor/firmware_mnt/image/sdx55m/sbl1.mbn",
			"-s", "37:/vendor/firmware_mnt/image/sdx55m/multi_image.mbn",
			"-s", "38:/vendor/firmware_mnt/image/sdx55m/xbl_config.elf",
			"-s", "36:/vendor/firmware_mnt/image/sdx55m/multi_image_qti.mbn",
			"-s", "41:/vendor/firmware_mnt/image/sdx55m/devcfg.mbn",
			"-s", "25:/vendor/firmware_mnt/image/sdx55m/tz.mbn",
			"-s", "23:/vendor/firmware_mnt/image/sdx55m/aop.mbn",
			"-s", "8:/vendor/firmware_mnt/image/sdx55m/qdsp6sw.mbn",
			"-s", "6:/vendor/firmware_mnt/image/sdx55m/apps.mbn",
			"-s", "16:/dev/block/bootdevice/by-name/mdm1m9kefs1",
			"-s", "17:/dev/block/bootdevice/by-name/mdm1m9kefs2",
			"-s", "20:/dev/block/bootdevice/by-name/mdm1m9kefs3",
			"-s", "29:/vendor/firmware_mnt/image/sdx55m/acdb.mbn",
			"-s", "34:/dev/block/bootdevice/by-name/mdmddr",
			"-s", "40:/vendor/firmware_mnt/image/sdx55m/apdp.mbn",
			"-s", "33:/vendor/firmware_mnt/image/sdx55m/hyp.mbn",
			"-s", "42:/vendor/firmware_mnt/image/sdx55m/sec.elf",
			NULL,
		},
	},
	{
		.mdm_name = "SDXPRAIRIE",
		.mdm_image_root_dir = "/vendor/firmware_mnt/image/sdx55m",
		.mappings = {
			"-s", "21:/vendor/firmware_mnt/image/sdx55m/sbl1.mbn",
			"-s", "37:/vendor/firmware_mnt/image/sdx55m/multi_image.mbn",
			"-s", "38:/vendor/firmware_mnt/image/sdx55m/xbl_config.elf",
			"-s", "36:/vendor/firmware_mnt/image/sdx55m/multi_image_qti.mbn",
			"-s", "41:/vendor/firmware_mnt/image/sdx55m/devcfg.mbn",
			"-s", "25:/vendor/firmware_mnt/image/sdx55m/tz.mbn",
			"-s", "23:/vendor/firmware_mnt/image/sdx55m/aop.mbn",
			"-s", "8:/vendor/firmware_mnt/image/sdx55m/qdsp6sw.mbn",
			"-s", "6:/vendor/firmware_mnt/image/sdx55m/apps.mbn",
			"-s", "16:/dev/block/bootdevice/by-name/mdm1m9kefs1",
			"-s", "17:/dev/block/bootdevice/by-name/mdm1m9kefs2",
			"-s", "20:/dev/block/bootdevice/by-name/mdm1m9kefs3",
			"-s", "29:/vendor/firmware_mnt/image/sdx55m/acdb.mbn",
			"-s", "34:/dev/block/bootdevice/by-name/mdmddr",
			"-s", "40:/vendor/firmware_mnt/image/sdx55m/apdp.mbn",
			"-s", "33:/vendor/firmware_mnt/image/sdx55m/hyp.mbn",
			"-s", "42:/vendor/firmware_mnt/image/sdx55m/sec.elf",
			NULL,
		},
	},
	{
		.mdm_name = "SDXMARMOT",
		.mdm_image_root_dir = "/vendor/firmware_mnt/image/sdx60m",
		.mappings = {
			"-s", "21:/vendor/firmware_mnt/image/sdx60m/sbl1.mbn",
			"-s", "37:/vendor/firmware_mnt/image/sdx60m/multi_image.mbn",
			"-s", "38:/vendor/firmware_mnt/image/sdx60m/xbl_cfg.elf",
			"-s", "36:/vendor/firmware_mnt/image/sdx60m/multi_image_qti.mbn",
			"-s", "41:/vendor/firmware_mnt/image/sdx60m/devcfg.mbn",
			"-s", "25:/vendor/firmware_mnt/image/sdx60m/tz.mbn",
			"-s", "23:/vendor/firmware_mnt/image/sdx60m/aop.mbn",
			"-s", "8:/vendor/firmware_mnt/image/sdx60m/qdsp6sw.mbn",
			"-s", "6:/vendor/firmware_mnt/image/sdx60m/apps.mbn",
			"-s", "16:/dev/block/bootdevice/by-name/mdm1m9kefs1",
			"-s", "17:/dev/block/bootdevice/by-name/mdm1m9kefs2",
			"-s", "20:/dev/block/bootdevice/by-name/mdm1m9kefs3",
			"-s", "29:/vendor/firmware_mnt/image/sdx60m/acdb.mbn",
			"-s", "34:/dev/block/bootdevice/by-name/mdmddr",
			"-s", "40:/vendor/firmware_mnt/image/sdx60m/apdp.mbn",
			"-s", "33:/vendor/firmware_mnt/image/sdx60m/hyp.mbn",
			"-s", "42:/vendor/firmware_mnt/image/sdx60m/sec.elf",
			NULL,
		},
	},
	//Default should always be the last entry in the array.
	//Any new modem structs should be added on top of this one
	{
		.mdm_name = "DEFAULT",
		.mdm_image_root_dir = "/firmware/image",
		.mappings = {
			"-s", "21:/firmware/image/sbl1.mbn",
			"-s", "25:/firmware/image/tz.mbn",
			"-s", "23:/firmware/image/rpm.mbn",
			"-s", "30:/firmware/image/sdi.mbn",
			"-s", "29:/firmware/image/acdb.mbn",
			"-s", "31:/firmware/image/mba.mbn",
			"-s", "8:/firmware/image/qdsp6sw.mbn",
			"-s", "28:/firmware/image/dsp2.mbn",
			"-s", "6:/firmware/image/apps.mbn",
			"-s", "16:/dev/block/bootdevice/by-name/mdm1m9kefs1",
			"-s", "17:/dev/block/bootdevice/by-name/mdm1m9kefs2",
			"-s", "20:/dev/block/bootdevice/by-name/mdm1m9kefs3",
			"-s", "34:/dev/block/bootdevice/by-name/mdmddr",
			NULL,
		},
	},
};

bool wait_for_com_port(char *filename)
{
  struct stat status_buf;
  size_t i;

  ALOGI("Testing if file(port) \"%s\" exists", filename);
  for (i = 0; i < INITIAL_RETRIES_FOR_MHI_BUS_IMG && stat(filename, &status_buf) < 0; i++) {
      usleep( INITIAL_DELAY_BETWEEN_RETRIES_MS * 1000);
  }

  if (i == INITIAL_RETRIES_FOR_MHI_BUS_IMG) {
      ALOGE("'%s' was not found in first 5 seconds. Retrying..", filename);

      /* Sleep for 100 ms between retries */
      for (i = 0; i < FINAL_RETRIES_FOR_MHI_BUS_IMG && stat(filename, &status_buf) < 0; i++) {
          usleep( FINAL_DELAY_BETWEEN_RETRIES_MS * 1000);
      }

      if (i == FINAL_RETRIES_FOR_MHI_BUS_IMG){
          ALOGE("'%s' was not found", filename);
          return false;
      }
  }
  ALOGI("File(port) :%s: located", filename);
  return true;
}

static int load_sahara(char* port_name,
                        char* ram_dump_path,
                        char* saved_file_prefix,
                        img_transfer_mode mode,
                        char* ram_dump_image,
                        char* sahara_mappings[])
{
  char* exec_args[MAX_EXEC_ARGS] = {NULL};
  char exec_string_log[EXEC_STRING_LENGTH] = {0};
  size_t exec_args_index = 0;
  size_t i = 0, written = 0;
  pid_t process_pid;
  int process_retval = 0;

  if (false == wait_for_com_port(port_name))
    return RET_FAILED;

  do {
    exec_args[exec_args_index++] = KS_BINARY_PATH;
    if (exec_args_index >= MAX_EXEC_ARGS) break;

    exec_args[exec_args_index++] = "-o";
    if (exec_args_index >= MAX_EXEC_ARGS) break;
    exec_args[exec_args_index++] = "-p";
    if (exec_args_index >= MAX_EXEC_ARGS) break;
    exec_args[exec_args_index++] = port_name;
    if (exec_args_index >= MAX_EXEC_ARGS) break;

    exec_args[exec_args_index++] = "-w";
    if (exec_args_index >= MAX_EXEC_ARGS) break;
    exec_args[exec_args_index++] = ram_dump_path;
    if (exec_args_index >= MAX_EXEC_ARGS) break;

    if (NULL != saved_file_prefix && '\0' != saved_file_prefix) {
      exec_args[exec_args_index++] = "-g";
      if (exec_args_index >= MAX_EXEC_ARGS) break;
      exec_args[exec_args_index++] = saved_file_prefix;
      if (exec_args_index >= MAX_EXEC_ARGS) break;
    }

    if (MODE_FORCE_RAM_DUMPS == mode) {
      exec_args[exec_args_index++] = "-m";
      if (exec_args_index >= MAX_EXEC_ARGS) break;
    }
    else if (MODE_IGNORE_RAM_DUMPS == mode) {
      exec_args[exec_args_index++] = "-i";
      if (exec_args_index >= MAX_EXEC_ARGS) break;
    }
    else if (MODE_NO_TIMEOUT == mode){
      exec_args[exec_args_index++] = "-t";
      if(exec_args_index >= MAX_EXEC_ARGS) break;
        exec_args[exec_args_index++] = "0";
      if(exec_args_index >= MAX_EXEC_ARGS) break;
    }

    if (NULL != ram_dump_image) {
      exec_args[exec_args_index++] = "-r";
      if (exec_args_index >= MAX_EXEC_ARGS) break;
      exec_args[exec_args_index++] = ram_dump_image;
      if (exec_args_index >= MAX_EXEC_ARGS) break;
    }
    exec_args[exec_args_index++] = "--partition_path";
    if (exec_args_index >= MAX_EXEC_ARGS) break;
    exec_args[exec_args_index++] = BLK_DEV_PATH;
    size_t sahara_mapping_index = 0;
    while (NULL != sahara_mappings[sahara_mapping_index] &&
           exec_args_index < MAX_EXEC_ARGS) {
      exec_args[exec_args_index++] = sahara_mappings[sahara_mapping_index++];
    }
    if (exec_args_index + 1 >= MAX_EXEC_ARGS) break;
    exec_args[exec_args_index++] = "-a";
    exec_args[exec_args_index++] = MDMDDR;

  } while(0);

  if (exec_args_index >= MAX_EXEC_ARGS - 1) {
    ALOGE("No more room for execve arguments");
    return false;
  }

  exec_args[exec_args_index] = NULL;

  for (i = 0; i < exec_args_index && written < sizeof(exec_string_log); ++i)
    written += snprintf(exec_string_log + written,
                        sizeof(exec_string_log) - written,
                        "%s ",
                        exec_args[i]);

  ALOGE("Running '%s'", exec_string_log);

  process_pid = fork();
  process_retval = 0;

  if (process_pid < 0) {
    ALOGE("Forking new process failed");
    return RET_FAILED;
  }
  else if (process_pid == 0) {
    if (execvp(exec_args[0], exec_args) == -1) {
      ALOGE("Spawning process using execvp failed");
      _exit(127);
    }
  }
  if (process_pid > 0) {
    waitpid(process_pid,&process_retval,0);
    ALOGE("Process return value %d", process_retval);
    process_pid = 0;
    if (WEXITSTATUS(process_retval) == 0) {
        ALOGE("Sahara transfer completed successfully");
        return RET_SUCCESS;
    }
    // If EAGAIN is returned from kickstart, we need to retry
    // Propagate the return back to IMG LOAD
    else if(WEXITSTATUS(process_retval) == EAGAIN){
        return RET_RETRY;
    }
    else {
      return RET_FAILED;
    }
  }
  return RET_FAILED;
}

static char** get_sahara_mappings(char *mdm_name)
{
	struct stat st;
	int i;
	for (i = 0; strncmp(sahara_map_arr[i].mdm_name, "DEFAULT", 7); i++)
	{
		if (!strncmp(sahara_map_arr[i].mdm_name, mdm_name,
					strlen(sahara_map_arr[i].mdm_name))) {
			//Found the path where the images should be as per the
			//new naming convention. Check if the path exists.If
			//not they would be at the default path.
			if (stat(sahara_map_arr[i].mdm_image_root_dir, &st)) {
				ALOGI("Unable to find %s.Using default mapping",
						strerror(errno));
				continue;
			}
			break;
		}
	}
	ALOGI("Using sahara mapping for %s", sahara_map_arr[i].mdm_name);
	return sahara_map_arr[i].mappings;
}

static char* get_fw_image_path(char *mdm_name)
{
	struct stat st;
	int i;
	for (i = 0; strncmp(sahara_map_arr[i].mdm_name, "DEFAULT", 7); i++)
	{
		if (!strncmp(sahara_map_arr[i].mdm_name, mdm_name,
					strlen(sahara_map_arr[i].mdm_name))) {
			//Found the path where the images should be as per the
			//new naming convention. Check if the path exists.If
			//not they would be at the default path.
			if (stat(sahara_map_arr[i].mdm_image_root_dir, &st)) {
				ALOGI("Unable to find %s.Using default mapping",
						strerror(errno));
				continue;
			}
			break;
		}
	}
	ALOGI("Returning image path %s for %s",
			sahara_map_arr[i].mdm_image_root_dir,
			sahara_map_arr[i].mdm_name);
	return sahara_map_arr[i].mdm_image_root_dir;
}

int mdm_img_transfer(char* mdm_name,
                      char* mdm_link,
		      char* mdm_link_extra_info,
                      char* ram_dump_path,
                      char* saved_file_prefix,
                      img_transfer_mode mode)
{
  char pcie_port_path[PATH_MAX] = {0};
  char **sahara_mappings;
  struct stat st;

  if (!mdm_name || !mdm_link) {
	  ALOGE("Invalid arguments passed to %s", __func__);
	  return false;
  }
  ALOGI("MDM name '%s', Link type '%s'", mdm_name, mdm_link);
  sahara_mappings = get_sahara_mappings(mdm_name);

  if (!strncmp(mdm_link, LINK_TYPE_PCIE, strlen(LINK_TYPE_PCIE))) {
      if (mdm_link_extra_info) {
	      snprintf(pcie_port_path,
			      sizeof(pcie_port_path) - 1,
			      "/dev/mhi_%s%s",
			      mdm_link_extra_info,
			      MHI_PIPE_SAHARA);
      } else {
	      snprintf(pcie_port_path,
			      sizeof(pcie_port_path) - 1,
			      "%s",
			      PCIE_PORT_NAME);
      }
		return load_sahara(pcie_port_path,
                         ram_dump_path,
                         saved_file_prefix,
                         mode,
                         RAM_DUMP_IMAGE_NONE,
                         sahara_mappings);

  }
  else {
    ALOGE("Invalid MDM and link type");
  }

  return RET_FAILED;
}

