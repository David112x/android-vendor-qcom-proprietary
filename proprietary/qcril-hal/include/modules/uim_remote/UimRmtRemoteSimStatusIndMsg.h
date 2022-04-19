/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_TransportType
   DESCRIPTION:
     Used to convey respective transport type to the modem
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_RMT_UICC_REMOTE_TRANSPORT_OTHER = 0,
  RIL_UIM_RMT_UICC_REMOTE_TRANSPORT_BLUETOOTH,
  RIL_UIM_RMT_UICC_REMOTE_TRANSPORT_IP
} RIL_UIM_RMT_TransportType;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_Usage
   DESCRIPTION:
     Used to convey respective usage to the modem
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_RMT_UICC_REMOTE_USAGE_REDUCED = 0,
  RIL_UIM_RMT_UICC_REMOTE_USAGE_NORMAL,
} RIL_UIM_RMT_Usage;

/* -----------------------------------------------------------------------------
   ENUM:      RIL_UIM_remoteEvent
   DESCRIPTION:
     Used to convey respective event info to the modem
-------------------------------------------------------------------------------*/
typedef enum {
  RIL_UIM_RMT_UICC_REMOTE_CONNECTION_UNAVAILABLE = 0,
  RIL_UIM_RMT_UICC_REMOTE_CONNECTION_AVAILABLE,
  RIL_UIM_RMT_UICC_REMOTE_CARD_INSERTED,
  RIL_UIM_RMT_UICC_REMOTE_CARD_REMOVED,
  RIL_UIM_RMT_UICC_REMOTE_CARD_ERROR,
  RIL_UIM_RMT_UICC_REMOTE_CARD_RESET,
  RIL_UIM_RMT_UICC_REMOTE_CARD_WAKEUP
} RIL_UIM_RMT_remoteEvent;

/* -----------------------------------------------------------------------------
   STRUCT:     RIL_UIM_RemoteSimInfo

   DESCRIPTION:
     Struct to get Remote Sim info
-------------------------------------------------------------------------------*/
typedef struct {
  bool                          isEnable;
  RIL_UIM_RMT_TransportType     transportType;
  RIL_UIM_RMT_Usage             usage;
  RIL_UIM_RMT_remoteEvent       remoteEvent;
} RIL_UIM_RMT_RemoteSIMInfo;

class UimRmtRemoteSimStatusIndMsg : public UnSolicitedMessage,
                                    public add_message_id<UimRmtRemoteSimStatusIndMsg>
{
  private:
    RIL_UIM_RMT_RemoteSIMInfo  mInfo;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.uim_remote.client.remote_sim_status_indication";
    UimRmtRemoteSimStatusIndMsg() = delete;
    inline ~UimRmtRemoteSimStatusIndMsg() {}

    inline UimRmtRemoteSimStatusIndMsg(RIL_UIM_RMT_RemoteSIMInfo  &info):
                                       UnSolicitedMessage(get_class_message_id())
    {
      mInfo = info;
      mName = MESSAGE_NAME;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UnSolicitedMessage>(std::make_shared<UimRmtRemoteSimStatusIndMsg>(mInfo));
    }

    inline string dump()
    {
      return mName;
    }

    inline RIL_UIM_RMT_RemoteSIMInfo &get_status(void)
    {
      return mInfo;
    }
};
