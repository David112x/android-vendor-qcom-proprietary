/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
=============================================================

                          EDIT HISTORY FOR FILE

when       who     what, where, why
--------   ---     ------------------------------------------
07/09/18   ac      Initial version
=============================================================*/

#ifndef SOCK_COMM_H
#define SOCK_COMM_H

#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <linux/qrtr.h>
#include <endian.h>
#include <stdint.h>
#include <linux/types.h>

static inline __le32 cpu_to_le32(uint32_t x) { return htole32(x); }
static inline uint32_t le32_to_cpu(__le32 x) { return le32toh(x); }

// TODO: This is temp service id
#define MLAPP_SERVICE_ID 7000

typedef enum {
    SOCK_E_SUCCESS            = 0,    /**< Success. */
    SOCK_E_FAIL               = 1,    /**< General error. */
    /** @endcond */
} sock_error_t;

typedef int seccam_socket;
typedef struct sockaddr_qrtr seccam_sockaddr;
typedef struct qrtr_ctrl_pkt seccam_sock_pkt;
typedef socklen_t seccam_sockaddr_len_type;
typedef struct pollfd seccam_socket_pollfd;

sock_error_t seccam_socket_open(seccam_socket *socket_handle);

sock_error_t seccam_socket_close(seccam_socket *socket_handle);

void seccam_socket_init_addr(seccam_sockaddr *sockaddr);

int seccam_socket_recvfrom(seccam_socket *socket_handle,
                           void *buffer,
                           int32_t buffer_size,
                           seccam_sockaddr *remote_addr,
                           int32_t *recd_size);

int seccam_socket_sendto(seccam_socket *socket_handle,
                         void *buffer,
                         int32_t buffer_size,
                         seccam_sockaddr *remote_addr,
                         int32_t *sent_size);

int seccam_socket_lookup(seccam_socket fd,
                         seccam_sockaddr *addrs);
#endif
