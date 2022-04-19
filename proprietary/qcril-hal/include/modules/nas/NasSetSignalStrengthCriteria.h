/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include <framework/GenericCallback.h>
#include <framework/SolicitedMessage.h>
#include <framework/Message.h>
#include <framework/add_message_id.h>
#include <framework/legacy.h>
#include <telephony/ril.h>

/******************************************************************************
#   Message Class: NasSetSignalStrengthCriteria
#   Sender: ConfigurationClients
#   Receiver: NasModule
#   Data transferred: A list of SignalStrengthCriteriaEntry
#   Usage: Client sends this message when wanting to set the reporting criteria
           for the signal strength reporting.
#******************************************************************************/

static inline std::string toString(RIL_RadioAccessNetworks ran) {
    std::string ret = "<Unknown>";
    switch(ran) {
        case GERAN:
            ret = "GERAN";
            break;
        case UTRAN:
            ret = "UTRAN";
            break;
        case EUTRAN:
            ret = "EUTRAN";
            break;
        case CDMA:
            ret = "CDMA";
            break;
        case IWLAN:
            ret = "IWLAN";
            break;
        default:
            break;
    }
    return ret;
}

struct SignalStrengthCriteriaEntry
{
    RIL_RadioAccessNetworks ran;

    /* NOTE: Only one of delta or thresholds can be used at a time.
     * If thresholds is empty, delta will be used. Otherwise,
     * delta will be ignored. If both delta, threshold not present
     * use implementation defined values.
     */
    int32_t delta; // RSSI delta in units of 0.1 dBm
    std::vector<int32_t> thresholds;

};

class NasSetSignalStrengthCriteria : public SolicitedMessage<RIL_Errno>,
    public add_message_id<NasSetSignalStrengthCriteria>
{
private:
    std::vector<SignalStrengthCriteriaEntry> mCriteria;

public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.nas.set_signal_strength_criteria";
    static constexpr int  RSSI_DELTA = 50;

    NasSetSignalStrengthCriteria() = delete;
    inline ~NasSetSignalStrengthCriteria() {};

    inline NasSetSignalStrengthCriteria(std::weak_ptr<MessageContext> ctx, std::vector<SignalStrengthCriteriaEntry> criteria):
        SolicitedMessage<RIL_Errno>(MESSAGE_NAME, ctx, get_class_message_id()),
        mCriteria(criteria)
    {
    }

    std::vector<SignalStrengthCriteriaEntry> getCriteria() { return mCriteria;}

    string dump()
    {
      string dump = mName + "{";
      for (SignalStrengthCriteriaEntry entry: mCriteria) {
          dump += "{ ran: " + toString(entry.ran);
          dump += " delta: " + std::to_string(entry.delta);
          dump += " thresholds: { ";
          for (int32_t th: entry.thresholds) {
              dump += std::to_string(th) + ", ";
          }
          dump += "}";
      }
      return dump;
    }
};

