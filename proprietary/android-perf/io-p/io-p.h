/******************************************************************************
  @file    io-p.h
  @brief   Header file for communication and actions for iop

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2011,2014-2015,2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef __IO_P_H__
#define __IO_P_H__

#define IOP_CMD_PERFLOCK_IOPREFETCH_START 1
#define IOP_CMD_PERFLOCK_IOPREFETCH_STOP 2
#define UXENGINE_CMD_PERFLOCK_EVENTS 3
#define UXENGINE_CMD_PERFLOCK_TRIGGER 4
#define PKG_LEN 1024

#ifdef __cplusplus
extern "C" {
#endif

#define FAILED                  -1
#define SUCCESS                 0
enum {
UXE_TRIGGER = 1,
UXE_EVENT_BINDAPP,
UXE_EVENT_DISPLAYED_ACT,
UXE_EVENT_KILL,
UXE_EVENT_GAME,
UXE_EVENT_SUB_LAUNCH,
UXE_EVENT_PKG_UNINSTALL,
UXE_EVENT_PKG_INSTALL,
UXE_TRIGGER_LIST_FAV_APPS,
UXE_ULMK_TRIGGER
};

typedef struct _iop_msg_t {
    int pid;
    char pkg_name[PKG_LEN];
    char code_path[PKG_LEN];
    int opcode;
    int lat;
    int cmd;
} iop_msg_t;

int  iop_server_init(void);
void iop_server_exit(void);
int  iop_server_submit_request(iop_msg_t *msg);
char * uxe_server_submit_request(iop_msg_t *msg);

#ifdef __cplusplus
}
#endif

#endif /* __IO_P_H__ */
