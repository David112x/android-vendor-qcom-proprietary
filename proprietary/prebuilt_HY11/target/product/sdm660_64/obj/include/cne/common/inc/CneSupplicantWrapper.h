#ifndef _CneSupplicantWrapper_h_
#define _CneSupplicantWrapper_h_
/*============================================================================
 FILE:         CneSupplicantWrapper.cpp

 OVERVIEW:     Provides Interface to wpa_supplicant

 DEPENDENCIES: Logging, C++ STL

 Copyright (c) 2013-2018 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ============================================================================*/

/*=============================================================================
 EDIT HISTORY FOR MODULE

 when        who     what, where, why
 ----------  ---     ---------------------------------------------------------
 05-21-2013  sregmi     First revision.
 06-25-2014  boxiang    attach to supplicant for monitor bssid change.
 03-01-2017  yuzhew     Extend for STA, SAP and P2P mode, issue chan_switch
 =============================================================================*/

/*------------------------------------------------------------------------------
 * Include Files
 * ---------------------------------------------------------------------------*/

#include "CneCom.h"
#include "CneSrmDefs.h"
#include <net/if.h>
#ifndef WPA_SUPPLICANT_DRIVER_DISABLED
   #include "wpa_ctrl.h"
#endif


typedef enum supplicantType_t{
    SUPPLICANT_TYPE_WLAN = 0,
    SUPPLICANT_TYPE_P2P,
    SUPPLICANT_TYPE_SAP
}supplicantType_e;

static const char P2P_SUP_CTRL_SOCKET_NAME[] = "/data/vendor/wifi/wpa/sockets/p2p0";
static const char WPA_SUP_CTRL_SOCKET_DIR[] = "/data/vendor/wifi/wpa/sockets";
static const char* SAP_SUP_CTRL_SOCKET_NAME;

typedef void (*scanResultReady_t)(void *data, void *arg);
typedef void (*chanSwitchComplete_t)(void *data, void *arg);
using namespace std;

class CneSrm;

//BssidSetType to contain a set of blacklisted bssid.
struct bssidStringCompare {
    bool operator()(const string &lhs, const string &rhs) const {
        return (lhs.compare(rhs) < 0);
    }
};

typedef std::set<string, bssidStringCompare> BssidSetType;

class CneSupplicantWrapper {
public:
    /*----------------------------------------------------------------------------
     * Public Attributes
     * -------------------------------------------------------------------------*/
    int16_t currRSSI;
    static const std::string scanCmd;
    static const std::string scanResultCmd;
    static const std::string signalPollCmd;
    static const std::string statusCmd;
    static const std::string chanSwitchCmd;
    static const std::string vendorCmd;

    /* Constructor */
    CneSupplicantWrapper(CneCom& setCom, CneSrm& setSrm);

    /* Destructor */
    ~CneSupplicantWrapper();

    /*----------------------------------------------------------------------------
     * FUNCTION      SrmEventHandler
     *
     * DESCRIPTION   Handles events notified from Srm.
     *
     * DEPENDENCIES  Logging
     *
     * RETURN VALUE  None
     *
     * SIDE EFFECTS
     *--------------------------------------------------------------------------*/
    static void SrmEventHandler(SrmEvent event, const void *event_data,
            void *cbData);

    /*----------------------------------------------------------------------------
     * FUNCTION      blacklistBSSIDs
     *
     * DESCRIPTION   Notify WPA Supplicant of bssid/ssid disabled by Cne
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  boolean
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    bool blacklistBSSIDs(const BssidSetType &bssidSet);

    /*----------------------------------------------------------------------------
     * FUNCTION      connectToSupplicant
     *
     * DESCRIPTION   Connect to WPA Supplicant.
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  Boolean
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    bool connectToSupplicant();

    /*----------------------------------------------------------------------------
     * FUNCTION      connectToSupplicant

     * DESCRIPTION   Connect to WPA Supplicant.

     * DEPENDENCIES  None

     * RETURN VALUE  Boolean

     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    bool connectToSupplicant(supplicantType_e suppType);

    /*----------------------------------------------------------------------------
     * FUNCTION      disconnectFromSupplicant
     *
     * DESCRIPTION   Disconnect from WPA Supplicant.
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  Boolean
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    bool disconnectFromSupplicant();

    /*----------------------------------------------------------------------------
     * FUNCTION      disconnectFromSupplicant

     * DESCRIPTION   Disconnect from WPA Supplicant.

     * DEPENDENCIES  None

     * RETURN VALUE  Boolean

     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    bool disconnectFromSupplicant(supplicantType_e suppType);

    /*----------------------------------------------------------------------------
     * FUNCTION      sendCommandToSupplicant
     *
     * DESCRIPTION   send command string to WPA Supplicant.
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  Boolean
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    bool sendCommandToSupplicant(const std::string cmd);

    /*----------------------------------------------------------------------------
     * FUNCTION      sendCommandToSupplicant
     *
     * DESCRIPTION   send command string to WPA Supplicant
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  Boolean
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    bool sendCommandToSupplicant(const std::string setCmdStr, char *responseBuf,
            size_t *responseBufLen);

    /*----------------------------------------------------------------------------
     * FUNCTION      sendCommandToSupplicant
     *
     * DESCRIPTION   send command string to WPA Supplicant and fill the response
     *               struct
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  Boolean
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    bool sendCommandToSupplicant(const std::string setCmdStr,
        char *responseBuf, size_t *responseBufLen, supplicantType_e suppType);

    /*----------------------------------------------------------------------------
     * FUNCTION      sendVendorCmdToSupplicant
     *
     * DESCRIPTION   send vendor command data to WPA Supplicant.
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  Boolean
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    bool sendVendorCmdToSupplicant(uint16_t vendorCmdId,
        std::string hexData, char *responseBuf, size_t *responseBufLen);

    /*----------------------------------------------------------------------------
     * FUNCTION      startMonitorBssidChange
     *
     * DESCRIPTION   open control handler to wpa supplicant, and attach to the
     *               control handler for listening to event from wpa supplicant
     *               Then add wpa_ctrl_listen_hdlr to CneCom for monitoring event
     *               from wpa supplicant. If wpa_ctrl_listen_hdlr is already open
     *               and attached, this function returns directly without doing
     *               anything.
     *
     * DEPENDENCIES  CneCom, wpa_ctrl.h
     *
     * RETURN VALUE  void
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    void startMonitorBssidChange();

    /*----------------------------------------------------------------------------
     * FUNCTION      stopMonitorBssidChange
     *
     * DESCRIPTION   it removes the wpa_ctrl_listen_hdlr from CneCom , then
     *               detach wpa_ctrl_listen_hdlr, and finally close connection
     *               to wpa supplicant. If wpa_ctrl_listen_hdlr is already closed
     *               then this function returns directly without doing anything.
     *
     * DEPENDENCIES  CneCom, wpa_ctrl.h
     *
     * RETURN VALUE  void
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    void stopMonitorBssidChange();

    /*----------------------------------------------------------------------------
     * FUNCTION      monitorBssidCb
     *
     * DESCRIPTION   it receives messages from wpa supplicant via wpa_ctrl_recv
     *               it looks for "CTRL_EVENT_CONNECTED" messages and extract
     *               Bssid from it.
     *               NOTE: if the format of messages from wpa_ctrl_recv changes,
     *               then we must update the parsing of bssid accordingly.
     *               If BSSID length is changed, we must update BSSID_OFFSET .
     * DEPENDENCIES  CneSrm, wpa_ctrl.h
     *
     * RETURN VALUE  void
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    static void monitorBssidCb(int fd, void* args);

    /*----------------------------------------------------------------------------
     * FUNCTION      startMonitorChanSwitchChange
     *
     * DESCRIPTION   it sets flag so CSA (Channel Switch Announcement) event will
     *               be processed in monitorEventsCb
     *
     * DEPENDENCIES
     *
     * RETURN VALUE  void
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    void startMonitorChanSwitchChange();

    /*----------------------------------------------------------------------------
     * FUNCTION      stopMonitorChanSwitchChange
     *
     * DESCRIPTION   it clears flag CSA event will not be processed in monitorEventsCb
     *
     * DEPENDENCIES
     *
     * RETURN VALUE  void
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    void stopMonitorChanSwitchChange();

    /*----------------------------------------------------------------------------
     * FUNCTION      startMonitorWpaEvents
     *
     * DESCRIPTION   open control handler to wpa supplicant, and attach to the
     *               control handler for listening to event from wpa supplicant
     *               Then add wpa_ctrl_listen_hdlr to CneCom for monitoring event
     *               from wpa supplicant. If wpa_ctrl_listen_hdlr is already open
     *               and attached, this function returns directly without doing
     *               anything.
     *
     * DEPENDENCIES  CneCom, wpa_ctrl.h
     *
     * RETURN VALUE  void
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    void startMonitorWpaEvents();

    /*----------------------------------------------------------------------------
     * FUNCTION      startMonitorWpaEvents
     *
     * DESCRIPTION   open control handler to wpa supplicant, and attach to the
     *               control handler for listening to event from wpa supplicant
     *               Then add wpa_ctrl_listen_hdlr to CneCom for monitoring event
     *               from wpa supplicant. If wpa_ctrl_listen_hdlr is already open
     *               and attached, this function returns directly without doing
     *               anything.
     *
     * DEPENDENCIES  CneCom, wpa_ctrl.h
     *
     * RETURN VALUE  void
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    void startMonitorWpaEvents(supplicantType_e suppType);

    /*----------------------------------------------------------------------------
     * FUNCTION      stopMonitorWpaEvents
     *
     * DESCRIPTION   it removes the wpa_ctrl_listen_hdlr from CneCom , then
     *               detach wpa_ctrl_listen_hdlr, and finally close connection
     *               to wpa supplicant. If wpa_ctrl_listen_hdlr is already closed
     *               then this function returns directly without doing anything.
     *
     * DEPENDENCIES  CneCom, wpa_ctrl.h
     *
     * RETURN VALUE  void
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    void stopMonitorWpaEvents();

    /*----------------------------------------------------------------------------
     * FUNCTION      stopMonitorWpaEvents
     *
     * DESCRIPTION   it removes the wpa_ctrl_listen_hdlr from CneCom , then
     *               detach wpa_ctrl_listen_hdlr, and finally close connection
     *               to wpa supplicant. If wpa_ctrl_listen_hdlr is already closed
     *               then this function returns directly without doing anything.
     *
     * DEPENDENCIES  CneCom, wpa_ctrl.h
     *
     * RETURN VALUE  void
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    void stopMonitorWpaEvents(supplicantType_e suppType);

    /*----------------------------------------------------------------------------
     * FUNCTION      monitorEventsCb
     *
     * DESCRIPTION   it receives messages from wpa supplicant via wpa_ctrl_recv
     *               it looks for "CTRL_EVENT_CONNECTED" messages and
     *               "AP-CSA-FINISHED" message.
     *
     *               NOTE: if the format of messages from wpa_ctrl_recv changes,
     *               then we must update the parsing of bssid and "AP-CSA-FINISHED"
     *               message accordingly.
     *               If BSSID length is changed, we must update BSSID_OFFSET .
     * DEPENDENCIES  CneSrm, wpa_ctrl.h
     *
     * RETURN VALUE  void
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    static void monitorEventsCb(int fd, void* args);

    void registerForScanReadyCb(scanResultReady_t cb, void *data);
    void registerForChanSwitchCb(chanSwitchComplete_t cb, void *data);
    bool setBssidFileterForScan(const std::string filter);
    bool setFrequencyFilterForScan(const std::string filter);

protected:
    /*----------------------------------------------------------------------------
     * Protected Method Specifications
     * -------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------
     * Private Method Specifications
     * -------------------------------------------------------------------------*/

    CneCom &suppCom;
    CneSrm &srm;
    bool _isMonitorBssidStarted;
    bool _isMonitorChanSwitchStarted;

    bool _isWlanMonitorStarted;
    bool _isP2pMonitorStarted;
    bool _isSapMonitorStarted;

    void *wifiProvider;
    scanResultReady_t scan_cb;
    void *chanSwitchCbData;
    chanSwitchComplete_t chanSwitch_cb;
    /*----------------------------------------------------------------------------
     * Private Attributes
     * -------------------------------------------------------------------------*/
#ifndef WPA_SUPPLICANT_DRIVER_DISABLED
    wpa_ctrl *wpa_wlan_ctrl_send_hdlr;
    wpa_ctrl *wpa_p2p_ctrl_send_hdlr;
    wpa_ctrl *wpa_sap_ctrl_send_hdlr;
    //needs to be static because it is used in monitorBssidCb, a static function
    static wpa_ctrl *wpa_wlan_ctrl_listen_hdlr;
    static wpa_ctrl *wpa_p2p_ctrl_listen_hdlr;
    static wpa_ctrl *wpa_sap_ctrl_listen_hdlr;
#endif
    // Wlan interface name
    char wlan_iface[IFNAMSIZ];

    /*----------------------------------------------------------------------------
     * FUNCTION      attachToSupplicant
     *
     * DESCRIPTION   open wpa_ctrl_listen_hdlr and attach to wpa supplicant for
     *               listening events from wpa supplicant
     *
     * DEPENDENCIES  wpa_ctrl.h
     *
     * RETURN VALUE  Boolean
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    bool attachToSupplicant();

    /*----------------------------------------------------------------------------
     * FUNCTION      attachTosSupplicant
     *
     * DESCRIPTION   open wpa_ctrl_listen_hdlr and attach to wpa supplicant for
     *               listening events from wpa supplicant for different supp type
     *
     * DEPENDENCIES  wpa_ctrl.h
     *
     * RETURN VALUE  Boolean
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    bool attachToSupplicant(supplicantType_e suppType);

    /*---------------------------------------------------------------------------
     * FUNCTION      validateBSSID
     *
     * DESCRIPTION   Validate BSSID String before sending to wpa
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  Boolean
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    bool isBSSIDValid(const std::string cmd);

    /*----------------------------------------------------------------------------
     * FUNCTION      createCmdDisallowedAps
     *
     * DESCRIPTION   Create Command with spec to disallow a bssid:
     *               SET disallow_aps <disallow_list>
     *               disallow_list ::= <ssid_spec>|<bssid_spec>|<disallow_list>|��
     *               SSID_SPEC ::= ssid <SSID_HEX>
     *               BSSID_SPEC ::= bssid <BSSID_HEX>
     *               Eg: set disallow_aps "bssid 123456789ABC bssid ABCDEF012345"
     *               Eg: set disallow_aps "" <-- Clears all disallowed APs
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  String containing the above command.
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/
    std::string createCmdDisallowedAps(const BssidSetType &bssidSet);

    /*----------------------------------------------------------------------------
     * FUNCTION      getValidBSSIDs
     *
     * DESCRIPTION   get valid bssid from bssidSet and put them into validSet
     *
     * DEPENDENCIES  None
     *
     * RETURN VALUE  void
     *
     * SIDE EFFECTS  None
     *--------------------------------------------------------------------------*/

    void getValidBSSIDs(const BssidSetType &bssidSet, BssidSetType &validSet);
};

#endif /* _CneSupplicantWrapper_h_ */
