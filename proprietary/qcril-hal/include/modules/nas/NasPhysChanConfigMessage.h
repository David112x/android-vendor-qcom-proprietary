/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include "framework/SolicitedMessage.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

class NasPhysChanInfo {
    public:
        enum class Status {
            NONE,
            PRIMARY,
            SECONDARY,
        };

        NasPhysChanInfo(Status status, int32_t bw, uint32_t pci):
            mStatus(status), mBandwidth(bw), mPhysicalCellId(pci) {}

        Status getStatus() const {
            return mStatus;
        }

        int32_t getBandwidth() const {
            return mBandwidth;
        }

        uint32_t getPhysicalCellId() const {
            return mPhysicalCellId;
        }

    private:
        Status mStatus;
        int32_t mBandwidth;
        uint32_t mPhysicalCellId;
};

/** NasPhysChanConfigMessage.
 * This message is sent by the Nas module to notify of changes in the
 * physical channel configuration
 */
class NasPhysChanConfigMessage : public UnSolicitedMessage,
                                 public add_message_id<NasPhysChanConfigMessage>
{
    private:
        std::shared_ptr<std::vector<NasPhysChanInfo>> info;

    public:
        static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.nas.PhysChanConfig";

        NasPhysChanConfigMessage(): UnSolicitedMessage(get_class_message_id()) {
            mName = MESSAGE_NAME;
        }

        NasPhysChanConfigMessage(std::shared_ptr<std::vector<NasPhysChanInfo>> physChanInfo):
                UnSolicitedMessage(get_class_message_id()), info(physChanInfo) {
            mName = MESSAGE_NAME;
        }

        ~NasPhysChanConfigMessage() {}

        std::shared_ptr<const std::vector<NasPhysChanInfo>> getInfo() {
            return info;
        }

        std::shared_ptr<UnSolicitedMessage> clone() {
            return nullptr;
        }

        string dump() {
            return mName;
        }
};

class NasGetPhyChanConfigMessage : public SolicitedMessage<std::vector<NasPhysChanInfo>>,
                                   public add_message_id<NasGetPhyChanConfigMessage>
{
    public:
        static constexpr const char *MESSAGE_NAME =
            "com.qualcomm.qti.qcril.nas.GetPhyChanConfig";

        inline NasGetPhyChanConfigMessage():
            SolicitedMessage<std::vector<NasPhysChanInfo>>(get_class_message_id())
        {
        }

        inline ~NasGetPhyChanConfigMessage() {}

        string dump()
        {
            return mName;
        }
};

/** NasPhysChanConfigReportingStatus.
 *  This message is sent by the NAS module whenever physical
 *  channel configuration reporting is enabled or disabled.
 */
class NasPhysChanConfigReportingStatus : public UnSolicitedMessage,
                                         public add_message_id<NasPhysChanConfigReportingStatus>
{
    private:
        bool physChanConfigReportingEnabled;

    public:
        static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.nas.PhysChanConfigReportingStatus";

        NasPhysChanConfigReportingStatus(): UnSolicitedMessage(get_class_message_id()) {
            mName = MESSAGE_NAME;
        }

        NasPhysChanConfigReportingStatus(bool enabled):
                UnSolicitedMessage(get_class_message_id()), physChanConfigReportingEnabled(enabled) {
            mName = MESSAGE_NAME;
        }

        ~NasPhysChanConfigReportingStatus() {}

        bool isPhysChanConfigReportingEnabled() {
            return physChanConfigReportingEnabled;
        }

        std::shared_ptr<UnSolicitedMessage> clone() {
            return nullptr;
        }

        string dump() {
            return mName;
        }
};
