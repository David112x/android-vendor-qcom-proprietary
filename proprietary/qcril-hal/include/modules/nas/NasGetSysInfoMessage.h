/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/SolicitedSyncMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "framework/Util.h"
#include "network_access_service_v01.h"

class NasGetSysInfoMessage : public SolicitedSyncMessage<nas_get_sys_info_resp_msg_v01>,
                                    public add_message_id<NasGetSysInfoMessage>
{
public:
  static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.NAS_GET_SYS_INFO";
  inline NasGetSysInfoMessage(GenericCallback<nas_get_sys_info_resp_msg_v01> *callback):
             SolicitedSyncMessage<nas_get_sys_info_resp_msg_v01>(get_class_message_id())
  {
    ignore(callback);
    mName = MESSAGE_NAME;
  }
  ~NasGetSysInfoMessage();
  string dump();
};