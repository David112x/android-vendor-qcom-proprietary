/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include <ril_service_1_3.h>
#include <framework/Log.h>
#include <framework/Module.h>
#include <framework/QcrilInitMessage.h>
#include <framework/UnSolicitedMessage.h>
#include <modules/nas/NasModule.h>
#include <RilServiceModule_1_2.h>

class RilServiceModule_1_3: public RilServiceModule_1_2 {
    public:
        RilServiceModule_1_3(qcril_instance_id_e_type instance);
        void handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg);
        void init() {
            Module::init();
            std::shared_ptr<NasSetSignalStrengthCriteriaSupported> cfgMsg =
                std::make_shared<NasSetSignalStrengthCriteriaSupported>(true);
            if (cfgMsg) {
                cfgMsg->dispatch();
            }
        }
};

