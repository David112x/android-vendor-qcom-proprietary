/*===========================================================================

  Copyright (c) 2013, 2015, 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#ifndef __THERMAL_LIB_COMMON_H__
#define __THERMAL_LIB_COMMON_H__

#define THERMAL_SEND_SOCKET_NAME            "thermal-send-client"
#define THERMAL_RECV_SOCKET_NAME            "thermal-recv-client"
#define THERMAL_RECV_PASSIVE_SOCKET_NAME    "thermal-recv-passive-client"
#define THERMAL_SEND_RULE_SOCKET_NAME       "thermal-send-rule"
#define THERMAL_SEND_CLIENT_SOCKET          "/dev/socket/"THERMAL_SEND_SOCKET_NAME
#define THERMAL_RECV_CLIENT_SOCKET          "/dev/socket/"THERMAL_RECV_SOCKET_NAME
#define THERMAL_RECV_PASSIVE_CLIENT_SOCKET  "/dev/socket/"THERMAL_RECV_PASSIVE_SOCKET_NAME
#define THERMAL_SEND_RULE_SOCKET            "/dev/socket/"THERMAL_SEND_RULE_SOCKET_NAME
#define CONFIG_QUERY_CLIENT   "config_query"
#define CONFIG_SET_CLIENT     "config_set"
#define NUM_LISTEN_QUEUE   (20U)
#define LEVEL_MAX          (3U)
#define MAX_CAMERA_MITIGATION_LEVEL  (10U)
#define MAX_CAMCORDER_MITIGATION_LEVEL  (10U)
#define CLIENT_HANDLE_MAX  (32U)
#define CLIENT_NAME_MAX    (20U)
#define MAX_CONFIG_INSTANCES_SUPPORTED (128U)
#define REPORT_MSG_MAX       (124)

/* Below macros are defined in
   inc/thermal_config.h */
#define MAX_ALGO_NAME   (10U)
#define MAX_FIELD_NAME  (20U)

#ifndef MAX_ALGO_DESC
#define MAX_ALGO_DESC  (32U)
#endif
#ifndef DEVICES_MAX_NAME_LEN
#define DEVICES_MAX_NAME_LEN (32U)
#endif
#ifndef ACTIONS_MAX
#define ACTIONS_MAX (32U)
#endif
#ifndef THRESHOLDS_MAX
#define THRESHOLDS_MAX (12U)
#endif

/* Below macro is defined in
   client/thermal_client.c and enum
   defined in inc/thermal_client.h and
   libThermal_headers */
#ifndef SUPPORTED_FIELDS_ENABLED
#define SUPPORTED_FIELDS_ENABLED  1
enum supported_fields {
	UNKNOWN_FIELD = 0x0,
	DISABLE_FIELD = 0x1,
	SAMPLING_FIELD = 0x2,
	POLLING_DELAY_FIELD = SAMPLING_FIELD,
	THRESHOLDS_FIELD = 0x4,
	SET_POINT_FIELD = THRESHOLDS_FIELD,
	THRESHOLDS_CLR_FIELD = 0x8,
	SET_POINT_CLR_FIELD = THRESHOLDS_CLR_FIELD,
	ACTION_INFO_FIELD = 0x10,
	UPPER_LIMIT_FIELD = ACTION_INFO_FIELD,
	LOWER_LIMIT_FIELD = 0x20,
	PASSIVE_DELAY_FIELD = 0x40,
	SUPPORTED_FIELD_MAX = 0x80,
};
#endif

#define BW_LM_INPUT_VALIDATE(req_val, ret)                                \
	do {                                                              \
		int bw_score, uc_id;                                      \
		if (req_val < 0) {                                        \
			msg("%s: Invalid input data: < 0\n", __func__);   \
			ret = -EINVAL;                                    \
			break;                                            \
		}                                                         \
		bw_score = req_val & 0xffff;                              \
		uc_id = (req_val >> 0x10) & 0xffff;                       \
		if (bw_score > 0x7fff || uc_id > 0x7fff) {                \
			msg("%s: Invalid input data: > lim \n", __func__);\
			ret = -EINVAL;                                    \
			break;                                            \
		}                                                         \
		ret = 0;                                                  \
	} while (0)

/* Thermal client data */
struct thermal_cdata {
	uint32_t client_cb_handle;
	char *client_name;
	void *callback;
	void *user_data;
	void *data_reserved;
	struct thermal_cdata *next;
};

struct override_action_t {
        char device[DEVICES_MAX_NAME_LEN];
        int  info;
};

struct override_threshold_t {
	int threshold_trig;
	int threshold_clr;
	struct override_action_t actions[ACTIONS_MAX];
	uint32_t num_actions;
};

struct monitor_settings {
	uint32_t num_thresholds;
	struct  override_threshold_t t[THRESHOLDS_MAX];
};

struct dynamic_settings {
	int set_point;
	int set_point_clr;
	char device[DEVICES_MAX_NAME_LEN];
};

struct step_device_info{
	char name[DEVICES_MAX_NAME_LEN];
	int upper_limit;
	int lower_limit;
};

struct trip_pt {
	int set_point;
	int set_point_clr;
	struct step_device_info devs[ACTIONS_MAX];
	uint32_t device_num;
};

struct step_settings {
	uint32_t trip_pt_num;
	struct trip_pt trips[THRESHOLDS_MAX];
	uint32_t polling_delay;
	uint32_t passive_delay;
};

struct config_data {
	char config_desc[MAX_ALGO_DESC];
	unsigned int fields_mask;
	uint8_t disable;
	uint32_t sampling;
	union {
		struct monitor_settings m_setting;
		struct dynamic_settings d_setting;
		struct step_settings s_setting;
	};
};

/* Thermal socket message data type */
struct thermal_msg_data {
	int msg_type;
	char client_name[CLIENT_NAME_MAX];
	union {
		int req_data;
		struct config_data config;
	};
};

unsigned int add_to_list(char *name,  void *callback, void *data);
struct thermal_cdata *get_callback_node_from_list(struct thermal_cdata *list, char *name);
int remove_from_list(unsigned int handle);
int send_to_fd(int fd, const unsigned char *buf, size_t count);
int recv_from_fd(int fd, unsigned char *buf, size_t count);

#endif  /* __THERMAL_LIB_COMMON_H__ */
