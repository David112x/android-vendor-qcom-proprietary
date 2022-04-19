/**
  @file
  port_bridge.c

  @brief
  Main port_bridge application which bridges communication
  between an external host port and a modem port.
*/

/*===========================================================================
  Copyright (c) 2011-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>

#ifdef FEATURE_PB_LINUX_ANDROID
  #include <cutils/properties.h>
#endif

#include "ds_util.h"
#include "portbridge_sockif.h"

#define MAX_RETRY_COUNT     (5)
#define MAX_MSG_SIZE        (16 * 1024)
#define GLINK_RX_INTENT_MIN (1)
#define LOG_FILE_PATH         "/data/vendor/port_bridge/log.txt"
#define USB_FILE_PATH         ("/dev/at_usb0")
#define MDM_FILE_PATH_GLINK   ("/dev/at_mdm0")
#define MDM_FILE_PATH_SMD     ("/dev/at_mdm0")
#define MDM_FILE_PATH_MHI_START   ("/dev/mhi_")
#define MDM_FILE_PATH_MHI_END     ("pipe_32")
#define MHI_ESOC_FILE_PATH        ("/sys/bus/esoc/devices/esoc0/esoc_link_info")
#define MHI_CTRL_POLLING_INTERVAL (500 * 1000)
#define MAX_PATH_LEN              (255)
#define MAX_SOC_LINK_INFO_LEN     (60)
#define PORT_BRIDGE_UNIX_API_ERROR (-1)
#define PORT_BRIDGE_UNIX_MAX_CLIENTS (1)

/* target specific stuff */
struct pb_targetinfo
{
  const char *mdm_chardev;
  const char *usb_chardev;
  void * (*mdm_handler)(void *);
};

/* Seconds to sleep after an unsuccessful open before trying again
   If underlying driver supports blocking open (Ex. GLINK) then
   failure is rare. This is mainly for cases where blocking open
   is not supported */
#define OPEN_FAILURE_SLEEP_TIME (1)

/* Exponential back off sleep factor for write failure retries */
#define WRITE_FAILURE_SLEEP_TIME (1)

// These GLink #defines are not defined in any common header.
// MPROC has confirmed they will not change.
#define GLINK_PKT_IOCTL_MAGIC (0xC3)
#define GLINK_PKT_IOCTL_QUEUE_RX_INTENT \
  _IOW(GLINK_PKT_IOCTL_MAGIC, 0, unsigned int)

#define PORT_BRIDGE_LOG_ENABLED "persist.vendor.data.port_bridge.log"
#define PORT_BRIDGE_TRUE_STR    "true"
#define PORT_BRIDGE_FALSE_STR   "false"
boolean port_bridge_log_enabled = FALSE;

FILE* log_file = NULL;

// Log Info, Error, Result
#define LOGI(...)\
{\
  if(port_bridge_log_enabled) {\
    fprintf(log_file, "[thread %p] %30s,%3u (INFO):    ", (void*)pthread_self(), __FUNCTION__, __LINE__);\
    fprintf(log_file, __VA_ARGS__);\
    fflush(log_file);\
  } \
}
#define LOGE(...)\
{\
  if(port_bridge_log_enabled) {\
    fprintf(log_file, "[thread %p] %30s,%3u (ERROR):   ", (void*)pthread_self(), __FUNCTION__, __LINE__);\
    fprintf(log_file, __VA_ARGS__);\
    fflush(log_file);\
  } \
}
#define LOGR(errcode, ...)\
{\
  if(port_bridge_log_enabled) {\
    if(errcode < 0)\
    {\
      fprintf(log_file, "[thread %p] %30s,%3u (FAILURE - %s): ", (void*)pthread_self(), __FUNCTION__, __LINE__, strerror(errno));\
      fprintf(log_file, __VA_ARGS__);\
    }\
    else\
    {\
      fprintf(log_file, "[thread %p] %30s,%3u (SUCCESS): ", (void*)pthread_self(), __FUNCTION__, __LINE__);\
      fprintf(log_file, __VA_ARGS__);\
    }\
    fflush(log_file);\
  } \
}

#define PB_PTHREAD_MUTEX_LOCK( mutex, path )             \
  do {                                                   \
   LOGI("trying to lock [0x%p] %s\n", mutex, path)       \
   if(pthread_mutex_lock(mutex) < 0 )                    \
     LOGE("mutex lock failed [0x%p] %s\n",mutex, path);  \
   LOGI("locked [0x%p] %s\n", mutex, path);              \
  }while (0);

#define PB_PTHREAD_MUTEX_UNLOCK( mutex, path )           \
  do {                                                   \
   LOGI("trying to unlock [0x%p] %s\n", mutex,path)      \
   if(pthread_mutex_unlock(mutex) < 0 )                  \
     LOGE("mutex unlock failed [0x%p] %s\n",mutex, path);\
   LOGI("unlocked [0x%p] %s\n", mutex, path);            \
  }while(0);

#define PB_PTHREAD_STATE_MUTEX_LOCK( mutex, var )        \
  do {                                                   \
   LOGI("trying to lock [0x%p] %d\n", mutex, var)        \
   if(pthread_mutex_lock(mutex) < 0 )                    \
     LOGE("mutex lock failed [0x%p] %d\n",mutex, var);   \
   LOGI("locked [0x%p] %d\n", mutex, var);               \
  }while (0);

#define PB_PTHREAD_STATE_MUTEX_UNLOCK( mutex, var )      \
  do {                                                   \
   LOGI("trying to unlock [0x%p] %d\n", mutex,var)       \
   if(pthread_mutex_unlock(mutex) < 0 )                  \
     LOGE("mutex unlock failed [0x%p] %d\n",mutex, var); \
   LOGI("unlocked [0x%p] %d\n", mutex, var);             \
  }while(0);


const char* usb_file_path    = USB_FILE_PATH;
char mdm_file_path_mhi[MAX_PATH_LEN];

pthread_mutex_t usb_mutex   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mdm_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dual_mode_state_mutex  = PTHREAD_MUTEX_INITIALIZER;

int usb_fd = -1;
int mdm_fd = -1;
int client_fd = -1;

/* Design Note: Each direction of communication is handled by one thread.  This thread
   manages (opens & closes) the file descriptor of its 'from' (source), and does not manage
   (does not open/close) the file descriptor of its 'to' (destination). */
typedef struct
{
  char* from_file_path;
  char* to_file_path;
  int * from_fd;
  int * to_fd;
  pthread_mutex_t* from_mutex;
  pthread_mutex_t* to_mutex;
} thread_info_t;

typedef enum
{
  PORT_BRIDGE_TETHERED_MODE = 0,
  PORT_BRIDGE_SOCKET_MODE,
  PORT_BRIDGE_DUAL_MODE
}port_bridge_mode;

port_bridge_mode  mode_of_operation = PORT_BRIDGE_TETHERED_MODE;  //Default mode

typedef enum
{
  PB_TETHERED_STATE = 0,
  PB_SOCKET_STATE
}dual_mode_state;

dual_mode_state state_of_operation = PB_TETHERED_STATE;    //Default

/*===========================================================================
  FUNCTION  construct_signal_string
===========================================================================*/
/*!
@brief
  Utility function which formats a signal bitmask into human-redable format.

@param
  str   A char buffer to store the formatted string.
  size  Size of the buffer.  Limits output to this number of characters.
  bit   Bitmask to format.
*/
void construct_signal_string(char* str, size_t size, int bits)
{
  // Print any bits that aren't DTR/CD/RI/RTS as a mask
  unsigned int unaccounted_bits = bits & ~(TIOCM_DTR | TIOCM_CD | TIOCM_RI | TIOCM_RTS);

  // And print DTR/CD/RI/RTS in readable format
  snprintf(str, size - 1, "0x%x%s%s%s%s",
    unaccounted_bits,
    bits & TIOCM_DTR  ? " | DTR"  : "",
    bits & TIOCM_CD   ? " | CD"   : "",
    bits & TIOCM_RI   ? " | RI"   : "",
    bits & TIOCM_RTS  ? " | RTS"  : "");
}

/*===========================================================================
  FUNCTION  close_fd_mutexed
===========================================================================*/
/*!
@brief
  Wrapper function around close() that handles locking/unlocking a mutex associated
  the file descriptor.

@param
  file_path   Path of the file being closed.  Used for logging.
  fd          File descriptor to close.
  mutex       Mutex associated with fd.

@note
  Errors with mutex functions are only logged without exiting the program.  This
  is because a mutex lock or unlock failure is not necessarily fatal or even detrimental to
  the normal operation of port-bridge.
*/
void close_fd_mutexed(const char* file_path, volatile int* const fd, pthread_mutex_t* mutex)
{
  int ret;
  ret = pthread_mutex_lock(mutex);
  if(ret < 0)
  {
    LOGR(ret, "pthread_mutex_lock for '%s' returned %i.  Exiting!\n", file_path, ret);
    exit(1);
  }
  close(*fd);
  *fd = -1;
  ret = pthread_mutex_unlock(mutex);
  if(ret < 0)
  {
    LOGR(ret, "pthread_mutex_unlock for '%s' returned %i.  Exiting!\n", file_path, ret);
    exit(1);
  }
}

/*===========================================================================
  FUNCTION  write_to_device
===========================================================================*/
/*!
@brief
  Utility function to write data to the provided file

@param
  to_fd            File descriptor of the file where data needs
                   to be written
  to_filename      Name of the file where data needs to be written
  bytes_to_write   Number of bytes to be written

@return
  0  on success
  -1 on failure

@note
  In general, errors are logged but considered non-fatal.
*/
/*==========================================================================*/
int write_to_device
(
  int           to_fd,
  const char*   to_filename,
  int           bytes_to_write,
  char*         buffer
)
{
  int num_written           = 0;
  int bytes_remaining       = bytes_to_write;
  int retry_count           = 0;
  int ret = 0;

  /* In case we are writing a very large buffer to the file we need to ensure that
     the complete data was written correctly */
  while (bytes_remaining)
  {

#ifdef PORT_BRIDGE_TEST
    LOGI("usb[%s] to_path[%s]\n", USB_FILE_PATH, to_filename);
    if( !strncmp(USB_FILE_PATH, to_filename, sizeof(USB_FILE_PATH)) )
    {
      int r = rand();
      LOGI("rand [%d]\n",r);
      if( !(r%17) )
      {
        errno = 0;
        num_written = 0;
        LOGI("Setting num_written to [%d]\n", num_written);
      }
      else if( !(r%19) )
      {
        errno= EAGAIN;
        num_written = -1;
        LOGI("Setting num_written [%d] errno[%d]\n", num_written, errno);
      }
      else if( !(r%23))
      {
        errno = ESOCKTNOSUPPORT;
        num_written = -1;
        LOGI("Setting num_written [%d] errno[%d]\n", num_written, errno);
      }
      else
      {
        errno = 0;
        LOGI("[%s] writing to usb \n",to_filename);
        num_written = write(to_fd, buffer, bytes_remaining);
      }
    }
    else
#endif
    {
      num_written = write(to_fd, buffer, bytes_remaining);
    }

    if ( -1 == num_written || 0 == num_written )
    {
      /* We can retry writing if the error code is EAGAIN */
      if ( ( 0 == num_written || EAGAIN == errno ) &&
             MAX_RETRY_COUNT > retry_count )
      {
        LOGI("Failed to write to '%s' due to err [%s] or num_written [%d]\n",
              to_filename, strerror(errno), num_written);
        retry_count++;
        sleep( WRITE_FAILURE_SLEEP_TIME * retry_count );
        LOGI("retrying now...\n");
        continue;
      }
      else
      {
        /* We got a non-recoverable error */
        LOGE("Failed to write to '%s' due to err [%s]\n", to_filename, strerror(errno));
        ret = -1;
        break;
      }
    }

    LOGR(num_written, "write to '%s' returned %i\n", to_filename, num_written);
    bytes_remaining -= num_written;
    buffer += num_written;
    retry_count = 0;
  }

  return ret;
}

void send_dtr_to_modem (void* thread_info)
{
  int signals = 0;
  int ret;
  thread_info_t tinfo = *((thread_info_t*) thread_info);

  LOGI("Sending DTR low signal to MODEM\n");
  signals = signals & ~TIOCM_DTR;

  /* Set DTR signal to off and pass it to MODEM  */

  PB_PTHREAD_MUTEX_LOCK(tinfo.to_mutex, tinfo.to_file_path);
  ret = ioctl(*tinfo.to_fd, TIOCMSET, &signals);
  PB_PTHREAD_MUTEX_UNLOCK(tinfo.to_mutex, tinfo.to_file_path);

  LOGR(ret, "TIOCMSET on '%s' returned %i, signals=0x%x \n",
       tinfo.to_file_path, ret, signals);

}

/*===========================================================================
  FUNCTION  open_device_file
===========================================================================*/
/*!
@brief
  Utility function to open the provided file

@param
  tinfo            pointer to communication handling thread info struct
  pinfo            pointer to polling info struct
  modem_to_usb     communication direction is from modem to usb

@return
  void

@note
  In general, errors are logged but considered non-fatal.
*/
/*==========================================================================*/
void open_device_file
(
  thread_info_t* tinfo,
  struct pollfd* pinfo,
  boolean        modem_to_usb
)
{
  int temp_fd    = -1;

  while (*(tinfo->from_fd) < 0)
  {
    LOGI("Attempting to open '%s'...\n", tinfo->from_file_path);
    temp_fd = open(tinfo->from_file_path, O_RDWR); // Custom blocking open
    LOGR(temp_fd, "Blocking open on '%s' returned %i.\n",
         tinfo->from_file_path, temp_fd);
    if(temp_fd < 0)
    {
      sleep(OPEN_FAILURE_SLEEP_TIME);
      continue;
    }
    else
    {
      PB_PTHREAD_MUTEX_LOCK(tinfo->from_mutex, tinfo->from_file_path);
      *(tinfo->from_fd) = temp_fd;
      PB_PTHREAD_MUTEX_UNLOCK(tinfo->from_mutex, tinfo->from_file_path);
      // Complete success
      pinfo->fd = modem_to_usb ? mdm_fd : temp_fd;
    }
  }
}

/*===========================================================================
  FUNCTION  do_polling
===========================================================================*/
/*!
@brief
  Utility function to poll to signal

@param
  tinfo            pointer to communication handling thread info struct
  pinfo            pointer to polling info struct
  handle_pollpri   handle the pollpri signal in this function

@return
  0  on success
  -1 on failure

@note
  In general, errors are logged but considered non-fatal.
*/
/*==========================================================================*/
int do_polling
(
  thread_info_t* tinfo,
  struct pollfd* pinfo,
  boolean        handle_pollpri
)
{
  int    ret     = 0;
  int signals    = 0;
  portbridge_sock_intf_data_t mdm_to_sock_msg;
  int num_of_bytes_to_write     = 0 ;
  char signal_str_buf[MAX_MSG_SIZE + 1]; /* Just need a large enough buffer to
                                            hold a signal printout for debug */
poll_begin:
  pinfo->revents = 0;
  LOGI("Attempting to poll '%s'...\n", tinfo->from_file_path);
  ret = poll(pinfo, 1, -1); // -1 timeout specifies infinite block
  LOGR(ret, "poll on '%s' returned %i\n", tinfo->from_file_path, ret);

  if(pinfo->revents & POLLERR)
  {
    LOGI("POLLERR is set.\n");
  }
  if(pinfo->revents & POLLHUP)
  {
    LOGI("POLLHUP is set.\n");
  }

  // If the poll didn't work, or if the subsystem isn't available, start over.
  if((ret < 0) || (pinfo->revents & (POLLERR | POLLHUP)))
  {
    return -1;
  }

  // POLLPRI is used to indicate a signal change.
  if((pinfo->revents & POLLPRI) && handle_pollpri)
  {
    LOGI("POLLPRI is set.\n");
    LOGI("Attempting TIOCMGET on '%s'...\n", tinfo->from_file_path);

    PB_PTHREAD_MUTEX_LOCK(tinfo->from_mutex, tinfo->from_file_path);
    ret = ioctl(*(tinfo->from_fd), TIOCMGET, &signals);
    PB_PTHREAD_MUTEX_UNLOCK(tinfo->from_mutex, tinfo->from_file_path);
    construct_signal_string(signal_str_buf, MAX_MSG_SIZE, signals);
    LOGR(ret, "TIOCMGET on '%s' returned %i, signals=0x%x (%s)\n",
       tinfo->from_file_path, ret, signals, signal_str_buf);

    // Blindly pass on the entire signal mask; the USB and Modem sides should
    // discard any bits they don't care about
    if(mode_of_operation == PORT_BRIDGE_DUAL_MODE)
    {
      PB_PTHREAD_STATE_MUTEX_LOCK(&dual_mode_state_mutex, state_of_operation);
    }

    if( mode_of_operation == PORT_BRIDGE_SOCKET_MODE ||
      ( mode_of_operation == PORT_BRIDGE_DUAL_MODE && state_of_operation == PB_SOCKET_STATE ))
    {
      mdm_to_sock_msg.msg_id = PORTBRIDGE_SOCK_SIGNAL_IND;
      mdm_to_sock_msg.data_msg.signals = signals;
      num_of_bytes_to_write = sizeof(mdm_to_sock_msg.data_msg.signals) +
                              sizeof(mdm_to_sock_msg.msg_id);

      if (send(client_fd, &mdm_to_sock_msg, num_of_bytes_to_write,
           MSG_DONTWAIT | MSG_NOSIGNAL ) == PORT_BRIDGE_UNIX_API_ERROR)
      {
        LOGE("Socket Send Failed\n");
        if(mode_of_operation == PORT_BRIDGE_DUAL_MODE)
        {
          PB_PTHREAD_STATE_MUTEX_UNLOCK(&dual_mode_state_mutex, state_of_operation);
        }
        goto poll_begin;
      }
    }
    else
    {
      LOGI("Attempting TIOCMSET on '%s', signals=0x%x (%s)...\n",
           tinfo->to_file_path, signals, signal_str_buf);

      PB_PTHREAD_MUTEX_LOCK(tinfo->to_mutex, tinfo->to_file_path);
      ret = ioctl(*(tinfo->to_fd), TIOCMSET, &signals);
      PB_PTHREAD_MUTEX_UNLOCK(tinfo->to_mutex, tinfo->to_file_path);

      LOGR(ret, "TIOCMSET on '%s' returned %i, signals=0x%x (%s)\n",
           tinfo->to_file_path, ret, signals, signal_str_buf);
    }
    if(mode_of_operation == PORT_BRIDGE_DUAL_MODE)
    {
      PB_PTHREAD_STATE_MUTEX_UNLOCK(&dual_mode_state_mutex, state_of_operation);
    }
  }

  return 0;
}


/*===========================================================================
  FUNCTION  read_write_data
===========================================================================*/
/*!
@brief
  Utility function to read data if it is availble and write it to device

@param
  tinfo            pointer to communication handling thread info struct
  pinfo            pointer to polling info struct

@return
  void

@note
  In general, errors are logged but considered non-fatal.
*/
/*==========================================================================*/
void read_write_data
(
  thread_info_t* tinfo,
  struct pollfd* pinfo
)
{
  int ret                = 0;
  int num_read           = 0;
  char buf[MAX_MSG_SIZE + 1];
  int num_of_bytes_to_write     = 0 ;
  portbridge_sock_intf_data_t mdm_to_sock_msg;

  // Read data if we can
  if(pinfo->revents & POLLIN)
  {
    LOGI("POLLIN is set.\n");
    LOGI("Attempting to read from '%s'...\n", tinfo->from_file_path);

    PB_PTHREAD_MUTEX_LOCK(tinfo->from_mutex, tinfo->from_file_path);
    num_read = read(*(tinfo->from_fd), buf, MAX_MSG_SIZE);
    PB_PTHREAD_MUTEX_UNLOCK(tinfo->from_mutex, tinfo->from_file_path);

    LOGR(num_read, "read from '%s' returned %i\n",
         tinfo->from_file_path, num_read);

    // If read gave us an error, then device node isn't available anymore.
    if(num_read < 0)
    {
      close_fd_mutexed(tinfo->from_file_path, tinfo->from_fd,
                       tinfo->from_mutex);
      return;
    }

    // Write to the other end.  Log but otherwise ignore failures.
    if(num_read > 0)
    {
      buf[num_read] = '\0';
      LOGI("buf contains '%s'\n", buf);
      if(mode_of_operation == PORT_BRIDGE_DUAL_MODE)
      {
        PB_PTHREAD_STATE_MUTEX_LOCK(&dual_mode_state_mutex, state_of_operation);
      }
      if( mode_of_operation == PORT_BRIDGE_SOCKET_MODE ||
        ( mode_of_operation == PORT_BRIDGE_DUAL_MODE && state_of_operation == PB_SOCKET_STATE ))
      {
        mdm_to_sock_msg.msg_id = PORTBRIDGE_SOCK_DATA_IND;
        memcpy( mdm_to_sock_msg.data_msg.data_buffer.data, buf, num_read);
        mdm_to_sock_msg.data_msg.data_buffer.data_len = num_read;
        num_of_bytes_to_write = sizeof(mdm_to_sock_msg.data_msg.data_buffer.data_len) +
                                sizeof(mdm_to_sock_msg.msg_id) +
                                mdm_to_sock_msg.data_msg.data_buffer.data_len;
        if (send(client_fd, &mdm_to_sock_msg, num_of_bytes_to_write,
               MSG_DONTWAIT | MSG_NOSIGNAL ) == PORT_BRIDGE_UNIX_API_ERROR)
        {
          LOGE("Socket Data Send Failed\n");
          if(mode_of_operation == PORT_BRIDGE_DUAL_MODE)
          {
            PB_PTHREAD_STATE_MUTEX_UNLOCK(&dual_mode_state_mutex, state_of_operation);
          }
          return;
        }
      }
      else
      {
        LOGI("Attempting to write to '%s'...\n", tinfo->to_file_path);
        PB_PTHREAD_MUTEX_LOCK(tinfo->to_mutex, tinfo->to_file_path);
        ret = write_to_device(*(tinfo->to_fd), tinfo->to_file_path,
                              num_read, buf);
        PB_PTHREAD_MUTEX_UNLOCK(tinfo->to_mutex, tinfo->to_file_path);

        if (ret < 0)
        {
          close_fd_mutexed(tinfo->to_file_path, tinfo->to_fd, tinfo->to_mutex);
          if(mode_of_operation == PORT_BRIDGE_DUAL_MODE)
          {
            PB_PTHREAD_STATE_MUTEX_UNLOCK(&dual_mode_state_mutex, state_of_operation);
          }
          return;
        }
      }

      if(mode_of_operation == PORT_BRIDGE_DUAL_MODE)
      {
        PB_PTHREAD_STATE_MUTEX_UNLOCK(&dual_mode_state_mutex, state_of_operation);
      }
    }
  }

  // Did something else happen?
  if(pinfo->revents & ~(pinfo->events))
  {
    LOGE("An unrecognized event happened!  pollinfo.revents=0x%0x\n",
         pinfo->revents);
  }
}

/*===========================================================================
  FUNCTION  modem_to_usb_bridge_smd
===========================================================================*/
/*!
@brief
  Thread function responsible for handling data transfer and control signal
  updates from modem to USB over SMD transport

@param
  thread_info   Pointer to a thread_info_t which tells the function which
                direction of communication it is handling.

@note
  In general, errors are logged but considered non-fatal.
*/
/*==========================================================================*/
void* modem_to_usb_bridge_smd(void* thread_info)
{
  thread_info_t tinfo   = *((thread_info_t*)thread_info);
  struct pollfd pollinfo;
  memset(&pollinfo, 0, sizeof(struct pollfd));

  /* These are the events we care about.  They don't change. */
  pollinfo.events = POLLIN | POLLPRI | POLLERR | POLLHUP;

  while (1)
  {
    /*######## OPEN THE FILE ########*/
    open_device_file(&tinfo, &pollinfo, TRUE);

    // At this point it's open, so poll for anything we need to do
    if (do_polling(&tinfo, &pollinfo, TRUE) == -1)
    {
      LOGI("Closing: '%s'\n", tinfo.from_file_path);
      close_fd_mutexed(tinfo.from_file_path, tinfo.from_fd, tinfo.from_mutex);
      continue;
    }


    // Read data if we can
    read_write_data(&tinfo, &pollinfo);
  }

  return NULL;
}


/*===========================================================================
  FUNCTION  modem_to_usb_bridge_mhi
===========================================================================*/
/*!
@brief
  Thread function responsible for handling data transfer and control signal
  updates from modem to USB over MHI transport

@param
  thread_info   Pointer to a thread_info_t which tells the function which
                direction of communication it is handling.

@note
  In general, errors are logged but considered non-fatal.
*/
/*==========================================================================*/
void* modem_to_usb_bridge_mhi(void* thread_info)
{
  thread_info_t tinfo           = *((thread_info_t*) thread_info);
  int dummy                     = 0;
  int new_bits                  = 0;
  int old_bits                  = 0;
  int ret                       = 0;
  char signal_str_buf[MAX_MSG_SIZE + 1];  /* Just need a large enough buffer to hold a signal
                                             printout for debug */
  struct pollfd pollinfo;
  memset(&pollinfo, 0, sizeof(struct pollfd));
  portbridge_sock_intf_data_t mdm_to_sock_msg;
  int num_of_bytes_to_write     = 0 ;

  /* These are the events we care about.  They don't change. */
  pollinfo.events = POLLIN | POLLPRI | POLLERR | POLLHUP;


  while (1)
  {
    /* ####### OPEN THE MHI FILE ####### */
    open_device_file(&tinfo, &pollinfo, TRUE);

    // At this point it's open, so poll for anything we need to do
    if (do_polling(&tinfo, &pollinfo, FALSE) == -1)
    {
      LOGI("Closing: '%s'\n", tinfo.from_file_path);
      close_fd_mutexed(tinfo.from_file_path, tinfo.from_fd, tinfo.from_mutex);
      continue;
    }

    else if(pollinfo.revents & POLLPRI)
    {
      /* MHI driver has a different implementation of TIOCMGET ioctl
         The bits are being returned as part of the ioctl call
         and not filled in the address sent in the third argument
         The third argument has no significance */
      LOGI("Attempting TIOCMGET on '%s'...\n", tinfo.from_file_path);

      PB_PTHREAD_MUTEX_LOCK(tinfo.from_mutex, tinfo.from_file_path);
      new_bits = ioctl(*tinfo.from_fd, TIOCMGET, &dummy);
      PB_PTHREAD_MUTEX_UNLOCK(tinfo.from_mutex, tinfo.from_file_path);

      if (new_bits != old_bits)
      {
        construct_signal_string(signal_str_buf, MAX_MSG_SIZE, new_bits);
        LOGI("TIOCMGET on '%s' returned signals=0x%x (%s)\n",
             tinfo.from_file_path, new_bits, signal_str_buf);
        /* Blindly pass on the entire signal mask; the USB and Modem sides
           should discard any bits they don't care about */
        if(mode_of_operation == PORT_BRIDGE_DUAL_MODE)
          PB_PTHREAD_STATE_MUTEX_LOCK(&dual_mode_state_mutex, state_of_operation);

        if( mode_of_operation == PORT_BRIDGE_SOCKET_MODE ||
            (mode_of_operation ==PORT_BRIDGE_DUAL_MODE && state_of_operation == PB_SOCKET_STATE) )
        {
          mdm_to_sock_msg.msg_id = PORTBRIDGE_SOCK_SIGNAL_IND;
          mdm_to_sock_msg.data_msg.signals = new_bits;
          old_bits = new_bits;

          num_of_bytes_to_write = sizeof(mdm_to_sock_msg.data_msg.signals) +
                                  sizeof(mdm_to_sock_msg.msg_id);

          if (send(client_fd, &mdm_to_sock_msg, num_of_bytes_to_write,
               MSG_DONTWAIT | MSG_NOSIGNAL ) == PORT_BRIDGE_UNIX_API_ERROR)
          {
            LOGE("MHI Socket Send Failed\n");
            if(mode_of_operation == PORT_BRIDGE_DUAL_MODE)
            {
              PB_PTHREAD_STATE_MUTEX_UNLOCK(&dual_mode_state_mutex, state_of_operation);
            }
            continue;
          }
        }
        else
        {
          LOGI("Attempting TIOCMSET on '%s', signals=0x%x (%s)...\n",
               tinfo.from_file_path, new_bits, signal_str_buf);

          PB_PTHREAD_MUTEX_LOCK(tinfo.to_mutex, tinfo.to_file_path);
          ret = ioctl(*tinfo.to_fd, TIOCMSET, &new_bits);
          PB_PTHREAD_MUTEX_UNLOCK(tinfo.to_mutex, tinfo.to_file_path);

          old_bits = new_bits;
          LOGR(ret, "TIOCMSET on '%s' returned %i, signals=0x%x (%s)\n",
                   tinfo.from_file_path, ret, new_bits, signal_str_buf);
        }
        if(mode_of_operation == PORT_BRIDGE_DUAL_MODE)
        {
          PB_PTHREAD_STATE_MUTEX_UNLOCK(&dual_mode_state_mutex, state_of_operation);
        }
      }
    }

    /* Check if there is data to be read */
    read_write_data(&tinfo, &pollinfo);
  }

  return NULL;
}

/*===========================================================================
  FUNCTION  modem_to_usb_bridge_glink
===========================================================================*/
/*!
@brief
  Thread function responsible for handling data transfer and control signal
  updates from modem to USB over GLINK transport

@param
  thread_info   Pointer to a thread_info_t which tells the function which
                direction of communication it is handling.

@note
  In general, errors are logged but considered non-fatal.
*/
/*==========================================================================*/
void* modem_to_usb_bridge_glink(void* thread_info)
{
  thread_info_t tinfo   = *((thread_info_t*)thread_info);
  int ret                 = 0;
  unsigned int glink_rx_intent_count = 0;
  struct pollfd pollinfo = {0};

  char signal_str_buf[MAX_MSG_SIZE + 1];  /* Just need a large enough buffer to hold a signal
                                             printout for debug */

  /* These are the events we care about.  They don't change. */
  pollinfo.events = POLLIN | POLLPRI | POLLERR | POLLHUP;

  while (1)
  {
    /*######## OPEN THE FILE ########*/
    open_device_file(&tinfo, &pollinfo, TRUE);

    //######## USE THE FILE ########
    /* Queue RX intent to allow G-link to receive data */
    while (glink_rx_intent_count < GLINK_RX_INTENT_MIN)
    {
      unsigned int rx_packet_max_size = MAX_MSG_SIZE;

      LOGI("Queuing RX Intent for '%s'\n", tinfo.from_file_path);
      PB_PTHREAD_MUTEX_LOCK(tinfo.from_mutex, tinfo.from_file_path);
      ret = ioctl(*tinfo.from_fd, GLINK_PKT_IOCTL_QUEUE_RX_INTENT, &rx_packet_max_size);
      PB_PTHREAD_MUTEX_UNLOCK(tinfo.from_mutex, tinfo.from_file_path);
      if (ret)
      {
        LOGE("Error queuing RX intent=%d\n", ret);
        exit(1);
      }
      glink_rx_intent_count++;
    }

    // At this point it's open, so poll for anything we need to do
    if (do_polling(&tinfo, &pollinfo, TRUE) == -1)
    {
      LOGI("Closing: '%s'\n", tinfo.from_file_path);
      close_fd_mutexed(tinfo.from_file_path, tinfo.from_fd, tinfo.from_mutex);

      /*Reset RX_INTENT*/
      glink_rx_intent_count = 0;
      continue;
    }

    // Read data if we can
    if(pollinfo.revents & POLLIN)
    {
      glink_rx_intent_count--;
      LOGI("Marking rx intent as consumed\n");
    }
    read_write_data(&tinfo, &pollinfo);
  }

  return NULL;
}

/*===========================================================================
  FUNCTION  usb_to_modem_bridge
===========================================================================*/
/*!
@brief
  Thread function responsible for handling data transfer and control
  signal updates from USB to modem.

@param
  thread_info   Pointer to a thread_info_t which tells the function which
                direction of communication it is handling.

@note
  In general, errors are logged but considered non-fatal.
*/
/*==========================================================================*/
void* usb_to_modem_bridge(void* thread_info)
{
  thread_info_t tinfo           = *((thread_info_t*) thread_info);
  int num_read                  = 0;
  int signals                   = 0;
  int ret                       = 0;
  struct pollfd pollinfo;
  memset(&pollinfo, 0, sizeof(struct pollfd));
  char buf[MAX_MSG_SIZE + 1];             /* +1 so we always have room for a trailing '\0' */
  char signal_str_buf[MAX_MSG_SIZE + 1];  /* Just need a large enough buffer to hold a signal
                                             printout for debug */

  /* These are the events we care about.  They don't change */
  pollinfo.events = POLLIN | POLLPRI | POLLERR | POLLHUP;

  while (1)
  {
    /* ####### OPEN THE USB FILE ####### */
    open_device_file(&tinfo, &pollinfo, FALSE);

    /* ####### USE THE USB FILE ####### */
poll_usb:
    /* At this point the file is open, so poll for anything we need to do */
     if (do_polling(&tinfo, &pollinfo, FALSE) == -1)
    {
      if (mode_of_operation == PORT_BRIDGE_DUAL_MODE)
      {
        PB_PTHREAD_STATE_MUTEX_LOCK(&dual_mode_state_mutex, state_of_operation);
        if(state_of_operation == PB_SOCKET_STATE)
        {
          LOGI("State of operation has changed..can't continue Tethered mode\n");
          PB_PTHREAD_STATE_MUTEX_UNLOCK(&dual_mode_state_mutex, state_of_operation);
          goto poll_usb;
        }
      }

      PB_PTHREAD_STATE_MUTEX_UNLOCK(&dual_mode_state_mutex, state_of_operation);
      send_dtr_to_modem(thread_info);

      LOGI("Closing: '%s'\n", tinfo.from_file_path);
      close_fd_mutexed(tinfo.from_file_path, tinfo.from_fd, tinfo.from_mutex);
      continue;
    }

    /* POLLPRI is used to indicate a signal change. */
    if(pollinfo.revents & POLLPRI)
    {
      LOGI("POLLPRI is set.\n");
      LOGI("Attempting TIOCMGET on '%s'...\n", tinfo.from_file_path);

      PB_PTHREAD_MUTEX_LOCK(tinfo.from_mutex, tinfo.from_file_path);
      ret = ioctl(*tinfo.from_fd, TIOCMGET, &signals);
      PB_PTHREAD_MUTEX_UNLOCK(tinfo.from_mutex, tinfo.from_file_path);

      construct_signal_string(signal_str_buf, MAX_MSG_SIZE, signals);
      LOGR(ret, "TIOCMGET on '%s' returned %i, signals=0x%x (%s)\n",
           tinfo.from_file_path, ret, signals, signal_str_buf);

      /* Blindly pass on the entire signal mask; the USB and Modem sides
         should discard any bits they don't care about */
      LOGI("Attempting TIOCMSET on '%s', signals=0x%x (%s)...\n",
           tinfo.to_file_path, signals, signal_str_buf);
      if (mode_of_operation == PORT_BRIDGE_DUAL_MODE)
      {
        PB_PTHREAD_STATE_MUTEX_LOCK(&dual_mode_state_mutex, state_of_operation);
        if(state_of_operation == PB_SOCKET_STATE)
        {
              LOGI("State of operation has changed..can't continue Tethered mode\n");
              PB_PTHREAD_STATE_MUTEX_UNLOCK(&dual_mode_state_mutex, state_of_operation);
              goto poll_usb;
        }
       }

      PB_PTHREAD_MUTEX_LOCK(tinfo.to_mutex, tinfo.to_file_path);
      ret = ioctl(*tinfo.to_fd, TIOCMSET, &signals);
      PB_PTHREAD_MUTEX_UNLOCK(tinfo.to_mutex, tinfo.to_file_path);

      if (mode_of_operation == PORT_BRIDGE_DUAL_MODE)
      {
        PB_PTHREAD_STATE_MUTEX_UNLOCK(&dual_mode_state_mutex, state_of_operation);
      }

      LOGR(ret, "TIOCMSET on '%s' returned %i, signals=0x%x (%s)\n",
           tinfo.to_file_path, ret, signals, signal_str_buf);
    }

    /* Read data if we can */
    if (pollinfo.revents & POLLIN)
    {
      LOGI("POLLIN is set.\n");
      LOGI("Attempting to read from '%s'...\n", tinfo.from_file_path);
      PB_PTHREAD_MUTEX_LOCK(tinfo.from_mutex, tinfo.from_file_path);
      num_read = read(*tinfo.from_fd, buf, MAX_MSG_SIZE);
      PB_PTHREAD_MUTEX_UNLOCK(tinfo.from_mutex, tinfo.from_file_path);
      LOGR(num_read, "read from '%s' returned %i\n", tinfo.from_file_path, num_read);

      /* If read gave an error then usb device is no longer available */
      if (num_read < 0)
      {
        close_fd_mutexed(tinfo.from_file_path, tinfo.from_fd, tinfo.from_mutex);
        continue;
      }

      if (num_read > 0)
      {
        buf[num_read] = '\0';
        LOGI("buf contains '%s'\n", buf);

        if (mode_of_operation == PORT_BRIDGE_DUAL_MODE)
        {
          PB_PTHREAD_STATE_MUTEX_LOCK(&dual_mode_state_mutex, state_of_operation);
          if(state_of_operation == PB_SOCKET_STATE)
          {
            LOGI("State of operation has changed..can't continue Tethered mode\n");
            PB_PTHREAD_STATE_MUTEX_UNLOCK(&dual_mode_state_mutex, state_of_operation);
            goto poll_usb;
          }
        }
        LOGI("Attempting to write to '%s'...\n", tinfo.to_file_path);
        PB_PTHREAD_MUTEX_LOCK(tinfo.to_mutex, tinfo.to_file_path);
        ret = write_to_device(*tinfo.to_fd, tinfo.to_file_path, num_read, buf);
        PB_PTHREAD_MUTEX_UNLOCK(tinfo.to_mutex, tinfo.to_file_path);

        if (mode_of_operation == PORT_BRIDGE_DUAL_MODE)
        {
          PB_PTHREAD_STATE_MUTEX_UNLOCK(&dual_mode_state_mutex, state_of_operation);
        }
        if (ret < 0)
        {
          close_fd_mutexed(tinfo.to_file_path, tinfo.to_fd, tinfo.to_mutex);
          continue;
        }
      }
    }

    /* Did something else happen? */
    if(pollinfo.revents & ~(pollinfo.events))
    {
      LOGE("An unrecognized event happened!  pollinfo.revents=0x%0x\n", pollinfo.revents);
    }
  }

  return NULL;
}

/*===========================================================================
  FUNCTION  port_bridge_recv_from_client
===========================================================================*/
/*!
@brief
  Thread function responsible for handling data transfer and control
  signal updates via socket communication from client to modem.

@param
  thread_info              Pointer to a thread_info_t which gives the function
                           modem file path, file descriptor and mutex.
  port_bridge_server_fd    The file descriptor of the server, on which it is
                           Listening for the connections.

@note
  In general, errors are logged but considered non-fatal.
*/
/*==========================================================================*/
void port_bridge_recv_from_client (int port_bridge_server_fd, void* thread_info)
{

  LOGI("Waiting for incoming client connection\n");

  int                          ret;
  int                          msg_size;
  thread_info_t tinfo          = *((thread_info_t*) thread_info);
  struct sockaddr_un           client_socket;
  portbridge_sock_intf_data_t  sock_to_mdm_msg;
  socklen_t socket_size        = PORT_BRIDGE_UNIX_API_ERROR;
  char                         signal_str_buf[MAX_MSG_SIZE + 1];
  char                         buf[MAX_MSG_SIZE + 1];
  socket_size                  = sizeof(client_socket);
  struct pollfd                pollinfo;
  int                          signals;
  int                          num_of_bytes_to_write;
  portbridge_sock_intf_data_t  pb_reply_msg;
  while(1)
  {
    ret                       = 0;
    msg_size                  = PORT_BRIDGE_UNIX_API_ERROR;
    num_of_bytes_to_write     = 0;

    if ((client_fd = accept4(port_bridge_server_fd, (struct sockaddr *)&client_socket,
                     &socket_size, SOCK_CLOEXEC)) == PORT_BRIDGE_UNIX_API_ERROR)
    {
      LOGE("Unable to accept connection on UNIX socket %s\n",strerror(errno));
      continue;
    }

    memset(&pollinfo, 0, sizeof(struct pollfd));
    pollinfo.events = POLLIN | POLLNVAL | POLLERR | POLLHUP;

poll_socket:
      pollinfo.fd = client_fd;

      pollinfo.revents = 0;
      ret = poll(&pollinfo, 1, -1);
      if (ret == PORT_BRIDGE_UNIX_API_ERROR)
      {
        LOGE("Error Occured during Poll %s\n", strerror(errno));
        if (errno == EINTR)
        {
          LOGE("Polling Unix Socket Again\n");
          goto poll_socket;
        }
        goto close_client_socket;
      }
      if (!(pollinfo.revents & POLLIN))
      {
        LOGE("Invalid event was received in UNIX socket - %d\n", pollinfo.revents);
        goto close_client_socket;
      }

      msg_size = recv(client_fd, &sock_to_mdm_msg,
                      sizeof(portbridge_sock_intf_data_t), 0);

      if(msg_size <= 0 )
      {
        LOGE("Error in Recv\n");
        send_dtr_to_modem(thread_info);

        //In case The Client side has ungracefully terminated, we will not
        //wait in PB_SOCKET_STATE rather in PB_TETHERED_STATE.
        if (mode_of_operation == PORT_BRIDGE_DUAL_MODE)
        {
          PB_PTHREAD_STATE_MUTEX_LOCK(&dual_mode_state_mutex, state_of_operation);
          state_of_operation = PB_TETHERED_STATE;
          PB_PTHREAD_STATE_MUTEX_UNLOCK(&dual_mode_state_mutex, state_of_operation);
        }
        goto close_client_socket;
      }
      if(msg_size > 0)
      {

        switch(sock_to_mdm_msg.msg_id)
        {
          case PORTBRIDGE_SOCK_SOCKET_MODE_ENABLE_REQ:

            LOGI("PB Socket Mode Enable Received\n");
            pb_reply_msg.msg_id = PORTBRIDGE_SOCK_SOCKET_MODE_ENABLE_RES;
            pb_reply_msg.data_msg.result = PORT_BRIDGE_SUCCESS;
            num_of_bytes_to_write = sizeof(pb_reply_msg.data_msg.result) +
                                    sizeof(pb_reply_msg.msg_id);
            if(state_of_operation != PB_SOCKET_STATE)
            {
              send_dtr_to_modem(thread_info);
            }

            if (send(client_fd, &pb_reply_msg, num_of_bytes_to_write,
                MSG_DONTWAIT | MSG_NOSIGNAL ) == PORT_BRIDGE_UNIX_API_ERROR)
            {
              LOGE("Enable Socket Mode Response Send Failed %s \n", strerror(errno));
              goto poll_socket;
            }
            //Check added if enable request received when already in PB_SOCKET_STATE.
            else if(state_of_operation != PB_SOCKET_STATE)
            {
              PB_PTHREAD_STATE_MUTEX_LOCK(&dual_mode_state_mutex, state_of_operation);
              state_of_operation = PB_SOCKET_STATE;
              PB_PTHREAD_STATE_MUTEX_UNLOCK(&dual_mode_state_mutex, state_of_operation);
              LOGI("Enable Response msg Successfully Sent\n");
            }
            break;

          case PORTBRIDGE_SOCK_SIGNAL_IND:

            if(state_of_operation == PB_TETHERED_STATE)
              goto poll_socket;

            signals = sock_to_mdm_msg.data_msg.signals;
            construct_signal_string(signal_str_buf, MAX_MSG_SIZE, signals);
            LOGI("Signals Received=0x%x (%s)\n",signals, signal_str_buf);

            /* Blindly pass on the entire signal mask; the Client and Modem sides
               should discard any bits they don't care about */

            LOGI("Attempting TIOCMSET on '%s', signals=0x%x (%s)...\n",
                 tinfo.to_file_path, signals, signal_str_buf);

            PB_PTHREAD_MUTEX_LOCK(tinfo.to_mutex, tinfo.to_file_path);
            ret = ioctl(*tinfo.to_fd, TIOCMSET, &signals);
            PB_PTHREAD_MUTEX_UNLOCK(tinfo.to_mutex, tinfo.to_file_path);

            LOGR(ret, "TIOCMSET on '%s' returned %i, signals=0x%x (%s)\n",
                 tinfo.to_file_path, ret, signals, signal_str_buf);
            break;

          case PORTBRIDGE_SOCK_DATA_IND:

            if(state_of_operation == PB_TETHERED_STATE)
              goto poll_socket;

            if(sock_to_mdm_msg.data_msg.data_buffer.data_len > MAX_MSG_SIZE)
            {
              LOGE("Data Received from client larger than max allowed\n");
              goto poll_socket;
            }
            memcpy(buf, sock_to_mdm_msg.data_msg.data_buffer.data,
                    sock_to_mdm_msg.data_msg.data_buffer.data_len);
            buf[sock_to_mdm_msg.data_msg.data_buffer.data_len] = '\0';
            LOGI("buf contains %s\n", buf);

            LOGI("Attempting to write to '%s'...\n", tinfo.to_file_path);
            PB_PTHREAD_MUTEX_LOCK(tinfo.to_mutex, tinfo.to_file_path);
            ret = write_to_device(*tinfo.to_fd, tinfo.to_file_path,
                                  sock_to_mdm_msg.data_msg.data_buffer.data_len, buf);
            PB_PTHREAD_MUTEX_UNLOCK(tinfo.to_mutex, tinfo.to_file_path);
            if (ret < 0)
              close_fd_mutexed(tinfo.to_file_path, tinfo.to_fd, tinfo.to_mutex);
            break;

          case PORTBRIDGE_SOCK_SOCKET_MODE_DISABLE_REQ:

            LOGI("PB Socket Mode Disable Received\n");
            pb_reply_msg.msg_id = PORTBRIDGE_SOCK_SOCKET_MODE_DISABLE_RES;

            if(state_of_operation == PB_TETHERED_STATE)
            {
              pb_reply_msg.data_msg.result = PORT_BRIDGE_FAILURE;
              LOGI("Disable Request received in invalid state\n");
            }

            else
            {
              send_dtr_to_modem(thread_info);
              pb_reply_msg.data_msg.result = PORT_BRIDGE_SUCCESS;
            }

            num_of_bytes_to_write = sizeof(pb_reply_msg.data_msg.result) +
                                    sizeof(pb_reply_msg.msg_id);

            if (send(client_fd, &pb_reply_msg, num_of_bytes_to_write,
                     MSG_DONTWAIT | MSG_NOSIGNAL ) == PORT_BRIDGE_UNIX_API_ERROR)
            {
              LOGE("Disable Socket Mode Response Send Failed %s \n", strerror(errno));
              goto poll_socket;
            }

            else if (state_of_operation != PB_TETHERED_STATE)
            {
              PB_PTHREAD_STATE_MUTEX_LOCK(&dual_mode_state_mutex, state_of_operation);
              state_of_operation = PB_TETHERED_STATE;
              PB_PTHREAD_STATE_MUTEX_UNLOCK(&dual_mode_state_mutex, state_of_operation);
              LOGI("Disable Response msg Successfully sent\n");
            }
            else
            {
               LOGI("Disable Response msg Successfully sent for invalid state\n");
            }
          break;

          default:
            LOGI("No case hit, unknown condition\n");
        }
      }

      goto poll_socket;

close_client_socket:
    if(client_fd)
    {
      close(client_fd);
      client_fd=-1;
    }
  }
}

/*===========================================================================
  FUNCTION  port_bridge_socket_handler
===========================================================================*/
/*!
@brief
  Initialization routine for UNIX domain socket interface message listener.
  This sets up the UNIX listener socket for receiving clien connection requests.

@param
  thread_info   Pointer to a thread_info_t which gives the function
                modem file path, file descriptor and mutex.
@note
  In general, errors are logged but considered non-fatal.
*/
/*==========================================================================*/
 void* port_bridge_socket_handler(void* thread_info)
 {
   int port_bridge_server_fd    = PORT_BRIDGE_UNIX_API_ERROR;
   int len                      = PORT_BRIDGE_UNIX_API_ERROR;
   int str_len                  = PORT_BRIDGE_UNIX_API_ERROR;
   struct sockaddr_un  server_socket;

   if ((port_bridge_server_fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0))
                              == PORT_BRIDGE_UNIX_API_ERROR)
   {
     LOGE("Unable to create UNIX socket: %s \n", strerror(errno));
     goto exit;
   }

   LOGI("UNIX Server Socket Created\n");
   server_socket.sun_family = AF_UNIX;
   str_len = strlcpy(server_socket.sun_path, PORTBRIDGE_SOCK_PATH, UNIX_PATH_MAX);

   if ((str_len <= PORT_BRIDGE_UNIX_API_ERROR) || (str_len > UNIX_PATH_MAX))
   {
     LOGE("String Truncation Occured\n");
     goto close_server_socket;
   }
   unlink(server_socket.sun_path);
   len = strlen(server_socket.sun_path) + sizeof(server_socket.sun_family);

   if (bind(port_bridge_server_fd, (struct sockaddr *)&server_socket, len)
                == PORT_BRIDGE_UNIX_API_ERROR)
   {
     LOGE("Unable to bind to UNIX server socket - %s\n", strerror(errno));
     goto close_server_socket;
   }

   LOGI("Bound to UNIX server socket\n");

   /* Allow RW permissions only for user and group */
   if (chmod(server_socket.sun_path, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)
        == PORT_BRIDGE_UNIX_API_ERROR)
   {
     LOGE("Unable to chmod the UNIX server socket %s - %s\n",
          server_socket.sun_path,strerror(errno));
     goto close_server_socket;
   }

    LOGI("Successful chmod of the UNIX server socket\n");

   if (listen(port_bridge_server_fd, PORT_BRIDGE_UNIX_MAX_CLIENTS)
                 == PORT_BRIDGE_UNIX_API_ERROR)
   {
     LOGE("Unable to listen on UNIX socket - %s\n", strerror(errno));
     goto close_server_socket;
   }

   LOGI("Listening to server socket\n");

   port_bridge_recv_from_client(port_bridge_server_fd, thread_info);

close_server_socket:
  if (port_bridge_server_fd)
    close(port_bridge_server_fd);
exit:
  LOGE("FAILURE Occurred in Socket Operations\n");
  return NULL;
 }

/*===========================================================================
  FUNCTION  Check_kernel_version
===========================================================================*/
/*!
@brief
   Check for Kernel version, char_bridge support depricated in kernel version above 4.4.
   From kernel verions 4.4 port_bridge support for SMD transport.

@note
  By default port_bridge is disabled for SMD target.
  it only enabled if SMD target support with kernel version above 4.4.
*/
/*===========================================================================*/
boolean Check_kernel_version(void)
{
  struct utsname system_info;
  int ret=0;
  if(0==(ret=uname(&system_info)))
  {
    LOGI("system_info : sysname [%s] nodename [%s] release [%s] version [%s]  machine [%s]",\
          system_info.sysname,system_info.nodename,system_info.release,system_info.version,system_info.machine);
  }
  else
  {
    LOGE("Failed to get system_info ret [%d] !!", ret);
  }

  if (system_info.release[0] && system_info.release[0]>='4')
  {
    LOGI("SMD transport supported!!");
    return TRUE;
  }
  else
  {
    LOGI("SMD transport not supported!!");
    return FALSE;
  }
}

/*===========================================================================
  FUNCTION  pb_resolve_target
===========================================================================*/
/*!
@brief
  Queries dsutils for target type. Fills in targetinfo with appropriate
  info.

@note
  pb_resolve_info guarantees that all fields in *targetinfo are filled
  in before returning. Non-zero return code indicates failure and caller
  should assume that *target is invalid and may contain NULL or stale
  pointers.
*/
/*===========================================================================*/
static int pb_resolve_target(struct pb_targetinfo *targetinfo)
{
  ds_target_t target;
  int rc = -1;
  int ret = -1;
  char mhi_esoc_link_info[MAX_SOC_LINK_INFO_LEN];

  if (!targetinfo)
    return -1;

  target = ds_get_target();
  switch (target)
  {
    case DS_TARGET_FUSION_8084_9X45:
    case DS_TARGET_FUSION_8096_9X55:
    case DS_TARGET_FUSION_SDM845_SDX24:
    case DS_TARGET_FUSION_8998_SDX50_SA:
    case DS_TARGET_FUSION_SDA855_SDX55:
    case DS_TARGET_FUSION_SDA865_SDX55:
    case DS_TARGET_FUSION_SDA855_SDXMARMOT:
      do
      {
        FILE *fp = NULL;

        memset(mhi_esoc_link_info, 0, MAX_SOC_LINK_INFO_LEN * sizeof(char));
        memset(mdm_file_path_mhi, 0, MAX_PATH_LEN * sizeof(char));

        fp = fopen(MHI_ESOC_FILE_PATH,"r");
        if(NULL == fp)
        {
          LOGI("%s: error opening MHI ESOC file\n", __func__);

          /* Use Legacy MHI pipe id name*/
          ret = snprintf(mdm_file_path_mhi,
                         sizeof(mdm_file_path_mhi),
                         "%s%s",
                         MDM_FILE_PATH_MHI_START,
                         MDM_FILE_PATH_MHI_END);

          if(ret < 0)
          {
            return -1;
          }
        }
        else if (0 == fscanf(fp,"%s", mhi_esoc_link_info))
        {
          LOGI("%s: error reading mhi_esoc_link_info\n", __func__);
          fclose(fp);
          fp = NULL;
          return -1;
        }
        else
        {
          /* Update new MHI pipe id name*/
          ret = snprintf(mdm_file_path_mhi,
                         sizeof(mdm_file_path_mhi),
                         "%s%s_%s",
                         MDM_FILE_PATH_MHI_START,
                         mhi_esoc_link_info,
                         MDM_FILE_PATH_MHI_END);

          LOGI("%s: mdm_file_path_mhi=\'%s\'\n", __func__, mdm_file_path_mhi);
          fclose(fp);
          fp = NULL;

          if(ret < 0)
          {
            return -1;
          }
        }
      }while(0);

      targetinfo->mdm_chardev = mdm_file_path_mhi;
      targetinfo->usb_chardev = USB_FILE_PATH;
      targetinfo->mdm_handler = modem_to_usb_bridge_mhi;
      rc = 0;

      if (target == DS_TARGET_FUSION_SDA855_SDX55 ||
          target == DS_TARGET_FUSION_SDA865_SDX55 ||
          target == DS_TARGET_FUSION_SDA855_SDXMARMOT)
        mode_of_operation = PORT_BRIDGE_DUAL_MODE;
      break;

    case DS_TARGET_MSM8996:
    case DS_TARGET_MSM8998:
    case DS_TARGET_MSMNILE:
    case DS_TARGET_SDM845:
    case DS_TARGET_STARLORD:
    case DS_TARGET_STARHAWK:
    case DS_TARGET_WARLOCK:
    case DS_TARGET_FUSION_SM8150_SDX50:
    case DS_TARGET_LITO:
    case DS_TARGET_MSMSTEPPE:
      targetinfo->mdm_chardev = MDM_FILE_PATH_GLINK;
      targetinfo->usb_chardev = USB_FILE_PATH;
      targetinfo->mdm_handler = modem_to_usb_bridge_glink;
      rc = 0;
      break;
    case DS_TARGET_JACALA:
    case DS_TARGET_FEERO:
    case DS_TARGET_JOLOKIA:
    case DS_TARGET_FEERO6:
      if(Check_kernel_version())
      {
        targetinfo->mdm_chardev = MDM_FILE_PATH_SMD;
        targetinfo->usb_chardev = USB_FILE_PATH;
        targetinfo->mdm_handler = modem_to_usb_bridge_smd;
        rc = 0;
      }
      else
      {
        LOGE("Unsupported target: %s - Check for char_bridge on this target!", ds_get_target_str(target));
        rc =-1;
      }
      break;

    default:
      LOGE("Unsupported target: %s\n", ds_get_target_str(target));
      rc = -1;
      break;
  }

  if (rc == 0)
  {
    LOGI("Resolved target to: %s\n", ds_get_target_str(target));
    LOGI("Using USB end point: %s\n", targetinfo->usb_chardev);
    LOGI("Using MDM end point: %s\n", targetinfo->mdm_chardev);
    LOGI("Using mdm_handler: %p\n", targetinfo->mdm_handler);
  }
  return rc;
}


/*===========================================================================
  FUNCTION  pb_get_log_enabled
===========================================================================*/
/*!
@brief
  Read android porperty to enable or disable port_bridge logging

@note
  By default we disabel logging unless the android property is set to true.

*/
/*===========================================================================*/
void pb_get_log_enabled()
{
#ifdef FEATURE_PB_LINUX_ANDROID
  char args[PROPERTY_VALUE_MAX];
  int ret;

  port_bridge_log_enabled = FALSE;
  memset(args, 0, sizeof(args));
  ret = property_get(PORT_BRIDGE_LOG_ENABLED,
                     args,
                     PORT_BRIDGE_FALSE_STR);
  if ( ret < (int)sizeof(PORT_BRIDGE_FALSE_STR) &&
       !strncmp(PORT_BRIDGE_TRUE_STR, args, sizeof(PORT_BRIDGE_TRUE_STR)))
  {
    port_bridge_log_enabled = TRUE;
  }
#else
  /* Probably this is not applicable. Adding this for completeness sake */
  port_bridge_log_enabled = TRUE;
#endif
}

/*===========================================================================
  FUNCTION  main
===========================================================================*/
/*!
@brief
  Launches two threads, one for each direction of communication.

@note
  In general, errors are logged but considered non-fatal.
*/
/*===========================================================================*/
int main()
{
  struct pb_targetinfo targetinfo;

  pb_get_log_enabled();

#ifdef PORT_BRIDGE_TEST
  srand( rand() );
  port_bridge_log_enabled = TRUE;
#endif

  if( TRUE == port_bridge_log_enabled )
  {
    // Initialize the log - use stdout as fallback
    log_file = fopen(LOG_FILE_PATH, "w");
    if(!log_file)
    {
      log_file = stdout;
    }
    LOGI("log file is %p.\n", log_file);
  }
  pthread_t mdm_thread, usb_thread, sock_thread;
  thread_info_t mdm_info, usb_info, sock_info;

  if (pb_resolve_target(&targetinfo) != 0)
  {
    LOGE("Failed to resolve target info. Dying...\n");
    if( TRUE == port_bridge_log_enabled )
    {
      fclose(log_file);
    }
    exit(1);
  }

  /* MDM -> USB info object */
  mdm_info.from_fd        = &mdm_fd;
  mdm_info.to_fd          = &usb_fd;
  mdm_info.to_file_path   = (char *) targetinfo.usb_chardev;
  mdm_info.from_file_path = (char *) targetinfo.mdm_chardev;
  mdm_info.to_mutex       = &usb_mutex;
  mdm_info.from_mutex     = &mdm_mutex;

  pthread_create(&mdm_thread, NULL, targetinfo.mdm_handler, &mdm_info);
  LOGI("Thread %p was created for direction %s -> %s\n",
        (void*)mdm_thread, mdm_info.from_file_path, mdm_info.to_file_path);

  if( mode_of_operation == PORT_BRIDGE_TETHERED_MODE ||
                           mode_of_operation == PORT_BRIDGE_DUAL_MODE )
  {
    /* USB -> MDM info object */
    usb_info.from_fd        = &usb_fd;
    usb_info.to_fd          = &mdm_fd;
    usb_info.from_file_path = (char *) targetinfo.usb_chardev;
    usb_info.to_file_path   = (char *) targetinfo.mdm_chardev;
    usb_info.to_mutex       = &mdm_mutex;
    usb_info.from_mutex     = &usb_mutex;

    pthread_create(&usb_thread,  NULL, usb_to_modem_bridge, &usb_info);
    LOGI("Thread %p was created for direction %s -> %s\n",
    (void*)usb_thread, usb_info.from_file_path, usb_info.to_file_path);
  }

  if( mode_of_operation == PORT_BRIDGE_SOCKET_MODE ||
                           mode_of_operation == PORT_BRIDGE_DUAL_MODE )
  {
    sock_info.to_fd         = &mdm_fd;
    sock_info.to_file_path   = (char *) targetinfo.mdm_chardev;
    sock_info.to_mutex       = &mdm_mutex;
    pthread_create(&sock_thread, NULL, port_bridge_socket_handler, &sock_info);
    LOGI("Thread %p was created for direction socket client to -> %s\n",
    (void*)sock_thread, sock_info.to_file_path);
    pthread_join(sock_thread, NULL);
  }

  pthread_join(mdm_thread,  NULL);

  if( mode_of_operation == PORT_BRIDGE_TETHERED_MODE ||
                           mode_of_operation == PORT_BRIDGE_DUAL_MODE )
  {
    pthread_join(usb_thread,   NULL);
  }

  if( TRUE == port_bridge_log_enabled )
  {
    /* Cleanup added to be idiomatic; program should run indefinitely */
    fclose(log_file);
  }

  return 0;
}

