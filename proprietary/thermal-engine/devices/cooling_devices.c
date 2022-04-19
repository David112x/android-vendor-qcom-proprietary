/*===========================================================================

  Copyright (c) 2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "devices_manager_internal.h"
#include "thermal.h"

struct tmd_cdev_info_t {
	char name[DEVICES_MAX_NAME_LEN];
	int max_state;
	int cur_state;
	int32_t cdev_id;
	pthread_mutex_t mtx;
	void *data;
};

/*========================================================================
FUNCTION cooling_device_set_cur_request

Action function to request cooling_device_set_cur_request.

ARGUMENTS
	dev_mgr - Device manager pointer

RETURN VALUE
	0 on success, -1 on failure.
===========================================================================*/
int cooling_device_set_cur_request(struct devices_manager_dev *dev_mgr)
{
	char buf[UINT_BUF_MAX] = {0};
	char node_buf[MAX_PATH] = {0};
	struct tmd_cdev_info_t *cdev = (struct tmd_cdev_info_t *)dev_mgr->data;
	int active_req = dev_mgr->active_req.value, ret = -1;

	if (active_req < 0 || dev_mgr->dev_info.cooling_dev_id < 0) {
		msg("Incorrect value %d value for cooling dev: %s with id:%d\n",
			active_req, cdev->name, dev_mgr->dev_info.cooling_dev_id);
		return ret;
	}

	pthread_mutex_lock(&cdev->mtx);

	if (active_req > cdev->max_state)
		active_req = cdev->max_state;

	if (cdev->cur_state != active_req) {
		snprintf(node_buf, MAX_PATH, CDEV_CUR_STATE,
				dev_mgr->dev_info.cooling_dev_id);
		snprintf(buf, UINT_BUF_MAX, "%d", active_req);
		if (write_to_file(node_buf, buf, strlen(buf)) > 0) {
			dbgmsg("ACTION: %s - "
			     "Setting %s Level to %d\n",
				cdev->name, cdev->name, active_req);
			thermalmsg(LOG_LVL_DBG,
				(LOG_LOGCAT | LOG_LOCAL_SOCKET | LOG_TRACE),
				"%s:%s:%d\n", MITIGATION, cdev->name, active_req);
			cdev->cur_state = active_req;
			ret = 0;
		} else {
			msg("Unable to set %s cdev state to %d\n", cdev->name,
				active_req);
		}
	} else {
		dbgmsg("Already in %d mitigation level for %s\n", active_req,
			cdev->name);
		ret = 0;
	}

	pthread_mutex_unlock(&cdev->mtx);

	return ret;
}

/*========================================================================
FUNCTION cooling_device_get_cur_request

Action function to request cooling_device_set_cur_request.

ARGUMENTS
	dev_mgr -> Device manager pointer.
	lvl     -> Pointer to be filled with current level.

RETURN VALUE
	0 on success, -1 on failure.
===========================================================================*/
int cooling_device_get_cur_request(struct devices_manager_dev *dev_mgr, int *lvl)
{
	char buf[UINT_BUF_MAX] = {0};
	char node_buf[MAX_PATH] = {0};
	int curr_lvl;
	char *end_ptr;
	struct tmd_cdev_info_t *cdev = NULL;

	if (dev_mgr == NULL || dev_mgr->dev_info.cooling_dev_id < 0) {
		msg("%s: Invalid input\n", __func__);
		return -1;
	}
	cdev = (struct tmd_cdev_info_t *)dev_mgr->data;

	pthread_mutex_lock(&cdev->mtx);
	snprintf(node_buf, MAX_PATH, CDEV_CUR_STATE,
			dev_mgr->dev_info.cooling_dev_id);
	if (read_line_from_file(node_buf, buf, UINT_BUF_MAX) <= 0) {
		msg("Could not read cur state for %s with cdev id:%d\n",
		    dev_mgr->dev_info.name, dev_mgr->dev_info.cooling_dev_id);
		pthread_mutex_unlock(&cdev->mtx);
		return -1;
	}
	pthread_mutex_unlock(&cdev->mtx);

	curr_lvl = (int)strtol(buf, &end_ptr, 10);
	if (curr_lvl == LONG_MIN || curr_lvl == LONG_MAX ||
		(curr_lvl == 0 && (end_ptr == buf || *end_ptr != '\n'))) {
		msg("Invalid cur state for %s %d\n", dev_mgr->dev_info.name,
			curr_lvl);
		return -1;
	}
	*lvl = curr_lvl;

	return 0;
}

static void cooling_device_release(struct devices_manager_dev *dev_mgr)
{
	if (dev_mgr) {
		if (dev_mgr->lvl_info)
			free(dev_mgr->lvl_info);
		if (dev_mgr->data)
			free(dev_mgr->data);
		free(dev_mgr);
	}
}

static void init_cooling_device(int cdev_id)
{
	char path[DEVICES_MAX_PATH] = {0};
	char buf[DEVICES_MAX_PATH] = {0};

	/* Clear any previous mitigation on cooling device*/
	snprintf(path, DEVICES_MAX_PATH, CDEV_CUR_STATE, cdev_id);
	snprintf(buf, DEVICES_MAX_PATH, "0");
	write_to_file(path, buf, strlen(buf));
}

int tmd_add_cooling_device(struct tmd_cdev_info_t *cdev)
{
	int ret_val = 0;
	uint32_t num_lvls = 0;
	struct devices_manager_dev *dev = NULL;
	int arr_idx = 0;

	if (cdev == NULL || cdev->max_state <= 0) {
		return -(EINVAL);
	}

	dev = calloc(1, sizeof(struct devices_manager_dev));
	if (dev == NULL) {
		msg("%s: calloc failed.\n", __func__);
		ret_val = -(ENOMEM);
		goto handle_error;
	}

	/* Max_state is index, so number of levels = max_state + 1 */
	num_lvls = (uint32_t)(cdev->max_state + 1);

	dev->lvl_info = calloc(num_lvls, sizeof(struct device_lvl_info));
	if (dev->lvl_info == NULL) {
		msg("%s: calloc failed.\n", __func__);
		ret_val = -(ENOMEM);
		goto handle_error;
	}
	snprintf(dev->dev_info.name, DEVICES_MAX_NAME_LEN, "%s", cdev->name);

	for (arr_idx = 0; (uint32_t)arr_idx < num_lvls; arr_idx++) {
		dbgmsg("%s: %s lvl_info[%d]=%d", __func__,
			dev->dev_info.name, arr_idx, arr_idx);
		dev->lvl_info[arr_idx].lvl.value = arr_idx;
		dev->lvl_info[arr_idx].perf_lvl = arr_idx;
	}

	dev->dev_info.num_of_levels = num_lvls;
	dev->dev_info.dev_type = DEVICE_CDEV_TYPE;
	/* Needed to act as generic device */
	dev->dev_info.min_lvl = 0;

	/* Needed to act as device_op_type device */
	dev->dev_info.max_dev_op_value_valid = 1;
	dev->dev_info.max_dev_op_value = dev->lvl_info[0].lvl.value;
	dev->active_req.value = dev->dev_info.max_dev_op_value;
	dev->dev_info.min_dev_op_value_valid = 1;
	dev->dev_info.min_dev_op_value =
		dev->lvl_info[num_lvls - 1].lvl.value;
	dev->action = cooling_device_set_cur_request;
	dev->release = cooling_device_release;
	dev->current_op = cooling_device_get_cur_request;
	pthread_mutex_init(&(cdev->mtx), NULL);

	strlcpy(dev->dev_info.device_units_name,
		UNIT_NAME_LVL, MAX_UNIT_NAME_SIZE);
	dev->dev_info.cooling_dev_id = cdev->cdev_id;
	init_cooling_device(cdev->cdev_id);

	dev->data = (struct tmd_cdev_info_t *)cdev;
	cdev->data = dev;

	if (devices_manager_add_dev(dev) != 0) {
		msg("%s: Can not add device\n", __func__);
		ret_val = -(EFAULT);
	}

handle_error:
	if (ret_val < 0) {
		/* Error clean up */
		if (dev) {
			if (dev->lvl_info)
				free(dev->lvl_info);
			free(dev);
		}
	}

	return ret_val;
}

static void parse_cooling_devices(void)
{
	DIR *tdir = NULL;
	struct dirent *tdirent = NULL;
	char name[MAX_PATH] = {0};
	char cwd[MAX_PATH] = {0};
	int ret = 0;

	if (!getcwd(cwd, sizeof(cwd)))
		return;

	/* Change dir to read the entries. Doesn't work otherwise */
	ret = chdir(THERMAL_SYSFS);
	if (ret < 0) {
		msg("Unable to change path to %s, err:%d\n",
			THERMAL_SYSFS, errno);
		return;
	}

	tdir = opendir(THERMAL_SYSFS);
	if (!tdir || ret) {
		msg("%s: Unable to open %s\n", __func__, THERMAL_SYSFS);
		ret = chdir(cwd); /* Restore current working dir */
		if (ret < 0)
			msg("Unable to change path to %s,err:%d\n", cwd, errno);
		return;
	}

	while ((tdirent = readdir(tdir))) {
		char buf[MAX_PATH];
		DIR *cdevdir = NULL;
		struct tmd_cdev_info_t *cdev = NULL;
		struct devices_manager_dev *dev_mgr = NULL;;

		/* Skip if it is not cooling device */
		if (strncmp(tdirent->d_name, CDEV_DIR_NAME,
					strlen(CDEV_DIR_NAME)))
			continue;

		cdevdir = opendir(tdirent->d_name);
		if (!cdevdir)
			continue;
		if (snprintf(name, MAX_PATH, THERMAL_TYPE, tdirent->d_name) < 0)
			msg("%s: truncation error\n", __func__);
		ret = read_line_from_file(name, buf, sizeof(buf));
		if (ret <= 0) {
			closedir(cdevdir);
			continue;
		}
		buf[strlen(buf) - 1] = '\0';

		/* Check same device is already added by legacy framework, if yes,
		   skip that cooling device assuming it is taken care by legacy framework */
		dev_mgr = NULL;
		dev_mgr = find_dev(buf);
		if (dev_mgr) {
			closedir(cdevdir);
			continue;
		}

		cdev = calloc(1, sizeof(*cdev));
		if (cdev == NULL) {
			msg("%s: calloc failed.\n", __func__);
			ret = -(ENOMEM);
			closedir(cdevdir);
			continue;
		}

		if (strlcpy(cdev->name, buf,
			DEVICES_MAX_NAME_LEN) >= DEVICES_MAX_NAME_LEN) {
			free(cdev);
			closedir(cdevdir);
			continue;
		}
		sscanf(tdirent->d_name, CDEV_DIR_FMT, &(cdev->cdev_id));

		if (snprintf(name, MAX_PATH, CDEV_MAX_STATE, cdev->cdev_id) < 0)
			msg("%s: truncation error\n", __func__);
		ret = read_line_from_file(name, buf, sizeof(buf));
		if (ret <= 0) {
			free(cdev);
			closedir(cdevdir);
			continue;
		}
		cdev->max_state = atoi(buf);
		cdev->cur_state = 0;
		ret = tmd_add_cooling_device(cdev);
		if (ret < 0) {
			free(cdev);
			closedir(cdevdir);
			continue;
		}
		dbgmsg("%s: Added cooling device: %s with cdev id:%d\n",
				__func__, cdev->name, cdev->cdev_id);
		closedir(cdevdir);
	}
	closedir(tdir);
	ret = chdir(cwd); /* Restore current working dir */
	if (ret < 0)
		msg("Unable to change path to %s, err:%d\n", cwd, errno);
}

int cooling_devices_init(void)
{
	parse_cooling_devices();

	return 0;
}

void cooling_devices_release(void)
{
}
