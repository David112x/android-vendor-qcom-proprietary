/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

const uint8_t MAX_UIM_RMT_CLIENT_APPS      = 2;

/* UIM REMOTE CARD APP type */
typedef enum
{
  UIM_RMT_APP_UNKNOWN                       = 0,
  UIM_RMT_APP_SIM                           = 1,
  UIM_RMT_APP_USIM                          = 2,
  UIM_RMT_APP_RUIM                          = 3,
  UIM_RMT_APP_CSIM                          = 4
} UimRmtCardAppType;

/* UIM REMOTE CARD APP Init State type */
typedef enum
{
  UIM_RMT_APP_STATE_UNKNOWN               = 0,
  UIM_RMT_APP_STATE_DETECTED              = 1,
  UIM_RMT_APP_STATE_WAITING_ON_USER       = 2,
  UIM_RMT_APP_STATE_HALTED                = 3,
  UIM_RMT_APP_STATE_READY                 = 4
} UimRmtCardAppInitStateType;

typedef struct
{
  UimRmtCardAppType                appType;
  UimRmtCardAppInitStateType       appState;
}UimRmtCardAppInfo;

/* UIM REMOTE CARD APP Status UNSOL struct */
typedef struct
{
  uint8_t                          numOfActiveSlots;
  uint8_t                          numOfApps;
  UimRmtCardAppInfo                appInfo[MAX_UIM_RMT_CLIENT_APPS];
} UimRmtCardAppInitStatusIndData;

class UimRmtCardAppInitStatusIndMsg : public UnSolicitedMessage,
                                    public add_message_id<UimRmtCardAppInitStatusIndMsg>
{
  private:
    UimRmtCardAppInitStatusIndData  mInfo;

  public:
    static constexpr const char *MESSAGE_NAME =
         "com.qualcomm.qti.uim_remote.client.uim_remote_card_app_init_status_indication";

    UimRmtCardAppInitStatusIndMsg() = delete;
    inline ~UimRmtCardAppInitStatusIndMsg() {}

    inline UimRmtCardAppInitStatusIndMsg(const UimRmtCardAppInitStatusIndData  &info):
                                       UnSolicitedMessage(get_class_message_id())
    {
      mInfo = info;
      mName = MESSAGE_NAME;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UnSolicitedMessage>(
                              std::make_shared<UimRmtCardAppInitStatusIndMsg>(mInfo));
    }

    inline string dump()
    {
      return mName;
    }

    inline const UimRmtCardAppInitStatusIndData*  getCardAppInitStatusIndData(void)
    {
      return &mInfo;
    }
};
