/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef QMI_RIL_UTF
#pragma once

#include "modules/uim/qcril_uim_types.h"
#include <android/hardware/radio/config/1.0/types.h>
#include <hidl/HidlTransportSupport.h>

using android::hardware::radio::config::V1_0::SimSlotStatus;
using android::hardware::hidl_string;

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
);

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
);

#endif
