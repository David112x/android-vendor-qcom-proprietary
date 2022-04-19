/**
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once

#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"

namespace rildata {

enum ReportFilter_t {
  disabled,
  enabled
};

/**
 * @brief      Support for the Link Capacity enstimation feature.
 *             This class is used to enable/disable reporting.
 */
class SetLinkCapFilterMessage : public SolicitedMessage<int>,
                                     public add_message_id<SetLinkCapFilterMessage> {
    private:
      RIL_Token t;
      ReportFilter_t params;

    public:

      static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.RilSetLinkCapFilter";

      inline SetLinkCapFilterMessage(RIL_Token tok, const ReportFilter_t &r):SolicitedMessage<int>(get_class_message_id()),
        t(tok), params(r) {
        mName = MESSAGE_NAME;
      }
      ~SetLinkCapFilterMessage() {
        if (mCallback) {
           delete mCallback;
           mCallback = nullptr;
        }
      }

      string dump() {
        return "[" + mName + "]";
      }

      ReportFilter_t &getParams() {
        return params;
      }

      RIL_Token getToken() {
        return t;
      }
};

} //namespace
