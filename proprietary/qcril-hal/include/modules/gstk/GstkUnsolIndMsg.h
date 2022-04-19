/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

/* GSTK UNSOL type */
typedef enum
{
  RIL_GSTK_UNSOL_PROACTIVE_COMMAND  = 0,
  RIL_GSTK_UNSOL_EVENT_NOTIFY       = 1,
  RIL_GSTK_UNSOL_STK_CALL_SETUP     = 2,
  RIL_GSTK_UNSOL_SESSION_END        = 3
} RIL_GSTK_UnsolType;

/* GSTK UNSOL struct */
typedef struct
{
  RIL_GSTK_UnsolType type;
  string             cmd;
  int64_t            setupCallTimeout;
} RIL_GSTK_UnsolData;

class GstkUnsolIndMsg : public UnSolicitedMessage,
                        public add_message_id<GstkUnsolIndMsg>
{
  private:
    RIL_GSTK_UnsolData  mCmd;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.gstk.unsol_request";
    GstkUnsolIndMsg() = delete;
    ~GstkUnsolIndMsg() = default;

    inline GstkUnsolIndMsg(const RIL_GSTK_UnsolData  &unsol_cmd):
                       UnSolicitedMessage(get_class_message_id())
    {
      mCmd = unsol_cmd;
      mName = MESSAGE_NAME;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UnSolicitedMessage>(std::make_shared<GstkUnsolIndMsg>(mCmd));
    }

    inline string dump()
    {
      return mName + "{}" + "Unsol type: " + std::to_string(mCmd.type);
    }

    inline  const RIL_GSTK_UnsolData &get_unsol_data(void)
    {
      return mCmd;
    }
};
