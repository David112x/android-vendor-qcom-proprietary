/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

GENERAL DESCRIPTION
  Izat WiFi DB Receiver Types and API functions

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#ifndef __IZAT_MANAGER_WIFIDBPROV_C_H__
#define __IZAT_MANAGER_WIFIDBPROV_C_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define IZAT_AP_LOC_WITH_LAT_LON          0x0;
#define IZAT_AP_LOC_MAR_VALID             0x1;
#define IZAT_AP_LOC_HORIZONTAL_ERR_VALID  0x2;
#define IZAT_AP_LOC_RELIABILITY_VALID     0x4;

#define IZAT_AP_SSID_PREFIX_LENGTH        32

struct APScanInfo_s {
public:
    // Full 48 bits of the Mac Address in reverse order represented
    // in 64 bit format
    uint64_t mac_R48b;

    // RSSI
    int16_t rssi;

    //Measurement age msec
    uint64_t age_usec;

    //channel id
    uint8_t channel_id;

    // SSID information. First 8 bytes of SSID if available
    char ssid[IZAT_AP_SSID_PREFIX_LENGTH];
    uint8_t ssid_valid_byte_count;

    inline APScanInfo_s() : mac_R48b(0)
        , rssi(0)
        , age_usec(0)
        , channel_id(0)
        , ssid {0}
        , ssid_valid_byte_count(0) {
    }
};

struct APCellInfo_s {
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

    inline APCellInfo_s() : cell_type(0)
        , cell_id1(0)
        , cell_id2(0)
        , cell_id3(0)
        , cell_id4(0) {
    }
};

struct APObsData_s {
public:
    uint64_t scanTimestamp_ms;
    UlpLocation gpsLoc;
    bool bUlpLocValid;
    struct APCellInfo_s cellInfo;
    size_t ap_scan_info_size;
    std::vector<APScanInfo_s> ap_scan_info;

    inline APObsData_s(): scanTimestamp_ms(0)
        ,gpsLoc()
        ,bUlpLocValid(0)
        ,cellInfo()
        ,ap_scan_info_size(0)
        ,ap_scan_info() {
    }
};

typedef void (*wifiDBProvObsLocDataUpdateCb)(APObsData_s *ap_obs_data[], size_t ap_obs_data_size,
        int apListStatus, void *clientData);
typedef void (*wifiDBProvServiceRequestCb)(void *clientData);
typedef void (*wifiDBProvEnvNotifyCb)(void *clientData);


/* registers callbacks for Wifi DB updater
   apObsLocDataAvailable  - callback to receive list of APs from GTP
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
void* registerWiFiDBProvider(wifiDBProvObsLocDataUpdateCb apObsLocDataAvailable,
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

#endif // #ifndef __IZAT_MANAGER_WIFIDBPROV_C_H__
