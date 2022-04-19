/******************************************************************************
#  Copyright (c) 2015, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************
  @file    qcril_qmi_oem_msg_meta.c
  @brief   qcril - OEM HOOK message meta info

  DESCRIPTION
    Handles OEM HOOK message meta info related functions.

******************************************************************************/

#include <vector>
#include <framework/Log.h>
#include "qcril_qmi_oem_msg_meta.h"

#define TAG "OEM_MSG"

typedef struct
{
    QtiTelephonyService_RILOEMMessageId msg_id;
    QtiTelephonyService_RILOEMMessageType msg_type;
    const pb_field_t* msg_pb_fields;
    size_t msg_req_size;
    qcril_evt_e_type msg_event;
    const char* msg_log_str;
} qcril_qmi_oem_proto_msg_meta_type;

static std::vector<qcril_qmi_oem_proto_msg_meta_type> qcril_qmi_oem_proto_msg_meta_data;

void qcril_qmi_oem_init_msg_meta()
{
}

size_t qcril_qmi_oem_proto_get_msg_size(QtiTelephonyService_RILOEMMessageId msg_id, QtiTelephonyService_RILOEMMessageType msg_type)
{
    size_t i;
    for (i=0; i<qcril_qmi_oem_proto_msg_meta_data.size(); i++)
    {
        if ( qcril_qmi_oem_proto_msg_meta_data[i].msg_id == msg_id &&
             qcril_qmi_oem_proto_msg_meta_data[i].msg_type == msg_type )
        {
            return qcril_qmi_oem_proto_msg_meta_data[i].msg_req_size;
        }
    }
    return -1;
}

const pb_field_t* qcril_qmi_oem_proto_get_msg_pb_fields(QtiTelephonyService_RILOEMMessageId msg_id, QtiTelephonyService_RILOEMMessageType msg_type)
{
    size_t i;
    for (i=0; i<qcril_qmi_oem_proto_msg_meta_data.size(); i++)
    {
        if ( qcril_qmi_oem_proto_msg_meta_data[i].msg_id == msg_id &&
             qcril_qmi_oem_proto_msg_meta_data[i].msg_type == msg_type )
        {
            return qcril_qmi_oem_proto_msg_meta_data[i].msg_pb_fields;
        }
    }
    return NULL;
}

const char* qcril_qmi_oem_proto_get_msg_log_str(QtiTelephonyService_RILOEMMessageId msg_id, QtiTelephonyService_RILOEMMessageType msg_type)
{
    size_t i;
    for (i=0; i<qcril_qmi_oem_proto_msg_meta_data.size(); i++)
    {
        if ( qcril_qmi_oem_proto_msg_meta_data[i].msg_id == msg_id &&
             qcril_qmi_oem_proto_msg_meta_data[i].msg_type == msg_type )
        {
            if (qcril_qmi_oem_proto_msg_meta_data[i].msg_log_str)
            {
                return qcril_qmi_oem_proto_msg_meta_data[i].msg_log_str;
            }
            else
            {
                break;
            }
        }
    }
    return "Unknown msg";
}

qcril_evt_e_type qcril_qmi_oem_proto_get_msg_event(QtiTelephonyService_RILOEMMessageId msg_id, QtiTelephonyService_RILOEMMessageType msg_type)
{
    size_t i;
    for (i=0; i<qcril_qmi_oem_proto_msg_meta_data.size(); i++)
    {
        if (qcril_qmi_oem_proto_msg_meta_data[i].msg_id == msg_id)
        {
            return qcril_qmi_oem_proto_msg_meta_data[i].msg_event;
        }
    }
    QCRIL_LOG_ERROR("did not find the corresponding event for %s(msg id %d, type %d)", qcril_qmi_oem_proto_get_msg_log_str(msg_id, msg_type), msg_id, msg_type);
    return QCRIL_EVT_NONE;
}

boolean qcril_qmi_oem_proto_is_msg_valid(QtiTelephonyService_RILOEMMessageId msg_id, QtiTelephonyService_RILOEMMessageType msg_type)
{
    size_t i;
    for (i=0; i<qcril_qmi_oem_proto_msg_meta_data.size(); i++)
    {
        if (qcril_qmi_oem_proto_msg_meta_data[i].msg_id == msg_id &&
             qcril_qmi_oem_proto_msg_meta_data[i].msg_type == msg_type)
        {
            return TRUE;
        }
    }
    QCRIL_LOG_ERROR("did not find a valid entry in the meta table for %s(msg id %d, type %d)", qcril_qmi_oem_proto_get_msg_log_str(msg_id, msg_type), msg_id, msg_type);
    return FALSE;
}
