/******************************************************************************
Copyright (c) 2016, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#define LOG_TAG "bt_logger_buffer"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cutils/sockets.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <dirent.h>
#include <utils/Log.h>
#include "bt_logger.h"

#define PACKET_LEN_STR 4

void create_log_node(bt_log_buffer_t *log_list, bt_log_node_t **ret_node)
{
    bt_log_node_t *log_node = NULL;
    static int create_new_nodes = 0;

    if (log_list->buff_size >= log_list->max_buff_size) {
        if(create_new_nodes) {
            create_new_nodes = 0;
            ALOGW("Started recycling nodes to save logs");
        }
        pop_head_node(log_list, &log_node);
        if (log_node) {
            log_node->nxt_node = NULL;
            free(log_node->log_str);
            *ret_node = log_node;
            return;
        }
    }

    log_node = (bt_log_node_t*)calloc(1, sizeof(bt_log_node_t));
    if(!create_new_nodes) {
        create_new_nodes = 1;
        ALOGW("Started creating new nodes to save logs");
    }

    if (log_node == NULL) {
        ALOGE("Allocation failed for bt_log_node_t, returning!");
        return;
    }

    *ret_node = log_node;
}

size_t add_data_to_node(bt_log_buffer_t *log_list, bt_log_node_t *l_node, char *l_data, int log_len)
{
    char *log_msg;

    log_msg = (char*)calloc(log_len + 1, sizeof(char));
    if (log_msg == NULL) {
        ALOGE("Allocation failed for log_msg, returning!");
        return 0;
    }

    log_list->saved_log_len = strlcpy(log_msg, l_data, log_len + 1);
    l_node->log_str = log_msg;

    if (log_len > log_list->saved_log_len) {
        log_list->pending_pkt_len = log_len - log_list->saved_log_len;
    }

    return ((log_len + 1) * sizeof(char));
}

void add_node_to_list(bt_log_buffer_t *log_list, bt_log_node_t *log_node, size_t log_size)
{
    if (log_list->head == NULL) {
        log_list->head = log_node;
        log_list->tail = log_node;
        log_list->buff_size = log_size;
    }else  {
        log_list->tail->nxt_node = log_node;
        log_list->tail = log_node;

        if (log_list->buff_size < log_list->max_buff_size)
            log_list->buff_size += log_size;
    }
}

void init_list( bt_log_buffer_t **list, size_t list_max_size )
{
    bt_log_buffer_t *log_list = malloc(sizeof(bt_log_buffer_t));

    if (log_list) {
        log_list->head = NULL;
        log_list->tail = NULL;
        log_list->max_buff_size =  list_max_size;
        log_list->buff_size =  0;
        log_list->pending_pkt_len =  0;
        log_list->saved_log_len =  0;
        *list = log_list;
    }else  {
        ALOGE("Allocation failed for list creation");
    }
}


size_t append_recovered_data_to_tail(bt_log_buffer_t *log_list, char *buff)
{
    size_t ret = 0;

    if (log_list->pending_pkt_len) {
        strlcpy(&log_list->tail->log_str[log_list->saved_log_len], buff,
            log_list->pending_pkt_len + 1);
        ret = log_list->pending_pkt_len;
        log_list->pending_pkt_len = 0;
    }
    return ret;
}

void pop_head_node(bt_log_buffer_t *log_list, bt_log_node_t **l_node)
{
    if (log_list->head) {
        *l_node = log_list->head;
        log_list->head = log_list->head->nxt_node;

        if (log_list->head == NULL)
            log_list->tail = NULL;
    }else  {
        *l_node = NULL;
    }
}

void free_node(bt_log_node_t *l_node)
{
    free(l_node->log_str);
    free(l_node);
    l_node = NULL;
}
