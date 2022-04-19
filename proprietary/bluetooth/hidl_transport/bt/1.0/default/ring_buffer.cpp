/*==========================================================================
Description
  It has implementation for ring buffer.

# Copyright (c) 2017 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#include <string.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#include "ring_buffer.h"
#include "logger.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "vendor.qti.bluetooth@1.0-ring_buffer"

int64_t gmt_offset;
uint8_t ibs_pkt[IBS_PKT_SIZE] = {'\0'};
uint64_t timestamp_us;
time_t t;
struct tm tm_cur;

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

#define IN_RINGBUFF_EVENTS_SIZE (192)

void RingBuffer::AddLog(short int dir, HciPacketType type, uint8_t *pkt, uint16_t len)
{
  std::unique_lock<std::mutex> guard(snoop_ring_buffer_mutex_);
  snoop_.AddBuffNode(dir, type, len, pkt);
}

void RingBuffer::UnlockMutex()
{
  ALOGE("%s: unlocking snoop ringbuffer lock", __func__);
  snoop_ring_buffer_mutex_.unlock();
}

void RingBuffer::UpdateRxEventTag(RxThreadEventType rx_event)
{
  switch (rx_event) {
    case RX_PRE_STACK_EVT_CALL_BACK:
      gettimeofday(&time_pre_stack_event_call_back, NULL);
      break;
    case RX_POST_STACK_EVT_CALL_BACK:
      gettimeofday(&time_post_stack_event_call_back, NULL);
      break;
    case RX_PRE_STACK_ACL_CALL_BACK:
      gettimeofday(&time_pre_stack_acl_call_back, NULL);
      break;
    case RX_POST_STACK_ACL_CALL_BACK:
      gettimeofday(&time_post_stack_acl_call_back, NULL);
      break;
    case RX_PRE_SELECT_CALL_BACK:
      gettimeofday(&time_pre_select_call_back, NULL);
      break;
    case RX_POST_SELECT_CALL_BACK:
      gettimeofday(&time_post_select_call_back, NULL);
      break;
    default:
      ALOGE("%s, received other Rx event tag", __func__);
      break;
  }
}

void RingBuffer::SaveLastStackHciCommand(char * cmd)
{
  strlcpy(last_hci_cmd, cmd, HCICMD_TIMESTAMP_LEN);
}

void RingBuffer::AddRxTagEvent(char* dest_tag_str, struct timeval& time_val)
{
  memset(dest_tag_str, 0, RX_TAG_STR_LEN);
  add_time_str(dest_tag_str, &time_val);
}

char* RingBuffer::GetPrimaryReason(PrimaryReasonCode temp)
{
   return snoop_.GetReason(temp);
}

void RingBuffer::DumpData()
{
  char dst_buff[MAX_BUFF_SIZE] = {'\0'};
  char tag_event_call_back[RX_TAG_STR_LEN];

  ALOGD("%s", __func__);

  std::unique_lock<std::mutex> guard(snoop_ring_buffer_mutex_);

  snprintf(dst_buff, MAX_BUFF_SIZE, "TS and last HCI cmd from stack = %s", last_hci_cmd);
  BtState :: Get()->SetLastHciCmdStack(dst_buff);
  ALOGE("%s", dst_buff);

  AddRxTagEvent(tag_event_call_back, time_pre_stack_event_call_back);
  snprintf(dst_buff, MAX_BUFF_SIZE, "TS of posting event to stack %s", tag_event_call_back);
  BtState::Get()->SetPreStackEvent(dst_buff);
  ALOGE("%s", dst_buff);

  AddRxTagEvent(tag_event_call_back, time_post_stack_event_call_back);
  snprintf(dst_buff, MAX_BUFF_SIZE, "TS of event posted to stack %s", tag_event_call_back);
  BtState::Get()->SetPostStackEvent(dst_buff);
  ALOGE("%s", dst_buff);

  AddRxTagEvent(tag_event_call_back,time_pre_stack_acl_call_back);
  snprintf(dst_buff, MAX_BUFF_SIZE, "TS of posting ACL packet to stack %s", tag_event_call_back);
  BtState::Get()->SetPreStackAcl(dst_buff);
  ALOGE("%s", dst_buff);

  AddRxTagEvent(tag_event_call_back, time_post_stack_acl_call_back);
  snprintf(dst_buff, MAX_BUFF_SIZE, "TS of ACL packet posted to stack %s", tag_event_call_back);
  BtState::Get()->SetPostStackAcl(dst_buff);
  ALOGE("%s", dst_buff);

  AddRxTagEvent(tag_event_call_back, time_pre_select_call_back);
  snprintf(dst_buff, MAX_BUFF_SIZE, "TS of pre select %s", tag_event_call_back);
  BtState::Get()->SetPreSelect(dst_buff);
  ALOGE("%s", dst_buff);

  AddRxTagEvent(tag_event_call_back, time_post_select_call_back);
  snprintf(dst_buff, MAX_BUFF_SIZE, "TS of post select %s", tag_event_call_back);
  BtState::Get()->SetPostSelect(dst_buff);
  ALOGE("%s", dst_buff);

  snoop_.DumpData();
}

PacketBuff :: PacketBuff()
{
  memset(buff_index_, 0, sizeof(buff_index_));
  active_buff_ = 0;

  map_code_reason_[REASON_DEFAULT_NONE] =  (char *)"Invalid reason";
  map_code_reason_[REASON_SOC_CRASHED] = (char *)"SOC crashed";
  map_code_reason_[REASON_SOC_CRASHED_DIAG_SSR] =  (char *)"SOC crashed with diag initiated SSR";
  map_code_reason_[REASON_PATCH_DLDNG_FAILED_SOCINIT] =  (char *)"SOC init failed during patch downloading";
  map_code_reason_[REASON_CLOSE_RCVD_DURING_INIT] =  (char *)"Close received from stack during SOC init";
  map_code_reason_[REASON_ERROR_READING_DATA_FROM_UART] =  (char *)"Error reading data from UART";
  map_code_reason_[REASON_WRITE_FAIL_SPCL_BUFF_CRASH_SOC] =  (char *)"Failed to write special bytes to crash SOC";
  map_code_reason_[REASON_RX_THREAD_STUCK] =  (char *)"Rx Thread Stuck";
  map_code_reason_[REASON_SSR_CMD_TIMEDOUT] =  (char *)"SSR due to command timed out";
  map_code_reason_[REASON_SSR_SPURIOUS_WAKEUP] =  (char *)"SSR due to spurious wakeup";
  map_code_reason_[REASON_SSR_INVALID_BYTES_RCVD] =  (char *)"Invalid HCI cmd type received";
  map_code_reason_[REASON_SSR_RCVD_LARGE_PKT_FROM_SOC] =  (char *)"Large packet received from SOC";
  map_code_reason_[REASON_SSR_UNABLE_TO_WAKEUP_SOC] =  (char *)"Unable to wake SOC";
  map_code_reason_[REASON_CMD_TIMEDOUT_SOC_WAIT_TIMEOUT] =  (char *)"Command timedout and SOC crash wait timeout";
  map_code_reason_[REASON_SPURIOUS_WAKEUP_SOC_WAIT_TIMEOUT] =  (char *)"Spurious wake and SOC crash wait timeout";
  map_code_reason_[REASON_INV_BYTES_SOC_WAIT_TIMEOUT] =  (char *)"Invalid bytes received and SOC crash wait timeout";
  map_code_reason_[REASON_SOC_WAKEUP_FAILED_SOC_WAIT_TIMEOUT] =  (char *)"SOC Wakeup failed and SOC crash wait timeout";
  map_code_reason_[REASON_SOC_CRASHED_DIAG_SSR_SOC_WAIT_TIMEOUT] = (char *)"SOC crashed with diag initiated SSR and SOC wait timeout";
  map_code_reason_[REASON_NONE_SOC_WAIT_TIMEOUT] =  (char *)"Invalid Reason and SOC crash wait timeout";
  map_code_reason_[REASON_SOC_DEINIT_STUCK] =  (char *)"SOC Deinit Stuck";
}

PacketBuff::~PacketBuff()
{
  memset(buff_index_, 0, sizeof(buff_index_));
  active_buff_ = 0;
  map_code_reason_.clear();
}

void PacketBuff::AddBuffNode(short int dir, HciPacketType type, uint16_t len, uint8_t *pkt)
{
  uint16_t log_len = snoop_hdr_len + len;
  uint32_t header_length = 0;
  uint32_t flags = 0;

  /* Header length hold the actual length of packet. Flag BIT 0
   * holds direction of packet i.e. 0 for data sent from HOST
   * 1 for received packet. Whereas BIT 1 holds packet type.
   * i.e. either command packet or event packet.
   */

  if (log_len + 2 > max_buff_size_ - buff_index_[active_buff_]) {
    //switch active buffer if current active buffer is full.
    active_buff_ = (active_buff_ + 1) % 2;
    buff_index_[active_buff_] = 0;
    memset(&buff_[active_buff_][0], 0x00, max_buff_size_);
  }

  switch (type) {
    case HCI_PACKET_TYPE_COMMAND:
    case HCI_PACKET_TYPE_FM_CMD:
    case HCI_PACKET_TYPE_ANT_CTRL:
      header_length = pkt[2] + HCI_PACKET_CMD_HDR;
      flags = 2;
      break;
    case HCI_PACKET_TYPE_ACL_DATA:
      header_length = (pkt[3] << 8) + pkt[2] + ACL_PACKET_HDR;
      if (dir == RingBuffer::HOST_TO_SOC)
        flags = 0;
      else
        flags = 1;
      break;
    case HCI_PACKET_TYPE_SCO_DATA:
      header_length = pkt[2] + HCI_PACKET_CMD_HDR;
      flags = 1;
      break;
    case HCI_PACKET_TYPE_EVENT:
    case HCI_PACKET_TYPE_FM_EVENT:
      header_length = pkt[1] + HCI_PACKET_EVT_HDR;
      flags = 3;
      break;
    case HCI_PACKET_TYPE_IBS_CMD:
      header_length = len + HCI_PACKET_CMD_HDR;
      flags = 2;
      break;
    case HCI_PACKET_TYPE_IBS_EVENT:
      header_length = (IBS_PKT_SIZE - 2) + HCI_PACKET_EVT_HDR;
      flags = 3;
      break;
    default:
      ALOGE("%s: Received packet with wrong packet type:%02x", __func__, type);
      return;
  }

  timestamp_us = time_gettimeofday_us();
  t = time(NULL);
  localtime_r (&t, &tm_cur);
  gmt_offset = tm_cur.tm_gmtoff;
  timestamp_us += gmt_offset*1000000LL;

  header.length_original = htonl(header_length);
  header.length_captured = header.length_original;
  header.flags = htonl(flags);
  header.dropped_packets = 0;
  header.timestamp = get_time_stamp(timestamp_us + BTSNOOP_EPOCH_DELTA);

  /* IBS command bytes are stored as BT command and events.
   * IBS bytes sent by HOST will be treated as command and
   * IBS bytes received by HOST will be treated as event.
   */
  if (type == HCI_PACKET_TYPE_IBS_CMD) {
    header.type = HCI_PACKET_TYPE_COMMAND;
    ibs_pkt[0] = 0x00;
    ibs_pkt[1] = 0xfc;
    ibs_pkt[2] = len;
    ibs_pkt[3] = *pkt;
    len = IBS_PKT_SIZE - 1;
  } else if (type == HCI_PACKET_TYPE_IBS_EVENT) {
    header.type = HCI_PACKET_TYPE_EVENT;
    ibs_pkt[0] = LOG_BT_EVT_VENDOR_SPECIFIC;
    ibs_pkt[1] = IBS_PKT_SIZE - 2;
    ibs_pkt[2] = 0x00;
    ibs_pkt[3] = 0xfc;
    ibs_pkt[4] = *pkt;
    len = IBS_PKT_SIZE;
  } else {
    header.type = type;
  }

  /* Store header before packet */
  memcpy(&buff_[active_buff_][buff_index_[active_buff_]], &header, sizeof(btsnoop_header_t));
  buff_index_[active_buff_] += snoop_hdr_len;
  /* Store actual packet */
  if (type == HCI_PACKET_TYPE_IBS_CMD || type == HCI_PACKET_TYPE_IBS_EVENT)
    memcpy(&buff_[active_buff_][buff_index_[active_buff_]], ibs_pkt, len);
  else
    memcpy(&buff_[active_buff_][buff_index_[active_buff_]], pkt, len);

  buff_index_[active_buff_] += len;

}

void PacketBuff::DumpStatInfo(int dst_fd) {
  write(dst_fd, &buff_[(active_buff_ + 1) % 2][0], buff_index_[(active_buff_ + 1) % 2]);
  write(dst_fd, &buff_[active_buff_][0], buff_index_[active_buff_]);
  if (fsync(dst_fd) == -1) {
    ALOGE("%s: Error while synchronization of logs with error code:%s", __func__,
          strerror(errno));
  }
}

void PacketBuff::DumpData()
{
#ifdef DUMP_RINGBUF_LOG
  struct timespec ts;
  struct timeval systs;
  char ts_buff[MAX_BUFF_SIZE];
  char sts_buff[MAX_BUFF_SIZE];
  char file_name[BT_FILE_NAME_LEN];
  int fd;
  int buffsz1 = 0;
  int buffsz2 = 0;

  if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
    ALOGE("%s: failed to get real time clock with error no: %d", __func__, errno);
  } else {
    gettimeofday(&systs, NULL);
    add_time_str(sts_buff, &systs);
    snprintf(ts_buff, MAX_BUFF_SIZE, "TS System %s TS kernel %lu:%lu", sts_buff,
             ts.tv_sec, ts.tv_nsec);
    BtState :: Get()->SetKernelTimeStamp(ts_buff);
    ALOGE("%s: %s", __func__, ts_buff);
  }

  // Check if something is there to dump. Avoid creating and dumping 0 byte files
  buffsz1 = buff_index_[(active_buff_ + 1) % 2];
  buffsz2 = buff_index_[active_buff_];
  if ((buffsz1 + buffsz2) <= IN_RINGBUFF_EVENTS_SIZE) {
    ALOGE("%s: No data to dump", __func__);
    return;
  }

  logger_ = static_cast<void*>(Logger::Get());
  static_cast<Logger*>(logger_)->GetSnoopFileName(file_name);

  fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
  if (fd < 0) {
    ALOGE("%s: File open (%s) failed: errno: %d", __func__, file_name, errno);
    return;
  }

  ALOGI("%s: writing dump into a file: %s", __func__, file_name);

  /* BT Snoop file format has File header followed by packets.
   * File header is a 16 bytes octet in which 8 bytes is an identification
   * pattern, 4 bytes is for version number and 4 bytes is for datalink
   * type.
   */
  write(fd, "btsnoop\0\0\0\0\1\0\0\x3\xea", 16);

  /* Store the data in buffer 1 */
  if (buffsz1 > 0)
    write(fd, &buff_[(active_buff_ + 1) % 2][0], buffsz1);

  /* Store the data in buffer 2 */
  if (buffsz2 > 0)
    write(fd, &buff_[active_buff_][0], buffsz2);

  if (fsync(fd) == -1) {
    ALOGE("%s: Error while synchronization of logs in :%s error code:%s", __func__,
          file_name, strerror(errno));
  }

  close(fd);

  memset(buff_index_, 0, sizeof(buff_index_));
  active_buff_ = 0;

#endif /* DUMP_RINGBUF_LOG */
}

char* PacketBuff::GetReason(PrimaryReasonCode reason)
{
  return (map_code_reason_.find(reason)->second);
}

static inline uint64_t time_gettimeofday_us(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return static_cast<uint64_t>(tv.tv_sec) * 1000000ULL + static_cast<uint64_t>(tv.tv_usec);
}

static inline uint64_t get_time_stamp(uint64_t ll)
{
  const uint32_t l = 1;
  if (*(reinterpret_cast<const uint8_t*>(&l)) == 1)
    return static_cast<uint64_t>(htonl(ll & 0xffffffff)) << 32 | htonl(ll >> 32);

  return ll;
}

void add_time_str(char *src, struct timeval* p_tval)
{
  unsigned short int t_hh, t_mm, t_ss;
  unsigned int temp = 0;
  struct timeval tval;

  if (!p_tval) {
    //Get current time stamp
    p_tval = &tval;
    gettimeofday(p_tval, NULL);
  }

  temp = p_tval->tv_sec;
  t_ss = temp%60;
  temp /= 60;
  t_mm = temp%60;
  temp /= 60;
  t_hh = temp%24;

  snprintf(src, TIME_STR_SIZE + 1, "%.2d:%.2d:%.2d:%.3d", t_hh, t_mm, t_ss,
     (int)(p_tval->tv_usec / 1000));
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android
