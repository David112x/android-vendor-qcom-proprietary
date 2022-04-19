/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include <framework/Log.h>
#include <framework/Module.h>
#include <framework/QcrilInitMessage.h>
#include <framework/UnSolicitedMessage.h>
#include <modules/pbm/PbmModule.h>
#include <modules/mbn/MbnModule.h>
#include <modules/uim/UimSimlockTempUnlockExpireInd.h>
#include <modules/uim/UimCardStateChangeInd.h>
#include <modules/uim/UimSlotStatusInd.h>
#include <modules/uim/UimSimRefreshIndication.h>
#include <modules/uim/UimVoltageStatusInd.h>
#include <modules/uim_remote/UimRmtRemoteSimStatusIndMsg.h>
#include "qcril_qmi_oemhook_service.h"

using namespace vendor::qti::hardware::radio::qcrilhook::V1_0::implementation;

class OemHookServiceModule: public Module {
    public:
        OemHookServiceModule(qcril_instance_id_e_type instance);
        virtual void init() {
            Module::init();
        }
    protected:
        sp<OemHookImpl> getOemHookImpl();
        qcril_instance_id_e_type mInstance;
        virtual void handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg);
    private:
        void handleAdnRecordsOnSimMessage(std::shared_ptr<QcRilUnsolAdnRecordsOnSimMessage> msg);
        void handleAdnInitDoneMessage(std::shared_ptr<QcRilUnsolAdnInitDoneMessage> msg);
        void handleMbnConfigResultMessage(std::shared_ptr<QcRilUnsolMbnConfigResultMessage> msg);
        void handleMbnConfigClearedMessage(std::shared_ptr<QcRilUnsolMbnConfigClearedMessage> msg);
        void handleMbnValidateDumpedMessage(std::shared_ptr<QcRilUnsolMbnValidateDumpedMessage> msg);
        void handleMbnConfigListMessage(std::shared_ptr<QcRilUnsolMbnConfigListMessage> msg);
        void handleMbnValidateConfigMessage(std::shared_ptr<QcRilUnsolMbnValidateConfigMessage> msg);
        void handleUimSimlockTempUnlockExpireInd(std::shared_ptr<UimSimlockTempUnlockExpireInd> msg);
        void handleUimCardStateChangeInd(std::shared_ptr<UimCardStateChangeInd> msg);
        void handleUimSlotStatusInd(std::shared_ptr<UimSlotStatusInd> msg);
        void handleUimSimRefreshIndication(std::shared_ptr<UimSimRefreshIndication> msg);
        void handleUimVoltageStatusInd(std::shared_ptr<UimVoltageStatusInd> msg);
        void handleUimRmtRemoteSimStatusIndMsg(std::shared_ptr<UimRmtRemoteSimStatusIndMsg> msg);
        void handleQcRilUnsolDtmfMessage(std::shared_ptr<QcRilUnsolDtmfMessage> msg);
        void handleQcRilUnsolExtBurstIntlMessage(std::shared_ptr<QcRilUnsolExtBurstIntlMessage> msg);
        void handleQcRilUnsolNssReleaseMessage(std::shared_ptr<QcRilUnsolNssReleaseMessage> msg);
        void handleQcRilUnsolSuppSvcErrorCodeMessage(std::shared_ptr<QcRilUnsolSuppSvcErrorCodeMessage> msg);
        void handleQcRilUnsolSpeechCodecInfoMessage(std::shared_ptr<QcRilUnsolSpeechCodecInfoMessage> msg);
};

