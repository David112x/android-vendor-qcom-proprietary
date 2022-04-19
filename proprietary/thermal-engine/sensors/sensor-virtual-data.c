/*===========================================================================
Copyright (c) 2015,2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include "thermal.h"
#include "thermal_config.h"

static char *vs_list_8996[] = {
	"tsens_tz_sensor4",
	"tsens_tz_sensor6",
	"tsens_tz_sensor9",
	"tsens_tz_sensor11"
};

static char *vs_trip_list_8996[] = {
	"tsens_tz_sensor11"
};

static struct setting_info vs_cfgs_8996[] =
{
	{
		.desc = "VIRTUAL-CPUS",
		.algo_type = VIRTUAL_SENSOR_TYPE,
		.dynamic = 0,
		.data.v = {
			.trip_sensor_list = vs_trip_list_8996,
			.trip_cnt = ARRAY_SIZE(vs_trip_list_8996),
			.sensor_list = vs_list_8996,
			.list_cnt = ARRAY_SIZE(vs_list_8996),
			.list_size = 1,
			.t[0] = {
				.lvl_trig = 75000,
				.lvl_clr =  65000,
			},
			.sampling_period_ms = 10,
			.math_type = MATH_MAX,
		},
	},
};

static struct setting_info vs_cfgs_8996au[] =
{
	{
		.desc = "VIRTUAL-CPUS",
		.algo_type = VIRTUAL_SENSOR_TYPE,
		.dynamic = 0,
		.data.v = {
			.trip_sensor_list = vs_trip_list_8996,
			.trip_cnt = ARRAY_SIZE(vs_trip_list_8996),
			.sensor_list = vs_list_8996,
			.list_cnt = ARRAY_SIZE(vs_list_8996),
			.list_size = 1,
			.t[0] = {
				.lvl_trig = 85000,
				.lvl_clr =  75000,
			},
			.sampling_period_ms = 10,
			.math_type = MATH_MAX,
		},
	},
};

static char *vs_list_cobalt_cluster0[] = {
	"tsens_tz_sensor1",
	"tsens_tz_sensor2",
	"tsens_tz_sensor3",
	"tsens_tz_sensor4"
};

static char *vs_list_cobalt_cluster1[] = {
	"tsens_tz_sensor7",
	"tsens_tz_sensor8",
	"tsens_tz_sensor9",
	"tsens_tz_sensor10"
};

static char *vs_list_cobalt_gpu_skin[] = {
	"tsens_tz_sensor13",
	"quiet_therm"
};

static int vs_list_cobalt_weight[] = {
	50,
	50,
};

static char *vs_trip_list_cobalt[] = {
	"tsens_tz_sensor1"
};

static char *vs_trip_list_cobalt1[] = {
	"tsens_tz_sensor7"
};

static char *vs_trip_list_cobalt_gpu[] = {
	"tsens_tz_sensor13"
};

static struct setting_info vs_cfgs_cobalt[] =
{
	{
		.desc = "VIRTUAL-CLUSTER0",
		.algo_type = VIRTUAL_SENSOR_TYPE,
		.dynamic = 0,
		.data.v = {
			.trip_sensor_list = vs_trip_list_cobalt,
			.trip_cnt = ARRAY_SIZE(vs_trip_list_cobalt),
			.sensor_list = vs_list_cobalt_cluster0,
			.list_cnt = ARRAY_SIZE(vs_list_cobalt_cluster0),
			.list_size = 1,
			.t[0] = {
				.lvl_trig = 60000,
				.lvl_clr =  50000,
			},
			.sampling_period_ms = 10,
			.math_type = MATH_MAX,
		},
	},
	{
		.desc = "VIRTUAL-CLUSTER1",
		.algo_type = VIRTUAL_SENSOR_TYPE,
		.dynamic = 0,
		.data.v = {
			.trip_sensor_list = vs_trip_list_cobalt1,
			.trip_cnt = ARRAY_SIZE(vs_trip_list_cobalt1),
			.sensor_list = vs_list_cobalt_cluster1,
			.list_cnt = ARRAY_SIZE(vs_list_cobalt_cluster1),
			.list_size = 1,
			.t[0] = {
				.lvl_trig = 60000,
				.lvl_clr =  50000,
			},
			.sampling_period_ms = 10,
			.math_type = MATH_MAX,
		},
	},
};

static struct setting_info vs_cfgs_cobalt_qrd[] =
{
	{
		.desc = "VIRTUAL-CLUSTER0",
		.algo_type = VIRTUAL_SENSOR_TYPE,
		.dynamic = 0,
		.data.v = {
			.trip_sensor_list = vs_trip_list_cobalt,
			.trip_cnt = ARRAY_SIZE(vs_trip_list_cobalt),
			.sensor_list = vs_list_cobalt_cluster0,
			.list_cnt = ARRAY_SIZE(vs_list_cobalt_cluster0),
			.list_size = 1,
			.t[0] = {
				.lvl_trig = 60000,
				.lvl_clr =  50000,
			},
			.sampling_period_ms = 10,
			.math_type = MATH_MAX,
		},
	},
	{
		.desc = "VIRTUAL-CLUSTER1",
		.algo_type = VIRTUAL_SENSOR_TYPE,
		.dynamic = 0,
		.data.v = {
			.trip_sensor_list = vs_trip_list_cobalt1,
			.trip_cnt = ARRAY_SIZE(vs_trip_list_cobalt1),
			.sensor_list = vs_list_cobalt_cluster1,
			.list_cnt = ARRAY_SIZE(vs_list_cobalt_cluster1),
			.list_size = 1,
			.t[0] = {
				.lvl_trig = 60000,
				.lvl_clr =  50000,
			},
			.sampling_period_ms = 10,
			.math_type = MATH_MAX,
		},
	},
	{
		.desc = "VIRTUAL-GPU",
		.algo_type = VIRTUAL_SENSOR_TYPE,
		.dynamic = 0,
		.data.v = {
			.trip_sensor_list = vs_trip_list_cobalt_gpu,
			.trip_cnt = ARRAY_SIZE(vs_trip_list_cobalt_gpu),
			.sensor_list = vs_list_cobalt_gpu_skin,
			.list_cnt = ARRAY_SIZE(vs_list_cobalt_gpu_skin),
			.list_size = 1,
			.t[0] = {
				.lvl_trig = 56000,
				.lvl_clr =  52000,
			},
			.sampling_period_ms = 100,
			.weight_list = vs_list_cobalt_weight,
			.weight_cnt = ARRAY_SIZE(vs_list_cobalt_weight),
			.weight_size = 1,
			.math_type = MATH_WEIGHT,
		},
	},
};
#ifndef USE_THERMAL_FRAMEWORK
static char *vs_list_sdm660_cluster1[] = {
	"tsens_tz_sensor3",
	"tsens_tz_sensor4",
	"tsens_tz_sensor5",
	"tsens_tz_sensor6",
	"tsens_tz_sensor7",
	"tsens_tz_sensor12"
};

static char *vs_list_sdm630_cluster1[] = {
	"tsens_tz_sensor1",
	"tsens_tz_sensor2",
	"tsens_tz_sensor3",
	"tsens_tz_sensor4",
	"tsens_tz_sensor5",
	"tsens_tz_sensor6",
};

static char *vs_list_sdm660_cluster0[] = {
	"tsens_tz_sensor1",
	"tsens_tz_sensor2"
};

static char *vs_trip_list_sdm660_c1[] = {
	"tsens_tz_sensor3"
};

static char *vs_trip_list_sdm660_c0[] = {
	"tsens_tz_sensor1"
};

static struct setting_info vs_cfgs_sdm630[] =
{
	{
		.desc = "VIRTUAL-CLUSTER1",
		.algo_type = VIRTUAL_SENSOR_TYPE,
		.dynamic = 0,
		.data.v = {
			.trip_sensor_list = vs_trip_list_sdm660_c1,
			.trip_cnt = ARRAY_SIZE(vs_trip_list_sdm660_c1),
			.sensor_list = vs_list_sdm630_cluster1,
			.list_cnt = ARRAY_SIZE(vs_list_sdm630_cluster1),
			.list_size = 1,
			.t[0] = {
				.lvl_trig = 60000,
				.lvl_clr =  50000,
			},
			.sampling_period_ms = 10,
			.math_type = MATH_MAX,
		},
	},
};

static struct setting_info vs_cfgs_sdm660[] =
{
	{
		.desc = "VIRTUAL-CLUSTER0",
		.algo_type = VIRTUAL_SENSOR_TYPE,
		.dynamic = 0,
		.data.v = {
			.trip_sensor_list = vs_trip_list_sdm660_c0,
			.trip_cnt = ARRAY_SIZE(vs_trip_list_sdm660_c0),
			.sensor_list = vs_list_sdm660_cluster0,
			.list_cnt = ARRAY_SIZE(vs_list_sdm660_cluster0),
			.list_size = 1,
			.t[0] = {
				.lvl_trig = 60000,
				.lvl_clr =  50000,
			},
			.sampling_period_ms = 10,
			.math_type = MATH_MAX,
		},
	},
	{
		.desc = "VIRTUAL-CLUSTER1",
		.algo_type = VIRTUAL_SENSOR_TYPE,
		.dynamic = 0,
		.data.v = {
			.trip_sensor_list = vs_trip_list_sdm660_c1,
			.trip_cnt = ARRAY_SIZE(vs_trip_list_sdm660_c1),
			.sensor_list = vs_list_sdm660_cluster1,
			.list_cnt = ARRAY_SIZE(vs_list_sdm660_cluster1),
			.list_size = 1,
			.t[0] = {
				.lvl_trig = 60000,
				.lvl_clr =  50000,
			},
			.sampling_period_ms = 10,
			.math_type = MATH_MAX,
		},
	},
};
#endif

static char *vs_list_8937[] = {
	"tsens_tz_sensor5",
	"tsens_tz_sensor6",
	"tsens_tz_sensor7",
	"tsens_tz_sensor8",
	"tsens_tz_sensor9"
};

static char *vs_trip_list_8937[] = {
	"tsens_tz_sensor5"
};

static struct setting_info vs_cfgs_8937[] =
{
	{
		.desc = "VIRTUAL-CPUS",
		.algo_type = VIRTUAL_SENSOR_TYPE,
		.dynamic = 0,
		.data.v = {
			.trip_sensor_list = vs_trip_list_8937,
			.trip_cnt = ARRAY_SIZE(vs_trip_list_8937),
			.sensor_list = vs_list_8937,
			.list_cnt = ARRAY_SIZE(vs_list_8937),
			.list_size = 1,
			.t[0] = {
				.lvl_trig = 75000,
				.lvl_clr =  65000,
			},
			.sampling_period_ms = 50,
			.math_type = MATH_MAX,
		},
	},
};

#ifndef USE_THERMAL_FRAMEWORK
static char *vs_list_8953[] = {
	"tsens_tz_sensor9",
	"tsens_tz_sensor10",
	"tsens_tz_sensor11",
	"tsens_tz_sensor12",
	"tsens_tz_sensor13",
	"tsens_tz_sensor4",
	"tsens_tz_sensor5",
	"tsens_tz_sensor6",
	"tsens_tz_sensor7",
	"tsens_tz_sensor8"
};

static char *vs_trip_list_8953[] = {
	"tsens_tz_sensor9"
};

static struct setting_info vs_cfgs_8953[] =
{
	{
		.desc = "VIRTUAL-CPUS",
		.algo_type = VIRTUAL_SENSOR_TYPE,
		.dynamic = 0,
		.data.v = {
			.trip_sensor_list = vs_trip_list_8953,
			.trip_cnt = ARRAY_SIZE(vs_trip_list_8953),
			.sensor_list = vs_list_8953,
			.list_cnt = ARRAY_SIZE(vs_list_8953),
			.list_size = 1,
			.t[0] = {
				.lvl_trig = 75000,
				.lvl_clr =  65000,
			},
			.sampling_period_ms = 50,
			.math_type = MATH_MAX,
		},
	},
};
#endif

static char *vs_list_8917[] = {
	"tsens_tz_sensor5",
	"tsens_tz_sensor6",
	"tsens_tz_sensor7",
	"tsens_tz_sensor8"
};

static char *vs_trip_list_8917[] = {
	"tsens_tz_sensor5"
};

static struct setting_info vs_cfgs_8917[] =
{
	{
		.desc = "VIRTUAL-CPUS",
		.algo_type = VIRTUAL_SENSOR_TYPE,
		.dynamic = 0,
		.data.v = {
			.trip_sensor_list = vs_trip_list_8917,
			.trip_cnt = ARRAY_SIZE(vs_trip_list_8917),
			.sensor_list = vs_list_8917,
			.list_cnt = ARRAY_SIZE(vs_list_8917),
			.list_size = 1,
			.t[0] = {
				.lvl_trig = 75000,
				.lvl_clr =  65000,
			},
			.sampling_period_ms = 50,
			.math_type = MATH_MAX,
		},
	},
};

void virtual_sensors_init_data(struct thermal_setting_t *setting)
{
	int i = 0, arr_size = 0;
	struct setting_info *cfg = NULL;

	switch (therm_get_msm_id()) {
		case THERM_MSM_8996:
			cfg = vs_cfgs_8996;
			arr_size = ARRAY_SIZE(vs_cfgs_8996);
			break;
		case THERM_MSM_8996AU:
			cfg = vs_cfgs_8996au;
			arr_size = ARRAY_SIZE(vs_cfgs_8996au);
			break;
		case THERM_MSM_COBALT:
			switch (therm_get_hw_platform()) {
			case THERM_PLATFORM_MTP:
				cfg = vs_cfgs_cobalt;
				arr_size = ARRAY_SIZE(vs_cfgs_cobalt);
				break;
			default:
				cfg = vs_cfgs_cobalt_qrd;
				arr_size = ARRAY_SIZE(vs_cfgs_cobalt_qrd);
				break;
			}
			break;
		case THERM_MSM_8937:
		case THERM_MSM_8940:
			cfg = vs_cfgs_8937;
			arr_size = ARRAY_SIZE(vs_cfgs_8937);
			break;
		case THERM_MSM_8953:
#ifndef USE_THERMAL_FRAMEWORK
			cfg = vs_cfgs_8953;
			arr_size = ARRAY_SIZE(vs_cfgs_8953);
#endif
			break;
		case THERM_MSM_8917:
		case THERM_MSM_8920:
			cfg = vs_cfgs_8917;
			arr_size = ARRAY_SIZE(vs_cfgs_8917);
			break;
		case THERM_SDM_660:
#ifndef USE_THERMAL_FRAMEWORK
			cfg = vs_cfgs_sdm660;
			arr_size = ARRAY_SIZE(vs_cfgs_sdm660);
#endif
			break;
		case THERM_SDM_630:
#ifndef USE_THERMAL_FRAMEWORK
			cfg = vs_cfgs_sdm630;
			arr_size = ARRAY_SIZE(vs_cfgs_sdm630);
#endif
			break;
		default:
			dbgmsg("%s: No virtual sensors added for this platform\n",
				__func__);
			return;
	}

	for (i = 0; i < arr_size; i++)
		add_setting(setting, &cfg[i]);
}

