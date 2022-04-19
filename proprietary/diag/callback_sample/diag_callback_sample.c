/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

		Sample Application for Diag Callback Interface

GENERAL DESCRIPTION
  Contains sample implementation of Diagnostic Callback APIs.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2012-2014, 2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "malloc.h"
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include "errno.h"
#include "msg.h"
#include "diag_lsm.h"
#include "stdio.h"
#include "diagpkt.h"
#include "diag_lsmi.h"
#include "diag_shared_i.h"

#define REQ_LOOPBACK_LEN	7
#define REQ_STRESSTEST_LEN	24

extern int proc_type;
static int disable_hdlc = 0;
static int use_mask_file = 0;

/* Callback for the receiving Diag data */
int process_diag_data(unsigned char *ptr, int len, void *context_data)
{
	int i;
	if (!ptr) {
		return 0;
	}

	if (context_data) {
		if (*(int *)context_data == MSM) {
			DIAG_LOGE("diag_callback_sample: Received data of len %d from MSM", len);
		} else if (*(int *)context_data == MDM) {
			DIAG_LOGE("diag_callback_sample: Received data of len %d from MDM", len);
		} else if (*(int *)context_data == MDM_2) {
			DIAG_LOGE("diag_callback_sample: Received data of len %d from MDM_2", len);
		} else {
			DIAG_LOGE("diag_callback_sample: Received data of len %d from unknown proc %d", len, *(int *)context_data);
		}
	}

	for (i = 0; i < len; i++) {
		if (i % 8 == 0) {
			DIAG_LOGE("\n  ");
		}
		DIAG_LOGE("%02x ", ptr[i]);
	}
	DIAG_LOGE("\n");

	return 0;
}

/* Helper function to check if MDM is supported */
static uint8 is_mdm_supported()
{
	uint16 remote_mask = 0;
	uint8 err = 0;
	err = diag_has_remote_device(&remote_mask);
	if (err != 1) {
		DIAG_LOGE("diag_callback_sample: Unable to check for MDM support, err: %d\n", errno);
		return 0;
	}
	return (remote_mask & MDM);
}

static void usage(char *progname)
{
	printf("\n Usage for %s:\n", progname);
	printf("\n-a  --hdlcdisable:\t Disable hdlc encoding\n");
	printf("\n-u, --filemsm:\t mask file name for MSM\n");
	exit(0);
}

static void parse_args(int argc, char **argv)
{
	int command, temp_size;
	struct option longopts[] =
	{
		{"disablehdlc",	1,	NULL,	'a'},
		{"usemaskfile",	1,	NULL,	'u'},
	};

	while ((command = getopt_long(argc, argv, "au", longopts, NULL))
			!= -1) {
		switch (command) {
		case 'a':
			disable_hdlc = 1;
			break;
		case 'u':
			use_mask_file = 1;
			break;
		case 'h':
		default:
			usage(argv[0]);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	int err = 0, ret = 0, z;
	int data_primary = MSM, data_remote;
	uint16 remote_mask = 0, remote_mask_reg = 0;
	boolean status = FALSE;
	unsigned char req_modem_loopback[REQ_LOOPBACK_LEN] =
		{ 75, 18, 41, 0, 1, 2, 3 };
	unsigned char req_modem_msg_stress_test[REQ_STRESSTEST_LEN] =
		{ 75, 18, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned char req_adsp_log_stress_test[REQ_STRESSTEST_LEN] =
		{ 75, 18, 7, 0, 1, 0, 0, 0, 16, 1, 1, 0, 0, 1, 0, 0, 5, 0, 0, 0, 10, 0, 0, 0 };

	parse_args(argc, argv);

	status = Diag_LSM_Init(NULL);
	if (!status) {
		DIAG_LOGE("diag_callback_sample: Diag LSM Init failed, exiting... err: %d\n", errno);
		exit(0);
	}

	/* Register the callback function for receiving data from MSM */
	diag_register_callback(process_diag_data, &data_primary);

	diag_has_remote_device(&remote_mask);
	remote_mask_reg = remote_mask;
	z = 1;
	while (remote_mask_reg) {
		if (remote_mask_reg & 1) {
			data_remote = z;
			diag_register_remote_callback(process_diag_data, data_remote, &data_remote);
		}
		z++;
		remote_mask_reg = remote_mask_reg >> 1;
	}

	/* Switch to Callback mode to receive Diag data in this application */
	diag_switch_logging(CALLBACK_MODE, NULL);

	if (disable_hdlc) {
		ret = diag_hdlc_toggle(HDLC_DISABLE);
		if (ret == 1) {
			DIAG_LOGE("diag_callback_sample: HDLC successfully disabled\n");
		} else {
			DIAG_LOGE("diag_callback_sample: Unable to switch the HDLC, exiting app\n");
			diag_switch_logging(USB_MODE, NULL);
			exit(0);
		}
	}

	if (use_mask_file) {
		proc_type = MSM;
		status = diag_read_mask_file();
		if (status)
			DIAG_LOGE("diag_callback_sample: Error reading mask file, proc_type: %d\n",
					proc_type);

		if(remote_mask) {
			remote_mask_reg = remote_mask;
			z = 1;
			while (remote_mask_reg) {
				if (remote_mask_reg & 1) {
					proc_type = z;
					DIAG_LOGE("\diag_callback_sample: Reading mask for proc_type: %d\n", proc_type);
					status = diag_read_mask_file();
					if (status)
						DIAG_LOGE("diag_callback_sample: Error reading mask file, proc_type: %d\n", proc_type);
				}
				z++;
				remote_mask_reg = remote_mask_reg >> 1;
			}
		}
	}

	/*
	 * You can now send requests to the processors. The response, and any
	 * log, event or F3s will be sent via the callback function.
	 */
	DIAG_LOGE("diag_callback_sample: Sending Modem loopback request to MSM\n");
	err = diag_callback_send_data(MSM, req_modem_loopback, REQ_LOOPBACK_LEN);
	if (err) {
		DIAG_LOGE("diag_callback_sample: Unable to send Modem loopback request to MSM\n");
	} else {
		sleep(2);
	}

	DIAG_LOGE("diag_callback_sample: Sending Modem Message Stress Test to MSM\n");
	err = diag_callback_send_data(MSM, req_modem_msg_stress_test, REQ_STRESSTEST_LEN);
	if (err) {
		DIAG_LOGE("diag_callback_sample: Unable to send Modem Message Stress Test to MSM\n");
	} else {
		sleep(30);
	}

	DIAG_LOGE("diag_callback_sample: Sending ADSP Log Stress Test to MSM\n");
	err = diag_callback_send_data(MSM, req_adsp_log_stress_test, REQ_STRESSTEST_LEN);
	if (err) {
		DIAG_LOGE("diag_callback_sample: Unable to send ADSP Log Stress Test to MSM\n");
	} else {
		sleep(30);
	}

	if (!remote_mask)
		goto finish;

	if(remote_mask) {
		remote_mask_reg = remote_mask;
		z = 1;
		while (remote_mask_reg) {
			if (remote_mask_reg & 1) {

				proc_type = z;

				DIAG_LOGE("diag_callback_sample: Sending Modem loopback request to proc: %d\n", proc_type);
				/* If remote_proc is supported, send the requests to external proc ASIC as well */
				err = diag_callback_send_data(proc_type, req_modem_loopback, REQ_LOOPBACK_LEN);
				if (err) {
					DIAG_LOGE("diag_callback_sample: Unable to send Modem loopback request to proc: %d\n", proc_type);
				} else {
					sleep(2);
				}

				DIAG_LOGE("diag_callback_sample: Sending Modem Message Stress Test to proc: %d\n", proc_type);
				err = diag_callback_send_data(proc_type, req_modem_msg_stress_test, REQ_STRESSTEST_LEN);
				if (err) {
					DIAG_LOGE("diag_callback_sample: Unable to send Modem Message Stress Test to proc: %d\n", proc_type);
				} else {
					sleep(30);
				}

				DIAG_LOGE("diag_callback_sample: Sending ADSP Log Stress Test to proc: %d\n", proc_type);
				err = diag_callback_send_data(proc_type, req_adsp_log_stress_test, REQ_STRESSTEST_LEN);
				if (err) {
					DIAG_LOGE("diag_callback_sample: Unable to send ADSP Log Stress Test to proc: %d\n", proc_type);
				} else {
					sleep(30);
				}
			}
			z++;
			remote_mask_reg = remote_mask_reg >> 1;
		}
	}

finish:
	/*
	 * When you are done using the Callback Mode, it is highly recommended
	 * that you switch back to USB Mode.
	 */
	diag_switch_logging(USB_MODE, NULL);

	/* Release the handle to Diag*/
	status = Diag_LSM_DeInit();
	if (!status) {
		DIAG_LOGE("diag_callback_sample: Unable to close handle to diag driver, err: %d\n", errno);
		exit(0);
	}

	return 0;
}
