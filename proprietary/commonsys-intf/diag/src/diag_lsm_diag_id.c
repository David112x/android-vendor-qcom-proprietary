/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
Copyright (c) 2017-2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

              Diag id Support

GENERAL DESCRIPTION

Library definition for using peripheral diag_id information
by sending diag command request/responses.

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#include <stdlib.h>
#include "comdef.h"
#include "stdio.h"
#include "stringl.h"
#include "diag_lsmi.h"
#include "./../include/diag_lsm.h"
#include "diag_lsmi.h"
#include "diag_shared_i.h"
#include "stdio.h"
#include "string.h"
#include <malloc.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include "diagcmd.h"
#include "errno.h"

#ifndef FEATURE_LE_DIAG
#include <cutils/log.h>
#endif

typedef enum {
	DIAG_ID_PARSER_STATE_OFF = 0,
	DIAG_ID_PARSER_STATE_ON,
} diag_id_parser_state;

int diag_id_state = 0;

struct diag_read_buf_pool {
	unsigned char* rsp_buf;
	int data_ready;
	pthread_mutex_t rsp_mutex;
	pthread_cond_t rsp_cond;
};

static struct diag_read_buf_pool diag_read_buffer_pool;
int diagid_kill_thread = 0;

pthread_t diagid_thread_hdl;	/* Diagid thread handle */

diag_id_list *diag_id_head[NUM_PROC];

static int wait_for_diag_id_response()
{
	struct timespec time;
	struct timeval now;
	int rt = 0;

	gettimeofday(&now, NULL);
	time.tv_sec = now.tv_sec + 10000 / 1000;
	time.tv_nsec = now.tv_usec + (10000 % 1000) * 1000000;
	pthread_mutex_lock(&(diag_read_buffer_pool.rsp_mutex));
	if (!diag_read_buffer_pool.data_ready)
		rt = pthread_cond_timedwait(&(diag_read_buffer_pool.rsp_cond),
			&(diag_read_buffer_pool.rsp_mutex), &time);
	return rt;
}

int diag_id_init()
{
	pthread_mutex_init(&(diag_read_buffer_pool.rsp_mutex), NULL);
	pthread_cond_init(&(diag_read_buffer_pool.rsp_cond), NULL);

	diag_read_buffer_pool.data_ready = 0;
	diag_read_buffer_pool.rsp_buf = malloc(DIAG_CMD_RSP_BUF_SIZE);
	if (!diag_read_buffer_pool.rsp_buf){
		DIAG_LOGE("%s:failed to create rsp buffer\n", __func__);
		return -1;
	}
	return 0;
}

static int diag_query_pd_name(char *process_name, char *search_str)
{
	if (!process_name)
		return -EINVAL;

	if (strstr(process_name, search_str))
		return 1;

	return 0;
}

int diag_query_pd(char *process_name)
{
	if (!process_name)
		return -EINVAL;

	if (diag_query_pd_name(process_name, "APPS"))
		return DIAG_APPS_PROC;
	if (diag_query_pd_name(process_name, "Apps"))
		return DIAG_APPS_PROC;
	if (diag_query_pd_name(process_name, "modem/root_pd"))
		return DIAG_MODEM_PROC;
	if (diag_query_pd_name(process_name, "adsp/root_pd"))
		return DIAG_LPASS_PROC;
	if (diag_query_pd_name(process_name, "slpi/root_pd"))
		return DIAG_SENSORS_PROC;
	if (diag_query_pd_name(process_name, "cdsp/root_pd"))
		return DIAG_CDSP_PROC;
	if (diag_query_pd_name(process_name, "npu/root_pd"))
		return DIAG_NPU_PROC;
	if (diag_query_pd_name(process_name, "wlan_pd"))
		return UPD_WLAN;
	if (diag_query_pd_name(process_name, "audio_pd"))
		return UPD_AUDIO;
	if (diag_query_pd_name(process_name, "sensor_pd"))
		return UPD_SENSORS;

	return -EINVAL;
}

diag_id_list *get_diag_id(int peripheral_type, int peripheral)
{
	diag_id_list *item = NULL;

	if (peripheral <  DIAG_MODEM_PROC || peripheral >  TOTAL_PD_COUNT)
		return 0;

	item = diag_id_head[peripheral_type];

	while (item != NULL) {
		if ((peripheral == item->peripheral) &&
			diag_query_pd_name(item->process_name, "root"))
			return item;

		item = item->next;
	}
	return 0;
}

void insert_diag_id_entry(diag_id_entry_struct *entry, int peripheral_type)
{
	uint8 peripheral;
	diag_id_list *new_entry = NULL, *temp = NULL;
	int count;
	unsigned char* src_ptr = NULL;

	new_entry = malloc(sizeof(diag_id_list)-1);
	if (!new_entry)
		return;
	new_entry->diag_id = entry->diag_id;
	strlcpy(new_entry->process_name, &entry->process_name, MAX_DIAGID_STR_LEN);
	new_entry->peripheral = diag_query_pd(new_entry->process_name);
	peripheral = new_entry->peripheral;
	new_entry->next = NULL;
	if (peripheral_type >= 0) {
		if (diag_id_head[peripheral_type] == NULL) {
			diag_id_head[peripheral_type] = new_entry;
		} else {
			temp = diag_id_head[peripheral_type];
			while (temp->next != NULL) {
				temp = (diag_id_list *)temp->next;
			}
			temp->next = new_entry;
		}
	}
}


void process_diag_id_response(int peripheral_type)
{
	diag_id_entry_struct *diag_id_ptr;
	int i = 0, offset;
	uint8 len;
	diag_id_list_rsp* rsp;
	unsigned char *buf_ptr, *temp_ptr_char, *src_ptr = NULL;
	unsigned char process_name[30];
	uint8 *temp_ptr = NULL;

	buf_ptr = diag_read_buffer_pool.rsp_buf;

	if (buf_ptr[0] == DIAG_BAD_CMD_F) {

	} else {
		rsp = (diag_id_list_rsp*)buf_ptr;
		if ((rsp->version != 1) || (rsp->header.subsys_cmd_code != 0x222))
			return;

		diag_id_ptr = &rsp->entry;
		for (i = 0; i < rsp->num_entries; i++) {
			temp_ptr = (uint8 *)diag_id_ptr;
			len = *(uint8 *)(temp_ptr + sizeof(uint8));
			offset = 2 * sizeof(uint8) + len;
			insert_diag_id_entry(diag_id_ptr, peripheral_type);
			diag_id_ptr = (diag_id_entry_struct *)(temp_ptr + offset);
		}
	}

	return;
}

unsigned char mask_buf[DIAG_CMD_REQ_BUF_SIZE];
int diag_query_diag_id(int peripheral_type)
{
	diag_id_list_req *req;
	int offset = 0, length = 0, ret = 0;
	unsigned char *ptr = mask_buf;

	memset(mask_buf, 0, DIAG_CMD_REQ_BUF_SIZE);

	if (peripheral_type < MSM || peripheral_type > MDM_2) {
		DIAG_LOGE("diag:%s Invalid peripheral_type = %d to query diag_id\n",
			__func__, peripheral_type);
		return FALSE;
	}

	*(int*)ptr = USER_SPACE_RAW_DATA_TYPE;
	offset += sizeof(int);
	if (peripheral_type) {
		*(int*)(ptr + offset) = -peripheral_type;
		offset += sizeof(int);
	}
	ptr = ptr + offset;
	req = (diag_id_list_req*)ptr;

	req->header.cmd_code = DIAG_SUBSYS_CMD_F;
	req->header.subsys_id = DIAG_SUBSYS_DIAG_SERV;
	req->header.subsys_cmd_code = DIAG_GET_DIAG_ID;
	req->version = 1;
	length = sizeof(diag_id_list_req);

	if (length + offset <= DIAG_CMD_REQ_BUF_SIZE) {
		ret = diag_send_data(mask_buf, offset + length);
		if (ret)
			return FALSE;
	}

	return TRUE;
}

int diag_get_peripheral_diag_id_info(int peripheral_type)
{
	int j, err;

	if (diagid_kill_thread)
		return 0;

	DIAG_LOGD("diag:%s query diagid information for ptype:%d\n",
				__func__, peripheral_type);

	diag_id_state = 1;
	diag_read_buffer_pool.data_ready = 0;
	err = diag_query_diag_id(peripheral_type);
	if (err == FALSE || diagid_kill_thread) {
		DIAG_LOGE("diag: %s, Failure to send diag_id query command to proc: %d\n",
			__func__, peripheral_type);
		return -1;
	}
	err = wait_for_diag_id_response();
	if (err == ETIMEDOUT || diagid_kill_thread) {
		DIAG_LOGE("diag:%s time out while waiting for diag_id cmd response for p_type:%d\n",
			__func__, peripheral_type);
		pthread_mutex_unlock(&(diag_read_buffer_pool.rsp_mutex));
		diag_id_state = 0;
		return -1;
	}

	return 0;
}
void diag_wait_for_diag_id_info()
{
	pthread_join(diagid_thread_hdl, NULL);
}
static void* diag_query_diag_id_info(void)
{
	int i, err;
	uint16 remote_mask = 0;

	if (diagid_kill_thread)
		return 0;

	diag_get_peripheral_diag_id_info(MSM);

	diag_has_remote_device(&remote_mask);
	if (remote_mask) {
		for (i = 1; i <= NUM_PROC; i++) {
			if (diagid_kill_thread)
				break;
			if (remote_mask & 1 << (i - 1)) {
				diag_get_peripheral_diag_id_info(i);
			}
		}
	}

	return 0;
}

void diag_get_diag_id_info(void)
{
	pthread_create(&diagid_thread_hdl, NULL, diag_query_diag_id_info, NULL);
	if (diagid_thread_hdl == 0)
		DIAG_LOGE("%s: Failed to create diagid thread", __func__);
}

static int check_for_diagid_cmd(uint8* src_ptr)
{
	uint16 cmd_code, version;
	unsigned int i, offset = 0;

	if (!src_ptr)
		return FALSE;

	if (((*src_ptr == DIAG_SUBSYS_CMD_F && *(src_ptr + 1) == DIAG_SUBSYS_DIAG_SERV) ||
		 (((*src_ptr == DIAG_BAD_CMD_F) || (*src_ptr == DIAG_BAD_LEN_F) || (*src_ptr == DIAG_BAD_PARM_F)) &&
		 *(src_ptr + 1) == DIAG_SUBSYS_CMD_F && *(src_ptr + 2) == DIAG_SUBSYS_DIAG_SERV)))
	{
		if (*src_ptr == DIAG_SUBSYS_CMD_F) {
			memcpy(&cmd_code, src_ptr + 2, sizeof(cmd_code));
		} else {
			memcpy(&cmd_code, src_ptr + 3, sizeof(cmd_code));
		}

		if (cmd_code == DIAG_GET_DIAG_ID)
			return TRUE;
		else
			return FALSE;
	} else {
		return FALSE;
	}
}

int parse_data_for_diag_id_rsp(uint8* ptr, int count_received_bytes, int index)
	{
		unsigned char* dest_ptr = NULL;
		unsigned int src_length = 0, dest_length = 0;
		unsigned int len = 0;
		unsigned int i;
		uint8 src_byte;
		uint8* src_ptr = NULL;
		uint16 payload_len = 0;
		int err, bytes_read = 0;

		if (diagid_kill_thread)
			return -1;

		while (bytes_read < count_received_bytes) {

			src_ptr = ptr + bytes_read;
			src_length = count_received_bytes - bytes_read;

			if (hdlc_disabled) {
				payload_len = *(uint16 *)(src_ptr + 2);
				if (check_for_diagid_cmd(src_ptr + 4))
				{
					diag_id_state = 0;
					dest_ptr = diag_read_buffer_pool.rsp_buf;
					dest_length = DIAG_CMD_RSP_BUF_SIZE;
					if (payload_len <= DIAG_CMD_RSP_BUF_SIZE)
						memcpy(dest_ptr, src_ptr + 4, payload_len);
					else
						return -1;

					process_diag_id_response(index);

					diag_read_buffer_pool.data_ready = 1;
					pthread_cond_signal(&(diag_read_buffer_pool.rsp_cond));
					pthread_mutex_unlock(&(diag_read_buffer_pool.rsp_mutex));
					bytes_read += payload_len + 5;
				} else {
					bytes_read += payload_len + 5;
				}
			} else {
				if (check_for_diagid_cmd(src_ptr)) {
					diag_id_state = 0;
					dest_ptr = diag_read_buffer_pool.rsp_buf;
					dest_length = DIAG_CMD_RSP_BUF_SIZE;
					for (i = 0; i < src_length; i++) {
						src_byte = src_ptr[i];

						if (src_byte == ESC_CHAR) {
							if (i == (src_length - 1)) {
								i++;
								break;
							} else {
								dest_ptr[len++] = src_ptr[++i] ^ 0x20;
							}
						} else if (src_byte == CTRL_CHAR) {
							if (i == 0 && src_length > 1)
								continue;
							dest_ptr[len++] = src_byte;
							i++;
							break;
						} else {
							dest_ptr[len++] = src_byte;
						}

						if (len >= dest_length) {
							i++;
							break;
						}
					}
					bytes_read += i;

					process_diag_id_response(index);

					i = 0;
					len = 0;
					diag_read_buffer_pool.data_ready = 1;
					pthread_cond_signal(&(diag_read_buffer_pool.rsp_cond));
					pthread_mutex_unlock(&(diag_read_buffer_pool.rsp_mutex));
				} else {
					for (i = 0; i < src_length; i++) {
						if (src_ptr[i] == CTRL_CHAR) {
							i++;
							break;
						}
					}
					bytes_read += i;
					i = 0;
					len = 0;
				}
			}
		}
		return 0;
	}

void diag_kill_diagid_threads(void)
{
	diagid_kill_thread = 1;
	DIAG_LOGE("diag: %s: Initiate diagid threads kill (diagid_kill_thread: %d)\n",
		__func__, diagid_kill_thread);

	pthread_cond_signal(&(diag_read_buffer_pool.rsp_cond));
	pthread_mutex_unlock(&(diag_read_buffer_pool.rsp_mutex));

	sleep(1);

	pthread_cond_destroy(&(diag_read_buffer_pool.rsp_cond));
	pthread_mutex_destroy(&(diag_read_buffer_pool.rsp_mutex));

	if (diag_read_buffer_pool.rsp_buf)
		free(diag_read_buffer_pool.rsp_buf);

	DIAG_LOGE("diag:In %s Cleaned up diagid resources\n", __func__);
}
