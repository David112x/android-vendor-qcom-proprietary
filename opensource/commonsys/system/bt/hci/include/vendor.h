/******************************************************************************
 *
 *  Copyright (C) 2014 Google, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "bt_types.h"
#include "bt_vendor_lib.h"
#include "hci_internals.h"
#include "hci_layer.h"

#define HCI_BT_SOC_CRASHED_OGF    0xfc
#define HCI_BT_SOC_CRASHED_OCF    0x00

#define HCI_CRASH_MESSAGE_SIZE    (60)

typedef enum {
  VENDOR_CHIP_POWER_CONTROL = BT_VND_OP_POWER_CTRL,
  VENDOR_OPEN_USERIAL = BT_VND_OP_USERIAL_OPEN,
  VENDOR_CLOSE_USERIAL = BT_VND_OP_USERIAL_CLOSE,
  VENDOR_GET_LPM_IDLE_TIMEOUT = BT_VND_OP_GET_LPM_IDLE_TIMEOUT,
  VENDOR_SET_LPM_WAKE_STATE = BT_VND_OP_LPM_WAKE_SET_STATE,
  VENDOR_SET_AUDIO_STATE = BT_VND_OP_SET_AUDIO_STATE
} vendor_opcode_t;

typedef enum {
  VENDOR_CONFIGURE_FIRMWARE = BT_VND_OP_FW_CFG,
  VENDOR_CONFIGURE_SCO = BT_VND_OP_SCO_CFG,
  VENDOR_SET_LPM_MODE = BT_VND_OP_LPM_SET_MODE,
  VENDOR_DO_EPILOG = BT_VND_OP_EPILOG,
  VENDOR_A2DP_OFFLOAD_START = BT_VND_OP_A2DP_OFFLOAD_START,
  VENDOR_A2DP_OFFLOAD_STOP = BT_VND_OP_A2DP_OFFLOAD_STOP,
  VENDOR_LAST_OP
} vendor_async_opcode_t;

typedef void (*vendor_cb)(bool success);

typedef struct vendor_t {
  // Opens the vendor-specific library and sets the Bluetooth
  // address of the adapter to |local_bdaddr|. |hci_interface| is
  // used to send commands on behalf of the vendor library.
  bool (*open)(const uint8_t* local_bdaddr, const hci_t* hci_interface);

  // Closes the vendor-specific library and frees all associated resources.
  // Only |vendor_open| may be called after |vendor_close|.
  void (*close)(void);

  // Sends a vendor-specific command to the library.
  int (*send_command)(vendor_opcode_t opcode, void* param);

  // Sends an asynchronous vendor-specific command to the library.
  int (*send_async_command)(vendor_async_opcode_t opcode, void* param);

  // Registers a callback for an asynchronous vendor-specific command.
  void (*set_callback)(vendor_async_opcode_t opcode, vendor_cb callback);
} vendor_t;

const vendor_t* vendor_get_interface();

typedef enum {
  BT_CRASH_REASON_DEFAULT        =  0x00,

  // SoC Crash Reasons
  BT_CRASH_REASON_RX_NULL        =  0x01,
  BT_CRASH_REASON_TX_RX_INVALID_PKT = 0x40,
  BT_CRASH_REASON_TX_RX_INVALID_LEN = 0x41,
  BT_CRASH_REASON_TX_RX_INVALID_PKT_FATAL = 0xC0,
  BT_CRASH_REASON_TX_RX_INVALID_LEN_FATAL = 0xC1,
  BT_CRASH_REASON_UNKNOWN        =  0x81,
  BT_CRASH_REASON_SW_REQUESTED   =  0x82,
  BT_CRASH_REASON_STACK_OVERFLOW =  0x83,
  BT_CRASH_REASON_EXCEPTION      =  0x84,
  BT_CRASH_REASON_ASSERT         =  0x85,
  BT_CRASH_REASON_TRAP           =  0x86,
  BT_CRASH_REASON_OS_FATAL       =  0x87,
  BT_CRASH_REASON_HCI_RESET      =  0x88,
  BT_CRASH_REASON_PATCH_RESET    =  0x89,
  BT_CRASH_REASON_ABT            =  0x8A,
  BT_CRASH_REASON_RAMMASK        =  0x8B,
  BT_CRASH_REASON_PREBARK        =  0x8C,
  BT_CRASH_REASON_BUSERROR       =  0x8D,
  BT_CRASH_REASON_IO_FATAL       =  0x8E,
  BT_CRASH_REASON_SSR_CMD        =  0x8F,
  BT_CRASH_REASON_POWERON        =  0x90,
  BT_CRASH_REASON_WATCHDOG       =  0x91,

  // Transport Driver Crash Reasons
  BT_CRASH_REASON_UARTINIT_STUCK        =  0xB1,
  BT_CRASH_REASON_GETVER_SEND_STUCK     =  0xB2,
  BT_CRASH_REASON_GETVER_NO_RSP_RCVD    =  0xB3,
  BT_CRASH_REASON_SETBAUDRATE_CMD_STUCK =  0xB4,
  BT_CRASH_REASON_PATCH_DNLD_STUCK      =  0xB5,
  BT_CRASH_REASON_GETBOARDID_CMD_STUCK  =  0xB6,
  BT_CRASH_REASON_NVM_DNLD_STUCK        =  0xB7,
  BT_CRASH_REASON_HCI_RESET_STUCK       =  0xB8,
  BT_CRASH_REASON_GETBLDINFO_CMD_STUCK  =  0xB9,
  BT_CRASH_REASON_ADDONFEAT_CMD_STUCK   =  0xBA,
  BT_CRASH_REASON_ENHLOG_CMD_STUCK      =  0xBB,
  BT_CRASH_REASON_DIAGINIT_STUCK        =  0xBC,
  BT_CRASH_REASON_DIAGDEINIT_STUCK      =  0xBD,
} soc_crash_reason_e;

typedef struct {
  soc_crash_reason_e reason;
  char reasonstr[HCI_CRASH_MESSAGE_SIZE];
} secondary_reason;

static secondary_reason secondary_crash_reason [] = {
{ BT_CRASH_REASON_DEFAULT                  ,    "Default"},
{ BT_CRASH_REASON_RX_NULL                  ,    "Rx Null"},
{ BT_CRASH_REASON_UNKNOWN                  ,    "Unknown"},
{ BT_CRASH_REASON_TX_RX_INVALID_PKT        ,    "Tx/Rx invalid packet"},
{ BT_CRASH_REASON_TX_RX_INVALID_LEN        ,    "Tx/Rx invalid len"},
{ BT_CRASH_REASON_TX_RX_INVALID_PKT_FATAL  ,    "Tx/Rx invalid packet fatal error"},
{ BT_CRASH_REASON_TX_RX_INVALID_LEN_FATAL  ,    "Tx/Rx invalid lenght fatal error"},
{ BT_CRASH_REASON_SW_REQUESTED             ,    "SW Requested"},
{ BT_CRASH_REASON_STACK_OVERFLOW           ,    "Stack Overflow"},
{ BT_CRASH_REASON_EXCEPTION                ,    "Exception"},
{ BT_CRASH_REASON_ASSERT                   ,    "Assert"},
{ BT_CRASH_REASON_TRAP                     ,    "Trap"},
{ BT_CRASH_REASON_OS_FATAL                 ,    "OS Fatal"},
{ BT_CRASH_REASON_HCI_RESET                ,    "HCI Reset"},
{ BT_CRASH_REASON_PATCH_RESET              ,    "Patch Reset"},
{ BT_CRASH_REASON_ABT                      ,    "SoC Abort"},
{ BT_CRASH_REASON_RAMMASK                  ,    "RAM MASK"},
{ BT_CRASH_REASON_PREBARK                  ,    "PREBARK"},
{ BT_CRASH_REASON_BUSERROR                 ,    "Bus error"},
{ BT_CRASH_REASON_IO_FATAL                 ,    "IO fatal eror"},
{ BT_CRASH_REASON_SSR_CMD                  ,    "SSR CMD"},
{ BT_CRASH_REASON_POWERON                  ,    "Power ON"},
{ BT_CRASH_REASON_WATCHDOG                 ,    "Watchdog"},
{ BT_CRASH_REASON_UARTINIT_STUCK           ,    "UartInitStuck"},
{ BT_CRASH_REASON_GETVER_SEND_STUCK        ,    "GetVerSendStuck"},
{ BT_CRASH_REASON_GETVER_NO_RSP_RCVD       ,    "GetVerNoRspRcvd"},
{ BT_CRASH_REASON_SETBAUDRATE_CMD_STUCK    ,    "SetBaudRateStuck"},
{ BT_CRASH_REASON_PATCH_DNLD_STUCK         ,    "PatchDnldStuck"},
{ BT_CRASH_REASON_GETBOARDID_CMD_STUCK     ,    "GetBoardIdStuck"},
{ BT_CRASH_REASON_NVM_DNLD_STUCK           ,    "NvmDnldStuck"},
{ BT_CRASH_REASON_HCI_RESET_STUCK          ,    "HciResetStuck"},
{ BT_CRASH_REASON_GETBLDINFO_CMD_STUCK     ,    "GetBldInfoCmdStuck"},
{ BT_CRASH_REASON_ADDONFEAT_CMD_STUCK      ,    "AddOnFeatCmdStuck"},
{ BT_CRASH_REASON_ENHLOG_CMD_STUCK         ,    "EnhLogCmdStuck"},
{ BT_CRASH_REASON_DIAGINIT_STUCK           ,    "DiagInitStuck"},
{ BT_CRASH_REASON_DIAGDEINIT_STUCK         ,    "DiagDeinitStuck"},
};

enum host_crash_reason_e  {
  REASON_DEFAULT_NONE  = 0x00,                         //INVALID REASON
  REASON_SOC_CRASHED = 0x01,                           //SOC WAS CRASHED
  REASON_SOC_CRASHED_DIAG_SSR = 0x02,                  //SOC CRASHED DIAG INITIATED SSR
  REASON_PATCH_DLDNG_FAILED_SOCINIT = 0x03,            //CONTROLLED INIT FAILED
  REASON_CLOSE_RCVD_DURING_INIT = 0x04,                //CLOSE RECEIVED FROM STACK DURING SOC INIT
  REASON_ERROR_READING_DATA_FROM_UART = 0x05,          //ERROR READING DATA FROM UART
  REASON_WRITE_FAIL_SPCL_BUFF_CRASH_SOC = 0x06,        //FAILED TO WRITE SPECIAL BYTES TO CRASH SOC
  REASON_RX_THREAD_STUCK = 0x07,                       //RX THREAD STUCK
  REASON_SSR_CMD_TIMEDOUT = 0x10,                      //SSR DUE TO CMD TIMED OUT
  REASON_SSR_SPURIOUS_WAKEUP = 0x11,                   //SSR DUE TO SPURIOUS WAKE UP
  REASON_SSR_INVALID_BYTES_RCVD = 0x12,                //INVALID HCI CMD TYPE RECEIVED
  REASON_SSR_RCVD_LARGE_PKT_FROM_SOC = 0x13,           //SSR DUE TO LARGE PKT RECVIVED FROM SOC
  REASON_SSR_UNABLE_TO_WAKEUP_SOC = 0x14,              //UNABLE TO WAKE UP SOC
  REASON_CMD_TIMEDOUT_SOC_WAIT_TIMEOUT = 0x20,         //COMMAND TIMEOUT AND SOC CRASH WAIT TIMEOUT
  REASON_SPURIOUS_WAKEUP_SOC_WAIT_TIMEOUT = 0x21,      //SPURIOUS WAKE AND SOC CRASH WAIT TIMEOUT
  REASON_INV_BYTES_SOC_WAIT_TIMEOUT = 0x22,            //INVALID BYTES AND SOC CRASH WAIT TIMEOUT
  REASON_SOC_WAKEUP_FAILED_SOC_WAIT_TIMEOUT = 0x23,    //SOC WAKEUP FAILURE AND SOC CRASH WAIT TIMEOUT
  REASON_SOC_CRASHED_DIAG_SSR_SOC_WAIT_TIMEOUT = 0x24, //SOC CRASHED DIAG INITIATED SSR CRASH WAIT TIMEOUT
  REASON_NONE_SOC_WAIT_TIMEOUT = 0x25,                 //INVALID FAILURE AND SOC CRASH WAIT TIMEOUT
  REASON_SOC_DEINIT_STUCK = 0x26,                      //SOC DEINIT STUCK
};

typedef struct {
  host_crash_reason_e reason;
  char reasonstr[HCI_CRASH_MESSAGE_SIZE];
} primary_reason;

static primary_reason primary_crash_reason [] = {
{ REASON_DEFAULT_NONE                         , "Invalid reason"},
{ REASON_SOC_CRASHED                          , "SOC crashed"},
{ REASON_SOC_CRASHED_DIAG_SSR                 , "SOC crashed with diag initiated SSR"},
{ REASON_PATCH_DLDNG_FAILED_SOCINIT           , "SOC init failed during patch downloading"},
{ REASON_CLOSE_RCVD_DURING_INIT               , "Close received from stack during SOC init"},
{ REASON_ERROR_READING_DATA_FROM_UART         , "Error reading data from UART"},
{ REASON_WRITE_FAIL_SPCL_BUFF_CRASH_SOC       , "Failed to write special bytes to crash SOC"},
{ REASON_RX_THREAD_STUCK                      , "Rx Thread Stuck"},
{ REASON_SSR_CMD_TIMEDOUT                     , "SSR due to command timed out"},
{ REASON_SSR_SPURIOUS_WAKEUP                  , "SSR due to spurious wakeup"},
{ REASON_SSR_INVALID_BYTES_RCVD               , "Invalid HCI cmd type received"},
{ REASON_SSR_RCVD_LARGE_PKT_FROM_SOC          , "Large packet received from SOC"},
{ REASON_SSR_UNABLE_TO_WAKEUP_SOC             , "Unable to wake SOC"},
{ REASON_CMD_TIMEDOUT_SOC_WAIT_TIMEOUT        , "Command timedout and SOC crash wait timeout"},
{ REASON_SPURIOUS_WAKEUP_SOC_WAIT_TIMEOUT     , "Spurious wake and SOC crash wait timeout"},
{ REASON_INV_BYTES_SOC_WAIT_TIMEOUT           , "Invalid bytes received and SOC crash wait timeout"},
{ REASON_SOC_WAKEUP_FAILED_SOC_WAIT_TIMEOUT   , "SOC Wakeup failed and SOC crash wait timeout"},
{ REASON_NONE_SOC_WAIT_TIMEOUT                , "Invalid Reason and SOC crash wait timeout"},
{ REASON_SOC_CRASHED_DIAG_SSR_SOC_WAIT_TIMEOUT, "SOC crashed with diag initiated SSR and SOC wait timeout"},
{ REASON_NONE_SOC_WAIT_TIMEOUT                , "Invalid Reason and SOC crash wait timeout"},
{ REASON_SOC_DEINIT_STUCK                     , "SOC DeInit Stuck"},
};

void decode_crash_reason(uint8_t* p, uint8_t evt_len);
char *get_secondary_reason_str(soc_crash_reason_e reason);
char *get_primary_reason_str(host_crash_reason_e reason);
