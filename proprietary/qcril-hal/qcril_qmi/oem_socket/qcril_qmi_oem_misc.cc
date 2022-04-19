/******************************************************************************
#  Copyright (c) 2015, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************
  @file    qcril_qmi_oem_misc.c
  @brief   qcril oem proto - misc

  DESCRIPTION
    Utility functions for qcril oem proto.

******************************************************************************/

#include "qcril_qmi_oem_misc.h"
#include "qcril_log.h"
#include "qcrili.h"
#include "qcril_memory_management.h"

//===========================================================================
// qcril_qmi_oem_proto_map_event_to_request
//===========================================================================
QtiTelephonyService_RILOEMMessageId qcril_qmi_oem_proto_map_event_to_request(qcril_evt_e_type event)
{
    QtiTelephonyService_RILOEMMessageId ret = static_cast<QtiTelephonyService_RILOEMMessageId>(0);

    QCRIL_LOG_INFO("event %d mapped to uim_msg %d", event, ret);
    return ret;
} //qcril_qmi_oem_proto_map_event_to_request

/*===========================================================================
  FUNCTION  qcril_qmi_oem_convert_proto_token_to_ril_token
===========================================================================*/
/*!
    @brief Convert proto token to RIL token
*/
/*=========================================================================*/
RIL_Token qcril_qmi_oem_convert_proto_token_to_ril_token(uint32_t oem_token)
{
  RIL_Token ret = qcril_malloc(sizeof(uint32_t));
  if (NULL != ret)
  {
    uint32_t *tmp = (uint32_t*) ret;
    *tmp = oem_token ^ 0xd0000000;
  }
  return ret;
} // qcril_qmi_oem_convert_proto_token_to_ril_token

/*===========================================================================
  FUNCTION  qcril_qmi_oem_free_and_convert_ril_token_to_proto_token
===========================================================================*/
/*!
    @brief get proto token from RIL token and free RIL token
*/
/*=========================================================================*/
uint32_t qcril_qmi_oem_free_and_convert_ril_token_to_proto_token(RIL_Token ril_token)
{
  uint32_t ret = 0xFFFFFFFF;
  if (ril_token)
  {
      ret = (*((uint32_t *) ril_token)) ^ 0xd0000000;
      QCRIL_LOG_INFO("proto token: %d", ret);
      qcril_free((void*) ril_token);
  }
  else
  {
      QCRIL_LOG_ERROR("ril_token is NULL");
  }

  return ret;
} // qcril_qmi_oem_free_and_convert_ril_token_to_proto_token

