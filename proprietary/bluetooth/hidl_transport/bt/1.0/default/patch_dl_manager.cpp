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

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <fcntl.h>
#include <string.h>
#include <asm-generic/ioctls.h>
#include <hidl/HidlSupport.h>
#include <patch_dl_manager.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>
#include <termios.h>
#include "logger.h"
#include "ibs_handler.h"
#include <sys/stat.h>

#include "bluetooth_address.h"
#define UNUSED(x) (void)(x)
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "vendor.qti.bluetooth@1.0-patch_dl_manager"

#define BT_VERSION_FILEPATH "/data/vendor/bluetooth/bt_fw_version.txt"
#define MSB_NIBBLE_MASK 0xF0
#define LSB_NIBBLE_MASK 0x0F


namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

PatchDLManager::PatchDLManager(BluetoothSocType soc_type, HciUartTransport* transport, PowerManager* power_manager) :
  soc_type_(soc_type), uart_transport_(transport), power_manager_(power_manager), dnld_fd_in_progress_(-1)
{
  ALOGD("%s", __func__);
  bt_logger_ = Logger::Get();
  chipset_ver_ = 0;
  fd_transport_ = uart_transport_->GetCtrlFd(),
  wait_vsc_evt_ = true;
  patch_dnld_pending_ = false;
  unified_hci = false;
  memset(&add_on_features, 0, sizeof(add_on_features));
  LoadPatchMaptable();
}

PatchDLManager::~PatchDLManager()
{
  patch_dnld_pending_ = false;
  unified_hci = false;
  PatchPathInfoMap_.clear();
}

bool PatchDLManager::GetBdaddrFromProperty(const char *property, uint8_t res_bd_addr[6], bool is_reverse)
{
  bool is_valid = false;
  int i = 0;
  static char bd_addr[PROPERTY_VALUE_MAX];
  char* tok = NULL;
  char *tmp_token = NULL;

  if (property_get(property, bd_addr, NULL)) {
    ALOGV("BD address read from Boot property: %s\n", bd_addr);
    tok = strtok_r(bd_addr, ":", &tmp_token);
    while (tok != NULL) {
      ALOGV("bd add [%d]: %ld ", i, strtol(tok, NULL, 16));
      if (i >= 6) {
        ALOGE("bd property of invalid length");
        is_valid = false;
        break;
      }
      if (!ValidateToken(tok)) {
        ALOGE("Invalid token in BD address");
        is_valid = false;
        break;
      }
      if (is_reverse) {
        res_bd_addr[5 - i] = strtol(tok, NULL, 16);
      } else {
        res_bd_addr[i] = strtol(tok, NULL, 16);
      }
      tok = strtok_r(NULL, ":", &tmp_token);
      i++;
    }
  }
  if (i == 6)
    is_valid = true;

  return is_valid;
}

bool PatchDLManager::ValidateToken(char* bdaddr_tok)
{
  int i = 0;
  bool ret;

  if (strlen(bdaddr_tok) != 2) {
    ret = false;
    ALOGE("Invalid token length");
  } else {
    ret = true;
    for (i = 0; i < 2; i++) {
      if ((bdaddr_tok[i] >= '0' && bdaddr_tok[i] <= '9') ||
          (bdaddr_tok[i] >= 'A' && bdaddr_tok[i] <= 'F') ||
          (bdaddr_tok[i] >= 'a' && bdaddr_tok[i] <= 'f')) {
        ret = true;
        ALOGV("%s: tok %s @ %d is good", __func__, bdaddr_tok, i);
      } else {
        ret = false;
        ALOGE("invalid character in tok: %s at ind: %d", bdaddr_tok, i);
        break;
      }
    }
  }
  return ret;
}

int PatchDLManager::CheckEmbeddedMode()
{
  int err = 0;

  wipower_flag_ = 0;
  /* Get current wipower charging status */
  if ((err = WipowerCurrentChargingStatusReq()) < 0) {
    ALOGI("%s: Wipower status req failed (0x%x)", __func__, err);
  }
  usleep(500);

  ALOGE("%s: wipower_flag_: %d", __func__, wipower_flag_);

  return wipower_flag_;
}

int PatchDLManager::PerformChipInit()
{
  int ret = 0, err = -1, flags;
  char dst_buff[MAX_BUFF_SIZE];
  struct timeval tv;
  bool is_emb_wp_mode = false;
  char wipower_status[PROPERTY_VALUE_MAX];

  BluetoothAddress::GetLocalAddress(vnd_local_bd_addr_);

  /* Workaround UART issue: Make sure RTS is flowed ON in case it was not flowed on during cleanup due to UART issue */
  if ((err = uart_transport_->Ioctl(USERIAL_OP_FLOW_ON, &flags)) < 0) {
    ALOGE("%s: HW Flow-on error: 0x%x \n", __func__, err);
    return err;
  }

  property_get("ro.vendor.bluetooth.wipower", wipower_status, "false");
  if (strcmp(wipower_status, "true") == 0) {
    CheckEmbeddedMode();
    is_emb_wp_mode = true;
  } else {
    ALOGI("Wipower not enabled");
  }

  ret = SocInit(vnd_local_bd_addr_, is_emb_wp_mode);
  gettimeofday(&tv, NULL);
  if (ret < 0) {
    BtState::Get()->AddLogTag(dst_buff, tv, (char *)"SoC initialization failed");
    ALOGE("%s: SoC initialization failed: %d\n", __func__, ret);
  } else {
    BtState::Get()->AddLogTag(dst_buff, tv, (char *)"SoC initialization successful");
  }
  BtState::Get()->SetFwDwnldSucceed(dst_buff);

  return ret;
}

/*
 * For Hand-Off related Wipower commands, Command complete arrives first and
 * the followd with VS event
 *
 */
int PatchDLManager::HciSendWipowerVsCmd(unsigned char *cmd, unsigned char *rsp, int size)
{
  int ret = 0;
  int err = 0;

  /* Send the HCI command packet to UART for transmission */
  ret = uart_transport_->UartWrite(cmd, size);
  if (ret != size) {
    ALOGE("%s: WP Send failed with ret value: %d", __func__, ret);
    goto failed;
  }

  /* Wait for command complete event */
  err = ReadHciEvent(rsp, HCI_MAX_EVENT_SIZE);
  if ( err < 0) {
    ALOGE("%s: Failed to charging status cmd on Controller", __func__);
    goto failed;
  }
  ALOGI("%s: WP Received HCI command complete Event from SOC", __func__);
 failed:
  return ret;
}


int PatchDLManager::WipowerCurrentChargingStatusReq()
{
  int size, err = 0;
  unsigned char cmd[HCI_MAX_CMD_SIZE];
  unsigned char rsp[HCI_MAX_EVENT_SIZE];
  hci_command_hdr *cmd_hdr;

  memset(cmd, 0x0, HCI_MAX_CMD_SIZE);

  cmd_hdr = (hci_command_hdr*)(cmd + 1);
  cmd[0]  = HCI_COMMAND_PKT;
  cmd_hdr->opcode = cmd_opcode_pack(HCI_VENDOR_CMD_OGF, EDL_WIPOWER_VS_CMD_OCF);
  cmd_hdr->plen     = EDL_WIP_QUERY_CHARGING_STATUS_LEN;
  cmd[4]  = EDL_WIP_QUERY_CHARGING_STATUS_CMD;

  /* Total length of the packet to be sent to the Controller */
  size = (HCI_CMD_IND + HCI_COMMAND_HDR_SIZE + EDL_WIP_QUERY_CHARGING_STATUS_LEN);

  ALOGD("%s: Sending EDL_WIP_QUERY_CHARGING_STATUS_CMD", __func__);
  ALOGV("HCI-CMD: \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);

  err = HciSendWipowerVsCmd((unsigned char*)cmd, rsp, size);
  if ( err != size) {
    ALOGE("Failed to send EDL_WIP_QUERY_CHARGING_STATUS_CMD command!");
    goto error;
  }

  /* Check for response from the Controller */
  if (!unified_hci) {
    if (ReadVsHciEvent(rsp, HCI_MAX_EVENT_SIZE) < 0) {
      err = -ETIMEDOUT;
      ALOGI("%s: WP Failed to get HCI-VS Event from SOC", __func__);
      goto error;
    }

    /* Read Command Complete Event */
    if (rsp[4] > NON_WIPOWER_MODE) {
      err = ReadHciEvent(rsp, HCI_MAX_EVENT_SIZE);
      if (err < 0) {
        ALOGE("%s: Failed to get charging status", __func__);
        goto error;
      }
    }
   }
 error:
  return err;
}

int PatchDLManager::WipowerForwardHandoffReq()
{
  int size, err = 0;
  unsigned char cmd[HCI_MAX_CMD_SIZE];
  unsigned char rsp[HCI_MAX_EVENT_SIZE];
  hci_command_hdr *cmd_hdr;

  memset(cmd, 0x0, HCI_MAX_CMD_SIZE);

  cmd_hdr = (hci_command_hdr*)(cmd + 1);
  cmd[0]  = HCI_COMMAND_PKT;
  cmd_hdr->opcode = cmd_opcode_pack(HCI_VENDOR_CMD_OGF, EDL_WIPOWER_VS_CMD_OCF);
  cmd_hdr->plen     = EDL_WIP_START_HANDOFF_TO_HOST_LEN;
  cmd[4]  = EDL_WIP_START_HANDOFF_TO_HOST_CMD;

  /* Total length of the packet to be sent to the Controller */
  size = (HCI_CMD_IND + HCI_COMMAND_HDR_SIZE + EDL_WIP_START_HANDOFF_TO_HOST_LEN);

  ALOGD("%s: Sending EDL_WIP_START_HANDOFF_TO_HOST_CMD", __func__);
  ALOGD("HCI-CMD: \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
  err = HciSendWipowerVsCmd((unsigned char*)cmd, rsp, size);
  if ( err != size) {
    ALOGE("Failed to send EDL_WIP_START_HANDOFF_TO_HOST_CMD command!");
    goto error;
  }

  /* Check for response from the Controller */
  if (ReadVsHciEvent(rsp, HCI_MAX_EVENT_SIZE) < 0) {
    err = -ETIMEDOUT;
    ALOGI("%s: WP Failed to get HCI-VS Event from SOC", __func__);
    goto error;
  }

 error:
  return err;
}

int PatchDLManager::PatchVerReq()
{
  int size, err = 0;
  unsigned char cmd[HCI_MAX_CMD_SIZE];
  unsigned char rsp[HCI_MAX_EVENT_SIZE];
  char dst_buff[MAX_BUFF_SIZE] = {'\0'};
  char res_buff[MAX_BUFF_SIZE] = {'\0'};
  struct timeval tv;
  /* Frame the HCI CMD to be sent to the Controller */
  FrameHciPkt(cmd, EDL_PATCH_VER_REQ_CMD, 0, -1, EDL_PATCH_CMD_LEN);

  /* Total length of the packet to be sent to the Controller */
  size = (HCI_CMD_IND + HCI_COMMAND_HDR_SIZE + EDL_PATCH_CMD_LEN);
  gettimeofday(&tv, NULL);
  ALOGD("%s: Sending Get Version CMD to SOC", __func__);
  BtState :: Get()->AddLogTag(dst_buff, tv, (char *)"Sending Get Version CMD to SOC");
  BtState :: Get()->SetGetVerReqTS(dst_buff, SEND_CMD_TS);
  bt_logger_->SetSecondaryCrashReason(SEC_REASON_BTON_GETVER_SEND_STUCK);
  /* Send HCI Command packet to Controller */
  err = HciSendVsCmd((unsigned char*)cmd, rsp, size);
  if ( err != size) {
    ALOGE("Failed to attach the patch payload to the Controller!");
    goto error;
  } else if( (wait_vsc_evt_) && ((rsp[CMD_RSP_OFFSET] != EDL_CMD_REQ_RES_EVT) ||
            (rsp[RSP_TYPE_OFFSET] != EDL_APP_VER_RES_EVT)) && (!unified_hci)) {
    /* This is a workaround for an issue where an unexpected event was received */
    ALOGE("%s: Unexpected event revceived! Reading the next event", __func__);
    err = ReadHciEvent(rsp, HCI_MAX_EVENT_SIZE);
  } else {
    ALOGE("%s: rsp[CMD_RSP_OFFSET] = %x ,  rsp[RSP_TYPE_OFFSET] = %x", __func__, rsp[CMD_RSP_OFFSET], rsp[RSP_TYPE_OFFSET]);
  }

  /* Read Command Complete Event*/
  if (!unified_hci) {
    err = ReadHciEvent(rsp, HCI_MAX_EVENT_SIZE);
    if ( err < 0) {
      ALOGE("%s: Failed to get patch version(s)", __func__);
      goto error;
    }
  }
  gettimeofday(&tv, NULL);
  snprintf(res_buff, sizeof(res_buff), "Get Version rsp rcvd, num bytes in last rsp = %d", err);
  BtState :: Get()->AddLogTag(dst_buff, tv, res_buff);
  BtState :: Get()->SetGetVerResEvtTS(dst_buff);
  bt_logger_->SetSecondaryCrashReason(SEC_REASON_DEFAULT);
 error:
  return err;

}

int PatchDLManager::GetBuildInfoReq()
{
  int size, err = 0;
  unsigned char cmd[HCI_MAX_CMD_SIZE];
  unsigned char rsp[HCI_MAX_EVENT_SIZE];

  /* Frame the HCI CMD to be sent to the Controller */
  FrameHciPkt(cmd, EDL_GET_BUILD_INFO, 0,
                -1, EDL_PATCH_CMD_LEN);

  /* Total length of the packet to be sent to the Controller */
  size = (HCI_CMD_IND + HCI_COMMAND_HDR_SIZE + EDL_PATCH_CMD_LEN);

  /* Send HCI Command packet to Controller */
  err = HciSendVsCmd((unsigned char*)cmd, rsp, size);
  if ( err != size) {
    ALOGE("Failed to send get build info cmd to the SoC!");
    goto error;
  }

  if (!unified_hci) {
    err = ReadHciEvent( rsp, HCI_MAX_EVENT_SIZE);
    if ( err < 0) {
      ALOGE("%s: Failed to get build info", __func__);
      goto error;
    }
  }
 error:
  return err;

}

int PatchDLManager::GetTlvFile(const char *file_path, const char* alt_file_path)
{
  FILE * pFile = NULL;
  long fileSize;
  int readSize, nvm_length, nvm_tot_len, nvm_index, i;
  unsigned short nvm_tag_len;
  tlv_patch_info *ptlv_header;
  tlv_nvm_hdr *nvm_ptr;
  unsigned char data_buf[PRINT_BUF_SIZE] = { 0, };
  unsigned char *nvm_byte_ptr;

  pFile = OpenPatchFile(file_path, alt_file_path);
  if( pFile == NULL) {
    return -1;
  }

  /* Get File Size */
  fseek(pFile, 0, SEEK_END);
  fileSize = ftell(pFile);
  rewind(pFile);

  pdata_buffer_ = (unsigned char*)new char[fileSize];
  if (pdata_buffer_ == NULL) {
    ALOGE("Allocated Memory failed");
    fclose(pFile);
    return -1;
  }

  /* Copy file into allocated buffer */
  readSize = fread(pdata_buffer_, 1, fileSize, pFile);

  /* File Close */
  fclose(pFile);

  if (readSize != fileSize) {
    ALOGE("Read file size(%d) not matched with actual file size (%ld bytes)", readSize, fileSize);
    delete []pdata_buffer_;
    return -1;
  }

  ptlv_header = (tlv_patch_info*)pdata_buffer_;

  /* To handle different event between rampatch and NVM */
  tlv_type_ = ptlv_header->tlv_type;
  tlv_dwn_cfg_ = ptlv_header->tlv.patch.dwnd_cfg;

  if (ptlv_header->tlv_type == TLV_TYPE_PATCH) {
    ALOGI("====================================================");
    ALOGI("TLV Type\t\t\t : 0x%x", ptlv_header->tlv_type);
    ALOGI("Length\t\t\t : %d bytes", (ptlv_header->tlv_length1) |
          (ptlv_header->tlv_length2 << 8) |
          (ptlv_header->tlv_length3 << 16));
    ALOGI("Total Length\t\t\t : %d bytes", ptlv_header->tlv.patch.tlv_data_len);
    ALOGI("Patch Data Length\t\t\t : %d bytes", ptlv_header->tlv.patch.tlv_patch_data_len);
    ALOGI("Signing Format Version\t : 0x%x", ptlv_header->tlv.patch.sign_ver);
    ALOGI("Signature Algorithm\t\t : 0x%x", ptlv_header->tlv.patch.sign_algorithm);
    ALOGI("Event Handling\t\t\t : 0x%x", ptlv_header->tlv.patch.dwnd_cfg);
    ALOGI("Reserved\t\t\t : 0x%x", ptlv_header->tlv.patch.reserved1);
    ALOGI("Product ID\t\t\t : 0x%04x\n", ptlv_header->tlv.patch.prod_id);
    ALOGI("Rom Build Version\t\t : 0x%04x\n", ptlv_header->tlv.patch.build_ver);
    ALOGI("Patch Version\t\t : 0x%04x\n", ptlv_header->tlv.patch.patch_ver);
    ALOGI("Reserved\t\t\t : 0x%x\n", ptlv_header->tlv.patch.reserved2);
    ALOGI("Patch Entry Address\t\t : 0x%x\n", (ptlv_header->tlv.patch.patch_entry_addr));
    ALOGI("====================================================");

  } else if ( (ptlv_header->tlv_type >= TLV_TYPE_BT_NVM) &&
              ((ptlv_header->tlv_type <= TLV_TYPE_BT_FM_NVM)) ) {
    ALOGI("====================================================");
    ALOGI("TLV Type\t\t\t : 0x%x", ptlv_header->tlv_type);
    nvm_tot_len = nvm_length  = (ptlv_header->tlv_length1) |
                                (ptlv_header->tlv_length2 << 8) |
                                (ptlv_header->tlv_length3 << 16);
    ALOGI("Length\t\t\t : %d bytes",  nvm_tot_len);
    ALOGI("====================================================");

    if (nvm_tot_len <= 0)
      return readSize;

    if (ptlv_header->tlv_type == TLV_TYPE_BT_FM_NVM) {
      nvm_byte_ptr = (unsigned char*)ptlv_header;
      nvm_byte_ptr += 4;
      ptlv_header = (tlv_patch_info*)nvm_byte_ptr;
      nvm_tot_len -= 4;
      ALOGI("====================================================");
      ALOGI("TLV Type\t\t\t : 0x%x", ptlv_header->tlv_type);
      nvm_length = (ptlv_header->tlv_length1) |
                   (ptlv_header->tlv_length2 << 8) |
                   (ptlv_header->tlv_length3 << 16);
      ALOGI("Length\t\t\t : %d bytes",  nvm_length);
      ALOGI("====================================================");
    }

 multi_nvm:
    for (nvm_byte_ptr = (unsigned char*)(nvm_ptr = &(ptlv_header->tlv.nvm)), nvm_index = 0;
         nvm_index < nvm_length; nvm_ptr = (tlv_nvm_hdr*)nvm_byte_ptr) {
      ALOGV("TAG ID\t\t\t : %d", nvm_ptr->tag_id);
      nvm_tag_len = nvm_ptr->tag_len;
      ALOGV("TAG Length\t\t : %d", nvm_tag_len);
      ALOGV("TAG Pointer\t\t : %d", nvm_ptr->tag_ptr);
      ALOGV("TAG Extended Flag\t : %d", nvm_ptr->tag_ex_flag);

      /* Increase nvm_index to NVM data */
      nvm_index += sizeof(tlv_nvm_hdr);
      nvm_byte_ptr += sizeof(tlv_nvm_hdr);
      if (ptlv_header->tlv_type == TLV_TYPE_BT_NVM) {
        /* Write BD Address */
        if (nvm_ptr->tag_id == TAG_NUM_2) {
          memcpy(nvm_byte_ptr, vnd_local_bd_addr_, 6);
          ALOGI("BD Address: %.02x:%.02x:%.02x:%.02x:%.02x:%.02x",
                *nvm_byte_ptr, *(nvm_byte_ptr + 1), *(nvm_byte_ptr + 2),
                *(nvm_byte_ptr + 3), *(nvm_byte_ptr + 4), *(nvm_byte_ptr + 5));
        }

        if (IS_NEWER_THAN_CHEROKEE_2_1(chipset_ver_)) {
          UpdateNewNvmFormat(nvm_ptr);
        } else {
          /* Change SIBS setting */
          if (!IsSibsEnabled()) {
            if (nvm_ptr->tag_id == 17) {
              *nvm_byte_ptr = ((*nvm_byte_ptr) & ~(0x80));
            }

            if (nvm_ptr->tag_id == 27) {
              *nvm_byte_ptr = ((*nvm_byte_ptr) & ~(0x01));
            }
          }

          /* Change Baudrate setting based on Host setting */
          if (nvm_ptr->tag_id == 17) {
            *(nvm_byte_ptr + 2) = uart_transport_->GetMaxBaudrate();
          }
        }
      }
      for (i = 0; (i < nvm_ptr->tag_len && (i * 3 + 2) < PRINT_BUF_SIZE); i++)
        snprintf((char*)data_buf, PRINT_BUF_SIZE, "%s%.02x ", (char*)data_buf, *(nvm_byte_ptr + i));

      ALOGV("TAG Data\t\t\t : %s", data_buf);

      /* Clear buffer */
      memset(data_buf, 0x0, PRINT_BUF_SIZE);

      /* increased by tag_len */
      nvm_index += nvm_ptr->tag_len;
      nvm_byte_ptr += nvm_ptr->tag_len;
    }

    nvm_tot_len -= nvm_index;

    if (nvm_tot_len > 4 ) {
      nvm_byte_ptr = (unsigned char*)ptlv_header;
      nvm_byte_ptr += (4 + nvm_index);
      ptlv_header = (tlv_patch_info*)nvm_byte_ptr;
      nvm_tot_len -= 4;
      ALOGI("====================================================");
      ALOGI("TLV Type\t\t\t : 0x%x", ptlv_header->tlv_type);
      nvm_length = (ptlv_header->tlv_length1) |
                   (ptlv_header->tlv_length2 << 8) |
                   (ptlv_header->tlv_length3 << 16);
      ALOGI("Length\t\t\t : %d bytes",  nvm_length);
      ALOGI("====================================================");
      goto multi_nvm;
    }

    ALOGI("====================================================");

  } else {
    ALOGI("TLV Header type is unknown (%d) ", ptlv_header->tlv_type);
  }

  return readSize;
}

FILE* PatchDLManager::OpenPatchFile(const char *file_path, const char* alt_file_path) {
  FILE *pFile = NULL;

  if (!(file_path && (pFile = fopen( file_path, "r" )))) {
    ALOGE("%s File Open Fail %s (%d)", file_path, strerror(errno), errno);
    //Try opening from alternate path
    if (!(alt_file_path && (pFile = fopen(alt_file_path, "r")))) {
      ALOGE("%s File Opening from alternate path: Fail %s (%d)", alt_file_path,
            strerror(errno), errno);
      return NULL;
    } else {
      ALOGI("File open %s succeeded", alt_file_path);
      return pFile;
    }
  } else {
    ALOGI("File open %s succeeded", file_path);
    return pFile;
  }
}

int PatchDLManager::TlvDnldSegment(int index, int seg_size, unsigned char wait_cc_evt)
{
  int size = 0, err = -1;
  unsigned char cmd[HCI_MAX_CMD_SIZE];
  unsigned char rsp[HCI_MAX_EVENT_SIZE];

  ALOGV("%s: Downloading TLV Patch segment no.%d, size:%d", __func__, index, seg_size);

  /* Frame the HCI CMD PKT to be sent to Controller*/
  FrameHciPkt(cmd, EDL_PATCH_TLV_REQ_CMD, 0, index, seg_size);

  /* Total length of the packet to be sent to the Controller */
  size = (HCI_CMD_IND + HCI_COMMAND_HDR_SIZE + cmd[PLEN]);

  /* Initialize the RSP packet everytime to 0 */
  memset(rsp, 0x0, HCI_MAX_EVENT_SIZE);

  /* Send HCI Command packet to Controller */
  err = HciSendVsCmd((unsigned char*)cmd, rsp, size);

  if ( err != size) {
    ALOGE("Failed to send the patch payload to the Controller! 0x%x", err);
    return err;
  }

  if (!unified_hci) {
    if (wait_cc_evt) {
      err = ReadHciEvent(rsp, HCI_MAX_EVENT_SIZE);
      if ( err < 0) {
        ALOGE("%s: Failed to downlaod patch segment: %d!",  __func__, index);
        return err;
      }
    }
  }
  ALOGV("%s: Successfully downloaded patch segment: %d", __func__, index);
  return err;
}

int PatchDLManager::TlvDnldReq(int tlv_size)
{
  int total_segment, remain_size, i, err = -1;
  unsigned char wait_cc_evt = true;
  bool is_last_seg = false;
  int segment_download_len = MAX_SIZE_PER_TLV_SEGMENT;

  total_segment = tlv_size / MAX_SIZE_PER_TLV_SEGMENT;
  remain_size = (tlv_size < MAX_SIZE_PER_TLV_SEGMENT) ? \
                tlv_size : (tlv_size % MAX_SIZE_PER_TLV_SEGMENT);

  ALOGI("%s: TLV size: %d, Total Seg num: %d, remain size: %d",
        __func__, tlv_size, total_segment, remain_size);

  if (tlv_type_ == TLV_TYPE_PATCH) {
    /* Prior to Rome version 3.2(including inital few rampatch release of Rome 3.2), the event
     * handling mechanism is SKIP_EVT_NONE. After few release of rampatch for Rome 3.2, the
     * mechamism is changed to SKIP_EVT_VSE_CC. Rest of the mechanism is not used for now
     */
    switch (tlv_dwn_cfg_) {
      case SKIP_EVT_NONE:
        wait_vsc_evt_ = true;
        wait_cc_evt = true;
        ALOGI("Event handling type: SKIP_EVT_NONE");
        break;
      case SKIP_EVT_VSE_CC:
        wait_vsc_evt_ = false;
        wait_cc_evt = false;
        ALOGI("Event handling type: SKIP_EVT_VSE_CC");
        break;
      /* Not handled for now */
      case SKIP_EVT_VSE:
      case SKIP_EVT_CC:
      default:
        ALOGE("Unsupported Event handling: %d", tlv_dwn_cfg_);
        break;
    }
  } else {
    wait_vsc_evt_ = true;
    wait_cc_evt = true;
  }

  for (i = 0; i <= total_segment && !is_last_seg; i++) {
    /* check for last segment based on remaining size
     * and total number of segments.
     */
    if ((remain_size && i == total_segment) ||
        (!remain_size && (i + 1) == total_segment)) {
      is_last_seg = true;
      // Update segment download len if last segment is being downloaded
      if (remain_size)
        segment_download_len = remain_size;
      ALOGD("%s: Updating seg len to %d as last segment",
        __func__, segment_download_len);
    }
    if (tlv_type_ == TLV_TYPE_PATCH && is_last_seg) {
      /*
       * 1. None of the command segments receive CCE
       * 2. No command segments receive VSE except the last one
       * 3. If tlv_dwn_cfg_ is SKIP_EVT_NONE then wait for VSE and CCE
       * ( except CCE is not received for last segment)
       */
        wait_cc_evt = false;
        wait_vsc_evt_ = true;
    }

    patch_dnld_pending_ = true;
    if ((err = TlvDnldSegment(i, segment_download_len, wait_cc_evt )) < 0)
      goto error;
    patch_dnld_pending_ = false;
  }

 error:
  if (patch_dnld_pending_)
    patch_dnld_pending_ = false;
  return err;
}

void PatchDLManager::FrameHciCmdPkt(
  unsigned char *cmd,
  int edl_cmd, unsigned int p_base_addr,
  int segtNo, int size
)
{
  int offset = 0;
  hci_command_hdr *cmd_hdr;

  memset(cmd, 0x0, HCI_MAX_CMD_SIZE);

  cmd_hdr = (hci_command_hdr*)(cmd + 1);

  cmd[0]      = HCI_COMMAND_PKT;
  cmd_hdr->opcode = cmd_opcode_pack(HCI_VENDOR_CMD_OGF, HCI_PATCH_CMD_OCF);
  cmd_hdr->plen   = size;
  cmd[4]      = edl_cmd;

  switch (edl_cmd) {
    case EDL_PATCH_SET_REQ_CMD:
      /* Copy the patch header info as CMD params */
      memcpy(&cmd[5], phdr_buffer_, PATCH_HDR_LEN);
      ALOGD("%s: Sending EDL_PATCH_SET_REQ_CMD", __func__);
      ALOGD("HCI-CMD %d:\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x",
            segtNo, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
      break;
    case EDL_PATCH_DLD_REQ_CMD:
      offset = ((segtNo - 1) * MAX_DATA_PER_SEGMENT);
      p_base_addr += offset;
      cmd_hdr->plen   = (size + 6);
      cmd[5]  = (size + 4);
      cmd[6]  = EXTRACT_BYTE(p_base_addr, 0);
      cmd[7]  = EXTRACT_BYTE(p_base_addr, 1);
      cmd[8]  = EXTRACT_BYTE(p_base_addr, 2);
      cmd[9]  = EXTRACT_BYTE(p_base_addr, 3);
      memcpy(&cmd[10], (pdata_buffer_ + offset), size);

      ALOGD("%s: Sending EDL_PATCH_DLD_REQ_CMD: size: %d bytes",
            __func__, size);
      ALOGD("HCI-CMD %d:\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t"
            "0x%x\t0x%x\t0x%x\t\n", segtNo, cmd[0], cmd[1], cmd[2],
            cmd[3], cmd[4], cmd[5], cmd[6], cmd[7], cmd[8], cmd[9]);
      break;
    case EDL_PATCH_ATCH_REQ_CMD:
      ALOGD("%s: Sending EDL_PATCH_ATTACH_REQ_CMD", __func__);
      ALOGD("HCI-CMD %d:\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x",
            segtNo, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
      break;
    case EDL_PATCH_RST_REQ_CMD:
      ALOGD("%s: Sending EDL_PATCH_RESET_REQ_CMD", __func__);
      ALOGD("HCI-CMD %d:\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x",
            segtNo, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
      break;
    case EDL_PATCH_VER_REQ_CMD:
      ALOGD("%s: Sending EDL_PATCH_VER_REQ_CMD", __func__);
      ALOGD("HCI-CMD %d:\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x",
            segtNo, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
      break;
    case EDL_PATCH_TLV_REQ_CMD:
      ALOGD("%s: Sending EDL_PATCH_TLV_REQ_CMD", __func__);
      /* Parameter Total Length */
      cmd[3] = size + 2;

      /* TLV Segment Length */
      cmd[5] = size;
      ALOGD("HCI-CMD %d:\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x",
            segtNo, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);
      offset = (segtNo * MAX_SIZE_PER_TLV_SEGMENT);
      memcpy(&cmd[6], (pdata_buffer_ + offset), size);
      break;
    case EDL_GET_BUILD_INFO:
      ALOGD("%s: Sending EDL_GET_BUILD_INFO", __func__);
      ALOGD("HCI-CMD %d:\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x",
            segtNo, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
      break;
    case EDL_GET_BOARD_ID:
      ALOGD("%s: Sending EDL_GET_BOARD_ID", __func__);
      ALOGD("HCI-CMD %d:\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x",
            segtNo, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
      break;
    default:
      ALOGE("%s: Unknown EDL CMD !!!", __func__);
  }
}

int PatchDLManager::GetBoardIdReq()
{
  int size, err = 0;
  unsigned char cmd[HCI_MAX_CMD_SIZE];
  unsigned char rsp[HCI_MAX_EVENT_SIZE];
  bool cmd_supported = true;

  /* Frame the HCI CMD to be sent to the Controller */
  FrameHciCmdPkt(cmd, EDL_GET_BOARD_ID, 0,
                    -1, EDL_PATCH_CMD_LEN);
  /* Total length of the packet to be sent to the Controller */
  size = (HCI_CMD_IND + HCI_COMMAND_HDR_SIZE + EDL_PATCH_CMD_LEN);

  ALOGI("%s: Sending EDL_GET_BOARD_ID", __func__);
  err = HciSendVsCmd((unsigned char*)cmd, rsp, size);
  if ( err != size) {
    ALOGE("Failed to send EDL_GET_BOARD_ID command!");
    cmd_supported = false;
  }

  if (!unified_hci) {
    err = ReadHciEvent(rsp, HCI_MAX_EVENT_SIZE);
    if (err < 0) {
      ALOGE("%s: Failed to get feature request", __func__);
    }
  }
  return (cmd_supported == true ? err : -1);
}

int PatchDLManager::DownloadTlvFile(const char *rampatch_file_path,
  const char *nvm_file_path, const char *rampatch_alt_file_path, const char *nvm_alt_file_path)
{
  int tlv_size = -1;
  int err = -1;
  char nvm_file_path_bid[256] = { 0, };
  char nvm_alt_file_path_bid[256] = { 0, };
  int nvm_file_path_len = strlen(nvm_file_path);
  int nvm_alt_file_path_len = strlen(nvm_alt_file_path);

  if (nvm_file_path_len != 0)
    memcpy(nvm_file_path_bid, nvm_file_path, nvm_file_path_len - 2);
  if (nvm_alt_file_path_len != 0)
    memcpy(nvm_alt_file_path_bid, nvm_alt_file_path, nvm_alt_file_path_len - 2);

  /* Rampatch TLV file Downloading */
  bt_logger_->SetSecondaryCrashReason(SEC_REASON_BTON_PATCH_DNLD_STUCK);
  if ((tlv_size = GetTlvFile(rampatch_file_path, rampatch_alt_file_path)) > 0) {
    err = TlvDnldReq(tlv_size);
    delete []pdata_buffer_;
    if (err < 0) {
      return err;
    }
  }

#ifdef DEBUG_CRASH_SOC_WHILE_DLD
  {
    char trial_prop[PROPERTY_VALUE_MAX];
    int value = 0;
    property_get("persist.vendor.bluetooth.csoc.cnt", trial_prop, "0");
    value = atoi(trial_prop);
    ALOGV("%s: value : %d\n", __func__, value);
    if (value > 0) {
      sprintf(trial_prop, "%d", (value = value - 1));
      property_set("persist.vendor.bluetooth.csoc.cnt", trial_prop);
      SendCrashCommand();
    }
  }
#endif      //DEBUG_CRASH_SOC_WHILE_DLD

  /* NVM TLV file Downloading */
  tlv_size = -1;
  err = -1;
  bt_logger_->SetSecondaryCrashReason(SEC_REASON_BTON_GETBOARDID_CMD_STUCK);
  if (GetBoardIdReq() < 0) {
    ALOGE("%s: failed to get board id(0x%x)", __func__, err);
  } else {
    strlcat(nvm_file_path_bid, (char*)board_id_, sizeof(nvm_file_path_bid));
    strlcat(nvm_alt_file_path_bid, (char*)board_id_, sizeof(nvm_alt_file_path_bid));
    if ((tlv_size = GetTlvFile(nvm_file_path_bid,nvm_alt_file_path_bid)) < 0) {
      ALOGI("%s: %s: file doesn't exist, falling back to default file", __func__, nvm_file_path_bid);
    }
  }

  if (tlv_size < 0) {
    if ((tlv_size = GetTlvFile(nvm_file_path, nvm_alt_file_path)) < 0) {
      return err;
    }
  }

  bt_logger_->SetSecondaryCrashReason(SEC_REASON_BTON_NVM_DNLD_STUCK);
  err = TlvDnldReq(tlv_size);

  delete []pdata_buffer_;
  return err;
}

/* Get addon features that are supported by FW */
features_list* PatchDLManager::GetAddOnFeatureList()
{
  int size, err = 0;
  unsigned char cmd[HCI_MAX_CMD_SIZE];
  unsigned char rsp[HCI_MAX_EVENT_SIZE];
  hci_command_hdr *cmd_hdr;
  uint8_t param_len, cmd_status;

  if (add_on_features.feat_mask_len == INVALID_FEATURE_MASK_LENGTH) {
    ALOGW("%s: Invalid features mask length is set, returning NULL", __func__);
    return NULL;
  } else if (add_on_features.feat_mask_len > 0) {
    return &add_on_features;
  }

  memset(cmd, 0x0, HCI_MAX_CMD_SIZE);

  cmd_hdr = (hci_command_hdr*)(cmd + 1);
  cmd[0]  = HCI_COMMAND_PKT;
  cmd_hdr->opcode = cmd_opcode_pack(HCI_VENDOR_CMD_OGF, HCI_VS_GET_ADDON_FEATURES_SUPPORT);
  cmd_hdr->plen     = 0x00;

  /* Total length of the packet to be sent to the Controller */
  size = (HCI_CMD_IND + HCI_COMMAND_HDR_SIZE);

  ALOGD("%s: Sending HCI_VS_GET_ADDON_FEATURES_SUPPORT", __func__);
  ALOGD("HCI-CMD: \t0x%x \t0x%x \t0x%x \t0x%x", cmd[0], cmd[1], cmd[2], cmd[3]);

  err = HciSendVsCmd((unsigned char*)cmd, rsp, size);
  if ( err != size) {
    ALOGE("Failed to send HCI_VS_GET_ADDON_FEATURES_SUPPORT command!");
    goto error;
  }

  if (!unified_hci) {
    err = ReadHciEvent(rsp, HCI_MAX_EVENT_SIZE);
    if (err < 0) {
      ALOGE("%s: Failed to get feature request", __func__);
      goto error;
    }
  }

  param_len = rsp[PARM_LEN_BYTE];
  cmd_status = rsp[STATUS_BYTE];

  ALOGI("%s: param_len %d, cmd_status = %d", __func__,
      param_len, cmd_status);

  if ((param_len > HCI_EVENT_PARAMS)  && (cmd_status == 0x00)) {
    PTR_TO_UINT16(add_on_features.product_id, &rsp[PRODUCT_ID_BYTE]);
    PTR_TO_UINT16(add_on_features.rsp_version, &rsp[RSP_VER_BYTE]);
    add_on_features.feat_mask_len = param_len - HCI_EVENT_PARAMS;
    ALOGI("%s: feat_mask_len %d product_id = %d, rsp_version = %d", __func__,
        add_on_features.feat_mask_len, add_on_features.product_id,
        add_on_features.rsp_version);
    memcpy(add_on_features.features, &rsp[FEAT_MASK_BYTES],
        add_on_features.feat_mask_len);
    return &add_on_features;
  }

 error:
  //older fw does not return proper mask length, hence setting length to invalid
  add_on_features.feat_mask_len = INVALID_FEATURE_MASK_LENGTH;

  return NULL;
}

void PatchDLManager::EnableControllerLog()
{
  int ret = 0;
  /* VS command to enable controller logging to the HOST. By default it is disabled */
  unsigned char cmd[6] = { 0x01, 0x17, 0xFC, 0x02, 0x00, 0x00 };
  unsigned char rsp[HCI_MAX_EVENT_SIZE];
  char value[PROPERTY_VALUE_MAX] = { '\0' };
  unsigned char temp_wait_vsc_evt;

  property_get("persist.vendor.service.bdroid.soclog", value, "false");

  // value at cmd[5]: 1 - to enable, 0 - to disable
  ret = (strcmp(value, "true") == 0) ? cmd[5] = 0x01 : 0;
  ALOGI("%s: %d", __func__, ret);

  temp_wait_vsc_evt = wait_vsc_evt_;
  wait_vsc_evt_ = false;
  /* only sending cmd */
  ret = HciSendVsCmd((unsigned char*)cmd, rsp, 6);
  if (ret != 6) {
    ALOGE("%s: command failed", __func__);
  }
  if (!unified_hci) {
    ret = ReadNewHciEvent(rsp, HCI_MAX_EVENT_SIZE);
    if (ret < 0) {
      ALOGE("%s: failed to read rsp", __func__);
    } else {
      bt_logger_->ProcessRx(HCI_PACKET_TYPE_EVENT, &rsp[1], ret - 1);
      /* Checking VSE rsp */
      if (rsp[1] != LOG_BT_EVT_VENDOR_SPECIFIC || rsp[2] != 1
          || rsp[3] != HCI_VS_HOST_LOG_CTRL_SUBID) {
        ALOGE("%s: Not rcvd correct VS event", __func__);
        ALOGE("%s: rsp Event type: 0x%x" , __func__, rsp[1]);
        ALOGE("%s: rsp parameter length: 0x%x" , __func__, rsp[2]);
        ALOGE("%s: rsp subopcode: 0x%x" , __func__, rsp[3]);
      }
    }
  }
  wait_vsc_evt_ = temp_wait_vsc_evt;

  ret = ReadHciEvent(rsp, HCI_MAX_EVENT_SIZE);
  if (ret < 0) {
    ALOGE("%s: Failed to get CC for enable SoC log", __func__);
  }
  return;
}

void PatchDLManager::EnableEnhControllerLog()
{
  int ret = 0;
  /* VS command to enable/disable enhanced controller logging to the HOST */
  unsigned char cmd[6] = { 0x01, 0x17, 0xFC, 0x02, 0x14, 0x00 };
  unsigned char temp_wait_vsc_evt;
  unsigned char rsp[HCI_MAX_EVENT_SIZE];
  char value[PROPERTY_VALUE_MAX] = { '\0' };

  property_get("persist.vendor.service.bdroid.soclog", value, "false");

  // value at cmd[5]: 1 - to enable, 0 - to disable
  ret = (strcmp(value, "true") == 0) ? cmd[5] = 0x01 : 0;
  ALOGI("%s: %s", __func__, (ret == 0) ? "disabled" : "enabled");

  // Command Status not expected for enhanced logging
  temp_wait_vsc_evt = wait_vsc_evt_;
  wait_vsc_evt_ = false;

  ret = HciSendVsCmd((unsigned char*)cmd, rsp, 6);
  if (ret != 6) {
    ALOGE("%s: command failed", __func__);
  }
  wait_vsc_evt_ = temp_wait_vsc_evt;

  ret = ReadHciEvent(rsp, HCI_MAX_EVENT_SIZE);

  if (ret < 0) {
   ALOGE("%s: Failed to get CC for enable enh SoC log", __func__);
  }

  return;
}

int PatchDLManager::SetBaudRateReq()
{
  int size, err = 0;
  unsigned char cmd[HCI_MAX_CMD_SIZE];
  unsigned char rsp[HCI_MAX_EVENT_SIZE];
  hci_command_hdr *cmd_hdr;
  int flags;
  uint8_t bt_baud_rate = uart_transport_->GetMaxBaudrate();

  memset(cmd, 0x0, HCI_MAX_CMD_SIZE);

  cmd_hdr = (hci_command_hdr*)(cmd + 1);
  cmd[0]  = HCI_COMMAND_PKT;
  cmd_hdr->opcode = cmd_opcode_pack(HCI_VENDOR_CMD_OGF, EDL_SET_BAUDRATE_CMD_OCF);
  cmd_hdr->plen     = VSC_SET_BAUDRATE_REQ_LEN;
  cmd[4]  = bt_baud_rate;

  /* Total length of the packet to be sent to the Controller */
  size = (HCI_CMD_IND + HCI_COMMAND_HDR_SIZE + VSC_SET_BAUDRATE_REQ_LEN);

  /* Flow off during baudrate change */
  if ((err = uart_transport_->Ioctl(USERIAL_OP_FLOW_OFF, &flags)) < 0) {
    ALOGE("%s: HW Flow-off error: 0x%x\n", __func__, err);
    goto error;
  }

  /* Send the HCI command packet to UART for transmission */
  err = uart_transport_->UartWrite(cmd, size);
  if (err != size) {
    ALOGE("%s: Send failed with ret value: %d", __func__, err);
    goto error;
  }

  usleep(20 * 1000);

  tcdrain(fd_transport_);
  /* Change Local UART baudrate to high speed UART */
  uart_transport_->SetBaudRate(bt_baud_rate);

  /* Check current Baudrate */
  uart_transport_->GetBaudRate();

  /* Flow on after changing local uart baudrate */
  if ((err = uart_transport_->Ioctl(USERIAL_OP_FLOW_ON, &flags)) < 0) {
    ALOGE("%s: HW Flow-on error: 0x%x \n", __func__, err);
    return err;
  }

  /* Check for response from the Controller */
  if (!unified_hci) {
    if ((err = ReadVsHciEvent(rsp, HCI_MAX_EVENT_SIZE)) < 0) {
      ALOGE("%s: Failed to get HCI-VS Event from SOC", __func__);
      goto error;
    }
    ALOGV("%s: Received HCI-Vendor Specific Event from SOC", __func__);
  }

  /* Wait for command complete event */
  err = ReadHciEvent(rsp, HCI_MAX_EVENT_SIZE);
  if ( err < 0) {
    ALOGE("%s: Failed to set patch info on Controller", __func__);
    goto error;
  }

 error:
  return err;

}

int PatchDLManager::SocInit(uint8_t *bdaddr, bool is_emb_wp_mode)
{
  int err = -1;
  PatchPathManager info;
  UNUSED(bdaddr);
  UNUSED(is_emb_wp_mode);
  ALOGI(" %s ", __func__);

  dnld_fd_in_progress_ = fd_transport_;
  /* If wipower charging is going on in embedded mode then start hand off req */
  if (wipower_flag_ == WIPOWER_IN_EMBEDDED_MODE && wipower_handoff_ready_ != NON_WIPOWER_MODE) {
    wipower_flag_ = 0;
    wipower_handoff_ready_ = 0;
    if ((err = WipowerForwardHandoffReq()) < 0) {
      ALOGI("%s: Wipower handoff failed (0x%x)", __func__, err);
    }
  }

  /* Get Chipset version information */
  if ((err = PatchVerReq()) < 0) {
    ALOGE("%s: Fail to get chipset Version (0x%x)", __func__, err);
    dnld_fd_in_progress_ = -1;
    return err;
  }
  ALOGI("%s: Chipset Version (0x%16llx)", __func__,
    (unsigned long long)chipset_ver_);
  info = PatchPathInfoMap_.find(chipset_ver_)->second;

  if (!info.GetChipName().empty()) {
    FormRegularPaths(info);
  } else {
    FormDefaultPaths();
  }

  bt_logger_->SetSecondaryCrashReason(SEC_REASON_BTON_SETBAUDRATE_CMD_STUCK);
  /* Change baud rate 115.2 kbps to 3Mbps*/
  err = SetBaudRateReq();
  if (err < 0) {
    ALOGE("%s: Baud rate change failed!", __func__);
    dnld_fd_in_progress_ = -1;
    return err;
  }

  ALOGI("%s: Baud rate changed successfully ", __func__);
  /* Donwload TLV files (rampatch, NVM) */
  err = DownloadTlvFile(rampatch_file_path, nvm_file_path, rampatch_alt_file_path, nvm_alt_file_path);
  if (err < 0) {
    ALOGE("%s: Download TLV file failed!", __func__);
    dnld_fd_in_progress_ = -1;
    return err;
  }
  ALOGI("%s: Download TLV file successfully ", __func__);

  /* Get SU FM label information */
  bt_logger_->SetSecondaryCrashReason(SEC_REASON_BTON_GETBLDINFO_CMD_STUCK);
  if ((err = GetBuildInfoReq()) < 0) {
    ALOGI("%s: Fail to get FW SU Build info (0x%x)", __func__, err);
    //Ignore the failure of FW SU label information
    err = 0;
  }

  /* Disable internal LDO to use external LDO instead*/
  err = DisableInternalLdo();

  /*  get chipset supported feature request */
  ALOGV("%s: chipset_ver_: 0x%16llx Calling get addon feature",__func__,
    (unsigned long long)chipset_ver_);
  bt_logger_->SetSecondaryCrashReason(SEC_REASON_BTON_ADDONFEAT_CMD_STUCK);
  GetAddOnFeatureList();

  /* send enhanced controller logging cmd only for non rome
     chipsets as this command is supported only for them
  */
  bt_logger_->SetSecondaryCrashReason(SEC_REASON_BTON_ENHLOG_CMD_STUCK);
  if(soc_type_ == BT_SOC_ROME) {
    EnableControllerLog();
  } else {
    EnableEnhControllerLog();
  }

  /* Send HCI Reset */
  bt_logger_->SetSecondaryCrashReason(SEC_REASON_BTON_HCI_RESET_STUCK);
  err = HciReset();
  if ( err < 0 ) {
    ALOGE("HCI Reset Failed !!");
  } else {
    ALOGV("HCI Reset is done\n");
  }

  dnld_fd_in_progress_ = -1;
  return err;
}

void PatchDLManager::FormRegularPaths(PatchPathManager info)
{
  struct stat sts;

  snprintf(rampatch_file_path, sizeof(rampatch_file_path), "%s%s", BT_FW_REGULAR_PATH,
           info.GetTLVName().c_str());
  snprintf(nvm_file_path, sizeof(nvm_file_path), "%s%s", BT_FW_REGULAR_PATH,
           info.GetNVMName().c_str());
  snprintf(rampatch_alt_path_qti_check, sizeof(rampatch_alt_path_qti_check), "%s%s",
           BT_FW_QTI_ALT_PATH, info.GetTLVName().c_str());
  snprintf(nvm_alt_path_qti_check, sizeof(nvm_alt_path_qti_check), "%s%s",
           BT_FW_QTI_ALT_PATH, info.GetNVMName().c_str());
  snprintf(rampatch_alt_path_check, sizeof(rampatch_alt_path_check), "%s%s",
           BT_FW_ALT_PATH, info.GetTLVName().c_str());
  snprintf(nvm_alt_path_check, sizeof(nvm_alt_path_check), "%s%s",
           BT_FW_ALT_PATH, info.GetNVMName().c_str());

  if (!stat(rampatch_alt_path_qti_check, &sts) && sts.st_size > 0) {
    strlcpy(rampatch_alt_file_path, rampatch_alt_path_qti_check,
            sizeof(rampatch_alt_file_path));
  } else if (!stat(rampatch_alt_path_check, &sts) && sts.st_size > 0) {
    strlcpy(rampatch_alt_file_path, rampatch_alt_path_check,
            sizeof(rampatch_alt_file_path));
  }
  if (!stat(nvm_alt_path_qti_check, &sts) && sts.st_size > 0) {
    strlcpy(nvm_alt_file_path, nvm_alt_path_qti_check, sizeof(nvm_alt_file_path));
  } else if (!stat(nvm_alt_path_check, &sts) && sts.st_size > 0) {
    strlcpy(nvm_alt_file_path, nvm_alt_path_check, sizeof(nvm_alt_file_path));
  }
}

void PatchDLManager::FormDefaultPaths()
{
  struct stat sts;

  strlcpy(rampatch_file_path, TEMP_RAMPATCH_TLV_0_0_0_PATH, sizeof(rampatch_file_path));
  strlcpy(nvm_file_path, TEMP_NVM_TLV_0_0_0_PATH, sizeof(nvm_file_path));
  if (!stat(TEMP_RAMPATCH_TLV_0_0_0_PATH_ALT_QTI, &sts) && sts.st_size > 0) {
    strlcpy(rampatch_alt_file_path, TEMP_RAMPATCH_TLV_0_0_0_PATH_ALT_QTI,
            sizeof(rampatch_alt_file_path));
  } else if (!stat(TEMP_RAMPATCH_TLV_0_0_0_PATH_ALT, &sts) && sts.st_size > 0) {
    strlcpy(rampatch_alt_file_path, TEMP_RAMPATCH_TLV_0_0_0_PATH_ALT,
            sizeof(rampatch_alt_file_path));
  }
  if (!stat(TEMP_NVM_TLV_0_0_0_PATH_ALT_QTI, &sts) && sts.st_size > 0) {
    strlcpy(nvm_alt_file_path, TEMP_NVM_TLV_0_0_0_PATH_ALT_QTI, sizeof(nvm_alt_file_path));
  } else if (!stat(TEMP_NVM_TLV_0_0_0_PATH_ALT, &sts) && sts.st_size > 0) {
    strlcpy(nvm_alt_file_path, TEMP_NVM_TLV_0_0_0_PATH_ALT, sizeof(nvm_alt_file_path));
  }

}

int PatchDLManager::HciReset()
{
  int size, err = 0;
  unsigned char cmd[HCI_MAX_CMD_SIZE];
  unsigned char rsp[HCI_MAX_EVENT_SIZE];
  hci_command_hdr *cmd_hdr;
#ifdef DUMP_RINGBUF_LOG
  DataHandler *data_handler = DataHandler::Get();
#endif

  ALOGI("%s: HCI RESET ", __func__);

  memset(cmd, 0x0, HCI_MAX_CMD_SIZE);

  cmd_hdr = (hci_command_hdr*)(cmd + 1);
  cmd[0]  = HCI_COMMAND_PKT;
  cmd_hdr->opcode = HCI_RESET;
  cmd_hdr->plen   = 0;

  /* Total length of the packet to be sent to the Controller */
#ifdef DUMP_RINGBUF_LOG
  gettimeofday(&data_handler->time_hci_cmd_arrived_, NULL);
  snprintf(data_handler->last_hci_cmd_timestamp_.opcode, OPCODE_LEN, "0x%x%x", cmd[0], cmd[1]);
  data_handler->AddHciCommandTag(data_handler->last_hci_cmd_timestamp_.hcicmd_timestamp,
                     data_handler->time_hci_cmd_arrived_, data_handler->last_hci_cmd_timestamp_.opcode);
  bt_logger_->SaveLastStackHciCommand(data_handler->last_hci_cmd_timestamp_.hcicmd_timestamp);
#endif

  size = (HCI_CMD_IND + HCI_COMMAND_HDR_SIZE);
  bt_logger_->ProcessTx(HCI_PACKET_TYPE_COMMAND, &cmd[1], size - 1);
  err = uart_transport_->UartWrite(cmd, size);
  if (err != size) {
    ALOGE("%s: Send failed with ret value: %d", __func__, err);
    err = -1;
    goto error;
  }

  /* Wait for command complete event */
  err = ReadCmdCmplEvent(rsp, HCI_MAX_EVENT_SIZE);
  if (err < 0) {
    ALOGE("%s: Failed to read rsp for HCI Reset command", __func__);
    goto error;
  }

 error:
  return err;

}

int PatchDLManager::ReadNewHciEvent(unsigned char* buf, int size)
{
  int retval;
  unsigned char protocol_byte;
  unsigned char hdr[BT_EVT_HDR_SIZE];
  unsigned char packet_len;
  unsigned short tot_len;
  unsigned char* tmp_buf = NULL;

  ALOGV("%s: >", __func__);

  if (size < BT_ACL_HDR_SIZE) {
    ALOGE("%s: Invalid size: %d\n", __func__, size);
    return -1;
  }

  do {
    retval = uart_transport_->Read(&protocol_byte, 1);
    if (retval < 0) {
      ALOGE("%s: read error", __func__);
      return -1;
    }
    if (protocol_byte == LOG_BT_EVT_PACKET_TYPE) {
      break;
    } else if (protocol_byte == LOG_BT_ACL_PACKET_TYPE) {
      /* Check and parse ACL data if received during init phase to prevent init failures */
      retval = uart_transport_->Read(buf, BT_ACL_HDR_SIZE);
      if (retval < 0) {
        ALOGE("%s: read error", __func__);
        return -1;
      }

      PTR_TO_UINT16(tot_len, &buf[BT_ACL_HDR_LEN_OFFSET]);
      tmp_buf = new (std::nothrow)unsigned char[tot_len + BT_ACL_HDR_SIZE];
      if (tmp_buf == NULL) {
        ALOGE("%s: Heap memory allocation failed ", __func__);
        return -1;
      }
      memcpy(tmp_buf, buf, BT_ACL_HDR_SIZE);
      retval = uart_transport_->Read(tmp_buf + BT_ACL_HDR_SIZE, tot_len);
      if (retval < 0) {
        ALOGE("%s: read error", __func__);
        if (tmp_buf != NULL) {
          delete [] tmp_buf;
          tmp_buf = NULL;
        }
        return -1;
      }

      ALOGD("%s: Number of bytes of ACL data(header + payload) read = %d",
            __func__, tot_len + BT_ACL_HDR_SIZE);

      bt_logger_->ProcessRx(HCI_PACKET_TYPE_ACL_DATA, tmp_buf, tot_len + BT_ACL_HDR_SIZE);
      if (tmp_buf != NULL) {
        delete [] tmp_buf;
        tmp_buf = NULL;
      }

    } else if (protocol_byte == 0xFD) {
      ALOGI("%s: Got FD , responding with FC", __func__);
      uint8_t wake_byte = 0xFC;
      uart_transport_->UartWrite(&wake_byte, 1);
    } else {
      ALOGI("%s: Got an invalid proto byte: %d", __func__, protocol_byte);
    }
  } while (1);

  retval = uart_transport_->Read(hdr, BT_EVT_HDR_SIZE);
  if (retval < 0) {
    ALOGE("%s:error in reading hdr: %d", __func__, retval);
    return -1;
  }
  ALOGV("read scucesssssfully HDR");
  packet_len = hdr[BT_EVT_HDR_LEN_OFFSET];
  ALOGV("packet_len: %d\n", packet_len);

  buf[0] = protocol_byte;
  memcpy(buf + 1, hdr, BT_EVT_HDR_SIZE);
  retval = uart_transport_->Read(buf + BT_EVT_HDR_SIZE + 1, packet_len);
  if (retval < 0) {
    ALOGE("%s:error in reading buf: %d", __func__, retval);
    retval = -1;
    return retval;
  }
  tot_len = packet_len + BT_EVT_HDR_SIZE + 1;
  ALOGV("read scucesssssfully payload: tot_len: %d", tot_len);
  return tot_len;
}

int PatchDLManager::ReadCmdCmplEvent(unsigned char* buf, int size)
{
  int tot_len = -1;
  do {
    tot_len = ReadNewHciEvent(buf, size);
    if (tot_len < 0) {
      ALOGE("%s: Error while reading the hci event", __func__);
      break;
    }
    bt_logger_->ProcessRx(HCI_PACKET_TYPE_EVENT, &buf[1], tot_len - 1);
    if (buf[1] == EVT_CMD_COMPLETE) {
      ALOGD("%s: Cmd Cmpl received for opcode %0x", __func__,
                                (buf[4] | (buf[5] << 8)));
      break;
    } else {
      ALOGE("%s: Unexpected event %0x received", __func__, buf[1]);
    }
  } while (buf[1] != EVT_CMD_COMPLETE);
  return tot_len;
}

int PatchDLManager::ReadHciEvent(unsigned char* buf, int size)
{
  int tot_len;
  unsigned short int opcode;
  tot_len = ReadNewHciEvent(buf, size);
  if (tot_len < 0) {
    ALOGE("%s: Error while reading the hci event", __func__);
    return -1;
  }

  bt_logger_->ProcessRx(HCI_PACKET_TYPE_EVENT, &buf[1], tot_len - 1);
  if (unified_hci &&  (buf[EVENTCODE_OFFSET] == VSEVENT_CODE)) {
    ALOGE("%s: Unexpected event recieved rather than CC", __func__);
    return 0;
  }
  if (buf[1] == LOG_BT_EVT_VENDOR_SPECIFIC) {
    if (buf[3] == LOG_BT_CONTROLLER_LOG) {
      /* if it is RAMDUMP event OR
       * if normal logging event and diag is enabled
       */
      if (buf[4] == LOG_BT_MESSAGE_TYPE_MEM_DUMP) {
        ALOGV("It is RAMDUMP, keep looping to get more RAMDUMP events");
        //HAVE A TERMINATION
        ReadVsHciEvent(buf, size);
      } else {
        ALOGV("%s: It is an LOG packet, just dumped it to DIAG", __func__);
        return 0;
      }
    }
  } else if (buf[1] == EVT_CMD_COMPLETE) {
    ALOGD("%s: Expected CC", __func__);
    if (tot_len > UNIFIED_HCI_CC_MIN_LENGTH) {
      opcode = (buf[4] | (buf[5] << 8));
      if (((HCI_VS_WIPOWER_CMD_OPCODE == opcode) && (UNIFIED_HCI_CODE == buf[6])) ||
          ((HCI_VS_GET_VER_CMD_OPCODE == opcode) && (buf[7] == EDL_PATCH_VER_REQ_CMD))) {
        unified_hci = true;
        ALOGI("HCI Unified command interface supported");
      }
    }
    if (unified_hci) {
      GetVsHciEvent(buf);
     }
  } else {
    ALOGE("%s: Unexpected event : protocol byte: %d", __func__, buf[1]);
    tot_len = -1;
  }
  return tot_len;
}

int PatchDLManager::ReadVsHciEvent(unsigned char* buf, int size)
{
  int tot_len;
  bool collecting_ram_dump = false;
  unsigned short int opcode;
  do {
    tot_len = ReadNewHciEvent(buf, size);
    if (tot_len < 0) {
      ALOGE("%s: Error while reading the hci event", __func__);
      return -1;
    }

    bt_logger_->ProcessRx(HCI_PACKET_TYPE_EVENT, &buf[1], tot_len - 1);

    if (buf[1] == LOG_BT_EVT_VENDOR_SPECIFIC) {
      /* if it is RAMDUMP event OR
       * if normal logging event and diag is enabled
       */
      if (buf[3] == LOG_BT_CONTROLLER_LOG) {
        if (buf[4] == LOG_BT_MESSAGE_TYPE_MEM_DUMP) {
          ALOGV("It is RAMDUMP, keep looping to get more RAMDUMP events");
          //HAVE A TERMINATION
          collecting_ram_dump = true;
        } else {
          ALOGV("%s: It is an LOG packet, just dumped it to DIAG", __func__);
          return 0;
        }
      } else {
        ALOGV("VSC Event! good");
      }
    } else if (buf[1] == EVT_CMD_COMPLETE) {
      ALOGI("%s: Expected CC", __func__);
      if (tot_len > UNIFIED_HCI_CC_MIN_LENGTH) {
        opcode = (buf[4] | (buf[5] << 8));
        if (((HCI_VS_WIPOWER_CMD_OPCODE == opcode) && (UNIFIED_HCI_CODE == buf[6])) ||
            ((HCI_VS_GET_VER_CMD_OPCODE == opcode) && (buf[7] == EDL_PATCH_VER_REQ_CMD))) {
          unified_hci = true;
          ALOGI("HCI Unified command interface supported");
        }
      }
    } else {
      ALOGI("%s: Unexpected event! : opcode: %d", __func__, buf[1]);
      tot_len = -1;
    }

  } while (collecting_ram_dump);

  /* Check if the set patch command is successful or not */
  if (GetVsHciEvent(buf) != HCI_CMD_SUCCESS)
    return -1;

  return tot_len;
}

int PatchDLManager::DisableInternalLdo()
{
  int ret = 0;

  if (IsExtldoEnabled()) {
    unsigned char cmd[5] = { 0x01, 0x0C, 0xFC, 0x01, 0x32 };
    unsigned char rsp[HCI_MAX_EVENT_SIZE];

    ALOGI(" %s ", __func__);
    ret = HciSendVsCmd(cmd, rsp, 5);
    if (ret != 5) {
      ALOGE("%s: Send failed with ret value: %d", __func__, ret);
      ret = -1;
    } else {
      /* Wait for command complete event */
      ret = ReadHciEvent(rsp, HCI_MAX_EVENT_SIZE);
      if ( ret < 0) {
        ALOGE("%s: Failed to get response from controller", __func__);
      }
    }
  }
  return ret;
}

int PatchDLManager::GetVsHciEvent(unsigned char *rsp)
{
  int err = 0;
  unsigned char paramlen = 0;
  unsigned char EMBEDDED_MODE_CHECK = 0x02;
  unsigned int opcode = 0;
  unsigned char subOpcode = 0;
  unsigned int ocf = 0;
  unsigned int ogf = 0;
  unsigned char status = 0;
  uint8_t baudrate_rsp_status_offset = 0;
  uint8_t addon_features_bitmask_offset = 0;
  if ( (rsp[EVENTCODE_OFFSET] == VSEVENT_CODE) || (rsp[EVENTCODE_OFFSET] == EVT_CMD_COMPLETE))
    ALOGV("%s: Received HCI-Vendor Specific event", __func__);
  else {
    ALOGI("%s: Failed to receive HCI-Vendor Specific event", __func__);
    err = -EIO;
    goto failed;
  }

  if (!unified_hci) {
    ocf = rsp[CMD_RSP_OFFSET];
    subOpcode = rsp[RSP_TYPE_OFFSET];
    paramlen = rsp[EVT_PLEN];
    ALOGV("%s: Parameter Length: 0x%x", __func__, paramlen);
    ALOGV("%s: Command response: 0x%x", __func__, ocf);
    ALOGV("%s: Response type   : 0x%x", __func__, subOpcode);
  } else {
    paramlen = rsp[EVT_PLEN];
    opcode = rsp[5]<<8 | rsp[4];
    ocf = opcode & 0x03ff;
    ogf = opcode >> 10;
    status = rsp[6];
    subOpcode = rsp[7];
    ALOGV("%s: Opcode: 0x%x", __func__, opcode);
    ALOGV("%s: ocf: 0x%x", __func__, ocf);
    ALOGV("%s: ogf: 0x%x", __func__, ogf);
    ALOGV("%s: Status: 0x%x", __func__, status);
    ALOGV("%s: Sub-Opcode: 0x%x", __func__, subOpcode);
    ALOGV("%s: Parameter Length: 0x%x", __func__, paramlen);
  }

  /* Check the status of the operation */
  switch ( ocf ) {
    case EDL_CMD_REQ_RES_EVT:
      ALOGV("%s: Command Request Response", __func__);
      HandleEdlCmdResEvt(subOpcode, paramlen, rsp);
      break;
    case NVM_ACCESS_CODE:
      ALOGI("%s: NVM Access Code!!!", __func__);
      err = HCI_CMD_SUCCESS;
      break;
    case EDL_SET_BAUDRATE_RSP_EVT:
      baudrate_rsp_status_offset = BAUDRATE_RSP_STATUS_OFFSET;
      [[fallthrough]];
    /* in case of unified cmd, rsp contains cmd opcode */
    case EDL_SET_BAUDRATE_CMD_OCF:
      /* incase of unified hci we have different offset for baudrate status */
      if (unified_hci)
        baudrate_rsp_status_offset = BAUDRATE_RSP_STATUS_OFFSET_UNIFIED;
      if (rsp[baudrate_rsp_status_offset] != BAUDRATE_CHANGE_SUCCESS) {
        ALOGE("%s: Set Baudrate request failed with status: 0x%x",
              __func__, rsp[baudrate_rsp_status_offset]);
        err = -1;
      }
      break;
    case EDL_WIP_QUERY_CHARGING_STATUS_EVT:
      /* Query charging command has below return values
         0 - in embedded mode not charging
         1 - in embedded mode and charging
         2 - hadofff completed and in normal mode
         3 - no wipower supported on mtp. so irrepective of charging
         handoff command has to be sent if return values are 0 or 1.
         These change include logic to enable generic BT turn on sequence.*/
      if (rsp[4] < EMBEDDED_MODE_CHECK) {
        ALOGI("%s: WiPower Charging in Embedded Mode!!!", __func__);
        wipower_handoff_ready_ = rsp[4];
        wipower_flag_ = 1;
      }
      break;
    case EDL_WIP_START_HANDOFF_TO_HOST_EVENT:
      /*TODO: rsp code 00 mean no charging
         this is going to change in FW soon*/
      if (rsp[4] == NON_WIPOWER_MODE) {
        ALOGE("%s: WiPower Charging hand off not ready!!!", __func__);
      }
      break;
    case HCI_VS_GET_ADDON_FEATURES_EVENT:
      addon_features_bitmask_offset = ADDON_FEATURES_BITMASK_OFFSET;
      [[fallthrough]];
    /* incase of unified cmd, rsp contains cmd opcode */
    case HCI_VS_GET_ADDON_FEATURES_SUPPORT:
      /* incase of unified hci we have different offset for addon feature bitmask */
      if (unified_hci)
        addon_features_bitmask_offset = ADDON_FEATURES_BITMASK_OFFSET_UNIFIED;
      if ((rsp[addon_features_bitmask_offset] & ADDON_FEATURES_EVT_WIPOWER_MASK)) {
        ALOGD("%s: WiPower feature supported!!", __func__);
      }
      break;
    case HCI_VS_STRAY_EVT:
      if (unified_hci && subOpcode == HCI_VS_HOST_LOG_ENH_ENABLE_SUBID) {
        if (status != LOG_ENH_ENABLE_SUCCESS) {
          ALOGE("%s: enable/disable Enhanced Logging cmd failed: 0x%x", __func__, status);
          err = -1;
        }
        break;
      }
      /* WAR to handle stray Power Apply EVT during patch download */
      ALOGD("%s: Stray HCI VS EVENT", __func__);
      if (patch_dnld_pending_ && dnld_fd_in_progress_ != -1) {
        unsigned char rsp[HCI_MAX_EVENT_SIZE];
        memset(rsp, 0x00, HCI_MAX_EVENT_SIZE);
        ReadVsHciEvent(rsp, HCI_MAX_EVENT_SIZE);
      } else {
        ALOGE("%s: Not a valid status!!!", __func__);
        err = -1;
      }
      break;
    default:
      ALOGE("%s: Not a valid status!!!", __func__);
      err = -1;
      break;
  }

 failed:
  return err;
}

void PatchDLManager::HandleEdlCmdResEvt(unsigned char subOpcode, unsigned char paramlen,
  unsigned char* rsp)
{
  int err = 0;
  unsigned int soc_id = 0;
  unsigned int productid = 0;
  unsigned short patchversion = 0;
  unsigned short buildversion = 0;
  unsigned int bld_len_offset = 5;
  char build_label[255];
  int build_lbl_len;
  FILE *btversionfile = 0;
  bool ret  = false;
  int build_lable_prefix;

  switch (subOpcode) {
    case EDL_PATCH_VER_RES_EVT:
    case EDL_APP_VER_RES_EVT:
      if (!unified_hci) {
        productid = (unsigned int)(rsp[PATCH_PROD_ID_OFFSET + 3] << 24 |
                                  rsp[PATCH_PROD_ID_OFFSET + 2] << 16 |
                                  rsp[PATCH_PROD_ID_OFFSET + 1] << 8 |
                                  rsp[PATCH_PROD_ID_OFFSET]  );
        patchversion = (unsigned short)(rsp[PATCH_PATCH_VER_OFFSET + 1] << 8 |
                                       rsp[PATCH_PATCH_VER_OFFSET] );
        buildversion = (int)(rsp[PATCH_ROM_BUILD_VER_OFFSET + 1] << 8 |
                    rsp[PATCH_ROM_BUILD_VER_OFFSET] );

        ALOGI("\t Current Product ID\t\t: 0x%08x", productid);
        ALOGI("\t Current Patch Version\t\t: 0x%04x", patchversion);
        ALOGI("\t Current ROM Build Version\t: 0x%04x", buildversion);

        if (paramlen - 10) {
          soc_id = (unsigned int)(rsp[PATCH_SOC_VER_OFFSET + 3] << 24 |
                                 rsp[PATCH_SOC_VER_OFFSET + 2] << 16 |
                                 rsp[PATCH_SOC_VER_OFFSET + 1] << 8 |
                                 rsp[PATCH_SOC_VER_OFFSET] );
          ALOGI("\t Current SOC Version\t\t: 0x%08x", soc_id);
        }
      } else {
        productid = (unsigned int)(rsp[PATCH_PROD_ID_OFFSET_UNIFIED + 3] << 24 |
                                  rsp[PATCH_PROD_ID_OFFSET_UNIFIED + 2] << 16 |
                                  rsp[PATCH_PROD_ID_OFFSET_UNIFIED + 1] << 8 |
                                  rsp[PATCH_PROD_ID_OFFSET_UNIFIED]  );
        ALOGI("\t unified Current Product ID\t\t: 0x%08x", productid);

        /* Patch Version indicates FW patch version */
        patchversion = (unsigned short)(rsp[PATCH_PATCH_VER_OFFSET_UNIFIED + 1] << 8 |
                                             rsp[PATCH_PATCH_VER_OFFSET_UNIFIED] );
        ALOGI("\t unified Current Patch Version\t\t: 0x%04x", patchversion);

        /* ROM Build Version indicates ROM build version like 1.0/1.1/2.0 */
        buildversion =
              (int)(rsp[PATCH_ROM_BUILD_VER_OFFSET_UNIFIED + 1] << 8 |
                    rsp[PATCH_ROM_BUILD_VER_OFFSET_UNIFIED] );
        ALOGI("\t unified Current ROM Build Version\t: 0x%04x", buildversion);

        if (paramlen - 10) {
          soc_id =
                (unsigned int)(rsp[PATCH_SOC_VER_OFFSET_UNIFIED + 3] << 24 |
                                rsp[PATCH_SOC_VER_OFFSET_UNIFIED + 2] << 16 |
                                rsp[PATCH_SOC_VER_OFFSET_UNIFIED + 1] << 8 |
                                rsp[PATCH_SOC_VER_OFFSET_UNIFIED]  );
          ALOGI("\t unified Current SOC Version\t\t: 0x%08x", soc_id);
        }
      }
      if (NULL != (btversionfile = fopen(BT_VERSION_FILEPATH, "wb"))) {
        fprintf(btversionfile, "Bluetooth Controller Product ID    : 0x%08x\n", productid);
        fprintf(btversionfile, "Bluetooth Controller Patch Version : 0x%04x\n", patchversion);
        fprintf(btversionfile, "Bluetooth Controller Build Version : 0x%04x\n", buildversion);
        fprintf(btversionfile, "Bluetooth Controller SOC Version   : 0x%08x\n", soc_id);
        fclose(btversionfile);
      } else {
        ALOGE("Failed to dump SOC version info. Errno:%d", errno);
      }

      chipset_ver_ = QCA_BT_VER(soc_id, productid, buildversion);
      ret = power_manager_->SendSocVersion(soc_id);
      if (ret != true ) {
         ALOGE ("SOC version ioctl failed");
      }

      break;
    case EDL_TVL_DNLD_RES_EVT:
    case EDL_CMD_EXE_STATUS_EVT:
    /* In case of unified HCI cmd, rsp will contain cmd subopcode in this case 0x1E */
    case EDL_PATCH_TLV_REQ_CMD:
      if (unified_hci)
        err = rsp[CMD_STATUS_OFFSET_UNIFIED];
      else
        err = rsp[CMD_STATUS_OFFSET];
      DownloadAndCmdExeStatus(err);
      break;
    case EDL_GET_BUILD_INFO:
    case HCI_VS_GET_BUILD_VER_EVT:
      if (unified_hci) {
        bld_len_offset += 3;
      }
      build_lbl_len = rsp[bld_len_offset];
      snprintf(build_label, sizeof(build_label), "BT SoC FW SU Build info: ");
      build_lable_prefix = strlen(build_label);
      memcpy(build_label + build_lable_prefix, &rsp[bld_len_offset + 1], build_lbl_len);
      *(build_label + build_lable_prefix + build_lbl_len) = '\0';
      BtState::Get()->SetFwSuBuildInfo(build_label);
      ALOGI("%s: %s, %d", __func__, build_label, build_lbl_len);
      if (NULL != (btversionfile = fopen(BT_VERSION_FILEPATH, "a+b"))) {
        fprintf(btversionfile, "Bluetooth Contoller SU Build info  : %s\n", build_label);
        fclose(btversionfile);
      } else {
        ALOGI("Failed to dump  FW SU build info. Errno:%d", errno);
      }
      break;
    case EDL_BOARD_ID_RESPONSE:
      uint8_t msbBoardId = 0;
      uint8_t lsbBoardId = 0;
      uint8_t boardIdLen = 0;
      board_id_[0] = '\0';

      paramlen = (uint8_t)rsp[EVT_PLEN];
      if (unified_hci) {
        if (paramlen < 8) {
          ALOGE("%s: Invalid Param Len in BoardId rsp:%d!!", __func__, paramlen);
          break;
        }
        boardIdLen = (uint8_t)rsp[8];
        msbBoardId = (uint8_t)rsp[9];
        lsbBoardId = (uint8_t)rsp[10];
      } else {
        if (paramlen < 5) {
          ALOGE("%s: Invalid Param Len in BoardId rsp:%d!!", __func__, paramlen);
          break;
        }
        boardIdLen = (uint8_t)rsp[5];
        msbBoardId = (uint8_t)rsp[6];
        lsbBoardId = (uint8_t)rsp[7];
      }

      if (boardIdLen != 2) {
        ALOGI("%s: Invalid Board Id Len %d!!", __func__, boardIdLen);
        break;
      }

      ALOGI("%s: Board Id %x %x!!", __func__, msbBoardId, lsbBoardId);
      if (msbBoardId == 0x00) {
        board_id_[0] = Convert2Ascii((lsbBoardId & MSB_NIBBLE_MASK) >> 4);
        board_id_[1] = Convert2Ascii(lsbBoardId & LSB_NIBBLE_MASK);
        board_id_[2] = '\0';
      } else {
        board_id_[0] = Convert2Ascii((msbBoardId & MSB_NIBBLE_MASK) >> 4);
        board_id_[1] = Convert2Ascii(msbBoardId & LSB_NIBBLE_MASK);
        board_id_[2] = Convert2Ascii((lsbBoardId & MSB_NIBBLE_MASK) >> 4);
        board_id_[3] = Convert2Ascii(lsbBoardId & LSB_NIBBLE_MASK);
        board_id_[4] = '\0';
      }
      break;
  }
}

void PatchDLManager::DownloadAndCmdExeStatus(int err) {
  switch (err) {
    case HCI_CMD_SUCCESS:
      ALOGV("%s: Download Packet successfully!", __func__);
      break;
    case PATCH_LEN_ERROR:
      ALOGE("%s: Invalid patch length argument passed for EDL PATCH "
                    "SET REQ cmd", __func__);
      break;
    case PATCH_VER_ERROR:
      ALOGE("%s: Invalid patch version argument passed for EDL PATCH "
                    "SET REQ cmd", __func__);
      break;
    case PATCH_CRC_ERROR:
      ALOGE("%s: CRC check of patch failed!!!", __func__);
      break;
    case PATCH_NOT_FOUND:
      ALOGE("%s: Invalid patch data!!!", __func__);
      break;
    case TLV_TYPE_ERROR:
      ALOGE("%s: TLV Type Error !!!", __func__);
      break;
    default:
      ALOGE("%s: Undefined error (0x%x)", __func__, err);
      break;
  }
}

bool PatchDLManager::IsExtldoEnabled()
{
  bool is_extldo = false;

  is_extldo = power_manager_->ExternalldoStatus();
  return is_extldo;
}

int PatchDLManager::HciSendVsCmd(unsigned char *cmd, unsigned char *rsp, int size)
{
  int ret = 0;
  char dst_buff[MAX_BUFF_SIZE] = {'\0'};
  struct timeval tv;

  /* Send the HCI command packet to UART for transmission */
  ret = uart_transport_->UartWrite(cmd, size);
  if (ret != size) {
    ALOGE("%s: Send failed with ret value: %d", __func__, ret);
    goto failed;
  }
  /* checking cmd size, ocf and subopcode */
  if (size == 5 && cmd[1] == 0 && cmd[4] == EDL_PATCH_VER_REQ_CMD) {
    ALOGD("%s: Get Version Cmd sent to SOC", __func__);
    bt_logger_->SetSecondaryCrashReason(SEC_REASON_BTON_GETVER_NO_RSP_RCVD);
    gettimeofday(&tv, NULL);
    BtState :: Get()->AddLogTag(dst_buff, tv,
                               (char *)"Reading Get Version CMD RSP from SOC");
    BtState :: Get()->SetGetVerReqTS(dst_buff, READ_RSP_TS);
  }

  if (wait_vsc_evt_) {
    /* Check for response from the Controller */
    if (!unified_hci) {
     if (ReadVsHciEvent(rsp, HCI_MAX_EVENT_SIZE) < 0) {
      ret = -ETIMEDOUT;
      ALOGI("%s: Failed to get ReadVsHciEvent Event from SOC", __func__);
      goto failed;
     }
      ALOGV("%s: Received HCI-Vendor Specific Event from SOC", __func__);
    }
    else
    {
     if (ReadHciEvent(rsp, HCI_MAX_EVENT_SIZE) < 0) {
      ret = -ETIMEDOUT;
      ALOGI("%s: Failed to get ReadHciEvent Event from SOC", __func__);
      goto failed;
     }
      ALOGV("%s: Received HCI-Vendor Specific Event from SOC", __func__);
    }
  }

 failed:
  return ret;
}

void PatchDLManager::FrameHciPkt(
  unsigned char *cmd,
  int edl_cmd, unsigned int p_base_addr,
  int segtNo, int size
)
{
  int offset = 0;
  hci_command_hdr *cmd_hdr;

  memset(cmd, 0x0, HCI_MAX_CMD_SIZE);

  cmd_hdr = (hci_command_hdr*)(cmd + 1);

  cmd[0]      = HCI_COMMAND_PKT;
  cmd_hdr->opcode = cmd_opcode_pack(HCI_VENDOR_CMD_OGF, HCI_PATCH_CMD_OCF);
  cmd_hdr->plen   = size;
  cmd[4]      = edl_cmd;

  switch (edl_cmd) {
    case EDL_PATCH_SET_REQ_CMD:
      /* Copy the patch header info as CMD params */
      memcpy(&cmd[5], phdr_buffer_, PATCH_HDR_LEN);
      ALOGV("%s: Sending EDL_PATCH_SET_REQ_CMD", __func__);
      ALOGV("HCI-CMD %d:\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x",
            segtNo, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
      break;
    case EDL_PATCH_DLD_REQ_CMD:
      offset = ((segtNo - 1) * MAX_DATA_PER_SEGMENT);
      p_base_addr += offset;
      cmd_hdr->plen   = (size + 6);
      cmd[5]  = (size + 4);
      cmd[6]  = EXTRACT_BYTE(p_base_addr, 0);
      cmd[7]  = EXTRACT_BYTE(p_base_addr, 1);
      cmd[8]  = EXTRACT_BYTE(p_base_addr, 2);
      cmd[9]  = EXTRACT_BYTE(p_base_addr, 3);
      memcpy(&cmd[10], (pdata_buffer_ + offset), size);

      ALOGV("%s: Sending EDL_PATCH_DLD_REQ_CMD: size: %d bytes",
            __func__, size);
      ALOGV("HCI-CMD %d:\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t"
            "0x%x\t0x%x\t0x%x\t\n", segtNo, cmd[0], cmd[1], cmd[2],
            cmd[3], cmd[4], cmd[5], cmd[6], cmd[7], cmd[8], cmd[9]);
      break;
    case EDL_PATCH_ATCH_REQ_CMD:
      ALOGV("%s: Sending EDL_PATCH_ATTACH_REQ_CMD", __func__);
      ALOGV("HCI-CMD %d:\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x",
            segtNo, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
      break;
    case EDL_PATCH_RST_REQ_CMD:
      ALOGV("%s: Sending EDL_PATCH_RESET_REQ_CMD", __func__);
      ALOGV("HCI-CMD %d:\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x",
            segtNo, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
      break;
    case EDL_PATCH_VER_REQ_CMD:
      ALOGV("%s: Sending EDL_PATCH_VER_REQ_CMD", __func__);
      ALOGV("HCI-CMD %d:\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x",
            segtNo, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
      break;
    case EDL_PATCH_TLV_REQ_CMD:
      ALOGV("%s: Sending EDL_PATCH_TLV_REQ_CMD", __func__);
      /* Parameter Total Length */
      cmd[3] = size + 2;

      /* TLV Segment Length */
      cmd[5] = size;
      ALOGV("HCI-CMD %d:\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x",
            segtNo, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);
      offset = (segtNo * MAX_SIZE_PER_TLV_SEGMENT);
      memcpy(&cmd[6], (pdata_buffer_ + offset), size);
      break;
    case EDL_GET_BUILD_INFO:
      ALOGV("%s: Sending EDL_GET_BUILD_INFO", __func__);
      ALOGV("HCI-CMD %d:\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x",
            segtNo, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
      break;
    case EDL_GET_BOARD_ID:
      ALOGV("%s: Sending EDL_GET_BOARD_ID", __func__);
      ALOGV("HCI-CMD %d:\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x",
            segtNo, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
      break;
    default:
      ALOGE("%s: Unknown EDL CMD !!!", __func__);
  }
}

unsigned char PatchDLManager::IsSibsEnabled()
{
  /* Check whether IBS is enabled through the macro 'WCNSS_IBS_ENABLED'
     and the property 'persist.vendor.service.bdroid.sibs'. */
#ifdef WCNSS_IBS_ENABLED
  return IbsHandler::IsEnabled();
#else
  return false;
#endif
}

unsigned char PatchDLManager::Convert2Ascii(unsigned char temp)
{
  unsigned char n = temp;

  if (n <= 9)
    n = n + 0x30;
  else
    n = n + 0x57;
  return n;
}

void PatchDLManager::UpdateNewNvmFormat(tlv_nvm_hdr *nvm)
{
  uint8_t *nvm_byte_ptr = (uint8_t *)nvm;

  if (!nvm)
    return;

  nvm_byte_ptr += sizeof(tlv_nvm_hdr);

  /* Update Tag#17: HCI UART Settings */
  if (nvm->tag_id == TAG_NUM_17) {
    uint8_t baudrate = uart_transport_->GetMaxBaudrate();

    ALOGI("%s: baudrate %02x", __func__, baudrate);

    /* Byte#1: UART baudrate */
    *(nvm_byte_ptr + 1) = baudrate;
  }

  /* Update Tag#27: SIBS Settings */
  if (nvm->tag_id == 27) {
    if (!IsSibsEnabled()) {
      /* TxP Sleep Mode: Disable */
      *(nvm_byte_ptr + 1) &= ~0x01;
      ALOGI("%s: SIBS Disable", __func__);
    } else {
      /* TxP Sleep Mode-1:UART_SIBS, 2:USB_SUSPEND, 3: GPIO_OOB, 4: UART_HWIBS */
      *(nvm_byte_ptr + 1) |= 0x01;
      ALOGI("%s: SIBS Enable", __func__);
    }
  }
}

#ifdef DEBUG_CRASH_SOC_WHILE_DLD
int PatchDLManager::SendCrashCommand()
{
  int ret = 0;
  unsigned char cmd[5] = { 0x01, 0x0C, 0xFC, 0x01, 0x28 };
  unsigned char rsp[HCI_MAX_EVENT_SIZE];

  ALOGI(" %s ", __func__);
  ret = uart_transport_->UartWrite(cmd, 5);
  if (ret != 5) {
    ALOGE("%s: Send failed with ret value: %d", __func__, ret);
    ret = -1;
  } else {
    /* Wait for command complete event */

    ret = ReadHciEvent(rsp, HCI_MAX_EVENT_SIZE);
    if ( ret < 0) {
      ALOGE("%s: Failed to get response from controller", __func__);
    }
  }
  return ret;
}
#endif    //DEBUG_CRASH_SOC_WHILE_DLD

void PatchDLManager::LoadPatchMaptable() {
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(ROME_VER_2_1,
                          PatchPathManager("ROME2_1", "rampatch_tlv_2.1.tlv", "nvm_tlv_2.1.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(ROME_VER_3_0,
                          PatchPathManager("ROME3_0", "btfw30.tlv", "btnv30.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(ROME_VER_3_2,
                          PatchPathManager("ROME3_2", "btfw32.tlv", "btnv32.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(CHEROKEE_VER_2_0,
                          PatchPathManager("CHE2_0", "crbtfw20.tlv", "crnv20.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(CHEROKEE_VER_2_0_1,
                          PatchPathManager("CHE2_0_1", "crbtfw20.tlv", "crnv20.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(CHEROKEE_VER_2_1,
                          PatchPathManager("CHE2_1", "crbtfw21.tlv", "crnv21.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(CHEROKEE_VER_2_1_1,
                          PatchPathManager("CHE2_1_1", "crbtfw21.tlv", "crnv21.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(CHEROKEE_VER_3_1,
                          PatchPathManager("CHE3_1", "crbtfw30.tlv", "crnv30.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(CHEROKEE_VER_3_2,
                          PatchPathManager("CHE3_2", "crbtfw32.tlv", "crnv32.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(CHEROKEE_VER_3_2_UMC,
                          PatchPathManager("CHE3_2", "crbtfw32.tlv", "crnv32u.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(APACHE_VER_1_0_0,
                          PatchPathManager("APA1_0_0", "apbtfw10.tlv", "apnv10.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(APACHE_VER_1_1_0,
                          PatchPathManager("APA1_1_0", "apbtfw11.tlv", "apnv11.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(APACHE_VER_1_2_0,
                          PatchPathManager("APA1_2_0", "apbtfw11.tlv", "apnv11.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(APACHE_VER_1_2_1,
                          PatchPathManager("APA1_2_1", "apbtfw11.tlv", "apnv11.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(COMANCHE_VER_1_0_1,
                          PatchPathManager("COM1_0_1", "cmbtfw10.tlv", "cmnv10.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(COMANCHE_VER_1_1,
                          PatchPathManager("COM1_1", "cmbtfw11.tlv", "cmnv11.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(COMANCHE_VER_1_2,
                          PatchPathManager("COM1_2", "cmbtfw12.tlv", "cmnv12.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(HASTINGS_VER_2_0,
                          PatchPathManager("HST2_0", "htbtfw20.tlv", "htnv20.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(GENOA_VER_1_0,
                          PatchPathManager("GEN1_0", "gnbtfw10.tlv", "gnnv10.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(GENOA_VER_2_0,
                          PatchPathManager("GEN2_0", "gnbtfw20.tlv", "gnnv20.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(GENOA_VER_2_0_0_2,
                          PatchPathManager("GEN2_0", "gnbtfw20.tlv", "gnnv20.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(HSP_VER_1_0,
                          PatchPathManager("HSP1_0", "hpbtfw10.tlv", "hpnv10.bin")));
  PatchPathInfoMap_.insert(std::make_pair<uint64_t, PatchPathManager>(HSP_VER_2_0,
                          PatchPathManager("HSP2_0", "hpbtfw20.tlv", "hpnv20.bin")));
}
} // namespace implementation
} // namespace V1_0
} // namespace bluetooth
} // namespace hardware
} // namespace android
