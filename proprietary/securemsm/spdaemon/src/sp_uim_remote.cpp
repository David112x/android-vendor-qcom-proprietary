/*=============================================================================
Copyright (c) 2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

/**
 * @file sp_uim_remote.c
 * @brief - A thread created by SPDaemon, that registers as
 *          clients with QMI UIM REMOTE on one side, and with
 *          IUICC SP application on the other side. It
 *          implements the UIM REMOTE C++ functions that are
 *          called by QCRIL whenever Modem has a
 *          response/indication to be sent to this QMI UIM
 *          REMOTE client. Once the response/indication is
 *          received, the file contains function calls that
 *          interact with SP IUICC application and get the
 *          response back from the SP UICC application, to be
 *          fed back to the Modem via QMI UIM REMOTE channel.
 */

/*-------------------------------------------------------------------------
 * Include Files
 * ----------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <log/log.h>
#include <pthread.h>
#include <cutils/properties.h>
#include <time.h>
#include <signal.h>

extern "C" {
    #include <spcomlib.h>
    #include <sp_jc_msg.h>
}

#include <vendor/qti/hardware/radio/uim_remote_client/1.0/IUimRemoteServiceClient.h>
#include <vendor/qti/hardware/radio/uim_remote_client/1.0/IUimRemoteServiceClientResponse.h>
#include <vendor/qti/hardware/radio/uim_remote_client/1.2/IUimRemoteServiceClientIndication.h>
#include <vendor/qti/hardware/radio/uim_remote_client/1.2/types.h>

using namespace vendor::qti::hardware::radio::uim_remote_client;
using namespace vendor::qti::hardware::radio::uim_remote_client::V1_0;
using namespace vendor::qti::hardware::radio::uim_remote_client::V1_2;
using ::android::hardware::Status;
using ::android::hardware::Return;
using ::android::hardware::hidl_vec;
using ::android::hardware::Void;
using ::android::hardware::hidl_death_recipient;
using android::sp;

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * ----------------------------------------------------------------------*/
#ifdef PRINT_LOG_TO_STDOUT
    #define LOGD(fmt, x...) printf("sp_uim_remote: dbg: %s: " fmt "\n", __func__,##x)
    #define LOGI(fmt, x...) printf("sp_uim_remote: dbg: %s: " fmt "\n", __func__,##x)
    #define LOGE(fmt, x...) printf("sp_uim_remote: err: %s: " fmt "\n", __func__,##x)
#else // print to system log a.k.a logcat
    #undef LOG_TAG
    #undef LOGD
    #undef LOGI
    #undef LOGE
    #define LOG_TAG "sp_uim_remote"
    #define LOGD(fmt, x...) SLOGD("[%s] " fmt, __func__, ##x)
    #define LOGI(fmt, x...) SLOGI("[%s] " fmt, __func__, ##x)
    #define LOGE(fmt, x...) SLOGE("[%s] " fmt, __func__, ##x)
#endif

/* Default uicc application name on the SP that may be replaced as needed
   using the android property below */
#define IUICC_APP_NAME_DEFAULT                 "iuicc"
#define IUICC_APP_NAME_LEN                     5
#define IUICC_APP_NAME_PROP_KEY                "persist.vendor.sp_iuicc_name"
#define IUICC_APP1_NAME_DEFAULT                "iuicc0"
#define IUICC_APP1_NAME_LEN                    6
#define IUICC_APP2_NAME_DEFAULT                "iuicc1"
#define IUICC_APP2_NAME_LEN                    6
/* Max number of indications that can be queued by UIM REMOTE to
   SPDaemon queue */
#define UIM_REMOTE_IND_QUEUE_MAX_SIZE          20 /* Increased from 10 because there is another thread introduced?? */
/* Maximum retries for registration attempt with UIM REMOTE */
#define UIM_REMOTE_REGISTER_RETRIES            5
/* Maximum wait per retry attempt for registration attempt with iUICC on SP */
#define UIM_REMOTE_IUICC_REGISTER_TIMEOUT_MSEC    1000
/* Maximum retries for registration attempt with iUICC on SP */
#define UIM_REMOTE_IUICC_REGISTER_RETRIES      60
/* Maximum wait per retry attempt for sp link up */
#define UIM_REMOTE_SP_LINK_UP_TIMEOUT_MSEC     1000
/* Maximum retries for sp link up wait attemps */
#define UIM_REMOTE_SP_LINK_UP_RETRIES          60
/* Maximum wait per retry attempt for checking iUICC load status */
#define UIM_REMOTE_IUICC_LOAD_STATUS_TIMEOUT_MSEC 2000
/* Maximum retries for checking iUICC load status */
#define UIM_REMOTE_IUICC_LOAD_STATUS_RETRIES      30

#define UIM_REMOTE_APDU_HDR_SIZE               5
#define UIM_REMOTE_MAX_RAPDU_LENGTH            JC_SPCOM_MAX_RESP_APDU_LENGTH
#define UIM_REMOTE_IUICC_MAX_REQ_SIZE          (JC_SPCOM_MAX_REQ_APDU_PAYLOAD + UIM_REMOTE_APDU_HDR_SIZE)
#define UIM_REMOTE_IUICC_MAX_RSP_SIZE          JC_SPCOM_MAX_RESP_APDU_LENGTH
#define UIM_REMOTE_IUICC_TECHNICAL_ERR_SW1     0x6F
#define UIM_REMOTE_IUICC_TECHNICAL_ERR_SW2     0x00

/* TODO make this runtime property */
#define MAX_NUM_SLOTS                          2

/* Timer values */
#define UIM_SANITY_TIMER_VALUE                 30
#define UIM_SIM1_PIN_INPUT_TIMER_VALUE         10

typedef enum {
    SLOT_1  = 0,
    SLOT_2  = 1,
    MAX_SLOT = MAX_NUM_SLOTS
}slot_type;

/*-------------------------------------------------------------------------
 * Structures and enums
 *-----------------------------------------------------------------------*/
typedef enum {
    UIM_REMOTE_POWER_UP_IND = 0,
    UIM_REMOTE_POWER_DOWN_IND,
    UIM_REMOTE_APDU_IND,
    UIM_REMOTE_RESET_IND,
    UIM_REMOTE_CONNECT_IND,
    UIM_REMOTE_DISCONNECT_IND,
    UIM_REMOTE_SERVICE_STATUS_CONNECTED_IND,
    UIM_REMOTE_SERVICE_STATUS_DISCONNECTED_IND,
    UIM_REMOTE_FORCE_DISCONNECT_FROM_IUICC_IND,
    UIM_REMOTE_CARD_STATUS_IND
} uim_remote_ind_enum_type;

typedef enum {
    UIM_REMOTE_APP_STATE_UNKNOWN = 0,
    UIM_REMOTE_APP_STATE_DETECTED,
    UIM_REMOTE_APP_STATE_WAITING_ON_USER,
    UIM_REMOTE_APP_STATE_HALTED,
    UIM_REMOTE_APP_STATE_READY
} uim_remote_app_state_enum_type;

typedef struct uim_remote_card_status_type {
    uint8_t active_slots;
    uint8_t num_apps;
    hidl_vec<UimRemoteClientAppInfo> vec;
} uim_remote_card_status_type;

typedef struct {
    uint16_t data_len;
    uint8_t  data_buf[UIM_REMOTE_IUICC_MAX_REQ_SIZE];
} uim_remote_apdu_ind_data_type;

typedef union {
    uim_remote_apdu_ind_data_type apdu_ind_data;
} uim_remote_ind_data_type;

typedef struct {
    uim_remote_ind_enum_type            ind_type;
    uim_remote_ind_data_type            ind_data;
    uim_remote_card_status_type         card_status_data;
    uint8_t                             slot;
} uim_remote_ind_type;

typedef struct uim_remote_ind_node_type {
    uim_remote_ind_type                 ind;
    struct uim_remote_ind_node_type    *next_ptr;
} uim_remote_ind_node_type;

typedef struct uim_channel_info_type {
    const char* channel_name;
    uint8_t     len;
} uim_channel_info_type;

struct UimRemoteClientResponseImpl : public IUimRemoteServiceClientResponse, public hidl_death_recipient {
    void serviceDied( uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &);

    Return<void> UimRemoteServiceClientEventResp(int32_t token, UimRemoteClientEventRsp eventResp);

    Return<void> UimRemoteServiceClientApduResp(int32_t token, UimRemoteClientApduRsp apduResp);

    uint8_t      slot;
};

struct UimRemoteClientIndicationImpl : V1_2::IUimRemoteServiceClientIndication {
    Return<void> UimRemoteServiceClientApduInd(const hidl_vec<uint8_t>& apduInd);

    Return<void> UimRemoteServiceClientConnectInd();

    Return<void> UimRemoteServiceClientDisconnectInd();

    Return<void> UimRemoteServiceClientPowerUpInd(bool hasTimeOut, int32_t timeOut, bool hasVoltageClass, UimRemoteClientVoltageClass powerUpVoltageClass);

    Return<void> UimRemoteServiceClientPowerDownInd(bool hasPowerDownMode, UimRemoteClientPowerDownMode powerDownMode);

    Return<void> UimRemoteServiceClientResetInd();

    Return<void> UimRemoteServiceClientServiceInd(bool status);

    Return<void> UimRemoteServiceClientCardInitStatusInd(const UimRemoteClientCardInitStatusType& cardInitStatusInd);

    uint8_t      slot; /* Holds the slot ID for the which the IND is received */
};

/*-------------------------------------------------------------------------
 * Global Variables
 * ----------------------------------------------------------------------*/
/* Globals related to the queue used by QCRIL to post messages/indications
   from modem */
static pthread_mutex_t                   g_pthread_api_mutex[MAX_NUM_SLOTS] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};
static uim_remote_ind_node_type         *g_head_ptr[MAX_NUM_SLOTS]          = {NULL, NULL};
static uint8_t                           g_queue_cnt[MAX_NUM_SLOTS]         = {0, 0};

/* Global related to the suspend-resume mechanism */
static pthread_mutex_t                   g_pthread_mutex[MAX_NUM_SLOTS]    = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};
static pthread_cond_t                    g_pthread_cond[MAX_NUM_SLOTS]     = {PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER};

/* Globals needed for UIM REMOTE client registration */
static sp<UimRemoteClientResponseImpl>   g_uim_rmt_rsp_cb[MAX_NUM_SLOTS]   = {NULL, NULL};
static sp<UimRemoteClientIndicationImpl> g_uim_rmt_ind_cb[MAX_NUM_SLOTS]   = {NULL, NULL};
static sp<IUimRemoteServiceClient>       g_uim_rmt_client[MAX_NUM_SLOTS]   = {NULL, NULL};

/* Globals needed for SPCOM client registration */
static struct spcom_client              *g_spcom_client_ptr[MAX_NUM_SLOTS] = {NULL, NULL};
static bool                              g_sp_reset_detected[MAX_NUM_SLOTS] = {false, false};

static int                               g_thread_resumed[MAX_NUM_SLOTS]   = {0, 0};

/* Timer related */
timer_t                                  g_sanity_timer;
timer_t                                  g_user_input_timer;
/* Start iuicc and iuicc1 thread from the same ctx but wait for iuicc1 until the timer expires */
uint8_t                                  g_number_active_slots          = 0;

/* Multiple mutexes */
static pthread_mutex_t                   g_user_input_timer_mutex       = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t                   g_active_slots_mutex           = PTHREAD_MUTEX_INITIALIZER;

/*-------------------------------------------------------------------------
 * Function "C" externs so they can be called from spdaemon.c
 * ----------------------------------------------------------------------*/
extern "C" {
    void sp_uim_remote_create_thread(void);
}

/*-------------------------------------------------------------------------
 * Function Forward Declarations
 * ----------------------------------------------------------------------*/

static void sp_uim_remote_notify_ssr_cb(void);
static void sp_uim_remote_notify_ssr_cb_2(void);

void sp_handle_sim1_modem_reg_timeout_evt(union sigval data);
void sp_handle_waiting_on_user_timeout_evt(union sigval data);
uint8_t sp_uim_get_num_active_slots();

/*-------------------------------------------------------------------------
 * Function Implementations
 * ----------------------------------------------------------------------*/

/*=========================================================================
  FUNCTIONS implementing general self-explained utilities
===========================================================================*/
static void suspend_me(slot_type slot)
{
    pthread_mutex_lock(&g_pthread_mutex[slot]);

    while (g_thread_resumed[slot] == 0) {
        pthread_cond_wait(&g_pthread_cond[slot], &g_pthread_mutex[slot]);
    }

    g_thread_resumed[slot] = 0;
    pthread_mutex_unlock(&g_pthread_mutex[slot]);
}

static void resume_me(slot_type slot)
{
    pthread_mutex_lock(&g_pthread_mutex[slot]);

    g_thread_resumed[slot] = 1;

    pthread_cond_signal(&g_pthread_cond[slot]);
    pthread_mutex_unlock(&g_pthread_mutex[slot]);
}

static void msleep(int msec)
{
    usleep(msec * 1000);
}

/**
 * memscpy
 * Utility memscpy function
 *
 * @param dst      destination pointer
 *        dst_size destination size
 *        src      source pointer
 *        src_size source size
 *
 */
static inline size_t memscpy(void *dst, size_t dst_size, const void *src, size_t src_size)
{
    size_t min_size = (dst_size < src_size) ? dst_size : src_size;
    memcpy(dst, src, min_size);
    return min_size;
}

/*=========================================================================
  FUNCTIONS implementing the Queue used by QCRIL for posting messages from
  Modem
===========================================================================*/
/**
 * sp_uim_remote_ind_queue_add() - Adds item to the queue.
 *
 * Performs deep copy of the indication received from UIM REMOTE and then
 * adds it to the global queue
 *
 * @param ind_ptr - indication received from UIM REMOTE
 *
 * return: 0 on success, negative error code on failure
 */
static int sp_uim_remote_ind_queue_add(uim_remote_ind_type *ind_ptr, slot_type slot)
{
    uim_remote_ind_node_type *current_ptr = NULL;
    int                       retval      = 0;

    if (ind_ptr == NULL || slot >= MAX_SLOT) {
        LOGE("invalid input ind_ptr %lu slot [%d]", reinterpret_cast<uintptr_t>(ind_ptr), slot);
        return -EINVAL;
    }

    pthread_mutex_lock(&g_pthread_api_mutex[slot]);

    do {
        if (g_queue_cnt[slot] >= UIM_REMOTE_IND_QUEUE_MAX_SIZE) {
            LOGE("queue full slot [%d]", slot);
            retval = -EPERM;
            break;
        }

        current_ptr = (uim_remote_ind_node_type *)malloc(sizeof(uim_remote_ind_node_type));
        if (current_ptr == NULL) {
            LOGE("malloc failure slot [%d]", slot);
            retval = -ENOMEM;
            break;
        }

        memset(current_ptr, 0, sizeof(uim_remote_ind_node_type));
        /* Copy over the indication from QCRIL thread context to our queue */
        memscpy(&(current_ptr->ind), sizeof(uim_remote_ind_type), ind_ptr, sizeof(uim_remote_ind_type));

        /* Add the incoming indication as the last node of the queue */
        if (g_head_ptr[slot] == NULL) {
            g_head_ptr[slot] = current_ptr;
        } else {
            uim_remote_ind_node_type *temp_ptr = g_head_ptr[slot];
            while (temp_ptr != NULL && temp_ptr->next_ptr != NULL) {
                temp_ptr = temp_ptr->next_ptr;
            }
            temp_ptr->next_ptr = current_ptr;
        }

        g_queue_cnt[slot]++;
    } while (0);

    pthread_mutex_unlock(&g_pthread_api_mutex[slot]);

    return retval;
} /* sp_uim_remote_ind_queue_add */

/**
 * sp_uim_remote_ind_queue_remove() - Gets item from the
 * queue.
 *
 * Retrieves the head node (indication) from the global queue
 * and frees the memory allocated when that node was first
 * created
 *
 * @ind_out_ptr - indication out pointer
 *
 * return: 0 on success, negative error code on failure (see errno.h)
 */
static int sp_uim_remote_ind_queue_remove(uim_remote_ind_type *ind_ptr, slot_type slot)
{
    uim_remote_ind_node_type *current_ptr = NULL;
    int                       retval      = 0;

    if (ind_ptr == NULL || slot >= MAX_SLOT) {
        LOGE("invalid input ind_ptr %lu slot [%d]", reinterpret_cast<uintptr_t>(ind_ptr), slot);
        return -EINVAL;
    }

    pthread_mutex_lock(&g_pthread_api_mutex[slot]);

    do {
        current_ptr = g_head_ptr[slot];
        if (current_ptr == NULL || g_queue_cnt[slot] == 0) {
            /* The queue is empty. Nothing valid to return */
            LOGD("queue empty slot [%d]", slot);
            retval = -EPERM;
            break;
        }

        /* Copy over the indication data from the head node to the out param and
           then free the memory that was allocated at the creation of this node */
        memscpy(ind_ptr, sizeof(uim_remote_ind_type), &current_ptr->ind, sizeof(uim_remote_ind_type));

        g_head_ptr[slot] = g_head_ptr[slot]->next_ptr;
        free(current_ptr);
        g_queue_cnt[slot]--;
    } while (0);

    pthread_mutex_unlock(&g_pthread_api_mutex[slot]);

    return retval;
} /* sp_uim_remote_ind_queue_remove */

/**
 * sp_uim_remote_ind_queue_cnt() - Gets number of items in
 * the queue.
 *
 * Retrieves the current number of nodes in the global queue
 *
 * @cnt_out_ptr - count out pointer
 *
 * return: 0 on success, negative error code on failure (see errno.h)
 */
static int sp_uim_remote_ind_queue_cnt(uint8_t *cnt_ptr, slot_type slot)
{
    if (cnt_ptr == NULL || slot >= MAX_SLOT) {
        LOGE("invalid input cnt_ptr %lu slot [%d]", reinterpret_cast<uintptr_t>(cnt_ptr), slot);
        return -EINVAL;
    }

    pthread_mutex_lock(&g_pthread_api_mutex[slot]);
    *cnt_ptr = g_queue_cnt[slot];
    pthread_mutex_unlock(&g_pthread_api_mutex[slot]);
    return 0;
} /* sp_uim_remote_ind_queue_cnt */

/**
 * sp_uim_remote_ind_queue_flush() - Flushes the global
 * queue.
 *
 * Flushes the global queue
 *
 * return: 0 on success, negative error code on failure (see errno.h)
 */
int sp_uim_remote_ind_queue_flush(slot_type slot)
{
    uim_remote_ind_node_type *current_ptr = NULL;

    if (slot >= MAX_SLOT) {
        LOGE("invalid input slot [%d]", slot);
        return -EINVAL;
    }
    pthread_mutex_lock(&g_pthread_api_mutex[slot]);

    do {
        current_ptr = g_head_ptr[slot];
        if (current_ptr == NULL) {
            LOGD("sp_uim_remote_ind_queue_flush: queue already empty");
            /* Reset the counter incase for some erroneous reasons it is not
               set to 0 already */
            g_queue_cnt[slot] = 0;
            break;
        }
        while (current_ptr) {
            g_head_ptr[slot] = g_head_ptr[slot]->next_ptr;
            free(current_ptr);
            current_ptr = g_head_ptr[slot];
        }

        g_queue_cnt[slot] = 0;
    } while (0);

    pthread_mutex_unlock(&g_pthread_api_mutex[slot]);

    return 0;
} /* sp_uim_remote_ind_queue_flush */

/**
 * sp_uim_remote_ind_queue_init() - Initialize the queue.
 *
 * Initialize the queue related initial state
 *
 * return: 0 on success, negative error code on failure (see errno.h)
 */
int sp_uim_remote_ind_queue_init(slot_type slot)
{
    if (slot >= MAX_SLOT || g_queue_cnt[slot] > 0) {
        LOGE("Invalid slot[%d] OR queue is non empty prior to its initialization!", slot);
        return -EPERM;
    }

    pthread_mutex_init(&g_pthread_api_mutex[slot], NULL);

    return 0;
} /* sp_uim_remote_ind_queue_init */

/*=========================================================================
  FUNCTIONS implementing client registrations/deregistrations
===========================================================================*/
/**
 * sp_uim_remote_register_with_iuicc()
 *
 * Registers as client for iuicc SP application via SPCOM. Once
 * a valid client pointer is received, it sends an even to UIM
 * REMOTE to indicate to the Modem that a remote SIM/iuicc is
 * available on the SP.
 *
 * @param: None
 *
 * @return: None
 */
void sp_uim_remote_register_with_iuicc(uint8_t  slot)
{
    struct                spcom_client_info client_info;
    bool                  is_connected                = false;
    bool                  is_link_up                  = false;
    bool                  is_loaded                   = false;
    uint8_t               retry_counter               = 0;
    UimRemoteEventReqType eventReq                    = {};
    int                   ret                         = 0;
    char                  ch_name[PROPERTY_VALUE_MAX] = {};

    if (slot >= MAX_SLOT) {
        return;
    }

    memset(&client_info, 0, sizeof(client_info));

    LOGD("sp_uim_remote_register_with_iuicc.\n");

    if (slot == SLOT_2) {
        client_info.ch_name = IUICC_APP2_NAME_DEFAULT;
        LOGD("sp_uim_remote_register_with_iuicc. channel name %s \n", client_info.ch_name);
        LOGD("Service IND handled, slot1 thread can sleep when needed");
        client_info.notify_ssr_cb = sp_uim_remote_notify_ssr_cb_2; //spcom ssr cb doesn't take client params as of now. so need separate CB
    }
    else if (slot == SLOT_1){
        property_get(IUICC_APP_NAME_PROP_KEY, ch_name, IUICC_APP_NAME_DEFAULT);
        if ((!strncmp(ch_name, "iuicc", IUICC_APP_NAME_LEN)) ||
            (!strncmp(ch_name, "iuicc0", IUICC_APP1_NAME_LEN))) {
            client_info.ch_name = ch_name;
        }
        else {
            client_info.ch_name = IUICC_APP_NAME_DEFAULT;
        }
        LOGD("sp_uim_remote_register_with_iuicc. channel name %s \n", client_info.ch_name);
        client_info.notify_ssr_cb = sp_uim_remote_notify_ssr_cb; //spcom ssr cb doesn't take client params as of now. so need separate CB
    }

    do {
        retry_counter++;
        is_link_up = spcom_is_sp_subsystem_link_up();
        if(is_link_up)
        {
            break;
        }

        msleep(UIM_REMOTE_SP_LINK_UP_TIMEOUT_MSEC);
    } while (retry_counter < UIM_REMOTE_SP_LINK_UP_RETRIES);

    if (is_link_up == false) {
        LOGE("Timeout wait for SP link UP");
        return;
    }

    retry_counter = 0;

    /* Wait for the iUICC SP app to get loaded by someone */
    do {
        retry_counter++;
        is_loaded = spcom_is_app_loaded(client_info.ch_name);

        if (is_loaded == true) {
            retry_counter = 0;
            break;
        }

        msleep(UIM_REMOTE_IUICC_LOAD_STATUS_TIMEOUT_MSEC);
    } while((is_loaded == false) && (retry_counter < UIM_REMOTE_IUICC_LOAD_STATUS_RETRIES));

    if(is_loaded == false) {
        LOGE("sp iuicc app not loaded");
        return;
    }

    retry_counter = 0;

    /* iUICC SP app is found to be loaded. Let's register with it */
    /* This is a shareable channel. Call create shareable channel first before register */
    ret = spcom_create_shareable_channel(client_info.ch_name);
    LOGD("spcom create shareable channel returns [%d].\n", ret);

    g_spcom_client_ptr[slot] = spcom_register_client(&client_info);
    if (g_spcom_client_ptr[slot]) {
        LOGD("Register client with iuicc for slot [%d] succeeded.\n", slot);
    } else {
        LOGE("Register client with iuicc for slot [%d] failed\n", slot);
        return;
    }

    do {
        /* Wait for the spcom connection to be available */
        retry_counter++;
        is_connected = spcom_client_is_server_connected(g_spcom_client_ptr[slot]);

        if (is_connected == true) {
            retry_counter = 0;
            break;
        }

        msleep(UIM_REMOTE_IUICC_REGISTER_TIMEOUT_MSEC);
    } while((is_connected == false) && (retry_counter < UIM_REMOTE_IUICC_REGISTER_RETRIES));

    if(is_connected == false) {
        LOGE("sp remote service not connected");
        return;
    }

    /* Indicate to Modem that a remote SIM/iuicc is available on the SP */
    eventReq.event = UimRemoteClientEventStatusType::UIM_REMOTE_STATUS_CONN_AVAILABLE;
    g_uim_rmt_client[slot]->UimRemoteServiceClientEventReq (0, eventReq);
} /* sp_uim_remote_register_with_iuicc */

/**
 * sp_uim_remote_iuicc_cleanup_and_dereg()
 *
 * Sends a power down command to the iuicc and deregisters
 * itself as client of iuicc via SPCOM
 *
 * @param: None
 *
 * @return: None
 */
void sp_uim_remote_iuicc_cleanup_and_dereg(slot_type slot)
{
    if (g_spcom_client_ptr[slot] == NULL) {
        LOGE("spcom client ptr is already invalid for slot [%d]", slot);
        return;
    }

    (void)sp_jc_send_pdown_cmd_and_wait_on_resp(g_spcom_client_ptr[slot]);

    if(spcom_unregister_client(g_spcom_client_ptr[slot])) {
        LOGE("unregister client with iuicc failed for slot [%d]", slot);
    }
    g_spcom_client_ptr[slot]        = NULL;
} /* sp_uim_remote_iuicc_cleanup_and_dereg */

/* Function to handle the timeout related to slot0
   modem registration. In case slot0 does not reach SUB READY within
   the timeout value, this function executes to handle the slot1 modem registration.
   This function conforms to the POSIX SA_SIGINFO action handler. */
void sp_handle_sim1_modem_reg_timeout_evt(union sigval data)
{
    (void)data;
    LOGD("Timer expired for SIM1 init");
    /* Start SIM2 init here */
    if (g_sanity_timer != NULL) {
        timer_delete(g_sanity_timer);
        g_sanity_timer = NULL;
    }
    resume_me(SLOT_2);
}

/* Function to handle timeout related to PIN input from user. */
void sp_handle_waiting_on_user_timeout_evt(union sigval data)
{
    (void)data;
    LOGD("In sp_handle_waiting_on_user_timeout_evt");
    /* In case the user input timer expires while waiting for the user,
       start SIM2 init. */
    pthread_mutex_lock(&g_user_input_timer_mutex);
    timer_delete(g_user_input_timer);
    g_user_input_timer = NULL;
    pthread_mutex_unlock(&g_user_input_timer_mutex);

    /* If the sanity timer hasn't expired yet, stop it. If it has already
       expired for some reason, then SIM2 initialization has started already. */
    if (g_sanity_timer != NULL) {
        timer_delete(g_sanity_timer);
        g_sanity_timer = NULL;
    }
    resume_me(SLOT_2);
}

/* Function to get the number of active slots */
uint8_t sp_uim_get_num_active_slots()
{
    uint8_t active_slots = 0;

    pthread_mutex_lock(&g_active_slots_mutex);
    active_slots = g_number_active_slots;
    pthread_mutex_unlock(&g_active_slots_mutex);

    return active_slots;
}
/**
 * sp_uim_remote_register_with_modem()
 *
 * Registers as client with UIM REMOTE, for intaraction with
 * modem. In case of registration failure, it retries the
 * registration attempt a few times before exiting. If this
 * registration is successful, the thread created by SPDaemon
 * shall receive an indication from UIM RMT for ServiceConnect
 * and SPDaemon will then register with iUICC as part of
 * processing that indication
 *
 * @param: None
 *
 * @return: true or false
 */
bool sp_uim_remote_register_with_modem(slot_type slot)
{
    if (slot >= MAX_SLOT) {
        LOGE("Invalid Param Slot %d \n", slot);
        return false;
    }

    struct sigevent     notify_timer_exp_evt;
    struct itimerspec   timeout_val;    uint8_t retry_counter              = 0;
    g_uim_rmt_rsp_cb[slot]             = new UimRemoteClientResponseImpl{};
    g_uim_rmt_ind_cb[slot]             = new UimRemoteClientIndicationImpl{};

    if (g_uim_rmt_rsp_cb[slot] == NULL || g_uim_rmt_ind_cb[slot] == NULL) {
        LOGE("Invalid uim remote rsp and/or ind callbacks, slot[%d]", slot);
        return false;
    }

    g_uim_rmt_ind_cb[slot]->slot       = slot;
    g_uim_rmt_rsp_cb[slot]->slot       = slot;

    do {
        /* UIM remote HIDL service connect */
        retry_counter++;
        g_uim_rmt_client[slot] = IUimRemoteServiceClient::getService("uimRemoteClient" + std::to_string(slot));
        if(g_uim_rmt_client[slot]) {
            LOGD("UIM Remote Service Client [%d] Registered successfully\n", slot);
            break;
        }
    } while(retry_counter < UIM_REMOTE_REGISTER_RETRIES);

    if (g_uim_rmt_client[slot] == NULL) {
        LOGE("Registration failed with UIM REMOTE\n");
        return false;
    }

    /* Register the service death handler */
    g_uim_rmt_client[slot]->linkToDeath(g_uim_rmt_rsp_cb[slot], 0);

    /* Register the UIM remote client response and indication functions */
    g_uim_rmt_client[slot]->setCallback(g_uim_rmt_rsp_cb[slot], g_uim_rmt_ind_cb[slot]);

    if (slot == SLOT_1) {
        /* Sanity Timer to keep track of overall SIM1 initialization.
           1. In case of SIM1 app not reaching SUB READY, this timer expires
              and starts SIM2 init.
           2. In case of SIM1 app waiting for user to enter PIN,
              the timer that is tracking the user input decides when
              to start SIM2 init.
           3. For any other case where SIM1 init is stuck/error,
              this timer would time out and start SIM2 init. */
        /* Using SIGEV_THREAD so the callback is handled in a new thread, which is anyway needed
           to start SIM2 init. */
        notify_timer_exp_evt.sigev_notify            = SIGEV_THREAD;
        notify_timer_exp_evt.sigev_notify_function   = sp_handle_sim1_modem_reg_timeout_evt;
        notify_timer_exp_evt.sigev_value.sival_ptr   = NULL;
        notify_timer_exp_evt.sigev_notify_attributes = NULL;

        /* Set the timer values */
        timeout_val.it_interval.tv_sec  = 0;
        timeout_val.it_interval.tv_nsec = 0;
        timeout_val.it_value.tv_sec     = UIM_SANITY_TIMER_VALUE;
        timeout_val.it_value.tv_nsec    = 0;

        if(timer_create(CLOCK_REALTIME, &notify_timer_exp_evt, &g_sanity_timer) != 0) {
            LOGE("Error creating sanity timer");
            /* Even if timer creation fails we still want this thread
               to continue.
               If timer fails,
               1. We wait until SIM1 reaches SUB READY and start SIM2 init.
               2. If SIM1 does not reach SUB READY, SIM2 init would not be triggered, since both the
                  ways we rely on for it have failed. */
            return true;
        }
        if(timer_settime(g_sanity_timer, 0, &timeout_val, NULL) != 0) {
            LOGE("Error starting sanity timer");
            timer_delete(g_sanity_timer);
            g_sanity_timer = NULL;
            return true;
        }
    }
    return true;
} /* sp_uim_remote_register_with_modem */

/*=========================================================================
  FUNCTIONS processing indications from the queue
===========================================================================*/
/**
 * sp_uim_remote_process_apdu_ind()
 *
 * Processes the APDU indication queued by UIM REMOTE
 *
 * @param: uim_remote_ind_type*
 *
 * @return: None
 */
static void sp_uim_remote_process_apdu_ind
(
  uim_remote_ind_type *apdu_ind_ptr
)
{
    uint8_t                   rsp_buf_ptr[UIM_REMOTE_IUICC_MAX_RSP_SIZE]; /* response from the card */
    uint16_t                  rsp_buf_len     = 0;
    uint8_t                   rsp_buf_with_sw_ptr[UIM_REMOTE_IUICC_MAX_RSP_SIZE + 2];
    uint8_t                   SW1             = UIM_REMOTE_IUICC_TECHNICAL_ERR_SW1;
    uint8_t                   SW2             = UIM_REMOTE_IUICC_TECHNICAL_ERR_SW2;
    jc_spcom_result_type      jc_spcom_result = JC_SPCOM_INTERNAL_ERR;
    UimRemoteClientApduStatus apduStatus      = UimRemoteClientApduStatus::UIM_REMOTE_FAILURE;
    std::vector<uint8_t>      apduResp;
    uint8_t                   slot            = MAX_SLOT;

    LOGD("Processing APDU indication from UIM REMOTE");

    memset(rsp_buf_ptr, 0, sizeof(rsp_buf_ptr));
    memset(rsp_buf_with_sw_ptr, 0, sizeof(rsp_buf_with_sw_ptr));

    if (apdu_ind_ptr &&
        (sizeof(apdu_ind_ptr->ind_data.apdu_ind_data.data_buf) <= UIM_REMOTE_IUICC_MAX_REQ_SIZE) &&
        apdu_ind_ptr->ind_data.apdu_ind_data.data_len <= UIM_REMOTE_IUICC_MAX_REQ_SIZE) {

        slot = (slot_type)apdu_ind_ptr->slot;
        if (slot >= MAX_SLOT || g_uim_rmt_client[slot] == NULL) {
            LOGE("Either invalid slot value [%d] or null client", slot);
            return;
        }

        LOGD("Sending APDU cmd to slot [%d]", slot);
        jc_spcom_result = sp_jc_send_apdu_cmd_and_wait_on_resp(g_spcom_client_ptr[slot],
                                                               (char*)apdu_ind_ptr->ind_data.apdu_ind_data.data_buf,
                                                               UIM_REMOTE_IUICC_MAX_REQ_SIZE,
                                                               apdu_ind_ptr->ind_data.apdu_ind_data.data_len,
                                                               (char*)rsp_buf_ptr,
                                                               UIM_REMOTE_IUICC_MAX_RSP_SIZE,
                                                               &rsp_buf_len,
                                                               (char*)&SW1,
                                                               (char*)&SW2);
    }

    if (jc_spcom_result != JC_SPCOM_SUCCESS) {
        LOGE("iUICC Apdu failure, Err - 0x%x", jc_spcom_result);
        apduStatus = UimRemoteClientApduStatus::UIM_REMOTE_FAILURE;
    } else {
        apduStatus = UimRemoteClientApduStatus::UIM_REMOTE_SUCCESS;
        if (rsp_buf_len > 0) {
            memscpy(rsp_buf_with_sw_ptr, sizeof(rsp_buf_with_sw_ptr), rsp_buf_ptr, rsp_buf_len);
        }
        rsp_buf_with_sw_ptr[rsp_buf_len] = SW1;
        rsp_buf_with_sw_ptr[rsp_buf_len + 1] = SW2;
        apduResp.resize(rsp_buf_len + 2);
        /* Copy rsp and SWs */
        memscpy(apduResp.data(), rsp_buf_len + 2, rsp_buf_with_sw_ptr, rsp_buf_len + 2);
    }
    g_uim_rmt_client[slot]->UimRemoteServiceClientApduReq (1, apduStatus, apduResp);
} /* sp_uim_remote_process_apdu_ind */

/**
 * sp_uim_remote_process_service_status_connected_ind()
 *
 * Processes the indication queued by UIM REMOTE to indicate the
 * status of UIM REMOTE service as connected (e.g. after Modem
 * SSR)
 *
 * @param: None
 *
 * @return: None
 */
static void sp_uim_remote_process_service_status_connected_ind
(
  uim_remote_ind_type *apdu_ind_ptr
)
{
    if (apdu_ind_ptr == NULL || apdu_ind_ptr->slot >= MAX_SLOT) {
        LOGE("Invalid Input apdu_ind_ptr %lu", reinterpret_cast<uintptr_t>(apdu_ind_ptr));
        return;
    }
    slot_type  slot  = (slot_type)apdu_ind_ptr->slot;
    /* UIM REMOTE service (modem) is back again after possible SSR. Reregister
       with iUICC SP application and send CONNECTION AVAILABLE to modem so that
       modem can re-initialize the iUICC, including powering it up */
    sp_uim_remote_register_with_iuicc(slot);
} /* sp_uim_remote_process_service_status_connected_ind */

/**
 * sp_uim_remote_process_service_status_disconnected_ind()
 *
 * Processes the indication queued by UIM REMOTE to indicate the
 * status of UIM REMOTE service as disconnected (e.g. due to
 * Modem SSR)
 *
 * @param: None
 *
 * @return: None
 */
static void sp_uim_remote_process_service_status_disconnected_ind
(
  uim_remote_ind_type *apdu_ind_ptr
)
{
    if (apdu_ind_ptr == NULL || apdu_ind_ptr->slot >= MAX_SLOT) {
        LOGE("Invalid Input apdu_ind_ptr %lu", reinterpret_cast<uintptr_t>(apdu_ind_ptr));
        return;
    }
    /* UIM RMT Service (modem) disconnected (due to Modem SSR) hence power down
       the iUICC. It is possible that the modem might not come back again. So,
       lets also dereg as iUICC client */
    sp_uim_remote_iuicc_cleanup_and_dereg((slot_type)apdu_ind_ptr->slot);
    sp_uim_remote_ind_queue_flush((slot_type)apdu_ind_ptr->slot);
} /* sp_uim_remote_process_service_status_disconnected_ind */

/**
 * sp_uim_remote_process_power_up_ind()
 *
 * Processes the indication queued by UIM REMOTE to send POWER
 * UP command to the iUICC
 *
 * @param: None
 *
 * @return: None
 */
static void sp_uim_remote_process_power_up_ind
(
  uim_remote_ind_type *apdu_ind_ptr
)
{
    jc_spcom_result_type  jc_spcom_result = JC_SPCOM_INTERNAL_ERR;
    uint8_t               resp_buf_ptr[JC_SPCOM_MAX_ATR_BYTES];
    uint16_t              resp_size       = 0;
    UimRemoteEventReqType event           = {};
    std::vector<uint8_t>  atr_ptr;

    if (apdu_ind_ptr == NULL || apdu_ind_ptr->slot >= MAX_SLOT) {
        LOGE("Invalid Input apdu_ind_ptr %lu", reinterpret_cast<uintptr_t>(apdu_ind_ptr));
        return;
    }
    slot_type  slot  = (slot_type)apdu_ind_ptr->slot;

    if(g_uim_rmt_client[slot] == NULL) {
        LOGE("Error processing power up ind. NULL client for slot [%d]", slot);
        return;
    }

    if(g_spcom_client_ptr[slot]) {
        jc_spcom_result = sp_jc_send_pup_cmd_and_wait_on_resp(g_spcom_client_ptr[slot]);
    }

    if (jc_spcom_result != JC_SPCOM_SUCCESS) {
        LOGE("iUICC power up failure, Err - %d, slot [%d]", jc_spcom_result, slot);
        event.event = UimRemoteClientEventStatusType::UIM_REMOTE_STATUS_CARD_ERROR;
    } else {
        LOGD("iUICC power up OK for slot [%d]", slot);
        jc_spcom_result = sp_jc_send_rst_cmd_and_wait_on_resp(g_spcom_client_ptr[slot],
                                                             (char*)resp_buf_ptr,
                                                              JC_SPCOM_MAX_ATR_BYTES,
                                                              &resp_size);
        if (jc_spcom_result != JC_SPCOM_SUCCESS) {
            event.event = UimRemoteClientEventStatusType::UIM_REMOTE_STATUS_CARD_ERROR;
        } else {
            atr_ptr.resize(resp_size);
            memscpy(atr_ptr.data(), resp_size, resp_buf_ptr, resp_size);
            event.atr = atr_ptr; /* copy atr vector array */
            event.event = UimRemoteClientEventStatusType::UIM_REMOTE_STATUS_CARD_RESET;
        }
    }
    g_uim_rmt_client[slot]->UimRemoteServiceClientEventReq (1, event);
} /* sp_uim_remote_process_power_up_ind */

/**
 * sp_uim_remote_process_power_down_ind()
 *
 * Processes the indication queued by UIM REMOTE to send POWER
 * DOWN command to the iUICC
 *
 * @param: None
 *
 * @return: None
 */
static void sp_uim_remote_process_power_down_ind
(
  uim_remote_ind_type *apdu_ind_ptr
)
{
    UimRemoteEventReqType event           = {};

    if (apdu_ind_ptr == NULL || apdu_ind_ptr->slot >= MAX_SLOT) {
        LOGE("Invalid Input apdu_ind_ptr %lu", reinterpret_cast<uintptr_t>(apdu_ind_ptr));
        return;
    }
    slot_type  slot  = (slot_type)apdu_ind_ptr->slot;

    if(g_uim_rmt_client[slot] == NULL) {
        LOGE("Error processing power down ind. NULL client for slot [%d]", slot);
        return;
    }
    /* Send Power Down command */
    if(g_spcom_client_ptr[slot]) {
        (void)sp_jc_send_pdown_cmd_and_wait_on_resp(g_spcom_client_ptr[slot]);
    }

    event.event = UimRemoteClientEventStatusType::UIM_REMOTE_STATUS_CARD_ERROR;
    LOGD("Calling UimRemoteServiceClientEventReq for power down IND for slot [%d]", slot);
    g_uim_rmt_client[slot]->UimRemoteServiceClientEventReq (1, event);
} /* sp_uim_remote_process_power_down_ind */

/**
 * sp_uim_remote_process_reset_ind()
 *
 * Processes the indication queued by UIM REMOTE to send RESET
 * command to the iUICC
 *
 * @param: None
 *
 * @return: None
 */
static void sp_uim_remote_process_reset_ind
(
  uim_remote_ind_type *apdu_ind_ptr
)
{
    jc_spcom_result_type  jc_spcom_result = JC_SPCOM_INTERNAL_ERR;
    uint8_t               resp_buf_ptr[JC_SPCOM_MAX_ATR_BYTES];
    uint16_t              resp_size       = 0;
    UimRemoteEventReqType event           = {};
    std::vector<uint8_t>  atr_ptr;

    if (apdu_ind_ptr == NULL || apdu_ind_ptr->slot >= MAX_SLOT) {
        LOGE("Invalid Input apdu_ind_ptr %lu", reinterpret_cast<uintptr_t>(apdu_ind_ptr));
        return;
    }
    slot_type  slot  = (slot_type)apdu_ind_ptr->slot;

    if(g_uim_rmt_client[slot] == NULL) {
        LOGE("Error processing reset ind. NULL client for slot [%d]", slot);
        return;
    }

    if(g_spcom_client_ptr[slot]) {
        jc_spcom_result = sp_jc_send_rst_cmd_and_wait_on_resp(g_spcom_client_ptr[slot],
                                                              (char*)resp_buf_ptr,
                                                              JC_SPCOM_MAX_ATR_BYTES,
                                                              &resp_size);
    }

    if (jc_spcom_result != JC_SPCOM_SUCCESS) {
        LOGE("spcom send reset cmd failed, spcom_result=%d, slot [%d]", jc_spcom_result, slot);
        event.event = UimRemoteClientEventStatusType::UIM_REMOTE_STATUS_CARD_ERROR;
    } else {
        atr_ptr.resize(resp_size);
        memscpy(atr_ptr.data(), resp_size, resp_buf_ptr, resp_size);
        event.atr = atr_ptr; /* copy atr vector array */
        event.event = UimRemoteClientEventStatusType::UIM_REMOTE_STATUS_CARD_RESET;
    }

    g_uim_rmt_client[slot]->UimRemoteServiceClientEventReq (1, event);
} /* sp_uim_remote_process_reset_ind */


/**
 * sp_uim_remote_process_card_status_ind()
 *
 * Processes the CARD STATUS indication queued by UIM REMOTE
 *
 * @param: uim_remote_ind_type
 *
 * @return: None
 */
static void sp_uim_remote_process_card_status_ind
(
  uim_remote_ind_type *card_status_ptr
)
{
    uint8_t num_apps  = 0;
    struct  sigevent     notify_timer_exp_evt;
    struct  itimerspec   timeout_val;
    uint8_t num_apps_halted  = 0;
    bool    start_sim2_thread  = false;

    if (card_status_ptr == NULL || card_status_ptr->slot > SLOT_1) {
        LOGE("Invalid Input card_status_ptr %lu", reinterpret_cast<uintptr_t>(card_status_ptr));
        return;
    }

    memset(&notify_timer_exp_evt, 0x00, sizeof(sigevent));
    memset(&timeout_val, 0x00, sizeof(itimerspec));

    num_apps = card_status_ptr->card_status_data.num_apps;

    /* Go through the vector to find an app whose state is READY */
    for (uint8_t app_info = 0; app_info < num_apps; app_info++) {
        UimRemoteClientAppInfo vec_app_info = card_status_ptr->card_status_data.vec[app_info];
        LOGD("App state %d App type %d Num slots %d ", (uint32_t)vec_app_info.appState, (uint32_t)vec_app_info.appType,
                                                        card_status_ptr->card_status_data.active_slots);

        switch ((uim_remote_app_state_enum_type)vec_app_info.appState) {
            /* For the below 2 cases, we'd either wait until the app state
               advances and in case it doesn't, then we rely on the sanity
               timer to start slot1 init. */
            case UIM_REMOTE_APP_STATE_UNKNOWN:
            case UIM_REMOTE_APP_STATE_DETECTED:
              break;

            case UIM_REMOTE_APP_STATE_HALTED:
              num_apps_halted++;

              if (num_apps_halted >= num_apps) {

                  /* If all the apps are halted then, stop the
                     sanity timer and start slot1 initialization. */
                  start_sim2_thread = true;
              }
              break;

            case UIM_REMOTE_APP_STATE_WAITING_ON_USER:
              /* Start a timer which would expire in case the user does not
                 enter PIN and indicate that we start slot1 init if the timer expires
                 and the app state has not advanced */
              pthread_mutex_lock(&g_user_input_timer_mutex);
              if (g_user_input_timer == NULL) {
                  notify_timer_exp_evt.sigev_notify            = SIGEV_THREAD;
                  notify_timer_exp_evt.sigev_notify_function   = sp_handle_waiting_on_user_timeout_evt;
                  notify_timer_exp_evt.sigev_value.sival_ptr   = NULL;
                  notify_timer_exp_evt.sigev_notify_attributes = NULL;

                  /* Set the timer values */
                  timeout_val.it_interval.tv_sec  = 0;
                  timeout_val.it_interval.tv_nsec = 0;
                  /* Assuming 10 seconds to be a good time to wait for the input */
                  timeout_val.it_value.tv_sec     = UIM_SIM1_PIN_INPUT_TIMER_VALUE;
                  timeout_val.it_value.tv_nsec    = 0;

                  if(timer_create(CLOCK_REALTIME, &notify_timer_exp_evt, &g_user_input_timer) != 0) {
                      LOGE("Error creating user input timer");
                      g_user_input_timer = NULL;
                      pthread_mutex_unlock(&g_user_input_timer_mutex);
                      break;
                  }

                  if(timer_settime(g_user_input_timer, 0, &timeout_val, NULL) != 0) {
                      LOGE("Error starting user input timer");
                      timer_delete(g_user_input_timer);
                      g_user_input_timer = NULL;
                      pthread_mutex_unlock(&g_user_input_timer_mutex);
                      break;
                  }
              }
              pthread_mutex_unlock(&g_user_input_timer_mutex);
              break;

            case UIM_REMOTE_APP_STATE_READY:
              LOGD("SIM1 app state ready; App type %d ", (uint32_t)vec_app_info.appType);
              /* We are looking at App Ready state for any one app on SIM1,
                 which is good enough to start SIM2 init. */
              start_sim2_thread  = true;
              break;

            default:
              break;
        }
        if (start_sim2_thread) {
            if (g_sanity_timer != NULL) {
                timer_delete(g_sanity_timer);
                g_sanity_timer = NULL;
            }
            resume_me(SLOT_2);
            break;
        }
    }

    LOGD("In sp_uim_remote_process_card_status_ind");
} /* sp_uim_remote_process_card_status_ind */

/**
 * sp_uim_remote_process_ind_queue()
 *
 * Entry function for processing the indications queued by UIM
 * REMOTE to SPDaemon
 *
 * @param: uim_remote_ind_type*
 *
 * @return: None
 */
void sp_uim_remote_process_ind_queue (slot_type queue_slot_id)
{
    uim_remote_ind_type uim_remote_ind;
    uint8_t             queue_cnt = 0;
    uint8_t             slot      = 0;

    memset(&uim_remote_ind, 0, sizeof(uim_remote_ind));

    while (sp_uim_remote_ind_queue_cnt(&queue_cnt, queue_slot_id) == 0 &&
           queue_cnt > 0) {
        if(sp_uim_remote_ind_queue_remove(&uim_remote_ind, queue_slot_id)) {
            /* Get and process next item from the queue */
            LOGD("queue empty.\n");
            continue;
        }

        slot = uim_remote_ind.slot;
        LOGD("sp_process_uim_remote_ind_queue removed uim_remote_ind.ind_type=%d, slot [%d]", uim_remote_ind.ind_type, slot);

        switch (uim_remote_ind.ind_type) {
            case UIM_REMOTE_POWER_UP_IND:
            case UIM_REMOTE_CONNECT_IND:
                sp_uim_remote_process_power_up_ind (&uim_remote_ind);
                break;
            case UIM_REMOTE_POWER_DOWN_IND:
                sp_uim_remote_process_power_down_ind (&uim_remote_ind);
                break;
            case UIM_REMOTE_RESET_IND:
                sp_uim_remote_process_reset_ind (&uim_remote_ind);
                break;
            case UIM_REMOTE_APDU_IND:
                sp_uim_remote_process_apdu_ind (&uim_remote_ind);
                break;
            case UIM_REMOTE_SERVICE_STATUS_CONNECTED_IND:
                sp_uim_remote_process_service_status_connected_ind (&uim_remote_ind);
                break;
            case UIM_REMOTE_SERVICE_STATUS_DISCONNECTED_IND:
            case UIM_REMOTE_FORCE_DISCONNECT_FROM_IUICC_IND:
                sp_uim_remote_process_service_status_disconnected_ind (&uim_remote_ind);
                break;
            case UIM_REMOTE_DISCONNECT_IND:
                break;
            case UIM_REMOTE_CARD_STATUS_IND:
                sp_uim_remote_process_card_status_ind(&uim_remote_ind);
                break;

            default:
                LOGE("Invalid UIM REMOTE indiationn received=%d", uim_remote_ind.ind_type);
                break;
        }
    }
} /* sp_process_uim_remote_ind_queue */

/*===========================================================================
  FUNCTIONS processing response (from UIM REMOTE) of the event request sent
  by the thread created by SPDaemon
===========================================================================*/
/**
 * UimRemoteClientResponseImpl::UimRemoteServiceClientEventResp()
 *
 * Processes the response sent by UIM REMOTE for the Client Event Req sent by
 * the thread created by SPDaemon
 *
 * @param: int32_t
 *         UimRemoteClientEventRsp
 *
 * @return: None
 */
Return<void> UimRemoteClientResponseImpl::UimRemoteServiceClientEventResp
(
  int32_t                 token,
  UimRemoteClientEventRsp eventResp)
{
    slot_type slot = (slot_type)this->slot;
    LOGD("token=%d, eventResp=%d slot=%d ", token, eventResp, slot);
    if (slot >= MAX_SLOT) {
        LOGE("Invalid slot ID [%d]", slot);
        return Void();
    }

    /* Power down the iUICC and deregister as iUICC client if the response
       is failure... we cant do much here!! */
    if (UimRemoteClientEventRsp::UIM_REMOTE_FAILURE == eventResp) {
        uim_remote_ind_type     uim_remote_ind;

        /* Reset the card on SPCOM interface */
        memset(&uim_remote_ind, 0, sizeof(uim_remote_ind));
        uim_remote_ind.ind_type = UIM_REMOTE_FORCE_DISCONNECT_FROM_IUICC_IND;
        uim_remote_ind.slot     = slot;
        /* Indication callback received from UIM REMOTE/QCRIL. Add it to our
           queue and return so that we do not block QCRIL. Indication will later be
           dequeued and process in this thread created by SPDaemon */
        if (sp_uim_remote_ind_queue_add(&uim_remote_ind, slot) == 0) {
            resume_me(slot);
        }
    }

    return Void();
} /* UimRemoteServiceClientEventResp */

/**
 * UimRemoteClientResponseImpl::UimRemoteServiceClientApduResp()
 *
 * Processes the response sent by UIM REMOTE for the Client APDU
 * Req sent by the thread created by SPDaemon
 *
 * @param: int32_t
 *         UimRemoteClientEventRsp
 *
 * @return: None
 */
Return<void> UimRemoteClientResponseImpl::UimRemoteServiceClientApduResp
( int32_t                token,
  UimRemoteClientApduRsp apduResp
)
{
    slot_type slot = (slot_type)this->slot;
    LOGD("token=%d, apduResp=%d slot [%d]", token, apduResp, slot);

    if (slot >= MAX_SLOT) {
        LOGE("Invalid slot ID [%d]", slot);
        return Void();
    }

    /* Power down the iUICC and deregister as iUICC client if the response
       is failure... we cant do much here!! */
    if (UimRemoteClientApduRsp::UIM_REMOTE_FAILURE == apduResp) {
        LOGD("APDU resp failed for slot [%d]",slot);
        uim_remote_ind_type     uim_remote_ind;

        /* Reset the card on SPCOM interface */
        memset(&uim_remote_ind, 0, sizeof(uim_remote_ind));
        uim_remote_ind.ind_type = UIM_REMOTE_FORCE_DISCONNECT_FROM_IUICC_IND;
        uim_remote_ind.slot     = slot;
        /* Indication callback received from UIM REMOTE/QCRIL. Add it to our
           queue and return so that we do not block QCRIL. Indication will later be
           dequeued and process in this thread created by SPDaemon */
        if (sp_uim_remote_ind_queue_add(&uim_remote_ind, slot) == 0) {
            resume_me(slot);
        }
    }

    return Void();
} /* UimRemoteServiceClientApduResp */

/**
 * UimRemoteClientResponseImpl::serviceDied()
 *
 * Implementation of what happens incase the UIM REMOTE service
 * dies
 *
 * @param: int32_t
 *         ::android::wp<::android::hidl::base::V1_0::IBase>
 *
 * @return: None
 */
void UimRemoteClientResponseImpl::serviceDied
(
  uint64_t token,
  const ::android::wp<::android::hidl::base::V1_0::IBase> &service)
{
    /* Service died reset when QCRIL restarts the SIM */
    uim_remote_ind_type     uim_remote_ind;
    (void) service;
    slot_type slot = (slot_type)this->slot;

    LOGD("serviceDied token=%lu slot [%d]", token, slot);

    if (slot >= MAX_SLOT) {
        LOGE("Invalid slot ID [%d]", slot);
        return;
    }

    /* It is possible that the modem might not come back again. So, lets power
       down the iUICC and dereg as iUICC client */
    memset(&uim_remote_ind, 0, sizeof(uim_remote_ind));
    uim_remote_ind.ind_type = UIM_REMOTE_FORCE_DISCONNECT_FROM_IUICC_IND;
    uim_remote_ind.slot     = slot;
    /* Indication callback received from UIM REMOTE/QCRIL. Add it to our
       queue and return so that we do not block QCRIL. Indication will later be
       dequeued and process in this thread created by SPDaemon */
    if (sp_uim_remote_ind_queue_add(&uim_remote_ind, slot) == 0) {
        resume_me(slot);
    }
} /* serviceDied */


/*===========================================================================
  FUNCTIONS queuing indications received from UIM REMOTE
===========================================================================*/
/**
 * UimRemoteClientResponseImpl::UimRemoteServiceClientApduInd()
 *
 * Function called by UIM REMOTE to queue the APDU indication to
 * UIM RMT indication queue, to be processed later in the thread created by
 * SPDaemon
 *
 * @param: hidl_vec<uint8_t>&
 *
 * @return: None
 */
Return<void> UimRemoteClientIndicationImpl::UimRemoteServiceClientApduInd
(
  const hidl_vec<uint8_t>& apduInd
)
{
    uint16_t            cmd_buf_len     = apduInd.size();
    uim_remote_ind_type uim_remote_ind;
    bool                is_ind_queued   = false;
    slot_type           slot            = (slot_type)this->slot;

    UimRemoteClientApduStatus apduStatus = UimRemoteClientApduStatus::UIM_REMOTE_FAILURE;
    std::vector<uint8_t>  apduResp;

    if (slot >= MAX_SLOT || g_uim_rmt_client[slot] == NULL) {
        LOGE("UimRemoteServiceClientApduInd: Error: slot [%d]\n", slot);
        return Void();
    }
    LOGD("Received APDU of size %d from modem for slot [%d]", cmd_buf_len, slot);

    memset(&uim_remote_ind, 0, sizeof(uim_remote_ind));

    if (cmd_buf_len > UIM_REMOTE_IUICC_MAX_REQ_SIZE ||
        cmd_buf_len > sizeof(uim_remote_ind.ind_data)) {
            LOGE("iUICC Apdu failure, request is too big");
    } else {
        uim_remote_ind.ind_type = UIM_REMOTE_APDU_IND;
        uim_remote_ind.slot     = slot;
        LOGD("Slot ID %d ", slot);
        memscpy(uim_remote_ind.ind_data.apdu_ind_data.data_buf,
        sizeof(uim_remote_ind.ind_data.apdu_ind_data.data_buf),
        apduInd.data(),
        cmd_buf_len);
        uim_remote_ind.ind_data.apdu_ind_data.data_len = cmd_buf_len;

        /* Indication callback received from UIM REMOTE/QCRIL. Add it to our
         queue and return so that we do not block QCRIL. Indication will later be
         dequeued and process in this thread created by SPDaemon */
        if (sp_uim_remote_ind_queue_add(&uim_remote_ind,slot) == 0) {
          resume_me(slot);
          is_ind_queued = true;
        }
    }

    if (!is_ind_queued) {
      /* Couldn't queue the indication to our queue. Send the error
         response back to UIM RMT from the caller thread itself */
      LOGD("Sending response for APDU IND slot0");
      g_uim_rmt_client[slot]->UimRemoteServiceClientApduReq (1, apduStatus, apduResp);
    }

    return Void();
} /* UimRemoteServiceClientApduInd */

/**
 * UimRemoteClientResponseImpl::UimRemoteServiceClientConnectInd()
 *
 * Function called by UIM REMOTE to queue the indication
 * informing the thread created by SPDaemon to start
 * connecting/powering up the iUICC, to be processed later in
 * this thread created by SPDaemon
 *
 * @param: None
 *
 * @return: None
 */
Return<void> UimRemoteClientIndicationImpl::UimRemoteServiceClientConnectInd()
{
    uim_remote_ind_type     uim_remote_ind;
    UimRemoteEventReqType   event;
    bool                    is_ind_queued  = false;
    slot_type               slot           = (slot_type)this->slot;

    memset(&event, 0, sizeof(event));

    event.event = UimRemoteClientEventStatusType::UIM_REMOTE_STATUS_CARD_ERROR;

    if (slot >= MAX_SLOT) {
        LOGE("Invalid slot value [%d]", slot);
        return Void();
    }

    LOGD("UIM REMOTE Connect indication, start connecting to the iUICC slot [%d]", slot);

    if (g_uim_rmt_client[slot] == NULL) {
        LOGE("Invalid UIM REMOTE client for slot [%d]", slot);
        return Void();
    }

    memset(&uim_remote_ind, 0, sizeof(uim_remote_ind));
    uim_remote_ind.ind_type = UIM_REMOTE_CONNECT_IND;
    uim_remote_ind.slot     = slot;
    LOGD("Slot ID %d ", slot);

    /* Indication callback received from UIM REMOTE/QCRIL. Add it to our
       queue and return so that we do not block QCRIL. Indication will later be
       dequeued and process in this thread created by SPDaemon */
    if (sp_uim_remote_ind_queue_add(&uim_remote_ind, slot) == 0) {
        resume_me(slot);
        is_ind_queued = true;
    }

    if (!is_ind_queued) {
        LOGD("Sending response for client connect IND slot0");
        /* Send card error event to modem. Is this the right thing to do? */
        g_uim_rmt_client[slot]->UimRemoteServiceClientEventReq (1, event);
    }

    return Void();
} /* UimRemoteServiceClientConnectInd */

/**
 * UimRemoteClientResponseImpl::UimRemoteServiceClientDisconnectInd()
 *
 * Function called by UIM REMOTE to queue the indication
 * informing the thread created by SPDaemon to start
 * disconnecting/powering down the iUICC, to be processed later
 * in this thread created by SPDaemon
 *
 * @param: None
 *
 * @return: None
 */
Return<void> UimRemoteClientIndicationImpl::UimRemoteServiceClientDisconnectInd()
{
    uim_remote_ind_type uim_remote_ind;
    slot_type           slot = (slot_type)this->slot;
    if (slot >= MAX_SLOT) {
        LOGE("Invalid slot ID [%d]", slot);
        return Void();
    }
    LOGD("UIM REMOTE disconnect indication, disconnect iUICC slot [%d]", slot);

    memset(&uim_remote_ind, 0, sizeof(uim_remote_ind));
    uim_remote_ind.ind_type = UIM_REMOTE_DISCONNECT_IND;
    uim_remote_ind.slot     = slot;

    /* Indication callback received from UIM REMOTE/QCRIL. Add it to our
       queue and return so that we do not block QCRIL. Indication will later be
       dequeued and process in this thread created by SPDaemon */
    if (sp_uim_remote_ind_queue_add(&uim_remote_ind, slot) == 0) {
        resume_me(slot);
    }
    return Void();
} /* UimRemoteServiceClientDisconnectInd */

/**
 * UimRemoteClientResponseImpl::UimRemoteServiceClientPowerUpInd()
 *
 * Function called by UIM REMOTE to queue the indication
 * informing the thread created by SPDaemon to send POWER UP
 * command to the iUICC, to be processed later in this thread
 * created by SPDaemon
 *
 * @param: None
 *
 * @return: None
 */
Return<void> UimRemoteClientIndicationImpl::UimRemoteServiceClientPowerUpInd
(
  bool                        hasTimeOut,
  int32_t                     timeOut,
  bool                        hasVoltageClass,
  UimRemoteClientVoltageClass powerUpVoltageClass
)
{
    uim_remote_ind_type     uim_remote_ind;
    UimRemoteEventReqType   event;
    bool                    is_ind_queued    = false;
    slot_type               slot = (slot_type)this->slot;

    if (slot >= MAX_SLOT) {
        LOGE("Invalid slot ID [%d]", slot);
        return Void();
    }

    /* Got the card power up request, power up the java card and send atr */
    LOGD("UIM REMOTE indication, power up the iUICC %d, %d, %d, %d", hasTimeOut, timeOut, hasVoltageClass, powerUpVoltageClass);

    if (g_uim_rmt_client[slot] == NULL) {
        LOGE("invalid UIM REMOTE client for slot [%d]", slot);
        return Void();
    }

    memset(&event, 0, sizeof(event));
    event.event = UimRemoteClientEventStatusType::UIM_REMOTE_STATUS_CARD_ERROR;

    memset(&uim_remote_ind, 0, sizeof(uim_remote_ind));
    uim_remote_ind.ind_type = UIM_REMOTE_POWER_UP_IND;
    uim_remote_ind.slot     = slot;
    LOGD("Slot ID %d ", slot);

    /* Indication callback received from UIM REMOTE/QCRIL. Add it to our
       queue and return so that we do not block QCRIL. Indication will later be
       dequeued and process in this thread created by SPDaemon */
    if (sp_uim_remote_ind_queue_add(&uim_remote_ind,slot) == 0) {
        resume_me(slot);
        is_ind_queued = true;
    }

    if (!is_ind_queued) {
        /* Send card error event to modem. Is this the right thing to do? */
        LOGD("Sending response for pup IND slot [%d]", slot);
        g_uim_rmt_client[slot]->UimRemoteServiceClientEventReq (1, event);
    }

    return Void();
} /* UimRemoteServiceClientPowerUpInd */

/**
 * UimRemoteClientResponseImpl::UimRemoteServiceClientPowerDownInd()
 *
 * Function called by UIM REMOTE to queue the indication
 * informing the thread created by SPDaemon to send POWER DOWN
 * command to the iUICC, to be processed later in this thread
 * created by SPDaemon
 *
 * @param: None
 *
 * @return: None
 */
Return<void> UimRemoteClientIndicationImpl::UimRemoteServiceClientPowerDownInd
(
  bool                         hasPowerDownMode,
  UimRemoteClientPowerDownMode powerDownMode
)
{
    uim_remote_ind_type     uim_remote_ind;
    UimRemoteEventReqType   event;
    bool                    is_ind_queued    = false;
    slot_type               slot = (slot_type)this->slot;

    if (slot >= MAX_SLOT) {
        LOGE("Invalid slot ID [%d]", slot);
        return Void();
    }

    /* Got the power down request, power down the java card */
    LOGD("UIM REMOTE indication for powering down the iUICC...%d, %d", hasPowerDownMode, powerDownMode);

    if (g_uim_rmt_client[slot] == NULL) {
        LOGE("invalid UIM REMOTE client for slot [%d]", slot);
        return Void();
    }
    memset(&event, 0, sizeof(event));
    event.event = UimRemoteClientEventStatusType::UIM_REMOTE_STATUS_CARD_ERROR;

    memset(&uim_remote_ind, 0, sizeof(uim_remote_ind));
    uim_remote_ind.ind_type = UIM_REMOTE_POWER_DOWN_IND;
    uim_remote_ind.slot     = slot;
    LOGD("Slot ID %d ", slot);

    /* Indication callback received from UIM REMOTE/QCRIL. Add it to our
       queue and return so that we do not block QCRIL. Indication will later be
       dequeued and process in this thread created by SPDaemon */
    if (sp_uim_remote_ind_queue_add(&uim_remote_ind, slot) == 0) {
        resume_me(slot);
        is_ind_queued = true;
    }

    if (!is_ind_queued) {
        /* Send card error event to modem. Is this the right thing to do given
           that the indication from modem was to power down the card? */
        LOGD("Sending response for pdown IND slot [%d]", slot);
        g_uim_rmt_client[slot]->UimRemoteServiceClientEventReq (1, event);
    }

    return Void();
} /* UimRemoteServiceClientPowerDownInd */

/**
 * UimRemoteClientResponseImpl::UimRemoteServiceClientResetInd()
 *
 * Function called by UIM REMOTE to queue the indication
 * informing the thread created by SPDaemon to send RESET
 * command to the iUICC, to be processed later in this thread
 * created by SPDaemon
 *
 * @param: None
 *
 * @return: None
 */
Return<void> UimRemoteClientIndicationImpl::UimRemoteServiceClientResetInd()
{
    uim_remote_ind_type     uim_remote_ind;
    UimRemoteEventReqType   event;
    bool                    is_ind_queued    = false;
    slot_type               slot = (slot_type)this->slot;

    if (slot >= MAX_SLOT) {
        LOGE("Invalid slot ID [%d]", slot);
        return Void();
    }

    LOGD("UIM REMOTE indication for resetting the iUICC, slot [%d]", slot);

    if (g_uim_rmt_client[slot] == NULL) {
        LOGE("invalid UIM REMOTE client for slot0");
        return Void();
    }

    memset(&event, 0, sizeof(event));
    event.event = UimRemoteClientEventStatusType::UIM_REMOTE_STATUS_CARD_ERROR;

    memset(&uim_remote_ind, 0, sizeof(uim_remote_ind));
    uim_remote_ind.ind_type = UIM_REMOTE_RESET_IND;
    uim_remote_ind.slot     = slot;
    /* Indication callback received from UIM REMOTE/QCRIL. Add it to our
       queue and return so that we do not block QCRIL. Indication will later be
       dequeued and process in this thread created by SPDaemon */
    if (sp_uim_remote_ind_queue_add(&uim_remote_ind, slot) == 0) {
        resume_me(slot);
        is_ind_queued = true;
    }

    if (!is_ind_queued) {
        /* Send card error event to modem. Is this the right thing to do? */
        LOGD("Sending response for Reset IND slot [%d]", slot);
        g_uim_rmt_client[slot]->UimRemoteServiceClientEventReq (1, event);
    }

    return Void();
} /* UimRemoteServiceClientResetInd */

/**
 * UimRemoteClientResponseImpl::UimRemoteServiceClientResetInd()
 *
 * Function called by UIM REMOTE to queue the indication
 * informing the thread created by SPDaemon the status of the
 * service from Modem (e.g. disconnected and then connected, in
 * case of Modem SSR)
 *
 * @param: boolean
 *
 * @return: None
 */
Return<void> UimRemoteClientIndicationImpl:: UimRemoteServiceClientServiceInd (bool status)
{
    uim_remote_ind_type     uim_remote_ind;
    slot_type               slot = (slot_type)this->slot;

    if (slot >= MAX_SLOT) {
        LOGE("Invalid slot ID [%d]", slot);
        return Void();
    }

	/* In case the number of active slots is 1, we don't have
	   to queue a CONNECTED IND for SIM2.
	   Since the slot number is obtained from the thread
	   context and QCRIL sends this callback regardless of the slot,
	   we would have to look for the number of active slots and queue this
	   command accordingly. */
	if (slot == SLOT_2 && (sp_uim_get_num_active_slots() < MAX_NUM_SLOTS)) {
	    LOGD("Number of active slots is one, not powering up iuicc1");
        return Void();
	}

    /* Reset the card on SPCOM interface */
    LOGD("UIM REMOTE service status=%d indication, slot [%d]", status, slot);

    memset(&uim_remote_ind, 0, sizeof(uim_remote_ind));
    /* if status true, qmi uim remote service is up */
    if (status) {
        uim_remote_ind.ind_type = UIM_REMOTE_SERVICE_STATUS_CONNECTED_IND;
        uim_remote_ind.slot     = slot;
    } else {
        uim_remote_ind.ind_type = UIM_REMOTE_SERVICE_STATUS_DISCONNECTED_IND;
        uim_remote_ind.slot     = slot;
    }

    /* Indication callback received from UIM REMOTE/QCRIL. Add it to our
       queue and return so that we do not block QCRIL. Indication will later be
       dequeued and process in this thread created by SPDaemon */
    if (sp_uim_remote_ind_queue_add(&uim_remote_ind, slot) == 0) {
        resume_me(slot);
    }
    return Void();
} /* UimRemoteServiceClientServiceInd */

/**
 * UimRemoteClientIndicationImpl::UimRemoteServiceClientCardInitStatusInd()
 * Function called by UIM REMOTE to queue the indication
 * informing the thread created by SPDaemon the status of the
 * cards. 
 * 
 * @param UimRemoteClientCardInitStatusType 
 * 
 * @return None
 */
Return<void> UimRemoteClientIndicationImpl::UimRemoteServiceClientCardInitStatusInd(const UimRemoteClientCardInitStatusType& cardInitStatusInd)
{
    uim_remote_ind_type     uim_remote_ind;
    slot_type               slot = (slot_type)this->slot;

    LOGD("UimRemoteServiceClientCardInitStatusInd, slot %d ", slot);

    /* For now, we are only interested in the card status IND of slot0 */
    if (slot > SLOT_1) {
        return Void();
    }

    /* Update the number of active slots */
    pthread_mutex_lock(&g_active_slots_mutex);
    g_number_active_slots = cardInitStatusInd.numOfActiveSlots;
    pthread_mutex_unlock(&g_active_slots_mutex);

    memset(&uim_remote_ind, 0x00, sizeof(uim_remote_ind_type));

    uim_remote_ind.ind_type = UIM_REMOTE_CARD_STATUS_IND;
    uim_remote_ind.slot     = slot;

    uim_remote_ind.card_status_data.active_slots = cardInitStatusInd.numOfActiveSlots;
    uim_remote_ind.card_status_data.num_apps     = cardInitStatusInd.numOfApps;
    uim_remote_ind.card_status_data.vec.setToExternal(const_cast<UimRemoteClientAppInfo*>(cardInitStatusInd.appInfo.data()),
                                                      cardInitStatusInd.appInfo.size());

    LOGD("UimRemoteServiceClientCardInitStatusInd, num active slots %d, num apps %d ", uim_remote_ind.card_status_data.active_slots,
                                                                                       uim_remote_ind.card_status_data.num_apps);

    if (sp_uim_remote_ind_queue_add(&uim_remote_ind, slot) == 0) {
        resume_me(slot);
    }

    return Void();
} /* UimRemoteServiceClientCardInitStatusInd */


/*===========================================================================
  FUNCTIONS such as thread creation and thread main entry funtions
===========================================================================*/
/**
 * sp_uim_remote_notify_ssr_cb() - a callback to notify on
 * SP SubSystem-Reset (SSR).
 *
 * The spcom shall notify upon SP reset.
 * This callback should wait until the SP LINK is UP agin and
 * then perform re-registration with iuicc on the SPr.
 */
static void sp_uim_remote_notify_ssr_cb(void)
{
    LOGD("SP subsystem reset detected for slot [0].\n");
    pthread_mutex_lock(&g_pthread_mutex[SLOT_1]);
    if (!g_sp_reset_detected[SLOT_1]) {
        g_sp_reset_detected[SLOT_1] = true;
    }
    pthread_mutex_unlock(&g_pthread_mutex[SLOT_1]);
    resume_me(SLOT_1);
} /* sp_uim_remote_notify_ssr_cb */


/**
 * sp_uim_remote_notify_ssr_cb() - a callback to notify on
 * SP SubSystem-Reset (SSR).
 *
 * The spcom shall notify upon SP reset.
 * This callback should wait until the SP LINK is UP agin and
 * then perform re-registration with iuicc on the SPr.
 */
static void sp_uim_remote_notify_ssr_cb_2(void)
{
    LOGD("SP subsystem reset detected for slot [1].\n");
    pthread_mutex_lock(&g_pthread_mutex[SLOT_2]);
    if (!g_sp_reset_detected[SLOT_2]) {
        g_sp_reset_detected[SLOT_2] = true;
    }
    pthread_mutex_unlock(&g_pthread_mutex[SLOT_2]);
    resume_me(SLOT_2);
} /* sp_uim_remote_notify_ssr_cb */

/**
 * sp_uim_remote_start_func()
 *
 * Entry point function for
 * sp_uim_remote code, called once the sp_uim_remote
 * thread is created
 *
 * return: None
 */
static void *sp_uim_remote_start_func(void* data)
{
    slot_type slot = (slot_type)((size_t)data);
    LOGD("Slot in sp_uim_remote_start_func %d ", slot);
    if (slot >= MAX_SLOT) {
        LOGE("Invalid Param slot [%d].\n", slot);
        pthread_exit(NULL);
        return NULL;
    }
    /* Initialize the queue used by QCRIL to deliver modem traffic to the
       thread created by SPDaemon */
    if(sp_uim_remote_ind_queue_init(slot) < 0) {
        pthread_exit(NULL);
        return NULL;
    }

    pthread_cond_init(&g_pthread_cond[slot], NULL);
    pthread_mutex_init(&g_pthread_mutex[slot], NULL);

    /* For SLOT 2, wait until the timer expires and then begin initialization.
       We'd ideally want SIM1 to reach SUB READY and then start SIM1 initialization.
       But in the case of SIM1 not able to reach SUB READY, we would wait until the
       sanity or user input timer expires to begin SIM2 initialization. */
    if(slot == SLOT_2) {
        suspend_me(slot);
        LOGD("SIM2 init triggered");
    }

    /* Register with QMI UIM REMOTE as client (for SPDaemon<->Modem bridge
       needed to realize the Modem<->SPDaemon<->iuicc_on_SPSS link). Once this
       registration is successful (communicated to us via an indication), we
       will proceed with registration with iUICC on the SP. */
    if(sp_uim_remote_register_with_modem(slot) == false) {
        pthread_exit(NULL);
        return NULL;
    }

    while (1) {
        LOGD("Go to sleep, wake up upon receiving command from modem, or upon SP reset.\n");
        pthread_cond_init(&g_pthread_cond[slot], NULL);
        pthread_mutex_init(&g_pthread_mutex[slot], NULL);

        suspend_me(slot);

        /* Whether or not we were processing SP SSR, Modem might have posted
           an indication/command to us. Make sure to process any queue item
           and then suspend the thread once/if the queue is empty */
        sp_uim_remote_process_ind_queue(slot);

        pthread_mutex_lock(&g_pthread_mutex[slot]);
        if (g_sp_reset_detected[slot]) {
            /* we got resumed by SP SSR callback. At this point, the iUICC app
               on the SP is already reset. We clean up the iUICC channel
               that we had registered to prior to SP going into SSR. Then,
               we wait until the SP link is UP again and the iUICC app on the
               SP is loaded again. iUICC at this point is in POWER_DOWN state.
               So, any APDU command that it receives from Modem from this point
               on will result in an error from iUICC. After a couple retries of
               such a failing command, Modem will POWER UP the card again and
               we expect everything to resume as normal from that point
               onwards.

               Following are some scenarios needed to be handled regarding any
               pending command from Modem.
               1. Command from Modem already sent to iUICC:
                    The processing of commands from Modem is a sync call and
                    hence if we reach here, it is certain that no command
                    is pending with iUICC.
               2. Command from Modem is received by us (sitting in our queue)
                  but not sent to iUICC yet:
                    Wait until the iUICC is back up again and then send the
                    command to iUICC. If the wait is too much, Modem/UIM based
                    command response timeout timer will kick off eventually,
                    and Modem will retry the command. Or if the command fails
                    because iUICC is in POWER DOWN state, again, the Modem will
                    retry the command until Modem RESETS the iUICC (POWER DOWN
                    followed by POWER UP) again.
               3. No command from Modem in progress:
                    Need to ensure Modem  */
            g_sp_reset_detected[slot] = false;
            pthread_mutex_unlock(&g_pthread_mutex[slot]);

            sp_uim_remote_iuicc_cleanup_and_dereg(slot);
            sp_uim_remote_register_with_iuicc(slot);
        } else {
            pthread_mutex_unlock(&g_pthread_mutex[slot]);
        }
    }

    /* We shouldn't reach here. */
    pthread_exit(NULL);
    return NULL;
} /* sp_uim_remote_start_func */


/**
 * sp_uim_remote_create_thread()
 *
 * Creates a pthread so that sp_uim_remote can function in
 * its own independent thread.
 *
 * return: None
 */
void sp_uim_remote_create_thread(void)
{
    static pthread_t sp_uim_remote_thread[MAX_NUM_SLOTS];
    int       rc                         = -EPERM;

    for (int i = 0; i < MAX_NUM_SLOTS; i++) {
        rc = pthread_create(&sp_uim_remote_thread[i],
                            NULL,
                            &sp_uim_remote_start_func,
                            (void*)(slot_type)i);

        if (rc != 0) {
            LOGE("failed to create thread");
            return;
        }
        LOGD("created thread for modem uim remote communication with SP for instance [%d]", i);
    }
} /* sp_uim_remote_create_thread */
