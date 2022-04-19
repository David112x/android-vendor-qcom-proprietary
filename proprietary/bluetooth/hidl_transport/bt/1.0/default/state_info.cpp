/*==========================================================================
Description
  It has implementation for BtState class

# Copyright (c) 2019 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#include <hidl/HidlSupport.h>
#include <fcntl.h>
#include <errno.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <unistd.h>
#include "state_info.h"
#include "ring_buffer.h"
#include "logger.h"
#include "ibs_handler.h"
#include "wake_lock.h"
#include "health_info_log.h"
#define LOG_TAG "vendor.qti.bluetooth@1.0-btstateinfo"
#define TIME_STAMP_LEN 13
#define TAG_STR_LEN    64

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

extern bool soc_need_reload_patch;
BtState * BtState::instance_ = NULL;

BtState * BtState::Get()
{
  if (!instance_) {
    instance_ = new BtState();
  }
  return instance_;
}

BtState::BtState()
{
  memset(&stat_info_obj, 0, sizeof(stat_info_obj));
  strlcpy(stat_info_obj.pri_crash_reason, "No Primary Crash reason set",
          MAX_CRASH_BUFF_SIZE);
  strlcpy(stat_info_obj.sec_crash_reason, "No Secondary Crash reason set",
          MAX_CRASH_BUFF_SIZE);
  strlcpy(stat_info_obj.last_health_stat_info, "Last health stats info: ",
          MAX_STATE_INFO_SIZE);
  strlcpy(stat_info_obj.last_wk_lck_acq_info, "00:00:00:000-Wakelock :: Acquire",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_wk_lck_rel_info, "00:00:00:000-Wakelock :: Release",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_hci_cmd_stack, "00:00:00:000-Last HCI cmd from stack",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_wk_ind_from_host, "00:00:00:000-Writing FD to SOC",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_wk_ack_from_soc, "00:00:00:000-Received FC from SOC",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_wk_ind_from_soc, "00:00:00:000-Received FD from SOC",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_wk_ack_from_host, "00:00:00:000-Writing FC to SOC",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_sleep_ind_from_host, "00:00:00:000-Writing FE to SoC",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_sleep_ind_from_soc, "00:00:00:000-Received FE from SOC",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.pc_addr, "PC address of FW crash: 0x00", MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.build_ver, "SOC build ver: 0x00", MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_getver_start_send_cmd, "00:00:00:000-Get Version CMD not sent to SOC"
          , MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_getver_start_read_rsp,
          "00:00:00:000-Get Version CMD rsp not yet read", MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_getver_rcvd, "00:00:00:000-Get Version CMD RSP not rcvd from SOC",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_hci_initialize, "00:00:00:000-HCI initialize rcvd from client",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_hci_close, "00:00:00:000-HCI close not rcvd from client",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_last_seq_number, "00:00:00:000-Last sequence num",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.pre_stack_event, "00:00:00:000-Pre stack event callback",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.post_stack_event, "00:00:00:000-Post stack event callback",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.pre_stack_acl, "00:00:00:000--Pre stack ACL callback",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.post_stack_acl, "00:00:00:000--Pre stack ACL callback",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.pre_select, "00:00:00:000--Pre select callback",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.post_select, "00:00:00:000--Pre select callback",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_uart_flow_on, "00:00:00:000-UART Flow On", MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_uart_flow_off, "00:00:00:000-UART Flow Off", MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_uart_clock_on, "00:00:00:000-UART Clock on", MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_uart_clock_off, "00:00:00:000-UART Clock off",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_status_init_cb_sent, "00:00:00:000-Init callback not sent",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_status_diag_init, "00:00:00:000:Diag not initialized",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.health_timer_status, "Health timer not started",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.status_read_thread, "Read thread not started", MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_status_ctrlr_init, "Controller Init not completed",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_tx_pkt_ts, "00:00:00:000-last tx packet", MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_rx_pkt_ts, "00:00:00:000-last rx packet", MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ssr_rx_pkt_ts, "00:00:00:000-last rx before ssr", MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.kernel_time, "TS System 00:00:00:000- TS kernel 000:00000", MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.pre_diag_deinit_ts, "00:00:00:000:Pre Diag deinit not started",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.post_diag_deinit_ts, "00:00:00:000:Post Diag deinit not started",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.fw_dwnld_success, "00:00:00:000:Soc Initialization not completed",
          MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.fw_su_build_info, "BT SoC FW SU Build info: Not Known", MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.alwayson_status, "SOC AlwaysOn disabled", MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_hci_reset_cmd, "First HCI_RESET_CMD not sent to SOC", MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_hci_reset_rsp, "Not rcvd RSP for first HCI_RESET_CMD", MAX_BUFF_SIZE);
  stat_info_obj.ssr_trigger = false;
}


void BtState::Cleanup(void)
{
  if (instance_) {
    delete instance_;
    instance_ = NULL;
  }
}

void BtState::AddLogTag(char* dest_tag_str, struct timeval& time_val, char * tag)
{
  uint32_t w_index = 0;
  memset(dest_tag_str, 0, TAG_STR_LEN);
  add_time_str(dest_tag_str, &time_val);
  w_index = strlen(dest_tag_str);
  snprintf(dest_tag_str+w_index, TAG_STR_LEN - w_index, "-%s", tag);
}

void BtState::SetTsHCIInitClose(HCIStatus hci_status, char *buf) {
  if(hci_status == HCI_INIT) {
    memset(stat_info_obj.ts_hci_initialize, 0, MAX_BUFF_SIZE);
    strlcpy(stat_info_obj.ts_hci_initialize, buf, sizeof(stat_info_obj.ts_hci_initialize));
    ALOGV("%s:is %s", __func__, stat_info_obj.ts_hci_initialize);
  } else if(hci_status == HCI_CLOSE) {
    memset(stat_info_obj.ts_hci_close, 0, MAX_BUFF_SIZE);
    strlcpy(stat_info_obj.ts_hci_close, buf, sizeof(stat_info_obj.ts_hci_close));
    ALOGV("%s:is %s", __func__, stat_info_obj.ts_hci_close);
 }
}

void BtState::SetDiagInitStatus(char *buf) {
  memset(stat_info_obj.ts_status_diag_init, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_status_diag_init, buf, sizeof(stat_info_obj.ts_status_diag_init));
  ALOGV("%s:is %s", __func__, stat_info_obj.ts_status_diag_init);
}

void BtState::SetHciResetCmdSent(char *buf) {
  memset(stat_info_obj.ts_hci_reset_cmd, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_hci_reset_cmd, buf, sizeof(stat_info_obj.ts_hci_reset_cmd));
  ALOGV("%s:is %s", __func__, stat_info_obj.ts_hci_reset_cmd);
}

void BtState::SetHciResetCmdRsp(char *buf) {
  memset(stat_info_obj.ts_hci_reset_rsp, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_hci_reset_rsp, buf, sizeof(stat_info_obj.ts_hci_reset_rsp));
  ALOGV("%s:is %s", __func__, stat_info_obj.ts_hci_reset_rsp);
}

void BtState::SetTsCtrlrInitStatus(char *buf) {
  memset(stat_info_obj.ts_status_ctrlr_init, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_status_ctrlr_init, buf, sizeof(stat_info_obj.ts_status_ctrlr_init));
  ALOGV("%s:is %s", __func__, stat_info_obj.ts_status_ctrlr_init);
}

void BtState::SetWakeLockAcqTimeName(char* buf) {
  memset(stat_info_obj.last_wk_lck_acq_info, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_wk_lck_acq_info, buf, sizeof(stat_info_obj.last_wk_lck_acq_info));
  ALOGV("%s:is %s", __func__, stat_info_obj.last_wk_lck_acq_info);
}

void BtState::SetHelthStatsTimerStatus(char* buf) {
  memset(stat_info_obj.health_timer_status, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.health_timer_status, buf, sizeof(stat_info_obj.health_timer_status));
  ALOGV("%s:is %s", __func__, stat_info_obj.health_timer_status);
}

void BtState::SetWakeLockRelTimeName(char* buf) {
  memset(stat_info_obj.last_wk_lck_rel_info, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_wk_lck_rel_info, buf, sizeof(stat_info_obj.last_wk_lck_rel_info));
  ALOGV("%s:is %s", __func__, stat_info_obj.last_wk_lck_rel_info);
}

void BtState::SetLastHciCmdStack(char* buf) {
  memset(stat_info_obj.last_hci_cmd_stack, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_hci_cmd_stack, buf, sizeof(stat_info_obj.last_hci_cmd_stack));
  ALOGV("%s:is %s", __func__, stat_info_obj.last_hci_cmd_stack);
}

void BtState::SetUARTFlowOn(char *buf) {
  memset(stat_info_obj.ts_uart_flow_on, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_uart_flow_on, buf, sizeof(stat_info_obj.ts_uart_flow_on));
  ALOGV("%s:is %s", __func__, stat_info_obj.ts_uart_flow_on);
}

void BtState::SetUARTFlowOff(char *buf) {
  memset(stat_info_obj.ts_uart_flow_off, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_uart_flow_off, buf, sizeof(stat_info_obj.ts_uart_flow_off));
  ALOGV("%s:is %s", __func__, stat_info_obj.ts_uart_flow_off);
}

void BtState::SetUARTClockOn(char *buf) {
  memset(stat_info_obj.ts_uart_clock_on, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_uart_clock_on, buf, sizeof(stat_info_obj.ts_uart_clock_on));
  ALOGV("%s:is %s", __func__, stat_info_obj.ts_uart_clock_on);
}

void BtState::SetUARTClockOff(char *buf) {
  memset(stat_info_obj.ts_uart_clock_off, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_uart_clock_off, buf, sizeof(stat_info_obj.ts_uart_clock_off));
  ALOGV("%s:is %s", __func__, stat_info_obj.ts_uart_clock_off);
}

void BtState::SetTsStatusOfCbSent(char *buf) {
  memset(stat_info_obj.ts_status_init_cb_sent, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_status_init_cb_sent, buf, sizeof(stat_info_obj.ts_status_init_cb_sent));
  ALOGV("%s:is %s", __func__, stat_info_obj.ts_status_init_cb_sent);
}
void BtState::SetLastStatsInfo(char *buf) {
  memset(stat_info_obj.last_health_stat_info, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_health_stat_info, buf, sizeof(stat_info_obj.last_health_stat_info));
  ALOGV("%s:is %s", __func__, stat_info_obj.last_health_stat_info);
}

void BtState::SetLastWakeIndFromHost(char* buf) {
  memset(stat_info_obj.last_wk_ind_from_host, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_wk_ind_from_host, buf, sizeof(stat_info_obj.last_wk_ind_from_host));
  ALOGV("%s:is %s", __func__, stat_info_obj.last_wk_ind_from_host);
}

void BtState::SetLastWakeAckFromHost(char* buf) {
  memset(stat_info_obj.last_wk_ack_from_host, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_wk_ack_from_host, buf, sizeof(stat_info_obj.last_wk_ack_from_host));
  ALOGV("%s:is %s", __func__, stat_info_obj.last_wk_ack_from_host);
}

void BtState::SetLastWakeIndFromSoc(char* buf) {
  memset(stat_info_obj.last_wk_ind_from_soc, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_wk_ind_from_soc, buf, sizeof(stat_info_obj.last_wk_ind_from_soc));
  ALOGV("%s:is %s", __func__, buf);
}

void BtState::SetLastWakeAckFromSoc(char* buf) {
  memset(stat_info_obj.last_wk_ack_from_soc, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_wk_ack_from_soc, buf, sizeof(stat_info_obj.last_wk_ack_from_soc));
  ALOGV("%s:is %s", __func__, stat_info_obj.last_wk_ack_from_soc);
}

void BtState::SetLastSleepIndFromHost(char* buf) {
  memset(stat_info_obj.last_sleep_ind_from_host, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_sleep_ind_from_host, buf,
          sizeof(stat_info_obj.last_sleep_ind_from_host));
  ALOGV("%s:is %s", __func__, stat_info_obj.last_sleep_ind_from_host);
}

void BtState::SetLastSleepIndFromSoc(char* buf) {
  memset(stat_info_obj.last_sleep_ind_from_soc, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.last_sleep_ind_from_soc, buf, sizeof(stat_info_obj.last_sleep_ind_from_soc));
  ALOGV("%s:is %s", __func__, stat_info_obj.last_sleep_ind_from_soc);
}

void BtState::SetPreStackEvent(char* buf) {
  memset(stat_info_obj.pre_stack_event, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.pre_stack_event, buf, sizeof(stat_info_obj.pre_stack_event));
  ALOGV("%s:is %s", __func__, stat_info_obj.pre_stack_event);
}

void BtState::SetPostStackEvent(char* buf) {
  memset(stat_info_obj.post_stack_event, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.post_stack_event, buf, sizeof(stat_info_obj.post_stack_event));
  ALOGV("%s:is %s", __func__, stat_info_obj.post_stack_event);
}

void BtState::SetPreStackAcl(char* buf) {
  memset(stat_info_obj.pre_stack_acl, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.pre_stack_acl, buf, sizeof(stat_info_obj.pre_stack_acl));
  ALOGV("%s:is %s", __func__, stat_info_obj.pre_stack_acl);
}

void BtState::SetPostStackAcl(char* buf) {
  memset(stat_info_obj.post_stack_acl, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.post_stack_acl, buf, sizeof(stat_info_obj.post_stack_acl));
  ALOGV("%s:is %s", __func__, stat_info_obj.post_stack_acl);
}

void BtState::SetPreSelect(char* buf) {
  memset(stat_info_obj.pre_select, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.pre_select, buf, sizeof(stat_info_obj.pre_select));
  ALOGV("%s:is %s", __func__, stat_info_obj.pre_select);
}

void BtState::SetPostSelect(char* buf) {
  memset(stat_info_obj.post_select, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.post_select, buf, sizeof(stat_info_obj.post_select));
  ALOGV("%s:is %s", __func__, stat_info_obj.post_select);
}

void BtState::SetPCAddr(char* buf) {
  memset(stat_info_obj.pc_addr, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.pc_addr, buf, sizeof(stat_info_obj.pc_addr));
  ALOGV("%s:is %s", __func__, stat_info_obj.pc_addr);
}

void BtState::SetBuildVersion(char* buf) {
  memset(stat_info_obj.build_ver, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.build_ver, buf, sizeof(stat_info_obj.build_ver));
  ALOGV("%s:is %s", __func__, stat_info_obj.build_ver);
}

void BtState::SetGetVerReqTS(char* buf, int type) {

  if (type == SEND_CMD_TS) {
    memset(stat_info_obj.ts_getver_start_send_cmd, 0, MAX_BUFF_SIZE);
    strlcpy(stat_info_obj.ts_getver_start_send_cmd, buf,
            sizeof(stat_info_obj.ts_getver_start_send_cmd));
    ALOGV("%s: %s", __func__, stat_info_obj.ts_getver_start_send_cmd);
  } else {
    memset(stat_info_obj.ts_getver_start_read_rsp, 0, MAX_BUFF_SIZE);
    strlcpy(stat_info_obj.ts_getver_start_read_rsp, buf,
            sizeof(stat_info_obj.ts_getver_start_read_rsp));
    ALOGV("%s: %s", __func__, stat_info_obj.ts_getver_start_read_rsp);
  }
}

void BtState::SetGetVerResEvtTS(char* buf) {
  memset(stat_info_obj.ts_getver_rcvd, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_getver_rcvd, buf, sizeof(stat_info_obj.ts_getver_rcvd));
  ALOGV("%s:is %s", __func__, stat_info_obj.ts_getver_rcvd);
}

void BtState::SetLastTxPacket(uint8_t *buf, uint16_t len, HciPacketType type) {
  struct timeval tv;
  ALOGV("%s", __func__);
  gettimeofday(&tv, NULL);
  stat_info_obj.actual_tx_pkt_len = len + 1; //1 for type
  AddLogTag(stat_info_obj.last_tx_pkt_ts, tv, (char *)"Last TX packet and timestamp ");
  stat_info_obj.last_tx_pkt_asc[0] = (char)type;
  stat_info_obj.last_tx_pkt_type = type;
  /*We are restricting size of packet upto 20 bytes due to performance issue*/
  if (len > TX_RX_PKT_ASC_SIZE -1) {
    len = TX_RX_PKT_ASC_SIZE ;
    stat_info_obj.last_tx_pkt_len = len;
    memcpy(&stat_info_obj.last_tx_pkt_asc[1], (char *)buf, len-1);
    ALOGV("Actual TX pkt size %d, Pkt restricted to size %d",
          stat_info_obj.actual_tx_pkt_len, TX_RX_PKT_ASC_SIZE);
  } else {
    stat_info_obj.last_tx_pkt_len = len + 1;
    memcpy(&stat_info_obj.last_tx_pkt_asc[1], (char *)buf, len);
  }
}

void BtState::SetLastRxPacket(uint8_t *buf, uint16_t len, HciPacketType type) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  stat_info_obj.actual_rx_pkt_len = len + 1; //1 for type
  ALOGV("%s", __func__);
  /*We are restricting size of packet upto 20 bytes due to performance issue*/
  if (stat_info_obj.ssr_trigger) {
    AddLogTag(stat_info_obj.ssr_rx_pkt_ts, tv, (char *)"Last RX packet before SSR and timestamp");
    stat_info_obj.ssr_rx_pkt_asc[0] = (char)type;
    stat_info_obj.ssr_rx_pkt_type = type;
    if (len > TX_RX_PKT_ASC_SIZE -1) {
      stat_info_obj.actual_ssr_rx_pkt_len = len;
      stat_info_obj.ssr_rx_pkt_len = TX_RX_PKT_ASC_SIZE;
      memcpy(&stat_info_obj.ssr_rx_pkt_asc[1], (char *)buf, stat_info_obj.ssr_rx_pkt_len-1);
      ALOGV("Actual RX pkt len before SSR %d, Pkt restricted to size %d",
            stat_info_obj.actual_ssr_rx_pkt_len, TX_RX_PKT_ASC_SIZE);
    } else {
      stat_info_obj.ssr_rx_pkt_len = len + 1;
      memcpy(&stat_info_obj.ssr_rx_pkt_asc[1], (char *)buf, len);
    }
    stat_info_obj.ssr_trigger = false;
  }


  AddLogTag(stat_info_obj.last_rx_pkt_ts, tv, (char *)"Last RX packet and timestamp");
  stat_info_obj.last_rx_pkt_asc[0] = (char)type;
  stat_info_obj.last_rx_pkt_type = type;
  if (len > TX_RX_PKT_ASC_SIZE -1) {
    stat_info_obj.actual_rx_pkt_len = len + 1; //1 for type
    len = TX_RX_PKT_ASC_SIZE ;
    stat_info_obj.last_rx_pkt_len = len;
    memcpy(&stat_info_obj.last_rx_pkt_asc[1], (char *)buf, len-1);
    ALOGV("Actual RX pkt size %d, Pkt restricted to size %d",
          stat_info_obj.actual_rx_pkt_len, TX_RX_PKT_ASC_SIZE);
  } else {
    stat_info_obj.last_rx_pkt_len = len + 1; //1 for type
    memcpy(&stat_info_obj.last_rx_pkt_asc[1], (char *)buf, len);
  }
}

void BtState::SetSSRtrigger(bool status) {
  stat_info_obj.ssr_trigger =  status;
  ALOGV("%s:is %d", __func__, status);
}

void BtState::SetAlwaysOnEnabled(bool status) {
  if (status)
    strlcpy(stat_info_obj.alwayson_status, "SOC AlwaysOn enabled", MAX_BUFF_SIZE);
  else
    strlcpy(stat_info_obj.alwayson_status, "SOC AlwaysOn disabled", MAX_BUFF_SIZE);

  ALOGV("%s:is %d", __func__, status);
}

void BtState::SetLastSeqTS(char *buf) {
  memset(stat_info_obj.ts_last_seq_number, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.ts_last_seq_number, buf, sizeof(stat_info_obj.ts_last_seq_number));
  ALOGV("%s:is %s", __func__, stat_info_obj.ts_last_seq_number);
}

void BtState::SetReadThreadStatus(char *buf) {
  memset(stat_info_obj.status_read_thread, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.status_read_thread, buf, sizeof(stat_info_obj.status_read_thread));
  ALOGV("%s:is %s", __func__, stat_info_obj.status_read_thread);
}

void BtState::SetPrimaryCrashReason(char *buf) {
  memset(stat_info_obj.pri_crash_reason, 0, MAX_CRASH_BUFF_SIZE);
  strlcpy(stat_info_obj.pri_crash_reason, buf, sizeof(stat_info_obj.pri_crash_reason));
  ALOGV("%s:is %s", __func__, stat_info_obj.pri_crash_reason);
}

void BtState::SetSecondaryCrashReason(char *buf) {
  memset(stat_info_obj.sec_crash_reason, 0, MAX_CRASH_BUFF_SIZE);
  strlcpy(stat_info_obj.sec_crash_reason, buf, sizeof(stat_info_obj.sec_crash_reason));
  ALOGV("%s:is %s", __func__, stat_info_obj.sec_crash_reason);
}

void BtState::SetCrashTS(char *buf) {
  memset(stat_info_obj.crash_ts, 0, MAX_CRASH_BUFF_SIZE);
  strlcpy(stat_info_obj.crash_ts, buf, sizeof(stat_info_obj.crash_ts));
  ALOGV("%s:is %s", __func__, stat_info_obj.crash_ts);
}

void BtState::SetKernelTimeStamp(char *buf) {
  memset(stat_info_obj.kernel_time, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.kernel_time, buf, sizeof(stat_info_obj.kernel_time));
  ALOGV("%s:is %s", __func__, stat_info_obj.kernel_time);
}

void BtState::SetDiagDeInitPreTS(char *buf) {
  memset(stat_info_obj.pre_diag_deinit_ts, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.pre_diag_deinit_ts, buf, sizeof(stat_info_obj.pre_diag_deinit_ts));
  ALOGV("%s:is %s", __func__, stat_info_obj.pre_diag_deinit_ts);
}

void BtState::SetDiagDeInitPostTS(char *buf) {
  memset(stat_info_obj.post_diag_deinit_ts, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.post_diag_deinit_ts, buf, sizeof(stat_info_obj.post_diag_deinit_ts));
  ALOGV("%s:is %s", __func__, stat_info_obj.post_diag_deinit_ts);
}

void BtState::SetFwDwnldSucceed(char *buf) {
  memset(stat_info_obj.fw_dwnld_success, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.fw_dwnld_success, buf, sizeof(stat_info_obj.fw_dwnld_success));
  ALOGV("%s:is %s", __func__, stat_info_obj.fw_dwnld_success);
}

void BtState::SetFwSuBuildInfo(char *buf) {
  memset(stat_info_obj.fw_su_build_info, 0, MAX_BUFF_SIZE);
  strlcpy(stat_info_obj.fw_su_build_info, buf, sizeof(stat_info_obj.fw_su_build_info));
  ALOGV("%s:is %s", __func__, stat_info_obj.fw_su_build_info);
}

void BtState::DeleteDumpsIfRequired()
{
  struct DIR* p_dir;
  struct dirent* p_dirent;
  char *first_entry = NULL;
  int count = 0;

  char *look_for = (char *)"ramdump_bt_state";

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

void BtState::DeleteDumps(char *first_entry)
{
  struct DIR* p_dir;
  struct dirent* p_dirent;
  int ret = 0;
  char *look_for = (char *)"ramdump_bt_state";
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

  strlcpy(timestamp, oldest_file + HAL_DUMP_TIMESTAMP_OFFSET + 1, HAL_DUMP_TIMESTAMP_LENGTH);

  p_dir = opendir(LOG_COLLECTION_DIR);
  if (p_dir == NULL) {
    ALOGE("%s: Cannot open dump location %s", __func__, LOG_COLLECTION_DIR);
    return;
  }

  while ((p_dirent = readdir(p_dir)) != NULL) {
   if (strstr(p_dirent->d_name, timestamp)) {
     strlcpy(path, LOG_COLLECTION_DIR, HAL_DUMP_SIZE);
     strlcat(path, p_dirent->d_name, HAL_DUMP_SIZE);
     ALOGV("%s: Deleting oldest dump file: %s", __func__, path);
     ret = remove(path);
     if(ret < 0) ALOGE("%s: Cannot delete file %s", __func__, path);
   }
  }
  closedir(p_dir);
}

void BtState::DumpBtState() {
  int fd;
  char file_name[BT_FILE_NAME_LEN];
  char hex_buff[3] = "";
  char dst_buff[MAX_BUFF_SIZE] = {'\0'};
  int j =0;

#ifndef DONT_DELETE_DUMPS_SET
  DeleteDumpsIfRequired();
#endif

  /* Get the exact time stamp of close */
  logger_ = static_cast<void*>(Logger::Get());
  static_cast<Logger*>(logger_)->GetStateFileName(file_name);

  fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
            | S_IROTH);
  if (fd < 0) {
     ALOGE("%s: File open (%s) failed: errno: %d", __func__, file_name, errno);
     return;
  }

  ALOGD("%s: Dumping stats into %s", __func__, file_name);

  ALOGD("%s", stat_info_obj.pri_crash_reason);
  write(fd, stat_info_obj.pri_crash_reason, strlen(stat_info_obj.pri_crash_reason));

  write(fd, " \n", 2);
  ALOGD("%s",stat_info_obj.sec_crash_reason);
  write(fd, stat_info_obj.sec_crash_reason, strlen(stat_info_obj.sec_crash_reason));

  write(fd, " \n", 2);
  ALOGD("%s",stat_info_obj.crash_ts);
  write(fd, stat_info_obj.crash_ts, strlen(stat_info_obj.crash_ts));

  ALOGD("%s ", stat_info_obj.kernel_time);
  write(fd, stat_info_obj.kernel_time, strlen(stat_info_obj.kernel_time));

  write(fd, " \n", 2);
  ALOGD("%s ", stat_info_obj.alwayson_status);
  write(fd, stat_info_obj.alwayson_status, strlen(stat_info_obj.alwayson_status));
  if (!soc_need_reload_patch) {

    write(fd, " \n", 2);
    ALOGD("%s ", stat_info_obj.ts_hci_reset_cmd);
    write(fd, stat_info_obj.ts_hci_reset_cmd, strlen(stat_info_obj.ts_hci_reset_cmd));

    write(fd, " \n", 2);
    ALOGD("%s ", stat_info_obj.ts_hci_reset_rsp);
    write(fd, stat_info_obj.ts_hci_reset_rsp, strlen(stat_info_obj.ts_hci_reset_rsp));
  }

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.ts_hci_initialize);
  write(fd, stat_info_obj.ts_hci_initialize, strlen(stat_info_obj.ts_hci_initialize));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.ts_hci_close);
  write(fd, stat_info_obj.ts_hci_close, strlen(stat_info_obj.ts_hci_close));
  if (soc_need_reload_patch) {
    write(fd, " \n", 2);
    ALOGD("%s", stat_info_obj.ts_getver_start_send_cmd);
    write(fd, stat_info_obj.ts_getver_start_send_cmd,
          strlen(stat_info_obj.ts_getver_start_send_cmd));

    write(fd, " \n", 2);
    ALOGD("%s", stat_info_obj.ts_getver_start_read_rsp);
    write(fd, stat_info_obj.ts_getver_start_read_rsp,
          strlen(stat_info_obj.ts_getver_start_read_rsp));

    write(fd, " \n", 2);
    ALOGD("%s", stat_info_obj.ts_getver_rcvd);
    write(fd, stat_info_obj.ts_getver_rcvd, strlen(stat_info_obj.ts_getver_rcvd));

    write(fd, " \n", 2);
    ALOGD("%s", stat_info_obj.fw_dwnld_success);
    write(fd, stat_info_obj.fw_dwnld_success, strlen(stat_info_obj.fw_dwnld_success));

    write(fd, " \n", 2);
    ALOGD("%s", stat_info_obj.fw_su_build_info);
    write(fd, stat_info_obj.fw_su_build_info, strlen(stat_info_obj.fw_su_build_info));
  }
  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.status_read_thread);
  write(fd, stat_info_obj.status_read_thread, strlen(stat_info_obj.status_read_thread));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.ts_status_diag_init);
  write(fd, stat_info_obj.ts_status_diag_init, strlen(stat_info_obj.ts_status_diag_init));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.ts_status_ctrlr_init);
  write(fd, stat_info_obj.ts_status_ctrlr_init, strlen(stat_info_obj.ts_status_ctrlr_init));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.ts_status_init_cb_sent);
  write(fd, stat_info_obj.ts_status_init_cb_sent, strlen(stat_info_obj.ts_status_init_cb_sent));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.ts_uart_flow_on);
  write(fd, stat_info_obj.ts_uart_flow_on, strlen(stat_info_obj.ts_uart_flow_on));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.ts_uart_flow_off);
  write(fd, stat_info_obj.ts_uart_flow_off, strlen(stat_info_obj.ts_uart_flow_off));

  write(fd, " \n", 2);
  ALOGD("%s ", stat_info_obj.pre_diag_deinit_ts);
  write(fd, stat_info_obj.pre_diag_deinit_ts, strlen(stat_info_obj.pre_diag_deinit_ts));

  write(fd, " \n", 2);
  ALOGD("%s ", stat_info_obj.post_diag_deinit_ts);
  write(fd, stat_info_obj.post_diag_deinit_ts, strlen(stat_info_obj.post_diag_deinit_ts));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.health_timer_status);
  write(fd, stat_info_obj.health_timer_status, strlen(stat_info_obj.health_timer_status));

  write(fd, " \n", 2);
  HealthInfoLog::Get()->ReportHealthInfo();
  ALOGD("%s", stat_info_obj.last_health_stat_info);
  write(fd, stat_info_obj.last_health_stat_info, strlen(stat_info_obj.last_health_stat_info));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.last_hci_cmd_stack);
  write(fd, stat_info_obj.last_hci_cmd_stack, strlen(stat_info_obj.last_hci_cmd_stack));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.ts_last_seq_number);
  write(fd, stat_info_obj.ts_last_seq_number, strlen(stat_info_obj.ts_last_seq_number));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.last_wk_ind_from_host);
  write(fd, stat_info_obj.last_wk_ind_from_host, strlen(stat_info_obj.last_wk_ind_from_host));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.last_wk_ack_from_soc);
  write(fd, stat_info_obj.last_wk_ack_from_soc, strlen(stat_info_obj.last_wk_ack_from_soc));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.last_wk_ind_from_soc);
  write(fd, stat_info_obj.last_wk_ind_from_soc, strlen(stat_info_obj.last_wk_ind_from_soc));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.last_wk_ack_from_host);
  write(fd, stat_info_obj.last_wk_ack_from_host, strlen(stat_info_obj.last_wk_ack_from_host));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.last_sleep_ind_from_host);
  write(fd, stat_info_obj.last_sleep_ind_from_host, strlen(stat_info_obj.last_sleep_ind_from_host));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.last_sleep_ind_from_soc);
  write(fd, stat_info_obj.last_sleep_ind_from_soc, strlen(stat_info_obj.last_sleep_ind_from_soc));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.pre_stack_event);
  write(fd, stat_info_obj.pre_stack_event, strlen(stat_info_obj.pre_stack_event));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.post_stack_event);
  write(fd, stat_info_obj.post_stack_event, strlen(stat_info_obj.post_stack_event));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.pre_stack_acl);
  write(fd, stat_info_obj.pre_stack_acl, strlen(stat_info_obj.pre_stack_acl));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.post_stack_acl);
  write(fd, stat_info_obj.post_stack_acl, strlen(stat_info_obj.post_stack_acl));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.pre_select);
  write(fd, stat_info_obj.pre_select, strlen(stat_info_obj.pre_select));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.post_select);
  write(fd, stat_info_obj.post_select, strlen(stat_info_obj.post_select));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.ts_uart_clock_on);
  write(fd, stat_info_obj.ts_uart_clock_on, strlen(stat_info_obj.ts_uart_clock_on));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.ts_uart_clock_off);
  write(fd, stat_info_obj.ts_uart_clock_off, strlen(stat_info_obj.ts_uart_clock_off));

  //last Tx packet
  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.last_tx_pkt_ts);
  write(fd, stat_info_obj.last_tx_pkt_ts, strlen(stat_info_obj.last_tx_pkt_ts));

  {
    j =0;
    memset(hex_buff, '\0', 3);
    for (int i = 0; i < stat_info_obj.last_tx_pkt_len; i++) {
      snprintf(hex_buff, 3, "%.2X ", stat_info_obj.last_tx_pkt_asc[i]);
      stat_info_obj.last_tx_pkt_hex[j++] = hex_buff[0];
      stat_info_obj.last_tx_pkt_hex[j++] = hex_buff[1];
      stat_info_obj.last_tx_pkt_hex[j++] = 32;
    }
    stat_info_obj.last_tx_pkt_hex [j] = '\0';
    write(fd, " \n", 2);
    write(fd, stat_info_obj.last_tx_pkt_hex, strlen(stat_info_obj.last_tx_pkt_hex));
    ALOGD("%s", stat_info_obj.last_tx_pkt_hex);
    snprintf(dst_buff, sizeof(dst_buff), "Actual last TX pkt len = %d",
             stat_info_obj.actual_tx_pkt_len);
    write(fd, " \n", 2);
    write(fd, dst_buff, strlen(dst_buff));
    ALOGD("Actual last TX pkt len: %d", stat_info_obj.actual_tx_pkt_len);
  }

  //last Rx packet before SSR
  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.ssr_rx_pkt_ts);
  write(fd, stat_info_obj.ssr_rx_pkt_ts, strlen(stat_info_obj.ssr_rx_pkt_ts));

  {
    j =0;
    memset(hex_buff, '\0', 3);
    for (int i = 0; i < stat_info_obj.ssr_rx_pkt_len; i++) {
      snprintf(hex_buff, 3, "%.2X ", stat_info_obj.ssr_rx_pkt_asc[i]);
      stat_info_obj.ssr_rx_pkt_hex[j++] = hex_buff[0];
      stat_info_obj.ssr_rx_pkt_hex[j++] = hex_buff[1];
      stat_info_obj.ssr_rx_pkt_hex[j++] = 32;
    }
    stat_info_obj.ssr_rx_pkt_hex [j] = '\0';
    write(fd, " \n", 2);
    write(fd, stat_info_obj.ssr_rx_pkt_hex, strlen(stat_info_obj.ssr_rx_pkt_hex));
    ALOGD("%s", stat_info_obj.ssr_rx_pkt_hex);
    memset(dst_buff, '\0', MAX_BUFF_SIZE);
    snprintf(dst_buff, sizeof(dst_buff), "Actual RX pkt len before SSR = %d",
             stat_info_obj.actual_ssr_rx_pkt_len);
    write(fd, " \n", 2);
    write(fd, dst_buff, strlen(dst_buff));
    ALOGD("Actual RX pkt len before SSR: %d", stat_info_obj.actual_ssr_rx_pkt_len);
  }

  //last Rx packet
  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.last_rx_pkt_ts);
  write(fd, stat_info_obj.last_rx_pkt_ts, strlen(stat_info_obj.last_rx_pkt_ts));

  {
    j =0;
    memset(hex_buff, '\0', 3);
    for (int i = 0; i < stat_info_obj.last_rx_pkt_len; i++) {
      snprintf(hex_buff, 3, "%.2X ", stat_info_obj.last_rx_pkt_asc[i]);
      stat_info_obj.last_rx_pkt_hex[j++] = hex_buff[0];
      stat_info_obj.last_rx_pkt_hex[j++] = hex_buff[1];
      stat_info_obj.last_rx_pkt_hex[j++] = 32;
    }
    stat_info_obj.last_rx_pkt_hex [j] = '\0';
    write(fd, " \n", 2);
    write(fd, stat_info_obj.last_rx_pkt_hex, strlen(stat_info_obj.last_rx_pkt_hex));
    ALOGD("%s", stat_info_obj.last_rx_pkt_hex);
    memset(dst_buff, '\0', MAX_BUFF_SIZE);
    snprintf(dst_buff, sizeof(dst_buff), "Actual last RX pkt len = %d",
             stat_info_obj.actual_rx_pkt_len);
    write(fd, " \n", 2);
    write(fd, dst_buff, strlen(dst_buff));
    ALOGD("Actual last RX pkt len: %d", stat_info_obj.actual_rx_pkt_len);
  }

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.last_wk_lck_acq_info);
  write(fd, stat_info_obj.last_wk_lck_acq_info, strlen(stat_info_obj.last_wk_lck_acq_info));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.last_wk_lck_rel_info);
  write(fd, stat_info_obj.last_wk_lck_rel_info, strlen(stat_info_obj.last_wk_lck_rel_info));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.pc_addr);
  write(fd, stat_info_obj.pc_addr, strlen(stat_info_obj.pc_addr));

  write(fd, " \n", 2);
  ALOGD("%s", stat_info_obj.build_ver);
  write(fd, stat_info_obj.build_ver, strlen(stat_info_obj.build_ver));

  if (fsync(fd) == -1) {
    ALOGE("%s: Error while synchronization of logs in :%s error code:%s",
          __func__, file_name, strerror(errno));
  }
  close(fd);
}
} // namespace implementation
} // namespace V1_0
} // namespace bluetooth
} // namespace hardware
} // namespace android
