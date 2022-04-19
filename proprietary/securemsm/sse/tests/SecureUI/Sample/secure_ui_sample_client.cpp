/********************************************************************
---------------------------------------------------------------------
 Copyright (c) 2013-2016, 2018-2019 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
----------------------------------------------------------------------
SecureUI Sample/Test Client app.
*********************************************************************/

#include <stdlib.h>
#include <utils/Log.h>
#include "QSEEComAPI.h"
#include "common_log.h"
#include <signal.h>
#include <SecureUILib.h>
#include <sys/eventfd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#if defined(DISPLAY_CONFIG_1_3)
#include <hidl/Status.h>
#include <vendor/display/config/1.3/IDisplayConfig.h>
using namespace android;
using android::sp;
using vendor::display::config::V1_3::IDisplayConfig;
#endif //DISPLAY_CONFIG_1_3

/** adb log */
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SECURE_UI_SAMPLE_CLIENT: "
#ifdef LOG_NDDEBUG
#undef LOG_NDDEBUG
#endif
#define LOG_NDDEBUG 0 //Define to enable LOGD
#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#endif
#define LOG_NDEBUG  0 //Define to enable LOGV

#define LOGD_PRINT(...) do { LOGD(__VA_ARGS__); printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGE_PRINT(...) do { LOGE(__VA_ARGS__); printf(__VA_ARGS__); printf("\n"); } while(0)

/* commands */
#define SEC_UI_SAMPLE_CMD0_START_SEC_UI     0
#define SEC_UI_SAMPLE_CMD1_SHOW_IMAGE       1
#define SEC_UI_SAMPLE_CMD2_STOP_DISP        2
#define SEC_UI_SAMPLE_CMD3_SHOW_IMGAGE_PTR  3
#define SEC_UI_SAMPLE_CMD4_TEST_APP_BUFFER  4

#define SEC_UI_SAMPLE_CMD10_GET_PIN           10
#define SEC_UI_SAMPLE_CMD11_LOGIN             11
#define SEC_UI_SAMPLE_CMD12_MSG_PIN           12
#define SEC_UI_SAMPLE_CMD13_MSG_LOGIN         13
#define SEC_UI_SAMPLE_CMD14_MSG_CLEAN_UP      14
#define SEC_UI_SAMPLE_CMD15_CUSTOM_DIALOG     15
#define SEC_UI_SAMPLE_CMD16_MSG_CUSTOM        16
#define SEC_UI_SAMPLE_CMD50_CONTEXT_CLEAN_UP  50

#define MAX_FILENAME_LEN (256)
#define SAMPLE_IMAGE1_PATH "/data/vendor/tui/sec_img1__194_259_RGBA8888"
#define SAMPLE_IMAGE2_PATH "/data/vendor/tui/sec_img2__196_244_RGBA8888"
#define SAMPLE_IMAGE1_PATH_LEN (strlen(SAMPLE_IMAGE1_PATH)+1)
#define SAMPLE_IMAGE2_PATH_LEN (strlen(SAMPLE_IMAGE2_PATH)+1)
#define SAMPLE_IMAGE1_HEIGHT 194
#define SAMPLE_IMAGE1_WIDTH 259
#define SAMPLE_IMAGE2_HEIGHT 196
#define SAMPLE_IMAGE2_WIDTH 244
#define SAMPLE_IMAGE_X 100
#define SAMPLE_IMAGE_Y 100
#define MAX(A,B)   (((A) > (B)) ? (A) : (B))

#define SEC_UI_DEMO_LOGO_PATH    "/data/vendor/tui/logo.png"
#define SEC_UI_DEMO_IND_PATH     "/data/vendor/tui/sec_ind.png"
#define SEC_UI_SAMPLE_APP_NAME_32BIT   "secure_ui_sample"
#define SEC_UI_SAMPLE_APP_NAME_64BIT   "secure_ui_sample64"
#define SEC_UI_SAMPLE_SH_MEM_SIZE   1024
#define TOUCH_EVENT_TIMEOUT     10000   /* in mili seconds */
#define SLEEP_TIME_BEFORE_ABORT  50    /* in seconds */
#define TEST_ITERATION 1
#define FAILURE (-1)

struct send_cmd{
	uint32_t cmd_id;
	uint32_t height;
	uint32_t width;
	uint32_t x;
	uint32_t y;
	uint32_t timeout;
	char img_path[MAX_FILENAME_LEN];
 };

struct send_cmd_rsp{
	int32_t status;
 };

/** Mutex protecting the send_command function (only 1 command at a time) */
static pthread_mutex_t command_mtx = PTHREAD_MUTEX_INITIALIZER;

static int g_efd = -1;
static volatile int g_run = 1;
static char* g_app_name = NULL;
int32_t issue_send_cmd(struct QSEECom_handle *l_QSEEComHandle,
                                         struct send_cmd *send_cmd);

extern "C" int svc_sock_send_notification_noreg(
        uint8_t const id,
        uint8_t const payload,
        char const * const dest_name,
        size_t const dest_len);

void abort_secure_ui(bool auth)
{
    char *lst_name = "\0suilst";
    if (true == auth)
        StopAuthenticateSecureDisplay();
    svc_sock_send_notification_noreg(0x2, 0x0F, lst_name, 1 + strlen(lst_name+1));
}

int32_t qsc_start_app(struct QSEECom_handle **l_QSEEComHandle,
        const char *appname, int32_t buf_size)
{
    int32_t ret = 0;

    /* start the application */
    ret = QSEECom_start_app(l_QSEEComHandle, "/vendor/firmware_mnt/image/",
            appname, buf_size);
    if (ret) {
        ret = QSEECom_start_app(l_QSEEComHandle, "/firmware/image",
                appname, buf_size);
        if (ret) {
            ret = QSEECom_start_app(l_QSEEComHandle, "/vendor/firmware",
                    appname, buf_size);
        }
    }
    if (ret) {
        LOGE_PRINT("Loading app -%s failed",appname);
    } else {
        LOGD("Loading app -%s succeded",appname);
        QSEECom_set_bandwidth(*l_QSEEComHandle, true);
    }

    return ret;
}

/**@brief:  Implement simple shutdown app
 * @param[in]	handle
 * @return	zero on success or error count on failure.
 */
int32_t qsc_shutdown_app(struct QSEECom_handle **l_QSEEComHandle)
{
    int32_t ret = 0;
    struct send_cmd cmd = {0};

    if (*l_QSEEComHandle != NULL) {
        cmd.cmd_id = SEC_UI_SAMPLE_CMD50_CONTEXT_CLEAN_UP;
        ret = issue_send_cmd(*l_QSEEComHandle,&cmd);
        if(ret != 0){
            LOGE_PRINT(" Failed SEC_UI_SAMPLE_CMD50_CONTEXT_CLEAN_UP %d",ret);
            return ret;
        }
        LOGD("qsc_shutdown_app: start");
        QSEECom_set_bandwidth(*l_QSEEComHandle, false);
        /* shutdown the application */
        ret = QSEECom_shutdown_app(l_QSEEComHandle);
        if (ret) {
            LOGE_PRINT("Shutdown app failed with ret = %d", ret);
        } else {
            LOGD("shutdown app: pass");
        }
    } else {
        LOGE_PRINT("cannot shutdown as the handle is NULL");
    }
    return ret;
}

int32_t issue_send_cmd(struct QSEECom_handle *l_QSEEComHandle,
        struct send_cmd *send_cmd)
{
    int32_t ret = 0;
    int32_t req_len = 0;
    int32_t rsp_len = 0;
    struct send_cmd_rsp *msgrsp;	/* response data sent from QSEE */

    pthread_mutex_lock(&command_mtx); // only 1 command at a time

    /* populate the data in shared buffer */
    memcpy(l_QSEEComHandle->ion_sbuffer,send_cmd, sizeof(struct send_cmd));

    req_len = sizeof(struct send_cmd);
    rsp_len = sizeof(struct send_cmd_rsp);

    if (req_len & QSEECOM_ALIGN_MASK)
        req_len = QSEECOM_ALIGN(req_len);

    if (rsp_len & QSEECOM_ALIGN_MASK)
        rsp_len = QSEECOM_ALIGN(rsp_len);

    msgrsp=(struct send_cmd_rsp *)(l_QSEEComHandle->ion_sbuffer + req_len);
    /* send request from HLOS to QSEApp */
    ret = QSEECom_send_cmd(l_QSEEComHandle,
            l_QSEEComHandle->ion_sbuffer,
            req_len,
            msgrsp,
            rsp_len);

    pthread_mutex_unlock(&command_mtx);

    if (ret) {
        LOGE_PRINT("send command %d failed with ret = %d\n", send_cmd->cmd_id,ret);
        return ret;
    }
    return msgrsp->status;
}

static void * abort_ui(void* arg){
    uint32_t * max_sleep_time = (uint32_t *) arg;
    srand(time(0));
    int32_t sleep_time = rand() % (*max_sleep_time) +1;
    uint64_t c = 1;

    LOGD_PRINT("started abort ui thread, aborting in %d seconds", sleep_time);

    while ((sleep_time-- > 0) && (g_run))
        sleep(1);
    write(g_efd,&c,sizeof(c));

    LOGD_PRINT("abort ui thread finished");
    return NULL;
}

int display_basic_test(bool auth) {
    struct send_cmd cmd = {0};
    int32_t ret, status;
    struct QSEECom_handle *l_QSEEComHandle = NULL;
    struct stat st;
    uint32_t lib_mem_size;
    uint32_t max_mem_size;

#if defined(DISPLAY_CONFIG_1_3)
    sp<IDisplayConfig> intf = IDisplayConfig::getService();
    if (intf == NULL) {
      LOGE_PRINT("secure display basic test: failed to communicate to"
              "display HAL");
      return FAILURE;
    }

    LOGD_PRINT("DISABLE MDP Idle Power Collapse for Secure UI Session");
    ret = intf->controlIdlePowerCollapse(0, false);
    if (ret) {
        LOGE_PRINT("IDisplayConfig::controlIdlePowerCollapse failed to "
                "DISABLE MDP Idle PC=[%d] ", ret);
        LOGE_PRINT("Make sure that the display is ON");
        return FAILURE;
    }
#endif

    if(stat(SAMPLE_IMAGE1_PATH, &st) != 0 || stat(SAMPLE_IMAGE2_PATH, &st) != 0) {
        LOGE_PRINT("secure display basic test: test images are missing, see securemsm/sse/tests/SecureUI/images/README.txt for instructions");
        return FAILURE;
    }

    ret = GetSharedMemorySize(&lib_mem_size);
    if (ret)
    {
        LOGE_PRINT("GetSharedMemorySize failed");
        return FAILURE;
    }
    max_mem_size = MAX(SEC_UI_SAMPLE_SH_MEM_SIZE, lib_mem_size);
    LOGD_PRINT("Shared buffer size: %d", max_mem_size);

    ret = qsc_start_app(&l_QSEEComHandle, g_app_name, max_mem_size);
    if (ret) {
        LOGE_PRINT("Start app: fail");
        return FAILURE;
    } else {
        LOGD_PRINT("Start app: pass");
    }
    do{
        cmd.cmd_id = SEC_UI_SAMPLE_CMD0_START_SEC_UI;
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret != 0){
            LOGE_PRINT("   Failed SEC_UI_SAMPLE_CMD0_START_SEC_UI: %d",ret);
            ret = FAILURE;
            break;
        }
        LOGD_PRINT("   Succeeded SEC_UI_SAMPLE_CMD0_START_SEC_UI (%d)",ret);

        if (true == auth) {
            LOGD_PRINT("Starting AuthenticateSecureDisplay....");
            ret = StartAuthenticateSecureDisplay(l_QSEEComHandle, &command_mtx);
            if (0 == ret) {
                LOGD_PRINT("Authenticate Thread created succefully !");
            } else {
                LOGD_PRINT("Could not create Authenticate Thread ...  !");
                abort_secure_ui(auth);
                break;
            }
        }

        cmd.cmd_id = SEC_UI_SAMPLE_CMD1_SHOW_IMAGE;
        cmd.height = SAMPLE_IMAGE1_HEIGHT;
        cmd.width = SAMPLE_IMAGE1_WIDTH;
        cmd.x = SAMPLE_IMAGE_X;
        cmd.y = SAMPLE_IMAGE_Y;
        strlcpy(cmd.img_path, SAMPLE_IMAGE1_PATH,SAMPLE_IMAGE1_PATH_LEN);
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret < 0){
            LOGE_PRINT("   Failed SEC_UI_SAMPLE_CMD1_SHOW_IMAGE: 0 %d",ret);
            abort_secure_ui(auth);
            ret = FAILURE;
            break;
        }
        printf("   Succeeded SEC_UI_SAMPLE_CMD1_SHOW_IMAGE for the first image\n"
                "press enter to continue ... \n");
        getchar();
        cmd.cmd_id = SEC_UI_SAMPLE_CMD3_SHOW_IMGAGE_PTR;
        cmd.height = SAMPLE_IMAGE2_HEIGHT;
        cmd.width = SAMPLE_IMAGE2_WIDTH;
        cmd.x = SAMPLE_IMAGE_X;
        cmd.y = SAMPLE_IMAGE_Y;
        strlcpy(cmd.img_path, SAMPLE_IMAGE2_PATH,SAMPLE_IMAGE2_PATH_LEN);
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret < 0){
            LOGE_PRINT("   Failed SEC_UI_SAMPLE_CMD1_SHOW_IMAGE_PTR: 0 %d",ret);
            abort_secure_ui(auth);
            ret = FAILURE;
            break;
        }
        printf("   Succeeded SEC_UI_SAMPLE_CMD1_SHOW_IMAGE_PTR for the second image\n"
                "press enter to continue ... \n");
        getchar();

        cmd.cmd_id = SEC_UI_SAMPLE_CMD1_SHOW_IMAGE;
        cmd.height = SAMPLE_IMAGE1_HEIGHT;
        cmd.width = SAMPLE_IMAGE1_WIDTH;
        cmd.x = SAMPLE_IMAGE_X;
        cmd.y = SAMPLE_IMAGE_Y;
        strlcpy(cmd.img_path, SAMPLE_IMAGE1_PATH,SAMPLE_IMAGE1_PATH_LEN);
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret < 0){
            LOGE_PRINT("   Failed SEC_UI_SAMPLE_CMD1_SHOW_IMAGE: 0 %d",ret);
            abort_secure_ui(auth);
            ret = FAILURE;
            break;
        }
        printf("   Succeeded SEC_UI_SAMPLE_CMD1_SHOW_IMAGE for the first image\n"
                "press enter to continue ... \n");

        cmd.cmd_id = SEC_UI_SAMPLE_CMD1_SHOW_IMAGE;
        cmd.height = SAMPLE_IMAGE1_HEIGHT;
        cmd.width = SAMPLE_IMAGE1_WIDTH;
        cmd.x = SAMPLE_IMAGE_X;
        cmd.y = SAMPLE_IMAGE_Y;
        strlcpy(cmd.img_path, SAMPLE_IMAGE1_PATH,SAMPLE_IMAGE1_PATH_LEN);
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret < 0){
            LOGE_PRINT("   Failed SEC_UI_SAMPLE_CMD1_SHOW_IMAGE: 0 %d",ret);
            abort_secure_ui(auth);
            ret = FAILURE;
            break;
        }
        printf("   Succeeded SEC_UI_SAMPLE_CMD1_SHOW_IMAGE for the first image\n"
                "press enter to continue ... \n");
        getchar();
        /* The second stage aborts the secure ui and tries to show image 2
         * showing the image should fail, if not we try to send stop display command and quit */
        abort_secure_ui(auth);
        printf("   Secure UI aborted! \n");

        cmd.cmd_id = SEC_UI_SAMPLE_CMD1_SHOW_IMAGE;
        cmd.height = SAMPLE_IMAGE2_HEIGHT;
        cmd.width = SAMPLE_IMAGE2_WIDTH;
        cmd.x = SAMPLE_IMAGE_X;
        cmd.y = SAMPLE_IMAGE_Y;
        strlcpy(cmd.img_path, SAMPLE_IMAGE2_PATH,SAMPLE_IMAGE2_PATH_LEN);
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret == 0){
            LOGE_PRINT("    SEC_UI_SAMPLE_CMD1_SHOW_IMAGE didn't fail, ret = %d", ret);
            cmd.cmd_id = SEC_UI_SAMPLE_CMD2_STOP_DISP;
            ret = issue_send_cmd(l_QSEEComHandle,&cmd);
            if(ret < 0){
                LOGE_PRINT("   Failed SEC_UI_SAMPLE_CMD2_STOP_DISP: %d",ret);
                break;
            }
            printf("   Succeeded SEC_UI_SAMPLE_CMD2_STOP_DISP\n");
            ret = FAILURE;
            break;
        }
        printf("   SEC_UI_SAMPLE_CMD1_SHOW_IMAGE failed as required \n"
                "press enter to continue ...\n");

        getchar();
        /* The second stage aborts the secure ui and tries to show image 2
         * showing the image should fail, if not we try to send stop display command and quit */


        /*The last part of the test is the following sequence:
         * 1. start the secure ui again
         * 2. show image 2
         * 3. switch to image 1
         * 4. stop secure display with request
         *
         * at any stage if failed, abort the secure ui and quit  */

        //step 1

        cmd.cmd_id = SEC_UI_SAMPLE_CMD2_STOP_DISP;
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret < 0){
            LOGE_PRINT("   Failed SEC_UI_SAMPLE_CMD2_STOP_DISP: %d",ret);
        } else {
            LOGD_PRINT("   Succeeded SEC_UI_SAMPLE_CMD2_STOP_DISP");
        }

        cmd.cmd_id = SEC_UI_SAMPLE_CMD0_START_SEC_UI;
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret < 0){
            LOGE_PRINT("   Failed SEC_UI_SAMPLE_CMD0_START_SEC_UI: %d",ret);
            ret = FAILURE;
            break;
        }
        printf("   Succeeded SEC_UI_SAMPLE_CMD0_START_SEC_UI\n");

        //step 2
        cmd.cmd_id = SEC_UI_SAMPLE_CMD1_SHOW_IMAGE;
        cmd.height = SAMPLE_IMAGE2_HEIGHT;
        cmd.width = SAMPLE_IMAGE2_WIDTH;
        cmd.x = SAMPLE_IMAGE_X;
        cmd.y = SAMPLE_IMAGE_Y;
        strlcpy(cmd.img_path, SAMPLE_IMAGE2_PATH,SAMPLE_IMAGE2_PATH_LEN);
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret < 0){
            LOGE_PRINT("   Failed SEC_UI_SAMPLE_CMD1_SHOW_IMAGE: 0 %d",ret);
            abort_secure_ui(auth);
            ret = FAILURE;
            break;
        }
        printf("   Succeeded SEC_UI_SAMPLE_CMD1_SHOW_IMAGE for the second image\n"
                "press enter to show image 1 ... \n");
        getchar();

        //step 3
        cmd.cmd_id = SEC_UI_SAMPLE_CMD1_SHOW_IMAGE;
        cmd.height = SAMPLE_IMAGE1_HEIGHT;
        cmd.width = SAMPLE_IMAGE1_WIDTH;
        cmd.x = SAMPLE_IMAGE_X;
        cmd.y = SAMPLE_IMAGE_Y;
        strlcpy(cmd.img_path, SAMPLE_IMAGE1_PATH,SAMPLE_IMAGE1_PATH_LEN);
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret < 0){
            LOGE_PRINT("   Failed SEC_UI_SAMPLE_CMD1_SHOW_IMAGE: 0 %d",ret);
            abort_secure_ui(auth);
            ret = FAILURE;
            break;
        }
        printf("   Succeeded SEC_UI_SAMPLE_CMD1_SHOW_IMAGE for the first image\n"
                "press enter to continue ... \n");
        getchar();

        if (true == auth)
            StopAuthenticateSecureDisplay();
        //step 4
        cmd.cmd_id = SEC_UI_SAMPLE_CMD2_STOP_DISP;
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret < 0){
            LOGE_PRINT("   Failed SEC_UI_SAMPLE_CMD2_STOP_DISP: %d",ret);
        } else {
            LOGD_PRINT("   Succeeded SEC_UI_SAMPLE_CMD2_STOP_DISP");
        }

#if defined(DISPLAY_CONFIG_1_3)
        LOGD_PRINT("ENABLE MDP Idle Power Collapse");
        ret = intf->controlIdlePowerCollapse(1, false);
        if (ret) {
            LOGE_PRINT("IDisplayConfig::controlIdlePowerCollapse failed to "
                    "ENABLE MDP Idle PC=[%d] ", ret);
            return FAILURE;
        }
#endif

    } while (0);

    status = qsc_shutdown_app(&l_QSEEComHandle);
    if (status) {
        LOGE_PRINT("   Failed to shutdown app: %d",ret);
        return FAILURE;
    }
    LOGD_PRINT("shutdown: pass\n");
    return ret;
}

int touch_basic_test(uint32_t test_iterations, uint32_t sleeping_time){
    struct QSEECom_handle *l_QSEEComHandle = NULL;
    struct send_cmd cmd = {0};
    int32_t ret;
    uint32_t rv, i;
    pthread_t abort_ui_thread = 0;
    uint8_t failed = 0;
    uint32_t lib_mem_size;
    uint32_t max_mem_size;

    ret = GetSharedMemorySize(&lib_mem_size);
    if (ret)
    {
        LOGE_PRINT("GetSharedMemorySize failed");
        return FAILURE;
    }
    max_mem_size = MAX(SEC_UI_SAMPLE_SH_MEM_SIZE, lib_mem_size);
    LOGD_PRINT("Shared buffer size: %d", max_mem_size);

    ret = qsc_start_app(&l_QSEEComHandle, g_app_name, max_mem_size);
    if (ret) {
        LOGE_PRINT("Start app: fail");
        return ret;
    } else {
        LOGD_PRINT("Start app: pass");
    }

    /* we run tests in loop, each iteration is aborted in random time (up to the received parameter) */
    for(i = 0; i< test_iterations ; i++){
        g_run = 1;

        /* setting up the fd for secure touch */
        g_efd = eventfd(0, 0);
        if (g_efd == -1) {
            LOGE_PRINT("Failed to create eventfd: %s", strerror(errno));
            failed = 1;
            break;
        }

        /* starting the abort thread */
        ret = pthread_create( &abort_ui_thread, NULL, abort_ui, &sleeping_time);
        if ( ret )
        {
            LOGE("Error: Creating abort_ui_thread thread failed!");
            failed = 1;
            break;
        }

        /* setting up the secure display and the touch callback to draw a dot*/
        cmd.cmd_id = SEC_UI_SAMPLE_CMD0_START_SEC_UI;
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret != 0){
            LOGE_PRINT("   Failed SEC_UI_SAMPLE_CMD0_START_SEC_UI: %d",ret);
            failed = 1;
            break;
        }
        LOGD_PRINT("   Succeeded SEC_UI_SAMPLE_CMD0_START_SEC_UI (%d)",ret);

        /* run the touch session with the registered callback */
        LOGD_PRINT("Starting secure touch test");
        rv = UseSecureTouch(g_efd, l_QSEEComHandle);
        if(-ECONNABORTED == rv){
            LOGD_PRINT("touch test succeeded!");
        } else {
            LOGE_PRINT("touch test aborted!, (%d)",rv);
            abort_secure_ui(false);
            break;
        }

        cmd.cmd_id = SEC_UI_SAMPLE_CMD2_STOP_DISP;
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret < 0){
            LOGE_PRINT("   Failed SEC_UI_SAMPLE_CMD2_STOP_DISP: %d",ret);
        } else {
            LOGD_PRINT("   Succeeded SEC_UI_SAMPLE_CMD2_STOP_DISP");
        }

        g_run = 0;
        if  (pthread_join(abort_ui_thread, NULL) == FAILURE){
            LOGE_PRINT("Error: joining abort ui thread failed!");
            failed = 1;
            break;
        } else {
            LOGD_PRINT("Joined abort ui thread!, finished iteration %d",i);
        }
        if (g_efd != -1) {
            close(g_efd);
            g_efd = -1;
        }
    }

    if (g_efd != -1) {
        close(g_efd);
        g_efd = -1;
    }

    ret = qsc_shutdown_app(&l_QSEEComHandle);
    if (ret) {
        LOGE_PRINT("   Failed to shutdown app: %d",ret);
        failed = 1;
    }
    LOGD_PRINT("shutdown: pass\n");

    if(!failed){
        LOGD_PRINT("Succeeded %d touch test iterations",test_iterations);
    } else {
        LOGE_PRINT("failed touch test after %d iterations",i);
    }

    return failed;
}

int app_buffer_basic_test(){
    struct send_cmd cmd = {0};
    int32_t ret, status;
    struct QSEECom_handle *l_QSEEComHandle = NULL;
    struct stat st;
    uint32_t lib_mem_size;
    uint32_t max_mem_size;

    if(stat(SAMPLE_IMAGE1_PATH, &st) != 0 || stat(SAMPLE_IMAGE2_PATH, &st) != 0) {
        LOGE_PRINT("secure display basic test: test images are missing, see securemsm/sse/tests/SecureUI/images/README.txt for instructions");
        return FAILURE;
    }

    ret = GetSharedMemorySize(&lib_mem_size);
    if (ret)
    {
        LOGE_PRINT("GetSharedMemorySize failed");
        return FAILURE;
    }
    max_mem_size = MAX(SEC_UI_SAMPLE_SH_MEM_SIZE, lib_mem_size);
    LOGD_PRINT("Shared buffer size: %d", max_mem_size);

    ret = qsc_start_app(&l_QSEEComHandle, g_app_name, max_mem_size);
    if (ret) {
        LOGE_PRINT("Start app: fail");
        return FAILURE;
    } else {
        LOGD_PRINT("Start app: pass");
    }
    do{
        cmd.cmd_id = SEC_UI_SAMPLE_CMD4_TEST_APP_BUFFER;
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret != 0){
            LOGE_PRINT("   Failed SEC_UI_SAMPLE_CMD4_TEST_APP_BUFFER %d",ret);
            ret = FAILURE;
            break;
        }
        LOGD_PRINT("   Succeeded SEC_UI_SAMPLE_CMD4_TEST_APP_BUFFER (%d)",ret);
    } while (0);

    status = qsc_shutdown_app(&l_QSEEComHandle);
    if (status) {
        LOGE_PRINT("   Failed to shutdown app: %d",ret);
        return FAILURE;
    }
    LOGD_PRINT("shutdown: pass\n");
    return ret;
}

int start_secure_ui_demo(){
    struct QSEECom_handle *l_QSEEComHandle = NULL;
    struct send_cmd cmd = {0};
    int32_t ret;
    uint32_t rv;
    struct stat st;
    uint32_t lib_mem_size;
    uint32_t max_mem_size;

    if(stat(SEC_UI_DEMO_LOGO_PATH, &st) != 0){
        LOGE_PRINT("secure ui demo: logo image is missing");
        LOGD_PRINT("secure ui demo: logo image should be a PNG located at: %s", SEC_UI_DEMO_LOGO_PATH);
        return FAILURE;
    }

    if(stat(SEC_UI_DEMO_IND_PATH, &st) != 0){
        LOGD_PRINT("secure ui demo: indicator image is missing, will try to display the secure indicator instead");
        LOGD_PRINT("secure ui demo: indicator image should be a PNG located at: %s", SEC_UI_DEMO_IND_PATH);
    }


    /* setting up the fd for secure touch */
    g_efd = eventfd(0, 0);
    if (g_efd == -1) {
        LOGE_PRINT("Failed to create eventfd: %s", strerror(errno));
        return errno;
    }

    ret = GetSharedMemorySize(&lib_mem_size);
    if (ret)
    {
        LOGE_PRINT("GetSharedMemorySize failed");
        return FAILURE;
    }
    max_mem_size = MAX(SEC_UI_SAMPLE_SH_MEM_SIZE, lib_mem_size);
    LOGD_PRINT("Shared buffer size: %d", max_mem_size);

    ret = qsc_start_app(&l_QSEEComHandle, g_app_name, max_mem_size);
    if (ret) {
        LOGE_PRINT("Start app: fail");
        return ret;
    } else {
        LOGD_PRINT("Start app: pass");
    }

    rv = GetSecureIndicator(l_QSEEComHandle);
    if(0 == rv){
        LOGD_PRINT("GetSecureIndicator succeeded");
    } else {
        LOGE_PRINT("GetSecureIndicator failed, (%d). Continuing anyway",rv);
    }

    /* we run over the GP screens in infinite loop, only the user can stop us */
    while(1){

        /* get pin screen */
        cmd.cmd_id = SEC_UI_SAMPLE_CMD10_GET_PIN;
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret < 0){
            LOGD_PRINT("   SEC_UI_SAMPLE_CMD10_GET_PIN returned (%d)",ret);
            break;
        }

        /* run the touch session with the registered callback */
        rv = UseSecureTouch(g_efd, l_QSEEComHandle);
        if(0 == rv){
            LOGD_PRINT("get pin succeeded!");
        } else {
            LOGD_PRINT("get pin aborted!, (%d)",rv);
            break;
        }

        /* message screen for showing the received pin */
        cmd.cmd_id = SEC_UI_SAMPLE_CMD12_MSG_PIN;
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret < 0){
            LOGD_PRINT("   SEC_UI_SAMPLE_CMD12_MSG_PIN returned (%d)",ret);
            break;
        }
        /* run the touch session with the registered callback */
        rv = UseSecureTouch(g_efd, l_QSEEComHandle);
        if(0 == rv){
            LOGD_PRINT("get pin msg succeeded!");
        } else {
            LOGD_PRINT("get pin msg aborted!, (%d)",rv);
            break;
        }

        /* login screen */
        cmd.cmd_id = SEC_UI_SAMPLE_CMD11_LOGIN;
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret < 0){
            LOGD_PRINT("   SEC_UI_SAMPLE_CMD11_LOGIN returned (%d)",ret);
            break;
        }
        /* run the touch session with the registered callback */
        rv = UseSecureTouch(g_efd, l_QSEEComHandle);
        if(0 == rv){
            LOGD_PRINT("login succeeded!");
        } else {
            LOGD_PRINT("login aborted!, (%d)",rv);
            break;
        }
        /* message screen for showing the received username & password */
        cmd.cmd_id = SEC_UI_SAMPLE_CMD13_MSG_LOGIN;
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret < 0){
            LOGD_PRINT("   SEC_UI_SAMPLE_CMD13_MSG_LOGIN returned (%d)",ret);
            break;
        }
        /* run the touch session with the registered callback */
        rv = UseSecureTouch(g_efd, l_QSEEComHandle);
        if(0 == rv){
            LOGD_PRINT("login msg succeeded!");
        } else {
            LOGD_PRINT("login msg aborted!, (%d)",rv);
            break;
        }

        /* get custom credit card screen */
        cmd.cmd_id = SEC_UI_SAMPLE_CMD15_CUSTOM_DIALOG;
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret < 0){
            LOGD_PRINT("   SEC_UI_SAMPLE_CMD15_CUSTOM_DIALOG returned (%d)",ret);
            break;
        }
        /* run the touch session with the registered callback */
        rv = UseSecureTouch(g_efd, l_QSEEComHandle);
        if(0 == rv){
            LOGD_PRINT("get custom dialog succeeded!");
        } else {
            LOGD_PRINT("get custom dialog aborted!, (%d)",rv);
            break;
        }
        /* message screen for showing the received credit card info */
        cmd.cmd_id = SEC_UI_SAMPLE_CMD16_MSG_CUSTOM;
        ret = issue_send_cmd(l_QSEEComHandle,&cmd);
        if(ret < 0){
            LOGD_PRINT("   SEC_UI_SAMPLE_CMD16_MSG_CUSTOM returned (%d)",ret);
            break;
        }
        /* run the touch session with the registered callback */
        rv = UseSecureTouch(g_efd, l_QSEEComHandle);
        if(0 == rv){
            LOGD_PRINT("get custom msg succeeded!");
        } else {
            LOGD_PRINT("get custom msg aborted!, (%d)",rv);
            break;
        }

        /* clean up */
        cmd.cmd_id = SEC_UI_SAMPLE_CMD14_MSG_CLEAN_UP;
        issue_send_cmd(l_QSEEComHandle,&cmd);

        sleep(1);
    }

    /* clean up */
    cmd.cmd_id = SEC_UI_SAMPLE_CMD14_MSG_CLEAN_UP;
    issue_send_cmd(l_QSEEComHandle,&cmd);

    if (g_efd != -1) close(g_efd);

    ret = qsc_shutdown_app(&l_QSEEComHandle);
    if (ret) {
        LOGE_PRINT("   Failed to shutdown app: %d",ret);
        return ret;
    }
    LOGD_PRINT("shutdown: pass\n");

    return 0;
}

int main(int argc, char *argv[])
{
    int32_t i = 0;
    char * help_msg = "Use one of the following options:\n\t"
        "-t for secure touch basic test\n\t"
        "-d for secure display basic test , use -auth to enable authentication\n\t"
        "-a for app buffer basic test\n\t"
        "-u for secure ui screens demo\n\n"
        "Optional: add '32bit' or '64bit' in order to choose the TZ app configuration\n\tDefault is '32bit'\n\n";

    if (argc > 4 || argc < 2) {
        printf("\nError: %s received wrong arguments.\n%s", argv[0], help_msg);
        return 0;
    }

    g_app_name = SEC_UI_SAMPLE_APP_NAME_32BIT;

    for (i = 1; i < argc; i++) {
        if(!strncmp("32bit", argv[i],5)) {
            printf("running with 32 bit secure_ui_sample app\n");
            g_app_name = SEC_UI_SAMPLE_APP_NAME_32BIT;

        } else if(!strncmp("64bit", argv[i],5)){
            printf("running with 64 bit secure_ui_sample app\n");
            g_app_name = SEC_UI_SAMPLE_APP_NAME_64BIT;
        }
    }

    for (i = 1; i < argc; i++) {

        if(!strncmp("-d", argv[i],2)){
            printf("secure display basic test\n");
            if (((i+1) < argc) && !strncmp("-auth", argv[i+1],5)) {
                printf(" Enabling Authentication\n");
                return display_basic_test(true);
            }
            else
                return display_basic_test(false);

        } else if(!strncmp("-t", argv[1],2)){
            printf("secure touch basic test\n");
            return touch_basic_test(TEST_ITERATION, SLEEP_TIME_BEFORE_ABORT);

        } else if(!strncmp("-a", argv[1],2)){
            printf("app buffer basic test\n");
            return app_buffer_basic_test();

        } else if(!strncmp("-u", argv[1],2)){
            printf("secure ui screens demo\n");
            return start_secure_ui_demo();

        } else if(!strncmp("-h", argv[1],2)){
            printf("\n%s", help_msg);
        } else {
            printf("\nError: %s received wrong arguments.\n%s", argv[0], help_msg);
        }
    }


    return 0;
}
