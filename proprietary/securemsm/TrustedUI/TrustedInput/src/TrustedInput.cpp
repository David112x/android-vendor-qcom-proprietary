/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <StDefs.h>
#include <TrustedInput.h>
#include <common_log.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <utils/Log.h>
#include <thread>

#undef LOG_TAG
#define LOG_TAG "ITrustedInput"

namespace vendor {
namespace qti {
namespace hardware {
namespace trustedui {
namespace V1_0 {
namespace implementation {

using ::vendor::qti::hardware::trustedui::V1_0::Response;
using namespace std;

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

enum poll_fds {
    IRQ_EVENT,
    ABORT_EVENT,
    MAX_EVENTS,
};

static int32_t touchSearchRecursive(const string &path, string &control,
                                    string &irq)
{
    DIR *dirp = NULL;
    struct dirent *dirInfo = NULL;
    string devicePath;
    int ret = -ENOENT;

    TUI_CHECK_ERR(!path.empty(), -EINVAL);

    LOGV("Searching %s", path.c_str());
    dirp = opendir(&path[0]);
    TUI_CHECK_ERR(dirp != nullptr, -EINVAL);

    devicePath = path + '/';
    while ((dirInfo = readdir(dirp)) != NULL) {
        if ((!strncmp(dirInfo->d_name, ".", 10)) ||
            (!strncmp(dirInfo->d_name, "..", 10)))
            continue;
        if ((dirInfo->d_type == DT_REG) &&
            (strncmp(dirInfo->d_name, "secure_touch",
                     strlen("secure_touch") + 1) == 0)) {
            irq += devicePath + dirInfo->d_name;
            control = irq + "_enable";
            LOGD("Secure touch found @ %s: %s - %s", devicePath.c_str(),
                 control.c_str(), irq.c_str());
            ret = 0;
        } else if (dirInfo->d_type == DT_DIR) {
            string newPath = devicePath + dirInfo->d_name;
            ret = touchSearchRecursive(&newPath[0], control, irq);
            if (ret == 0) break;
        }
    }
    closedir(dirp);

end:
    return ret;
}

static int32_t touchSearchDevice(string &controlFile, string &irqFile)
{
    int32_t rv = -ENOENT;
    int32_t i;

    for (i = 0; i < ARRAY_LENGTH(locations); i++) {
        LOGD("Searching %s", locations[i].c_str());
        rv = touchSearchRecursive(locations[i], controlFile, irqFile);
        if (rv == 0) break;
    }

    return rv;
}

static void getControllerId(char *str, int32_t &mTouchControllerId)
{
    char *token, *stringPtr;

    token = strtok_r(str, "-", &stringPtr);
    if (token != NULL) token = strtok_r(NULL, "-", &stringPtr);
    if (token != NULL) token = strtok_r(NULL, "/", &stringPtr);
    if (token != NULL) mTouchControllerId = strtol(token, NULL, 16);
}

static int32_t stWaitForEvent(int32_t abortFd, int32_t irqFd, int32_t timeout)
{
    int32_t ret = -1;
    ssize_t readBytes = 0;
    char c;
    uint64_t w;
    struct pollfd fds[MAX_EVENTS]; /* Used for poll() */
    bool pollAgain = 0;

    /* IRQ FD, for interrupt event */
    fds[IRQ_EVENT].fd = irqFd;
    fds[IRQ_EVENT].events = POLLERR | POLLPRI;
#ifdef ST_TARGET_TS_LOOPBACK
    fds[IRQ_EVENT].events = POLLERR | POLLPRI | POLLIN;
    pollAgain = true;
#endif
    /* FD for abort requests */
    fds[ABORT_EVENT].fd = abortFd;
    fds[ABORT_EVENT].events = POLLIN;

    do {
        ret = poll(fds, MAX_EVENTS, timeout);
        /* Error, return error condition */
        TUI_CHECK_ERR(ret >= 0, -errno);
        /* timeout */
        TUI_CHECK_ERR(ret > 0 && timeout != 0, -ETIMEDOUT);
        /* Check for external abort */
        if (fds[1].revents) {
            LOGD("%s: Read from abortFd to empty it", __func__);
            read(abortFd, &w, sizeof(uint64_t));
            ret = -ECONNABORTED;
            goto end;
        }
        /* Consume data, or error, and return */
        if (fds[0].revents) {
            readBytes = pread(irqFd, &c, 1, 0);
            TUI_CHECK_ERR(readBytes > 0, -errno);
            // interrupt
            if (c == '1') {
                ret = 0;
                goto end;
            }
        }
    } while (pollAgain);
end:
    LOGD("%s --fd[D].eve:%x fd[A].eve:%x ret : %d", __func__, fds[0].revents,
            fds[1].revents, ret);
    return ret;
}

int32_t TrustedInput::_stStartSession(void)
{
    int32_t ret = 0;
    ssize_t writtenBytes = 0;
    string str;

    ret = touchSearchDevice(mControlFile, mIrqFile);
    TUI_CHECK_ERR(ret == 0 && !mControlFile.empty() && !mIrqFile.empty(), ret);

    str = mControlFile;
    getControllerId(&str[0], mTouchControllerId);

    LOGD("%s: Opening irq file: %s", __func__, mIrqFile.c_str());
    mIrqFd = open(mIrqFile.c_str(), O_RDONLY);
    TUI_CHECK_ERR(mIrqFd > 0, -errno);

    LOGD("%s: Opening control file: %s, mTouchControllerId:0x%x", __func__,
            mControlFile.c_str(), mTouchControllerId);
    mControlFd = open(mControlFile.c_str(), O_WRONLY);
    if (mControlFd <= 0) {
        ret = -errno;
        goto error1;
    }

    writtenBytes = pwrite(mControlFd, "1", 1, 0);
    if (writtenBytes <= 0) {
        ret = -errno;
        goto error0;
    }
    goto end;

error0:
    close(mControlFd);
    mControlFd = -1;
error1:
    close(mIrqFd);
    mIrqFd = -1;
    mIrqFile.clear();
    mControlFile.clear();
    TUI_ERROR(ret);
end:
    LOGD("%s -- ret : %d", __func__, ret);
    return ret;
}

int32_t TrustedInput::_stTerminateSession(void)
{
    int32_t ret = 0;
    ssize_t writtenBytes = 0;

    writtenBytes = pwrite(mControlFd, "0", 1, 0);
    TUI_CHECK_ERR(writtenBytes > 0, -errno);

    close(mControlFd);
    mControlFd = -1;
    mControlFile.clear();
    close(mIrqFd);
    mIrqFd = -1;
    mIrqFile.clear();

    stSession = false;
end:
    LOGD("%s -- ret : %d", __func__, ret);
    return ret;
}

void TrustedInput::_waitForInputThreadFunc(int32_t _timeout)
{
    constexpr uint32_t MAX_INPUT_SIZE = 4;
    vector<uint8_t> _input(MAX_INPUT_SIZE);
    int32_t ret;

    LOGD("%s: waiting for input, with a timeout of:%d", __func__, _timeout);
    ret = stWaitForEvent(mAbortFd, mIrqFd, _timeout);
    TUI_CHECK_ERR(stSession == true && mCB != nullptr, -EBUSY);
    if (ret == -ETIMEDOUT) {
        auto ret = mCB->notifyTimeout();
        if (!ret.isOk() && ret.isDeadObject()) {
            LOGE("%s: failed to call notifyTimeout callback (CB object dead)",
                 __func__);
        }
    } else if (ret != -ECONNABORTED) {
        LOGD("%s: notifyInput callback", __func__);
        auto ret = mCB->notifyInput(_input);
        if (!ret.isOk() && ret.isDeadObject()) {
            LOGE("%s: failed to call notifyInput callback (CB object dead)",
                 __func__);
        }
    }
    LOGD("%s -- ret:%d", __func__, ret);
end:
    return;
}

// Methods from ::vendor::qti::hardware::trustedui::V1_0::ITrustedInput follow.

Return<void> TrustedInput::init(const sp<ITrustedInputCallback> &cb,
                                init_cb _hidl_cb)
{
    int32_t ret = -1;

    TUI_CHECK_ERR(stSession == false, -EBUSY);
    TUI_CHECK_ERR(cb != nullptr, -EINVAL);

    ret = _stStartSession();
    TUI_CHECK_ERR(ret == 0, ret);

    mAbortFd = eventfd(0, 0);
    if (mAbortFd == -1) {
        LOGE("Error creating eventfd: %d %s", -errno, strerror(errno));
        ret = -errno;
        _stTerminateSession();
        goto end;
    }
    // Register the callback to notify input events
    mCB = cb;
    stSession = true;
end:
    _hidl_cb((ret == 0) ? Response::TUI_SUCCESS : Response::TUI_FAILURE,
             mTouchControllerId);
    return Void();
}

Return<Response> TrustedInput::terminate()
{
    uint64_t c = 1;
    int32_t ret = -1;
    ssize_t writtenBytes = 0;

    TUI_CHECK_ERR(stSession == true, -EBUSY);

    mCB = nullptr;
    LOGV("terminate: write to abortFd");
    writtenBytes = write(mAbortFd, &c, sizeof(c));
    LOGV("terminate : write(fd=%d) writtenBytes %zd", mAbortFd, writtenBytes);
    close(mAbortFd);
    mAbortFd = -1;

    if (mThread && mThread->joinable()) {
        LOGV("TrustedInput::terminate: stopping wait-thread");
        mThread->join();
        mThread = nullptr;
    }
    LOGV("%s: TrustedInput::terminate: wait-thread stopped", __func__);

    ret = _stTerminateSession();
    TUI_CHECK_ERR(ret == 0, ret);
end:
    return (ret == 0) ? Response::TUI_SUCCESS : Response::TUI_FAILURE;
}

Return<Response> TrustedInput::getInput(int32_t timeout)
{
    int32_t res = 0;

    if (stSession != true) {
        LOGE("Failed due to no or incorrect initalization");
        res = -EPERM;
        goto end;
    }
    LOGD("%s: Starting input-wait thread with timeout:%d", __func__, timeout);
    if (mThread && mThread->joinable()) {
        LOGD("%s: getInput called,while one is still pending completion!", __func__);
        mThread->join();
        mThread = nullptr;
    }

    mThread =
        make_shared<thread>(
                [&](int _timeout) {
                    this->_waitForInputThreadFunc(_timeout);
                },
                timeout);
    if (!mThread) {
        LOGE("TrustedInput::getInput: Failed to create thread !");
        res = -EPERM;
    }
end:
    return (res == 0) ? Response::TUI_SUCCESS : Response::TUI_FAILURE;
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

}  // namespace implementation
}  // namespace V1_0
}  // namespace trustedui
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
