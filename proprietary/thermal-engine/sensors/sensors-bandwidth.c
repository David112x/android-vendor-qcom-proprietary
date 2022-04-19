/*===========================================================================
  copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sensors_manager_internal.h"
#include "thermal.h"
#include "sensors-bandwidth.h"
#include "thermal_server.h"

#define BW_SCORE_APPEND  "_score"
#define BW_UC_ID_APPEND "_uc_id"

enum sensor_type {
	BW_UTIL_SCORE,
	BW_UC_ID,
	BW_SENSOR_MAX,
};

struct bw_client_data {
	int value;
	char client_name[MAX_TZ_NAME_LEN];
	pthread_mutex_t bw_mutex;
	pthread_cond_t bw_condition;
	unsigned int threshold_reached;
	struct bw_sensor_info *sensors[BW_SENSOR_MAX];
};

struct bw_sensor_info {
	int value;
	enum sensor_type type;
	struct sensor_info *sensor;
	struct bw_client_data *clnt;
};

#ifdef ENABLE_CAMERA_REG_BW_CALLBACK
static char *bw_client_list[] =
{
	"camera_bw",
};
#else
static char *bw_client_list[] =
{
	"camera_bw",
	"display_bw",
};
#endif

static int bw_client_hal_callback(int val, void *data, void *reserved)
{
	struct bw_client_data *bw_clnt = (struct bw_client_data *)data;

	if (bw_clnt == NULL) {
		msg("%s: unexpected NULL", __func__);
		return -1;
	}

	dbgmsg("%s: received 0x%x value from client:%s\n", __func__, val,
		bw_clnt->client_name);

	/* notify the waiting threads */
	pthread_mutex_lock(&(bw_clnt->bw_mutex));
	if(bw_clnt->value == val) {
		pthread_mutex_unlock(&(bw_clnt->bw_mutex));
		return 0;
	}
	bw_clnt->value = val;

	if (bw_clnt->sensors[BW_UTIL_SCORE])
		bw_clnt->sensors[BW_UTIL_SCORE]->value = val & 0x7fff;
	if (bw_clnt->sensors[BW_UC_ID])
		bw_clnt->sensors[BW_UC_ID]->value = (val >> 0x10) & 0x7fff;

	bw_clnt->threshold_reached = (1 << BW_UTIL_SCORE) | (1 << BW_UC_ID);
	pthread_cond_broadcast(&(bw_clnt->bw_condition));
	pthread_mutex_unlock(&(bw_clnt->bw_mutex));

	return 0;
}

int bw_sensors_setup(struct sensor_info *sensor)
{
	struct bw_sensor_info *bw_sensor = NULL;
	struct bw_client_data *bw_clnt = NULL;

	if (sensor == NULL || sensor->data == NULL) {
		msg("%s: Invalid sensor pointer\n", __func__);
		return 0;
	}

	bw_clnt = sensor->data;
	bw_sensor = calloc(1, sizeof(struct bw_sensor_info));
	if (bw_sensor == NULL) {
		msg("%s: calloc failed", __func__);
		return 0;
	}

	sensor->data = (void *) bw_sensor;
	bw_sensor->sensor = sensor;
	bw_sensor->clnt = bw_clnt;

	pthread_mutex_lock(&(bw_clnt->bw_mutex));
	if (strstr(sensor->name, BW_UC_ID_APPEND)) {
		bw_sensor->type = BW_UC_ID;
		bw_sensor->value =  (bw_clnt->value >> 0x10) & 0x7fff;
		bw_clnt->sensors[BW_UC_ID] = bw_sensor;
	} else {
		bw_sensor->type = BW_UTIL_SCORE;
		bw_sensor->value =  bw_clnt->value & 0x7fff;
		bw_clnt->sensors[BW_UTIL_SCORE] = bw_sensor;
	}
	pthread_mutex_unlock(&(bw_clnt->bw_mutex));

	return 1;
}

void bw_sensors_shutdown(struct sensor_info *sensor)
{
	struct bw_sensor_info *bw_sensor = NULL;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}
	bw_sensor = (struct bw_sensor_info *) sensor->data;
	sensor->data = NULL;
	free(bw_sensor);
}

int bw_sensor_get_temperature(struct sensor_info *sensor)
{
	int bw_val = 0;
	struct bw_sensor_info *bw_sensor = NULL;

	if (NULL == sensor || NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return 0;
	}
	pthread_mutex_lock(&(sensor->read_mtx));
	bw_sensor = (struct bw_sensor_info *) sensor->data;
	bw_val = bw_sensor->value;
	pthread_mutex_unlock(&(sensor->read_mtx));

	return bw_val;
}

void bw_sensor_interrupt_wait(struct sensor_info *sensor)
{
	struct bw_sensor_info *bw_sensor = NULL;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}
	if (sensor->interrupt_enable) {
		bw_sensor = (struct bw_sensor_info *) sensor->data;
		/* Wait for sensor threshold condition */
		pthread_mutex_lock(&(bw_sensor->clnt->bw_mutex));
		while (!(bw_sensor->clnt->threshold_reached &
				(1 << bw_sensor->type))) {
			pthread_cond_wait(&(bw_sensor->clnt->bw_condition),
				&(bw_sensor->clnt->bw_mutex));
		}
		bw_sensor->clnt->threshold_reached &= ~(1 << bw_sensor->type);
		pthread_mutex_unlock(&(bw_sensor->clnt->bw_mutex));
	}
}

int get_bw_client_sensors_list(struct sensor_info **sensors)
{
	int i, j, count = 0;
	size_t len = 0;
	struct sensor_info *sensors_loc;
	struct bw_client_data *bw_clnt;

	count = ARRAY_SIZE(bw_client_list) * BW_SENSOR_MAX;
	if (count <= 0)
		return -1;

	sensors_loc = calloc((unsigned int)count, sizeof(struct sensor_info));
	if (sensors_loc == NULL) {
		msg("%s: calloc failed for sensors_loc\n", __func__);
		return -1;
	}

	for (i = 0, j = 0; i < count && j < ARRAY_SIZE(bw_client_list);
		i += BW_SENSOR_MAX, j++) {

		bw_clnt = calloc(1, sizeof(struct bw_client_data));
		if (bw_clnt == NULL) {
			msg("%s: calloc failed for bw_clnt\n", __func__);
			goto err_exit;
		}
		pthread_mutex_init(&(bw_clnt->bw_mutex), NULL);
		pthread_cond_init(&(bw_clnt->bw_condition), NULL);
		strlcpy(bw_clnt->client_name, bw_client_list[j], MAX_TZ_NAME_LEN);
		sensors_loc[i].data = sensors_loc[i + 1].data = bw_clnt;

		len = strlen(bw_client_list[j]) + strlen(BW_SCORE_APPEND) + 1;
		sensors_loc[i].name = calloc(len, sizeof(char));
		if (sensors_loc[i].name == NULL) {
			msg("%s: calloc failed for sensors_loc[i].name\n", __func__);
			goto err_exit;
		}
		snprintf(sensors_loc[i].name, len, "%s%s", bw_client_list[j],
				BW_SCORE_APPEND);

		len = strlen(bw_client_list[j]) + strlen(BW_UC_ID_APPEND) + 1;
		sensors_loc[i + 1].name = calloc(len, sizeof(char));
		if (sensors_loc[i + 1].name == NULL) {
			msg("%s: calloc failed for sensors_loc[i + 1].name\n", __func__);
			goto err_exit;
		}
		snprintf(sensors_loc[i + 1].name, len, "%s%s", bw_client_list[j],
				BW_UC_ID_APPEND);

#ifdef ENABLE_CAMERA_REG_BW_CALLBACK
		thermal_server_register_client_req_handler("camera",
			bw_client_hal_callback, bw_clnt);
#else
		thermal_server_register_client_req_handler(bw_client_list[j],
			bw_client_hal_callback, bw_clnt);
#endif
	}

	for (i = 0; i < count; i++) {
		sensors_loc[i].setup = bw_sensors_setup;
		sensors_loc[i].shutdown = bw_sensors_shutdown;
		sensors_loc[i].get_temperature = bw_sensor_get_temperature;
		sensors_loc[i].interrupt_wait = bw_sensor_interrupt_wait;
		sensors_loc[i].interrupt_enable = 1;
	}
	*sensors = sensors_loc;

	return  count;

err_exit:
	if (sensors_loc) {
		for (i = 0; i < count; i += BW_SENSOR_MAX) {
			if (sensors_loc[i].data)
				free(sensors_loc[i].data);
			if (sensors_loc[i].name)
				free(sensors_loc[i].name);
			if (sensors_loc[i + 1].name)
				free(sensors_loc[i + 1].name);
		}
		free(sensors_loc);
	}

	return -1;
}
