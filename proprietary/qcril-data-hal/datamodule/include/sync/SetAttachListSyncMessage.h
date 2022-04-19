/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <list>
#include "framework/SolicitedSyncMessage.h"
#include "framework/Util.h"
#include "framework/add_message_id.h"
#include "framework/Dispatcher.h"

class SetAttachListSyncMessage : public SolicitedSyncMessage<int>,
                                    public add_message_id<SetAttachListSyncMessage>
{
public:
  enum AttachListAction
  {
    NONE,
    DISCONNECT_ATTACH_APN_ONLY,
    DETACH
  };
  static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.SET_ATTACH_LIST_SYNC";
  inline SetAttachListSyncMessage(GenericCallback<int> *callback) :
    SolicitedSyncMessage<int>(get_class_message_id())
  {
    std::ignore=callback;
    mName = MESSAGE_NAME;
  }
  ~SetAttachListSyncMessage()
  {

  }

  void setParams(const std::list<uint16_t>& attach_list,
    const AttachListAction action)
  {
    m_attach_list = attach_list;
    m_action = action;
  }

  void getParams(std::list<uint16_t>& attach_list,
                 AttachListAction &action)
  {
    attach_list = m_attach_list;
    action = m_action;
  }

  string dump()
  {
    return mName;
  }
private:
  std::list<uint16_t> m_attach_list;
  AttachListAction m_action;
};