/*=============================================================================
  @file aon_control.c

  @brief controls the always on feature sam algo

  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ===========================================================================*/

/*=============================================================================
  Include Files
  ===========================================================================*/
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <grp.h>
#include <sys/inotify.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/system_properties.h>
#include <fcntl.h>
#include <errno.h>
#include "sensor1.h"
#include "sns_init.h"
#include "sns_fsa.h"
#include "sns_debug_str.h"
#include "sns_common.h"
#include "sns_common_v01.h"
#include "sns_sam_aon_test_v01.h"
#include "libsensor1.h"
/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/
/* Retry a standard library command if the error code is EINTR */
#define  RETRY_ON_EINTR(ret,cond) \
do {                              \
  ret = (cond);                   \
} while (ret < 0 && errno == EINTR)

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define FD_ADD( fd, fdset, max )                \
  do {                                          \
    FD_SET((fd),(fdset));                       \
    (max) = MAX((max),(fd));                    \
  } while(0);


#ifndef SENSORS_SETTINGS_DIR
#define AON_EN_FILENAME         "/mnt/vendor/persist/sensors/aon_enable"
#define AON_EN_FILENAME_LEGACY  "/persist/sensors/aon_enable"
#endif
#define AON_SETTINGS_FILE  "aon_enable"

static char aon_en_file_name[255] = "";

/*============================================================================
  Static Variable Definitions
  ============================================================================*/
static int inotifyfd;
static int uimagealgo_ctrl_file_created;
uint8_t instance_id;

/*============================================================================
  Static Function Definitions
  ============================================================================*/
static int read_setting(bool *en)
{
  int fd = -1;

  if(SNS_SUCCESS == sensor1_has_acessto_directory(AON_EN_FILENAME))
  {
     strlcpy(aon_en_file_name, AON_EN_FILENAME, strlen(AON_EN_FILENAME) + 1);
  }
  else
  {
     strlcpy(aon_en_file_name, AON_EN_FILENAME_LEGACY, strlen(AON_EN_FILENAME_LEGACY) + 1);
  }

  fd = open(aon_en_file_name, O_RDONLY);
  if(fd == -1) {
    SNS_PRINTF_STRING_HIGH_0(SNS_MODULE_APPS_AON,
        "opening the uimage settings file failed");
    return -1;
  } else {
    char buf[2]="";
    if(1 == read(fd,buf,1))
    {
      SNS_PRINTF_STRING_HIGH_1(SNS_MODULE_APPS_AON,"read the control string %c",buf[0]);
      ('1' == buf[0]) ? (*en = true) : (*en = false);
    } else {
      SNS_PRINTF_STRING_HIGH_0(SNS_MODULE_APPS_AON,
        "reading the uimage settings file failed");
      return -1;
    }
  }
  return 0;
}

static void check_and_add_watch()
{
  int fd = -1;
  char aon_en_dir[255] = "";
  fd = open(aon_en_file_name,O_RDONLY);
  if(fd == -1) {
    SNS_PRINTF_STRING_HIGH_1( SNS_MODULE_APPS_AON,
              "/persist/sensors/aon_enable file not present %d", fd );
    /*add watch for creation*/
    strlcpy(aon_en_dir, aon_en_file_name, strlen(aon_en_file_name) - strlen(AON_SETTINGS_FILE) - 1);
    if(-1 == inotify_add_watch(inotifyfd,
                               aon_en_dir,
                               IN_CREATE)) {
        SNS_PRINTF_STRING_ERROR_0(SNS_MODULE_APPS_AON,
              "inotify_add_watch::/persist/sensors failed");
        return ;
    } else {
        SNS_PRINTF_STRING_HIGH_0(SNS_MODULE_APPS_AON,
              "inotify_add_watch::/persist/sensors");
        close(fd);
        return ;
    }
  } else {
    /*control file present watch for changes*/
    uimagealgo_ctrl_file_created = true;
    if(-1 == inotify_add_watch(inotifyfd,
                               aon_en_file_name,
                               IN_DELETE_SELF | IN_CLOSE_WRITE)) {
      SNS_PRINTF_STRING_ERROR_0(SNS_MODULE_APPS_AON,
                "inotify_add_watch::/persist/sensors/aon_enable failed");
      close(fd);
      return;
    } else {
      SNS_PRINTF_STRING_HIGH_0(SNS_MODULE_APPS_AON,
                "inotify_add_watch::/persist/sensors/aon_enable");
      close(fd);
      return ;
    }
  }
}


void process_aon_resp(sensor1_msg_header_s *msg_hdr, void *msg_ptr) {

  const sns_common_resp_s_v01*  crsp_ptr = (sns_common_resp_s_v01 *)msg_ptr;
  bool                          error = false;

  SNS_PRINTF_STRING_HIGH_2(SNS_MODULE_APPS_AON,
            "process_aon_resp Msg_id %d; Result(0:sucess):%u",
            msg_hdr->msg_id, crsp_ptr->sns_result_t);

  if(crsp_ptr->sns_result_t != 0) {
    SNS_PRINTF_STRING_ERROR_3(SNS_MODULE_APPS_AON,
          "process_aon_resp Msg %i; Result: %u, Error: %u",
          msg_hdr->msg_id, crsp_ptr->sns_result_t, crsp_ptr->sns_err_t);
    error = true;
  }

  if(true != error) {
    switch (msg_hdr->msg_id) {
    case SNS_SAM_AON_TEST_ENABLE_RESP_V01:
      instance_id = ((sns_sam_aon_test_enable_resp_msg_v01 *)msg_ptr)->instance_id;
      SNS_PRINTF_STRING_HIGH_0(SNS_MODULE_APPS_AON,
            "Received SNS_SAM_AON_TEST_ENABLE_RESP_V01");
      break;
    case SNS_SAM_AON_TEST_DISABLE_RESP_V01:
      SNS_PRINTF_STRING_HIGH_0(SNS_MODULE_APPS_AON,
            "Received SNS_SAM_AON_TEST_DISABLE_RESP_V01");
      /* Reset instance ID */
      instance_id = 0xFF;
      break;
    default:
      SNS_PRINTF_STRING_ERROR_2(SNS_MODULE_APPS_AON,
            "%s: Unknown msg id: %d",
            __func__, msg_hdr->msg_id );
      return;
    }
  }
}

void process_aon_ind()
{
  /*calculate the time gap between 2 successive indications*/
  struct timeval present_indtime;
  static struct timeval last_indtime;
  float ind_ts_gap;
  gettimeofday(&present_indtime, NULL);
  ind_ts_gap = (((present_indtime.tv_sec - last_indtime.tv_sec) * 1000)
              +((present_indtime.tv_usec - last_indtime.tv_usec) / 1000));

  last_indtime = present_indtime;

  SNS_PRINTF_STRING_HIGH_1(SNS_MODULE_APPS_AON,
          "AON Ind TimeGap %f(ms)" ,ind_ts_gap);
}
void aon_notify_data_cb(intptr_t cb_data,
                            sensor1_msg_header_s *msg_hdr,
                            sensor1_msg_type_e msg_type,
                           void *msg_ptr)
{
  UNREFERENCED_PARAMETER(cb_data);

  SNS_PRINTF_STRING_HIGH_2(SNS_MODULE_APPS_AON,
            "%s and msg_type :%d",
            __func__, msg_type);
  switch(msg_type) {
    case SENSOR1_MSG_TYPE_RESP:
      process_aon_resp(msg_hdr, msg_ptr);
      break;
    case SENSOR1_MSG_TYPE_IND:
        SNS_PRINTF_STRING_HIGH_0(SNS_MODULE_APPS_AON,
          "received SENSOR1_MSG_TYPE_IND");
        process_aon_ind();
      break;
    default:
      break;
  }
}

static void disable_aon(sensor1_handle_s *aon_sam_hndl)
{
  sensor1_error_e error;
  sensor1_msg_header_s msg_hdr;
  SNS_PRINTF_STRING_HIGH_0(SNS_MODULE_APPS_AON, "aon_disable");
  sns_sam_aon_test_disable_req_msg_v01 *disable_msg;
  error = sensor1_alloc_msg_buf(aon_sam_hndl,
                           sizeof(sns_sam_aon_test_disable_req_msg_v01),
                           (void**)&disable_msg );
  if(SENSOR1_SUCCESS != error) {
    SNS_PRINTF_STRING_ERROR_0(SNS_MODULE_APPS_AON,
              "sensor1_alloc_msg_buf failed");
    return;
  }
  msg_hdr.msg_id = SNS_SAM_AON_TEST_DISABLE_REQ_V01;
  msg_hdr.service_number = SNS_SAM_AON_TEST_SVC_ID_V01;
  msg_hdr.txn_id = 0;
  msg_hdr.msg_size = sizeof(sns_sam_aon_test_disable_req_msg_v01);
  disable_msg->instance_id = instance_id;
  error = sensor1_write(aon_sam_hndl,&msg_hdr,disable_msg);
  if(SENSOR1_SUCCESS != error) {
    SNS_PRINTF_STRING_ERROR_0(SNS_MODULE_APPS_AON,
              "sensor1_write failed");
    sensor1_free_msg_buf(aon_sam_hndl,disable_msg);
    return;
  }
  SNS_PRINTF_STRING_HIGH_0(SNS_MODULE_APPS_AON, "disabled:aon");
}

static void enable_aon(sensor1_handle_s *aon_sam_hndl)
{
  sensor1_error_e error;
  sensor1_msg_header_s msg_hdr;
  SNS_PRINTF_STRING_HIGH_0(SNS_MODULE_APPS_AON, "aon_enable");
  sns_sam_aon_test_enable_req_msg_v01 *req_msg;
  /*at present sns_sam_aon_test_enable_req_msg_v01 is empty structure*/
  error = sensor1_alloc_msg_buf(aon_sam_hndl,
                           sizeof(sns_sam_aon_test_enable_req_msg_v01),
                           (void**)&req_msg );
  if(SENSOR1_SUCCESS != error) {
    SNS_PRINTF_STRING_ERROR_0(SNS_MODULE_APPS_AON,
              "sensor1_alloc_msg_buf failed");
    return;
  }
  msg_hdr.msg_id = SNS_SAM_AON_TEST_ENABLE_REQ_V01;
  msg_hdr.service_number = SNS_SAM_AON_TEST_SVC_ID_V01;
  msg_hdr.txn_id = 0x0;
  msg_hdr.msg_size = sizeof(sns_sam_aon_test_enable_req_msg_v01);
  error = sensor1_write(aon_sam_hndl,&msg_hdr,req_msg);
  if(SENSOR1_SUCCESS != error) {
    SNS_PRINTF_STRING_ERROR_0(SNS_MODULE_APPS_AON,
              "sensor1_write failed");
    sensor1_free_msg_buf(aon_sam_hndl,req_msg);
    return;
  }
  SNS_PRINTF_STRING_HIGH_0(SNS_MODULE_APPS_AON, "enabled:aon");
}

static void aon_ctrl_task(void *arg)
{
  UNREFERENCED_PARAMETER(arg);
  bool en,read_settings = true;
  sensor1_handle_s *aon_sam_hndl;

  if(SENSOR1_SUCCESS != sensor1_open(&aon_sam_hndl, aon_notify_data_cb, 0)) {
    SNS_PRINTF_STRING_ERROR_0(SNS_MODULE_APPS_AON, "sensor1_open failed for aon");
    return;
  }

  for(;;) {
    int sel_err;
    int max_fd = 0;
    fd_set read_fd;
    fd_set error_fd;
    /*IN_MODIFY  0x00000002 */
    int flags = IN_MODIFY|IN_CLOSE_WRITE|IN_DELETE_SELF;

    FD_ZERO( &read_fd );
    FD_ZERO( &error_fd );
    FD_ADD( inotifyfd, &read_fd, max_fd );

    error_fd = read_fd;
    max_fd++;
    errno = 0;

    /*read the settings file first time by default and wait on select further*/
    if(true == read_settings){
      if(!read_setting(&en)){
        if(en)
          enable_aon(aon_sam_hndl);
        else
          disable_aon(aon_sam_hndl);
        }
    }


    check_and_add_watch();

    read_settings = false;

    SNS_PRINTF_STRING_HIGH_1(SNS_MODULE_APPS_AON,
                              "%s: wait on select", __func__);
    RETRY_ON_EINTR( sel_err, select( max_fd, &read_fd, (fd_set*)NULL, &error_fd,
                                     (struct timeval*) NULL) );
    SNS_PRINTF_STRING_HIGH_1(SNS_MODULE_APPS_AON,
                               "%s: come out of select", __func__);
    if( sel_err < 0 ) {
      SNS_PRINTF_STRING_ERROR_2(SNS_MODULE_APPS_AON,
                                "%s: Error on select %i", __func__, sel_err);
    } else if( FD_ISSET( inotifyfd, &read_fd ) ) {
      /* Change in settings file */
      char buf[500];
      struct inotify_event *evt = (struct inotify_event*)buf;
      read(inotifyfd, evt, 500);
      if(evt->mask & IN_IGNORED) {
          /* Previous watch was removed. Nothing to do here */
          SNS_PRINTF_STRING_HIGH_2(SNS_MODULE_APPS_AON,
                     "%s: intofyevent:IGNORE 0x%x",
                    __func__, evt->mask);
      } else if((evt->mask & IN_CREATE) &&
                   (0 == strncmp( evt->name, AON_SETTINGS_FILE, evt->len))) {
          inotify_rm_watch( inotifyfd, evt->wd );
          SNS_PRINTF_STRING_HIGH_2(SNS_MODULE_APPS_AON,
                          "%s: intofyevent:IN_CREATE 0x%x",
                           __func__, evt->mask);
          check_and_add_watch();
      } else if ( (evt->mask & flags) ) {
          SNS_PRINTF_STRING_HIGH_2(SNS_MODULE_APPS_AON,
                             "%s: received one of the expected intofyevent:0x%x ",
                             __func__, evt->mask);
          read_settings = true;
          /*read the settings file information*/
          continue;
      } else {
        SNS_PRINTF_STRING_HIGH_2(SNS_MODULE_APPS_AON,
                                "%s: intofyevent:0x%x ", __func__, evt->mask);
      }
    }
  }
}

/*Support required only for MTP and QRDs*/
bool supported_target() {
  bool ret_val = false;
#ifdef FEATURE_SNS_AON
  FILE *fp;
  static char platform_string[20] = "";
  static const char invalid_str[] = "Invalid file";

  fp = sns_fsa_open("/sys/devices/soc0/hw_platform", "r" );
  if( fp == NULL )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_MODULE_APPS_AON,
                               "hw_platform fopen failed %i", errno );
    strlcpy(platform_string, invalid_str, sizeof(platform_string));
  }
  else if( fgets(platform_string, sizeof(platform_string), fp) == NULL )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_MODULE_APPS_AON,
                               "hw_platform fgets failed %i: %i", errno );
  }
  if( fp != NULL )
  {
    sns_fsa_close( fp );
  }

  SNS_PRINTF_STRING_HIGH_1(SNS_MODULE_APPS_AON,
                          "platform_string : %s",
                          platform_string);

  /*check for MTP or QRD*/
  if(!(strncmp("MTP", platform_string, 3))
      || (!strncmp("QRD", platform_string, 3)))
      ret_val = true;
#endif
  return ret_val;
}

sns_err_code_e sns_aon_algo_init() {
  bool en;
/*specific to 8998 so check if required or not*/
  if(true == supported_target()) {
    inotifyfd = -1;
    inotifyfd = inotify_init();
    if(inotifyfd == -1) {
        SNS_PRINTF_STRING_ERROR_1( SNS_MODULE_APPS_AON,
                    "inotify_init failed %i", inotifyfd );
        return -1;
    }
    read_setting(&en);
    check_and_add_watch();
    sns_os_task_create(aon_ctrl_task, NULL, NULL, 0);
  }
  sns_init_done();
  return SNS_SUCCESS;
}
