/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#define LOG_NDEBUG 0
#define LOG_TAG "LocSvc_LBSProxy"

#include <unistd.h>
#include <dlfcn.h>
#ifdef _HAS_LOC_V02_
#include <LBSApiV02.h>
#elif _HAS_LOC_RPC_
#include <LBSApiRpc.h>
#endif
#include <LBSProxy.h>
#include <loc_pla.h>
#include <loc_cfg.h>
#include <LBSAdapterBase.h>
#include <loc_target.h>

#ifdef MODEM_POWER_VOTE
#include <pm-service.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <mdm_detect.h>
#ifdef PDMAPPER_AVAILABLE
#include "libpdmapper.h"
#endif /*PDMAPPER_AVAILABLE*/
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*MODEM_POWER_VOTE*/


using namespace loc_core;
using namespace lbs_core;

namespace lbs_core {

pthread_mutex_t LBSProxy::mLock = PTHREAD_MUTEX_INITIALIZER;
LocAdapterBase* LBSProxy::mAdapter = NULL;
unsigned long LBSProxy::mCapabilities = 0;
const char * LBSProxy::COM_QUALCOMM_LOCATION_APK_FILE_PATH =
    "/system/priv-app/com.qualcomm.location/com.qualcomm.location.apk";
const char * LBSProxy::FFOS_LOCATION_EXTENDED_CLIENT =
    "/system/b2g/distribution/bundles/location/location.so";
const bool LBSProxy::mLocationExtendedClientExists =
    LBSProxy::checkIfLocationExtendedClientExists();
const bool LBSProxy::mNativeXtraClientExists =
    LBSProxy::checkIfNativeXtraClientExists();

#define SLL_API_PDS_LIB_NAME "libsynergy_loc_api.so"
#define IZAT_API_PDS_LIB_NAME "libizat_api_pds.so"
#define IS_SS5_HW_ENABLED  1
#define MAX_ELEMENTS_OF_GPS_CONF (1)

#ifdef MODEM_POWER_VOTE
typedef struct {
    //MAX_NAME_LEN defined in mdm_detect.h
    char modem_name[MAX_NAME_LEN];
    //MAX_PATH_LEN defined in mdm_detect.h
    char powerup_node[MAX_PATH_LEN];
    //this handle is used by peripheral mgr
    void *handle;
    int mdm_fd;
    MdmType mdm_type;
    bool peripheral_mgr_supported;
    bool peripheral_mgr_registered;
}s_loc_mdm_info;
static s_loc_mdm_info loc_mdm_info;
static void pm_event_notifier(void *client_data, enum pm_event event);
// For shutting down MDM in fusion devices
static int mdm_fd = -1;
#endif /*MODEM_POWER_VOTE*/

LocApiBase* LBSProxy::getLocApi(LOC_API_ADAPTER_EVENT_MASK_T exMask,
                                ContextBase *context) const {
#ifdef _HAS_LOC_V02_
    void *handle = NULL;
    const char * libname = NULL;
    uint32_t gnssDeployment  = 0;
    loc_param_s_type gps_conf_feature_table[MAX_ELEMENTS_OF_GPS_CONF] =
    {
        {"GNSS_DEPLOYMENT",  &gnssDeployment, NULL, 'n'}
        /* Update array size using MAX_ELEMENT_OF_GPS_CONF */
    };

    loc_read_conf(LOC_PATH_GPS_CONF, gps_conf_feature_table, MAX_ELEMENTS_OF_GPS_CONF);

    if (IS_SS5_HW_ENABLED == gnssDeployment) {
        libname = SLL_API_PDS_LIB_NAME;
    } else {
        libname = IZAT_API_PDS_LIB_NAME;
    }

    if ((handle = dlopen(libname, RTLD_NOW)) != NULL) {
        LOC_LOGD("%s:%d]: %s is present Now trying to load LBSProxy.",
                __func__, __LINE__, libname);
        getLocApi_t* getter = (getLocApi_t*)dlsym(handle, "getLocApi");
        if(getter != NULL) {
            LocApiBase* locApi = (*getter)(exMask, context);
            if ( locApi != NULL ) {
                return locApi;
            }
        }
    }
    return (LocApiBase*)LBSApiV02::createLBSApiV02(exMask, context);
#elif _HAS_LOC_RPC_
    return (LocApiBase*)LBSApiRpc::createLBSApiRpc(exMask, context);
#endif
    return NULL;
}

#ifdef MODEM_POWER_VOTE
static void pm_event_notifier(void* /*client_data*/, enum pm_event event)
{
    ENTRY_LOG();
    LOC_LOGD("%s:%d]: event: %d", __func__, __LINE__, (int)event);
    pm_client_event_acknowledge(loc_mdm_info.handle, event);
    EXIT_LOG(%s, VOID_RET);
}

#ifdef PDMAPPER_AVAILABLE
void LBSProxy::parseModemRegistryForGpsService(int* pModemType) const {
    /*
        A sample of modemr.jsn is below:
        {
            "sr_version": {
                "major": 1,
                "minor": 1,
                "patch": 1
            },
            "sr_domain": {
                "soc": "msm",
                "domain": "modem",
                "subdomain": "root_pd",
                "qmi_instance_id": 180
            },
            "sr_service": [
                {
                    "provider": "tms",
                    "service": "servreg",
                    "service_data_valid": 0,
                    "service_data": 0
                },
                {
                    "provider": "gps",
                    "service": "gps_service",
                    "service_data_valid": 0,
                    "service_data": 0
                }
            ]
        }

        We look for gps/gps_service string and if found
        we sniff at 'soc' - then "msm" = intrenal modem
        "mdm" = external modem
    */
    SR_DL_Handle *handle = NULL;
    int num_providers = 0;
    int instance_id = 0;
    char *notifier_string = NULL;
    SR_Result_Enum rc = SR_RESULT_SUCCESS;
    char const *service_name = "gps/gps_service";
    char* soc = NULL;
    char* saveptr = NULL;
    char const *internal_modem = "msm";
    char const *external_modem = "mdm";

    do {
        handle = servreg_alloc_DLHandle();
        if (!handle) {
            LOC_LOGd("Failed to allocate dlhandle");
            break;
        }
        rc = servreg_get_domainlist((char*)service_name, handle);
        if (rc != SR_RESULT_SUCCESS) {
            LOC_LOGe("servreg_get_domainlist failed with error: %d", rc);
            break;
        }
        num_providers = servreg_get_numentries(handle);
        if (num_providers <= 0) {
            LOC_LOGe("Error while getting num providers for %s",
                service_name);
            break;
        }
        else if (num_providers > 1) {
            LOC_LOGd("More than one provider for %s, rely on other methods",
                service_name);
            break;
        }
        LOC_LOGd("%s is provided by %d domains", service_name, num_providers);
        rc = servreg_get_data_for_notifier(handle,
            &notifier_string,
            &instance_id,
            0);
        if (rc != SR_RESULT_SUCCESS ||
            nullptr == notifier_string) {
            LOC_LOGe("Failed to get data for notifier");
            break;
        }
        LOC_LOGi("Notifier_string is : :%s:", notifier_string);

        soc = strtok_r(notifier_string, "/", &saveptr);
        if (NULL != soc && NULL != pModemType) {
            LOC_LOGi("soc is %s", soc);
            if (0 == strncmp(soc, internal_modem, strlen(internal_modem))) {
                LOC_LOGi("Internal modem");
                *pModemType = 1;
            }
            else if (0 == strncmp(soc, external_modem, strlen(external_modem))) {
                LOC_LOGi("External modem");
                *pModemType = 0;
            }
            else {
                LOC_LOGi("Unknown modem");
            }
        }
    } while (0);
    if (handle) {
        servreg_free_DLHandle(handle);
    }
    if (notifier_string) {
        free(notifier_string);
    }
}
#endif /*PDMAPPER_AVAILABLE*/
#endif /*MODEM_POWER_VOTE*/

void LBSProxy::modemPowerVote(bool power) const {

#ifdef MODEM_POWER_VOTE
    if (power) {
        enum pm_event mdm_state;
        static int mdm_index = -1;
        int peripheral_mgr_ret = PM_RET_FAILED;
        int modem_type = 2; // 0=external, 1=internal, 2=unknown

        //if index is 0 or more, then we've looked for mdm already
        LOC_LOGD("%s:%d]: mdm_index: %d", __func__, __LINE__,
                 mdm_index);
        if (mdm_index < 0) {
#ifdef PDMAPPER_AVAILABLE
            /* In case there is more than one modem in the system we need
               to identify which one has GPS, so we can power-vote for it.
               We first use servreg_get_data_for_notifier to get this information
               from the jsn file. In case this fails we fall back on the setting
               in gps.conf. Finally if that fails as well we vote for the internal
               modem
            */
            parseModemRegistryForGpsService(&modem_type);
#endif /*PDMAPPER_AVAILABLE*/

            struct dev_info modem_info;
            memset(&modem_info, 0, sizeof(struct dev_info));
            if (get_system_info(&modem_info) != RET_SUCCESS) {
                LOC_LOGE("%s:%d]: Error: get_system_info returned error",
                         __func__, __LINE__);
                return;
            }

            for (mdm_index = 0;
                mdm_index < modem_info.num_modems;
                mdm_index++) {

                /* if modem_type is 2 here (unknown) it means parsing .jsn file
                   failed and we have to rely on gps.conf setting if any */
                if (2 == modem_type) {
                    modem_type = loc_core::ContextBase::mGps_conf.MODEM_TYPE;
                }
                // Check if mdm_name contains a valid value
                // modem_type can be unknown (not set in gps.conf) = 2
                // in which case we just vote for the first modem in the list
                // OR it could be external(0) or internal(1), in which case
                // we vote for the modem in the list with respective type
                if((modem_info.mdm_list[mdm_index].mdm_name[0] != '\0') &&
                    ((modem_type > 1) ||
                     (modem_type == modem_info.mdm_list[mdm_index].type))) {

                    //Copy modem name to register with peripheral manager
                    strlcpy(loc_mdm_info.modem_name,
                            modem_info.mdm_list[mdm_index].mdm_name,
                            sizeof(loc_mdm_info.modem_name));
                    //copy powerup node name if we need to use mdmdetect method
                    strlcpy(loc_mdm_info.powerup_node,
                            modem_info.mdm_list[mdm_index].powerup_node,
                            sizeof(loc_mdm_info.powerup_node));
                    loc_mdm_info.mdm_type = modem_info.mdm_list[mdm_index].type;
                    LOC_LOGD("%s:%d]: Found modem: %s, powerup node:%s at index: %d",
                             __func__, __LINE__, loc_mdm_info.modem_name, loc_mdm_info.powerup_node,
                             mdm_index);
                    break;
                }
            }
        }
        if (loc_mdm_info.peripheral_mgr_registered != true) {
            peripheral_mgr_ret = pm_client_register(pm_event_notifier,
                                                    &loc_mdm_info,
                                                    loc_mdm_info.modem_name,
                                                    "GPS",
                                                    &mdm_state,
                                                    &loc_mdm_info.handle);
            if (peripheral_mgr_ret == PM_RET_SUCCESS) {
                loc_mdm_info.peripheral_mgr_supported = true;
                loc_mdm_info.peripheral_mgr_registered = true;
                LOC_LOGD("%s:%d]: registered with peripheral mgr for %s",
                         __func__, __LINE__, loc_mdm_info.modem_name);
            }
            else if (peripheral_mgr_ret == PM_RET_UNSUPPORTED) {
                loc_mdm_info.peripheral_mgr_registered = true;
                loc_mdm_info.peripheral_mgr_supported = false;
                LOC_LOGD("%s:%d]: peripheral mgr unsupported for: %s",
                         __func__, __LINE__, loc_mdm_info.modem_name);
            }
            else {
                //Not setting any flags here so that we can try again the next time around
                LOC_LOGE("%s:%d]: Error: pm_client_register returned: %d",
                         __func__, __LINE__, peripheral_mgr_ret);
            }
        }

        if (loc_mdm_info.peripheral_mgr_supported == false &&
            loc_mdm_info.peripheral_mgr_registered == true) {
            //Peripheral mgr is not supported
            //use legacy method to open the powerup node
            LOC_LOGD("%s:%d]: powerup_node: %s", __func__, __LINE__,
                     loc_mdm_info.powerup_node);
            loc_mdm_info.mdm_fd = open(loc_mdm_info.powerup_node, O_RDONLY);

            if (loc_mdm_info.mdm_fd < 0) {
                LOC_LOGE("Error: %s open failed: %s\n",
                         loc_mdm_info.powerup_node, strerror(errno));
            } else {
                LOC_LOGD("%s opens success!", loc_mdm_info.powerup_node);
            }
        } else if(loc_mdm_info.peripheral_mgr_supported == true &&
                  loc_mdm_info.peripheral_mgr_registered == true) {
            LOC_LOGD("%s:%d]: Voting for modem power up", __func__, __LINE__);
            pm_client_connect(loc_mdm_info.handle);
        } else {
            LOC_LOGD("%s:%d]: Not voted for modem power up due to errors", __func__, __LINE__);
        }

    } else {
        if (loc_mdm_info.peripheral_mgr_supported == true) {
            LOC_LOGD("%s:%d]: Voting for modem power down", __func__, __LINE__);
            pm_client_disconnect(loc_mdm_info.handle);
        }
        else if (loc_mdm_info.mdm_fd >= 0) {
            LOC_LOGD("closing the powerup node");
            close(loc_mdm_info.mdm_fd);
            loc_mdm_info.mdm_fd = -1;
            LOC_LOGD("finished closing the powerup node");
        }
        else {
            LOC_LOGD("powerup node has not been opened yet.");
        }
    }
#endif /*MODEM_POWER_VOTE*/

}

bool LBSProxy::checkIfLocationExtendedClientExists()
{

    // To differentiate between PDK and non PDK builds
    if((access(COM_QUALCOMM_LOCATION_APK_FILE_PATH, F_OK) != -1) ||
        (access(FFOS_LOCATION_EXTENDED_CLIENT, F_OK) != -1)) {
        ALOGD("%s] %s",__func__, "File Exists");
        return true;
    } else {
        ALOGD("%s] %s",__func__, "File does not Exist");
        return false;
    }
}

bool LBSProxy::checkIfNativeXtraClientExists()
{
    return access("/vendor/bin/xtra-daemon", // executable of XTRA Client 2.0
                  F_OK) != -1;
}

void LBSProxy::injectFeatureConfig(ContextBase* context) const
{
    class InjectAdapter:public LBSAdapterBase {
    public:
        InjectAdapter(ContextBase *context):
            LBSAdapterBase(0, context) {}

        virtual void injectFeatureConfig()
        {
            e_premium_mode sap_mode=DISABLED;
            e_premium_mode gtp_cell_mode=DISABLED;
            e_premium_mode gtp_wifi_mode=DISABLED;
            // As this is read using loc cfg routine, the buffer size
            // need to conform to what is supported by loc cfg module. */
            char conf_feature_gtp_mode[LOC_MAX_PARAM_STRING];
            char conf_feature_sap[LOC_MAX_PARAM_STRING];
            loc_param_s_type izat_conf_feature_table[] =
            {
                {"GTP_MODE",      &conf_feature_gtp_mode,      NULL, 's'},
                {"SAP",           &conf_feature_sap,           NULL, 's'}
            };
            UTIL_READ_CONF(LOC_PATH_IZAT_CONF, izat_conf_feature_table);

            if(strcmp(conf_feature_gtp_mode, "LEGACY_WWAN") == 0) {
                LOC_LOGd("GTP MODE mode set to LEGACY_WWAN");
                gtp_cell_mode = BASIC;
                gtp_wifi_mode = DISABLED;
            }
            else if(strcmp(conf_feature_gtp_mode, "DISABLED") == 0) {
                LOC_LOGd("GTP MODE mode set to DISABLED");
                gtp_cell_mode = DISABLED;
                gtp_wifi_mode = DISABLED;
            }
            else if(strcmp(conf_feature_gtp_mode, "SDK") == 0) {
                LOC_LOGd("GTP MODE mode set to SDK");
                gtp_cell_mode = THIRDPARTY;
                gtp_wifi_mode = BASIC;
            }
            else {
                LOC_LOGe("Unrecognized value for GTP CELL Mode." \
                         " Setting GTP MODE to default mode: DISABLED");
                gtp_cell_mode = DISABLED;
                gtp_wifi_mode = DISABLED;
            }

            //SAP
            char socid_value[LOC_MAX_PARAM_STRING];
            // special case to disable SAP based on Soc Id
            #define SOC_ID_QM215 386
            #define SOC_ID_QM2150 436
            loc_get_device_soc_id(socid_value, sizeof(socid_value));
            int soc_id = atoi(socid_value);
            if ((SOC_ID_QM215 == soc_id) || (SOC_ID_QM2150 == soc_id)) {
                LOC_LOGd("Setting SAP to mode: DISABLED for soc_id %d", soc_id);
                sap_mode = DISABLED;
            } else {
                if (strcmp(conf_feature_sap, "BASIC") == 0) {
                    sap_mode = BASIC;
                    LOC_LOGd("Setting SAP to mode: BASIC");
                }
                else if (strcmp(conf_feature_sap, "PREMIUM") == 0) {
                    LOC_LOGd("Setting SAP to mode: PREMIUM");
                    sap_mode = PREMIUM;
                }
                else if (strcmp(conf_feature_sap, "DISABLED") == 0) {
                    LOC_LOGd("Setting SAP to mode: DISABLED");
                }
                else if (strcmp(conf_feature_sap, "MODEM_DEFAULT") == 0) {
                    LOC_LOGd("Setting SAP to mode: MODEM_DEFAULT");
                    sap_mode = MODEM_DEFAULT;
                }
                else {
                    LOC_LOGe("Unrecognized value for SAP Mode.Setting to default mode: BASIC");
                    sap_mode = BASIC;
                }

                LOC_LOGd("Setting SAP mLBSApi: %p", mLBSApi);

                mLBSApi->injectFeatureConfig(sap_mode, gtp_cell_mode, gtp_wifi_mode,
                        new LocApiResponse(*getContext(), [adapter = this] (LocationError /*err*/) {
                    delete adapter;
                }));
            }
        }
    };

    struct LBSProxyInjectFeatureConfig : public LocMsg {
        InjectAdapter *mAdapter;
        inline LBSProxyInjectFeatureConfig(ContextBase *context) :
                LocMsg()
        {
            locallog();
            mAdapter = new InjectAdapter(context);
        }

        inline virtual ~LBSProxyInjectFeatureConfig() {
        }
        virtual void proc() const {
            mAdapter->injectFeatureConfig();
        }
        virtual void locallog()
        {
            LOC_LOGD("%s:%d]:LBSProxyInjectFeatureConfig", __func__, __LINE__);
        }
    };
    LOC_LOGD("%s:%d]: Enter", __func__, __LINE__);
    context->sendMsg(new LBSProxyInjectFeatureConfig(context));
    LOC_LOGD("%s:%d]: Exit", __func__, __LINE__);
}
}  // namespace lbs_core

extern "C" {
LBSProxyBase* getLBSProxy() {
    return new lbs_core::LBSProxy();
}
}
