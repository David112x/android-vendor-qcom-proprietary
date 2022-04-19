/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef QMI_RIL_UTF

#include "RadioConfigUtil.h"

using android::hardware::radio::V1_0::CardState;
using android::hardware::radio::config::V1_0::SlotState;

/*===========================================================================

FUNCTION:  radio_config_iccid_to_string

===========================================================================*/
static char * radio_config_iccid_to_string
(
  const uint8_t * iccid_ptr,
  uint16_t        iccid_len
)
{
  char * iccid_str = nullptr;

  if (iccid_ptr == NULL || iccid_len == 0)
  {
    return iccid_str;
  }

  iccid_str = new char[2 * iccid_len + 1]{};

  if (iccid_str == nullptr)
  {
    return iccid_str;
  }

  for (uint32_t i = 0, j = 0; i < iccid_len && j < (2 * iccid_len); i++, j++)
  {
    uint8_t ch = iccid_ptr[i] & 0x0F;

    if (ch > 9)
    {
      iccid_str[j] = ch + 'A' - 0x0A;
    }
    else
    {
      iccid_str[j] = ch + '0';
    }

    j++;
    if (j >= (2 * iccid_len))
    {
      break;
    }

    ch = iccid_ptr[i] >> 4;
    if (ch > 9)
    {
      iccid_str[j] = ch + 'A' - 0x0A;
    }
    else
    {
      iccid_str[j] = ch + '0';
    }
  }

  return iccid_str;
} /* radio_config_iccid_to_string */


/*===========================================================================

  FUNCTION:  convertUimSlotStatusToHal

===========================================================================*/
/*!
    @brief
    Converts UIM slot status info to HIDL Slot Status info

    @return
    None
*/
/*=========================================================================*/
void convertUimSlotStatusToHal
(
  RIL_UIM_SlotStatus &ril_slot_status,
  SimSlotStatus      &slot_status
)
{
  switch(ril_slot_status.card_state)
  {
    case RIL_UIM_CARDSTATE_ABSENT:
      slot_status.cardState = CardState::ABSENT;
      break;

    case RIL_UIM_CARDSTATE_PRESENT:
      slot_status.cardState = CardState::PRESENT;
      break;

    default:
      slot_status.cardState = CardState::ERROR;
      break;
  }

  switch(ril_slot_status.slot_state)
  {
    case RIL_UIM_PHYSICAL_SLOT_STATE_ACTIVE:
      slot_status.slotState = SlotState::ACTIVE;
      break;

    default:
      slot_status.slotState = SlotState::INACTIVE;
      break;
  }

  slot_status.logicalSlotId = ril_slot_status.logical_slot;

  if (slot_status.cardState == CardState::PRESENT)
  {
    char * iccid = nullptr;
    char * atr   = nullptr;

    iccid = radio_config_iccid_to_string(ril_slot_status.iccid.data(),
                                         ril_slot_status.iccid.size());

    atr = radio_config_bin_to_hexstring(ril_slot_status.atr.data(),
                                        ril_slot_status.atr.size());

    if (atr != nullptr)
    {
      hidl_string hidl_atr = {};

      hidl_atr.setToExternal(atr, strlen(atr));
      slot_status.atr = hidl_atr;

      delete[] atr;
    }

    if (iccid != nullptr)
    {
      hidl_string hidl_iccid = {};

      hidl_iccid.setToExternal(iccid, strlen(iccid));
      slot_status.iccid = hidl_iccid;

      delete[] iccid;
    }
  }

} /* convertUimSlotStatusToHal */


/*===========================================================================

  FUNCTION:  radio_config_bin_to_hexstring

===========================================================================*/
/*!
    @brief
    Generates a hex string from given binary data

    @return
    Pointer to the generated hex string

    @Note
    Caller must free the memory allocated to store the hex string.
    Since "new" is used for allocation, free using "delete"
*/
/*=========================================================================*/
char * radio_config_bin_to_hexstring
(
  const uint8_t * data_ptr,
  uint16_t        data_len
)
{
  char*     out_ptr    = NULL;
  uint32_t  string_len = 0;

  if (data_ptr == NULL || data_len == 0)
  {
    return NULL;
  }

  string_len = (2 * data_len) + sizeof(char);

  out_ptr = new char[string_len];

  if (out_ptr != NULL)
  {
    uint16_t i = 0;
  
    for (i = 0; i < data_len; i++)
    {
      if (((data_ptr[i] >> 4) & 0x0F) < 0x0a)
      {
        out_ptr[i * 2] = ((data_ptr[i] >> 4) & 0x0F) + '0';
      }
      else
      {
        out_ptr[i * 2] = ((data_ptr[i] >> 4) & 0x0F) + 'a' - 10;
      }
  
      if ((data_ptr[i] & 0x0F) < 0x0a)
      {
        out_ptr[i * 2 + 1] = (data_ptr[i] & 0x0F) + '0';
      }
      else
      {
        out_ptr[i * 2 + 1] = (data_ptr[i] & 0x0F) + 'a' - 10;
      }
    }
    out_ptr[data_len * 2] = 0x0;
  }

  return out_ptr;
} /* radio_config_bin_to_hexstring */

#endif
