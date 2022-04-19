/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2016, 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#define LOG_NDEBUG 0
#define LOG_TAG "LocSvc_LBSApiRpc"
#include <log_util.h>
#include <loc_api_fixup.h>
#include <LBSApiRpc.h>
#include <IzatApiRpc.h>
using namespace loc_core;

namespace lbs_core {

// Conversion factor for Latitude and Longitude
// Convert from degrees to units required by locapi
#define COARSE_LAT_SCALE_FACTOR 23860929.4222
#define COARSE_LON_SCALE_FACTOR 11930464.7111

#define LAT_DEGREE_TO_RPC(x) ((int)(x*COARSE_LAT_SCALE_FACTOR))
#define LONG_DEGREE_TO_RPC(x) ((int)(x*COARSE_LON_SCALE_FACTOR))

int LBSApiRpc::locEventCB(rpc_loc_client_handle_type handle,
                          rpc_loc_event_mask_type event,
                          const rpc_loc_event_payload_u_type* payload)
{
    int ret = -1;
    if(((LocApiProxyRpc *)mLocApiProxy)->locEventCB(handle, event, payload)) {
        ret = 0;
    }
    else if(LocApiRpc::locEventCB(handle, event, payload)) {
        ret = 0;
    }
    return ret;
}

LBSApiRpc :: LBSApiRpc(LOC_API_ADAPTER_EVENT_MASK_T exMask) :
    LocApiRpc(exMask),
    LBSApiBase(new LocApiProxyRpc(this)),
{
    LOC_LOGD("%s:%d]: LBSApiRpc created. lbsapi: %p, izatapi: %p\n",
             __func__, __LINE__, this, mIzatApi);
}

LBSApiRpc* LBSApiRpc :: createLBSApiRpc(LOC_API_ADAPTER_EVENT_MASK_T exMask)
{
    return new LBSApiRpc(exMask);
}

}  // namespace lbs_core
