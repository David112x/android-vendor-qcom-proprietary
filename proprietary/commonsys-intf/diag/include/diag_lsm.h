#ifndef DIAG_LSM_H
#define DIAG_LSM_H

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================

                   Diag Mapping Layer DLL declarations

DESCRIPTION
  Function declarations for Diag Service Mapping Layer


Copyright (c)2007-2012, 2014-2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header:

when       who     what, where, why
--------   ---     ----------------------------------------------------------
02/04/08   mad     Added declarations for Diag_LSM_Init and DeInit functions
                   to enable diag clients to call these directly.
                   Moved IDiagPkt handle declaration to an internal header
                   file, Diag_LSMi.h
11/29/07   mad     Created File
===========================================================================*/

#define MSG_MASKS_TYPE		0x00000001
#define LOG_MASKS_TYPE		0x00000002
#define EVENT_MASKS_TYPE	0x00000004
#define PKT_TYPE		0x00000008
#define DEINIT_TYPE		0x00000010
#define USER_SPACE_DATA_TYPE	0x00000020
#define DCI_DATA_TYPE		0x00000040
#define USER_SPACE_RAW_DATA_TYPE	0x00000080
#define DCI_LOG_MASKS_TYPE	0x00000100
#define DCI_EVENT_MASKS_TYPE	0x00000200
#define DCI_PKT_TYPE		0x00000400
#define HDLC_SUPPORT_TYPE	0x00001000

#define USB_MODE		1
#define MEMORY_DEVICE_MODE	2
#define NO_LOGGING_MODE		3
#define UART_MODE		4
#define SOCKET_MODE		5
#define CALLBACK_MODE		6
#define PCIE_MODE		7
#define MAX_NUM_FILES_ON_DEVICE 2000 /* If user wants to stop logging on SD after reaching a max file limit */
#define CONTROL_CHAR 0x7E
#define FILE_NAME_LEN 500
#define MASK_FILE_BUF_SIZE	8192
#define NUM_PROC 10
/* Token to identify MDM log */
#define MDM_TOKEN      -1
/* Token to identify QSC log */
#define QSC_TOKEN      -5
#define MSM	0
#define MDM	1
#define MDM_2	2
/* For backward compatibility*/
#define QSC 2
#define MODE_NONREALTIME	0
#define MODE_REALTIME		1
#define MODE_UNKNOWN		2

#define MAX_SOCKET_CHANNELS 3

#define DIAG_PROC_DCI		1
#define DIAG_PROC_MEMORY_DEVICE	2

#define DIAG_CMD_REQ_BUF_SIZE	50
#define DIAG_CMD_RSP_BUF_SIZE	500

/* List of processors */
#define DIAG_ALL_PROC		-1
#define DIAG_MODEM_PROC		0
#define DIAG_LPASS_PROC		1
#define DIAG_WCNSS_PROC		2
#define DIAG_SENSORS_PROC	3
#define DIAG_WDSP_PROC		4
#define DIAG_CDSP_PROC		5
#define DIAG_NPU_PROC		6
#define NUM_PERIPHERALS		7
#define DIAG_APPS_PROC		(NUM_PERIPHERALS)

#define UPD_WLAN		8
#define UPD_AUDIO		9
#define UPD_SENSORS		10
#define UPD_CHARGER		11
#define NUM_UPD			4

#define TOTAL_PD_COUNT	(NUM_PERIPHERALS + NUM_UPD + 1)

#define BUF_DIAG_MODEM_PROC		0
#define BUF_DIAG_LPASS_PROC		1
#define BUF_DIAG_WCNSS_PROC		2
#define BUF_DIAG_SENSORS_PROC	3
#define BUF_DIAG_WDSP_PROC		4
#define BUF_DIAG_CDSP_PROC		5
#define BUF_DIAG_APPS_PROC		6
#define BUF_UPD_WLAN		7
#define BUF_UPD_AUDIO		8
#define BUF_UPD_SENSORS		9
#define BUF_DIAG_NPU_PROC	10
#define BUF_UPD_CHARGER		11


#define DIAG_CON_APSS		(0x0001)	/* Bit mask for APSS */
#define DIAG_CON_MPSS		(0x0002)	/* Bit mask for MPSS */
#define DIAG_CON_LPASS		(0x0004)	/* Bit mask for LPASS */
#define DIAG_CON_WCNSS		(0x0008)	/* Bit mask for WCNSS */
#define DIAG_CON_SENSORS	(0x0010)	/* Bit mask for Sensors */
#define DIAG_CON_WDSP 		(0x0020) 	/* Bit mask for WDSP */
#define DIAG_CON_CDSP 		(0x0040)	/* Bit mask for CDSP */
#define DIAG_CON_NPU		(0x0080)

#define DIAG_CON_UPD_WLAN		(0x1000)	/* Bit mask for WLAN USERPD */
#define DIAG_CON_UPD_AUDIO		(0x2000)	/* Bit mask for AUDIO USERPD */
#define DIAG_CON_UPD_SENSORS	(0x4000)	/* Bit mask for SENSORS USERPD */
#define DIAG_CON_UPD_CHARGER	(0x8000)	/* Bit mask for CHARGER USERPD */

#define DIAG_CON_NONE		(0x0000)	/* Bit mask for No SS*/
#define DIAG_CON_ALL		(DIAG_CON_APSS | DIAG_CON_MPSS \
				| DIAG_CON_LPASS | DIAG_CON_WCNSS \
				| DIAG_CON_SENSORS | DIAG_CON_WDSP \
				| DIAG_CON_CDSP | DIAG_CON_NPU)

#define DIAG_CON_UPD_ALL	(DIAG_CON_UPD_WLAN \
				| DIAG_CON_UPD_AUDIO \
				| DIAG_CON_UPD_SENSORS \
				| DIAG_CON_UPD_CHARGER)

#define DIAG_MSM_MASK (0x0001)   /* Bit mask for APSS */
#define DIAG_MDM_MASK (0x0002)   /* Bit mask for first mdm device */
#define DIAG_MDM2_MASK (0x0004) /* Bit mask for second mdm device */

#define DIAG_STREAMING_MODE		0
#define DIAG_THRESHOLD_BUFFERING_MODE	1
#define DIAG_CIRCULAR_BUFFERING_MODE	2

#define DIAG_MD_NONE			0
#define DIAG_MD_PERIPHERAL		1

/* Standard Watermark Values*/
#define DIAG_HI_WM_VAL			85
#define DIAG_LO_WM_VAL			15

#define ESC_CHAR 0x7d
#define CTRL_CHAR 0x7e

#define DIAG_GET_DIAG_ID	0x222
#define DIAG_HW_ACCEL_CMD	0x224

#define MAX_DIAGID_STR_LEN	30
#define MIN_DIAGID_STR_LEN	5

/*
 * HW Acceleration operation definition
 */
#define DIAG_HW_ACCEL_OP_DISABLE	0
#define DIAG_HW_ACCEL_OP_ENABLE	1
#define DIAG_HW_ACCEL_OP_QUERY	2

/*
 * HW Acceleration TYPE definition
 */
#define DIAG_HW_ACCEL_TYPE_ALL	0
#define DIAG_HW_ACCEL_TYPE_STM	1
#define DIAG_HW_ACCEL_TYPE_ATB	2
#define DIAG_HW_ACCEL_TYPE_MAX	2

#define DIAG_HW_ACCEL_VER_MIN 1
#define DIAG_HW_ACCEL_VER_MAX 1

/*
 * HW Acceleration CMD Error codes
 */
#define DIAG_HW_ACCEL_STATUS_SUCCESS	0
#define DIAG_HW_ACCEL_FAIL	1
#define DIAG_HW_ACCEL_INVALID_TYPE	2
#define DIAG_HW_ACCEL_INVALID_VER	3

/*
 * HW Acceleration Transport types
 */
#define DIAG_TRANSPORT_UNKNOWN 0
#define DIAG_TRANSPORT_UART    1
#define DIAG_TRANSPORT_USB     2
#define DIAG_TRANSPORT_PCIE    3

/*
 * The status bit masks when received in a signal handler are to be
 * used in conjunction with the peripheral list bit mask to determine the
 * status for a peripheral. For instance, 0x00010002 would denote an open
 * status on the MPSS
 */
#define DIAG_STATUS_OPEN (0x00010000)	/* Bit mask for DCI channel open status   */
#define DIAG_STATUS_CLOSED (0x00020000)	/* Bit mask for DCI channel closed status */

#define GUID_LEN 16

#if defined (ANDROID) || defined (USE_ANDROID_LOGGING)
	#define LOG_TAG "Diag_Lib"
	#define DIAG_LOGE(...)  { \
		ALOGE(__VA_ARGS__); \
		if (!diag_disable_console) \
			printf(__VA_ARGS__); \
	}
	#define DIAG_LOGD(...)  { \
		ALOGE(__VA_ARGS__); \
		if (!diag_disable_console) \
			printf(__VA_ARGS__); \
	}
	#include <log/log.h>
    #include "common_log.h"
#else
	#define DIAG_LOGE(...) printf (__VA_ARGS__)
	#define DIAG_LOGD(...) printf (__VA_ARGS__)
#endif
#include <pthread.h>
#include <stdio.h>

#ifdef FEATURE_LE_DIAG
int tgkill(int tgid, int tid, int sig);
#endif

#define GUID_LIST_XML_TAG_SIZE 13
#define GUID_LIST_END_XML_TAG_SIZE 20

#ifdef USE_GLIB
#define strlcpy g_strlcpy
#define strlcat g_strlcat
#endif

extern int logging_mode;
extern char mask_file_proc[NUM_PROC][FILE_NAME_LEN];
#define mask_file mask_file_proc[MSM]
#define mask_file_mdm mask_file_proc[MDM]
extern char output_dir[NUM_PROC][FILE_NAME_LEN];
extern int diag_disable_console;
extern char dir_name[FILE_NAME_LEN];
extern char proc_name[NUM_PROC][6];
extern pthread_cond_t qsr4_read_db_cond;
extern uint8 hdlc_disabled;
extern char qsr4_xml_file_name[FILE_NAME_LEN];
extern int fd_qsr4_xml[NUM_PROC];

typedef enum {
	DB_PARSER_STATE_OFF,
	DB_PARSER_STATE_ON,
	DB_PARSER_STATE_LIST,
	DB_PARSER_STATE_OPEN,
	DB_PARSER_STATE_READ,
	DB_PARSER_STATE_CLOSE,
} qsr4_db_file_parser_state;

typedef struct {
	uint8 cmd_code;
	uint8 subsys_id;
	uint16 subsys_cmd_code;
} __attribute__ ((packed)) diag_pkt_header_t;

/*
 * Structure to keep track of diag callback interface clients. Please note that
 * there can be only client communicating with an ASIC at a given time.
 *
 * @inited: flag to indicate if the table entry is initialized
 * @cb_func_ptr: callback function pointer
 * @context_data: user specified data
 *
 */
struct diag_callback_tbl_t {
	int inited;
	int (*cb_func_ptr)(unsigned char *, int len, void *context_data);
	void *context_data;
};

struct diag_uart_tbl_t {
	int proc_type;
	int pid;
	int (*cb_func_ptr)(unsigned char *, int len, void *context_data);
	void *context_data;
};

struct diag_con_all_param_t {
	uint32 diag_con_all;
	uint32 num_peripherals;
	uint32 upd_map_supported;
};

struct diag_query_pid_t {
	uint32 peripheral_mask;
	uint32 pd_mask;
	int pid;
	uint32 device_mask;
};

struct diag_logging_mode_param_t {
	uint32 req_mode;
	uint32 peripheral_mask;
	uint32 pd_mask;
	uint8 mode_param;
	uint8 diag_id;
	uint8 pd_val;
	uint8 reserved;
	int peripheral;
	uint32 device_mask;
};

struct diag_query_diag_id_t {
	uint8_t pd_val;
	uint8_t diag_id;
	uint16_t reserved;
	int peripheral;
	char process_name[30];
};

typedef struct {
   diag_pkt_header_t header;
   uint8 version;
} __attribute__ ((packed)) diag_id_list_req;

typedef struct {
	uint8 diag_id;
	uint8 peripheral;
	char process_name[30];
    void *next;
} __attribute__ ((packed)) diag_id_list;

typedef struct {
	uint8 diag_id;
	uint8 len;
	char *process_name;
} __attribute__ ((packed)) diag_id_entry_struct;

typedef struct {
	diag_pkt_header_t header;
	uint8 version;
	uint8 num_entries;
	diag_id_entry_struct entry;
} __attribute__ ((packed)) diag_id_list_rsp;

/*
 * hw acceleration command request payload structure
 */
typedef struct {
	uint8 hw_accel_type;
	uint8 hw_accel_ver;
	uint32 diagid_mask;
} __attribute__ ((packed)) diag_hw_accel_op_t;

/*
 * hw acceleration command request structure
 */

typedef struct {
	diag_pkt_header_t header;
	uint8 version;
	uint8 operation;
	uint16 reserved;
	diag_hw_accel_op_t op_req;
} __attribute__ ((packed)) diag_hw_accel_cmd_req_t;

/*
 * hw acceleration command response payload structure
 */
typedef struct {
	uint8 status;
	uint8 hw_accel_type;
	uint8 hw_accel_ver;
	uint32 diagid_status;
} __attribute__ ((packed)) diag_hw_accel_op_resp_payload_t;

/*
 * hw acceleration command op response structure
 */

typedef struct {
	diag_pkt_header_t header;
	uint8_t version;
	uint8_t operation;
	uint16_t reserved;
	diag_hw_accel_op_resp_payload_t op_rsp;
} __attribute__ ((packed)) diag_hw_accel_cmd_op_resp_t;

/*
 * hw acceleration query response sub payload
 * in mulitples of the num_accel_rsp
 */

typedef struct {
	uint8 hw_accel_type;
	uint8 hw_accel_ver;
	uint32 diagid_mask_supported;
	uint32 diagid_mask_enabled;
} __attribute__ ((packed)) diag_hw_accel_query_sub_payload_rsp_t;

/*
 * hw acceleration query operation response payload structure
 */
typedef struct {
	uint8 status;
	uint8 diag_transport;
	uint8 num_accel_rsp;
	diag_hw_accel_query_sub_payload_rsp_t
		sub_query_rsp[DIAG_HW_ACCEL_TYPE_MAX][DIAG_HW_ACCEL_VER_MAX];
} __attribute__ ((packed)) diag_hw_accel_query_rsp_payload_t;

/*
 * hw acceleration command query response structure
 */
typedef struct {
	diag_pkt_header_t header;
	uint8 version;
	uint8 operation;
	uint16 reserved;
	diag_hw_accel_query_rsp_payload_t query_rsp;
} __attribute__ ((packed)) diag_hw_accel_cmd_query_resp_t;

/*===========================================================================
FUNCTION   hw_accel_operation_handler

DESCRIPTION
 Command request handler for hardware acceleration query/enable/disable operations

DEPENDENCIES
Command request structure properly filled.

RETURN VALUE
  -1 = failure, else 0

SIDE EFFECTS
  None

===========================================================================*/
int hw_accel_operation_handler(diag_hw_accel_cmd_req_t *pReq,
	diag_hw_accel_query_sub_payload_rsp_t *query_rsp, uint8 operation);

/*===========================================================================
FUNCTION   Diag_LSM_Init

DESCRIPTION
  Initializes the Diag Legacy Mapping Layer. This should be called
  only once per process.

DEPENDENCIES
  Successful initialization requires Diag CS component files to be present
  and accessible in the file system.

RETURN VALUE
  FALSE = failure, else TRUE

SIDE EFFECTS
  None

===========================================================================*/
boolean Diag_LSM_Init (uint8_t* pIEnv);

/*===========================================================================
FUNCTION   diag_switch_logging_proc

DESCRIPTION
  This swtiches the logging mode from default USB to memory device logging

DEPENDENCIES
  valid data type to be passed in:

RETURN VALUE
  0 - Success; failure otherwise

SIDE EFFECTS
  None

===========================================================================*/
int diag_switch_logging_proc(struct diag_logging_mode_param_t *params);

/*===========================================================================
FUNCTION   diag_switch_logging

DESCRIPTION
  This swtiches the logging mode from default USB to memory device logging

DEPENDENCIES
  valid data type to be passed in:
  In case of ODL second argument is to specifying directory location.
  In case of UART mode second argument is specify PROC type.

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void diag_switch_logging(int requested_mode, char *dir_location);
/*===========================================================================
FUNCTION   diag_read_mask_file

DESCRIPTION
  This reads the mask file

DEPENDENCIES
  valid data type to be passed in

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/

int diag_read_mask_file_proc(int proc_type);
int diag_read_mask_file(void);
/*===========================================================================
FUNCTION   diag_register_callback

DESCRIPTION
  This allows diag client to register a callback function with LSM library.
  If the library receives data from kernel space, it will invoke this call
  back function, thus passing the data to the client through this function.

DEPENDENCIES
  valid data type to be passed in

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void diag_register_callback(int (*client_cb_func_ptr)(unsigned char *ptr,
				int len, void *context_data), void *context_data);

/*===========================================================================
FUNCTION   diag_register_remote_callback

DESCRIPTION
  This allows diag client to register a callback function with LSM library.
  If the library receives data from kernel space originating from the remote
  processor, it will invoke this call back function, thus passing the data
  to the client through this function.

DEPENDENCIES
  valid data type to be passed in

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void diag_register_remote_callback(int (*client_rmt_cb_func_ptr)(unsigned char *ptr,
					int len, void *context_data), int proc,
					void *context_data);

/*===========================================================================

FUNCTION    diag_send_data

DESCRIPTION
  Inject data into diag kernel driver

DEPENDENCIES
  None.

RETURN VALUE
  FALSE = failure, else TRUE.

SIDE EFFECTS
  None

===========================================================================*/
int diag_send_data(unsigned char *, int);

/*===========================================================================

FUNCTION    diag_callback_send_data

DESCRIPTION
  Inject data into diag kernel driver for a specific processor in
  callback mode

DEPENDENCIES
  None.

RETURN VALUE
  FALSE = failure, else TRUE.

SIDE EFFECTS
  None

===========================================================================*/
int diag_callback_send_data(int proc, unsigned char * buf, int len);

/*===========================================================================

FUNCTION    diag_callback_send_data_hdlc

DESCRIPTION
  Inject hdlc data into diag kernel driver for a specific processor in
  callback mode

DEPENDENCIES
  None.

RETURN VALUE
  FALSE = failure, else TRUE.

SIDE EFFECTS
  None

===========================================================================*/
int diag_callback_send_data_hdlc(int proc, unsigned char * buf, int len);

/*===========================================================================

FUNCTION    diag_vote_md_real_time

DESCRIPTION
  Votes the on device logging process for real/non-real time
  mode

DEPENDENCIES
  None.

RETURN VALUE
  0 = success, -1 = failure

SIDE EFFECTS
  Puts the entire diag in the mode specified if the process wins
  the vote

===========================================================================*/
int diag_vote_md_real_time(int real_time);

/*===========================================================================

FUNCTION    diag_vote_md_real_time_proc

DESCRIPTION
  Votes the on device logging process for real/non-real time
  mode, in a particular processor.

DEPENDENCIES
  None.

RETURN VALUE
  0 = success, -1 = failure

SIDE EFFECTS
  Puts the entire diag in the mode specified if the process wins
  the vote

===========================================================================*/
int diag_vote_md_real_time_proc(int proc, int real_time);

/*===========================================================================

FUNCTION    diag_get_real_time_status

DESCRIPTION
  Gets the mode (real time or non real time) in which Diag is in

DEPENDENCIES
  None.

RETURN VALUE
  0 = success, -1 = failure

SIDE EFFECTS
  None

===========================================================================*/
int diag_get_real_time_status(int *real_time);

/*===========================================================================

FUNCTION    diag_get_real_time_status_proc

DESCRIPTION
  Gets the mode (real time or non real time) in which Diag is
  in, in a particular processor

DEPENDENCIES
  None.

RETURN VALUE
  0 = success, -1 = failure

SIDE EFFECTS
  None

===========================================================================*/
int diag_get_real_time_status_proc(int proc, int *real_time);

/*===========================================================================

FUNCTION    Diag_LSM_DeInit

DESCRIPTION
  De-Initialize the Diag service.

DEPENDENCIES
  None.

RETURN VALUE
  FALSE = failure, else TRUE.
  Currently all the internal boolean return functions called by
  this function just returns TRUE w/o doing anything.

SIDE EFFECTS
  None

===========================================================================*/
boolean Diag_LSM_DeInit(void);

/*===========================================================================

FUNCTION    diag_configure_peripheral_buffering_tx_mode

DESCRIPTION
  Configure the  peripheral Diag's TX mode to Streaming, Circular, or
  Threshold buffering mode  and set high and low watermark threshold limits.
  Streaming Mode is a default TX mode for peripheral Diag.
  Switching to Threshold or Circular buffering mode puts the  peripheral
  Diag to Non-Real Time mode (NRT).
  Switching to streaming mode will put the peripheral to Real Time (RT) mode.

DEPENDENCIES
  None.

RETURN VALUE
  1 = success, else failure

SIDE EFFECTS
  Clients cannot vote for real/non-real time when the Tx mode is set
  to Circular, or Threshold buffering mode.

===========================================================================*/
int diag_configure_peripheral_buffering_tx_mode(uint8 peripheral, uint8 tx_mode,
						uint8 low_wm_val, uint8 high_wm_val);

/*===========================================================================

FUNCTION    diag_peripheral_buffering_drain_immediate

DESCRIPTION

Request  the peripheral to drain its Tx buffering pool immediately.
If peripheral Diag receives this request in
Streaming mode - No action is taken since Diag is already streaming.
Threshold or Circular buffering modes - Diag will drain its Tx buffering
pool until the low watermark threshold is reached, and then resume
buffering in the tx mode it was set


DEPENDENCIES
  None.

RETURN VALUE
  1 = success, else failure

SIDE EFFECTS
  None

===========================================================================*/

int diag_peripheral_buffering_drain_immediate(uint8 peripheral);

void diag_get_diag_id_info(void);
void diag_qshrink4_init(void);
diag_id_list *get_diag_id(int peripheral_type, int peripheral);
void diag_wait_for_diag_id_info(void);

#ifdef __cplusplus
}
#endif

#endif /* DIAG_LSM_H */

