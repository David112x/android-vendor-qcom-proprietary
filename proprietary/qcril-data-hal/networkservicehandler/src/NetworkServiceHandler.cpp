/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <framework/Log.h>
#include "legacy/qcril_event_facade.h"
#include "NetworkServiceHandler.h"
#include "IWlanDataRegistrationStateChangeIndMessage.h"

#include "modules/qmi/ModemEndPointFactory.h"
#include "modules/qmi/QmiSetupRequestCallback.h"

#include "DSDModemEndPoint.h"

using namespace rildata;

NetworkServiceHandler::NetworkServiceHandler()
{
  Log::getInstance().d("[NetworkServiceHandler]: constructor");
  QmiSetupRequestCallback qmiDsdSetupCallback("DSDModule-Token");
  ModemEndPointFactory<DSDModemEndPoint>::getInstance().buildEndPoint()->requestSetup(
    "DSDModule-Token",
    &qmiDsdSetupCallback);
}

void NetworkServiceHandler::processRadioRegStatusInd(dsd_system_status_ind_msg_v01 *ind)
{
    Log::getInstance().d("apassist update radio DsdSysStatusInd");
    qcril_data_set_dsd_sys_status(ind);
}

void NetworkServiceHandler::processQmiDsdSystemStatusInd(dsd_system_status_ind_msg_v01 * dsdSysStatusInd)
{
    Log::getInstance().d("[NetworkServiceHandler]: processQmiDsdSystemStatusInd");

    dsd_system_status_ind_msg_v01 radioDsdSysStatusInd;
    memset(&radioDsdSysStatusInd, 0, sizeof(radioDsdSysStatusInd));

    if(dsdSysStatusInd->avail_sys_valid) {
        int radioSysIdx = 0;
        for(uint32_t i=0 ; i<dsdSysStatusInd->avail_sys_len ; i++) {
            Log::getInstance().d("sys ["+std::to_string((int)i)+
                                "] network="+std::to_string((int)dsdSysStatusInd->avail_sys[i].technology)+
                                ", rat="+std::to_string((int)dsdSysStatusInd->avail_sys[i].rat_value)+
                                ", so_mask="+std::to_string((int)dsdSysStatusInd->avail_sys[i].so_mask));
            radioDsdSysStatusInd.avail_sys[radioSysIdx++] = dsdSysStatusInd->avail_sys[i];
        }
        radioDsdSysStatusInd.avail_sys_valid = (radioSysIdx==0?false:true);
        radioDsdSysStatusInd.avail_sys_len = radioSysIdx;
    }

    if(radioDsdSysStatusInd.avail_sys_valid) {
        Log::getInstance().d("legacy update radio DsdSysStatusInd");
        qcril_data_set_dsd_sys_status(&radioDsdSysStatusInd);
    }
}

ApAssistNetworkServiceHandler::ApAssistNetworkServiceHandler()
{
  iwlanRegCache.valid = false;
  iwlanRegCache.regState = DataRegState_t::NOT_REG_AND_NOT_SEARCHING;
  iwlanRegCache.reasonForDenial = -1;
}

void ApAssistNetworkServiceHandler::dump(string padding, ostream& os)
{
    os << padding << "RegistrationState=" << (iwlanRegCache.valid ?
            convertToString(iwlanRegCache.regState) : "invalid") << endl;
}

DataRegState_t ApAssistNetworkServiceHandler::getIWlanDataRegistrationState()
{
    return iwlanRegCache.regState;
}

std::string ApAssistNetworkServiceHandler::convertToString(const DataRegState_t & regState)
{
    switch (regState) {
        case DataRegState_t::NOT_REG_AND_NOT_SEARCHING:
            return "NOT_REG_AND_NOT_SEARCHING";
        case DataRegState_t::REG_HOME:
            return "REG_HOME";
        case DataRegState_t::NOT_REG_AND_SEARCHING:
            return "NOT_REG_AND_SEARCHING";
        case DataRegState_t::REG_DENIED:
            return "REG_DENIED";
        case DataRegState_t::UNKNOWN:
            return "UNKNOWN";
        case DataRegState_t::REG_ROAMING:
            return "REG_ROAMING";
        case DataRegState_t::NOT_REG_AND_NOT_SEARCHING_EMERGENCY:
            return "NOT_REG_AND_NOT_SEARCHING_EMERGENCY";
        case DataRegState_t::NOT_REG_AND_SEARCHING_EMERGENCY:
            return "NOT_REG_AND_SEARCHING_EMERGENCY";
        case DataRegState_t::REG_DENIED_EMERGENCY:
            return "REG_DENIED_EMERGENCY";
        case DataRegState_t::UNKNOWN_EMERGENCY:
            return "UNKNOWN_EMERGENCY";
    }
}

void ApAssistNetworkServiceHandler::processIWlanRegStatusInd(dsd_system_status_ind_msg_v01 *ind)
{
    DataRegState_t regstate = DataRegState_t::NOT_REG_AND_NOT_SEARCHING;
    for(uint32_t i=0 ; i<ind->avail_sys_len ; i++) {
        Log::getInstance().d("iwlan sys ["+std::to_string((int)i)+
                            "] network="+std::to_string((int)ind->avail_sys[i].technology)+
                            ", rat="+std::to_string((int)ind->avail_sys[i].rat_value)+
                            ", so_mask="+std::to_string((int)ind->avail_sys[i].so_mask));
        if(ind->avail_sys[i].rat_value == DSD_SYS_RAT_EX_3GPP_WLAN_V01) {
            if(ind->avail_sys[i].so_mask == QMI_DSD_3GPP_SO_MASK_S2B_V01) {
                regstate = DataRegState_t::REG_HOME;
                break;
            }
        }
    }

    if((iwlanRegCache.valid != true) || (regstate != iwlanRegCache.regState)) {
        Log::getInstance().d("apassist update iwlan DsdSysStatusInd with regstate = "+std::to_string((int)regstate));
        iwlanRegCache.valid = true;
        iwlanRegCache.regState = regstate;
        auto msg = std::make_shared<IWlanDataRegistrationStateChangeIndMessage>();
        msg->broadcast();
    }
}

void ApAssistNetworkServiceHandler::processQmiDsdSystemStatusInd(dsd_system_status_ind_msg_v01 * dsdSysStatusInd)
{
    Log::getInstance().d("[ApAssistNetworkServiceHandler]: processQmiDsdSystemStatusInd");

    dsd_system_status_ind_msg_v01 radioDsdSysStatusInd;
    memset(&radioDsdSysStatusInd, 0, sizeof(radioDsdSysStatusInd));
    dsd_system_status_ind_msg_v01 iwlanDsdSysStatusInd;
    memset(&iwlanDsdSysStatusInd, 0, sizeof(iwlanDsdSysStatusInd));

    if(dsdSysStatusInd->avail_sys_valid) {
        int radioSysIdx=0;
        int iwlanSysIdx=0;
        for(uint32_t i=0 ; i<dsdSysStatusInd->avail_sys_len ; i++) {
            Log::getInstance().d("sys ["+std::to_string((int)i)+
                                "] network="+std::to_string((int)dsdSysStatusInd->avail_sys[i].technology)+
                                ", rat="+std::to_string((int)dsdSysStatusInd->avail_sys[i].rat_value)+
                                ", so_mask="+std::to_string((int)dsdSysStatusInd->avail_sys[i].so_mask));
            if((dsdSysStatusInd->avail_sys[i].technology == DSD_SYS_NETWORK_WLAN_V01) ||
               (dsdSysStatusInd->avail_sys[i].rat_value == DSD_SYS_RAT_EX_3GPP_WLAN_V01) ||
               (dsdSysStatusInd->avail_sys[i].rat_value == DSD_SYS_RAT_EX_3GPP2_WLAN_V01) ||
               (dsdSysStatusInd->avail_sys[i].rat_value == DSD_SYS_RAT_EX_WLAN_V01) ||
               (dsdSysStatusInd->avail_sys[i].rat_value == DSD_SYS_RAT_EX_WLAN_MAX_V01)) {
                iwlanDsdSysStatusInd.avail_sys[iwlanSysIdx++] = dsdSysStatusInd->avail_sys[i];
            }
            else {
                radioDsdSysStatusInd.avail_sys[radioSysIdx++] = dsdSysStatusInd->avail_sys[i];
            }
        }
        radioDsdSysStatusInd.avail_sys_valid = (radioSysIdx == 0 ? false:true);
        iwlanDsdSysStatusInd.avail_sys_valid = (iwlanSysIdx == 0 ? false:true);
        radioDsdSysStatusInd.avail_sys_len = radioSysIdx;
        iwlanDsdSysStatusInd.avail_sys_len = iwlanSysIdx;
    }

    processRadioRegStatusInd(&radioDsdSysStatusInd);
    processIWlanRegStatusInd(&iwlanDsdSysStatusInd);
}
