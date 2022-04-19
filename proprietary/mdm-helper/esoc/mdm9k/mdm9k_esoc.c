/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2012-2014, 2018-2019 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 *  ---------------------------------------------------------------------------
 */

#include "mdm9k_esoc.h"
#include "mdm_img_transfer.h"

int efs_sync_kill(int pid);
#define MDM_BOOT_TIMEOUT_SEC 10
#define DEBUG_MODE_FLAG "/persist/debug_mode"
#define MANUAL_MODE_FLAG "/persist/manual_mode"
#define TIMEOUT_PROPERTY "persist.vendor.mdm_helper.timeout"
#define PAUSE_PROPERTY "persist.vendor.mdm_helper.pause"
#define VOLD_DECRYPT_PROP "vold.decrypt"
#define RESTART_FRAMEWORK_VAL "trigger_restart_framework"
#define RESTART_FRAMEWORK_MIN_VAL "trigger_restart_framework_min"
#define SKILL SIGKILL

#define WAKELOCK_NAME "mdm_helper"
#define WAKELOCK_FILE "/sys/power/wake_lock"
#define WAKEUNLOCK_FILE "/sys/power/wake_unlock"

#define BOOTFAIL_ACCESFAIL_RETRY_MAX 0  // change to 1 for 1 retry on kickstart failure

static int manual_mode_enabled;
static int debug_mode_enabled;
static int boot_timeout;

struct mdm_private_data private_sdx_data = {
	.reset_device_before_ramdumps = 1,
	.efs_sync_device = "/dev/mhi_pipe_10",
	.ram_dump_file_prefix = NULL,
	.efs_sync_file_prefix = "mdm1",
	.efs_ks_pid = 0,
	.efs_file_dir = "/dev/block/bootdevice/by-name/",
	.collected_dumps = false
};
static int wait_for_mdm_boot(struct mdm_device *dev, int verbose)
{
	int status;
	int log_count = 0;
	int retry_count = 0;

	if (boot_timeout == 0 || manual_mode_enabled || debug_mode_enabled) {
		ALOGI("%s: MDM_BOOT_TIMEOUT is disabled",
				dev->mdm_name);
	}
	do {
		if (boot_timeout != 0 && retry_count >= boot_timeout) {
			ALOGE("%s: Failed to boot within timeout",
					dev->mdm_name);
			return RET_FAILED;
		}
		if(ioctl(dev->device_descriptor,
					ESOC_GET_STATUS,
					&status) < 0) {
			ALOGE("%s: ESOC_GET_STATUS ioctl failed",
					dev->mdm_name);
			goto error;
		}
		if(status == 1) {
			ALOGI("%s: MDM2AP_STATUS is now high", dev->mdm_name);
			return RET_SUCCESS;
		}
		if (verbose || log_count++ == 0)
			ALOGI("%s: Waiting for mdm boot", dev->mdm_name);
		usleep(1000000);
		retry_count++;
	} while(1);
error:
	return RET_FAILED;
}

//Send a notification to the mdm-driver
static int send_esoc_notification(struct mdm_device *dev, int cmd)
{
	if (!dev) {
		ALOGE("NULL device descriptor passed to send_esoc_notification");
		goto error;
	}
	if (ioctl(dev->device_descriptor, ESOC_NOTIFY, &cmd) < 0) {
		ALOGE("%s: Failed to send notification",
				dev->mdm_name);
		goto error;
	}
	return RET_SUCCESS;
error:
	return RET_FAILED;
}

#ifdef ESOC_BOOT_FAIL_ACTION
//Send request to change boot fail action to esoc
static int set_esoc_boot_fail_action(struct mdm_device *dev, int action)
{
	int rc;
	if (!dev) {
		ALOGE("NULL device descriptor passed to set_esoc_boot_fail_action");
		goto error;
	}
	rc = ioctl(dev->device_descriptor, ESOC_SET_BOOT_FAIL_ACT, &action);
	if (rc < 0) {
		ALOGE("%s: Failed to set boot fail action %d",
				dev->mdm_name, rc);
		goto error;
	}
	return RET_SUCCESS;
error:
	return RET_FAILED;
}
#endif

static int do_initial_setup(struct mdm_device *dev)
{
	if (!dev) {
		ALOGE("Invalid dev passed to do_initial_Setup");
		goto error;
	}
	ALOGI("%s: Initializing environment", dev->mdm_name);
	if (dev->device_descriptor == -1) {
		dev->device_descriptor = open(dev->mdm_port, O_RDONLY | O_NONBLOCK);

		if (dev->device_descriptor < 0) {
			ALOGE("%s: Failed to open mdm device node: %s",
					dev->mdm_name,
					strerror(errno));
			goto error;
		}
		if (ioctl(dev->device_descriptor, ESOC_REG_REQ_ENG) < 0) {
			ALOGE("%s: Failed to set thread as request engine",
					dev->mdm_name);
			goto error;
		}
	}
	//Wait for the vold to get done with decryption of the data
	// partition
/*	do {
		property_get(VOLD_DECRYPT_PROP,
				data_partition_state,
				"N/A");
		if(!strncmp(data_partition_state,
					RESTART_FRAMEWORK_VAL,
					sizeof(RESTART_FRAMEWORK_VAL))||
			!strncmp(data_partition_state,
				RESTART_FRAMEWORK_MIN_VAL,
				sizeof(RESTART_FRAMEWORK_MIN_VAL))){
			ALOGE("Data partition ready");
			break;
		} else {
			ALOGE("Waiting for vold to finish device encryption");
			usleep(1000000);
		}
	} while(1); */
	return RET_SUCCESS;
error:
	return RET_FAILED;
}

static int setup_efs_sync(struct mdm_device *dev)
{
	struct mdm_private_data *pdata;
	char *efs_sync_dev = NULL;
	char buf[PATH_MAX] = {0};
	int ret = 0;
	if (!dev) {
		ALOGE("Invalid device structure passed to %s",
				__func__);
		return RET_FAILED;
	}
	if (!dev->private_data) {
		ALOGE("%s: Private data not found", dev->mdm_name);
		return RET_FAILED;
	}
	pdata = (struct mdm_private_data *)dev->private_data;
	if (!strncmp(dev->mdm_link_extra_data, "N/A", 3))
		efs_sync_dev = pdata->efs_sync_device;
	else {
		snprintf(buf, sizeof(buf) - 1, "/dev/mhi_%s%s",
				dev->mdm_link_extra_data,
				MHI_PIPE_EFS_SYNC);
		efs_sync_dev = buf;
	}

	char *efs_sync_args[] = {
		KS_PATH,"-m",
		"-p", efs_sync_dev,
		"-w", pdata->efs_file_dir,
		"-t", "-1",
		"-l",
		(pdata->efs_sync_file_prefix == NULL) ? NULL : "-g",
		pdata->efs_sync_file_prefix,
		NULL };

	//Wait for a max of 100 seconds for EFS sync port to appear.
	int num_retries = (!boot_timeout) ? INT_MAX : MAX_RETRIES_FOR_MHI_BUS_EFS;

	if(efs_sync_dev) {
		if(WaitForCOMport(efs_sync_dev,
					num_retries, 0) !=
				RET_SUCCESS) {
			ALOGE("%s: Could not detect EFS sync port",
				dev->mdm_name);
			goto error;
		}
	} else {
		ALOGE("%s: No efs_sync_device specified for target",
				dev->mdm_name);
		goto error;
	}
	if ( pdata->efs_ks_pid > 0) {
		ALOGI("%s: Killing earlier EFS Sync instance pid:%d",dev->mdm_name,pdata->efs_ks_pid);
		efs_sync_kill(pdata->efs_ks_pid);
	}
	ret = fork();
	if (ret < 0) {
		ALOGE("%s: Failed to create efs sync process", dev->mdm_name);
		goto error;
	} else if(ret == 0) {
		if (execve(KS_PATH, efs_sync_args, NULL) < 0) {
			ALOGE("%s: Failed to exec KS process for efs sync",
					dev->mdm_name);
			_exit(127);
		}
	} else {
		// Store KS efs sync pid given to parent for later cleanup.
		pdata->efs_ks_pid = ret;
	}
	return RET_SUCCESS;
error:
	return RET_FAILED;
}

int efs_sync_kill(int pid) {

    int process_retval;
	char* name = (char*)calloc(32,sizeof(char));

	if(name){
		snprintf(name, 32, "/proc/%d/comm",pid);
		FILE* f = fopen(name,"r");
		if(f){
			size_t size;
			size = fread(name, sizeof(char), 32, f);
			if(size>0){
				if('\n'==name[size-1])
					name[size-1]='\0';
			}
			fclose(f);
		}
		if(!strcmp(name, "ks")) {
			kill(pid, SKILL);
			waitpid(pid, &process_retval, 0);
		}
		else
			ALOGI("ks not running, hence skipping the termination");
		free(name);
	}
	return RET_SUCCESS;
}

int mdm9k_powerup(struct mdm_device *dev)
{
	u32 cmd;
	char timeout_property_string[PROPERTY_VALUE_MAX];

	if (!dev) {
		ALOGE("Invalid device structure passed to %s",
				__func__);
		return RET_FAILED;
	}
	if (!dev->private_data) {
		ALOGE("%s: Private data not found", dev->mdm_name);
		return RET_FAILED;
	}

	property_get(TIMEOUT_PROPERTY, timeout_property_string, "");
	if(sscanf(timeout_property_string,"%d",&boot_timeout)!= 1 || boot_timeout < 0){
		boot_timeout = MDM_BOOT_TIMEOUT_SEC;
	}

	if (do_initial_setup(dev) != RET_SUCCESS)
		goto error;

	return RET_SUCCESS;

error:
	cmd = ESOC_IMG_XFER_FAIL;
	if (dev) {
		if (send_esoc_notification(dev, cmd) != RET_SUCCESS)
			ALOGE("%s: Failed to send IMG_XFER_FAIL notification",
					dev->mdm_name);
	}
	return RET_FAILED;
}


bool use_wakelock (wakelock_action action) {

	static bool wakelock_works = true;
	bool retval = true;

	const char *filename;
	int ret = 0;
	int wake_lock_name_length = sizeof(WAKELOCK_NAME);
	int fd;

	if (false == wakelock_works)
		return false;

	if (action == WAKELOCK_ACQUIRE) {
		filename = WAKELOCK_FILE;
	}
	else {
		filename = WAKEUNLOCK_FILE;
	}
	fd = open(filename, O_WRONLY|O_APPEND);

	if (fd < 0) {
		ALOGE("Failed to open wake lock file %s: %s", filename, strerror(errno));
		wakelock_works = false;
		return false;
	}

	ret = write(fd, WAKELOCK_NAME, wake_lock_name_length);
	if (ret != wake_lock_name_length) {
		ALOGE("Error writing to %s: %s", filename, strerror(errno));
		retval = false;
	}
	else {
		ALOGI("Wrote to %s", filename);
		retval = true;
	}

	close(fd);
	return retval;
}

/*
 * Detect for write access permission on ramdump path location.
 * This can fail on user variant builds due to security enforcement
 * that disables ramdump on commercial (user) config.
 *
 * Check only for access denied.  All other failures are critical
 * as that indicates unsupported/misconfigurations.
 */
static int is_ramdump_storage_valid() {
	static int write_possible = -1;
	int rc;

	if (write_possible == -1) {
		rc = access(RAMDUMP_ROOT_DIR, R_OK|W_OK);
		if ((rc != 0) && (errno == EACCES))  {
			write_possible = 0;
		} else {
			write_possible = 1;
		}
	}
	return write_possible;
}

/*
 * Check whether to retry on SDX bootup failure based on ramdump
 * path write access.
 *
 * This is a workaround on devices with mismatched SDX ramdump debug
 * configuration (enabled) where Android has it disabled due to
 * user build variant.
 *
 * Requires kernel and module support of ESOC_BOOT_FAIL_ACTION
 *
 * Side:  Counter will auto-decrement on each invocation
 * Input: init_counter - if set to 1 will reset counter
 */
static unsigned int is_bootfail_retry_available(int init_counter) {
	static unsigned int ramdump_path_access_fail_reboot_remainder = 0;

	if (!is_ramdump_storage_valid()) {
		// Retry on failed bootup due to known firmware load error if ramdump
		// path access is denied but there are ramdumps to collect retry is
		// only possible if ESOC_BOOT_FAIL_ACTION handling is supported
		if (init_counter) {
			ramdump_path_access_fail_reboot_remainder = BOOTFAIL_ACCESFAIL_RETRY_MAX;
		} else if (ramdump_path_access_fail_reboot_remainder > 0) {
			return ramdump_path_access_fail_reboot_remainder--;
		}
	}
	return ramdump_path_access_fail_reboot_remainder;
}

/*
 * The sahara function takes care of standard image load / ramdump collect,
 * waiting for subsequent boot, and efs sync establishment.
 * This state is reached only after receiving a REQ_IMG from esoc.
 */
int mdm9k_sahara(struct mdm_device *dev){
	static int power_on_count = 0;
	static int ramdump_accessfail_init = 0;
	u32 cmd;
	int img_ret = RET_FAILED;
	int image_xfer_complete = 0;
	char pause_property_string[PROPERTY_VALUE_MAX];
#ifdef ESOC_BOOT_FAIL_ACTION
	u32 action;
#endif

	char *extra_link_data = NULL;
	struct mdm_private_data *pdata = NULL;

	if (!dev) {
		ALOGE("Invalid device structure passed to %s",
				__func__);
		return RET_FAILED;
	}
	if (!strncmp(dev->mdm_link_extra_data, "N/A", 3))
		extra_link_data = NULL;
	else
		extra_link_data = dev->mdm_link_extra_data;

	pdata = (struct mdm_private_data*)dev->private_data;
	if (!pdata) {
		ALOGE("private data is NULL");
		return RET_FAILED;
	}

	// Check whether ramdump path is writable.  This can be the case
	// of user variant build
	if (!is_ramdump_storage_valid()) {
		ALOGE("Ramdump path not writable; ramdumps will not be collected '%s'",
			RAMDUMP_ROOT_DIR);
		if (!ramdump_accessfail_init) {
			is_bootfail_retry_available(1);  // reset retry counter
			ramdump_accessfail_init = 1;
		}
	}

	use_wakelock(WAKELOCK_ACQUIRE);

	// Wait to initiate boot if either pause property is set to true,
	// or dumps have just been collected and pause property is set to on_dump.
	// If we have just collected dumps, wait while the dump collection
	// property is set.
	property_get(PAUSE_PROPERTY, pause_property_string, "false");

	while(!strncmp(pause_property_string, "true", 4) ||
		(!strncmp(pause_property_string, "on_dump",7)
		&& pdata->collected_dumps)){

		property_get(PAUSE_PROPERTY, pause_property_string,
			"false");

		ALOGI("%s: Sahara operations paused. Clear persist.vendor.mdm_helper.pause to continue.",
			dev->mdm_name);
		usleep(1000000);
	}
	pdata->collected_dumps = false;

	ALOGI("%s: Beginning sahara image transfer.",dev->mdm_name);

	img_transfer_mode mode = (!boot_timeout) ? MODE_NO_TIMEOUT : MODE_DEFAULT;

	img_ret = mdm_img_transfer(
					dev->mdm_name,
					dev->mdm_link,
					extra_link_data,
					dev->ram_dump_path,
					((struct mdm_private_data *)dev->private_data)\
					->ram_dump_file_prefix,
					mode);
	if (img_ret == RET_FAILED || img_ret == RET_RETRY)
		goto error;
	image_xfer_complete = 1;
	if (send_esoc_notification(dev, ESOC_IMG_XFER_DONE) != RET_SUCCESS) {
		ALOGE("%s: Failed to send IMG_XFER_DONE notification",
				dev->mdm_name);
		goto error;
	}
	//Wait for MDM2AP_STATUS to go high.
	if (wait_for_mdm_boot(dev, 1) != RET_SUCCESS) {
		ALOGE("%s: Failed to boot", dev->mdm_name);
		goto error;
	}

	if (setup_efs_sync(dev) != RET_SUCCESS)
			goto error;

	//For successful boot, re-initialize the property string (as strtok_r injects null characters)
	//followed by re-starting the tokenizer
#ifdef ESOC_BOOT_FAIL_ACTION
	strlcpy(boot_fail_property_string_copy, boot_fail_property_string, PROPERTY_VALUE_MAX);
	token = strtok_r(boot_fail_property_string_copy, ",", &saveptr);
#endif
	is_bootfail_retry_available(1);  // reset retry counter for successful bootup

	if (send_esoc_notification(dev, ESOC_BOOT_DONE) != RET_SUCCESS) {
		ALOGE("%s: Failed to send ESOC_BOOT_DONE notification",
				dev->mdm_name);
		use_wakelock(WAKELOCK_RELEASE);
		return RET_FAILED;
	}
	power_on_count++;
	use_wakelock(WAKELOCK_RELEASE);

	return RET_SUCCESS;
error:
	use_wakelock(WAKELOCK_RELEASE);
	if (manual_mode_enabled || debug_mode_enabled) {
		ALOGW("Sahara function error: Looping forever");
		do {
			sleep(5000);
		} while(1);
	}
	//KS returned EAGAIN, we will need a reset and retry
	//Propagate the retry back to esoc and state machine
	if (img_ret == RET_RETRY){
		dev->required_action = MDM_REQUIRED_ACTION_RETRY;
			pdata->collected_dumps = true;
		cmd = ESOC_PON_RETRY;
	}
	else if (!image_xfer_complete)
		cmd = ESOC_IMG_XFER_FAIL;
	else
		cmd = ESOC_BOOT_FAIL;
	if (dev) {
#ifdef ESOC_BOOT_FAIL_ACTION
		if (cmd == ESOC_IMG_XFER_FAIL || cmd == ESOC_BOOT_FAIL){
			if (is_bootfail_retry_available(0) > 0) {
				// Boot failure can be due to ramdump write failure - retry

				// TODO:  Require kickstart support to return a specific error if it failed due
				//        to write access failure.  Otherwise, we'll always unconditionally retry
				//        if BOOTFAIL_ACCESFAIL_RETRY_MAX is set to a value > 0.
				ALOGE("%s: Attempt reboot for possible ramdump write access failure", dev->mdm_name);
				action = BOOT_FAIL_ACTION_COLD_RESET;
			} else if (token != NULL) {
				// Use the token from boot fail property and send the corresponding action to esoc
				if(!strncmp(token, "nop", 3)){
					ALOGE("%s: Boot fail action = BOOT_FAIL_ACTION_NOP", dev->mdm_name);
					action = BOOT_FAIL_ACTION_NOP;
				}
				else if(!strncmp(token, "panic", 5)){
					ALOGE("%s: Boot fail action = BOOT_FAIL_ACTION_PANIC", dev->mdm_name);
					action = BOOT_FAIL_ACTION_PANIC;
				}
				else if(!strncmp(token, "cold_reset", 10)){
					ALOGE("%s: Boot fail action = BOOT_FAIL_ACTION_COLD_RESET", dev->mdm_name);
					action = BOOT_FAIL_ACTION_COLD_RESET;
				}
				else if(!strncmp(token, "shutdown", 8)){
					ALOGE("%s: Boot fail action = BOOT_FAIL_ACTION_SHUTDOWN", dev->mdm_name);
					action = BOOT_FAIL_ACTION_SHUTDOWN;
				}
				else if(!strncmp(token, "s3_reset", 8)){
					ALOGE("%s: Boot fail action = BOOT_FAIL_ACTION_S3_RESET", dev->mdm_name);
					action = BOOT_FAIL_ACTION_S3_RESET;
				}
				else{
					ALOGE("%s: Unrecognized config. action = BOOT_FAIL_ACTION_PANIC", dev->mdm_name);
					action = BOOT_FAIL_ACTION_PANIC;
				}
				//Move the token to the next boot fail sequence.
				token = strtok_r(NULL, ",", &saveptr);
				if (action == BOOT_FAIL_ACTION_NOP || action == BOOT_FAIL_ACTION_PANIC || action == BOOT_FAIL_ACTION_SHUTDOWN){
					strlcpy(boot_fail_property_string_copy, boot_fail_property_string, PROPERTY_VALUE_MAX);
					token = strtok_r(boot_fail_property_string_copy, ",", &saveptr);
				}
			} else {
				ALOGE("%s: Reached end of boot fail action property. Ending the boot fail action sequence with action = BOOT_FAIL_ACTION_PANIC", dev->mdm_name);
				action = BOOT_FAIL_ACTION_PANIC;
			}
			//Send the corresponding boot fail action to esoc
			if (set_esoc_boot_fail_action(dev, action) != RET_SUCCESS)
				ALOGE("%s: Failed to set boot fail action", dev->mdm_name);
		}
#endif
		if (send_esoc_notification(dev, cmd) != RET_SUCCESS)
			ALOGE("%s: Failed to send notification",
					dev->mdm_name);
	}
	return RET_FAILED;
}

/*
 * The shutdown function here is technically a post
 * shutdown cleanup. The notification will only be
 * recieved by mdm-helper once the 9k has finished
 * powering down. This function essentially exists
 * to clean up the HSIC link.
 */
int mdm9k_shutdown(struct mdm_device *dev)
{
	struct mdm_private_data *pdata = NULL;
	if (!dev) {
		ALOGE("Invalid device structure passed in");
		return RET_FAILED;
	}
	pdata = (struct mdm_private_data*)dev->private_data;
	if (!pdata) {
		ALOGE("private data is NULL");
		return RET_FAILED;
	}
	return RET_SUCCESS;
}

int mdm9k_monitor(struct mdm_device *dev)
{
	u32 request = 0;
	if (!dev) {
		ALOGE("%s: Invalid device structure passed to %s",
				dev->mdm_name,
				__func__);
		return RET_FAILED;
	}
	ALOGI("%s: Monitoring mdm", dev->mdm_name);
	if (ioctl(dev->device_descriptor,
				ESOC_WAIT_FOR_REQ, &request) < 0) {
		ALOGE("%s: ESOC_WAIT_FOR_REQ ioctl failed",
				dev->mdm_name);
		return RET_FAILED;
	}
	switch(request) {
	case ESOC_REQ_DEBUG:
		ALOGI("%s: Recieved request for ramdump collection",
				dev->mdm_name);
		dev->required_action = MDM_REQUIRED_ACTION_RAMDUMPS;
		break;
	case ESOC_REQ_IMG:
		ALOGI("%s: Recieved request to transfer images",
				dev->mdm_name);
		dev->required_action = MDM_REQUIRED_ACTION_NORMAL_BOOT;
		break;
	case ESOC_REQ_SHUTDOWN:
	case ESOC_REQ_CRASH_SHUTDOWN:
		ALOGI("%s: Recieved shutdown request",
				dev->mdm_name);
		dev->required_action = MDM_REQUIRED_ACTION_SHUTDOWN;
		break;
	default:
		ALOGE("%s: Unknown request recieved: %u", dev->mdm_name,
				request);
		break;
	}
	return RET_SUCCESS;
}

int mdm9k_ramdump_collect(struct mdm_device *dev)
{
	u32 cmd = ESOC_DEBUG_DONE;
	int dump_collection_complete = 0;
	char *extra_link_data = NULL;
	if (!dev) {
		ALOGE("%s: Invalid device structure passed to %s",
				dev->mdm_name,
				__func__);
		return RET_FAILED;
	}
	dev->required_action = MDM_REQUIRED_ACTION_RAMDUMPS;
	if (!strncmp(dev->mdm_link_extra_data, "N/A", 3))
		extra_link_data = NULL;
	else
		extra_link_data = dev->mdm_link_extra_data;
	if (mdm_img_transfer(
				dev->mdm_name,
				dev->mdm_link,
				extra_link_data,
				dev->ram_dump_path,
				((struct mdm_private_data *)dev->private_data)\
					->ram_dump_file_prefix,
				MODE_FORCE_RAM_DUMPS) != RET_SUCCESS) {
			ALOGE("%s: Failed to collect dumps", dev->mdm_name);
			return RET_FAILED;
	}
	if (ioctl(dev->device_descriptor, ESOC_NOTIFY,
				&cmd) < 0) {
		ALOGE("%s: :%s: Failed to send debug done notification",
				dev->mdm_name,
				__func__);
		goto error;
	}
	dump_collection_complete = 1;
	return RET_SUCCESS;
error:
	cmd = ESOC_DEBUG_FAIL;
	if (!dump_collection_complete) {
		if (ioctl(dev->device_descriptor, ESOC_NOTIFY,
					&cmd) < 0) {
			ALOGE("%s: :%s: Failed to reset mdm",
					dev->mdm_name,
					__func__);
		}
	}
	return RET_FAILED;
}

int mdm9k_cleanup(struct mdm_device *dev)
{
	if (debug_mode_enabled || manual_mode_enabled) {
		do {
			ALOGE("%s: Reached failed state with dbg mode set",
					dev->mdm_name);
			usleep(1000000000);
		} while(1);
	}
	ALOGE("%s: mdm-helper reached fail state", dev->mdm_name);
	return RET_FAILED;
}

int WaitForCOMport(char *DevNode, int max_retries, int attempt_read)
{
	struct stat status_buf;
	int i;

	ALOGI("Testing if port \"%s\" exists", DevNode);
	for (i = 0; i < max_retries && stat(DevNode, &status_buf) < 0; i++) {
		usleep(DELAY_BETWEEN_RETRIES_MS * 1000);
	}
	if (i == max_retries) {
		ALOGE("'%s' was not found", DevNode);
		return RET_FAILED;
	}
	if (attempt_read) {
		FILE *fd;
		ALOGI("Attempting to open port \"%s\" for reading", DevNode);
		for (i=0; i<NUM_COM_PORT_RETRIES && (fd = fopen(DevNode,"r"))==\
				NULL; i++) {
			ALOGE("Couldn't open \"%s\", %i of %i", DevNode, i+1,
					NUM_COM_PORT_RETRIES);
			usleep(DELAY_BETWEEN_RETRIES_MS*1000);
		}
		if (i == NUM_COM_PORT_RETRIES) {
			ALOGE("'%s' could not be opened for reading", DevNode);
			return RET_FAILED;
		}
		fclose(fd);
	}
	return RET_SUCCESS;
}
