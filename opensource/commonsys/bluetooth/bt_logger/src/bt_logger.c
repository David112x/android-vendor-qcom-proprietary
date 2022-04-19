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

#define LOG_TAG "bt_logger"

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
#include <signal.h>
#include <private/android_filesystem_config.h>
#include <android/log.h>


#include "bt_logger.h"

#define SOCKET_NAME "btloggersock"
#define BTLOGGER_PATH "/data/misc/bluetooth/logs"
#define VND_LOG_FILENAME "bt_vnd_log"
#define MAX_LOG_FILES        (3)
#define MAX_READ_LEN         (2048)
#define MAX_FILE_NAME_SIZE   (100)
#define PAYLOAD_OFFSET       (3)
#define MAX_LOG_STR_LEN      (512)
#define LEN_OFFSET           (1)
#define TYPE_OFFSET          (0)

// Re-run |fn| system call until the system call doesn't cause EINTR.
#define BTLOG_NO_INTR(fn)  do {} while ((fn) == -1 && errno == EINTR)
/*____________________________________________________________________
    CONSTANTS
   ____________________________________________________________________*/
typedef enum  {
    VENDOR_LOGGER_LOGS = 201,       // Signifies Packet containing Logger Log
    GENERATE_VND_LOG_SIGNAL,        // Signifies command to generate logs
    START_SNOOP_SIGNAL,
    STOP_SNOOP_SIGNAL,
    STOP_LOGGING_SIGNAL,
} CommandTypes;

/*____________________________________________________________________
    VARIABLES
   ____________________________________________________________________*/
static int server_socket = -1;
static int sock_client[MAX_SOCK_CONNECTIONS] = { -1 };
fd_set sock_fds;
static bt_log_buffer_t *log_list = NULL;
static short int looper;
static pthread_t dump_logs_t = -1;
static unsigned char pending_dump = 0;
static bt_log_buffer_t *log_list_dump = NULL;

/*____________________________________________________________________
    FUNCTIONS
   ____________________________________________________________________*/

void listen_data(void);
void process_incoming_data(bt_log_buffer_t *log_list, char *buff, int buff_len);
void dump_logs();
int process_packet(bt_log_buffer_t *log_list, char *l_data, short int pkt_len);
void dump_log_to_logcat(void);

int create_server_socket()
{
    FD_ZERO(&sock_fds);
    BTLOG_NO_INTR(server_socket = socket(AF_LOCAL, SOCK_STREAM, 0));
    if (server_socket < 0) {
        ALOGE("%s, socket creation failed", __func__);
        return -1;
    }

    FD_SET(server_socket, &sock_fds);

    if (socket_local_server_bind(server_socket, SOCKET_NAME,
                     ANDROID_SOCKET_NAMESPACE_ABSTRACT) < 0) {
        ALOGE("%s, socket_local_server_bind failed: %s", __func__, strerror(errno));
        return -1;
    }

    if (listen(server_socket, MAX_SOCK_CONNECTIONS) < 0) {
        ALOGE("%s, listen failed", __func__);
        close(server_socket);
        return -1;
    }
    return 0;
}

static void SignalHandler(int sig)
{
    ALOGE("SignalHandler: Got the signal %d", sig);
    pending_dump = 1;
}

int main()
{
    int i = 0;

    ALOGE("main: logger process");
    /* set the file creation mask to allow read/write */
    umask(0111);

    // initialize signal handler
    signal(SIGUSR1, SignalHandler);

    if (create_server_socket()) {
        return -1;
    }

    ALOGE("main: socket created");
    listen_data();

    if (dump_logs_t != -1) {
        pthread_join(dump_logs_t, NULL);
    }
    // TODO free all entries
    close(server_socket);
    for (i = 0; i < MAX_SOCK_CONNECTIONS; i++) {
        if (sock_client[i] != -1) {
            close(sock_client[i]);
        }
    }
    return 0;
}

void listen_data(void)
{
    int max_fd = -1, retval, read_len, i, s_fd;
    struct sockaddr_un cliaddr;
    socklen_t addrlen;
    char buff[MAX_READ_LEN + 1];

    looper = 1;
    log_list = NULL;
    init_list(&log_list, BT_LOGGER_BUFFER_LIMIT);

    if (!log_list) {
        return;
    }
    while (looper) {

        FD_ZERO(&sock_fds);

        FD_SET(server_socket, &sock_fds);
        max_fd = server_socket;

        if (pending_dump) {
            dump_logs();
            pending_dump = 0;
        }

        for (i = 0; i < MAX_SOCK_CONNECTIONS; i++) {
            s_fd = sock_client[i];
            if (s_fd > 0) {
                FD_SET(s_fd, &sock_fds);
                if (s_fd > max_fd)
                    max_fd = s_fd;
            }
        }

        retval = select(max_fd + 1, &sock_fds, NULL, NULL, NULL);
        if ( retval < 0 ) {
            if (errno != EINTR) {
                ALOGE("%s, Select exit with error: %s", __func__, strerror(errno));
                goto cleanup;
            }else  {
                continue;
            }
        }

        if (FD_ISSET(server_socket, &sock_fds)) {
            int new_client;
            addrlen = sizeof(cliaddr);
            BTLOG_NO_INTR(new_client = accept(server_socket, (struct sockaddr*)&cliaddr, &addrlen));
            if (new_client == -1) {
                if (errno == EINVAL || errno == EBADF) {
                    ALOGE("%s, Logger Process: Error while connecting to client: %s", __func__, strerror(errno));
                    break;
                }
                // print error here
                continue;
            }
            for (i = 0; i < MAX_SOCK_CONNECTIONS; i++) {
                if (sock_client[i] == -1) {
                    ALOGE("%s, Logger Process: Client socket connected %d", __func__, new_client);
                    sock_client[i] = new_client;
                    if (new_client > max_fd)
                        max_fd = new_client;
                    break;
                }
            }
            if ( i == MAX_SOCK_CONNECTIONS) {
                close(new_client);
            }
            continue;
        }

        for (i = 0; i < MAX_SOCK_CONNECTIONS; i++) {
            s_fd = sock_client[i];
            if (FD_ISSET(s_fd, &sock_fds)) {
                BTLOG_NO_INTR(read_len = recv(s_fd, buff, MAX_READ_LEN, 0));
                if ( read_len > 0) { // comment : intr error code
                    process_incoming_data(log_list, buff, read_len);
                }else  {
                    ALOGE("%s, Error Reading Data", __func__);
                    close(s_fd);
                    close(server_socket);
                    sock_client[i] = -1;
                    dump_log_to_logcat();
                    looper = 0;
                }
            }
        }
    }

 cleanup:
    return;
}

int process_packet(bt_log_buffer_t *log_list, char *l_data, short int pkt_len)
{
    int ret = 0;

    // process pending data if any

    switch (l_data[TYPE_OFFSET]) {
    case VENDOR_LOGGER_LOGS:
    {
        unsigned short int log_len;
        bt_log_node_t *log_node = NULL;
        size_t log_size;

        if(pkt_len < PAYLOAD_OFFSET) {
            ALOGE("Logger Process: Invalid packet with no length field");
            break;
        }

        log_len = *(( unsigned short*)(&l_data[LEN_OFFSET]));
        if (log_len == 0 || log_len > MAX_LOG_STR_LEN) {
            ALOGE("Logger Process: log_len = %d, returning", log_len);
            break;
        }

        create_log_node(log_list, &log_node);
        if (!log_node)
            break;

        log_size = add_data_to_node(log_list, log_node, &l_data[PAYLOAD_OFFSET], log_len);
        // handle the error cases
        if (!log_size) {
            if (log_node)
                free(log_node);
            break;
        }

        add_node_to_list(log_list, log_node, log_size);

        ret = log_len + PAYLOAD_OFFSET;
    }
    break;

    case GENERATE_VND_LOG_SIGNAL:
        dump_logs();
        ret = 1;
        break;

    case START_SNOOP_SIGNAL:
        start_snoop_logging();
        ret = 1;
        break;

    case STOP_SNOOP_SIGNAL:
        stop_snoop_logging();
        ret = 1;
        break;

    case STOP_LOGGING_SIGNAL:
        looper = 0;
        ret = 1;
        break;
    }
    return ret;
}

void process_incoming_data(bt_log_buffer_t *log_list, char *buff, int buff_len)
{
    int proc_len = 0;

    buff[buff_len] = '\0';

    proc_len = append_recovered_data_to_tail(log_list, buff);

    while (buff_len > proc_len) {
        int len = 0;
        len = process_packet(log_list, &buff[proc_len], buff_len - proc_len);

        if (len == 0) {
            ALOGE("Error saving packet, buff = %s", buff);
            break;
        }

        proc_len += len;
    }
}

int vnd_log_file_name(char *log_file_name)
{
    struct tm *tmp;
    time_t t;
    char time_string[64];
    struct DIR* p_dir;
    struct dirent* p_dirent;
    unsigned short int vndlog_file_count = 0, old_file_index = 0;
    char file_name[MAX_LOG_FILES][MAX_FILE_NAME_SIZE];

    p_dir = opendir(BTLOGGER_PATH);
    if (p_dir == NULL) {
        ALOGE("Unable to open the Dir %s", BTLOGGER_PATH);
#ifdef LOGGER_USERDEBUG
        goto file_creation;
#endif
        return -1;
    }

    while ((p_dirent = readdir(p_dir)) != NULL) {
        short int ret;

        if ((ret = strncmp(p_dirent->d_name, VND_LOG_FILENAME, strlen(VND_LOG_FILENAME))) == 0) {
            vndlog_file_count++;
        }

        if (vndlog_file_count > MAX_LOG_FILES) {
            ALOGE("Error More than %d BT vnd log files found, ABORT!", MAX_LOG_FILES);
            return -1;
        }else if (!ret) {
            strlcpy(file_name[vndlog_file_count - 1], p_dirent->d_name, MAX_FILE_NAME_SIZE);
            if (old_file_index != (vndlog_file_count - 1) && strncmp(file_name[vndlog_file_count - 1], file_name[old_file_index], MAX_FILE_NAME_SIZE) < 0) {
                old_file_index = vndlog_file_count - 1;
            }
        }
    }
    closedir(p_dir);

    if (vndlog_file_count == MAX_LOG_FILES) {
        char del_file[MAX_FILE_NAME_SIZE];
        snprintf(del_file, MAX_FILE_NAME_SIZE, BTLOGGER_PATH "/%s", file_name[old_file_index]);
        ALOGE("Deleting old log file %s", del_file);
        unlink(del_file);
    }

file_creation:
    t = time(NULL);
    tmp = localtime(&t);
    if (tmp == NULL) {
        ALOGE("Error : get localtime");
        return -1;
    }

    if (strftime(time_string, 64, "%Y%m%d%H%M%S", tmp) == 0) {
        ALOGE("Error : strftime :");
        return -1;
    }
    snprintf(log_file_name, MAX_FILE_NAME_SIZE, BTLOGGER_PATH "/%s%s.txt", VND_LOG_FILENAME, time_string);
    return 0;
}

void *log_dump_thread(void *param)
{
    int fd = -1, ret = -1;
    bt_log_node_t *l_node = NULL;
    char file_name[MAX_FILE_NAME_SIZE];
    bt_log_buffer_t * log_list = (bt_log_buffer_t*)param;

    if (!log_list || log_list->head == NULL) {
        ALOGE("No Log to Dump to file");
        goto exit;
        return NULL;
    }

    if (vnd_log_file_name(file_name)) {
        ALOGE("Error in creating file name");
        goto exit;
        return NULL;
    }

    BTLOG_NO_INTR(fd = open(file_name, \
                O_WRONLY | O_CREAT | O_TRUNC, \
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH));

    if (fd <= -1) {
        ALOGE("Unable to open log file fd:: %d ---> file name is :: %s --> Error :: %s", fd, file_name, strerror(errno));
        return NULL;
    }
    ALOGE("Writing logs to file %d ---> file name is %s", fd, file_name);

    while (log_list->head) {
        pop_head_node(log_list, &l_node);
        BTLOG_NO_INTR(ret = write(fd, l_node->log_str, strlen(l_node->log_str)));
        BTLOG_NO_INTR(ret = write(fd, "\n", 1));
        free_node(l_node);
    }

    free(log_list);
    log_list = NULL;
    close(fd);
 exit:
    dump_logs_t = -1;
    return NULL;
}

void dump_logs()
{
    log_list_dump = log_list;
    if (dump_logs_t != -1) {
        ALOGE("Dump thread is already running, return");
        return;
    }
    if (pthread_create(&dump_logs_t, NULL, log_dump_thread, ( void*)log_list_dump)) {
        ALOGE("ERROR creating log dump thread");
    }else  {
        log_list = NULL;
        init_list(&log_list, BT_LOGGER_BUFFER_LIMIT);
    }
}

void dump_log_to_logcat()
{
    bt_log_node_t *l_node = NULL;

    if (!log_list) {
        ALOGE("No Log to Dump to file");
        return;
    } else if ( !log_list->head) {
        free(log_list);
        log_list = NULL;
        ALOGE("No Log to Dump to file");
        return;
    }

    while (log_list->head) {
        pop_head_node(log_list, &l_node);
        ALOGE("%s", l_node->log_str);
        free_node(l_node);
    }
    free(log_list);
    log_list = NULL;
}
