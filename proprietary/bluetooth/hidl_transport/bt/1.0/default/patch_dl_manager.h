/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 *  Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *  Not a Contribution.
 *
 *  Copyright 2012 The Android Open Source Project
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you
 *  may not use this file except in compliance with the License. You may
 *  obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 *  implied. See the License for the specific language governing
 *  permissions and limitations under the License.
 *
 */

#pragma once

#include "data_handler.h"
#include <hidl/HidlSupport.h>
#include <hci_uart_transport.h>
#include <logger.h>
#include "power_manager.h"
/******************************************************************************
**  Constants & Macros
******************************************************************************/
#define HCI_MAX_CMD_SIZE                     260
#define HCI_MAX_EVENT_SIZE                   260
#define PRINT_BUF_SIZE                       ((HCI_MAX_CMD_SIZE * 3) + 2)

#define HCI_CHG_BAUD_CMD_OCF                 0x0C
#define HCI_VENDOR_CMD_OGF                   0x3F
#define WRITE_BDADDR_CMD_LEN                 14
#define WRITE_BAUD_CMD_LEN                   6
#define MAX_CMD_LEN                          WRITE_BDADDR_CMD_LEN
#define GET_VERSION_OCF                      0x1E

#define PS_HDR_LEN                           4
#define HCI_VENDOR_CMD_OGF                   0x3F
#define HCI_PS_CMD_OCF                       0x0B

#define HCI_COMMAND_HDR_SIZE                 3
#define EVT_CMD_COMPLETE_SIZE                3
#define EVT_CMD_STATUS                       0x0F
#define EVT_CMD_STATUS_SIZE                  4
#define HCI_EVENT_HDR_SIZE                   2
#define HCI_EV_SUCCESS                       0x00
/* HCI Socket options */
#define HCI_DATA_DIR                         1
#define HCI_FILTER                           2
#define HCI_TIME_STAMP                       3

#define P_ID_OFFSET                          (0)
#define HCI_CMD_IND                          (1)
#define EVENTCODE_OFFSET                     (1)
#define EVT_PLEN                             (2)
#define PLEN                                 (3)
#define CMD_RSP_OFFSET                       (3)
#define RSP_TYPE_OFFSET                      (4)
#define BAUDRATE_RSP_STATUS_OFFSET           (4)
#define CMD_STATUS_OFFSET                    (5)
#define P_CHIP_VER_OFFSET                    (4)
#define P_BUILD_VER_OFFSET                   (6)
#define P_BASE_ADDR_OFFSET                   (8)
#define P_ENTRY_ADDR_OFFSET                  (12)
#define P_LEN_OFFSET                         (16)
#define P_CRC_OFFSET                         (20)
#define P_CONTROL_OFFSET                     (24)
#define PATCH_HDR_LEN                        (28)
#define MAX_DATA_PER_SEGMENT                 (239)
#define VSEVENT_CODE                         (0xFF)
#define HC_VS_MAX_CMD_EVENT                  (0xFF)
#define PATCH_PROD_ID_OFFSET                 (5)
#define PATCH_PATCH_VER_OFFSET               (9)
#define PATCH_ROM_BUILD_VER_OFFSET           (11)
#define PATCH_SOC_VER_OFFSET                 (13)
#define MAX_SIZE_PER_TLV_SEGMENT             (243)
/* Unified VSC offset*/
#define PATCH_PROD_ID_OFFSET_UNIFIED         (9)
#define PATCH_PATCH_VER_OFFSET_UNIFIED       (13)
#define PATCH_ROM_BUILD_VER_OFFSET_UNIFIED   (15)
#define PATCH_SOC_VER_OFFSET_UNIFIED         (17)
#define BAUDRATE_RSP_STATUS_OFFSET_UNIFIED   (6)
#define CMD_STATUS_OFFSET_UNIFIED            (6)
#define UNIFIED_HCI_CODE                     (0x01)
#define UNIFIED_HCI_CC_MIN_LENGTH            (6)
#define HCI_VS_WIPOWER_CMD_OPCODE            (0xFC1F)
#define HCI_VS_GET_VER_CMD_OPCODE            (0xFC00)
/* VS Opcode */
#define HCI_PATCH_CMD_OCF                    (0)
#define EDL_SET_BAUDRATE_CMD_OCF             (0x48)
#define EDL_WIPOWER_VS_CMD_OCF               (0x1f)
#define HCI_VS_GET_ADDON_FEATURES_SUPPORT    (0x1d)
#define EDL_GET_BOARD_ID                     (0x23)
#define HCI_SPLIT_A2DP_OPCODE                (0x0A)

/* VS Commands */
#define VSC_SET_BAUDRATE_REQ_LEN             (1)
#define EDL_PATCH_CMD_LEN                    (1)
#define EDL_PATCH_CMD_REQ_LEN                (1)
#define EDL_WIP_QUERY_CHARGING_STATUS_LEN    (0x01)
#define EDL_WIP_START_HANDOFF_TO_HOST_LEN    (0x01)
#define EDL_PATCH_DLD_REQ_CMD                (0x01)
#define EDL_PATCH_RST_REQ_CMD                (0x05)
#define EDL_PATCH_SET_REQ_CMD                (0x16)
#define EDL_PATCH_ATCH_REQ_CMD               (0x17)
#define EDL_PATCH_VER_REQ_CMD                (0x19)
#define EDL_GET_BUILD_INFO                   (0x20)
#define EDL_PATCH_TLV_REQ_CMD                (0x1E)
#define EDL_WIP_QUERY_CHARGING_STATUS_CMD    (0x1D)
#define EDL_WIP_START_HANDOFF_TO_HOST_CMD    (0x1E)
#define EDL_WIP_START_HANDOFF_TO_FW_CMD      (0x25)
#define HCI_VSC_GET_SCRAMBLING_FREQS         (0x11)

/* VS Event */
#define EDL_CMD_REQ_RES_EVT                  (0x00)
#define EDL_CMD_EXE_STATUS_EVT               (0x00)
#define EDL_SET_BAUDRATE_RSP_EVT             (0x92)
#define EDL_PATCH_VER_RES_EVT                (0x19)
#define EDL_TVL_DNLD_RES_EVT                 (0x04)
#define EDL_APP_VER_RES_EVT                  (0x02)
#define EDL_WIP_QUERY_CHARGING_STATUS_EVT    (0x18)
#define EDL_WIP_START_HANDOFF_TO_HOST_EVENT  (0x19)
#define HCI_VS_GET_ADDON_FEATURES_EVENT      (0x1B)
#define EDL_BOARD_ID_RESPONSE                (0x23)
#define HCI_VS_GET_BUILD_VER_EVT             (0x05)
#define HCI_VS_STRAY_EVT                     (0x17)
#define HCI_VS_HOST_LOG_ENH_ENABLE_SUBID     (0x14)
#define HCI_VS_HOST_LOG_CTRL_SUBID           (0x00)

/* Status Codes of HCI CMD execution*/
#define HCI_CMD_SUCCESS                      (0x0)
#define PATCH_LEN_ERROR                      (0x1)
#define PATCH_VER_ERROR                      (0x2)
#define PATCH_CRC_ERROR                      (0x3)
#define PATCH_NOT_FOUND                      (0x4)
#define TLV_TYPE_ERROR                       (0x10)
#define NVM_ACCESS_CODE                      (0x0B)
#define BAUDRATE_CHANGE_SUCCESS              (1)
#define LOG_ENH_ENABLE_SUCCESS               (0x00)

/* Wipower status codes */
#define WIPOWER_IN_EMBEDDED_MODE             0x01
#define NON_WIPOWER_MODE                     0x02

/* masks to validate support for wipower */
#define ADDON_FEATURES_EVT_WIPOWER_MASK       (0x01)
#define ADDON_FEATURES_BITMASK_OFFSET_UNIFIED (11)
#define ADDON_FEATURES_BITMASK_OFFSET         (4)


/* TLV_TYPE */
#define TLV_TYPE_PATCH                       (1)
#define TLV_TYPE_BT_NVM                      (2)
#define TLV_TYPE_FM_NVM                      (3)
#define TLV_TYPE_BT_FM_NVM                   (4)

/* NVM */
#define MAX_TAG_CMD                          30
#define TAG_END                              0xFF
#define NVM_ACCESS_SET                       0x01
#define TAG_NUM_OFFSET                       5
#define TAG_NUM_2                            2
#define TAG_BDADDR_OFFSET                    7

#define TAG_NUM_17                           0x11  /* 17  : HCI UART Settings */

#define LSH(val, n)     ((uint32_t)(val) << (n))
#define EXTRACT_BYTE(val, pos)      (char)(((val) >> (8 * (pos))) & 0xFF)
#define CALC_SEG_SIZE(len, max)   ((plen) % (max)) ? ((plen / max) + 1) : ((plen) / (max))
#define PTR_TO_UINT16(u16, p)  ((u16) = ((uint16_t)(*(p)) + (((uint16_t)(*((p) + 1))) << 8)))

/*BT FW path */
#define BT_FW_REGULAR_PATH                      "/bt_firmware/image/"
#define BT_FW_QTI_ALT_PATH                      "/vendor/bt_firmware/image/"
#define BT_FW_ALT_PATH                          "/vendor/firmware/"
#define TEMP_RAMPATCH_TLV_0_0_0_PATH            "/bt_firmware/image/tpbtfw00.tlv"
#define TEMP_NVM_TLV_0_0_0_PATH                 "/bt_firmware/image/tpnv00.bin"
#define TEMP_RAMPATCH_TLV_0_0_0_PATH_ALT        "/vendor/firmware/tpbtfw00.tlv"
#define TEMP_NVM_TLV_0_0_0_PATH_ALT             "/vendor/firmware/tpnv00.bin"
#define TEMP_RAMPATCH_TLV_0_0_0_PATH_ALT_QTI    "/vendor/bt_firmware/image/tpbtfw00.tlv"
#define TEMP_NVM_TLV_0_0_0_PATH_ALT_QTI         "/vendor/bt_firmware/image/tpnv00.bin"
/* This header value in rampatch file decides event handling mechanism in the HOST */
#define SKIP_EVT_NONE     0x00
#define SKIP_EVT_VSE      0x01
#define SKIP_EVT_CC       0x02
#define SKIP_EVT_VSE_CC   0x03

/* Rome 3.1 FW SU release has been branched from rome 3.0 SU 224
   So, rome 3.1 formula is SU = patch version -(0xE0 = 224) - 0x111 -1
 */
#define ROME_3_1_FW_SW_OFFSET   0x01F2

/* Rome 3.2 FW SU formula is SU = patch version - 0x111 -1 */
#define ROME_3_2_FW_SW_OFFSET   0x0112

/* This header value in rampatch file decides event handling mechanism in the HOST */
#define ROME_SKIP_EVT_NONE     0x00
#define ROME_SKIP_EVT_VSE      0x01
#define ROME_SKIP_EVT_CC       0x02
#define ROME_SKIP_EVT_VSE_CC   0x03

#define AUDIO_SPLIT_A2DP_SCRAMBLING_DATA_SUPPORT(x) ((x)[0] & 0x02)
#define SPLIT_A2DP_44P1KHZ_FREQ(x) ((x)[0] & 0x04)
#define SPLIT_A2DP_48KHZ_FREQ(x) ((x)[0] & 0x08)

#define PARM_LEN_BYTE      2
#define STATUS_BYTE        6
#define PRODUCT_ID_BYTE    7
#define RSP_VER_BYTE       9
#define HCI_EVENT_PARAMS   8
#define FEAT_MASK_BYTES    11

#define BOARD_ID_LEN 0x5

#define FILE_PATH_LEN (64)

/* chipset id(64 bit)= (((Soc ID) << 32) | ((Product ID &0xffff) <<16)
                       | (Build version)& 0xffff)
   Create a 64bit unique chipset ver by storing 2 bytes of build version in LSB
   , next to it store 2 bytes of product id and 4 bytes SOC ID in MSB then ORing
   them for uniqueness */

#define QCA_BT_VER(s, p, b) (((uint64_t)(s) << 32) | ((uint64_t)(p & 0xffff) << 16) \
                            | ((uint64_t)(b & 0xffff)))
namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

/******************************************************************************
**  Local type definitions
******************************************************************************/
typedef struct {
  unsigned short rom_version;
  unsigned short build_version;
} __attribute__ ((packed)) patch_version;

typedef struct {
  uint16_t opcode;
  uint8_t plen;
} __attribute__ ((packed))  hci_command_hdr;

typedef struct {
  unsigned int patch_id;
  patch_version patch_ver;
  unsigned int patch_base_addr;
  unsigned int patch_entry_addr;
  unsigned short patch_length;
  int patch_crc;
  unsigned short patch_ctrl;
} __attribute__ ((packed)) patch_info;

typedef struct {
  unsigned int tlv_data_len;
  unsigned int tlv_patch_data_len;
  unsigned char sign_ver;
  unsigned char sign_algorithm;
  unsigned char dwnd_cfg;
  unsigned char reserved1;
  unsigned short prod_id;
  unsigned short build_ver;
  unsigned short patch_ver;
  unsigned short reserved2;
  unsigned int patch_entry_addr;
} __attribute__ ((packed)) tlv_patch_hdr;

typedef struct {
  unsigned short tag_id;
  unsigned short tag_len;
  unsigned int tag_ptr;
  unsigned int tag_ex_flag;
} __attribute__ ((packed)) tlv_nvm_hdr;

typedef struct {
  unsigned char tlv_type;
  unsigned char tlv_length1;
  unsigned char tlv_length2;
  unsigned char tlv_length3;

  union {
    tlv_patch_hdr patch;
    tlv_nvm_hdr nvm;
  }tlv;
} __attribute__ ((packed)) tlv_patch_info;


enum {
  PROD_ID_ROME = 0x08,
  PROD_ID_CHEROKEE = 0x0A,
  PROD_ID_COMANCHE = 0x0F,
  PROD_ID_HASTINGS = 0x10,
  PROD_ID_GENOA = 0x12,
  PROD_ID_HSP = 0x13,
  PROD_ID_APACHE = PROD_ID_CHEROKEE
};

enum {
  ROME_PATCH_VER_0200 = 0x0200,
  ROME_PATCH_VER_0300 = 0x0300,
  ROME_PATCH_VER_0302 = 0x0302
};

enum {
  ROME_SOC_ID_11 = 0x00000011,
  ROME_SOC_ID_22 = 0x00000022,
  ROME_SOC_ID_44 = 0x00000044
};

enum {
  ROME_VER_2_1 = QCA_BT_VER(ROME_SOC_ID_11, PROD_ID_ROME, ROME_PATCH_VER_0200),
  ROME_VER_3_0 = QCA_BT_VER(ROME_SOC_ID_22, PROD_ID_ROME, ROME_PATCH_VER_0300),
  ROME_VER_3_2 = QCA_BT_VER(ROME_SOC_ID_44, PROD_ID_ROME, ROME_PATCH_VER_0302),
};

enum {
  CHEROKEE_BUILD_VER_0200   = 0x0200,
  CHEROKEE_BUILD_VER_0201   = 0x0201,
  CHEROKEE_BUILD_VER_0211   = 0x1001,
  CHEROKEE_BUILD_VER_0310   = 0x0300,
  CHEROKEE_BUILD_VER_0302   = 0x0302,
};

enum {
  QCA_CHEROKEE_SOC_ID_0200  = 0x40010200,
  QCA_CHEROKEE_SOC_ID_0201  = 0x40010201,
  QCA_CHEROKEE_SOC_ID_0210  = 0x40010214,
  QCA_CHEROKEE_SOC_ID_0211  = 0x40010224,
  QCA_CHEROKEE_SOC_ID_0310  = 0x40010310,
  QCA_CHEROKEE_SOC_ID_0320  = 0x40010320,
  QCA_CHEROKEE_SOC_ID_0320_UMC  = 0x40014320,
};

enum {
  CHEROKEE_VER_2_0 = QCA_BT_VER(QCA_CHEROKEE_SOC_ID_0200, PROD_ID_CHEROKEE, CHEROKEE_BUILD_VER_0200),
  CHEROKEE_VER_2_0_1 = QCA_BT_VER(QCA_CHEROKEE_SOC_ID_0201, PROD_ID_CHEROKEE, CHEROKEE_BUILD_VER_0200),
  CHEROKEE_VER_2_1 = QCA_BT_VER(QCA_CHEROKEE_SOC_ID_0210, PROD_ID_CHEROKEE, CHEROKEE_BUILD_VER_0201),
  CHEROKEE_VER_2_1_1 = QCA_BT_VER(QCA_CHEROKEE_SOC_ID_0211, PROD_ID_CHEROKEE, CHEROKEE_BUILD_VER_0211),
  CHEROKEE_VER_3_1 = QCA_BT_VER(QCA_CHEROKEE_SOC_ID_0310, PROD_ID_CHEROKEE, CHEROKEE_BUILD_VER_0310),
  CHEROKEE_VER_3_2 = QCA_BT_VER(QCA_CHEROKEE_SOC_ID_0320, PROD_ID_CHEROKEE, CHEROKEE_BUILD_VER_0302),
  CHEROKEE_VER_3_2_UMC = QCA_BT_VER(QCA_CHEROKEE_SOC_ID_0320_UMC, PROD_ID_CHEROKEE,
                                    CHEROKEE_BUILD_VER_0302),
};

enum{
    APACHE_BUILD_VER_0100   = 0x0200,
    APACHE_BUILD_VER_0110   = 0x0201,
    APACHE_BUILD_VER_0120   = 0x0201
};

enum{
    QCA_APACHE_SOC_ID_0100  = 0x40020120,
    QCA_APACHE_SOC_ID_0110  = 0x40020130,
    QCA_APACHE_SOC_ID_0120  = 0x40020140,
    QCA_APACHE_SOC_ID_0121  = 0x40020150
};

enum {
  APACHE_VER_1_0_0 = QCA_BT_VER(QCA_APACHE_SOC_ID_0100, PROD_ID_APACHE, APACHE_BUILD_VER_0100),
  APACHE_VER_1_1_0 = QCA_BT_VER(QCA_APACHE_SOC_ID_0110, PROD_ID_APACHE, APACHE_BUILD_VER_0110),
  APACHE_VER_1_2_0 = QCA_BT_VER(QCA_APACHE_SOC_ID_0120, PROD_ID_APACHE, APACHE_BUILD_VER_0120),
  APACHE_VER_1_2_1 = QCA_BT_VER(QCA_APACHE_SOC_ID_0121, PROD_ID_APACHE, APACHE_BUILD_VER_0120),
};

enum {
    COMANCHE_BUILD_VER_0100   = 0X0100,
    COMANCHE_BUILD_VER_0101   = 0X0101,
    COMANCHE_BUILD_VER_0120   = 0X0102,
};

enum {
    QCA_COMANCHE_SOC_ID_0101  = 0x40070101,
    QCA_COMANCHE_SOC_ID_0110  = 0x40070110,
    QCA_COMANCHE_SOC_ID_0120  = 0x40070120,
};

enum {
    COMANCHE_VER_1_0_1 = QCA_BT_VER(QCA_COMANCHE_SOC_ID_0101, PROD_ID_COMANCHE, COMANCHE_BUILD_VER_0100),
    COMANCHE_VER_1_1   = QCA_BT_VER(QCA_COMANCHE_SOC_ID_0110, PROD_ID_COMANCHE, COMANCHE_BUILD_VER_0101),
    COMANCHE_VER_1_2   = QCA_BT_VER(QCA_COMANCHE_SOC_ID_0120, PROD_ID_COMANCHE, COMANCHE_BUILD_VER_0120),
};

#define IS_NEWER_THAN_CHEROKEE_2_1(v)  (((uint16_t) (((v) & 0x00000000FFFF0000) >> 16) \
                                                > PROD_ID_CHEROKEE) || \
                                                 (((uint16_t) (((v) & 0x00000000FFFF0000) >> 16) \
                                                == PROD_ID_CHEROKEE) \
                                                && ((uint16_t) ((v) & 0x000000000000FFFF) \
                                                >= CHEROKEE_BUILD_VER_0310) && \
                                                ((uint32_t) (((v) & 0xFFFFFFFF00000000) >> 32) \
                                                >= QCA_CHEROKEE_SOC_ID_0310)))
enum {
  HASTINGS_BUILD_VER_0200 = 0x0200,
};

enum {
    QCA_HASTINGS_SOC_ID_0200 = 0x400A0200,
};

enum {
  HASTINGS_VER_2_0   = QCA_BT_VER(QCA_HASTINGS_SOC_ID_0200, PROD_ID_HASTINGS, HASTINGS_BUILD_VER_0200),
};

enum {
  HSP_BUILD_VER_0100 = 0x0100,
  HSP_BUILD_VER_0200 = 0x0200,
};

enum {
  QCA_HSP_SOC_ID_0100 = 0x400C0100,
  QCA_HSP_SOC_ID_0200 = 0x400C0200,
};

enum {
  HSP_VER_1_0   = QCA_BT_VER(QCA_HSP_SOC_ID_0100, PROD_ID_HSP, HSP_BUILD_VER_0100),
  HSP_VER_2_0   = QCA_BT_VER(QCA_HSP_SOC_ID_0200, PROD_ID_HSP, HSP_BUILD_VER_0200),
};

enum {
  GENOA_BUILD_VER_0100 = 0x0100,
  GENOA_BUILD_VER_0200 = 0x0200,
};

enum {
  QCA_GENOA_SOC_ID_0100 = 0x400B0100,
  QCA_GENOA_SOC_ID_0200 = 0x400B0200,
};

enum {
  GENOA_VER_1_0     = QCA_BT_VER(QCA_GENOA_SOC_ID_0100, PROD_ID_GENOA, GENOA_BUILD_VER_0100),
  GENOA_VER_2_0     = QCA_BT_VER(QCA_GENOA_SOC_ID_0200, PROD_ID_GENOA, GENOA_BUILD_VER_0100),
  GENOA_VER_2_0_0_2 = QCA_BT_VER(QCA_GENOA_SOC_ID_0200, PROD_ID_GENOA, GENOA_BUILD_VER_0200),
};

class PatchPathManager {
 private:
  std::string chip_name_;
  std::string tlv_name_;
  std::string nvm_name_;
 public:
  PatchPathManager() {}

  PatchPathManager(std::string chip_name, std::string tlv_name, std::string nvm_name) :
  chip_name_(chip_name), tlv_name_(tlv_name), nvm_name_(nvm_name) {
  }

  const std::string& GetChipName() const {
    return chip_name_;
  }

  const std::string& GetTLVName() const {
    return tlv_name_;
  }

  const std::string& GetNVMName() const {
    return nvm_name_;
  }

};

class PatchDLManager {
 public:
  PatchDLManager(BluetoothSocType soc_type, HciUartTransport* transport, PowerManager* power_manager);
  int PerformChipInit();
  int SetBaudRateReq();
  unsigned char IsSibsEnabled();
  features_list* GetAddOnFeatureList();
  ~PatchDLManager();

 protected:
  bool GetBdaddrFromProperty(const char *property, uint8_t res_bd_addr[6], bool is_reverse);
  int SocInit(uint8_t *bdaddr, bool is_emb_wp_mode);
  int DownloadTlvFile(const char *rampatch_file_path, const char *nvm_file_path, const char *rampatch_alt_file_path, const char *nvm_alt_file_path);
  int HciReset(void);
  int WipowerCurrentChargingStatusReq();
  int WipowerForwardHandoffReq();
  int HciSendWipowerVsCmd(unsigned char *cmd, unsigned char *rsp, int size);
  int PatchVerReq();
  void FrameHciCmdPkt(unsigned char *cmd, int edl_cmd, unsigned int p_base_addr,
                         int segtNo, int size);
  int CheckEmbeddedMode();
  int GetBuildInfoReq();
  bool IsExtldoEnabled();
  int HciSendVsCmd(unsigned char *cmd, unsigned char *rsp, int size);
  int ReadVsHciEvent(unsigned char* buf, int size);
  int ReadHciEvent(unsigned char* buf, int size);
  int ReadCmdCmplEvent(unsigned char* buf, int size);
  int GetVsHciEvent(unsigned char *rsp);
  int TlvDnldReq(int tlv_size);
  void FrameHciPkt(unsigned char *cmd, int edl_cmd, unsigned int p_base_addr,
                     int segtNo, int size);
  int GetBoardIdReq(void);
  bool ValidateToken(char *bdaddr_tok);
  int GetTlvFile(const char *file_path, const char *alt_file_path);
  int TlvDnldSegment(int index, int seg_size, unsigned char wait_cc_evt);
  int DisableInternalLdo();
  void EnableControllerLog();
  void EnableEnhControllerLog();
  int ReadNewHciEvent(unsigned char* buf, int size);
  unsigned char Convert2Ascii(unsigned char temp);
  void UpdateNewNvmFormat(tlv_nvm_hdr *nvm);
  int GetUnifiedHciEvent(unsigned char* rsp);
  void LoadPatchMaptable();
  void FormRegularPaths(PatchPathManager info);
  void FormDefaultPaths();
  void DownloadAndCmdExeStatus(int err);
  void HandleEdlCmdResEvt(unsigned char subOpcode, unsigned char paramlen, unsigned char* rsp);
  FILE* OpenPatchFile(const char *file_path, const char* alt_file_path);
#ifdef DEBUG_CRASH_SOC_WHILE_DLD
  int SendCrashCommand();
#endif

 private:
  BluetoothSocType soc_type_;
  HciUartTransport* uart_transport_;
  PowerManager* power_manager_;
  int fd_transport_;
  int tlv_type_;
  uint64_t chipset_ver_;
  Logger *bt_logger_;
  unsigned int wipower_flag_;
  unsigned int wipower_handoff_ready_;
  int dnld_fd_in_progress_;
  unsigned char *pdata_buffer_;
  unsigned char *phdr_buffer_;
  unsigned char tlv_dwn_cfg_;
  unsigned char wait_vsc_evt_;
  patch_info rampatch_patch_info_;
  bool patch_dnld_pending_;
  unsigned char board_id_[BOARD_ID_LEN];
  uint8_t vnd_local_bd_addr_[6];
  const char *fw_su_info_ = NULL;
  unsigned short fw_su_offset_ = 0;
  bool unified_hci;
  features_list add_on_features;
  std::map<uint64_t, PatchPathManager> PatchPathInfoMap_;
  char rampatch_file_path[FILE_PATH_LEN] = {'\0'};
  char nvm_file_path[FILE_PATH_LEN] = {'\0'};
  char rampatch_alt_file_path[FILE_PATH_LEN] = {'\0'};
  char nvm_alt_file_path[FILE_PATH_LEN] = {'\0'};
  char rampatch_alt_path_qti_check[FILE_PATH_LEN] = {'\0'};
  char rampatch_alt_path_check[FILE_PATH_LEN] = {'\0'};
  char nvm_alt_path_qti_check[FILE_PATH_LEN] = {'\0'};
  char nvm_alt_path_check[FILE_PATH_LEN] = {'\0'};
};

} // namespace implementation
} // namespace V1_0
} // namespace bluetooth
} // namespace hardware
} // namespace android
