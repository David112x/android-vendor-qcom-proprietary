/*==========================================================================
Description
  It has implementation for logger class

# Copyright (c) 2017 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#include <hidl/HidlSupport.h>
#include <fcntl.h>
#include <errno.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <string.h>
#include <stdlib.h>
#include "logger.h"
#include "uart_controller.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "vendor.qti.bluetooth@1.0-logger"

#define SOC_DUMP_FIRST_PKT 0x0000
#define SOC_DUMP_LAST_PKT 0xFFFF

#define SOC_DUMP_PATH "/data/vendor/ssrdump/ramdump_bt_fw_crashdump_%.02d-%.02d-%.02d_%.02d-%.02d-%.02d.bin"
#define LOG_COLLECTION_DIR "/data/vendor/ssrdump/"
#define SOC_DUMP_PATH_BUF_SIZE 255
#define SOC_DUMP_SIGNATURE_SIZE 24

#define HAL_DUMP_FILE_SIZE 256
#define HAL_DUMP_TIMESTAMP_LENGTH 20
#define HAL_DUMP_TIMESTAMP_OFFSET 23
#define HAL_DUMP_SIZE 85
#ifdef USER_DEBUG
#define HAL_DUMP_ITERATION_LIMIT 80
#else
#define HAL_DUMP_ITERATION_LIMIT 10
#endif

#define BT_VND_SPECIFIC_EVENT    0xFF
#define BT_HW_ERR_EVT            0x10
#define BT_HW_ERR_FRAME_SIZE     0x01
#define BT_HW_ERR_EVT_BYTE       0x08
#define BT_HW_ERR_EVT_VALID      0xBD

#define BT_CONTROLLER_LOG        0x01
#define LOG_BT_MESSAGE_TYPE_VSTR     0x02
#define LOG_BT_MESSAGE_TYPE_PACKET   0x05
#define LOG_BT_MESSAGE_TYPE_MEM_DUMP 0x08
#define LOG_BT_MESSAGE_TYPE_HW_ERR   0x09
#define DIAG_SSR_BT_CMD     0x0007

#define LAST_SEQUENCE_NUM 0xFFFF
#define CRASH_REASON_NOT_FOUND  ((char *)"Crash reason not found")

uint8_t clientstatus;
struct crash_reason {
  uint32_t reason;
  time_t time_instance;
} sec_crash_reason;

uint32_t pri_crash_reason;

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

Logger * Logger :: instance_ = NULL;

Logger * Logger :: Get()
{
  if (!instance_) {
    instance_ = new Logger();
  }
  return instance_;
}

Logger :: Logger()
{
  /*To me initiated dynamically by config input*/
  ibs_log_level_ = MID;
  rx_log_level_ = MID;
  tx_log_level_ = MID;
  gen_log_level_ = MID;
  is_crash_dump_in_progress_ = false;
  crash_dump_status_cb_ = nullptr;
  dump_reason_ = REASON_SOC_CRASHED;
  state_info_  = BtState::Get();
}

bool Logger :: IsSnoopLogEnabled(void)
{
  char value[PROPERTY_VALUE_MAX] = {'\0'};
  property_get("persist.vendor.service.bdroid.snooplog", value, "false");
  return (strcmp(value, "true") == 0);
}

bool Logger :: IsHciFwSnoopEnabled(void)
{
#ifdef USER_DEBUG
  return true;
#else
  char value[PROPERTY_VALUE_MAX] = {'\0'};
  property_get("persist.vendor.service.bdroid.fwsnoop", value, "false");
  return (strcmp(value, "true") == 0);
#endif
}

bool Logger::IsControllerLogPkt(HciPacketType type, const uint8_t *buff, uint16_t len)
{
  if (len < 3) {
    return false;
  } else if ((HCI_PACKET_TYPE_EVENT == type) &&
             (BT_VND_SPECIFIC_EVENT == buff[0]) &&
             (BT_CONTROLLER_LOG == buff[2])) {
    return true;
  } else {
    return false;
  }
}

bool Logger :: IsCrashDumpStarted(void)
{
  return is_crash_dump_in_progress_;
}

bool Logger:: RegisterCrashDumpCallback(CrashDumpStatusCallback crash_dump_cb)
{
  crash_dump_status_cb_ = crash_dump_cb;
  return true;
}

void Logger:: RegisterHWEventCallback(HWEventCallback hw_event_cb)
{
  hw_event_cb_= hw_event_cb;
}

bool Logger :: ProcessRx(HciPacketType type, const uint8_t *buff, uint16_t len)
{
  bool ret  = true;
#ifdef DUMP_RINGBUF_LOG
      state_info_->SetLastRxPacket((uint8_t*)buff, len, type);
#endif
  switch (type) {
    case HCI_PACKET_TYPE_ACL_DATA:
#ifdef DUMP_RINGBUF_LOG
      snoop_buff_.AddLog(RingBuffer::SOC_TO_HOST, type, (uint8_t*)buff, len);
#endif

#ifdef DIAG_ENABLED
      /* Post the event once the Diag init is success */
      if (diag_interface_.GetDiagInitStatus() && IsSnoopLogEnabled()) {
        diag_interface_.SendLogs(buff, len, LOG_BT_HCI_RX_ACL_C);
      }
#endif
      break;

    case HCI_PACKET_TYPE_EVENT:
      ret =  HandleHciEvent(type, (uint8_t*)buff, len);

#ifdef DIAG_ENABLED
      /* Post the event once the Diag init is success */
      if (diag_interface_.GetDiagInitStatus() && IsSnoopLogEnabled()) {
        diag_interface_.SendLogs(buff, len, LOG_BT_HCI_EV_C);
      }
      break;
#endif

    case HCI_PACKET_TYPE_FM_EVENT:
    case HCI_PACKET_TYPE_ANT_CTRL:
    case HCI_PACKET_TYPE_ANT_DATA:
#ifdef DUMP_RINGBUF_LOG
      snoop_buff_.AddLog(RingBuffer::SOC_TO_HOST, type, (uint8_t*)buff, len);
#endif
     break;
    default:
      ret =  false;
      break;
  }
  return ret;
}

bool Logger :: ProcessTx(HciPacketType type, const uint8_t *buff, uint16_t len)
{
#ifdef DUMP_RINGBUF_LOG
  snoop_buff_.AddLog(RingBuffer::HOST_TO_SOC, type, (uint8_t*)buff, len);
  state_info_->SetLastTxPacket((uint8_t*)buff, len, type);
#endif

#ifdef DIAG_ENABLED
  /* Post the event once the Diag init is success */
  if (diag_interface_.GetDiagInitStatus() && IsSnoopLogEnabled()) {
    if (type == HCI_PACKET_TYPE_COMMAND)
      diag_interface_.SendLogs(buff, len, LOG_BT_HCI_CMD_C);
    else if (type == HCI_PACKET_TYPE_ACL_DATA)
      diag_interface_.SendLogs(buff, len, LOG_BT_HCI_TX_ACL_C);
  }
#endif

  return true;
}

void Logger :: DecodeHwErrorEvt(uint8_t *buff)
{
  /* We receive dump via an VSE command, the dump reason hold two
   * bytes, one is fixed byte and other holds the reason for crash.
   */
  SecondaryReasonCode reason = (SecondaryReasonCode)buff[BT_HW_ERR_EVT_BYTE];
  ALOGE("Rx VSE HW error event::%s\n", get_reset_reason_str(reason));
  if (hw_event_cb_)
    hw_event_cb_();
  if (isDiagSsrTriggered()) {
    dump_reason_ = REASON_SOC_CRASHED_DIAG_SSR;
    SetPrimaryCrashReason(dump_reason_);
  }
  SetSecondaryCrashReason(reason);
}

bool Logger :: HandleHciEvent(HciPacketType type, uint8_t *buff, uint16_t len)
{
  bool ret = true;

  if ((buff[0] == BT_VND_SPECIFIC_EVENT) &&
      (buff[2] == BT_CONTROLLER_LOG)) {
    switch (buff[3]) {
      case LOG_BT_MESSAGE_TYPE_VSTR:
      case LOG_BT_MESSAGE_TYPE_PACKET:
#ifdef DUMP_RINGBUF_LOG
        snoop_buff_.AddLog(RingBuffer::SOC_TO_HOST, type, buff, len);
#endif
        break;

      case LOG_BT_MESSAGE_TYPE_MEM_DUMP:
        if (isDiagSsrTriggered())
          dump_reason_ = REASON_SOC_CRASHED_DIAG_SSR;
        SaveSocMemDump(buff, len, dump_reason_);
        break;

      case LOG_BT_MESSAGE_TYPE_HW_ERR:
        if (buff[1] >= BT_HW_ERR_EVT_BYTE &&
            buff[9] == BT_HW_ERR_EVT_VALID)
          DecodeHwErrorEvt(buff);
        else
          ALOGE("Rx HW error event with wrong payload");
        break;

      default:
        ret = false;
        break;
    }
  } else if (buff[0] == BT_HW_ERR_EVT) {
    /* Check the size of the event */
    if (buff[1] == BT_HW_ERR_FRAME_SIZE) {
      ALOGE("Rx HW error event::%s\n", get_reset_reason_str((SecondaryReasonCode)buff[2]));
      SetSecondaryCrashReason((SecondaryReasonCode)buff[2]);
      if (isDiagSsrTriggered()) {
       dump_reason_ = REASON_SOC_CRASHED_DIAG_SSR;
       SetPrimaryCrashReason(dump_reason_);
      }
    } else {
      ALOGE("Rx HW error event with wrong payload");
    }
#ifdef DUMP_RINGBUF_LOG
    snoop_buff_.AddLog(RingBuffer::SOC_TO_HOST, type, buff, len);
#endif
    if (hw_event_cb_)
      hw_event_cb_();
  } else {
#ifdef DUMP_RINGBUF_LOG
    snoop_buff_.AddLog(RingBuffer::SOC_TO_HOST, type, buff, len);
#endif
  }
  return ret;
}

#ifdef DIAG_ENABLED
const diagpkt_user_table_entry_type Logger :: ssr_bt_tbl_[] = {
  { DIAG_SSR_BT_CMD, DIAG_SSR_BT_CMD, SsrBtHandler },
};
#endif

void Logger :: UnlockRingbufferMutex()
{
#ifdef DUMP_RINGBUF_LOG
  snoop_buff_.UnlockMutex();
#endif
}

#ifdef DIAG_ENABLED
void * Logger :: SsrBtHandler(void *req_pkt, uint16_t pkt_len)
{
  Logger *logger = Logger :: Get();

  if (logger) {
    return logger->diag_interface_.SsrBtHandler(req_pkt, pkt_len);
  } else {
    return nullptr;
  }
}
#endif

char * Logger::get_reset_reason_str(SecondaryReasonCode reason)
{
  for(int i =0; i < (int)(sizeof(secReasonMap)/sizeof(SecondaryReasonMap)); i++)
    if (secReasonMap[i].reason == reason)
      return secReasonMap[i].reasonstr;

  return CRASH_REASON_NOT_FOUND;
}

void Logger::StoreCrashReason()
{
  char value[PROPERTY_VALUE_MAX] = {'\0'};
  char temp[PROPERTY_VALUE_MAX] = {'\0'};
  char buff[MAX_CRASH_BUFF_SIZE] = {'\0'};

  if (sec_crash_reason.reason == SEC_REASON_DEFAULT &&
      pri_crash_reason == REASON_RX_THREAD_STUCK)
      snprintf(temp, PROPERTY_VALUE_MAX, "%s", asctime(localtime(&time_rx_call_back)));
  else
      snprintf(temp, PROPERTY_VALUE_MAX, "%s",
               asctime(localtime(&sec_crash_reason.time_instance)));

  if (pri_crash_reason == REASON_DEFAULT_NONE)
    pri_crash_reason = REASON_SOC_CRASHED;

  snprintf(buff, MAX_CRASH_BUFF_SIZE, "BtPrimaryCrashReason:%s",
           snoop_buff_.GetPrimaryReason((PrimaryReasonCode)pri_crash_reason));
  ALOGI("%s", buff);
  state_info_->SetPrimaryCrashReason(buff);

  snprintf(buff, MAX_CRASH_BUFF_SIZE, "BtSecondaryCrashReason:%s",
           get_reset_reason_str((SecondaryReasonCode)sec_crash_reason.reason));
  ALOGI("%s", buff);
  state_info_->SetSecondaryCrashReason(buff);

  snprintf(buff, MAX_CRASH_BUFF_SIZE, "TS for SoC Crash:%s", temp);
  ALOGI("%s", buff);
  state_info_->SetCrashTS(buff);


  snprintf(value, PROPERTY_VALUE_MAX, "%u-%u-%s", pri_crash_reason,
           sec_crash_reason.reason, temp);
  property_set("persist.vendor.bluetooth.crash.reasons", value);
}

bool Logger::IsPreviousItrCrashed()
{
  char value[PROPERTY_VALUE_MAX] = {'\0'};

  /* Property value will not be false, if previous iteration has crash
   * and crash event it not sent to client's due Rx thread issue
   * or client is dead before the event received.
   */
  property_get("persist.vendor.bluetooth.crash.reasons", value, "false");
  if (!strcmp(value, "false"))
    return false;

  return true;
}

bool Logger::FrameCrashEvent(hidl_vec<uint8_t>*bt_packet_)
{
  char value[PROPERTY_VALUE_MAX] = {'\0'};

  property_get("persist.vendor.bluetooth.crash.reasons", value, "false");
  if (strcmp(value, "false") != 0) {
    unsigned char bt_eve_buf[PROPERTY_VALUE_MAX] = { 0xff, 0x0e, 0xfc, 0x00};
    int count = 4;
    uint32_t primary_reason;
    uint32_t secondary_reason;
    uint8_t temp[255], i;
    int len = strlen(value);
    char *reason = value;

    /* Crash reason frame format
     * Primary crash reason | Secondary crash reason | time stamp
     */
    /* Decode primary crash reason */
    for(i = 0; i < len && reason[i] != '-'; i++)
      temp [i] = reason[i];

    if (i == len) {
      ALOGE("Unable to %s due to presist flag format is invalid", __func__);
      return false;
    }

    temp[i] = '\0';

    reason += (i+1);
    primary_reason = atoi((char *)temp);
    len = len - i -1;

    /* Decode secondary crash reason */
    for(i = 0; i < len && reason[i] != '-'; i++)
      temp [i] = reason[i];

    if (i == len) {
      ALOGE("Unable to %s due to presist flag format is invalid", __func__);
      return false;
    }

    temp[i] = '\0';

    reason += (i+1);
    secondary_reason = atoi((char *)temp);
    len = len - i -1;

    /* LSB byte holds the crash reason, for safer side keeping in
     * view of future implementations using lower two LSB bytes.
     */
    bt_eve_buf[count] = ((uint8_t) ((primary_reason) & 0xFF));
    bt_eve_buf[count + 1] = ((uint8_t) ((primary_reason >> 0x08) & 0xFF));
    bt_eve_buf[count + 2] = ((uint8_t) ((secondary_reason) & 0xFF));
    bt_eve_buf[count + 3] = ((uint8_t) ((secondary_reason >> 0x08) & 0xFF));

    count += 4;

    /* copy remaining as a string */
    memcpy(bt_eve_buf + count, reason, strlen(reason));
    count = count + strlen(reason);
    /*Discard the separator after time stamp */
    count = count - 1;
    bt_eve_buf[1] = count - 2;
    bt_packet_->resize(count);
    memcpy(bt_packet_->data(), bt_eve_buf, count);
    ALOGI("%s: for crash reason :%s", __func__, value);

    return true;
  }
  ALOGI("%s: Nothing to send crash reason is :%s", __func__, value);
  return false;
}

void Logger :: SaveSocMemDump(uint8_t *eventBuf, uint16_t packet_len, PrimaryReasonCode reason)
{
  static unsigned int dump_size = 0, total_size = 0;
  unsigned short seq_num = 0;
  static unsigned short seq_num_cnt = 0;
  uint8_t *dump_ptr = NULL;
  static char *temp_buf, *p, path[SOC_DUMP_PATH_BUF_SIZE + 1];
  static int dump_fd = -1;
  char nullBuff[255] = { 0 };
  char dst_buff[MAX_BUFF_SIZE] = {'\0'};
  dump_ptr = &eventBuf[7];
  seq_num = eventBuf[4] | (eventBuf[5] << 8);
  packet_len -= 7;

  ALOGV("%s: LOG_BT_MESSAGE_TYPE_MEM_DUMP (%d) ", __func__, seq_num);

  if ((seq_num != seq_num_cnt) && seq_num != LAST_SEQUENCE_NUM) {
    ALOGE("%s: current sequence number: %d, expected seq num: %d ", __func__,
          seq_num, seq_num_cnt);
  }

  if (seq_num == 0x0000) {
    PrepareDumpProcess();
#ifdef DUMP_RINGBUF_LOG
    snoop_buff_.DumpData();
#endif

    SetPrimaryCrashReason(reason);

#ifdef DUMP_IPC_LOG
    ipc_log_.DumpUartLogs();
#endif

    dump_size = (unsigned int)
                (eventBuf[7] | (eventBuf[8] << 8) | (eventBuf[9] << 16) | (eventBuf[10] << 24));
    dump_ptr = &eventBuf[11];
    total_size = seq_num_cnt = 0;
    packet_len -= 4;

    //memset(path, 0, SOC_DUMP_PATH_BUF_SIZE);
    /* first pack has total ram dump size (4 bytes) */
    ALOGD("%s: Crash Dump Start - total Size: %d ", __func__, dump_size);
    is_crash_dump_in_progress_ = true;
    p = temp_buf = (char*)malloc(dump_size);
    if (p != NULL) {
      memset(p, 0, dump_size);
    } else {
      ALOGE("Failed to allocate mem for the crash dump size: %d", dump_size);
    }

    GetCrashDumpFileName(path);
    dump_fd = open(path, O_CREAT | O_SYNC | O_WRONLY,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (dump_fd < 0) {
      ALOGE("%s: File open (%s) failed: errno: %d", __func__, path, errno);
      seq_num_cnt++;
      return;
    }
    ALOGV("%s: File Open (%s) successfully ", __func__, path);
  }

  if (dump_fd >= 0) {
    for (; (seq_num > seq_num_cnt) && (seq_num != LAST_SEQUENCE_NUM); seq_num_cnt++) {
      ALOGE("%s: controller missed packet : %d, write null (%d) into file ",
            __func__, seq_num_cnt, packet_len);

      if (p != NULL) {
        memcpy(temp_buf, nullBuff, packet_len);
        temp_buf = temp_buf + packet_len;
      }
    }

    if (p != NULL) {
      memcpy(temp_buf, dump_ptr, packet_len);
      temp_buf = temp_buf + packet_len;
    }
    total_size += packet_len;
  }

  seq_num_cnt++;
  if (seq_num == LAST_SEQUENCE_NUM && p != NULL) {
    char dst_buff[MAX_BUFF_SIZE] = {'\0'};
    char seq_buff[MAX_BUFF_SIZE] = {'\0'};
    struct timeval tv;
    ALOGE("Writing crash dump of size %d bytes", total_size);
    gettimeofday(&tv, NULL);
    snprintf(dst_buff, sizeof(dst_buff), "Last sequence num %d rcvd from SOC", seq_num_cnt);
    BtState ::Get()->AddLogTag(seq_buff, tv, dst_buff);
    BtState ::Get()->SetLastSeqTS(seq_buff);
    write(dump_fd, p, total_size);
    free(p);
    temp_buf = NULL;
    p = NULL;
    seq_num_cnt = 0;
  }

  if (seq_num == LAST_SEQUENCE_NUM) {
    if (dump_fd >= 0) {
      if ( fsync(dump_fd) < 0 ) {
        ALOGE("%s: File flush (%s) failed: %s, errno: %d", __func__,
              path, strerror(errno), errno);
      }
      close(dump_fd);
      dump_fd = -1;

      ALOGI("%s: Write crashdump successfully : \n"
            "\t\tFile: %s\n\t\tdump_size: %d\n\t\twritten_size: %d",
            __func__, path, dump_size, total_size );
      int cntr = 0;
      size_t readsize;
      unsigned char *dumpinfo, *tempptr;
      uint32_t ucFilename;
      uint32_t uLinenum;
      uint32_t uPCAddr;
      uint32_t uResetReason;
      uint32_t uBuildVersion;
      dump_fd = -1;
      int i = 0;
      char filenameBuf [SOC_DUMP_PATH_BUF_SIZE];
      char *decode_reason = NULL;

      memset(filenameBuf, 0, SOC_DUMP_PATH_BUF_SIZE);
      if (-1 != (dump_fd = open(path, O_RDONLY))) {
        if (NULL != (dumpinfo = (unsigned char*)malloc(SOC_DUMP_SIGNATURE_SIZE))) {
          tempptr = dumpinfo;
          lseek(dump_fd, 0xFEE0, SEEK_SET);
          readsize = SOC_DUMP_SIGNATURE_SIZE;
          while (readsize) {
            cntr = read(dump_fd, (void*)tempptr, readsize);
            tempptr += cntr;
            readsize -= cntr;
          }

          tempptr = dumpinfo;
          memcpy(&ucFilename, tempptr, 4); tempptr += 4;
          memcpy(&uLinenum, tempptr, 4); tempptr += 4;
          memcpy(&uPCAddr, tempptr, 4); tempptr += 4;
          memcpy(&uResetReason, tempptr, 4); tempptr += 4;
          memcpy(&uBuildVersion, tempptr, 4); tempptr += 4;

          if (0 != ucFilename) {
            lseek(dump_fd, (off_t)ucFilename, SEEK_SET);
            cntr = read(dump_fd, (void*)filenameBuf, SOC_DUMP_PATH_BUF_SIZE);
            while (i < SOC_DUMP_PATH_BUF_SIZE && filenameBuf[i++] != '\0' ) ;
            if (i < SOC_DUMP_PATH_BUF_SIZE) {
              ALOGE("Filename::%s\n", filenameBuf);
            }
          }

          decode_reason = get_reset_reason_str((SecondaryReasonCode)uResetReason);
          ALOGE("Linenum::%d\n", uLinenum);
          ALOGE("PC Addr::0x%x\n", uPCAddr);
          ALOGE("Reset reason::%s\n", decode_reason);
          ALOGE("Reset reason::%d\n", uResetReason);
          ALOGE("Build Version::0x%x\n", uBuildVersion);

         if (decode_reason)
           SetSecondaryCrashReason((SecondaryReasonCode)uResetReason);
         else
           SetSecondaryCrashReason(SEC_REASON_SOCCRASH_RX_NULL);

         snprintf(dst_buff, MAX_BUFF_SIZE, (char *) "PC Addr::0x%x", uPCAddr);
         state_info_->SetPCAddr(dst_buff);
         memset(dst_buff, 0, MAX_BUFF_SIZE);
         snprintf(dst_buff, MAX_BUFF_SIZE, (char *) "Build Version::0x%x", uBuildVersion);
         state_info_->SetBuildVersion(dst_buff);
        }
        if (NULL != dumpinfo) free(dumpinfo);
        close(dump_fd);
      }
    }

    is_crash_dump_in_progress_ = false;
  }

  if ( (0 == seq_num || LAST_SEQUENCE_NUM == seq_num) && crash_dump_status_cb_) {
    crash_dump_status_cb_(is_crash_dump_in_progress_);
  }
}

void Logger :: Init(HciTransport *hci_transport)
{
  bool status = false;
  char dst_buff[MAX_BUFF_SIZE];
  struct timeval tv;

  SetSecondaryCrashReason(SEC_REASON_BTON_DIAGINIT_STUCK);
  status = diag_interface_.Init(hci_transport);

#ifdef DIAG_ENABLED
  gettimeofday(&tv, NULL);
  memset(dst_buff, 0, MAX_BUFF_SIZE);
  if (status) {
    state_info_->AddLogTag(dst_buff, tv, (char *)"Diag Init successful");

    ALOGD("%s: Registered Diag callbacks", __func__);
    DIAGPKT_DISPATCH_TABLE_REGISTER(DIAG_SUBSYS_BT, ssr_bt_tbl_);
  } else {
    state_info_->AddLogTag(dst_buff, tv, (char *)"Diag Init failed");
  }
  state_info_->SetDiagInitStatus(dst_buff);
#endif

}


void Logger :: Cleanup(void)
{
  diag_interface_.CleanUp();

  if (!DiagInterface::GetDiagDeInitTimeoutTriggered()) {
    if (instance_) {
      delete instance_;
      instance_ = NULL;
    }
  }
}

void Logger :: GetCrashDumpFileName(char* path)
{
  snprintf(path, SOC_DUMP_PATH_BUF_SIZE, SOC_DUMP_PATH, time_year, time_month, time_day, time_hour, time_min, time_sec);
}

#ifdef DUMP_IPC_LOG
void Logger :: GetUartDumpFilename(short int uart_dump, char* path)
{
  switch (uart_dump){
  case IpcLogs::TX_LOG:
    snprintf(path, UART_IPC_PATH_BUF_SIZE, UART_IPC_TX_LOGS_DEST_PATH, time_year, time_month, time_day, time_hour, time_min, time_sec);
    break;
  case IpcLogs::RX_LOG:
    snprintf(path, UART_IPC_PATH_BUF_SIZE, UART_IPC_RX_LOGS_DEST_PATH, time_year, time_month, time_day, time_hour, time_min, time_sec);
    break;
  case IpcLogs::STATE_LOG:
    snprintf(path, UART_IPC_PATH_BUF_SIZE, UART_IPC_STATE_LOGS_DEST_PATH, time_year, time_month, time_day, time_hour, time_min, time_sec);
    break;
  case IpcLogs::POWER_LOG:
    snprintf(path, UART_IPC_PATH_BUF_SIZE, UART_IPC_PWR_LOGS_DEST_PATH, time_year, time_month, time_day, time_hour, time_min, time_sec);
    break;
  }
}
#endif

#ifdef DUMP_RINGBUF_LOG
void Logger :: GetSnoopFileName(char* path)
{
  snprintf(path, BT_FILE_NAME_LEN, SNOOP_FILE_NAME, time_year, time_month,
           time_day, time_hour, time_min, time_sec);
}
#endif

void Logger :: GetStateFileName(char* path)
{
  snprintf(path, BT_FILE_NAME_LEN, BT_STATE_FILE_NAME, time_year, time_month,
           time_day, time_hour, time_min, time_sec);
}

bool Logger::PrepareDumpProcess()
{
  SetDumpTimestamp();
#ifndef DONT_DELETE_DUMPS_SET
  DeleteDumpsIfRequired();
#endif
  return true;
}

void Logger::SetDumpTimestamp()
{
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  time_year = tm.tm_year + 1900;
  time_month = tm.tm_mon+ 1;
  time_day = tm.tm_mday;
  time_hour = tm.tm_hour;
  time_min = tm.tm_min;
  time_sec = tm.tm_sec;
}

void Logger :: DeleteDumpsIfRequired()
{
  struct DIR* p_dir;
  struct dirent* p_dirent;
  char *first_entry = NULL;
  int count = 0;

#ifdef DUMP_IPC_LOG
  char *look_for = (char *)"uart_ipc_rx";
#else
  char *look_for = (char *)"fw_crashdump";
#endif

  p_dir = opendir(LOG_COLLECTION_DIR);
  if (p_dir == NULL) {
    ALOGE("%s: Cannot open dump location %s", __func__, LOG_COLLECTION_DIR);
    return;
  }
  while ((p_dirent = readdir(p_dir)) != NULL) {
    if (strstr(p_dirent->d_name, look_for)) {
      count++;
      if(count == 1) {
        first_entry = p_dirent->d_name;
      }
    }
  }
  if (count >= HAL_DUMP_ITERATION_LIMIT)
    DeleteDumps(first_entry);

  closedir(p_dir);
}

void Logger :: DeleteDumps(char *first_entry)
{
  struct DIR* p_dir;
  struct dirent* p_dirent;
  int ret = 0;

#ifdef DUMP_IPC_LOG
  char *look_for = (char *)"uart_ipc_rx";
#else
  char *look_for = (char *)"fw_crashdump";
#endif
  char timestamp[HAL_DUMP_TIMESTAMP_LENGTH];
  char path[HAL_DUMP_SIZE];
  char oldest_file[HAL_DUMP_FILE_SIZE];

  strlcpy(oldest_file, first_entry, HAL_DUMP_FILE_SIZE);

  p_dir = opendir(LOG_COLLECTION_DIR);
  if (p_dir == NULL) {
    ALOGE("%s: Cannot open dump location %s", __func__, LOG_COLLECTION_DIR);
    return;
  }

  while ((p_dirent = readdir(p_dir)) != NULL) {
    if (strstr(p_dirent->d_name, look_for) ) {
      if(strcmp(p_dirent->d_name, oldest_file) < 0) {
        strlcpy(oldest_file, p_dirent->d_name, HAL_DUMP_FILE_SIZE);
      }
    }
  }
  closedir(p_dir);

#ifdef DUMP_IPC_LOG
  strlcpy(timestamp, oldest_file + HAL_DUMP_TIMESTAMP_OFFSET, HAL_DUMP_TIMESTAMP_LENGTH);
#else
  strlcpy(timestamp, oldest_file + HAL_DUMP_TIMESTAMP_OFFSET + 1, HAL_DUMP_TIMESTAMP_LENGTH);
#endif

  p_dir = opendir(LOG_COLLECTION_DIR);
  if (p_dir == NULL) {
    ALOGE("%s: Cannot open dump location %s", __func__, LOG_COLLECTION_DIR);
    return;
  }

  while ((p_dirent = readdir(p_dir)) != NULL) {
   if (strstr(p_dirent->d_name, timestamp)) {
     strlcpy(path, LOG_COLLECTION_DIR, HAL_DUMP_SIZE);
     strlcat(path, p_dirent->d_name, HAL_DUMP_SIZE);
     ALOGE("%s: Deleting oldest dump file: %s", __func__, path);
     ret = remove(path);
     if(ret < 0) ALOGE("%s: Cannot delete file %s", __func__, path);
   }
  }
  closedir(p_dir);
}

void Logger:: ResetCrashReasons()
{
  pri_crash_reason  = REASON_DEFAULT_NONE;
  sec_crash_reason.reason = SEC_REASON_DEFAULT;
  sec_crash_reason.time_instance = 0;
}

void Logger:: SetPrimaryCrashReason(PrimaryReasonCode reason)
{
  pri_crash_reason  = reason;
   dump_reason_ = reason;
   /* Some host trigger crashes don't get crash dump, mapping the
    * time stamp at the time of primary reason. If we have dump
    * collected will take collection time stamp into account.
    */
  sec_crash_reason.time_instance = time(NULL);
}

void Logger:: SetSecondaryCrashReason(SecondaryReasonCode reason)
{
  sec_crash_reason.reason = reason;
  sec_crash_reason.time_instance = time(NULL);
}

char * Logger:: GetSecondaryCrashReason()
{
  return get_reset_reason_str((SecondaryReasonCode)sec_crash_reason.reason);
}

bool Logger :: isSsrTriggered()
{
  return diag_interface_.isSsrTriggered();
}

bool Logger :: isDiagSsrTriggered()
{
  return diag_interface_.isDiagSsrTriggered();
}

void Logger :: ResetSsrTriggeredFlag()
{
  return diag_interface_.ResetSsrTriggeredFlag();
}

bool Logger :: SetSsrTriggeredFlag()
{
  return diag_interface_.SetSsrTriggeredFlag();
}

uint8_t Logger :: GetClientStatus(ProtocolType type)
{
  return (clientstatus & (0x01 << type));
}

void Logger :: SetClientStatus(bool status, ProtocolType type)
{
  if (status)
    clientstatus = (clientstatus | (0x01 << type));
  else
    clientstatus = (clientstatus & (~(0x01 << type)));
}

bool Logger :: GetCleanupStatus(ProtocolType ptype)
{
  return diag_interface_.GetCleanupStatus(ptype);
}

void Logger:: UpdateRxTimeStamp()
{
  time_rx_call_back = time(NULL);
}

PrimaryReasonCode Logger::GetPrimaryReason()
{
  return (PrimaryReasonCode)pri_crash_reason;
}

void Logger :: UnlockDiagMutex()
{
  diag_interface_.UnlockDiagMutex();
}

void Logger :: ResetForceSsrTriggeredFlag()
{
  return diag_interface_.ResetForceSsrTriggeredFlag();
}

} // namespace implementation
} // namespace V1_0
} // namespace bluetooth
} // namespace hardware
} // namespace android
