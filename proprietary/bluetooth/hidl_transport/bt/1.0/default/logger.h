/*==========================================================================
Description
  It has implementation for logger class

# Copyright (c) 2017 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#pragma once

#include <hidl/HidlSupport.h>
#include "ring_buffer.h"
#include "uart_ipc.h"
#include "hci_internals.h"
#include "diag_interface.h"
#include "state_info.h"
/* Packet types */
#define LOG_BT_CMD_PACKET_TYPE     0x01
#define LOG_BT_ACL_PACKET_TYPE     0x02
#define LOG_BT_SCO_PACKET_TYPE     0x03
#define LOG_BT_EVT_PACKET_TYPE     0x04
#define LOG_ANT_CTL_PACKET_TYPE    0x0c
#define LOG_ANT_DATA_PACKET_TYPE   0x0e
#define LOG_BT_EVT_VENDOR_SPECIFIC 0xFF
#define LOG_BT_EVT_CMD_CMPLT 0x0E

/* Message type of the log from controller */
#define LOG_BT_CONTROLLER_LOG        0x01
#define LOG_BT_MESSAGE_TYPE_VSTR     0x02
#define LOG_BT_MESSAGE_TYPE_PACKET   0x05
#define LOG_BT_MESSAGE_TYPE_MEM_DUMP 0x08

/* Sub log ID for the message type PACKET */
#define LOG_BT_HCI_CMD   0
#define LOG_BT_HCI_EVENT 1

#define LOG_BT_RX_LMP_PDU      18
#define LOG_BT_TX_LMP_PDU      19
#define LOG_BT_RX_LE_CTRL_PDU  20
#define LOG_BT_TX_LE_CTRL_PDU  21
#define LOG_BT_TX_LE_CONN_MNGR 22

#define LOG_BT_LINK_MANAGER_STATE    0x80
#define LOG_BT_CONN_MANAGER_STATE    0x81
#define LOG_BT_SECURITY_STATE        0x82
#define LOG_BT_LE_CONN_MANAGER_STATE 0x83
#define LOG_BT_LE_CHANNEL_MAP_STATE  0x84
#define LOG_BT_LE_ENCRYPTION_STATE   0x85

/* Sub log ID for the message type VSTR */
#define LOG_BT_VSTR_ERROR 0
#define LOG_BT_VSTR_HIGH  1
#define LOG_BT_VSTR_LOW   2

/* Diag ID for LMP packers */
#define LOG_BT_DIAG_LMP_LOG_ID 0x1041
#define LOG_BT_DIAG_LMP_RX_ID  0x1048
#define LOG_BT_DIAG_LMP_TX_ID  0x1049

/* To format LMP logs */
#define LOG_BT_QXDM_PKT_LENGTH_POS    0
#define LOG_BT_QXDM_PKT_LENGTH2_POS   1
#define LOG_BT_QXDM_DEVICE_IDX_POS    2
#define LOG_BT_QXDM_PKT_POS           3

#define LOG_BT_DBG_DEVICE_IDX_POS 0
#define LOG_BT_DBG_PKT_LENGTH_POS 1
#define LOG_BT_DBG_PKT_POS 2

/* Headed size of the log */
#define LOG_BT_HEADER_SIZE (sizeof(bt_log_pkt) - 1)
#define CRASH_SOURCE_FILE_PATH_LEN 50

#define MAX_RING_BUF_SIZE (8*1024)
#define OUT_RING_BUF 0
#define IN_RING_BUF 1

#define LOG_I(tag, fmt, args...) ALOG(LOG_INFO, tag, fmt, ## args)
#define LOG_W(tag, fmt, args...) ALOG(LOG_WARN, tag, fmt, ## args)
#define LOG_E(tag, fmt, args...) ALOG(LOG_ERROR, tag, fmt, ## args)

#define IBS_LOG_INFO(tag, fmt, args...) if(Logger.GetIbsLogLevel() == Logger::log_level::HIGH)LOG_I(tag, fmt, ##args)
#define IBS_LOG_WAR(tag, fmt, args...) if(Logger.GetIbsLogLevel() >= Logger::log_level::MID)LOG_W(tag, fmt, ##args)
#define IBS_LOG_ERR(tag, fmt, args...) LOG_E(tag, fmt, ##args)

#define RX_LOG_INFO(tag, fmt, args...) if(Logger.GetRxLogLevel() == Logger::log_level::HIGH)LOG_I(tag, fmt, ##args)
#define RX_LOG_WAR(tag, fmt, args...) if(Logger.GetRxLogLevel() >= Logger::log_level::MID)LOG_W(tag, fmt, ##args)
#define RX_LOG_ERR(tag, fmt, args...) LOG_E(tag, fmt, ##args)

#define TX_LOG_INFO(tag, fmt, args...) if(Logger.GetTxLogLevel() == Logger::log_level::HIGH)LOG_I(tag, fmt, ##args)
#define TX_LOG_WAR(tag, fmt, args...) if(Logger.GetTxLogLevel() >= Logger::log_level::MID)LOG_W(tag, fmt, ##args)
#define TX_LOG_ERR(tag, fmt, args...) LOG_E(tag, fmt, ##args)

#define GEN_LOG_INFO(tag, fmt, args...) if(Logger.GetGenLogLevel() == Logger::log_level::HIGH)LOG_I(tag, fmt, ##args)
#define GEN_LOG_WAR(tag, fmt, args...) if(Logger.GetGenLogLevel() >= Logger::log_level::MID)LOG_W(tag, fmt, ##args)
#define GEN_LOG_ERR(tag, fmt, args...) LOG_E(tag, fmt, ##args)

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {
using CrashDumpStatusCallback = std::function<void(bool)>;
using HWEventCallback = std::function<void()>;

typedef enum {
  SEC_REASON_DEFAULT                 =  0x00,

  // SoC Crash Reasons
  SEC_REASON_SOCCRASH_RX_NULL        =  0x01,
  SEC_REASON_SOCCRASH_TX_RX_INVALID_PKT = 0x40,
  SEC_REASON_SOCCRASH_TX_RX_INVALID_LEN = 0x41,
  SEC_REASON_SOCCRASH_TX_RX_INVALID_PKT_FATAL = 0xC0,
  SEC_REASON_SOCCRASH_TX_RX_INVALID_LEN_FATAL = 0xC1,
  SEC_REASON_SOCCRASH_UNKNOWN        =  0x81,
  SEC_REASON_SOCCRASH_SW_REQUESTED   =  0x82,
  SEC_REASON_SOCCRASH_STACK_OVERFLOW =  0x83,
  SEC_REASON_SOCCRASH_EXCEPTION      =  0x84,
  SEC_REASON_SOCCRASH_ASSERT         =  0x85,
  SEC_REASON_SOCCRASH_TRAP           =  0x86,
  SEC_REASON_SOCCRASH_OS_FATAL       =  0x87,
  SEC_REASON_SOCCRASH_HCI_RESET      =  0x88,
  SEC_REASON_SOCCRASH_PATCH_RESET    =  0x89,
  SEC_REASON_SOCCRASH_ABT            =  0x8A,
  SEC_REASON_SOCCRASH_RAMMASK        =  0x8B,
  SEC_REASON_SOCCRASH_PREBARK        =  0x8C,
  SEC_REASON_SOCCRASH_BUSERROR       =  0x8D,
  SEC_REASON_SOCCRASH_IO_FATAL       =  0x8E,
  SEC_REASON_SOCCRASH_SSR_CMD        =  0x8F,
  SEC_REASON_SOCCRASH_POWERON        =  0x90,
  SEC_REASON_SOCCRASH_WATCHDOG       =  0x91,

  // Transport Driver Crash Reasons
  SEC_REASON_BTON_UARTINIT_STUCK        =  0xB1,
  SEC_REASON_BTON_GETVER_SEND_STUCK     =  0xB2,
  SEC_REASON_BTON_GETVER_NO_RSP_RCVD    =  0xB3,
  SEC_REASON_BTON_SETBAUDRATE_CMD_STUCK =  0xB4,
  SEC_REASON_BTON_PATCH_DNLD_STUCK      =  0xB5,
  SEC_REASON_BTON_GETBOARDID_CMD_STUCK  =  0xB6,
  SEC_REASON_BTON_NVM_DNLD_STUCK        =  0xB7,
  SEC_REASON_BTON_HCI_RESET_STUCK       =  0xB8,
  SEC_REASON_BTON_GETBLDINFO_CMD_STUCK  =  0xB9,
  SEC_REASON_BTON_ADDONFEAT_CMD_STUCK   =  0xBA,
  SEC_REASON_BTON_ENHLOG_CMD_STUCK      =  0xBB,
  SEC_REASON_BTON_DIAGINIT_STUCK        =  0xBC,
  SEC_REASON_BTOFF_DIAGDEINIT_STUCK     =  0xBD,
} SecondaryReasonCode;

typedef struct {
  SecondaryReasonCode reason;
  char reasonstr[50];
} SecondaryReasonMap;

static SecondaryReasonMap secReasonMap[] = {
  { SEC_REASON_DEFAULT                          ,    "Default"},
  { SEC_REASON_SOCCRASH_RX_NULL                 ,    "Rx Null"},
  { SEC_REASON_SOCCRASH_UNKNOWN                 ,    "Unknown"},
  { SEC_REASON_SOCCRASH_TX_RX_INVALID_PKT       ,    "Tx/Rx invalid packet"},
  { SEC_REASON_SOCCRASH_TX_RX_INVALID_LEN       ,    "Tx/Rx invalid len"},
  { SEC_REASON_SOCCRASH_TX_RX_INVALID_PKT_FATAL ,    "Tx/Rx invalid packet fatal error"},
  { SEC_REASON_SOCCRASH_TX_RX_INVALID_LEN_FATAL ,    "Tx/Rx invalid lenght fatal error"},
  { SEC_REASON_SOCCRASH_SW_REQUESTED            ,    "SW Requested"},
  { SEC_REASON_SOCCRASH_STACK_OVERFLOW          ,    "Stack Overflow"},
  { SEC_REASON_SOCCRASH_EXCEPTION               ,    "Exception"},
  { SEC_REASON_SOCCRASH_ASSERT                  ,    "Assert"},
  { SEC_REASON_SOCCRASH_TRAP                    ,    "Trap"},
  { SEC_REASON_SOCCRASH_OS_FATAL                ,    "OS Fatal"},
  { SEC_REASON_SOCCRASH_HCI_RESET               ,    "HCI Reset"},
  { SEC_REASON_SOCCRASH_PATCH_RESET             ,    "Patch Reset"},
  { SEC_REASON_SOCCRASH_ABT                     ,    "SoC Abort"},
  { SEC_REASON_SOCCRASH_RAMMASK                 ,    "RAM MASK"},
  { SEC_REASON_SOCCRASH_PREBARK                 ,    "PREBARK"},
  { SEC_REASON_SOCCRASH_BUSERROR                ,    "Bus error"},
  { SEC_REASON_SOCCRASH_IO_FATAL                ,    "IO fatal eror"},
  { SEC_REASON_SOCCRASH_SSR_CMD                 ,    "SSR CMD"},
  { SEC_REASON_SOCCRASH_POWERON                 ,    "Power ON"},
  { SEC_REASON_SOCCRASH_WATCHDOG                ,    "Watchdog"},
  { SEC_REASON_BTON_UARTINIT_STUCK              ,    "UartInitStuck"},
  { SEC_REASON_BTON_GETVER_SEND_STUCK           ,    "GetVerSendStuck"},
  { SEC_REASON_BTON_GETVER_NO_RSP_RCVD          ,    "GetVerNoRspRcvd"},
  { SEC_REASON_BTON_SETBAUDRATE_CMD_STUCK       ,    "SetBaudRateStuck"},
  { SEC_REASON_BTON_PATCH_DNLD_STUCK            ,    "PatchDnldStuck"},
  { SEC_REASON_BTON_GETBOARDID_CMD_STUCK        ,    "GetBoardIdStuck"},
  { SEC_REASON_BTON_NVM_DNLD_STUCK              ,    "NvmDnldStuck"},
  { SEC_REASON_BTON_HCI_RESET_STUCK             ,    "HciResetStuck"},
  { SEC_REASON_BTON_GETBLDINFO_CMD_STUCK        ,    "GetBldInfoCmdStuck"},
  { SEC_REASON_BTON_ADDONFEAT_CMD_STUCK         ,    "AddOnFeatCmdStuck"},
  { SEC_REASON_BTON_ENHLOG_CMD_STUCK            ,    "EnhLogCmdStuck"},
  { SEC_REASON_BTON_DIAGINIT_STUCK              ,    "DiagInitStuck"},
  { SEC_REASON_BTOFF_DIAGDEINIT_STUCK           ,    "DiagDeinitStuck"},
};

class Logger
{
 private:
  static Logger *instance_;
  static const diagpkt_user_table_entry_type ssr_bt_tbl_[];
  short int ibs_log_level_, rx_log_level_, tx_log_level_, gen_log_level_;
  int time_year, time_month, time_day, time_hour, time_min, time_sec;
  PrimaryReasonCode dump_reason_;
  RingBuffer snoop_buff_;
#ifdef DUMP_IPC_LOG
  IpcLogs ipc_log_;
#endif
  DiagInterface diag_interface_;
  bool is_crash_dump_in_progress_;
  CrashDumpStatusCallback crash_dump_status_cb_;
  HWEventCallback hw_event_cb_;
  BtState *state_info_;

#ifdef DIAG_ENABLED
  static void *SsrBtHandler(void *, uint16);
#endif

  bool HandleHciEvent(HciPacketType, uint8_t *, uint16_t);
  void SaveSocMemDump(uint8_t *, uint16_t, PrimaryReasonCode);
  void DeleteDumpsIfRequired();
  void DeleteDumps(char *);
  void SetDumpTimestamp();
  time_t time_rx_call_back;
 public:
  Logger();
  enum {
    LOW = 0,
    MID,
    HIGH,
  } log_level;

  static Logger *Get(void);
  void UnlockDiagMutex();
  void GetCrashDumpFileName(char*);
#ifdef DUMP_RINGBUF_LOG
  void GetSnoopFileName(char*);
  void SetNormalSsrTrigered(bool);
  void SaveIbsToRingBuf(HciPacketType type,  uint8_t ibs_data) {
    /* IBS is a one byte command or event sent or received */
    snoop_buff_.AddLog(RingBuffer::IBS_BYTE, type, &ibs_data, 1);
  };

  void DumpRingBuffers(void) {
    snoop_buff_.DumpData();
  };
#endif
#ifdef DUMP_IPC_LOG
  void GetUartDumpFilename(short int, char*);
  void DumpUartIpcLogs(void) {
    ipc_log_.DumpUartLogs();
  };
#endif

  bool GetCleanupStatus(ProtocolType ptype);
  void StoreCrashReason(void);
  bool FrameCrashEvent(hidl_vec<uint8_t>*bt_packet_);
  bool ProcessRx(HciPacketType type, const uint8_t *buff, uint16_t len);
  bool ProcessTx(HciPacketType, const uint8_t *, uint16_t);
  bool IsSnoopLogEnabled(void);
  bool IsHciFwSnoopEnabled(void);

  void Cleanup(void);

  bool IsControllerLogPkt(HciPacketType type, const uint8_t *buff, uint16_t len);
  bool IsCrashDumpStarted(void);
  bool RegisterCrashDumpCallback (CrashDumpStatusCallback crash_dump_cb);
  void RegisterHWEventCallback(HWEventCallback);
  bool PrepareDumpProcess();
  bool isSsrTriggered();
  bool isDiagSsrTriggered();
  void UpdateRxEventTag(RxThreadEventType rx_event) {
#ifdef DUMP_RINGBUF_LOG
    snoop_buff_.UpdateRxEventTag(rx_event);
#endif
  };
  void UpdateRxTimeStamp();
  void ResetSsrTriggeredFlag();
  bool SetSsrTriggeredFlag();
  void UnlockRingbufferMutex();

  void SaveLastStackHciCommand(char * cmd) {
#ifdef DUMP_RINGBUF_LOG
    snoop_buff_.SaveLastStackHciCommand(cmd);
#endif
  };

  void ResetCrashReasons();
  void SetPrimaryCrashReason(PrimaryReasonCode);
  void SetSecondaryCrashReason(SecondaryReasonCode);
  char *get_reset_reason_str(SecondaryReasonCode);
  void DecodeHwErrorEvt(uint8_t *buff);
  uint8_t GetClientStatus(ProtocolType);
  void SetClientStatus(bool , ProtocolType);
  bool IsPreviousItrCrashed();
  char *GetSecondaryCrashReason();
  PrimaryReasonCode GetPrimaryReason();

  void Init(HciTransport *hci_transport);

  void GetStateFileName(char*);

  inline short int GetIbsLogLevel() {return ibs_log_level_;};
  inline short int GetRxLogLevel() {return rx_log_level_;};
  inline short int GetTxLogLevel() {return tx_log_level_;};
  inline short int GetGenLogLevel() {return gen_log_level_;};
  void ResetForceSsrTriggeredFlag();
};

} // namespace implementation
} // namespace V1_0
} // namespace bluetooth
} // namespace hardware
} // namespace android
