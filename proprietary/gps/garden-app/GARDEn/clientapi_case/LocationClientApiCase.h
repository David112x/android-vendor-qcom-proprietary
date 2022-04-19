/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#ifndef LOCATIONCLIENTAPI_CASE_H
#define LOCATIONCLIENTAPI_CASE_H

#include "GardenFramework.h"
#include "GnssLocationClientAPI.h"

namespace garden {

    class LocationClientApiCase : public IGardenCase {
    public:
        LocationClientApiCase();
        virtual ~LocationClientApiCase();
        virtual GARDEN_RESULT preRun();
        virtual GARDEN_RESULT postRun();
        GARDEN_RESULT menuCase();
        GARDEN_RESULT autoStartCases(uint32_t interval, uint32_t loop,
            uint32_t option, int redirectNMEA);
        void testDeleteAll(::PositioningEngineMask posEngineMask);

    protected:
        GnssCbBase* mCb;

    private:
        GnssLocationClientAPI* mPLocationClient_1;
        GnssLocationClientAPI* mPLocationClient_2;

        GARDEN_RESULT testGeofence(int clientId);
        void testGetEnergy();
        void testRegSystemInfo();
        void testUnregSystemInfo();
        void menuAddGeofence(int clientId);
        void menuModifyGeofence(int clientId);
        void menuPauseGeofence(int clientId);
        void menuResumeGeofence(int clientId);
        void menuRemoveGeofence(int clientId);
        void testPing();
    };

} // namespace garden
#endif // LOCATIONCLIENTAPI_CASE_H
