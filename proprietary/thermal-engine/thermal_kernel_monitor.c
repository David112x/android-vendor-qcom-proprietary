/*=============================================================
 *Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =============================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inc/thermal.h"
#include "inc/thermal_config.h"
#include "thermal_kernel_monitor.h"
#include "thermal_server.h"
#include "server/thermal_lib_common.h"

struct setting_info step_algo_zone;
struct thermal_zone_info *zones[MAX_NUM_TZ];
int step_count;

void step_wise_init_data(struct thermal_setting_t *setting)
{
	memset(&step_algo_zone, 0, sizeof(struct setting_info));

	step_count = 0;
	for (int i = 0; i < (int)tz_list.num_tz; i++) {
		if (strncmp(tz_list.tz_info_list[i].algo_type, "step_wise",
				MAX_TZ_NAME_LEN) == 0) {
			zones[step_count] = &(tz_list.tz_info_list[i]);
			++step_count;
		}
	}

	snprintf(step_algo_zone.desc, MAX_TZ_NAME_LEN, "STEP_WISE_ALGO");
	step_algo_zone.algo_type = STEP_WISE_ALGO_TYPE;
	step_algo_zone.data.step.step_zone_num = step_count;
	add_setting(setting, &step_algo_zone);
}

int override_notify(int algo_type, void *data, void *reserved)
{
	info("step wise override not supported");
	return 1;
}

int config_query_notify(int algo_type, void *data, void *reserved)
{
	struct thermal_msg_data *query_data = NULL;
	struct step_settings *s_setting = NULL;

	if ((algo_type != STEP_WISE_ALGO_TYPE) && (algo_type != ALGO_IDX_MAX))
		return 0;

	if (reserved != NULL)
		return step_count;

	query_data = (struct thermal_msg_data *)
			malloc(sizeof(struct thermal_msg_data));
	if (!query_data) {
		msg("%s: malloc error\n", __func__);
		return -1;
	}
	s_setting = &query_data->config.s_setting;

	for (int i = 0; i < step_count; ++i) {
		struct thermal_zone_info *step_zone = zones[i];
		struct trip_temp *trip;
		struct cooling_device *cdev;
		uint32_t j;

		memset(query_data, 0, sizeof(struct thermal_msg_data));

		query_data->msg_type = STEP_WISE_ALGO_TYPE;
		strlcpy(query_data->client_name, CONFIG_QUERY_CLIENT,
				CLIENT_NAME_MAX);
		strlcpy(query_data->config.config_desc, step_zone->sensor,
				MAX_TZ_NAME_LEN);
		query_data->config.disable = step_zone->mode ? 0 : 1;

		s_setting->trip_pt_num = step_zone->trip_temp_num;

		for (j = 0; j < step_zone->trip_temp_num; j++) {
			uint32_t k;
			trip = &(step_zone->trip_temp_list[j]);

			s_setting->trips[j].set_point =	trip->set_temp;
			s_setting->trips[j].set_point_clr = trip->clr_temp;
			s_setting->trips[j].device_num = trip->cdev_num;

			for (k = 0; k < trip->cdev_num; k++) {
				cdev = &(trip->cdev_list[k]);
				s_setting->trips[j].devs[k].upper_limit
					= cdev->upper_limit;
				s_setting->trips[j].devs[k].lower_limit
					= cdev->lower_limit;
				strlcpy(s_setting->trips[j].devs[k].name,
				        cdev->cdev_name, DEVICES_MAX_NAME_LEN);
			}
		}
		s_setting->polling_delay = step_zone->polling_delay;
		s_setting->passive_delay = step_zone->passive_delay;
		thermal_server_config_info_to_client(query_data);
	}

	free(query_data);
	return 0;
}

/*
 * validate_new_config_settings: Validates the new configuration settings
 * 				 from the client
 *
 * Parameter:
 * @new_config	- containing the new client settings for one
 * 		configuration/thermal zone
 * @tz_list_index - the thermal zone number
 *
 * Return: 0 if values are valid, -1 if values are not valid
 * */
static int validate_new_config_settings(struct thermal_msg_data* new_config,
		int tz_list_index)
{
	struct thermal_zone_info *step_zone = NULL;
	struct trip_temp *trip;
	struct step_settings *s_setting = &new_config->config.s_setting;

	if (tz_list_index > (int)tz_list.num_tz) {
		msg("%s: Invalid number thermal zone index: %d", __func__,
					tz_list_index);
		return -1;
	}

	if (new_config->config.fields_mask == UNKNOWN_FIELD)
		return -1;

	if (new_config->config.disable > 1)
		return -1;

	step_zone = &(tz_list.tz_info_list[tz_list_index]);
	if ((new_config->config.fields_mask & SET_POINT_FIELD) ||
		(new_config->config.fields_mask & SET_POINT_CLR_FIELD)) {
		int set_temp;
		int clr_temp;
		int hyst = 0;

		for (uint i = 0; i < step_zone->trip_temp_num; i++) {
			trip = &(step_zone->trip_temp_list[i]);
			if (new_config->config.fields_mask & SET_POINT_FIELD)
				set_temp = s_setting->trips[i].set_point;
			else
				set_temp = trip->set_temp;

			if (new_config->config.fields_mask & SET_POINT_CLR_FIELD)
				clr_temp = s_setting->trips[i].set_point_clr;
			else
				clr_temp = trip->clr_temp;

			hyst = set_temp - clr_temp;
			if (hyst < 0)
				break;
		}

		if (hyst < 0)
			return -1;
	}

	if ((new_config->config.fields_mask & UPPER_LIMIT_FIELD) ||
			(new_config->config.fields_mask & LOWER_LIMIT_FIELD)) {
		int up_limit;
		int low_limit;
		int diff = 0;
		struct step_device_info *devs = NULL;

		for (uint i = 0; i < step_zone->trip_temp_num; i++) {
			trip = &(step_zone->trip_temp_list[i]);
			for (uint j = 0; j < trip->cdev_num; j++) {
				devs = &s_setting->trips[i].devs[j];
				if (new_config->config.fields_mask &
					UPPER_LIMIT_FIELD)
					up_limit = devs->upper_limit;
				else
					up_limit =
						trip->cdev_list[j].upper_limit;

				if (new_config->config.fields_mask &
					LOWER_LIMIT_FIELD)
					low_limit = devs->lower_limit;
				else
					low_limit =
						trip->cdev_list[j].lower_limit;

				diff = up_limit - low_limit;
				if (diff < 0)
					break;
			}

			if (diff < 0)
				break;
		}

		if (diff < 0)
			return -1;
	}

	return 0;
}

static int write_int_to_sysfs(int value, char path[], const char call_func[])
{
	int ret;
	char num_string[UINT_BUF_MAX] = "";

	snprintf(num_string, UINT_BUF_MAX, "%i", value);
	ret = write_to_file(path, num_string, strlen(num_string));
	if (ret <= 0)
		msg("%s: Error writing to file %s", call_func, path);

	return ret;
}


static int get_tz_id_from_sensor_name(char *sensor_name)
{
	for (uint tz_id = 0; tz_id <= tz_list.num_tz; tz_id++)
		if (strcmp(tz_list.tz_info_list[tz_id].sensor, sensor_name) == 0)
			return (int)tz_id;
	return -1;
}

int config_parameter_set_notify(int num_cfg, void *data, void *reserved)
{
	int cfg_set_cnt = num_cfg;
	char desc_letters[MAX_TZ_NAME_LEN];
	int desc_num;
	struct thermal_zone_info *step_zone;
	struct trip_temp *trip;

	if (reserved == NULL || num_cfg <= 0 || (unsigned int)num_cfg > MAX_NUM_TZ)
		return -1;

	struct thermal_msg_data *current = (struct thermal_msg_data *)reserved;

	for (int index = 0; index < cfg_set_cnt; index++) {
		char path[MAX_PATH];
		int write_ret;
		int valid_ret;

		if (current[index].msg_type != STEP_WISE_ALGO_TYPE)
			continue;

		if (current[index].config.fields_mask == UNKNOWN_FIELD)
			continue;

		if (strstr(current[index].config.config_desc, "THERMAL_ZONE")) {
			sscanf(current[index].config.config_desc, "%13c%i", desc_letters, &desc_num);
		} else {
			/* There is possible that client may pass sensor name/zone->type as input
			   instead of thermal_zone id */
			desc_num = get_tz_id_from_sensor_name(current[index].config.config_desc);
			if (desc_num < 0)
				continue;
		}

		snprintf(path, MAX_PATH, TZ_MODE, desc_num);
		valid_ret = validate_new_config_settings(&(current[index]), desc_num);
		if (valid_ret < 0)
			continue;

		step_zone = &(tz_list.tz_info_list[desc_num]);

		write_ret = write_to_file(path, "disabled", strlen("disabled"));
		if (write_ret <= 0) {
			msg("%s: Error writing to mode sysfs, unable to disable, Thermal_Zone_%d", __func__, desc_num);
			continue;
		}

		info("zone %d disabled", desc_num);

		if (current[index].config.fields_mask & POLLING_DELAY_FIELD) {

			snprintf(path, MAX_PATH, TZ_POLLING_DELAY, desc_num);
			write_int_to_sysfs((int)current[index].config.s_setting.polling_delay, path, __func__);

		}

		if (current[index].config.fields_mask & PASSIVE_DELAY_FIELD) {
			snprintf(path, MAX_PATH, TZ_PASSIVE_DELAY, desc_num);
			write_int_to_sysfs((int)current[index].config.s_setting.passive_delay, path, __func__);
		}

		if ((current[index].config.fields_mask & SET_POINT_FIELD) ||
				(current[index].config.fields_mask & SET_POINT_CLR_FIELD)) {
			int set_temp;
			int clr_temp;
			int hyst;

			for (uint i = 0; i < step_zone->trip_temp_num; i++) {
				if (current[index].config.fields_mask & SET_POINT_FIELD) {
					set_temp = current[index].config.s_setting.trips[i].set_point;
					snprintf(path, MAX_PATH, TZ_SET_TEMP, desc_num, i);
					write_ret = write_int_to_sysfs(set_temp, path, __func__);

					//if set temp does not write, don't change hyst value
					if (write_ret <=0)
						continue;

				} else {
					set_temp = step_zone->trip_temp_list[i].set_temp;
				}

				if (current[index].config.fields_mask & SET_POINT_CLR_FIELD)
					clr_temp = current[index].config.s_setting.trips[i].set_point_clr;
				else
					clr_temp = step_zone->trip_temp_list[i].clr_temp;

				hyst = set_temp - clr_temp;
				snprintf(path, MAX_PATH, TZ_HYST, desc_num, i);
				write_ret = write_int_to_sysfs(hyst, path, __func__);
				if (write_ret <= 0) {
					//return set_temp to original value if hyst does not write
					set_temp = clr_temp + hyst;
					snprintf(path, MAX_PATH, TZ_SET_TEMP, desc_num, i);
					write_ret = write_int_to_sysfs(hyst, path, __func__);
				}
			}
		}

		if ((current[index].config.fields_mask & LOWER_LIMIT_FIELD) &&
				(current[index].config.fields_mask & UPPER_LIMIT_FIELD)) {
			int cdevID;
			int upper_limit;
			int lower_limit;

			for (uint i = 0; i < step_zone->trip_temp_num; i++) {
				trip = &(step_zone->trip_temp_list[i]);
				for (uint j = 0; j < trip->cdev_num; j++) {
					upper_limit = current[index].config.s_setting.trips[i].devs[j].upper_limit;
					lower_limit = current[index].config.s_setting.trips[i].devs[j].lower_limit;
					cdevID = trip->cdev_list[j].cdevID;

					if (upper_limit < trip->cdev_list[j].lower_limit) {
						snprintf(path, MAX_PATH, TZ_CDEV_LOWER_LIM, desc_num, cdevID);
						write_ret = write_int_to_sysfs(lower_limit, path, __func__);

						snprintf(path, MAX_PATH, TZ_CDEV_UPPER_LIM, desc_num, cdevID);
						write_ret = write_int_to_sysfs(upper_limit, path, __func__);
					} else {
						snprintf(path, MAX_PATH, TZ_CDEV_UPPER_LIM, desc_num, cdevID);
						write_ret = write_int_to_sysfs(upper_limit, path, __func__);

						snprintf(path, MAX_PATH, TZ_CDEV_LOWER_LIM, desc_num, cdevID);
						write_ret = write_int_to_sysfs(lower_limit, path, __func__);
					}
				}
			}
		} else if (current[index].config.fields_mask & LOWER_LIMIT_FIELD) {
			int cdevID;
			int lower_limit;
			for (uint i = 0; i < step_zone->trip_temp_num; i++) {
				trip = &(step_zone->trip_temp_list[i]);
				for (uint j = 0; j < trip->cdev_num; j++) {
					lower_limit = current[index].config.s_setting.trips[i].devs[j].lower_limit;
					cdevID = trip->cdev_list[j].cdevID;

					snprintf(path, MAX_PATH, TZ_CDEV_LOWER_LIM, desc_num, cdevID);
					write_ret = write_int_to_sysfs(lower_limit, path, __func__);
				}
			}
		} else if (current[index].config.fields_mask & UPPER_LIMIT_FIELD) {
			int cdevID;
			int upper_limit;
			for (uint i = 0; i < step_zone->trip_temp_num; i++) {
				trip = &(step_zone->trip_temp_list[i]);
				for (uint j = 0; j < trip->cdev_num; j++) {
					upper_limit = current[index].config.s_setting.trips[i].devs[j].upper_limit;
					cdevID = trip->cdev_list[j].cdevID;

					snprintf(path, MAX_PATH, TZ_CDEV_UPPER_LIM, desc_num, cdevID);
					write_ret = write_int_to_sysfs(upper_limit, path, __func__);
				}
			}
		}

		snprintf(path, MAX_PATH, TZ_MODE, desc_num);
		if (current[index].config.fields_mask & DISABLE_FIELD) {
			if (current[index].config.disable)
				continue;
		}

		write_ret = write_to_file(path, "enabled", strlen("enabled"));

		if (write_ret <= 0) {
			msg("%s: Error writing to mode sysfs, unable to enable, Thermal_Zone_%d", __func__, desc_num);
		} else {
			info("re-enabled zone %d", desc_num);
		}
	}
	get_thermal_zone_info();
	return 0;
}

void step_wise_algo_init(struct thermal_setting_t *setting)
{
	thermal_server_register_client_req_handler("override", override_notify, NULL);
	thermal_server_register_client_req_handler(CONFIG_QUERY_CLIENT, config_query_notify, NULL);
	thermal_server_register_client_req_handler(CONFIG_SET_CLIENT, config_parameter_set_notify, NULL);
}
