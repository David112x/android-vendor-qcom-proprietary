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

/******************************************************************************
 *
 *  Filename:      bt_vendor_logc.c
 *
 *  Description:   Bluetooth logger client
 *
 ******************************************************************************/

#define LOG_TAG "bt_loglib"

#include <utils/Log.h>
#include <cutils/properties.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include "bt_logger_lib.h"
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <linux/un.h>
#include <sys/poll.h>

#define SOCKET_NAME "btloggersock"

#define CONN_MAX_TRIALS 2
#define TIME_STR_SIZE 13

#define VND_PKT_SIZE_BLOCKS 2
#define VND_PKT_HEADER_SIZE 3   //(VND_PKT_SIZE_BLOCKS + 1)
#define VND_PKT_BODY_SIZE 1021
#define VND_LOG_BUFF_SIZE 1024  // (VND_PKT_HEADER_SIZE+VND_PKT_BODY_SIZE)
#define VND_LOG_INFO_SIZE 6     // 5 bytes tid + 1

#define DBG 1

// Re-run |fn| system call until the system call doesn't cause EINTR.
#define BTLOG_NO_INTR(fn) do {} while ((fn) == -1 && errno == EINTR)

/******************************************************************************
**  Externs
******************************************************************************/


/******************************************************************************
**  Variables
******************************************************************************/
int client_sock;
uint16_t logging_level;
uint16_t logcat_capture_enabled;

/******************************************************************************
**  Local type definitions
******************************************************************************/


/******************************************************************************
**  Functions
******************************************************************************/
static int cleanup(void);

void lib_log(const char *fmt_str, ...)
{
    if (DBG) {
        static char buff[1024];
        va_list ap;

        va_start(ap, fmt_str);
        vsnprintf(buff, 1024, fmt_str, ap);
        va_end(ap);

        ALOGE("%s", buff);
    }
}


int connect_to_logger_server(void)
{
    struct sockaddr_un serv_addr;
    struct timeval sock_timeout;
    int sock, ret = -1, i, addr_len;

    BTLOG_NO_INTR(sock = socket(AF_LOCAL, SOCK_STREAM, 0));
    if (sock < 0) {
        lib_log("%s, client socket creation failed: %s\n", __func__, strerror(errno));
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_LOCAL;
    strlcpy(&serv_addr.sun_path[1], SOCKET_NAME, strlen(SOCKET_NAME) + 1);
    addr_len =  strlen(SOCKET_NAME) + 1;
    addr_len += sizeof(serv_addr.sun_family);

    for (i = 0; (ret < 0) && (i < CONN_MAX_TRIALS); i++) {
        BTLOG_NO_INTR(ret = connect(sock, (struct sockaddr*)&serv_addr, addr_len));
        if (ret < 0) {
            lib_log("%s, failed to connect to logger process: %s\n", __func__, strerror(errno));
            usleep(100000);
        }
    }
    sock_timeout.tv_sec = 0;
    sock_timeout.tv_usec = 50000;
    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &sock_timeout, sizeof(sock_timeout)) < 0) {
        lib_log("%s, failed to set timeout value: %s\n", __func__, strerror(errno));
        ret = -1;
    }

    if (ret < 0) {
        close(sock);
        return -1;
    }

    lib_log("%s, Connected to logger process, socket fd: %d", __func__, sock);
    return sock;
}

static void send_log(char *buff, int buff_size)
{
    int ret = 0;
    static int send_retry = 0;

    if (client_sock < 0) {
        return;
    }

    BTLOG_NO_INTR(ret = send(client_sock, buff, buff_size, 0));
    if ( ret < 0 && errno == ECONNRESET) {
        lib_log("%s, Connection closed", __func__);
        close(client_sock);
        return;
    } else if( ret < 0 && errno == EAGAIN ) {
        lib_log("%s, Socket write error: %s\n", __func__, strerror(errno));
        send_retry++;
        if(send_retry == 20) {
            lib_log("%s, Closing socket", __func__);
            cleanup();
        }
        return;
    }

    send_retry = 0;
    return;
}

unsigned short int format_time_str(char *curr_time)
{
    struct timeval tval;
    unsigned short int t_size, t_hh, t_mm, t_ss;

    gettimeofday(&tval, NULL);
    t_hh = tval.tv_sec / 3600;
    tval.tv_sec = tval.tv_sec % 3600;
    t_mm = tval.tv_sec / 60;
    t_ss = tval.tv_sec % 60;

    t_size = snprintf(curr_time, TIME_STR_SIZE, "%.2d:%.2d:%.2d:%.3d", t_hh, t_mm, t_ss, (int)(tval.tv_usec / 1000));
    return t_size;
}

unsigned short int create_log_info(const char *tag, char *buff)
{
    pid_t tid;
    unsigned short int tag_size, time_size, info_size;
    char time_str[TIME_STR_SIZE];

    time_size = format_time_str(time_str);

    tag_size = strlen(tag) + 2;
    tid = gettid();

    info_size = snprintf(&buff[VND_PKT_HEADER_SIZE], VND_LOG_INFO_SIZE + tag_size + time_size, "%s\t%d\t%s:\t", time_str, tid, tag) - 1;
    return info_size;
}

/*****************************************************************************
**
**   BLUETOOTH VENDOR LOGGER LIBRARY FUNCTIONS
**
*****************************************************************************/

/*******************************************************************************
**
** Function         init
**
** Description      First function to be called after linking to the library.
**                  Modules call this function to initiate logging with
**                  Bluetooth Logger Process.
**                  It open a client socket and connects to server socket of
**                  Logger process.
**
** Parameters:      Void

** Returns          Bluetooth logging level set in the device.
**
*******************************************************************************/
static int init(void)
{
    lib_log("%s", __func__);
    logging_level = property_get_int32("persist.vendor.bt_logger.log_mask", 0xFFFF);

    client_sock = connect_to_logger_server();
    if (client_sock < 0)
        return -1;

    logcat_capture_enabled = logging_level&DYNAMIC_LOGCAT_CAPTURE;
    return logging_level;
}

/*******************************************************************************
**
** Function         send_log
**
** Description      It is called to send logs to Logger process.
**                  It formats the log string received and writes to the socket
**                  to Bluetooth Logger Process.
**
** Parameters:      String

** Returns          void
**
*******************************************************************************/
static void send_log_msg(const char *tag, const char *fmt_str, va_list ap)
{
    int info_size, log_size = 0;
    char buffer[VND_LOG_BUFF_SIZE];

    if(!logcat_capture_enabled)
        return;

    buffer[0] = VENDOR_LOGGER_LOGS;

    info_size = create_log_info(tag, buffer);

    if (info_size + VND_PKT_HEADER_SIZE < VND_LOG_BUFF_SIZE)
        log_size = vsnprintf(&buffer[VND_PKT_HEADER_SIZE + info_size], VND_PKT_BODY_SIZE - info_size, fmt_str, ap);

    log_size += info_size;
    *(( unsigned short*)(&buffer[1])) = log_size;

    send_log(buffer, VND_PKT_HEADER_SIZE + log_size);

    return;
}

static void send_log_data(const char *tag, const char *fmt_str, ...)
{
    va_list ap;

    va_start(ap, fmt_str);
    send_log_msg(tag, fmt_str, ap);
    va_end(ap);

    return;
}

static void send_event(char evt)
{
    send_log(&evt, 1);
}

/*******************************************************************************
**
** Function         cleanup
**
** Description      It is called during cleanup after BT is turned Off
**                  Will send the safe shutdown packet to BT Logger process
**                  and then close the client connection with server socket.
**
** Parameters:      Void

** Returns          true if socket connection is closed successfully, false
**                  otherwise.
**
*******************************************************************************/
static int cleanup(void)
{
    lib_log(__func__);
    if (client_sock > 0) {
        send_event(STOP_LOGGING_SIGNAL);
        close(client_sock);
        client_sock = -1;
    }
    return true;
}

const bt_logger_interface_t BLUETOOTH_LOGGER_LIB_INTERFACE = {
    sizeof(bt_logger_interface_t),
    init,
    send_log_msg,
    send_log_data,
    send_event,
    cleanup
};
