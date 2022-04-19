/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include <ril_service_1_2.h>
#include <framework/Log.h>
#include <framework/Module.h>
#include <framework/QcrilInitMessage.h>
#include <framework/UnSolicitedMessage.h>
#include <request/LinkCapIndMessage.h>
#include <modules/nas/NasModule.h>
#include <RilServiceModule.h>

using rildata::LinkCapIndMessage;
using rildata::LinkCapEstimate_t;

class RilServiceModule_1_2: public RilServiceModule {
    public:
        RilServiceModule_1_2(qcril_instance_id_e_type instance);
        void handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg);
        void handleLinkCapIndMessage(std::shared_ptr<LinkCapIndMessage> msg);
        void init() {
            Module::init();
            std::shared_ptr<NasSetSignalStrengthCriteriaSupported> cfgMsg =
                std::make_shared<NasSetSignalStrengthCriteriaSupported>(true);
            if (cfgMsg) {
                cfgMsg->dispatch();
            }
        }
};

