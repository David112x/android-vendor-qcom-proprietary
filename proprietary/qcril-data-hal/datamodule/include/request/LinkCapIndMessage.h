/**
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once

#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "DataCommon.h"

namespace rildata {

typedef struct {
    /**
     * Estimated downlink capacity in kbps.
     */
    uint32_t downlinkCapacityKbps;
    /**
     * Estimated uplink capacity in kbps.
     */
    uint32_t uplinkCapacityKbps;
}LinkCapEstimate_t;

/**
 * @brief      Class for link capacity estimates. The client can register
 *             for this to receive notifications regarding downlink and
 *             uplink capacity once the provided criteria is met.
 */
class LinkCapIndMessage : public UnSolicitedMessage,
                                     public add_message_id<LinkCapIndMessage> {
    private:
      LinkCapEstimate_t params;

    public:

      static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.LinkCapIndMessage";

      inline LinkCapIndMessage(const LinkCapEstimate_t &p):UnSolicitedMessage(get_class_message_id()), params(p) {
        mName = MESSAGE_NAME;
      }
      ~LinkCapIndMessage() {}

      std::shared_ptr<UnSolicitedMessage> clone() {
            return (std::make_shared<LinkCapIndMessage>(params));
      }

      string dump() {
        return ("[" + mName + "]");
      }

      LinkCapEstimate_t &getParams() {
        return params;
      }
};

} //namespace
