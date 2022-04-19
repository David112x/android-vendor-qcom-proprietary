/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2012-2014, 2018 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 *  ---------------------------------------------------------------------------
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <linux/esoc_ctrl.h>
#include <linux/ioctl.h>
#include <cutils/properties.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <mdm_helper.h>
#include <cutils/android_reboot.h>


#define NUM_ESOC_THREADS 2
#define MDM9K_IMAGE_PATH "/firmware/image/"
#define ESOC_CMD_MASK_FILE "/sys/bus/esoc/drivers/mdm-4x/command_mask"
#define ESOC_NOTIFIER_MASK_FILE "/sys/bus/esoc/drivers/mdm-4x/notifier_mask"
#define PON_MASK_CMD "PON"
#define PANIC_NOTIFIER_MASK "PANIC"
#define NUM_COMPORT_RETRIES 50
#define MAX_PATH_LEN 255
#define MODE_BOOT 1
#define MODE_RUNTIME 2
#define MODE_RAMDUMP 3
#define PCIE_DEBUGFS_NODE "/sys/kernel/debug/pci-msm/ep_wakeirq"
#define PCIE_DEBUG_ENABLE_VAL "0"
#define KS_PATH "/vendor/bin/ks"
#define RAM_DUMP_IMAGE 21
#define NUM_EFS_PARTITIONS 3
#define NUM_OTHER_HEADER_PREPEND_FILES 1
#define NUM_LINK_RETRIES 50
#define DELAY_BETWEEN_RETRIES_MS 500
#define NUM_COM_PORT_RETRIES 50
#define MAX_STATUS_UP_CHECKS 60
#define BOOT_TYPE_NODE "/proc/sys/kernel/cold_boot"
#define ARG_INFILE 1
#define INFILE_STRING "--infile"
#define DEFAULT_INFILE "/dev/mhi_pipe_24"

typedef __u32 u32;

//Maintain backward compatibility for older esoc drivers which
//do not support retry requests
#ifndef ESOC_PON_RETRY
    #define ESOC_PON_RETRY ESOC_BOOT_FAIL
#endif

#ifndef ESOC_REQ_CRASH_SHUTDOWN
    #define ESOC_REQ_CRASH_SHUTDOWN 0
#endif

typedef enum Mdm9kEsocEvents {
	ESOC_EVENT_REQ_ENG_RDY = 0,
	ESOC_EVENT_REQ_XFER_IMG
} Mdm9kEsocEvents;

struct image_id_mapping {
	int image_id;
	char* filename;
};

struct headers_to_prepend {
	struct image_id_mapping file_details;
	char* header_file;
	char* binary_file;
};
struct partitions_to_file_dump {
	struct headers_to_prepend header_info;
	char *partition;
	size_t kb;
};

struct mdm_private_data {
	int reset_device_before_ramdumps;
	char* efs_sync_device;
	// Prefix added to files received as RAM dumps, this
	// is used when launching kickstart for flashless boot
	char* ram_dump_file_prefix;
	// Prefix added to files recveived during an EFS sync. This
	// is used when launching kickstart for EFS sync
	char* efs_sync_file_prefix;
	char* efs_file_dir;
	pid_t efs_ks_pid;
	bool collected_dumps;
};

typedef enum {
    WAKELOCK_RELEASE,
    WAKELOCK_ACQUIRE
} wakelock_action;
bool use_wakelock (wakelock_action action);

struct mdm_private_data private_sdx_data;

int mdm9k_powerup(struct mdm_device *dev);
int mdm9k_sahara(struct mdm_device *dev);
int mdm9k_shutdown(struct mdm_device *dev);
int mdm9k_monitor(struct mdm_device *);
int mdm9k_ramdump_collect(struct mdm_device *);
int mdm9k_cleanup(struct mdm_device *);
int WaitForCOMport(char *DevNode, int max_retries, int attempt_read);
