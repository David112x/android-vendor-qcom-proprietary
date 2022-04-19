#ifndef CNE_SRM_H
#define CNE_SRM_H

/**----------------------------------------------------------------------------
  @file CneSrm.h

-----------------------------------------------------------------------------*/

/*=============================================================================
    Copyright (c) 2009-2017 Qualcomm Technologies, Inc.
    All Rights Reserved.
    Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/

#include "CneSrmDefs.h"
#include "EventDispatcher.h"
#include "CneDefs.h"
#include <string>
#include <cutils/properties.h>
#include <map>
#include <list>
#include "CneSupplicantWrapper.h"

using namespace std;

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/
class Cne;
class CneResourceInfo;
class CneBatteryInfo;
class CneWlanInfo;
class CneWwanInfo;

/*----------------------------------------------------------------------------
 * Class Definitions
 * -------------------------------------------------------------------------*/
class CneSrm : public EventDispatcher<SrmEvent>
{
public:

  CneSrm
  (
    Cne &setCne
  );

  ~CneSrm();

  static CneSrm* getInstance
  (
    void
  );

  CneRetType putResourceInfo
  (
    int resType,
    void *res
  );

  int getListResourceInfo
  (
    int numItem,
    int resType,
    void *listRes
  );

  CneRetType startScanWlan
  (
    void
  );

  CneRetType getCurrentBatteryLevel
  (
    CneSrmBatteryLevelType* batteryLevel
  );

  CneRetType updateWlanScanResults
  (
    void *res
  );

  CneRetType updateWwanDormancyStatus
  (
    int status
  );

  bool getFeatureStatus(
     CneFeatureInfoType* cneFeatureInfoCmd
  );

  void setFeatureStatus(
     CneFeatureInfoType* cneFeatureInfoCmd
  );

  list<CneWlanScanListResourceType> getWlanScanResults(void);

  static void processCommand
  (
    int fd,
    cne_cmd_enum_type cmdType,
    void *cmdData,
    void *handlerDataPtr
  );

  static void processCneCmd
  (
    CneEvent event,
    const void *event_data,
    void *cbData
  );

  /* check if a RAT is dormant
   * returns: true if dormant, otherwise false
   */
  bool isRatDormant(cne_rat_type rat);

  /* check which network is the default
   * returns: default network
   */
  cne_rat_type getDefaultNetwork();

  //Reset default network variable if none available
  void invalidateDefaultNetwork();

  //Returns the number of available interfaces
  unsigned int getNumInterfacesAvailable();
  bool isIfaceSelectable(char *ifname);

  // return the current wlan info
  CneWlanResourceType getWlanInfo();

  // return the current wwan info
  CneWwanResourceType getWwanInfo();

  const char * getWifiIfaceDefName ();
  // returns whether WQE is enabled/disabled by user
  bool hasUserEnabledWqe(void);

  bool getAppInfoByName(const char *appName, AppInfoType& appInfo);

/*----------------------------------------------------------------------------
 * FUNCTION     CneSupplicantWrapper& getSupplicant
 *
 * DESCRIPTION  returns the reference to CneSupplicantWrapper object
 *
 * DEPENDENCIES CneSupplicantWrapper
 *
 * RETURN VALUE a reference to CneSupplicantWrapper object
 *
 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
 CneSupplicantWrapper& getSupplicant();

/*----------------------------------------------------------------------------
 * FUNCTION     updateWlanInfo
 *
 * DESCRIPTION  update wlanInfo stored in srm by calling
 *  processCneCmd with CNE_REQUEST_UPDATE_WLAN_INFO_CMD event
 *
 * DEPENDENCIES CneWlanResourceType, CneWlanInfo.cpp, EventDispatcher
 *
 * RETURN VALUE void
 *
 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
  void updateWlanInfo(CneWlanResourceType &wlanInfo);

/*----------------------------------------------------------------------------
 * FUNCTION     updateSoftApInfo
 *
 * DESCRIPTION  update SAP information by dispatching SRM_NOTIFY_WIFI_AP_INFO_EVENT
 *              event
 *
 * DEPENDENCIES EventDispatcher, CneCde
 *
 * RETURN VALUE void
 *
 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
  void updateSoftApInfo();

/*----------------------------------------------------------------------------
 * FUNCTION     updateP2pInfo
 *
 * DESCRIPTION  update P2P information by dispatching SRM_NOTIFY_WIFI_AP_INFO_EVENT
 *              event
 *
 * DEPENDENCIES EventDispatcher, CneCde
 *
 * RETURN VALUE void
 *
 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
  void updateP2pInfo();

/*----------------------------------------------------------------------------
 * FUNCTION     bringUpNetwork
 *
 * DESCRIPTION  bring up the requested network
 *
 * DEPENDENCIES None
 *
 * RETURN VALUE None
 *
 * SIDE EFFECTS None
 *--------------------------------------------------------------------------*/
  void bringUpNetwork(cne_rat_type rat, cne_slot_type slot);

/*----------------------------------------------------------------------------
 * FUNCTION     tearDownNetwork
 *
 * DESCRIPTION  tear down the requested network
 *
 * DEPENDENCIES None
 *
 * RETURN VALUE None
 *
 * SIDE EFFECTS None
 *--------------------------------------------------------------------------*/
  void tearDownNetwork(cne_rat_type rat, cne_slot_type slot);

/*----------------------------------------------------------------------------
 * FUNCTION     updateSlsInterfaceInfo
 *
 * DESCRIPTION  update SLS iface information by dispatching
                SRM_SLS_EVENT event
 *
 * DEPENDENCIES EventDispatcher
 *
 * RETURN VALUE void
 *
 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
  void updateSlsInterfaceInfo(CneSlsInterfaceInfo &mCneSlsInterfaceInfo);

/*----------------------------------------------------------------------------
 * FUNCTION     updateSlaRatioInfo
 *
 * DESCRIPTION  update SLA ratio information by dispatching
                SRM_SLA_EVENT event
 *
 * DEPENDENCIES EventDispatcher
 *
 * RETURN VALUE void
 *
 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
  void updateSlaRatioInfo(CneSlaRatioInfo &mCneSlaRatioInfo);

  void requestForPreciseSampling();
  void stopPreciseSampling();
  bool getPreciseSampling()const;
  bool getMobileDataEnabled() {return _isMobileDataEnabled;};
  bool getIMSProfileOverrideSetting() {return _isOverrideSet;};
  static CneRatSubType getWwanSubrat()
  {
    return _wwansubrat;
  }

  private:
    static CneSrm* instancePtr;
    CneSupplicantWrapper srmSupp;
    Cne &cne;
    CneResourceInfo *resInfo;
    CneWlanInfo *wlanInfo;
    CneWwanInfo *wwanInfo;
    CneWlanInfo *wifiP2pInfo;
    CneWlanInfo *wifiSapInfo;
    CneWlanInfo *lastConnectedWlanInfo;
    CneWwanInfo *lastConnectedWwanInfo;
    CneWlanInfo *lastNotifiedWlanInfo;
    // TODO
    // Switch from system properties to
    // database after initial merge
    // CneFeatureStore *fs;

    static bool isWwanDormant;
    static cne_rat_type defaultNetwork;
    bool isDhcpRenewState( CneWlanInfoType &curr, CneWlanResourceType &prev );
    bool dhcpRenewState;
    bool bWqe;
    char wifiIfaceDefName[PROPERTY_VALUE_MAX];
    char _ssid[CNE_MAX_SSID_LEN];
    char _dnsInfo[CNE_MAX_DNS_ADDRS][CNE_MAX_IPADDR_LEN];
    bool _isLastStatusConnectedOrConnecting;
    bool _isCurrentStatusConnectedOrConnecting;
    bool _isPreciseSampling;
    bool _isMobileDataEnabled;
    bool _isOverrideSet;
    static CneRatSubType _wwansubrat;

/*----------------------------------------------------------------------------
 * FUNCTION     startMonitorBssid
 *
 * DESCRIPTION  It calls startMonitorBssidChange in CneSupplicantWrapper.cpp
 *
 * DEPENDENCIES CneSupplicantWrapper
 *
 * RETURN VALUE void
 *
 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
    void startMonitorBssid();
/*----------------------------------------------------------------------------
 * FUNCTION     startMonitorBssid
 *
 * DESCRIPTION  It calls stopMonitorBssidChange in CneSupplicantWrapper.cpp
 *
 * DEPENDENCIES CneSupplicantWrapper
 *
 * RETURN VALUE void
 *
 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
    void stopMonitorBssid();

    void updateSubrat(CneRatSubType subrat);

};


#endif /* CNE_SRM_H */
