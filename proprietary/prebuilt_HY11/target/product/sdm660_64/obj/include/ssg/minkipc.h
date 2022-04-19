/********************************************************************
 Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/
#ifndef __MINKIPC_H
#define __MINKIPC_H

#include <sys/types.h>
#include <sys/stat.h>
#include "object.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct MinkIPC MinkIPC;

/** service: A valid UNIX domain socket (file/abstract) name
    opener : A mink object which is given out when a connection
             is made using MinkIPC_connect
**/
MinkIPC * MinkIPC_startService(const char *service, Object opener);


/** sock: A valid bound UNIX domain socket
    opener : A mink object which is given out when a connection
             is made using MinkIPC_connect
**/

MinkIPC * MinkIPC_startServiceOnSocket(int sock, Object opener);

/** service: A valid UNIX domain socket (file/abstract) name
    opener : An out pointer to a mink object which'll be
             filled on successful connection
**/
MinkIPC * MinkIPC_connect(const char *service, Object *opener);


/** uid: out param - contains client user ID
    pid: out param - contains client group ID
    on success, the function returns 0

    This function is DEPRECATED. The client identity will be inherently
    availalbe to the primordial object in future.
**/
int MinkIPC_getClientInfo(uid_t* uid, gid_t* gid);

/**
   wait for the service to finish ..
   waits until stopped or the service dies
**/
void MinkIPC_join(MinkIPC *me);


/**
   Increment reference count to keep the object live.
**/
void MinkIPC_retain(MinkIPC *me);

/**
   Decrement reference count.
   When the count goes to 0, *me* is deleted.
**/
void MinkIPC_release(MinkIPC *me);

/**
   Wrap an fd into a Object.
   This takes ownership of the Fd.
   The caller must relinquish control
   of the descriptor after calling this method.

   int fd: The fd to wrap
   Object* obj: The Obj that represents the fd
**/
void MinkIPC_wrapFd(int fd, Object *obj);

#if defined (__cplusplus)
}
#endif

#endif //__MINKIPC_H
