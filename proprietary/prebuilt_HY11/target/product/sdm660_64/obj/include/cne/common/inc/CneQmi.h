
#ifndef CNE_QMI_H
#define CNE_QMI_H
/**
 * @file CneQmi.h
 *
 *
 * ============================================================================
 *             Copyright (c) 2011-2020 Qualcomm Technologies, Inc.
 *             All Rights Reserved.
 *             Confidential and Proprietary - Qualcomm Technologies, Inc.
 * ============================================================================
 */

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <stdlib.h>
#include <CneSrmDefs.h>
#include <CneSrm.h>
#include "CneQmiSvc.h"
#include "CneQmiDsd.h"
#include "CneQmiDpm.h"
#include "CneQmiDms.h"
#include "CneQmiWds.h"
#include "CneQmiImsa.h"
#include <string>
#include <pthread.h>
#include <queue.h>
#include <unordered_map>
#include <set>
#include <memory>
#include <functional>
#include "CneCom.h"
#include "CneDefs.h"
#include "Call.h"
#include "SubManagerDef.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "qmi_cci_target_ext.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "qmi_client.h"
#include "qmi_cci_target_ext.h"
#include "data_system_determination_v01.h"
#include "data_port_mapper_v01.h"
#include "device_management_service_v01.h"
#include "wireless_data_service_v01.h"
#ifdef __cplusplus
}
#endif
#pragma GCC diagnostic pop

//WARNING This should never be defined as 1 when committing
#define CNE_QMI_SANITY 0

class WmsInterface;

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#ifndef NELEM
#define NELEM(x) (sizeof(x)/sizeof*(x))
#endif

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#ifndef CNE_QMI_CLIENT_INIT_TIMEOUT
#define CNE_QMI_CLIENT_INIT_TIMEOUT 1000
#endif

using namespace std;

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/
/**
 * @brief
 * CneQmi events type.
 * TODO: handle event registrations for each QMI port separately and
 * dispatch events as appropriate
 */
typedef enum {
  QMI_EVENT_MIN = 0,
  QMI_EVENT_DSD_UP,
  QMI_EVENT_MAX
} CneQmiEventType;

/*----------------------------------------------------------------------------
 * Class Definitions
 * -------------------------------------------------------------------------*/

class CneQmi: public EventDispatcher<CneQmiEventType> {
  public:
      typedef enum
      {
        DPM = 0,
        DMS = 1,
        DSD_PRIMARY = 2,
        DSD_SECONDARY = 3,
        WDS = 4,
        VOICE = 5,
        IMSA_PRIMARY = 6,
        IMSA_SECONDARY = 7
      } cne_qmi_enum_type;

      static const uint32_t INSTANCE_ANY = 0xFFFF;
      /**
       * @brief Constructor
       */
      CneQmi (CneSrm &setSrm, CneCom &setCneCom, CneTimer &setTimer);
      /**
       * @brief Destructor
       */
      ~CneQmi (void);
      /**
       *  @brief Init QMI service for CNE.
       *  @return None
       */
      void init (void);

      void sendWifiAvailableStatus(dsd_wlan_available_req_msg_v01 &status);

      void sendWqeProfileStatus(dsd_set_wqe_profile_quality_status_req_msg_v01 &status);

      void sendWifiMeasurementReport(dsd_wifi_meas_report_req_msg_v01 &report);

      void stopWifiMeasurement(uint32_t meas_id);

      void sendNotifyDataSettings(dsd_notify_data_settings_req_msg_v01 &dataSettingsReq);

      CneSrm& getSrm();

      CneTimer& getTimer();

      bool setLoActivity(dsd_wwan_activity_enum_type_v01 loActivity);
      void registerQosListener(std::function<void(sp<Call>)> f);

      void setDataLowLatencyLevel
      (
        const int subId,
        const int uplink,
        const int downlink,
        const bool enableConnectionExtension
      );

      void setDataLowLatencyPriorityData(bool isPriority);

      void resetDataLowLatencyLevelAndPriorityData
      (
        const int slotId,
        const int uplink,
        const int downlink,
        bool isPriority,
        bool enableConnectionExtension
      );

      bool setDynamicDDSSwitch(bool isEnabled);

      bool clearAppPreferences();

      bool getAppList();

      bool getAppPreference(string appName, AppPreference& preference);

      bool configAppPreference(string appName, AppPreference preference);

      void reportImsRegistrationState();

      // Listeners for Qmi
      void registerServiceUpCb(cne_qmi_enum_type qmi_service,
                               shared_ptr<function<void()>> cb_function);
      void releaseServiceUpCb(cne_qmi_enum_type qmi_service,
                              shared_ptr<function<void()>> cb_function);

      void registerServiceDownCb(cne_qmi_enum_type qmi_service,
                                 shared_ptr<function<void()>> cb_function);
      void releaseServiceDownCb(cne_qmi_enum_type qmi_service,
                                shared_ptr<function<void()>> cb_function);

      void registerRespListener(cne_qmi_enum_type qmi_service,
                                shared_ptr<function<void(sp<QmiResponse>)>> listener);
      void releaseRespListener(cne_qmi_enum_type qmi_service,
                               shared_ptr<function<void(sp<QmiResponse>)>> listener);

      void registerIndListener(cne_qmi_enum_type qmi_service,
                               shared_ptr<function<void(sp<QmiIndication>)>> listener);
      void releaseIndListener(cne_qmi_enum_type qmi_service,
                              shared_ptr<function<void(sp<QmiIndication>)>> listener);

      void registerCurrentDDSListener(shared_ptr<function<void(SubscriptionType)>> listener);
      void releaseCurrentDDSListener(shared_ptr<function<void(SubscriptionType)>> listener);

      void registerClearAppPreferencesListener(shared_ptr<function<void(bool)>> listener);
      void releaseClearAppPreferencesListener(shared_ptr<function<void(bool)>> listener);

      void registerGetAppListListener(shared_ptr<function<void(sp<AppList>)>> listener);
      void releaseGetAppListListener(shared_ptr<function<void(sp<AppList>)>> listener);

      void registerConfigAppPreferenceListener(shared_ptr<function<void(sp<ConfigResult>)>> listener);
      void releaseConfigAppPreferenceListener(shared_ptr<function<void(sp<ConfigResult>)>> listener);

      void sendRequest(cne_qmi_enum_type qmi_service, uint32_t messageId, vector<uint8_t>& qmiArgs,
                       SubscriptionType sub = SubscriptionType::NOT_SPECIFIED,
                       uint32_t instanceId = INSTANCE_ANY, string ifname = "", void* cb = nullptr);

/**
 * @brief Private class to track each QMI port wds connection
 */

  private:
      /**private copy constructor* - no copies allowed */
      CneQmi (const CneQmi &);

      /** reference to SRM */
      CneSrm &srm;

      CneCom &com;

      CneTimer &timer;

      qmi_client_type qmi_client_hndl;

      unordered_map <int, CneQmiSvc*> qmiObjMap;

      map <int, set<shared_ptr<function<void()>>>> qmiServiceUpCb;
      map <int, set<shared_ptr<function<void()>>>> qmiServiceDownCb;

      static WmsInterface *wmsInst;
      static qmi_client_os_params os_params;

      int errorFd;

      /**
       * @brief SRM event handler wrapper
       *
       * @return None
       */
      static void srmEventHandler
      (
        SrmEvent    event,
        const void  *event_data,
        void    *user_data
      );

      /**
       * @brief processes SRM events
       *
       * @return None
       */
      void processSrmEvent (SrmEvent event, const void *event_data);

      static CneQmi *qmiSelf;

      static void cneQmiErrorCb
      (
        qmi_client_type userHandle,
        qmi_client_error_type error,
        void *err_cb_data
      );

      static void handleQmiSSR(int fd,void *arg);

      static void onDsdSvcUp(SvcUpEvent, const void *event_data, void *cb_data);
      static void onDmsSvcUp(SvcUpEvent, const void *event_data, void *cb_data);
      static void onDpmSvcUp(SvcUpEvent, const void *event_data, void *cb_data);
      static void onVoiceSvcUp(SvcUpEvent, const void *event_data, void *cb_data);
      static void onDsdSecondarySvcUp(SvcUpEvent, const void *event_data, void *cb_data);
      static void onWdsSvcUp(SvcUpEvent, const void *event_data, void *cb_data);
      static void onImsaSvcUp(SvcUpEvent, const void *event_data, void *cb_data);
      static void onImsaSecondarySvcUp(SvcUpEvent, const void *event_data, void *cb_data);
      void dumpReport(dsd_wifi_meas_report_req_msg_v01 &report);

      WmsInterface* loadWms(void);

      void processQmiCb (int qmi_service);

      void processModemDownCb (int qmi_service);
};


class QmiResponse : public RefBase {
public:
    typedef enum {
      OK = 0,
      MESSAGE_NOT_SUPPORTED = 1,
      FAILED = 2,
    } StatusCode;

    StatusCode status;
    string errorMessage;
    vector<uint8_t> payload;
    void* cb;

    QmiResponse() {};
    ~QmiResponse() {};
};

class QmiIndication : public RefBase {
public:
    vector<uint8_t> payload;
    void* cb;

    QmiIndication() {};
    ~QmiIndication() {};
};
#endif /* CNE_QMI_H */
