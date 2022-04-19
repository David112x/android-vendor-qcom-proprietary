/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>

#include "sock_comm.h"

sock_error_t guest_socket_open(guest_socket *socket_handle)
{
    sock_error_t ret = SOCK_E_SUCCESS;

    *socket_handle = socket(AF_MSM_IPC, SOCK_DGRAM, 0);
    if (*socket_handle == -1) {
        printf("failed to create socket ret=%d errno=%s(%d)\n",
               ret, strerror(errno), errno);
        ret = SOCK_E_FAIL;
    }

    return ret;
}

sock_error_t guest_socket_close(guest_socket *socket_handle)
{
    sock_error_t ret = SOCK_E_SUCCESS;

    if (close(*socket_handle) != 0) {
        printf("failed to close socket ret=%d errno=%s(%d)\n",
               ret, strerror(errno), errno);
        ret = SOCK_E_FAIL;
    }

    return ret;
}

void guest_socket_set_addr_by_name(guest_sockaddr *sockaddr,
                                   uint32_t service_id,
                                   uint32_t instance_id)
{
    memset(sockaddr, 0, sizeof(guest_sockaddr));
    sockaddr->family = AF_MSM_IPC;
    sockaddr->address.addrtype = MSM_IPC_ADDR_NAME;
    sockaddr->address.addr.port_name.service = service_id;
    sockaddr->address.addr.port_name.instance = instance_id;
}

sock_error_t guest_socket_bind(guest_socket *socket_handle,
                               guest_sockaddr *local_addr)
{
    sock_error_t ret = SOCK_E_SUCCESS;

    if (bind(*socket_handle, (struct sockaddr *) local_addr,
             sizeof(guest_sockaddr)) != 0) {
        printf("failed to bind socket ret=%d errno=%s(%d)\n",
                ret, strerror(errno), errno);
        ret = SOCK_E_FAIL;
    }

    return ret;
}

int guest_socket_recvfrom(guest_socket *socket_handle,
                          void *buffer,
                          int32_t buffer_size,
                          guest_sockaddr *remote_addr,
                          int32_t *recd_size)
{
    int ret = 0;
    guest_sockaddr_len_type size_of_addr = sizeof(guest_sockaddr);

    ret = recvfrom(*socket_handle, buffer, buffer_size, 0,
                   (struct sockaddr *) remote_addr, &size_of_addr);
    if (ret < 0) {
        printf("failed to recvfrom socket ret=%d errno=%s(%d)\n",
               ret, strerror(errno), errno);
    } else {
        *recd_size = ret;
    }

    return ret;
}

int guest_socket_sendto(guest_socket *socket_handle,
                        void *buffer,
                        int32_t buffer_size,
                        guest_sockaddr *remote_addr,
                        int32_t *sent_size)
{
    int32_t bytes = 0;

    bytes = sendto(*socket_handle, buffer, buffer_size, 0,
                   (struct sockaddr *) remote_addr,
                   sizeof(guest_sockaddr));
    if (bytes < 0) {
        printf("failed to sendto socket ret=%d errno=%s(%d)\n",
                bytes, strerror(errno), errno);
    } else {
        *sent_size = bytes;
    }

    return bytes;
}
