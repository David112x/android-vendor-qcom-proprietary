/*==========================================================================
Description
  It has implementation for ring buffer.

# Copyright (c) 2017 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#include <fcntl.h>
#include <errno.h>
#include <mutex>
#include <unistd.h>
#include "hci_internals.h"
#include "ibs_handler.h"
#include "health_info_log.h"
#include <utils/Log.h>
#include "state_info.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "vendor.qti.bluetooth@1.0-health-info"

#define HEALTH_INFO_PATH_BUF_SIZE 255
#define HEALTH_INFO_PATH        "/data/vendor/bluetooth"
#define MAX_HEALTH_INFO_DUMP_COUNT  100
#define HEALTH_STATISTICS_TIMER_VALUE_MS (15*60*1000);
namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

enum HealthInfoTimerState {
  TIMER_NOT_CREATED = 0x00,
  TIMER_CREATED = 0x01,
  TIMER_ACTIVE = 0x02
};

HealthInfoLog* HealthInfoLog::instance = NULL;
int HealthInfoLog::max_interval_rx_sleep_wake = 0;
int HealthInfoLog::max_interval_tx_packet = 0;
int HealthInfoLog::count_wake_sent = 0;
int HealthInfoLog::count_wake_received = 0;
int HealthInfoLog::count_sleep_sent = 0;
int HealthInfoLog::count_sleep_received = 0;
uint8_t HealthInfoLog::tx_vote_ = 0;
uint8_t HealthInfoLog::rx_vote_ = 0;


HealthInfoLog::HealthInfoLog():
    health_statistics_timer_state(TIMER_NOT_CREATED)
{
  instance = this;
  max_interval_rx_sleep_wake = 0;
  max_interval_tx_packet = 0;
  count_wake_sent = 0;
  count_wake_received = 0;
  count_sleep_sent = 0;
  count_sleep_received = 0;
  tx_vote_ = 0;
  rx_vote_ = 0;
}

HealthInfoLog::~HealthInfoLog()
{
#ifdef ENABLE_HEALTH_TIMER
  HealthStatisticsTimerCleanup();
#endif
  instance = NULL;
}

HealthInfoLog * HealthInfoLog::Get()
{
  return instance;
}

void HealthInfoLog::UpdateIntervalRxSleepWake(int interval)
{
  max_interval_rx_sleep_wake = max_interval_rx_sleep_wake > interval ?
                               max_interval_rx_sleep_wake : interval;
}

void HealthInfoLog::UpdateIntervalTxPackets(int interval)
{
  max_interval_tx_packet = max_interval_tx_packet > interval ?
			   max_interval_tx_packet : interval;
}

void HealthInfoLog::UpdateTxVote(uint8_t vote)
{
  tx_vote_ = vote;
}

void HealthInfoLog::UpdateRxVote(uint8_t vote)
{
  rx_vote_ = vote;
}

int HealthInfoLog::GetStatInfoDumpIndex() {
  static int dumpFd = -1;
  unsigned char inx_scan;
  static char path[HEALTH_INFO_PATH_BUF_SIZE];

  /* Scan for present crash dump file */
  for (inx_scan = 0; inx_scan < MAX_HEALTH_INFO_DUMP_COUNT; inx_scan++) {
    snprintf(path, HEALTH_INFO_PATH_BUF_SIZE, "%s/%.02d-stat_info.txt",
                   HEALTH_INFO_PATH,inx_scan);
    ALOGV("%s: try to open file : %s", __func__, path);
    dumpFd = open(path, O_RDONLY);
    if (dumpFd < 0 && (errno == ENOENT)) {
        ALOGV("%s: file( %s) can be used", __func__, path);
        break;
    }
    close(dumpFd);
  }
  if(inx_scan >= MAX_HEALTH_INFO_DUMP_COUNT)
      inx_scan = 0;

  return inx_scan;
}

void HealthInfoLog::CommitStatInfoBuff() {
#ifdef DUMP_HEALTH_INFO_TO_FILE
  char dpath[255];
  int dfd, ret;
  int swap_dfd;
  int inx_oldest;
  int inx;
  ALOGV(">%s", __func__);

  inx = GetStatInfoDumpIndex();

  /* Find index of the oldest out ring buffer entry */
  inx_oldest = (inx + 1) % MAX_HEALTH_INFO_DUMP_COUNT;

  snprintf((char *)dpath, 255, "/data/misc/bluetooth/%.02d-stat_info.txt",
                          inx_oldest);
  remove(dpath);

  snprintf((char *)dpath, 255, "/data/misc/bluetooth/%.02d-stat_info.txt",
                            inx);
  remove(dpath);

  ALOGD("%s: Dumping statistics info buffer in %s", __func__, dpath);

  dfd = open((char *)dpath, O_CREAT| O_SYNC | O_WRONLY,  S_IRUSR |S_IWUSR |S_IRGRP );
  if (dfd < 0) {
     ALOGE("%s: Error opening destination file %d (%s)", __func__, dfd, strerror(errno));
     return;
  }

  std::unique_lock<std::mutex> guard(health_info_mutex_);
  DumpStatInfo(dfd);
  if (fsync(dfd) < 0 ) {
     ALOGE("%s: File flush (%s) failed: %s, errno: %d", __func__,
      dpath, strerror(errno), errno);
  }
  close(dfd);
  ALOGV("<%s", __func__);
#endif /*DUMP_HEALTH_INFO_TO_FILE */
}

void HealthInfoLog::ReportHealthInfo()
{
  char stat_buf[512];
  unsigned int temp, second, minute, hour;
  struct timeval tv;

  gettimeofday(&tv, NULL);
  temp = tv.tv_sec;
  second = temp%60;
  temp /= 60;
  minute = temp%60;
  temp /= 60;
  hour = temp%24;

  bool lock_held = Wakelock::GetWakeLockStatus();

  snprintf(stat_buf, 512, "ts <%02d:%02d:%02d:%06d> rx_vote:%0x, tx_vote:%0x, wake_lock holding:%d, "
            "maximum rx sleep-wake-interval:%dsec, maximum tx packet-interval:%dsec"
            "Wake_Req sent out:%d, Wake_Req received:%d"
            "Sleep_Ind sent out:%d, Sleep_Ind received:%d\n",
            hour, minute, second, (unsigned int)tv.tv_usec,  rx_vote_,
            tx_vote_, lock_held, max_interval_rx_sleep_wake,
            max_interval_tx_packet, count_wake_sent, count_wake_received,
            count_sleep_sent, count_sleep_received);

  ALOGD("%s:statistics:\n%s",__FUNCTION__, stat_buf);
  BtState :: Get()->SetLastStatsInfo(stat_buf);
  //Reset statistics variables
  max_interval_rx_sleep_wake = max_interval_tx_packet = 0;
  count_wake_sent = count_wake_received = 0;
  count_sleep_sent = count_sleep_received = 0;
  rx_vote_ = tx_vote_ = 0;
}

int HealthInfoLog::HealthStatisticsTimerStart()
{
    int status;
    struct itimerspec ts;
    struct sigevent se;
    uint32_t timeout_ms;
    ALOGV("%s",__FUNCTION__);
    if(health_statistics_timer_state == TIMER_NOT_CREATED)
    {
        se.sigev_notify_function = (void (*)(union sigval))ReportHealthInfo;
        se.sigev_notify = SIGEV_THREAD;
        se.sigev_value.sival_ptr = &health_statistics_timer;
        se.sigev_notify_attributes = NULL;

        status = timer_create(CLOCK_MONOTONIC, &se, &health_statistics_timer);
        if (status == 0)
        {
            ALOGV("%s: Health statistics timer created",__FUNCTION__);
            health_statistics_timer_state = TIMER_CREATED;
        } else {
            ALOGE("%s: Error creating health statistics timer %d\n", __func__, status);
        }
    }
    if(health_statistics_timer_state == TIMER_CREATED)
    {
        timeout_ms = HEALTH_STATISTICS_TIMER_VALUE_MS;
        ts.it_value.tv_sec = timeout_ms/1000;
        ts.it_value.tv_nsec = 1000000*(timeout_ms%1000);
        ts.it_interval.tv_sec = timeout_ms/1000;
        ts.it_interval.tv_nsec = 1000000*(timeout_ms%1000);

        status = timer_settime(health_statistics_timer, 0, &ts, 0);
        if (status < 0)
            ALOGE("%s:Failed to set health statistics timer: %d",__FUNCTION__, status);
        else {
            ALOGV("%s: time started", __func__);
            health_statistics_timer_state = TIMER_ACTIVE;
        }
    }
    return health_statistics_timer_state;
}

bool HealthInfoLog::HealthStatisticsTimerStop()
{
    int status;
    struct itimerspec ts;
    bool was_active = false;
    ALOGV("%s",__FUNCTION__);

    if(health_statistics_timer_state == TIMER_ACTIVE)
    {
        ts.it_value.tv_sec = 0;
        ts.it_value.tv_nsec = 0;
        ts.it_interval.tv_sec = 0;
        ts.it_interval.tv_nsec = 0;
        was_active = true;
        status = timer_settime(health_statistics_timer, 0, &ts, 0);
        if (status == -1)
            ALOGE("%s:Failed to stop health statistics timer",__FUNCTION__);
        else if (status == 0) {
             ALOGV("%s: Health_statistics timer Stopped",__FUNCTION__);
             health_statistics_timer_state = TIMER_CREATED;
        }
    }
    return was_active;
}

void HealthInfoLog::HealthStatisticsTimerCleanup() {
   if (health_statistics_timer_state == TIMER_ACTIVE)
     HealthStatisticsTimerStop();
   if (health_statistics_timer_state == TIMER_CREATED) {
     timer_delete(health_statistics_timer);
     health_statistics_timer_state = TIMER_NOT_CREATED;
     health_statistics_timer = NULL;
   }
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android
