/* mct_debug_fdleak.cpp
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#include <dlfcn.h>
#include <stdio.h>
#include <ctype.h>
#include <unwind.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <cutils/properties.h>

#include <iostream>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include "camera_dbg.h"
#include "memleak.h"
#include "mct_debug_fdleak.h"
#include <errno.h>

#define MAX_BACKTRACE_DEPTH 15
#define MAGIC_ALLOC 0x7abc0fb5
#define MAGIC_FREE 0x087cbc8a

struct fdlist_t {
  uintptr_t bt[MAX_BACKTRACE_DEPTH];
  int bt_depth;
  int fd;
  struct fdlist_t *next;
  int allocated;
} __attribute__((packed));
typedef struct fdlist_t fdlist_t;

struct map_info_holder {
  char *name;
  struct map_info_holder* next;
  uintptr_t start;
  uintptr_t end;
};

struct stack_crawl_state_t {
  uintptr_t *addr;
  int skip;
  size_t stack_count;
  size_t max_depth;
};

pthread_mutex_t mct_debug_fdleak_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t mct_debug_fdleak_con = PTHREAD_COND_INITIALIZER;
char mct_debug_fdleak_event;
static unsigned int fdleak_count = 0;
static fdlist_t *head = NULL;

typedef int (*real_open_type)(const char *, int, ...);
typedef int (*real_pipe_type)(int *);
typedef int (*real_socket_type)(int, int, int);
typedef void* (*real_mmap_type)(void*, size_t, int, int, int, off_t);
typedef int (*real_close_type)(int);

static real_open_type __real_open = NULL;
static real_pipe_type __real_pipe= NULL;
static real_socket_type __real_socket= NULL;
static real_mmap_type __real_mmap= NULL;
static real_close_type __real_close= NULL;

static inline void add(int fd_value)
{
  fdlist_t *fdlist;
  fdlist = (fdlist_t *) malloc(sizeof(fdlist_t));
  if (!fdlist)
    return;
  fdlist->allocated = MAGIC_ALLOC;
  fdlist->bt_depth = 0;
  fdlist->fd = fd_value;

  pthread_mutex_lock(&mct_debug_fdleak_mut);
  fdlist->bt_depth = mmcamera_stacktrace(fdlist->bt, MAX_BACKTRACE_DEPTH);
  fdlist->next = head;
  head = fdlist;
  fdleak_count++;
  pthread_mutex_unlock(&mct_debug_fdleak_mut);
}

extern "C" void fdleak_dump_list()
{
  fdlist_t *del; int cnt, cnt_all = 0;
  struct map_info_holder *p_map_info;
  struct fdlist_t *temp;
  temp = head;
  pthread_mutex_lock(&mct_debug_fdleak_mut);
  p_map_info = lib_map_create(getpid());
  while (temp != NULL) {
    CLOGE(CAM_NO_MODULE,"leaked fd %d\n",temp->fd);
    print_backtrace(p_map_info, temp->bt, temp->bt_depth);
    temp = temp->next;
  }
  lib_map_destroy(p_map_info);
  pthread_mutex_unlock(&mct_debug_fdleak_mut);
  fdleak_count = 0;
}

int __open(const char* dev_name, int flags, ...)
{
  int fd_value;
  mode_t mode = 0;

  if ((flags & O_CREAT) != 0) {
    va_list args;
    va_start(args, flags);
    mode = static_cast<mode_t>(va_arg(args, int));
    va_end(args);
  }
  fd_value = open(dev_name, flags, mode);
  if (errno == EMFILE) {
    CLOGE(CAM_NO_MODULE, "FATAL during open %s Restart camera daemon !!!",strerror(errno));
    raise(SIGABRT);
    raise(SIGKILL);
  }
    if (fd_value > 0) {
      add( fd_value);
      return fd_value;
    }
 return fd_value;
}

int __pipe(int fd[])
{
  int ret_value;

  ret_value = pipe(fd);
  if (errno == EMFILE) {
    CLOGE(CAM_NO_MODULE, "FATAL during pipe creation %s Restart camera daemon !!!",strerror(errno));
    raise(SIGABRT);
    raise(SIGKILL);
  }
  if (ret_value >= 0) {
    add( fd[0]);
    add( fd[1]);
    return ret_value;
  }
  return ret_value;
}

int __socket(int domain, int type, int protocol)
{
  int ds_fd ;

  ds_fd = socket(domain, type, protocol);
  if (errno == EMFILE) {
    CLOGE(CAM_NO_MODULE, "FATAL during socket create %s Restart camera daemon !!!",strerror(errno));
    raise(SIGABRT);
    raise(SIGKILL);
  }
  if (ds_fd > 0) {
    add( ds_fd);
    return ds_fd;
  }
  return ds_fd;
}

void* __mmap(void* addr, size_t size, int prot, int flags, int fd, off_t offset)
{
  void* ret;

  ret = mmap(addr, size, prot, flags, fd, offset);
  if (errno == EMFILE) {
    CLOGE(CAM_NO_MODULE, "FATAL during mmap %s Restart camera daemon !!!",strerror(errno));
    raise(SIGABRT);
    raise(SIGKILL);
  }
  if (fd > 0) {
    add( fd);
    return ret;
  }
  return ret;
}

void delete_node(int fd_value)
{
  static fdlist_t *temp,*prev;
  temp = head;
  while (temp != NULL) {
    if (temp->fd != fd_value) {
      prev = temp;
      temp = temp->next;
    } else {
        fdleak_count--;
        if (temp == head) {
          head = temp->next;
          free(temp);
          return;
        }else {
           prev->next = temp->next;
          free(temp);
          return;
        }
    }
  }
}

static int __close(int fd_value)
{
  pthread_mutex_lock(&mct_debug_fdleak_mut);
  delete_node(fd_value);
  pthread_mutex_unlock(&mct_debug_fdleak_mut);
  return (close(fd_value));
}

extern "C" int __wrap_open(const char* dev_name, int flags, ...)
{
  mode_t mode = 0;

  if ((flags & O_CREAT) != 0) {
    va_list args;
    va_start(args, flags);
    mode = static_cast<mode_t>(va_arg(args, int));
    va_end(args);
  }
  return __real_open(dev_name, flags, mode);
}

extern "C" int  __wrap_pipe(int *fd)
{
  return __real_pipe(fd);
}

extern "C" int  __wrap_socket(int domain, int type, int protocol)
{
  return __real_socket(domain,type,protocol);
}

extern "C" void*  __wrap_mmap(void* addr, size_t size, int prot, int flags, int fd, off_t offset)
{
  return __real_mmap(addr, size, prot, flags, fd, offset);
}

extern "C" int  __wrap_close(int fd_value)
{
   return __real_close(fd_value);
}


static __attribute__((constructor)) void init(void)
{
  __real_open = open;
  __real_pipe = pipe;
  __real_socket = socket;
  __real_mmap = mmap;
  __real_close = close;
}
void mct_degug_enable_fdleak_trace()
{
  __real_open = __open;
  __real_pipe = __pipe;
  __real_socket = __socket;
  __real_mmap = __mmap;
  __real_close = __close;
}
void mct_degug_dump_fdleak_trace()
{

    __real_open = open;
    __real_pipe = pipe;
    __real_socket = socket;
    __real_mmap = mmap;
    __real_close = close;
    if (fdleak_count) {
      CLOGE(CAM_NO_MODULE,"FATAL fdleak found in camera daemon %d",
         fdleak_count);
      fdleak_dump_list();
    }
}
static __attribute__((destructor)) void finish(void)
{
  CLOGH(CAM_NO_MODULE,"fdleak lib deinit.\n");
  if (fdleak_count)
    fdleak_dump_list();
}
