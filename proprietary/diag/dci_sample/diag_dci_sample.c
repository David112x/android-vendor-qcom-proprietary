
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

		Sample Application for Diag Consumer Interface

GENERAL DESCRIPTION
This is a sample application to demonstrate using Diag Consumer Interface APIs.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS


Copyright (c) 2013-2016, 2018-2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

Author: Ravi Aravamudhan
Source: Jamila Murabbi
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================*/

#include "event.h"
#include "msg.h"
#include "log.h"
#include "diag_lsm.h"
#include "diag_lsmi.h"
#include "diag_lsm_dci.h"
#include "diag_shared_i.h"
#include "stdio.h"
#include "string.h"
#include "malloc.h"
#include "diagpkt.h"
#include "diagdiag.h"	/* For macros used in this sample. */
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/klog.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/time.h>

#define TOTAL_LOG_CODES		1
#define TOTAL_LOG_CODES_APPS	3
#define TOTAL_EVENT_CODES	4
#define DIAG_SAMPLE_SIGNAL	SIGRTMIN + 15

#define DIAG_CMD_CODE_IDX		0
#define DIAG_SUBSYS_ID_IDX		1
#define DIAG_STRESS_TEST_OP_IDX		8

/* Structure for Log packet parsing*/
struct log_code_name_type {
	int log_code;
	char *name;
};

struct event_code_name_type {
	event_id_enum_type event_code;
	char *name;
};

struct dci_channel_info {
	int ch_id;
	void (*dci_log_cb)(unsigned char *buf, int len);
	void (*dci_event_cb)(unsigned char *buf, int len);
	void (*dci_cmd_rsp_cb)(unsigned char *buf, int len);
};
void process_dci_log_stream(unsigned char *ptr, int len);
void process_dci_event_stream(unsigned char *ptr, int len);
void process_response(unsigned char *ptr, int len, void *data_ptr);
void process_mdm_response(unsigned char *ptr, int len, void *data_ptr);
void process_mdm_dci_log_stream(unsigned char *ptr, int len);
void process_mdm_dci_event_stream(unsigned char *ptr, int len);
struct dci_channel_info ch_info[NUM_PROC] = {
	{
		.ch_id = MSM,
		.dci_log_cb = process_dci_log_stream,
		.dci_event_cb = process_dci_event_stream,
		.dci_cmd_rsp_cb = process_response
	},
	{
		.ch_id = MDM,
		.dci_log_cb = process_mdm_dci_log_stream,
		.dci_event_cb = process_mdm_dci_event_stream,
		.dci_cmd_rsp_cb = process_mdm_response
	},

};
/* Channel proc set to MSM by default */
static int channel_mask = 1;
static unsigned int max_channel_mask = 0;
/* Set a sample log codes */
struct log_code_name_type log_codes[TOTAL_LOG_CODES] = {
	{ LOG_DIAG_STRESS_TEST_C, "Diag Log Stress Test" }
};

struct log_code_name_type log_codes_apps[TOTAL_LOG_CODES_APPS] = {
	{ LOG_WMS_SET_ROUTES_C, "WMS Set Routes" },
	{ LOG_DATA_PROTOCOL_LOGGING_C, "DPL Log Messages" },
	{ LOG_WMS_READ_C, "WMS Read" }
};

struct event_code_name_type event_codes[TOTAL_EVENT_CODES] = {
	{ EVENT_DIAG_STRESS_TEST_NO_PAYLOAD, "Diag Event Stress Test" },
	{ EVENT_DIAG_STRESS_TEST_WITH_PAYLOAD, "Diag Event w/ Payload Stress Test"},
	{ EVENT_DIAG_STRESS_TEST_COMPLETED, "Diag Event Stress Test Complete"},
	{ EVENT_DIAG_DROP_DEBUG, "Diag Event Drop Debug"}
};
void dci_execute_func(void * data);
/* Set flag to print the bytes */
static inline void print_bytes(unsigned char *buf, int len, int flag)
{
	int i = 0;
	if (!flag)
		return;
	if (!buf || len <= 0)
		return;
	for (i = 0; i < len; i++) {
		if(i % 8 == 0)
			printf("\n");
		printf("  %02x  ", buf[i]);
	}
	printf("\n");
}

static void usage(char *progname)
{
	printf("\n Usage for %s:\n", progname);
	printf("\n-c  --channel name:\t Designate the channel name for all operations 1 - MSM, 2 - MDM 3 - MSM + MDM\n");
	exit(0);
}

static void parse_args(int argc, char **argv)
{
	int command, proc_mask;
	int file_num = 0;
	struct option longopts[] =
	{
		{ "channelname",	1,	NULL,	'c'},
		{ "help",	0,	NULL,	'h'},
	};

	while ((command = getopt_long(argc, argv, "c:h", longopts, NULL))
			!= -1) {
		switch (command) {
			case 'c':
				proc_mask = atol(optarg);
				if (proc_mask > 0 && proc_mask <= max_channel_mask)
					channel_mask = proc_mask;
				break;
			case 'h':
			default:
				usage(argv[0]);
				break;
		}
	}
}

/* Signal handler that handles the change in DCI channel */
void notify_handler(int signal, siginfo_t *info, void *unused)
{
	(void)unused;

	if (info) {
		int err;
		diag_dci_peripherals list = 0;

		DIAG_LOGE("diag: In %s, signal %d received from kernel, data is: %x\n",
			__func__, signal, info->si_int);

		if (info->si_int & DIAG_STATUS_OPEN) {
			if (info->si_int & DIAG_CON_MPSS) {
				DIAG_LOGE("diag: DIAG_STATUS_OPEN on DIAG_CON_MPSS\n");
			} else if (info->si_int & DIAG_CON_LPASS) {
				DIAG_LOGE("diag: DIAG_STATUS_OPEN on DIAG_CON_LPASS\n");
			} else {
				DIAG_LOGE("diag: DIAG_STATUS_OPEN on unknown peripheral\n");
			}
		} else if (info->si_int & DIAG_STATUS_CLOSED) {
			if (info->si_int & DIAG_CON_MPSS) {
				DIAG_LOGE("diag: DIAG_STATUS_CLOSED on DIAG_CON_MPSS\n");
			} else if (info->si_int & DIAG_CON_LPASS) {
				DIAG_LOGE("diag: DIAG_STATUS_CLOSED on DIAG_CON_LPASS\n");
			} else {
				DIAG_LOGE("diag: DIAG_STATUS_CLOSED on unknown peripheral\n");
			}
		}
		err = diag_get_dci_support_list_proc(MSM, &list);
		if (err != DIAG_DCI_NO_ERROR) {
			DIAG_LOGE("diag: could not get support list, err: %d\n", err);
		}
		/* This will print out all peripherals supporting DCI */
		if (list & DIAG_CON_MPSS)
			DIAG_LOGE("diag: Modem supports DCI\n");
		if (list & DIAG_CON_LPASS)
			DIAG_LOGE("diag: LPASS supports DCI\n");
		if (list & DIAG_CON_WCNSS)
			DIAG_LOGE("diag: RIVA supports DCI\n");
		if (list & DIAG_CON_APSS)
			DIAG_LOGE("diag: APSS supports DCI\n");
		if (!list)
			DIAG_LOGE("diag: No current dci support\n");
	} else {
		DIAG_LOGE("diag: In %s, signal %d received from kernel, but no info value, info: 0x%p\n",
			__func__, signal, info);
	}
}

/* Singal Handler that will be fired when we receive DCI data */
void dci_data_handler(int signal)
{
	(void)signal;

	/* Do something here when you receive DCI data. */

	/* This is usually for holding wakelocks when the
	   clients are running in Diag Non Real Time mode
	   or when they know the Apps processor is in deep
	   sleep but they still need to process DCI data.

	   Please Note: Wakelocks must be released
	   after processing the data in the respective
	   response/log/event handler. Failure to do so
	   will affect the power consumption of the Apps
	   processor. */
}

void process_dci_log_stream(unsigned char *ptr, int len)
{
	int i, found = 0;
	for (i = 0; i < TOTAL_LOG_CODES; i++) {
		if (*(uint16 *)(ptr + 2) == log_codes[i].log_code) {
			DIAG_LOGE("  Received a Log of type %s, length = %d from MSM\n", log_codes[i].name, len);
			found = 1;
			break;
		}
	}
	for (i = 0; i < TOTAL_LOG_CODES_APPS && !found; i++) {
		if (*(uint16 *)(ptr + 2) == log_codes_apps[i].log_code) {
			DIAG_LOGE("  Received a Log of type %s, length = %d from MSM\n", log_codes_apps[i].name, len);
			break;
		}
	}
	if (*(uint8 *)ptr == 0x98) {
		DIAG_LOGE("  Received an EXT API LOG PKT %d bytes from MSM\n", len);
		print_bytes(ptr, len, FALSE);
	}

	print_bytes(ptr, len, FALSE);
}

void process_dci_event_stream(unsigned char *ptr, int len)
{
	if (*(uint8 *)ptr == 0x98) {
		DIAG_LOGE("  Received an EXT API EVENT PKT %d bytes from MSM\n", len);
		print_bytes(ptr, len, FALSE);
		return;
	}
	DIAG_LOGE(" Received an event of size %d bytes from MSM\n", len);
	print_bytes(ptr, len, FALSE);
}

void process_mdm_dci_log_stream(unsigned char *ptr, int len)
{
	int i, found = 0;
	for (i = 0; i < TOTAL_LOG_CODES; i++) {
		if (*(uint16 *)(ptr + 2) == log_codes[i].log_code) {
			DIAG_LOGE("  Received a Log of type %s, length = %d from MDM\n", log_codes[i].name, len);
			found = 1;
			break;
		}
	}
	for (i = 0; i < TOTAL_LOG_CODES_APPS && !found; i++) {
		if (*(uint16 *)(ptr + 2) == log_codes_apps[i].log_code) {
			DIAG_LOGE("  Received a Log of type %s, length = %d from MDM \n", log_codes_apps[i].name, len);
			break;
		}
	}
	if (*(uint8 *)ptr == 0x98) {
		DIAG_LOGE("  Received an EXT API LOG PKT %d bytes from MDM\n", len);
		print_bytes(ptr, len, FALSE);
	}

	print_bytes(ptr, len, FALSE);
}

void process_mdm_dci_event_stream(unsigned char *ptr, int len)
{
	if (*(uint8 *)ptr == 0x98) {
		DIAG_LOGE("  Received an EXT API EVENT PKT %d bytes from MDM\n", len);
		print_bytes(ptr, len, FALSE);
		return;
	}
	DIAG_LOGE(" Received an event of size %d bytes from MDM\n", len);
	print_bytes(ptr, len, FALSE);
}

void process_response(unsigned char *ptr, int len, void *data_ptr)
{
	int i = 0;
	uint8 operation = 0;
	(void)data_ptr;

	if (!ptr || len < 0)
		return;

	DIAG_LOGE(" Received Response of size %d bytes.\n", len);
	print_bytes(ptr, len, TRUE);

	/* Parsing Logic for the response - Based on the request in this sample */
	if (len <= DIAG_STRESS_TEST_OP_IDX)
		return;

	operation = ptr[DIAG_STRESS_TEST_OP_IDX];
	if (ptr[DIAG_CMD_CODE_IDX] == DIAG_SUBSYS_CMD_F &&
	    ptr[DIAG_SUBSYS_ID_IDX] == DIAG_SUBSYS_DIAG_SERV) {
		DIAG_LOGE("  Command Code:   Diag\n");
		DIAG_LOGE("  Subsystem ID:   Diag\n");
		if (operation <= DIAGDIAG_STRESS_TEST_ERR_FATAL) {
			DIAG_LOGE("  Test for:       F3s\n");
		} else if (operation == DIAGDIAG_STRESS_TEST_LOG) {
			DIAG_LOGE("  Test for:       Logs\n");
		} else if (operation == DIAGDIAG_STRESS_TEST_EVENT_NO_PAYLOAD) {
			DIAG_LOGE("  Test for:       Events without payload\n");
		} else if (operation == DIAGDIAG_STRESS_TEST_EVENT_WITH_PAYLOAD) {
			DIAG_LOGE("  Test for:       Events with payload\n");
		}
	}
	printf("\n");
}

void process_mdm_response(unsigned char *ptr, int len, void *data_ptr)
{
        int i = 0;
        uint8 operation = 0;
        (void)data_ptr;

        if (!ptr || len < 0)
                return;

        DIAG_LOGE(" Received Response of size %d bytes from MDM.\n", len);
        print_bytes(ptr, len, TRUE);

        /* Parsing Logic for the response - Based on the request in this sample */
        if (len <= DIAG_STRESS_TEST_OP_IDX)
                return;

        operation = ptr[DIAG_STRESS_TEST_OP_IDX];
        if (ptr[DIAG_CMD_CODE_IDX] == DIAG_SUBSYS_CMD_F &&
            ptr[DIAG_SUBSYS_ID_IDX] == DIAG_SUBSYS_DIAG_SERV) {
                DIAG_LOGE("  Command Code:   Diag\n");
                DIAG_LOGE("  Subsystem ID:   Diag\n");
                if (operation <= DIAGDIAG_STRESS_TEST_ERR_FATAL) {
                        DIAG_LOGE("  Test for:       F3s\n");
                } else if (operation == DIAGDIAG_STRESS_TEST_LOG) {
                        DIAG_LOGE("  Test for:       Logs\n");
                } else if (operation == DIAGDIAG_STRESS_TEST_EVENT_NO_PAYLOAD) {
                        DIAG_LOGE("  Test for:       Events without payload\n");
                } else if (operation == DIAGDIAG_STRESS_TEST_EVENT_WITH_PAYLOAD) {
                        DIAG_LOGE("  Test for:       Events with payload\n");
                }
        }
        printf("\n");
}

/* Main Function. This demonstrates using the DCI APIs defined in diag_lsm_dci.h*/
int main(int argc, char *argv[])
{
	int err, client_id;
	int signal_type = SIGCONT;
	boolean bInit_success = FALSE;
	diag_dci_peripherals list = DIAG_CON_MPSS | DIAG_CON_APSS | DIAG_CON_LPASS;
	struct diag_dci_health_stats *dci_health_stats; /* To collect DCI Health Statistics */
	unsigned char *dci_rsp_pkt = NULL;
	(void)argc;
	(void)argv;
	pthread_t thread_handle[NUM_PROC];
	int i;

	/* Signal handling to handle SSR */
	struct sigaction notify_action;	
	sigemptyset(&notify_action.sa_mask);
	notify_action.sa_sigaction = notify_handler;
	/* Use SA_SIGINFO to denote we are expecting data with the signal */
	notify_action.sa_flags = SA_SIGINFO;
	sigaction(signal_type, &notify_action, NULL);

	/* Signal handling for DCI Data */
	struct sigaction dci_data_action;
	sigemptyset(&dci_data_action.sa_mask);
	dci_data_action.sa_handler = dci_data_handler;
	dci_data_action.sa_flags = 0;
	sigaction(DIAG_SAMPLE_SIGNAL, &dci_data_action, NULL);

	for (i = 0; i < NUM_PROC; i++) {
		max_channel_mask = max_channel_mask | (1 << i);
	}
	parse_args(argc, argv);
	/* Registering with Diag which gives the client a handle to the Diag driver */
	bInit_success = Diag_LSM_Init(NULL);
	if (!bInit_success) {
		DIAG_LOGE(" Couldn't register with Diag LSM, errno: %d\n", errno);
		return -1;
	}

	for (i = 0; i < NUM_PROC; i++) {
		if (channel_mask & (1 << i)) {
			pthread_create(&thread_handle[i],NULL,dci_execute_func,&ch_info[i].ch_id);
			if (thread_handle[i] == 0) {
				DIAG_LOGE(" Failed to create thread .... Exiting App.\n");
			 	goto fail;
			}
		}
	}
	for (i = 0; i < NUM_PROC; i++) {
		if (channel_mask & (1 << i)) {
			pthread_join(thread_handle[i], NULL);
		}
	}
fail:
	Diag_LSM_DeInit();
	exit(0);
}
void dci_execute_func(void* data)
{
	int err, client_id;
	int signal_type = SIGCONT;
	boolean bInit_success = FALSE;
	diag_dci_peripherals list = DIAG_CON_MPSS | DIAG_CON_APSS | DIAG_CON_LPASS;
	struct diag_dci_health_stats *dci_health_stats; /* To collect DCI Health Statistics */
	unsigned char *dci_rsp_pkt = NULL;
	int channel = *(int *)data;
	dci_rsp_pkt = (unsigned char *)malloc(DIAG_MAX_RX_PKT_SIZ);
	if (!dci_rsp_pkt) {
		DIAG_LOGE("  Unable to allocate memory for DCI rsp pkt, errno: %d", errno);
		return;
	}

	/* Registering with DCI - This assigns a client ID */
	/* Channel 0 - MSM, 1 - MDM */
	err = diag_register_dci_client(&client_id, &list, channel, &signal_type);
	if (err != DIAG_DCI_NO_ERROR) {
		DIAG_LOGE(" Could not register with DCI, err: %d, errno: %d ch:%d\n", err, errno,channel);
		free(dci_rsp_pkt);
		return;
	} else
		DIAG_LOGE(" Successfully registered with DCI for ch:%d, client ID = %d\n", channel, client_id);

	err = diag_register_dci_signal_data(client_id, DIAG_SAMPLE_SIGNAL);
	if (err != DIAG_DCI_NO_ERROR) {
		DIAG_LOGE(" Could not register signal for DCI Data for ch:%d, err: %d, errno: %d\n", channel, err, errno);
		free(dci_rsp_pkt);
		return;
	}

	/* Getting supported Peripherals list*/
	DIAG_LOGE(" DCI Status on Processors for ch%d:\n",channel);
	err = diag_get_dci_support_list_proc(channel, &list);
	if (err != DIAG_DCI_NO_ERROR) {
		printf(" Could not get support list for ch %d, err: %d, errno: %d\n", channel, err, errno);
		free(dci_rsp_pkt);
		return;
	}
	DIAG_LOGE("   MPSS on ch:%d:\t ",channel);
	DIAG_LOGE((list & DIAG_CON_MPSS) ? "UP\n" : "DOWN\n");
	DIAG_LOGE("   LPASS on ch:%d:\t ", channel);
	DIAG_LOGE((list & DIAG_CON_LPASS) ? "UP\n" : "DOWN\n");
	DIAG_LOGE("   WCNSS on ch:%d:\t ",channel);
	DIAG_LOGE((list & DIAG_CON_WCNSS) ? "UP\n" : "DOWN\n");
	DIAG_LOGE("   APSS on ch:%d:\t ",channel);
	DIAG_LOGE((list & DIAG_CON_APSS) ? "UP\n" : "DOWN\n");


	/* Initializing Log and Event streaming by registering
	   listeners - This is required to receive Logs and Events */
	DIAG_LOGE(" Opening Data Stream, registering listeners for ch:%d\n", channel);
	err = diag_register_dci_stream_proc(client_id, ch_info[channel].dci_log_cb,  ch_info[channel].dci_event_cb);
	if (err != DIAG_DCI_NO_ERROR)
		DIAG_LOGE(" Unable to register DCI stream for ch:%d, err: %d, errno: %d\n", channel, err, errno);

	printf("\n");

	DIAG_LOGE(" Sending Asynchronous Command for ch:%d\n", channel);
	/* Diag Log stress test command - one Log packet comes out every second */
	unsigned char buf[24] = {75, 18, 0, 0, 1, 0, 0, 0, 16, 1, 1, 0, 0, 1, 0, 0, 232, 3, 0, 0, 1, 0, 0, 0};
	err = diag_send_dci_async_req(client_id, buf, 24, dci_rsp_pkt, DIAG_MAX_RX_PKT_SIZ, ch_info[channel].dci_cmd_rsp_cb, NULL);
	if (err != DIAG_DCI_NO_ERROR)
		DIAG_LOGE(" Error sending SEND_REQUEST_ASYNC to peripheral %d for ch:%d\n", err, channel);

	sleep(1);
	printf("\n");

	/* Setting Log masks ----------------------------
	   0x115F - Diag Stress Test Log
	   -------------------------------------------*/
	DIAG_LOGE(" Setting Log masks for channel %d\n", channel);
	DIAG_LOGE("   [0x115F] Diag Stress Test Log\n");
	uint16 log_codes_array[TOTAL_LOG_CODES] = { LOG_DIAG_STRESS_TEST_C };
	err = diag_log_stream_config(client_id, ENABLE, log_codes_array, TOTAL_LOG_CODES);
	if (err != DIAG_DCI_NO_ERROR)
		DIAG_LOGE(" Error sending SET_LOG_MASK to peripheral for ch:%d, err: %d, errno: %d\n", channel, err, errno);

	/* Putting the thread to sleep for 30 seconds. The client must sleep for the time it
	   wants to receive logs/events/command responses */
	sleep(30);
	printf("\n");

	/* Display Health Statistics */
	DIAG_LOGE(" Modem Health Statistics on the Apps processor for ch:%d :\n", channel);
	dci_health_stats = malloc(sizeof(struct diag_dci_health_stats));
	if (!dci_health_stats) {
		DIAG_LOGE("  Unable to allocate memory for DCI health stats, errno: %d", errno);
		free(dci_rsp_pkt);
		return;
	}

	err = diag_get_health_stats_proc(client_id, dci_health_stats, DIAG_MODEM_PROC);
	if (err == DIAG_DCI_NO_ERROR) {
		DIAG_LOGE("   Log Drop Count for Modem:\t%d\n", dci_health_stats->dropped_logs);
		DIAG_LOGE("   Log Total Count for Modem:\t%d\n", dci_health_stats->received_logs);
		DIAG_LOGE("   Event Drop Count for Modem:\t%d\n", dci_health_stats->dropped_events);
		DIAG_LOGE("   Event Total Count for Modem:\t%d\n", dci_health_stats->received_events);
	} else
		DIAG_LOGE("   Error in collecting statistics, err: %d, errno: %d\n", err, errno);

	sleep(1);
	printf("\n");

	/* Clearing these log masks ------------------------
	   0x115F - Diag Stress Test Log
	   ---------------------------------------------------- */
	DIAG_LOGE(" Clearing log masks for ch:%d\n", channel);
	DIAG_LOGE("   [0x115F] Diag Stress Test Log\n");
	err = diag_log_stream_config(client_id, DISABLE, log_codes_array, TOTAL_LOG_CODES);
	if (err != DIAG_DCI_NO_ERROR)
		DIAG_LOGE(" Error sending CLEAR_LOG_MASK to peripheral for ch:%d, err: %d, errno: %d\n", channel, err, errno);

	sleep(5);
	printf("\n");

	/* Setting Log masks ----------------------------
	   0x117B - WMS Set Routes
	   0x11EB - DPL Log Messages
	   0x1160 - WMS Read
	   -------------------------------------------*/
	DIAG_LOGE(" *** To generate the following logs on APSS, please run the test_diag app\n");
	DIAG_LOGE(" Setting Log masks for ch:%d\n", channel);
	DIAG_LOGE("   [0x117B] WMS Set Routes\n");
	DIAG_LOGE("   [0x11EB] DPL Log Messages\n");
	DIAG_LOGE("   [0x1160] WMS Read\n");

	uint16 log_codes_array_apps[TOTAL_LOG_CODES_APPS] = { LOG_WMS_SET_ROUTES_C,
							      LOG_DATA_PROTOCOL_LOGGING_C,
							      LOG_WMS_READ_C };
	err = diag_log_stream_config(client_id, ENABLE, log_codes_array_apps, TOTAL_LOG_CODES_APPS);
	if (err != DIAG_DCI_NO_ERROR)
		DIAG_LOGE(" Error sending SET_LOG_MASK to peripheral for ch:%d, err: %d, errno: %d\n", channel, err, errno);

	/* Putting the thread to sleep for 30 seconds. The client must sleep for the time it
	   wants to receive logs/events/command responses */
	sleep(30);
	printf("\n");

	/* Display Health Statistics */
	DIAG_LOGE(" Health statistics for the Application processor Data for ch:%d: \n", channel);
	err = diag_get_health_stats_proc(client_id, dci_health_stats, DIAG_APPS_PROC);
	if (err == DIAG_DCI_NO_ERROR) {
		DIAG_LOGE("   Log Drop Count for APSS:\t%d\n", dci_health_stats->dropped_logs);
		DIAG_LOGE("   Log Total Count for APSS:\t%d\n", dci_health_stats->received_logs);
		DIAG_LOGE("   Event Drop Count for APSS:\t%d\n", dci_health_stats->dropped_events);
		DIAG_LOGE("   Event Total Count for APSS:\t%d\n", dci_health_stats->received_events);
	} else
		DIAG_LOGE("   Error in collecting statistics, err: %d, errno: %d\n", err, errno);

	sleep(1);

	printf("\n");

	/* Clearing these log masks ------------------------
	   0x117B - WMS Set Routes
	   0x11EB - DPL Log Messages
	   0x1160 - WMS Read
	   ---------------------------------------------------- */
	DIAG_LOGE(" Clearing log masks for ch:%d\n", channel);
	DIAG_LOGE("   [0x117B] WMS Set Routes\n");
	DIAG_LOGE("   [0x11EB] DPL Log Messages\n");
	DIAG_LOGE("   [0x1160] WMS Read\n");
	err = diag_log_stream_config(client_id, DISABLE, log_codes_array_apps, TOTAL_LOG_CODES_APPS);
	if (err != DIAG_DCI_NO_ERROR)
		DIAG_LOGE(" Error sending CLEAR_LOG_MASK to peripheral, err: %d, errno: %d\n", err, errno);

	sleep(5);
	printf("\n");

	DIAG_LOGE(" ##### STARTING Ext API Log Packets unit test ##### for channel:%d\n", channel);
	DIAG_LOGE(" Registering for Version 1 of DCI (EXT Header Support) for channel:%d", channel);
	err = diag_dci_set_version(client_id, 1);
	if (err != DIAG_DCI_NO_ERROR)
		DIAG_LOGE(" Error registering version to DCI %d\n", err);


	DIAG_LOGE(" Sending Asynchronous Command for Ext API Log Packets for channel %d\n", channel);
	/* Diag Log stress test command - one Log packet comes out every second */
	unsigned char log_ext[24] = {75, 18, 0, 0, 1, 0, 0, 0, 57, 1, 1, 0, 0, 1, 0, 0, 232, 3, 0, 0, 1, 0, 0, 0};
	err = diag_send_dci_async_req(client_id, log_ext, 24, dci_rsp_pkt, DIAG_MAX_RX_PKT_SIZ, &process_response, NULL);
	if (err != DIAG_DCI_NO_ERROR)
		DIAG_LOGE(" Error sending SEND_REQUEST_ASYNC to peripheral %d\n", err);

	/* Diag Events w/ payload test command - one Log packet comes out every second */
	DIAG_LOGE(" Sending Asynchronous Command for Ext API Event Packets for ch:%d\n", channel);
	unsigned char event_ext[24] = {75, 18, 0, 0, 1, 0, 0, 0, 70, 1, 1, 0, 0, 1, 0, 0, 232, 3, 0, 0, 1, 0, 0, 0};
	err = diag_send_dci_async_req(client_id, event_ext, 24, dci_rsp_pkt, DIAG_MAX_RX_PKT_SIZ, &process_response, NULL);
	if (err != DIAG_DCI_NO_ERROR)
		DIAG_LOGE(" Error sending SEND_REQUEST_ASYNC to peripheral %d\n", err);

	sleep(1);
	printf("\n");

	/* Setting Log masks ----------------------------
	   0x115F - Diag Stress Test Log
	   -------------------------------------------*/
	DIAG_LOGE(" Setting Log masks for ch:%d\n", channel);
	DIAG_LOGE("   [0x115F] Diag Stress Test Log\n");
	err = diag_log_stream_config(client_id, ENABLE, log_codes_array, TOTAL_LOG_CODES);
	if (err != DIAG_DCI_NO_ERROR)
		DIAG_LOGE(" Error sending SET_LOG_MASK to peripheral, err: %d, errno: %d\n", err, errno);

	/* Setting Log masks ----------------------------
	   0x115F - Diag Stress Test Event
	   -------------------------------------------*/
	DIAG_LOGE(" Setting Event masks for ch:%d\n", channel);
	DIAG_LOGE("   EVENT_DIAG_STRESS_TEST_NO_PAYLOAD\n");
	DIAG_LOGE("   EVENT_DIAG_STRESS_TEST_WITH_PAYLOAD\n");
	DIAG_LOGE("   EVENT_DIAG_STRESS_TEST_COMPLETED\n");
	DIAG_LOGE("   EVENT_DIAG_DROP_DEBUG\n");

	int event_codes_array[TOTAL_EVENT_CODES] = {
		EVENT_DIAG_STRESS_TEST_NO_PAYLOAD,
		EVENT_DIAG_STRESS_TEST_WITH_PAYLOAD,
		EVENT_DIAG_STRESS_TEST_COMPLETED,
		EVENT_DIAG_DROP_DEBUG
	};

	err = diag_event_stream_config(client_id, ENABLE, event_codes_array, TOTAL_EVENT_CODES);
	if (err != DIAG_DCI_NO_ERROR)
		DIAG_LOGE(" Error sending SET_EVENT_MASK to peripheral, err: %d, errno: %d\n", err, errno);

	/* Putting the thread to sleep for 30 seconds. The client must sleep for the time it
	   wants to receive logs/events/command responses */
	sleep(30);
	printf("\n");

	/* Test Ext Header stripping for dci clients that do not support version 1 of DCI */
	DIAG_LOGE(" Un-Registering for Version 1 of DCI (EXT Header Support) for ch:%d", channel);
	err = diag_dci_set_version(client_id, 0);
	if (err != DIAG_DCI_NO_ERROR)
		DIAG_LOGE(" Error registering version to DCI %d\n", err);

	sleep(30);
	printf("\n");

	/* Display Health Statistics */
	DIAG_LOGE(" Modem Health Statistics on the Apps processor for ch:%d:\n");
	dci_health_stats = malloc(sizeof(struct diag_dci_health_stats));
	if (!dci_health_stats) {
		DIAG_LOGE("  Unable to allocate memory for DCI health stats, errno: %d", errno);
		free(dci_rsp_pkt);
		return;
	}

	err = diag_get_health_stats_proc(client_id, dci_health_stats, DIAG_MODEM_PROC);
	if (err == DIAG_DCI_NO_ERROR) {
		DIAG_LOGE("   Log Drop Count for Modem:\t%d\n", dci_health_stats->dropped_logs);
		DIAG_LOGE("   Log Total Count for Modem:\t%d\n", dci_health_stats->received_logs);
		DIAG_LOGE("   Event Drop Count for Modem:\t%d\n", dci_health_stats->dropped_events);
		DIAG_LOGE("   Event Total Count for Modem:\t%d\n", dci_health_stats->received_events);
	} else
		DIAG_LOGE("   Error in collecting statistics, err: %d, errno: %d\n", err, errno);

	sleep(1);
	printf("\n");

	/* Clearing these log masks ------------------------
	   0x115F - Diag Stress Test Log
	   ---------------------------------------------------- */
	DIAG_LOGE(" Clearing log masks for ch:%d\n", channel);
	DIAG_LOGE("   [0x115F] Diag Stress Test Log\n");
	err = diag_log_stream_config(client_id, DISABLE, log_codes_array, TOTAL_LOG_CODES);
	if (err != DIAG_DCI_NO_ERROR)
		DIAG_LOGE(" Error sending CLEAR_LOG_MASK to peripheral, err: %d, errno: %d\n", err, errno);

	/* Clearing Event masks ----------------------------
	   0x115F - Diag Stress Test Event
	   -------------------------------------------*/
	DIAG_LOGE(" Clearing event masks for ch:%d\n", channel);
	DIAG_LOGE("   EVENT_DIAG_STRESS_TEST_NO_PAYLOAD\n");
	DIAG_LOGE("   EVENT_DIAG_STRESS_TEST_WITH_PAYLOAD\n");
	DIAG_LOGE("   EVENT_DIAG_STRESS_TEST_COMPLETED\n");
	DIAG_LOGE("   EVENT_DIAG_DROP_DEBUG\n");
	err = diag_event_stream_config(client_id, DISABLE, event_codes_array, TOTAL_EVENT_CODES);
	if (err != DIAG_DCI_NO_ERROR)
		DIAG_LOGE(" Error sending CLEAR_EVENT_MASK to peripheral, err: %d, errno: %d\n", err, errno);

	/* Releasing DCI connection */
	DIAG_LOGE(" Releasing DCI connection  for ch:%d\n", channel);
	err = diag_release_dci_client(&client_id);
	if (err != DIAG_DCI_NO_ERROR)
		DIAG_LOGE(" Error releasing DCI connection, err: %d, errno: %d\n", err, errno);

	printf("\n");
	free(dci_health_stats);
	free(dci_rsp_pkt);
	return;
}
