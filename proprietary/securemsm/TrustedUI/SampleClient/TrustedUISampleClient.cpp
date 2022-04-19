/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <android-base/logging.h>
#include <cutils/properties.h>
#include <errno.h>
#include <gtest/gtest.h>
#include <hidl/HidlSupport.h>
#include <hidl/Status.h>
#include <poll.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <stringl.h>
#include <sys/stat.h>
#include <utils/Log.h>
#include <vendor/qti/hardware/trustedui/1.0/ITrustedUI.h>
#include <thread>
#include "TrustedUICallback.h"
#include "common_log.h"

using vendor::qti::hardware::trustedui::V1_0::ITrustedUI;
using vendor::qti::hardware::trustedui::V1_0::ITrustedUICallback;
using vendor::qti::hardware::trustedui::V1_0::ITrustedInput;
using vendor::qti::hardware::trustedui::V1_0::implementation::TrustedUICallback;
using ::vendor::qti::hardware::trustedui::V1_0::Response;
using ::vendor::qti::hardware::trustedui::V1_0::TUICreateParams;
using ::vendor::qti::hardware::trustedui::V1_0::TUIConfig;
using ::android::hardware::hidl_death_recipient;
using ::android::hidl::base::V1_0::IBase;

#if defined(DISPLAY_CONFIG_1_3)
#include <vendor/display/config/1.3/IDisplayConfig.h>
using vendor::display::config::V1_3::IDisplayConfig;
#endif  // DISPLAY_CONFIG_1_3

GTEST_DEFINE_bool_(enable_frame_auth, false,
                   "Enable periodic (every vsync) frame auth");
GTEST_DEFINE_int32_(logLevel, 0, "Set logging level");
using namespace android;
using android::sp;
using android::hardware::Return;
using android::hardware::Void;

/** adb log */
#undef LOG_TAG
#define LOG_TAG "TUI_TEST_APP:"

#define LOGD_PRINT(...)      \
    do {                     \
        LOGD(__VA_ARGS__);   \
        printf(__VA_ARGS__); \
        printf("\n");        \
    } while (0)
//#define LOGD_PRINT(...) do {  } while(0)

#define LOGE_PRINT(...)      \
    do {                     \
        LOGE(__VA_ARGS__);   \
        printf(__VA_ARGS__); \
        printf("\n");        \
    } while (0)

#define FRAME_AUTH_START \
    if (GTEST_FLAG(enable_frame_auth)) startAuthenticateSecureDisplay();

#define FRAME_AUTH_STOP \
    if (GTEST_FLAG(enable_frame_auth)) stopAuthenticateSecureDisplay();

#define TUI_DEMO_LOGO_PATH "/data/vendor/tui/logo.png"
#define TUI_DEMO_IND_PATH "/data/vendor/tui/sec_ind.png"
#define TUI_SAMPLE_APP_NAME_32BIT "tuiapp32"
#define TUI_SAMPLE_APP_NAME_64BIT "tuiapp"

// custom commands are 16bits
#define TUI_CMD_AUTHENTICATE_FRAME 5

bool mInputHandlingError = false;
sem_t mSessionComplete;

// TODO: Implement app_buffer_basic_test
static constexpr char *config_app_name = TUI_SAMPLE_APP_NAME_64BIT;

class TrustedUIAppTest : public ::testing::Test
{
   public:
    virtual ~TrustedUIAppTest() = default;
    sp<ITrustedUI> mTUIIntf = nullptr;
    sp<ITrustedUICallback> mSessionCallback = nullptr;
    uint32_t mSessionId = -1;

   protected:
    virtual void SetUp();
    virtual void TearDown();
    virtual int32_t checkPrerequisites();
    void *postVSyncLoop();
    int32_t startAuthenticateSecureDisplay();
    int32_t stopAuthenticateSecureDisplay();
    void setLogLevel(uint32_t level);

   private:
    bool mSetUpOk = false;
    bool gVSyncThreadDone = false;
    std::string mTrustedEE;
    sp<ITrustedInput> mTrustedInputDevice = nullptr;
    sp<IDisplayConfig> mDisplayConfigIntf = nullptr;
    std::shared_ptr<std::thread> mAuthThread = nullptr;
};

// TrustedUICallback methods
Return<void> TrustedUICallback::onComplete()
{
    LOGD_PRINT("%s: TrustedUICallback:: onComplete Notification", __func__);
    sem_post(&mSessionComplete);
    return Void();
}

Return<void> TrustedUICallback::onError()
{
    LOGD_PRINT("%s: onError Notification", __func__);
    mInputHandlingError = true;
    sem_post(&mSessionComplete);
    return Void();
}

// TrustedUIAppTest methods
void TrustedUIAppTest::SetUp()
{
    int32_t dpyIdx = 0;

    LOGD("%s: ITrustedUI::getService ", __func__);
    mTUIIntf = ITrustedUI::getService();
    ASSERT_TRUE(mTUIIntf != nullptr);

    LOGD("%s: ITrustedInput::getService ", __func__);
    mTrustedInputDevice = ITrustedInput::getService();
    ASSERT_TRUE(mTrustedInputDevice != nullptr);

    LOGD("%s: TrustedUICallback ", __func__);
    mSessionCallback = new TrustedUICallback();
    ASSERT_TRUE(mSessionCallback != nullptr);

#if defined(DISPLAY_CONFIG_1_3)
    mDisplayConfigIntf = IDisplayConfig::getService();
    ASSERT_TRUE(mDisplayConfigIntf != nullptr);

    LOGD_PRINT("%s; DISABLE MDP Idle Power Collapse for Trusted UI Session",
               __func__);
    int32_t ret = mDisplayConfigIntf->controlIdlePowerCollapse(0, false);
    if (ret) {
        LOGE_PRINT(
            "%s: IDisplayConfig::controlIdlePowerCollapse failed to "
            "DISABLE MDP Idle PC=[%d] ",
            __func__, ret);
        LOGE_PRINT("%s: Make sure that the display is ON", __func__);
    }
    ASSERT_TRUE(ret == 0);
#endif

    LOGD_PRINT(
        "TrustedUIAppTest:: SetUp SUCCESS in connecting to TrustedUI HAL");
    setLogLevel(GTEST_FLAG(logLevel));
    mTrustedEE = config_app_name;
    TUICreateParams inParams = {config_app_name, dpyIdx, 0};
    TUICreateParams outParams;
    Response res;
    mTUIIntf->createSession(inParams, mTrustedInputDevice, mSessionCallback,
                            [&](Response _res, uint32_t _sessionId) {
                                res = _res;
                                if (res == Response::TUI_SUCCESS) {
                                    mSessionId = _sessionId;
                                }
                            });
    ASSERT_EQ(res, Response::TUI_SUCCESS);
    LOGD_PRINT("TrustedUIAppTest:: SetUp sharedMemSize: %d ",
               outParams.minSharedMemSize);
    LOGD_PRINT("TrustedUIAppTest:: SetUp  HAL State is now : LOADED ");
    sem_init(&mSessionComplete, 0, 0);
    mSetUpOk = true;
}

void TrustedUIAppTest::TearDown()
{
    if (mSetUpOk == false) {
        LOGE_PRINT("TrustedUIAppTest:: Teardown Nothing to be done");
        return;
    }

    /* Call TUI Stop in case the test is tearing down due to error
     * and did not get to Stop the TUI Session. If not called, it will
     * result in failure of all subsequent tests */
    LOGD_PRINT("TrustedUIAppTest:: Teardown Issuing STOP to TUI HAL");
    mTUIIntf->stopSession(mSessionId);

    Response ret = mTUIIntf->deleteSession(mSessionId);
    if (ret == Response::TUI_FAILURE) {
        LOGE_PRINT("TrustedUIAppTest::TearDown failed to delete the Session");
    }
    sem_destroy(&mSessionComplete);
    LOGD_PRINT("TrustedUIAppTest::TearDown complete!");
    mSetUpOk = false;

#if defined(DISPLAY_CONFIG_1_3)
    LOGD_PRINT("%s: ENABLE MDP Idle Power Collapse", __func__);
    int32_t res = mDisplayConfigIntf->controlIdlePowerCollapse(1, false);
    if (res) {
        LOGE_PRINT(
            "%s: IDisplayConfig::controlIdlePowerCollapse failed to "
            "ENABLE MDP Idle PC=[%d] ",
            __func__, res);
    }
    ASSERT_EQ(res, 0);
#endif
}

int32_t TrustedUIAppTest::checkPrerequisites()
{
    struct stat st;
    if (stat(TUI_DEMO_LOGO_PATH, &st) != 0) {
        LOGE_PRINT("TUI Sample Client: logo image is missing");
        LOGD_PRINT(
            "TUI Sample Client: logo image should be a PNG located at: %s",
            TUI_DEMO_LOGO_PATH);
        return -1;
    }

    if (stat(TUI_DEMO_IND_PATH, &st) != 0) {
        LOGE_PRINT(
            "TUI Sample Client: indicator image is missing, will try to "
            "display the secure indicator instead");
        LOGD_PRINT(
            "TUI Sample Client: indicator image should be a PNG located at: %s",
            TUI_DEMO_IND_PATH);
    }
    return 0;
}

void TrustedUIAppTest::setLogLevel(uint32_t debugLevel)
{
    char *level = NULL;
    switch(debugLevel) {
        case 0x1:
            level = "INFO";
            break;
        case 0x2:
            level = "DEBUG";
            break;
        case 0x3:
            level = "ALL";
            break;
        default:
            level = "ERROR";
    }
    if (0 == property_set("vendor.tui_debug_level", level)) {
        LOGD_PRINT("%s: set logging level:%s", __func__, level);
    }
}

void *TrustedUIAppTest::postVSyncLoop()
{
    int32_t fds[2] = {-1};
    Response ret;

    if (pipe(fds) == -1) {
        ALOGE("%s: pipe() failed !", __func__);
        return NULL;
    }
    struct pollfd poll_fd = {
        .fd = fds[0], .events = POLLIN,
    };

    while (!gVSyncThreadDone) {
        /* poll timeout time is kept random between 16 and 32 ms , so
         * as to make the "time of CRC check in TZ" random   and a max of 32 ms
         * as we want to check CRC every Vsync */
        int random_poll_timeout = 16 + (rand() % 16);
        int status = poll(&poll_fd, 1, random_poll_timeout /*ms*/);
        if (status == 0) { /*TIMEOUT*/
            ALOGD("%s: Sending cmd TUI_CMD_AUTHENTICATE_FRAME", __func__);
            mTUIIntf->sendCommand(
                mSessionId, TUI_CMD_AUTHENTICATE_FRAME, NULL,
                [&](Response _res, std::vector<uint8_t> _responseData) {
                    ret = _res;
                });
            if (ret != Response::TUI_SUCCESS) {
                ALOGE("%s: send command failed with ret : %d", __func__, ret);
                break;
            }
        } else {
            ALOGE("%s: poll failed !", __func__);
            break;
        }
    }

    close(fds[0]);
    close(fds[1]);
    return NULL;
}

int32_t TrustedUIAppTest::startAuthenticateSecureDisplay()
{
    int32_t ret = 0;

    /* Create the main thread of sending Secure UI authenticate cmd requests in
     * a while loop */
    mAuthThread =
        std::make_shared<std::thread>([&]() { this->postVSyncLoop(); });
    if (!mAuthThread->joinable()) {
        ALOGE(
            "Error: Creating thread for vsync-trigger in secure UI client "
            "failed!");
        ret = -1;
        goto end;
    }
end:
    return ret;
}

int32_t TrustedUIAppTest::stopAuthenticateSecureDisplay()
{
    int32_t ret = 0;

    if (gVSyncThreadDone || !mAuthThread->joinable()) {
        ALOGE(
            "Error: %s called while thread not joinable, gVSyncThreadDone:%d "
            "joinable:%d",
            gVSyncThreadDone, mAuthThread->joinable());
        ret = -1;
        goto end;
    }
    gVSyncThreadDone = true;
    mAuthThread->join();
end:
    return ret;
}

// TODO: Address this in current layout instead of doing it for next layout
#define CHECK_FOR_TEST_COMPLETION(ret)                         \
    if ((ret) > 0) {                                           \
        ret = 0;                                               \
        LOGD_PRINT("\nUser Pressed Cancel. Test Completed\n"); \
        goto end;                                              \
    }

TEST_F(TrustedUIAppTest, TestTUILayouts)
{
    ASSERT_TRUE(mTUIIntf != nullptr);
    Response ret;
    int32_t rv = 0;
    mInputHandlingError = 0;
    ASSERT_EQ(checkPrerequisites(), 0);

    TUIConfig cfg;
    while (1) {
        LOGD_PRINT("TrustedUIAppTest:: Start PIN Screen..");

        /* get pin screen */
        cfg = {0 /*useSecureIndicator*/, "pin", false /*enableFrameAuth*/};
        ASSERT_GE(ret = mTUIIntf->startSession(mSessionId, cfg),
                  Response::TUI_SUCCESS);
        rv = (int)ret;
        CHECK_FOR_TEST_COMPLETION(rv);
        LOGD_PRINT(
            "TrustedUIAppTest:: Launched the layout. Start user interaction "
            "..");
        LOGD_PRINT("TrustedUIAppTest:: waiting for session to complete ...zz");

        FRAME_AUTH_START;
        sem_wait(&mSessionComplete);
        LOGD_PRINT("TrustedUIAppTest:: Session completed.");
        FRAME_AUTH_STOP;

        LOGD_PRINT("TrustedUIAppTest:: Sending cmd STOP to TUI HAL");
        ASSERT_EQ(mTUIIntf->stopSession(mSessionId), Response::TUI_SUCCESS);

        if (mInputHandlingError) break;

        /* message screen for showing the received pin */
        LOGD_PRINT("TrustedUIAppTest:: Start MSG_PIN Screen..");
        cfg = {0, "msg_pin", false /*enableFrameAuth*/};
        ASSERT_GE(ret = mTUIIntf->startSession(mSessionId, cfg),
                  Response::TUI_SUCCESS);
        rv = (int)ret;
        CHECK_FOR_TEST_COMPLETION(rv);

        LOGD_PRINT(
            "TrustedUIAppTest:: Launched the layout. Start user interaction "
            "..");
        LOGD_PRINT("TrustedUIAppTest:: waiting for session to complete ...zz");

        FRAME_AUTH_START;
        sem_wait(&mSessionComplete);
        LOGD_PRINT("TrustedUIAppTest:: Session completed.");
        FRAME_AUTH_STOP;

        LOGD_PRINT("TrustedUIAppTest:: Sending cmd STOP to TUI HAL");
        ASSERT_EQ(mTUIIntf->stopSession(mSessionId), Response::TUI_SUCCESS);

        if (mInputHandlingError) break;

        /* login screen */
        LOGD_PRINT("TrustedUIAppTest:: Start LOGIN Screen..");
        cfg = {0, "login", false /*enableFrameAuth*/};
        ASSERT_GE(ret = mTUIIntf->startSession(mSessionId, cfg),
                  Response::TUI_SUCCESS);
        rv = (int)ret;
        CHECK_FOR_TEST_COMPLETION(rv);

        LOGD_PRINT(
            "TrustedUIAppTest:: Launched the layout. Start user interaction "
            "..");
        LOGD_PRINT("TrustedUIAppTest:: waiting for session to complete ...zz");

        FRAME_AUTH_START;
        sem_wait(&mSessionComplete);
        LOGD_PRINT("TrustedUIAppTest:: Session completed.");
        FRAME_AUTH_STOP;

        LOGD_PRINT("TrustedUIAppTest:: Sending cmd STOP to TUI HAL");

        ASSERT_EQ(mTUIIntf->stopSession(mSessionId), Response::TUI_SUCCESS);

        if (mInputHandlingError) break;

        /* message screen for showing the received username & password */
        LOGD_PRINT("TrustedUIAppTest:: Start MSG_LOGIN Screen..");
        cfg = {0, "msg_login", false /*enableFrameAuth*/};
        ASSERT_GE(ret = mTUIIntf->startSession(mSessionId, cfg),
                  Response::TUI_SUCCESS);
        rv = (int)ret;
        CHECK_FOR_TEST_COMPLETION(rv);

        LOGD_PRINT(
            "TrustedUIAppTest:: Launched the layout. Start user interaction "
            "..");
        LOGD_PRINT("TrustedUIAppTest:: waiting for session to complete ...zz");

        FRAME_AUTH_START;
        sem_wait(&mSessionComplete);
        LOGD_PRINT("TrustedUIAppTest:: Session completed.");
        FRAME_AUTH_STOP;

        LOGD_PRINT("TrustedUIAppTest:: Sending cmd STOP to TUI HAL");
        ASSERT_EQ(mTUIIntf->stopSession(mSessionId), Response::TUI_SUCCESS);
    }
end:
    return;
}

TEST_F(TrustedUIAppTest, Basic_SecureTouch)
{
    LOGD_PRINT("TrustedUIAppTest:: Basic_SecureTouch");
    ASSERT_TRUE(mTUIIntf != nullptr);

    Response ret;
    TUIConfig cfg = {0 /*useSecureIndicator*/, "draw_dot",
                     false /*enableFrameAuth*/};

    LOGD_PRINT("TrustedUIAppTest::Sending cmd START to TUI HAL");
    ASSERT_EQ(ret = mTUIIntf->startSession(mSessionId, cfg),
              Response::TUI_SUCCESS);

    LOGD_PRINT(
        "TrustedUIAppTest:: Launched the layout. Start user interaction ..");
    LOGD_PRINT("TrustedUIAppTest:: waiting for session to complete ...zz");

    FRAME_AUTH_START;
    sem_wait(&mSessionComplete);
    LOGD_PRINT("TrustedUIAppTest:: Session completed.");
    FRAME_AUTH_STOP;

    LOGD_PRINT("TrustedUIAppTest::Sending cmd STOP to TUI HAL");
    ASSERT_EQ(mTUIIntf->stopSession(mSessionId), Response::TUI_SUCCESS);
}

// NOTE: display-only tests are currently not supported
TEST_F(TrustedUIAppTest, Basic_SecureDisplayStartStop)
{
    LOGD_PRINT("TrustedUIAppTest::Basic_SecureDisplayStartStop");
    ASSERT_TRUE(mTUIIntf != nullptr);

    TUIConfig cfg = {0 /*useSecureIndicator*/, "no_layout",
                     false /*enableFrameAuth*/};
    Response ret;

    LOGD_PRINT("TrustedUIAppTest::Sending cmd startSession to TUI HAL");
    ASSERT_EQ(ret = mTUIIntf->startSession(mSessionId, cfg),
              Response::TUI_SUCCESS);

    sem_wait(&mSessionComplete);
    LOGD_PRINT("TrustedUIAppTest:: Session completed.");

    LOGD_PRINT("TrustedUIAppTest::Sending cmd STOP to TUI HAL");
    ASSERT_EQ(mTUIIntf->stopSession(mSessionId), Response::TUI_SUCCESS);
}

TEST_F(TrustedUIAppTest, NegativeTest_DeleteSessionWithoutStopSession)
{
    LOGD_PRINT(
        "TrustedUIAppTest::NegativeTest_DeleteSessionWithoutStopSession");
    ASSERT_TRUE(mTUIIntf != nullptr);

    TUIConfig cfg = {0 /*useSecureIndicator*/, "pin",
                     false /*enableFrameAuth*/};
    Response ret;

    LOGD_PRINT("TrustedUIAppTest::Sending cmd startSession to TUI HAL");
    ASSERT_EQ(ret = mTUIIntf->startSession(mSessionId, cfg),
              Response::TUI_SUCCESS);

    LOGD_PRINT(
        "TrustedUIAppTest:: Launched the layout. Start user interaction ..");
    LOGD_PRINT("TrustedUIAppTest:: waiting for session to complete ...zz");

    sem_wait(&mSessionComplete);
    LOGD_PRINT("TrustedUIAppTest:: Session completed.");

    LOGD_PRINT("TrustedUIAppTest:: deleteSession ..");
    ASSERT_EQ(ret = mTUIIntf->deleteSession(mSessionId), Response::TUI_FAILURE);
    LOGD_PRINT("TrustedUIAppTest::deleteSession failed as expected !");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    if (argc == 1) goto run;

    for (int i = 1; i < argc; i++) {
        // Enable frame authentication
        if ((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--frameauth") == 0)) {
            GTEST_FLAG(enable_frame_auth) = true;
            continue;
        } // Enable Medium level logging
        else if ((strcmp(argv[i], "-d1") == 0) || (strcmp(argv[i], "--debug=medium") == 0)) {
            GTEST_FLAG(logLevel) = 1;
            continue;
        // Enable Low level logging
        } else if ((strcmp(argv[i], "-d2") == 0) || (strcmp(argv[i], "--debug=low") == 0)) {
            GTEST_FLAG(logLevel) = 2;
            continue;
        // Enable all logs
        } else if ((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--debug=all") == 0)) {
            GTEST_FLAG(logLevel) = 3;
            continue;
        } else {
            LOGD_PRINT("Use: TrustedUISampleTest --gtest_filter=*[test]* [-opts]");
            LOGD_PRINT("options:");
            LOGD_PRINT("--gtest_filter=*{test_name}* \t\t Run specific tests (TestTUILayouts, SecureTouch).");
            LOGD_PRINT("-f \t\t\t\t Enable frame authentication");
            LOGD_PRINT("-d1 \t\t\t\t Enable INFO logging. Error is enabled by default");
            LOGD_PRINT("-d2 \t\t\t\t Enable DEBUG logging. Error is enabled by default");
            LOGD_PRINT("-d \t\t\t\t Enable ALL logging. Error is enabled by default");
            return 0;
        }
    }
run:
    return RUN_ALL_TESTS();
}
