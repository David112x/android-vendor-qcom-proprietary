/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/types.h>

#include "sock_comm.h"

#define SCLOGE(fmt, ...) ALOGE("%s - " fmt, __FUNCTION__, ##__VA_ARGS__)
#define SCLOGW(fmt, ...) ALOGW("%s - " fmt, __FUNCTION__, ##__VA_ARGS__)
#define SCLOGI(fmt, ...) ALOGI("%s - " fmt, __FUNCTION__, ##__VA_ARGS__)
#define SCLOGD(fmt, ...) ALOGD("%s - " fmt, __FUNCTION__, ##__VA_ARGS__)

sock_error_t seccam_socket_open(seccam_socket *socket_handle)
{
    sock_error_t ret = SOCK_E_SUCCESS;

    *socket_handle = socket(AF_QIPCRTR, SOCK_DGRAM, 0);
    if (*socket_handle == -1) {
        SCLOGE("failed to create socket errno=%s(%d)\n",
               strerror(errno), errno);
        ret = SOCK_E_FAIL;
    }

    return ret;
}

sock_error_t seccam_socket_close(seccam_socket *socket_handle)
{
    sock_error_t ret = SOCK_E_SUCCESS;

    if (close(*socket_handle) != 0) {
        SCLOGE("failed to close socket errno=%s(%d)\n",
               strerror(errno), errno);
        ret = SOCK_E_FAIL;
    }
    return ret;
}

void seccam_socket_init_addr(seccam_sockaddr *sockaddr)
{
    memset(sockaddr, 0, sizeof(seccam_sockaddr));
    sockaddr->sq_family = AF_QIPCRTR;
}

int seccam_socket_recvfrom(seccam_socket *socket_handle,
                           void *buffer, int32_t buffer_size,
                           seccam_sockaddr *remote_addr,
                           int32_t *recd_size)
{
    int ret = 0;
    struct pollfd fd;

    fd.fd = *socket_handle;
    fd.events = POLLIN;
    ret = poll(&fd, 1, 1000);
    if (ret > 0) {
       while (1) {
           *recd_size = recv(*socket_handle, buffer, buffer_size, 0);
           if (*recd_size < 0 && errno == EINTR ) {
               SCLOGE("recv errno=%s(%d)\n", strerror(errno), errno);
               continue;
           }
           break;
       }
    } else {
       SCLOGE("failed to recv data on socket ret=%d(%s) errno=%s(%d)\n",
              ret, (ret) ? "ERROR" : "TIMEOUT", strerror(errno), errno);
    }

    return ret;
}

int seccam_socket_sendto(seccam_socket *socket_handle,
                         void *buffer, int32_t buffer_size,
                         seccam_sockaddr *remote_addr,
                         int32_t *sent_size)
{
    int32_t bytes = 0;

    bytes = sendto(*socket_handle, buffer, buffer_size, 0,
                   (struct sockaddr *) remote_addr,
                   sizeof(seccam_sockaddr));
    if (bytes == -1) {
        SCLOGE("failed to sendto socket ret=%d errno=%s(%d)\n",
               bytes, strerror(errno), errno);
    } else{
        *sent_size = bytes;
    }

    return bytes;
}

int seccam_socket_lookup(seccam_socket fd,
                         seccam_sockaddr *addrs)
{
    seccam_sock_pkt pkt;
    int status = 0;
    int ret = -1;
    seccam_sockaddr_len_type size_of_addr = sizeof(seccam_sockaddr);
    unsigned int service = MLAPP_SERVICE_ID;
    unsigned int instance = 0;
    unsigned int version = 0;
    unsigned int node = 0;
    unsigned int port = 0;
    unsigned int len = 0;

    status = getsockname(fd, (struct sockaddr *) addrs, &size_of_addr);
    if (status < 0) {
        printf("failed to getsockname status=%d errno=%s(%d)\n",
                status, strerror(errno), errno);
        goto EXIT;
    }

    addrs->sq_port = QRTR_PORT_CTRL;

    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd = cpu_to_le32(QRTR_TYPE_NEW_LOOKUP);
    pkt.server.service = cpu_to_le32(service);
    pkt.server.instance = cpu_to_le32(1);

    status = sendto(fd, &pkt, sizeof(pkt), 0, (struct sockaddr *) addrs, sizeof(seccam_sockaddr));
    if (status < 0) {
        SCLOGE("failed to send lookup cmd status=%d errno=%s(%d)\n",
               status, strerror(errno), errno);
        goto EXIT;
    }

    memset(&pkt, 0, sizeof(pkt));
    while ((len = recv(fd, &pkt, sizeof(pkt), 0)) > 0) {
        unsigned int type = le32_to_cpu(pkt.cmd);

        if (len < sizeof(pkt) || type != QRTR_TYPE_NEW_SERVER) {
            SCLOGE("seccam_socket_lookup received invalid/short packet");
            continue;
        }

        if (!pkt.server.service && !pkt.server.instance &&
            !pkt.server.node && !pkt.server.port) {
            break;
        }

        service = le32_to_cpu(pkt.server.service);
        version = le32_to_cpu(pkt.server.instance) & 0xff;
        instance = le32_to_cpu(pkt.server.instance) >> 8;
        node = le32_to_cpu(pkt.server.node);
        port = le32_to_cpu(pkt.server.port);

        SCLOGD("found service: [%d:%x]@[%d:%d]\n",
               service, version, node, port);
    }

    if (node && port) {
        addrs->sq_node = node;
        addrs->sq_port = port;
        ret = 0;
    }

EXIT:
    return ret;
}
