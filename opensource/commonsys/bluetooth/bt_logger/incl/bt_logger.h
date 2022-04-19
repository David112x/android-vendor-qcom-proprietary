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

#define SOCKET_NAME "btloggersock"
#define MAX_SOCK_CONNECTIONS 4
#define BT_LOGGER_BUFFER_LIMIT 4250000

typedef struct bt_log_node
{
    char *log_str;
    struct bt_log_node *nxt_node;
} bt_log_node_t;

typedef struct
{
    bt_log_node_t *head;
    bt_log_node_t *tail;
    size_t buff_size;
    size_t max_buff_size;
    unsigned short pending_pkt_len;
    unsigned short saved_log_len;
} bt_log_buffer_t;

void start_snoop_logging();
void stop_snoop_logging ();
void pop_head_node(bt_log_buffer_t *log_list, bt_log_node_t **l_node);
void free_node(bt_log_node_t *l_node);
size_t append_recovered_data_to_tail(bt_log_buffer_t *log_list, char *buff);
void create_log_node(bt_log_buffer_t *log_list, bt_log_node_t **ret_node);
size_t add_data_to_node(bt_log_buffer_t *log_list, bt_log_node_t *l_node, char *l_data, int log_len);
void add_node_to_list (bt_log_buffer_t *log_list, bt_log_node_t *log_node, size_t log_size);
void init_list ( bt_log_buffer_t **list, size_t list_max_size );
