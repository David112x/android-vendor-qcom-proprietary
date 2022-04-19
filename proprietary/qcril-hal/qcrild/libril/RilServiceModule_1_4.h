/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include <ril_service_1_4.h>
#include <framework/Log.h>
#include <framework/Module.h>
#include <framework/QcrilInitMessage.h>
#include <framework/UnSolicitedMessage.h>
#include <modules/nas/NasModule.h>
#include <RilServiceModule_1_3.h>
#include "interfaces/nas/nas_types.h"
#include "interfaces/nas/RilUnsolEmergencyListIndMessage.h"
#include <UnSolMessages/RadioKeepAliveStatusIndMessage.h>
#include <UnSolMessages/RadioDataCallListChangeIndMessage.h>
#include <request/DeactivateDataCallRequestMessage.h>
#include <request/SetupDataCallRequestMessage.h>
#include <UnSolMessages/PhysicalConfigStructUpdateMessage.h>
#include <UnSolMessages/RilPCODataMessage.h>
#include <UnSolMessages/CarrierInfoForImsiEncryptionRefreshMessage.h>

V1_4::RadioTechnology convertRadioAccessFamilyTypeToRadioTechnology(rildata::RadioAccessFamily_t accessFamily);
V1_2::CellConnectionStatus convertCellConnStatusToHalType(rildata::CellConnectionStatus_t status);
V1_4::FrequencyRange convertFrequencyRangeToHalType(rildata::FrequencyRange_t range);

class RilServiceModule_1_4: public RilServiceModule_1_3 {
    private:
        RadioImpl_1_4 &mRadioImpl_1_4;

    public:
        RilServiceModule_1_4(qcril_instance_id_e_type instance, RadioImpl_1_4 &radioImpl);
        void handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg);
        void handleEmergencyListIndMessage(std::shared_ptr<RilUnsolEmergencyListIndMessage> msg);
        void handleRadioDataCallListChangeIndMessage(std::shared_ptr<rildata::RadioDataCallListChangeIndMessage> msg);
        void handlePhysicalConfigStructUpdateMessage(std::shared_ptr<rildata::PhysicalConfigStructUpdateMessage> msg);
        void handleSetupDataCallRadioResponseIndMessage(std::shared_ptr<rildata::SetupDataCallRadioResponseIndMessage> msg);
        void handleDeactivateDataCallRadioResponseIndMessage(std::shared_ptr<rildata::DeactivateDataCallRadioResponseIndMessage> msg);
        void handleRadioKeepAliveStatusIndMessage(std::shared_ptr<rildata::RadioKeepAliveStatusIndMessage> msg);
        void handleRilPCODataMessage(std::shared_ptr<rildata::RilPCODataMessage> msg);
        void handleCarrierInfoForImsiEncryptionRefreshMessage(std::shared_ptr<rildata::CarrierInfoForImsiEncryptionRefreshMessage> msg);
        void init() {
            Module::init();
            std::shared_ptr<NasSetSignalStrengthCriteriaSupported> cfgMsg =
                std::make_shared<NasSetSignalStrengthCriteriaSupported>(true);
            if (cfgMsg) {
                cfgMsg->dispatch();
            }
        }
};

