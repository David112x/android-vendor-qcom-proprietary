/******************************************************************************
  @file    qsocket_ipcr.c
  @brief   IPC Router protocol specific implementation

  ---------------------------------------------------------------------------
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
 *******************************************************************************/
#include <stdlib.h>
#include "qsocket_ipcr.h"

int ipcr_find_name(int fd, ipcr_name_t *name, struct qsockaddr_ipcr *addrs,
                   unsigned int *instances, unsigned int *num_entries,
                   unsigned int flags)
{
  uint32_t num_servers_found = 0;
  uint32_t num_entries_to_fill = 0;
  struct server_lookup_args *lookup_arg;
  int i;

  if (fd < 0)
    return -QEBADF;

  if (num_entries)
  {
    num_entries_to_fill = *num_entries;
    *num_entries = 0;
  }

  lookup_arg = (struct server_lookup_args *)malloc(sizeof(*lookup_arg)
                        + (num_entries_to_fill * sizeof(struct msm_ipc_server_info)));
  if (!lookup_arg)
    return -QENOMEM;

  lookup_arg->port_name.service = name->service;
  if (flags == IPCR_FLAGS_ANY_INSTANCE)
  {
    lookup_arg->port_name.instance = 0;
    lookup_arg->lookup_mask = 0;
  } else
  {
    lookup_arg->port_name.instance = name->instance;
    lookup_arg->lookup_mask = 0xFFFFFFFF;
  }
  lookup_arg->num_entries_in_array = num_entries_to_fill;
  lookup_arg->num_entries_found = 0;
  if (ioctl(fd, IPC_ROUTER_IOCTL_LOOKUP_SERVER, lookup_arg) < 0) {
    free(lookup_arg);
    return -errno;
  }

  for (i = 0; ((i < (int)num_entries_to_fill) && (i < lookup_arg->num_entries_found)); i++) {
    (addrs + i)->family = AF_MSM_IPC;
    (addrs + i)->address.addrtype = MSM_IPC_ADDR_ID;
    (addrs + i)->address.addr.port_addr.node_id = lookup_arg->srv_info[i].node_id;
    (addrs + i)->address.addr.port_addr.port_id = lookup_arg->srv_info[i].port_id;
    if (instances)
      *(instances + i) = lookup_arg->srv_info[i].instance;
  }
  if (num_entries)
    *num_entries = i;
  num_servers_found = lookup_arg->num_entries_found;
  free(lookup_arg);
  return num_servers_found;
}

