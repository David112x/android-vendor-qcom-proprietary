/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

GENERAL DESCRIPTION
  Izat WiFi DB Receiver Types and API functions

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#ifndef __IZAT_MANAGER_WWANDBPROV_C_H__
#define __IZAT_MANAGER_WWANDBPROV_C_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define IZAT_AP_LOC_WITH_LAT_LON          0x0;
#define IZAT_AP_LOC_MAR_VALID             0x1;
#define IZAT_AP_LOC_HORIZONTAL_ERR_VALID  0x2;
#define IZAT_AP_LOC_RELIABILITY_VALID     0x4;

#define IZAT_AP_SSID_PREFIX_LENGTH        8


struct BSCellInfo_s {
public:
    // Type of cell in which this Mac Address is observerd atleast
    // once, if available
    // 0: unknown, 1: GSM, 2: WCDMA, 3: CDMA, 4: LTE
    uint8_t  cell_type;

    // Cell ID if known
    // Cell RegionID1 Mobile Country Code(MCC), For CDMA Set to 0
    uint32_t cell_id1;
    // Cell RegionID2 Mobile Network Code(MNC), For CDMA set to
    // System ID(SID), For WCDMA set to 0 if not available
    uint32_t cell_id2;
    // Cell RegionID3 GSM: Local Area Code(LAC), WCDMA: Local Area
    // Code(LAC) set to 0 if not available, CDMA: Network ID(NID),
    // LTE: Tracking Area Code(TAC) set to 0 if not available
    uint32_t cell_id3;
    // Cell RegionID4 Cell ID(CID), For CDMA set to Base Station
    // ID(BSID)
    uint32_t cell_id4;

    inline BSCellInfo_s() : cell_type(0)
        , cell_id1(0)
        , cell_id2(0)
        , cell_id3(0)
        , cell_id4(0) {
    }
};

struct BSObsData_s {
public:
    uint64_t scanTimestamp_ms;
    UlpLocation gpsLoc;
    bool bUlpLocValid;
    float speed_unc;
    float bearing_unc;
    struct BSCellInfo_s cellInfo;

    inline BSObsData_s(): scanTimestamp_ms(0)
        ,gpsLoc()
        ,bUlpLocValid(0)
        ,cellInfo() {
    }
};

typedef void (*wifiDBProvObsLocDataUpdateCb)(BSObsData_s *bs_obs_data, size_t bs_obs_data_size,
        int bsListStatus, void *clientData);
typedef void (*wifiDBProvServiceRequestCb)(void *clientData);
typedef void (*wifiDBProvEnvNotifyCb)(void *clientData);

struct CellCSCallbacks {
    wifiDBProvObsLocDataUpdateCb bsObsLocDataAvailable;
    wifiDBProvServiceRequestCb serviceReqCb;
    wifiDBProvEnvNotifyCb envNotifyCb;
};
struct CellCSInfo {
    std::vector<BSObsData_s> bsObsDataList;
};

/* registers callbacks for Wifi DB updater
   bsObsLocDataAvailable  - callback to receive list of APs from GTP
            Can not be NULL.
   serviceReqCb  - callback to receive service request from GTP.
            Can not be NULL.
   envNotifyCb  - callback environement notification.
            Can be NULL.
   clientData - an opaque data pointer from client. This pointer will be
                provided back when the callbacaks are called.
                This can be used by client to store application context
                or statemachine etc.
                Can be NULL.
   return: an opaque pointer that serves as a request handle. This handle
           is to be fed to the unregisterWiFiDBProvider() call.
*/
void* registerWiFiDBProvider(wifiDBProvObsLocDataUpdateCb bsObsLocDataAvailable,
                            wifiDBProvServiceRequestCb serviceReqCb,
                            wifiDBProvEnvNotifyCb envNotifyCb,
                            void *clientData);

/* unregisters the client callback
   wifiDBProviderHandle - the opaque pointer from the return of
                           registerWiFiADProvider()
*/
void unregisterWiFiDBProvider(void *wifiDBProviderHandle);

/* Send request to obatin the observation data.
   wifiADProviderHandle - the opaque pointer from the return of
                           registerWiFiADProvider()
*/
void sendAPObsLocDataRequest(void *wifiADProviderHandle);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // #ifndef __IZAT_MANAGER_WWANDBPROV_C_H__
