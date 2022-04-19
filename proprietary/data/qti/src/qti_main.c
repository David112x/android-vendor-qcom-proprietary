/******************************************************************************

                        QTI_MAIN.C

******************************************************************************/

/******************************************************************************

  @file    qti_main.c
  @brief   Tethering Interface module

  DESCRIPTION
  Implementation of Tethering Interface module.

  ---------------------------------------------------------------------------
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
01/22/14   sb         Added QTI for Fusion
12/9/13    sb         Add port mapper functionality. Modem interface file interaction
11/15/12   sb         Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/inotify.h>
#include <cutils/sockets.h>

#include "qmi_idl_lib.h"
#include "qmi_csi.h"
#include "qti.h"

/*===========================================================================
                              VARIABLE DECLARATIONS
===========================================================================*/

static qti_conf_t                qti_conf;
static qti_rmnet_param           rmnet_config_param;
static qti_dpl_param             dpl_config_param;
qcmap_sk_fd_set_info_t           sk_fdset;
boolean                          is_fusion_target = FALSE;
boolean                          is_fusion_sw_path;

static qti_conf_t                qti_conf;
static qmi_csi_os_params         os_params;

/* Global Netlink Socket. variable */
qcmap_sk_info_t     sk_info;

/*Server socket*/
unsigned int qti_dpm_sockfd;

/*Client socket*/
unsigned int dpm_qti_sockfd;

#define INOTIFY_EVENT_SIZE  (sizeof(struct inotify_event))
#define INOTIFY_BUF_LEN     (INOTIFY_EVENT_SIZE + 2*sizeof(HEX_DUMPS_FOLDER))


/*===========================================================================
                              FUNCTION DEFINITIONS
===========================================================================*/

/*==========================================================================

FUNCTION PRINT_MSG()

DESCRIPTION

  This function is used to print QTI logs to a file

DEPENDENCIES
  None.

RETURN VALUE


SIDE EFFECTS
  None

==========================================================================*/

void PRINT_msg
(
  const char *funcName,
  uint32_t lineNum,
  const char*fmt, ...
)
{
  va_list args;
  memset(&args, 0, sizeof(va_list));
  /*------------------------------------------------------------------------*/
  va_start (args, fmt);
  fprintf(fp_logs,"\n Func: %s Line: %d :", funcName, lineNum);
  vfprintf (fp_logs, fmt, args);
  va_end (args);
}

/*===========================================================================

FUNCTION qti_print_logcat_msg()

DESCRIPTION
- print the message in logcat
- Used for LA targets only

DEPENDENCIES
  None.

RETURN VALUE
  None

SIDE EFFECTS
  None

==========================================================================*/
void qti_print_logcat_msg
(
  uint32_t msgType,
  const char *funcName,
  uint32_t lineNum,
  const char *fileInfo,
  const char *fmt, ...
)
{
  char msg[QTI_MAX_COMMAND_STR_LEN] = {0};
  int len = 0;
  va_list args;
  char *fileName = NULL;

  if (funcName == NULL || fileInfo == NULL || fmt == NULL)
  {
    LOG_MSG_ERROR("Invalid params", 0, 0, 0);
    return ;
  }

  fileName = strrchr(fileInfo, '/'); //absolute path to only filename
  if (fileName == NULL)
  {
    len = snprintf(msg, QTI_MAX_COMMAND_STR_LEN, "[%s:%d] %s():",
                   fileInfo, lineNum, funcName);
  }
  else
  {
    len = snprintf(msg, QTI_MAX_COMMAND_STR_LEN, "[%s:%d] %s():",
                   fileName + 1, lineNum, funcName);
  }
  if (len > 0 && len < QTI_MAX_COMMAND_STR_LEN)
  {
    va_start (args, fmt);
    vsnprintf(msg + len, QTI_MAX_COMMAND_STR_LEN - len, fmt, args);
    va_end (args);
  }

  if (msgType == LOGCAT_INFO)
    __android_log_print( ANDROID_LOG_INFO, MAIN_LOG_TAG,"%s", msg);
  else
    __android_log_print( ANDROID_LOG_ERROR, MAIN_LOG_TAG,"%s", msg);

  return ;
}

/*==========================================================================

FUNCTION HEX_DUMPS_RESET()

DESCRIPTION

  The function which enables the logging in QTI.

DEPENDENCIES
  None.

RETURN VALUE
  0 on SUCCESS

SIDE EFFECTS
  None

==========================================================================*/
int hex_dumps_reset
(
  int hex_dump_fd
)
{
  int length;
  int ret_val;
  char buffer[INOTIFY_BUF_LEN] = {0};
  uint32_t mask = IN_CREATE | IN_DELETE;

  LOG_MSG_INFO1("Received notifications in dir %s", HEX_DUMPS_FOLDER, 0, 0);

  struct inotify_event* event = NULL;

  length = read(hex_dump_fd, buffer, INOTIFY_BUF_LEN);

  if (length < 0)
  {
    LOG_MSG_ERROR("inotify read() error return length: %d and mask: 0x%x\n", length, mask, 0);
    return QTI_FAILURE;
  }
  event = (struct inotify_event*)calloc(1,length);
  if(event == NULL)
  {
    LOG_MSG_ERROR("Unable to allocate memory to event", 0, 0, 0);
    return QTI_FAILURE;
  }

  memcpy(event, buffer, length);

  if (event->len > 0)
  {
     if (!(event->mask & IN_ISDIR))
     {
       if(event->mask & IN_CREATE)
       {
         LOG_MSG_INFO1("event mask is create", 0, 0, 0);
         if(strncmp(event->name, HEX_DUMP_QXDM_FLAG, strlen(HEX_DUMP_QXDM_FLAG))==0)
           hex_dumps_qxdm_mode = TRUE;
         if(strncmp(event->name, HEX_DUMP_FILE_FLAG, strlen(HEX_DUMP_FILE_FLAG))==0)
           hex_dumps_file_mode = TRUE;
       }
       else if(event->mask & IN_DELETE)
       {
         LOG_MSG_INFO1("event mask is delete", 0, 0, 0);
         if(strncmp(event->name, HEX_DUMP_QXDM_FLAG, strlen(HEX_DUMP_QXDM_FLAG))==0)
           hex_dumps_qxdm_mode = FALSE;
         if(strncmp(event->name, HEX_DUMP_FILE_FLAG, strlen(HEX_DUMP_FILE_FLAG))==0)
           hex_dumps_file_mode = FALSE;
       }
     }
  }
  free(event);

  return QTI_SUCCESS;
}

/*===========================================================================
  FUNCTION create_socket
=============================================================================
  @brief
    creates a unix domain socket

  @input
    sockfd - socket handler

  @return
    0  - success
  - 1 - failure

  @dependencies
    none

  @sideefects
  None
===========================================================================*/
int create_socket(unsigned int *sockfd)
{
  if ((*sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == QTI_FAILURE)
  {
    LOG_MSG_ERROR("Error creating socket, errno: %d", errno, 0, 0);
    return QTI_FAILURE;
  }

  if(fcntl(*sockfd, F_SETFD, FD_CLOEXEC) < 0)
  {
    LOG_MSG_ERROR("Couldn't set Close on Exec, errno: %d", errno, 0, 0);
  }

  return QTI_SUCCESS;
}

/*===========================================================================
  FUNCTION create_server_socket
=============================================================================
  @brief
    creates server socket

  @input
    unsigned int sock

  @return
    0  - success
  - 1 - failure

  @dependencies
    none

  @sideefects
  None
===========================================================================*/
int create_server_socket(unsigned int* sockfd, char* file_path)
{
  int val, rval;
  struct sockaddr_un qti_un;
  int len=0;
  struct timeval rcv_timeo;

  *sockfd = android_get_control_socket(file_path);
  if( *sockfd < 0 )
  {
    LOG_MSG_ERROR("failed to create server sockfd ", 0, 0, 0);
    return QTI_FAILURE;
  }
  else if (*sockfd >= FD_SETSIZE)
  {
    LOG_MSG_ERROR(" create_server_socket fd=%d is exceeding range", *sockfd, 0, 0);
    return QTI_FAILURE;
  }

  rcv_timeo.tv_sec = 0;
  rcv_timeo.tv_usec = 100000;
  setsockopt(*sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&rcv_timeo, sizeof(rcv_timeo));
  val = fcntl(*sockfd, F_GETFL, 0);
  fcntl(*sockfd, F_SETFL, val | O_NONBLOCK);
  /* set bit in os_params */
  FD_SET(*sockfd, &(os_params.fds));
  os_params.max_fd = MAX(os_params.max_fd, *sockfd);

  return QTI_SUCCESS;
}


/*===========================================================================
  FUNCTION qti_cmd_process
=============================================================================
  @brief
    Process the cmd received in a socket

  @input
    int fd

  @return

  @dependencies
    none

  @sideefects
  None
===========================================================================*/
int qti_cmd_process
(
  int fd
)
{
   qti_event_t *             event_buf=NULL;
   int                       nbytes = 0;
   char                      buf[QTI_MAX_BUF_LEN];
   struct sockaddr_storage   buf_addr;
   socklen_t                 addr_len=sizeof(struct sockaddr_storage);
   char                      buffer;

  if ( ( nbytes = recvfrom(fd, buf, QTI_MAX_BUF_LEN-1 , 0, (struct sockaddr *)&buf_addr, &addr_len)) <= 0 )
  {
    if ( nbytes == 0 )
    {
      LOG_MSG_INFO1("Completed full recv from a callback", 0, 0, 0);
    }
    else
    {
      LOG_MSG_ERROR("recvfrom returned error, errno:%d", errno, 0, 0);
    }
  }

  else
  {
    event_buf = (qti_event_t *)buf;

    if (fd == qti_dpm_sockfd)
    {
      if(event_buf != NULL)
      {
        if(event_buf->event == QTI_DPM_INIT_EVENT)
        {
          qti_rmnet_modem_in_service();
        }
        else if(event_buf->event == QTI_RMNET_DPM_MODEM_NOT_IN_SERVICE_EVENT)
        {
          qti_rmnet_modem_not_in_service();
        }
      }
    }
  }
  return QTI_SUCCESS;
}

/*==========================================================================

FUNCTION MAIN()

DESCRIPTION

  The main function for QTI which is first called when QTI gets started on
  boot up.

DEPENDENCIES
  None.

RETURN VALUE
  0 on SUCCESS
  -1 on FAILURE

SIDE EFFECTS
  None

==========================================================================*/
int main(int argc, char ** argv)
{
  int                       ret_val,i;
  uint8_t                   embd_mode = FALSE;
  char                      command[QTI_MAX_COMMAND_STR_LEN];
  static char               data_mode_value[QTI_PROPERTY_VALUE_MAX];
  char                      data_mode_default_value[PROPERTY_DATA_MODE_SIZE+1];
  int                       fusion_dpl_mode_only = 0;
  FILE                      *fp = NULL;
  int                       inotify_fd;
  uint32_t                  mask;
  int                       wd;
  char                      ro_baseband_value[QTI_MAX_COMMAND_STR_LEN];
  char                      ro_baseband_default_value[QTI_PROPERTY_VALUE_MAX];
  int                       rval;

/*------------------------------------------------------------------------*/

  file_log_mode = FALSE;
  hex_dumps_qxdm_mode = FALSE;
  hex_dumps_file_mode = FALSE;

/*-----------------------------------------------------------------------
    Initialize QTI variables
------------------------------------------------------------------------*/
  memset(&sk_fdset, 0, sizeof(qcmap_sk_fd_set_info_t));
  memset(&qti_conf, 0, sizeof(qti_conf_t));
  memset(&rmnet_config_param, 0, sizeof(qti_rmnet_param));
  memset(&dpl_config_param, 0, sizeof(qti_dpl_param));
  /*-----------------------------------------------------------------------
    Check for fusion targets
  ------------------------------------------------------------------------*/
  memset(ro_baseband_value, 0, sizeof(ro_baseband_value));
  memset(ro_baseband_default_value, 0x0, sizeof(ro_baseband_default_value) );
/*-----------------------------------------------------------------------
  Identify target in use
------------------------------------------------------------------------*/
  rmnet_config_param.target = ds_get_target();
  dpl_config_param.target = ds_get_target();

  snprintf( ro_baseband_default_value, sizeof(ro_baseband_default_value)-1, "%s", "" );
  ret_val = property_get( QTI_BASEBAND_PROP, ro_baseband_value, ro_baseband_default_value );

  if (ret_val > (int)(sizeof(QTI_BASEBAND_PROP_VAL)))
  {
    LOG_MSG_ERROR("Android property for baseband has unexpected size(%d)",
                   ret_val, 0, 0);
  }
  if(strncmp(ro_baseband_value, QTI_BASEBAND_PROP_VAL, strlen(QTI_BASEBAND_PROP_VAL)) == 0)
  {
    if (DS_TARGET_FUSION_SM8150_SDX50 != rmnet_config_param.target)
    {
      is_fusion_target = TRUE;
      /*Currently on Fusion SW path is NOT enabled, have a way to detect it and enable this accordingly*/
      /*TODO: implement IPA IOCTL for EP info. If query fails or returns error => it is RMNET SW path else HW path */
      is_fusion_sw_path = FALSE;
      LOG_MSG_INFO1("QTI:Fusion functionality enabled ", 0, 0, 0);
    }
    else
    {
      LOG_MSG_INFO1("QTI:Fusion device acting as MSM functionality", 0, 0, 0);
    }
  }
/*-------------------------------------------------------------------------
 qti invoked in embedded mode, during this mode qti does not initialize rndis
 and ecm.
-------------------------------------------------------------------------*/

  for(i = 1; i < argc; i++)
  {
    if(0 == strncasecmp(argv[i],"em",strlen("em")))
    {
      system("echo QTI embd_mode > /dev/kmsg");
      embd_mode = TRUE;
    }

    if(0 == strncasecmp(argv[i],"-d",strlen("-d")))
    {
      file_log_mode = TRUE;

/*-------------------------------------------------------------------------
Delete the LOGs.txt file if present
-------------------------------------------------------------------------*/
      snprintf(command, QTI_MAX_COMMAND_STR_LEN, "rm %s", LOG_FILE);
      ds_system_call(command, strlen(command));

      fp_logs = fopen( LOG_FILE, "w" );
      if(fp_logs == NULL)
      {
        printf(" Error in opening /data/LOGs.txt\n");
        return QTI_FAILURE;
      }

      fprintf( fp_logs, "QTI LOGGING IN /data/LOGs.txt \n" );

/*-------------------------------------------------------------------------
Fetch the file descriptor from file pointer
-------------------------------------------------------------------------*/
      qtilogfd = fileno(fp_logs);
      if( qtilogfd == QTI_FAILURE )
      {
        printf( " File /data/LOGs.txt is not open, errno: %d\n",errno );
        fclose ( fp_logs );
        return QTI_FAILURE;
      }

      if(fcntl(qtilogfd, F_SETFD, FD_CLOEXEC) < 0)
      {
        printf( "Couldn't set Close on Exec, errno: %d", errno );
        fclose ( fp_logs );
        return QTI_FAILURE;
      }
    }
  }

/*-------------------------------------------------------------------------
  Initializing Diag for QXDM logs
-------------------------------------------------------------------------*/
  if (TRUE != Diag_LSM_Init(NULL))
  {
     printf("Diag_LSM_Init failed !!");
  }

/*-------------------------------------------------------------------------
  Read the hex dumps options in the qti folder
-------------------------------------------------------------------------*/
  fp = fopen(HEX_DUMP_QXDM_FILE, "r");
  if (fp!=NULL)
  {
    hex_dumps_qxdm_mode = TRUE;
    fclose(fp);
  }
  fp = fopen(HEX_DUMP_LOG_FILE, "r");
  if (fp!=NULL)
  {
    hex_dumps_file_mode = TRUE;
    fclose(fp);
  }

  HEXDUMP_LOG_MSG("QTI Start \n");

  inotify_fd= inotify_init();
  mask = IN_CREATE | IN_DELETE;

  if (inotify_fd < 0)
  {
    LOG_MSG_ERROR("Error in inotify_init for hex dumps", 0, 0, 0);
  }
  else
  {
    wd = inotify_add_watch(inotify_fd, HEX_DUMPS_FOLDER, mask);
    if(wd > 0)
    {
      LOG_MSG_INFO1("Successfully got the watch descriptor fd:%d for the hex dumps", inotify_fd, 0, 0);
      if( qcmap_nl_addfd_map(&sk_fdset,
                             inotify_fd,
                             hex_dumps_reset,
                             MAX_NUM_OF_FD) != QCMAP_UTIL_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to map inotify fd with the hex dumps reset function",0,0,0);
        inotify_rm_watch(inotify_fd, wd);
        close(inotify_fd);
        inotify_fd = 0;
        return QCMAP_UTIL_FAILURE;
      }
    }
    else
      LOG_MSG_ERROR("Failed to add a watch to the hex dumps folder",0,0,0);
  }

  LOG_MSG_INFO1("Start QTI", 0, 0, 0);

/*-----------------------------------------------------------------------
    Identify target in use
------------------------------------------------------------------------*/
  rmnet_config_param.target = ds_get_target();
  dpl_config_param.target = ds_get_target();

  if( DS_TARGET_FUSION_SDM845_SDX24 == rmnet_config_param.target ||
      DS_TARGET_FUSION4_5_PCIE == rmnet_config_param.target ||
      DS_TARGET_OFF_TARGET == rmnet_config_param.target ||
      DS_TARGET_FUSION_8084_9X45 == rmnet_config_param.target ||
      DS_TARGET_FUSION_8096_9X55 == rmnet_config_param.target ||
      is_fusion_target)
  {
    if(DS_TARGET_FUSION_SDM845_SDX24 == rmnet_config_param.target ||
       is_fusion_target)
    {

      if(is_fusion_target){
          memcpy(rmnet_config_param.ph_iface[PH_DRIVER_TYPE_USB].ph_iface_device_file,
                 RMNET_USB_DEV_FILE,
                 strlen(RMNET_USB_DEV_FILE));
      }
      else{
       memcpy(rmnet_config_param.ph_iface[PH_DRIVER_TYPE_USB].ph_iface_device_file,
              RMNET_USB_DEV_NODE,
              strlen(RMNET_USB_DEV_NODE));
       }
    }
    else{
       memcpy(rmnet_config_param.ph_iface[PH_DRIVER_TYPE_USB].ph_iface_device_file,
              RMNET_USB_DEV_FILE,
              strlen(RMNET_USB_DEV_FILE));
       memcpy(rmnet_config_param.modem_iface_device_file,
              RMNET_MHI_DEV_FILE,
              strlen(RMNET_MHI_DEV_FILE));
    }
    memcpy(rmnet_config_param.ph_iface[PH_DRIVER_TYPE_USB].ph_data_iface_name,
           USB_DATA_INTERFACE,
           strlen(USB_DATA_INTERFACE));
    memcpy(rmnet_config_param.modem_data_iface_name,
           MHI_DATA_INTERFACE,
           strlen(MHI_DATA_INTERFACE));
    memcpy(dpl_config_param.dpl_iface_device_file,
           DPL_USB_DEV_FILE,
           strlen(DPL_USB_DEV_FILE));

    rmnet_config_param.qmux_conn_id = QMI_CONN_ID_RMNET_MHI_1;
  }
  else
  {
    memcpy(rmnet_config_param.ph_iface[PH_DRIVER_TYPE_USB].ph_iface_device_file,
           RMNET_USB_DEV_FILE,
           strlen(RMNET_USB_DEV_FILE));
    memcpy(rmnet_config_param.ph_iface[PH_DRIVER_TYPE_MHI].ph_iface_device_file,
           RMNET_MHI_PH_DEV_FILE,
           strlen(RMNET_MHI_PH_DEV_FILE));
    memcpy(rmnet_config_param.modem_iface_device_file,
           RMNET_SMD_DEV_FILE,
           strlen(RMNET_SMD_DEV_FILE));
    memcpy(dpl_config_param.dpl_iface_device_file,
             DPL_USB_DEV_FILE,
             strlen(DPL_USB_DEV_FILE));
    rmnet_config_param.qmux_conn_id = QMI_CONN_ID_RMNET_8;
  }

#ifndef FEATURE_MDM_LE
  if( DS_TARGET_FUSION4_5_PCIE == rmnet_config_param.target ||
      DS_TARGET_OFF_TARGET == rmnet_config_param.target ||
      DS_TARGET_FUSION_8084_9X45 == rmnet_config_param.target ||
      DS_TARGET_FUSION_8096_9X55 == rmnet_config_param.target ||
      (is_fusion_target && is_fusion_sw_path) )
  {
    memset(data_mode_value, 0, sizeof(data_mode_value));
    memset( data_mode_default_value, 0x0, sizeof(data_mode_default_value) );

    snprintf( data_mode_default_value, sizeof(data_mode_default_value)-1, "%s", "" );
    ret_val = property_get( QTI_PROPERTY_DATA_MODE, data_mode_value, data_mode_default_value );

    if (ret_val > (int)(PROPERTY_DATA_MODE_SIZE))
    {
      LOG_MSG_ERROR("Android property for data mode has unexpected size(%d)",
                    ret_val, 0, 0);
    }

    if(strncmp(data_mode_value, TETHERED_VAL, strlen(TETHERED_VAL)) == 0)
    {
      fusion_dpl_mode_only = 0;
      LOG_MSG_INFO1("QTI:Not DPL only mode", 0, 0, 0);
    }
    else
    {
      fusion_dpl_mode_only = 1;
      LOG_MSG_INFO1("QTI:DPL only mode", 0, 0, 0);
    }
  }
#endif //FEATURE_MDM_LE
  LOG_MSG_INFO1("DPL mode only %d", fusion_dpl_mode_only, 0, 0);


/*---------------------------------------------------------------------
  Initialize QTI peripheral interface
---------------------------------------------------------------------*/

  if(!fusion_dpl_mode_only)
  {
    ret_val = qti_rmnet_ph_init(&rmnet_config_param,
                                &sk_fdset,
                                qti_rmnet_ph_recv_msg);
    if(ret_val != QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to initialize QTI peripheral interface",
                    0, 0, 0);
      return QTI_FAILURE;
    }

  /* Create dpm -> qti client socket */
  if (create_socket(&dpm_qti_sockfd) != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("Error creating dpm_qti_sockfd client socket", 0, 0, 0);
  }

  /* Create qti -> dpm server socket */
  rval = create_server_socket(&qti_dpm_sockfd, QTI_DPM_UDS_FILE);

  if (rval != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("Unable to create qti dpm server socket!", 0,0,0);
    return QTI_FAILURE;
  }

  if( qcmap_nl_addfd_map(&sk_fdset,
                         qti_dpm_sockfd,
                         qti_cmd_process,
                         MAX_NUM_OF_FD) != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("Unable to map the server socket fd to "
                  "cmd process function", 0,0,0);
    close(qti_dpm_sockfd);
    return QTI_FAILURE;
  }


/*---------------------------------------------------------------------
  Initialize DPM client
---------------------------------------------------------------------*/
    /*For newer fusion targets, let it go in to DPM notifier init*/
    if(DS_TARGET_FUSION4_5_PCIE != rmnet_config_param.target &&
       DS_TARGET_FUSION_8084_9X45 != rmnet_config_param.target &&
       DS_TARGET_FUSION_8096_9X55 != rmnet_config_param.target)
    {

      ret_val = qti_rmnet_modem_init(&rmnet_config_param,
                                     &dpl_config_param);
      if(ret_val != QTI_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to initialize Modem",
                      0, 0, 0);
        return QTI_FAILURE;
      }
      ret_val = qti_dpm_init(&rmnet_config_param,
                             &dpl_config_param);

      if(ret_val != QTI_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to initialize DPM",
                      0, 0, 0);
        return QTI_FAILURE;
      }
    }
  }
#ifdef FEATURE_MDM_LE

  if( !embd_mode )
  {
    /*-----------------------------------------------------------------------
      Initialize QTI interfaces for netlink events
      ------------------------------------------------------------------------*/
    ret_val = qti_netlink_init(&qti_conf);

    if(ret_val != QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to initialize netlink interfaces for QTI",
          0, 0, 0);
      return QTI_FAILURE;
    }

    /*---------------------------------------------------------------------
      Call into the netlink listener init function which sets up QTI to
      listen to netlink events
      ---------------------------------------------------------------------*/
    if (qcmap_nl_listener_init(
                                 NETLINK_ROUTE,
                                 RTMGRP_LINK,
                                 &sk_fdset,
                                 qti_nl_recv_msg,
                                 &sk_info,
                                 MAX_NUM_OF_FD) == QCMAP_UTIL_SUCCESS)
    {
      LOG_MSG_INFO1("Open netlink socket succeeds",0,0,0);
    }
    else
    {
      LOG_MSG_ERROR("Netlink socket open failed",0,0,0);
      return QTI_FAILURE;
    }
    /*-----------------------------------------------------------------------
      Initialize QTI to be a client of QCMAP
      ------------------------------------------------------------------------*/
    ret_val = qti_qcmap_init(&qti_conf);
    if(ret_val != QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to post qcmap init command to cmd_queue",
          0, 0, 0);
      return QTI_FAILURE;
    }
  }

#endif //FEATURE_MDM_LE
  if(!fusion_dpl_mode_only)
  {
    if( (DS_TARGET_FUSION_SDM845_SDX24 == rmnet_config_param.target) ||
        (DS_TARGET_FUSION4_5_PCIE == rmnet_config_param.target) ||
        (DS_TARGET_OFF_TARGET == rmnet_config_param.target) ||
        (DS_TARGET_FUSION_8084_9X45 == rmnet_config_param.target) ||
        (DS_TARGET_FUSION_8096_9X55 == rmnet_config_param.target) ||
        (DS_TARGET_JOLOKIA == rmnet_config_param.target) ||
        (DS_TARGET_LYKAN == rmnet_config_param.target) ||
        (DS_TARGET_FEERO == rmnet_config_param.target) ||
         is_fusion_target)
    {
      ret_val = qti_rmnet_qmi_init(&rmnet_config_param);
      if(ret_val != QTI_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to initialize QMI message handler",
              0, 0, 0);
        return QTI_FAILURE;
      }
      if(is_fusion_sw_path)
      {
        qti_rmnet_data_init(&rmnet_config_param);
      }
    }
  }

/*---------------------------------------------------------------------
  Initialize QTI peripheral interface for DPL logging
---------------------------------------------------------------------*/
  if( !((DS_TARGET_MSMNILE == dpl_config_param.target) ||
        (DS_TARGET_FUSION_SM8150_SDX50 == dpl_config_param.target) ||
        (DS_TARGET_LITO == dpl_config_param.target) ||
        (DS_TARGET_MSMSTEPPE == dpl_config_param.target) ||
        is_fusion_target))
  {
    ret_val = qti_dpl_ph_init(&dpl_config_param,
                              &sk_fdset,
                              qti_dpl_ph_recv_msg);
    if(ret_val != QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to initialize QTI peripheral interface for DPL logging",
                     0, 0, 0);
    }
  }

/*--------------------------------------------------------------------
  Start the listener which listens to netlink events and QMI packets
  coming on USB-Rmnet device file
---------------------------------------------------------------------*/
  ret_val = qcmap_listener_start(&sk_fdset);

  if(ret_val != QCMAP_UTIL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to start NL listener",
                   0, 0, 0);
  }
/*--------------------------------------------------------------------
  Wait for the QTI command queue to finish before exiting QTI
---------------------------------------------------------------------*/
#ifdef FEATURE_MDM_LE

  if(!embd_mode)
  {
    qti_qcmap_exit();
  }
#endif //FEATURE_MDM_LE

  fclose(fp_logs);

  return QTI_SUCCESS;
}


