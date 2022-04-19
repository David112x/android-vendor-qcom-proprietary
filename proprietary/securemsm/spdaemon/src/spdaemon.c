/*=============================================================================
Copyright (c) 2016-2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

/**
 * @file spdaemon.c
 * @brief - Secure Processor Daemon (spdaemon)
 *
 * This driver is responsible for loading SP Applications that
 * doesn't have an owner HLOS Application.
 */

/*-------------------------------------------------------------------------
 * Include Files
 * ----------------------------------------------------------------------*/
#include <stdlib.h>    // malloc()
#include <stdio.h>     // fopen()
#include <fcntl.h>     // O_RDONLY
#include <unistd.h>    // sleep() / usleep()
#include <errno.h>     // ENODEV
#include <memory.h>
#include <pthread.h>
#include <time.h>

#include <log/log.h>        // SLOGE()

#include <spcomlib.h>
#include <sp_uim_remote.h>
#include "mdm_detect.h"
#include "pm-service.h"
#include <cutils/properties.h> // property_get()

#include "sp_iar_lib.h"

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * ----------------------------------------------------------------------*/
#ifdef PRINT_LOG_TO_STDOUT
    #define LOGD(fmt, x...) printf("spdaemon: dbg: %s: " fmt "\n", __func__,##x)
    #define LOGI(fmt, x...) printf("spdaemon: dbg: %s: " fmt "\n", __func__,##x)
    #define LOGE(fmt, x...) printf("spdaemon: err: %s: " fmt "\n", __func__,##x)
#else // print to system log a.k.a logcat
    #undef LOG_TAG
    #undef LOGD
    #undef LOGI
    #undef LOGE
    #define LOG_TAG "spdaemon"
    #define LOGD SLOGD
    #define LOGI SLOGI
    #define LOGE SLOGE
#endif

#define SZ_1K (1024)

#ifndef OFFSET_OF
    #define OFFSET_OF(field, base) ((int)((char*)(field) - (char*)(base)))
#endif

#define FILE_PATH_SIZE 256

#define SP_APP_LOAD_FLAG_NONE           0x00000000
#define SP_APP_LOAD_FLAG_OPTIONAL       0x00000001
#define SP_APP_LOAD_FLAG_SILENT_LOAD    0x01000002
#define SP_APP_LOAD_FLAG_UEFI           0x00000004
#define SP_APP_LOAD_FLAG_SOC_MSM8998    0x00010000
#define SP_APP_LOAD_FLAG_SOC_SDM845     0x00020000
#define SP_APP_LOAD_FLAG_SOC_SM8150     0x00040000
#define SP_APP_LOAD_FLAG_SOC_SM8250     0x00080000

/*-------------------------------------------------------------------------
 * Structures and enums
 * ----------------------------------------------------------------------*/

typedef struct _sp_app_info_t {
    const char*     ch_name;
    const char*     base_name; // FAT 8.3 chars format
    int             swap_size;
    uint32_t        id;
    uint32_t        flags;
} sp_app_info_t;

typedef enum _spss_firmware_t {
    SPSS_FW_TYPE_DEV = 'd',
    SPSS_FW_TYPE_TEST = 't',
    SPSS_FW_TYPE_PROD = 'p',
} spss_firmware_t;

typedef enum _spss_soc_t {
    SOC_MSM8998 = SP_APP_LOAD_FLAG_SOC_MSM8998,
    SOC_SDM845  = SP_APP_LOAD_FLAG_SOC_SDM845,
    SOC_SM8150  = SP_APP_LOAD_FLAG_SOC_SM8150,
    SOC_SM8250  = SP_APP_LOAD_FLAG_SOC_SM8250,
} spss_soc_t;

// Note: The request and response structure format should be packed

/*-------------------------------------------------------------------------
 * Global Variables
 * ----------------------------------------------------------------------*/

static spss_firmware_t g_firmware_type = SPSS_FW_TYPE_TEST;

// SPSS Apps
static sp_app_info_t g_apps_to_load[] = {
    // This App must be loaded before crypto App
    {
        .ch_name = "asym_cryptoapp",
        .base_name = "asym",
        .swap_size = 256 * SZ_1K,
        .id = 0x3D86A887,
        .flags =
            SP_APP_LOAD_FLAG_OPTIONAL |
            SP_APP_LOAD_FLAG_SOC_SM8150 |
            SP_APP_LOAD_FLAG_SILENT_LOAD |
            SP_APP_LOAD_FLAG_UEFI,
    },

    {
        // This App should be loaded as soon as possible to
        // handle all the subsequent service requests
        .ch_name = "cryptoapp",
        .base_name = "crypt",
        .swap_size = 256 * SZ_1K,
        .id = 0xEF80345E,
        .flags =
            SP_APP_LOAD_FLAG_SOC_SDM845 |
            SP_APP_LOAD_FLAG_SOC_SM8150 |
            SP_APP_LOAD_FLAG_UEFI,
    },

    {
        .ch_name = "sp_keymaster",
        .base_name = "keym",
        .swap_size = 256 * SZ_1K,
        .id = 0x24500E2F,
        .flags =
            SP_APP_LOAD_FLAG_SOC_SDM845 |
            SP_APP_LOAD_FLAG_SOC_SM8150 |
            SP_APP_LOAD_FLAG_UEFI,
    },

    {
        .ch_name = "macchiato",
        .base_name = "macch",
        .swap_size = 256 * SZ_1K,
        .id = 0x09068E40,
        .flags =
            SP_APP_LOAD_FLAG_SOC_SDM845 |
            SP_APP_LOAD_FLAG_SOC_SM8250,
    },
};

// The spss_utils kernel driver sysfs path depends on the linux kernel version
static const char* g_firmware_name_path =  "./sys/devices/platform/soc/soc:qcom,spss_utils/firmware_name";
static const char* g_firmware_name_path2 = "./sys/devices/soc/soc:qcom,spss_utils/firmware_name";

static const char* g_iar_db_path = "/mnt/vendor/persist/iar_db/";

static struct spcom_client* g_ssr_spcom_client = NULL;

static pthread_mutex_t  g_pthread_mutex;
static pthread_cond_t   g_pthread_cond;

#ifndef SPCOM_CMD_RESTART_SP
    static void* g_pm_spss_handle = NULL;
#endif // !SPCOM_CMD_RESTART_SP

static const char*  g_machine_name_path = "/sys/devices/soc0/machine";
static char         g_machine_name[20] = {}; // i.e. "MSM8998" , "SDM845", "SM8150"

static spss_soc_t   g_soc_type = SOC_SM8150;

static int          g_soc_version = 0;

// wait for sec_nvm to finish first interaction with SPU - only then load apps
// Reason: both use sp_kernel channel. Also - apps might need nvm services
static const char*  WAIT_FOR_SECNVM_PROP = "vendor.spcom.sec_nvm_init";
static const char*  WAIT_FOR_SPDAEMON_PROP = "vendor.spcom.spdaemon_init";

static int g_thread_resumed = 0;

/*-------------------------------------------------------------------------
 * Function Implementations
 * ----------------------------------------------------------------------*/

static inline void msleep(int msec)
{
    usleep(msec * 1000);
}

static char* get_machine_name(void)
{
    int fd;
    int ret;
    char* p;

    fd = open(g_machine_name_path, O_RDONLY);
    if (fd < 0) {
        LOGD("open() file [%s] Failed, ret =%d.\n", g_machine_name_path, fd);
        return NULL;
    }

    ret = read(fd, g_machine_name, sizeof(g_machine_name) - 1);
    if (ret < 0) {
        LOGE("read() file [%s] Failed, ret =%d.\n", g_machine_name_path, ret);
        return NULL;
    }

    //Zero the last byte of the array to make sure the string is valid
    g_machine_name[sizeof(g_machine_name) - 1] = 0;

    p = strstr(g_machine_name,"\n");
    if (p != NULL)
        *p = 0; // remove new line char

    LOGD("machine_name [%s]\n", g_machine_name);

    return g_machine_name;
}

static spss_soc_t get_soc_type(void)
{
    char* name;

    name = get_machine_name();
    if (name == NULL)
        return SOC_SM8150; // default

    if (strstr(name, "8998"))
        return SOC_MSM8998;

    if (strstr(name, "845"))
        return SOC_SDM845;

    if (strstr(name, "SM8150"))
        return SOC_SM8150;

    if (strstr(name, "SM8250"))
        return SOC_SM8250;

    //default
#if (SPSS_TARGET == 8150)
    return SOC_SM8150;
#elif (SPSS_TARGET == 8250)
    return SOC_SM8250;
#endif
}

static void get_app_file_path(char* path, const char* base_name, spss_soc_t soc, spss_firmware_t fw_type, int soc_version)
{
    const char* base_path = "/vendor/firmware_mnt/image/";
    char type[2] = {0};

    type[0]= (char) fw_type;

    if (iar_is_iar_state_activated()) {
        strlcpy(path, g_iar_db_path, FILE_PATH_SIZE);
    } else {
        strlcpy(path, base_path, FILE_PATH_SIZE);
    }

    strlcat(path, base_name, FILE_PATH_SIZE);
    if (soc == SOC_MSM8998) {
        strlcat(path, "_", FILE_PATH_SIZE);
    } else {
        if (soc_version == 1)
            strlcat(path, "1", FILE_PATH_SIZE);
        else
            strlcat(path, "2", FILE_PATH_SIZE);
    }

    strlcat(path, type, FILE_PATH_SIZE);
    strlcat(path, ".sig", FILE_PATH_SIZE);
}

static spss_firmware_t get_firmware_type(void)
{
    int fd;
    int ret;
    char name[10] = {}; // up to 8 chars , example "spss2t".
    const char* path = g_firmware_name_path;
    char soc_ver;
    char fw_type;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        LOGD("open() file [%s] Failed, ret [%d] try another path.\n", path, fd);
        path = g_firmware_name_path2; // Alternative path

        fd = open(path, O_RDONLY);
        if (fd < 0) {
            LOGD("open() file [%s] Failed, ret =%d.\n", path, fd);
            return SPSS_FW_TYPE_TEST;
        }
    }

    ret = read(fd, name, sizeof(name) - 1);
    if (ret < 0) {
        LOGE("read() file [%s] Failed, ret =%d.\n", path, ret);
    } else {
        char* p;

        //Zero the last byte of the array to make sure the string is valid
        name[sizeof(name) - 1] = 0;

        p = strstr(name, "\n");
        if (p != NULL)
            *p = 0; // remove new line char
        LOGD("Firmware Name [%s].\n", name);
        if (strstr(name, "spss") == 0) {
            LOGE("Not SPSS firmware name.\n");
            return SPSS_FW_TYPE_TEST;
        }

        soc_ver = name[4];
        fw_type = name[5];

        if (soc_ver == '1' || soc_ver == '2')
            g_soc_version = soc_ver - '0';

        if (fw_type == 'd') {
            LOGD("Dev FW.\n");
            return SPSS_FW_TYPE_DEV;
        }
        if (fw_type == 't') {
            LOGD("Test FW.\n");
            return SPSS_FW_TYPE_TEST;
        }
        if (fw_type == 'p') {
            LOGD("Prod FW.\n");
            return SPSS_FW_TYPE_PROD;
        }
    }

    return SPSS_FW_TYPE_TEST;
}

static void suspend_me(void)
{
    pthread_mutex_lock(&g_pthread_mutex);

    // wait for a flag here
    // If resume_me() is called before suspend_me() we will release the wait immediately
    while (g_thread_resumed == 0) {
        pthread_cond_wait(&g_pthread_cond, &g_pthread_mutex);
    }

    //reset the flag here to prepare for the next ssr
    g_thread_resumed = 0;
    pthread_mutex_unlock(&g_pthread_mutex);
}

static void resume_me(void)
{
    pthread_mutex_lock(&g_pthread_mutex);
    g_thread_resumed = 1;
    pthread_cond_signal(&g_pthread_cond);
    pthread_mutex_unlock(&g_pthread_mutex);
}

static int load_app(sp_app_info_t* info, struct spcom_client** service_client)
{
    int ret = 0;
    bool is_loaded = false;
    int timeout_msec = 60 * 1000;
    int time_msec = 0;
    const char* ch_name = info->ch_name;
    char file_path[FILE_PATH_SIZE] = {};

    // check if the App is already loaded
    is_loaded = spcom_is_app_loaded(ch_name);
    if (!is_loaded) {

        // Check If the App is required for current SOC
        if (!(info->flags & g_soc_type)) {
            if (info->flags & SP_APP_LOAD_FLAG_UEFI) {

                // If SP is started in UEFI stage, SP itself
                // loads the UEFI apps(cryptoapp,
                // asymm_crypto, keymaster). Load status of the
                // UEFI apps are retrieved from SP and system
                // properties are updated.
                goto update_load_status;
            } else {
                // Skip App - silently return success
                return 0;
            }
        }

        LOGD("Load SP App [%s].\n", ch_name);
        get_app_file_path(file_path, info->base_name,
                g_soc_type, g_firmware_type, g_soc_version);

        // Load the app
        ret = spcom_load_app(info->ch_name,
                file_path,
                info->swap_size);
        if (ret < 0) {
            LOGE("Loading SP App [%s] failed. ret [%d].\n", ch_name, ret);
            return ((info->flags & SP_APP_LOAD_FLAG_OPTIONAL)) ? 0 : ret;
        }

        while (!is_loaded) {
            is_loaded = spcom_is_app_loaded(ch_name);
            msleep(10);
            time_msec += 10;
            if (time_msec >= timeout_msec) {
                LOGE("Timeout wait for char dev creation.\n");
                return ((info->flags & SP_APP_LOAD_FLAG_OPTIONAL)) ? 0 : -ETIMEDOUT;
            }
        }

        LOGD("SP App [%s] is loaded successfully.\n", ch_name);
    }
    else {
        LOGD("SP App [%s] already loaded.\n", ch_name);
    }

    if ((info->flags & SP_APP_LOAD_FLAG_SILENT_LOAD)) {
        // App is loaded silently without reporting version
        return 0;
    }

    update_load_status:
        // Read App version
        if (NULL == *service_client) {
            // Since first loaded App is responsible for handling all the service
            // requests, initialize the client here and let upper layers to release
            // it after
            ret = spcom_connect_service_client(service_client);
            if (ret) {
                // Not really an error, version is an option, return success
                return 0;
            }
        }
        uint32_t build_version = (uint32_t)-1;
        uint32_t build_version_hash = (uint32_t)-1;

        if(!spcom_sp_sysparam_read_ext(SP_SYSPARAM_ID_APP_BUILD_VERSION,
                info->id, 0, &build_version, *service_client) &&
                !spcom_sp_sysparam_read_ext(SP_SYSPARAM_ID_APP_BUILD_VERSION_HASH,
                        info->id, 0, &build_version_hash, *service_client)) {

            // Apps version is not reported, if SILENT_LOAD flag is enabled
            if (!(info->flags & SP_APP_LOAD_FLAG_SILENT_LOAD)) {
                LOGI("%s, v%d.%d.%08X\n",
                    ch_name,
                    ((uint32_t)-1 == build_version) ? 0 : (build_version >> 16),
                    ((uint32_t)-1 == build_version) ? 0 : (build_version & 0xFFFF),
                    ((uint32_t)-1 == build_version_hash) ? 0 : build_version_hash);
            }

            if (!spcom_is_app_loaded(ch_name)){
                if (!spcom_set_app_load_status(ch_name))
                    LOGE("spcom_set_app_load_status failed. \n");
            }
        }
    return 0;
}

/**
 * spcom_notify_ssr_cb() - a callback to notify on SP SubSystem-Reset (SSR).
 *
 * The spcom shall notify upon SP reset.
 * This callback should wait until the SP is up again (LINK is UP),
 * and then re-load the SP Applications and do any init sequence required.
 */
static void spcom_notify_ssr_cb(void)
{
    LOGD("SP subsystem reset detected.\n");

    property_set(WAIT_FOR_SECNVM_PROP, "wait");
    property_set(WAIT_FOR_SPDAEMON_PROP, "wait");
    resume_me();
}

static int register_ssr_callback(void)
{
    struct spcom_client *client = NULL;
    struct spcom_client_info reg_info = {};
    const char* ch_name = "spdaemon_ssr"; // use known node for sepolicy

    reg_info.ch_name = ch_name;
    reg_info.notify_ssr_cb = spcom_notify_ssr_cb;

    // register to spcom for sending request
    client = spcom_register_client(&reg_info);
    if (client == NULL) {
        LOGE("spcom register failed.\n");
        return -EFAULT;
    }

    // Note1: don't unregister spcom for SSR awareness

    LOGD("SSR callback registered ok.\n");
    g_ssr_spcom_client = client;

    return 0;
}

static int unregister_ssr_callback(void)
{
    spcom_unregister_client(g_ssr_spcom_client);
    g_ssr_spcom_client = NULL;

    return 0;
}

/**
 *  Wait until SP is up and running.
 */
static int wait_for_sp_link_up(int timeout_sec)
{
    bool sp_is_up = false;
    int timeout_msec = timeout_sec * 1000;
    int time_msec = 0;

    while (!sp_is_up) {
        sp_is_up = spcom_is_sp_subsystem_link_up();
        msleep(10);
        time_msec += 10;
        if (time_msec >= timeout_msec) {
            LOGE("Timeout wait for SP link UP.\n");
            return -ETIMEDOUT;
        }
    }

    LOGD("SP LINK is UP in [%d] msec.\n", time_msec);

    return 0;
}

/**
 * Load SP Apps
 */
static int load_sp_apps(sp_app_info_t apps[], size_t apps_num, struct spcom_client** service_client)
{
    int ret = 0;
    int link_up_timeout_sec = 60;

    // Wait for link up
    LOGD("Wait for sp link up.\n");
    ret = wait_for_sp_link_up(link_up_timeout_sec);
    if (ret < 0) {
        LOGE("load_sp_apps: wait_for_sp_link_up failed. ret [%d].\n", ret);
        return ret;
    }

    // Load Apps
    for (size_t index = 0; index < apps_num; index++) {
        ret = load_app(&apps[index], service_client);
        if (ret < 0) {
            LOGE("Loading SP %s App failed. ret [%d].\n",
                g_apps_to_load[index].ch_name,
                ret);
            break;
        }
    }

    if (!ret)
      LOGD("SP Apps were loaded successfully.\n");

    return ret;
}

#ifndef SPCOM_CMD_RESTART_SP
static void pm_spss_event_notifier(void *client_data, enum pm_event event)
{
    char* client_name = (char*) client_data;

    LOGD("client [%s] got event [%d] notification", client_name , (int) event);

    // Don't really care much here about events.Just ack whatever comes in.
    pm_client_event_acknowledge(g_pm_spss_handle, event);
}

/*
 * PIL load sp
 */
static int pil_load_sp(void)
{
    int i, ret = 0;
    struct dev_info devinfo = {};
    enum pm_event event = 0;
    bool spss_found = false;
    void* client_data = "spdaemon";

    LOGD("Starting to get system info");
    ret = get_system_info(&devinfo);
    if (ret != RET_SUCCESS) {
        LOGE("Failed to get_system_info (%d)", ret);
        goto error;
    }

    LOGD("devinfo.num_additional_subsystems = %d",
            devinfo.num_additional_subsystems);
    for (i = 0; i < devinfo.num_additional_subsystems; i++) {
        LOGD("devinfo.subsystem_list[%d].type = %d",
                i, devinfo.subsystem_list[i].type);
        if (devinfo.subsystem_list[i].type == SUBSYS_TYPE_SPSS) {
            spss_found = true;
            LOGD("Found spss subsystem.");
            LOGD("devinfo.subsystem_list[%d].mdm_name = %s",
                    i , devinfo.subsystem_list[i].mdm_name);
            ret = pm_client_register(pm_spss_event_notifier,
                    client_data,
                    devinfo.subsystem_list[i].mdm_name, // "spss" dev name
                    "spdaemon", // client name
                    &event,
                    &g_pm_spss_handle);
            if (ret != PM_RET_SUCCESS) {
                LOGE("pm_client_register failed. ret [%d].\n", ret);
                goto error;
            }
            LOGD("pm-spss-thread Voting for spss subsystem");
            ret = pm_client_connect(g_pm_spss_handle);
            if (ret != PM_RET_SUCCESS) {
                LOGE("pm_client_connect() for spss fail. ret [%d].\n", ret);
                goto error;
            }
        }
    }

    if (!spss_found) {
        LOGE("SUBSYS_TYPE_SPSS not found.\n");
        goto error;
    }

    LOGD("SPSS-PIL completed successfully");

    return 0;
error:
    LOGD("SPSS-PIL failed.");
    return -EFAULT;
}
#endif // !SPCOM_CMD_RESTART_SP

static int wait_sec_nvm()
{
    char sec_nvm_done[PROPERTY_VALUE_MAX];

    property_get(WAIT_FOR_SECNVM_PROP, sec_nvm_done, NULL);
    LOGD("Start waiting for sec_nvm\n");
    while (!strncmp(sec_nvm_done, "wait", strlen("wait"))) {
        msleep(20);
        property_get(WAIT_FOR_SECNVM_PROP, sec_nvm_done, NULL);

        if (!strncmp(sec_nvm_done, "failed", strlen("failed")))
            return -ENODEV;
    }
    // set property back - to wait again after SSR
    property_set(WAIT_FOR_SECNVM_PROP, "wait");

    return 0;
}

/**
 *  re-load SP App after SP reset
 */
static int handle_sp_out_of_reset(void)
{
    int ret = 0;
    // The client will be initialized by the first loaded App
    struct spcom_client* service_client = NULL;

    ret = wait_sec_nvm();
    if (ret < 0) {
        LOGE("Failed to initialize NVM. ret [%d].\n", ret);
        return ret;
    }

    // Re-register SSR callback before loading Apps to handle SSR
    // events in case there is any issue with App loading
    if (g_ssr_spcom_client) {
        unregister_ssr_callback();
    }

    ret = register_ssr_callback();
    if (ret < 0) {
        LOGE("SSR callback registration failed. ret [%d].\n", ret);
        return ret;
    }

    ret = load_sp_apps(g_apps_to_load,
            sizeof(g_apps_to_load)/sizeof(g_apps_to_load[0]), &service_client);
    if (ret < 0) {
        LOGE("Loading SP Apps failed. ret [%d].\n", ret);
    }

    if (iar_is_recovery_needed()) {
        ret = iar_perform_recovery();
        if (ret != 0) {
            LOGE("perform_iar_recovery Failed. ret [%d].\n", ret);
        }
    }

    // If IAR activated and IAR PBL CE occurred -
    // save new SPU fw and UEFI apps cmac
    // read calc cmacs only after apploader loaded UEFI apps
    // if not loading any app - check apploader is ready
    if (iar_is_iar_state_activated()) {
        iar_check_spu_pbl_ce();
    }

    // All Apps loaded - Get SP Build version and Health parameters
    if (service_client) {
        int retval = 0;
        sp_health_status_data status_data;
        uint32_t build_version = (uint32_t)-1;
        uint32_t build_version_hash = (uint32_t)-1;
        uint32_t hw_version = (uint32_t)-1;

        if(!spcom_sp_sysparam_read_ext(SP_SYSPARAM_ID_BUILD_VERSION,
                0, 0, &build_version, service_client) &&
                !spcom_sp_sysparam_read_ext(SP_SYSPARAM_ID_BUILD_VERSION_HASH,
                        0, 0, &build_version_hash, service_client)) {

            retval = spcom_sp_sysparam_read_ext(SP_SYSPARAM_ID_HW_VERSION,
                    0, 0, &hw_version, service_client);
            if (retval < 0) {
                LOGE("Failed to read HW Version. [%d]\n", retval);
            }

            LOGI("SP Build v%d.%d.%08X, HW v%08X\n",
                    ((uint32_t)-1 == build_version) ? 0 : (build_version >> 16),
                    ((uint32_t)-1 == build_version) ? 0 : (build_version & 0xFFFF),
                    ((uint32_t)-1 == build_version_hash) ? 0 : build_version_hash,
                    hw_version);
        }
        else {
            LOGE("Failed to read SP Build Version.\n");
        }

        retval = spcom_check_sp_health_ext(SP_HEALTH_ARI_MODE,
                &status_data, service_client);
        if (!retval) {
            LOGI("Sensors Calibration: %s, ARI mode: %d, ARI fuse: %d, ARI Gauge: %d",
                    (status_data.sensors_calibrated)?"Ok":"None",
                    status_data.ari_mode,
                    status_data.ari_on_fuse,
                    status_data.ari_fuse_gauge);

            if ((sp_health_ari_on_fuse_t)status_data.ari_on_fuse == ARI_FUSE_ON ||
                    (sp_health_ari_on_fuse_t)status_data.ari_on_fuse == ARI_FUSE_RESERVED) {
                LOGI("POR indicator: %d, POR counter %d",
                        status_data.por_indicator,
                        status_data.por_counter);
            }
        }
        else {
            LOGE("Failed to read SP Health Info.[%d]\n", retval);
        }

        uint32_t fips_override = 0;

        retval = spcom_sp_sysparam_read_ext(SP_SYSPARAM_ID_FIPS_OVERRIDE,
                0, 0, &fips_override, service_client);
        if (!retval) {
            LOGI("FIPS: %s, FIPS override: %s",
                    (status_data.fips_enabled)?"on":"off",
                    (fips_override)?"on":"off");
        }
        else {
            LOGE("Failed to read FIPS Config.[%d]\n", retval);
        }

        if (spcom_disconnect_service_client(service_client)) {
            LOGD("Failed to disconnect service client.\n");
        }
    }

    property_set(WAIT_FOR_SPDAEMON_PROP, "done");
    return ret;
}

int main(void)
{
    int ret = -1;
    bool is_fw_update = false;
    char bootmode[PROPERTY_VALUE_MAX];

#ifndef SPCOM_CMD_RESTART_SP
    static const int PIL_RETRIES = 5;
#endif // !SPCOM_CMD_RESTART_SP

    g_firmware_type = get_firmware_type();
    LOGD("firmware_type [%c].\n", (int) g_firmware_type);

    g_soc_type = get_soc_type();

    // this causes to wait for sec_nvm daemon finishes nvm init
    property_set(WAIT_FOR_SECNVM_PROP, "wait");
    // this property is to inform user when spdaemon is done its initialization
    property_set(WAIT_FOR_SPDAEMON_PROP, "wait");

    if (iar_is_iar_state_activated()) {
        LOGI("IAR is activated\n");

        ret = iar_maintain_database_sanity();
        if (ret < 0) {
            LOGE("Error while trying to maintain IAR database sanity, ret [%d]. Continue anyway\n", ret);
        }

        ret = iar_load_spu_firmware_cmac();
        if (0 != ret) {
            LOGE("Loading SPU FW CMAC failed, ret [%d]. Continue anyway\n", ret);
        }

        ret = iar_check_if_fw_update();
        if(ret < 0) {
            LOGE("Fail to check if spu images was updated %d\n", ret);
        } else if (ret > 0) {
            LOGE("Firmware/UEFI apps was updated\n");
            is_fw_update = true;
        }

    } else {
        LOGI("IAR is not activated\n");
    }

#ifndef SPCOM_CMD_RESTART_SP
    //start SP PIL
    for (int i=0; i<PIL_RETRIES; i++) {
        ret = pil_load_sp();
        if (ret == 0) { //success
            break;
        }
        LOGD("pil_load_sp failed. ret [%d]. retrying..\n", ret);
        sleep(1);
    }
    if (ret != 0) {
        LOGE("pil_load_sp failed. ret [%d].\n", ret);
        goto pil_error;
    }
#else // !SPCOM_CMD_RESTART_SP

    //For SM8250, SP is started already in UEFI stage. In that case, pass true
    //as a paramater to the kernel so it skips calling PIL boot function and
    //instead calls a local power up function that only sets the INIT DONE
    //register to satisfy PIL.
    LOGD("------ going to start SP From kernel ------\n");
    ret = spcom_restart_sp_subsystem((g_soc_type == SOC_SM8250) ? true : false, is_fw_update);

    if (ret < 0) {
        LOGE("PIL from kernel failed. ret [%d].\n", ret);
        goto pil_error;
    }
#endif // !SPCOM_CMD_RESTART_SP

    // notify pil was called
    spcom_pil_was_called();

    ret = handle_sp_out_of_reset();
    if (ret < 0) {
        LOGE("Initial SP initialization failed. ret [%d].\n", ret);
        goto load_app_error;
    }

    /* Create a thread for handling MPSS<->SPSS<->APSS communication */
    sp_uim_remote_create_thread();



    while(1) {
        // go to sleep , nothing to do now , wake up upon SP reset event
        LOGD("Go to sleep , nothing to do now , wake up upon SP reset event.\n");
        pthread_cond_init(&g_pthread_cond, NULL);
        pthread_mutex_init(&g_pthread_mutex, NULL);

        suspend_me();
        ret = handle_sp_out_of_reset();
        if (ret < 0) {
            LOGE("SSR SP initialization failed. ret [%d].\n", ret);
        }
    }

    return 0;

pil_error:

#ifndef SPCOM_CMD_RESTART_SP
    LOGD("SPSS-PIL Load failure, power down the SPSS.\n");
    pm_client_disconnect(g_pm_spss_handle) ;
    pm_client_unregister(g_pm_spss_handle) ;
#endif // !SPCOM_CMD_RESTART_SP

load_app_error:
    // Avoid exit, since init process will start the daemon again
    while(1) {
        LOGD("going to sleep for a day.\n");
        sleep(60*60*24); // sleep 60 sec x 60 min x 24 hours = 1 day
    }

    return -ENODEV; // never happens
}
