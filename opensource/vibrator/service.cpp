/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "android.hardware.vibrator@1.2-service.qti"

#include <android/hardware/vibrator/1.2/IVibrator.h>
#include <hidl/LegacySupport.h>
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>

#include <dirent.h>
#include <string.h>

#include <log/log.h>
#include <linux/input.h>
#include <sys/ioctl.h>

#include "Vibrator.h"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::vibrator::V1_2::IVibrator;
using android::hardware::vibrator::V1_2::implementation::Vibrator;
using namespace android;

#define test_bit(bit, array)    ((array)[(bit)/8] & (1<<((bit)%8)))
int main() {
    DIR *dp;
    struct dirent *dir;
    uint8_t ffBitmask[FF_CNT / 8];
    char devicename[PATH_MAX];
    const char *INPUT_DIR = "/dev/input/";
    bool supportGain = false, supportEffects = false;
    bool found = false;
    int fd, ret, vibraFd = -1;

    configureRpcThreadpool(1, true);

    dp = opendir(INPUT_DIR);
    if (!dp) {
        ALOGE("open %s failed, errno = %d", INPUT_DIR, errno);
        return UNKNOWN_ERROR;
    }
    memset(ffBitmask, 0, sizeof(ffBitmask));
    while ((dir = readdir(dp)) != NULL){
        if (dir->d_name[0] == '.' &&
            (dir->d_name[1] == '\0' ||
             (dir->d_name[1] == '.' && dir->d_name[2] == '\0')))
            continue;

        snprintf(devicename, PATH_MAX, "%s%s", INPUT_DIR, dir->d_name);
        fd = TEMP_FAILURE_RETRY(open(devicename, O_RDWR));
        if (fd < 0) {
            ALOGE("open %s failed, errno = %d", devicename, errno);
            continue;
        }

        ret = TEMP_FAILURE_RETRY(ioctl(fd, EVIOCGBIT(EV_FF, sizeof(ffBitmask)), ffBitmask));
        if (ret == -1) {
            ALOGE("ioctl failed, errno = %d", errno);
            close(fd);
            continue;
        }

        if (test_bit(FF_CONSTANT, ffBitmask) ||
                test_bit(FF_PERIODIC, ffBitmask)) {
            found = true;
            vibraFd = fd;
            if (test_bit(FF_CUSTOM, ffBitmask))
                supportEffects = true;
            if (test_bit(FF_GAIN, ffBitmask))
                supportGain = true;
            break;
        }
        close(fd);
    }
    closedir(dp);

    if (found) {
        sp<IVibrator> vibrator = new Vibrator(vibraFd, supportGain, supportEffects);
        ret =  vibrator->registerAsService();
    } else {
        ALOGE("Can't find vibrator device");
        ret = UNKNOWN_ERROR;
    }

    joinRpcThreadpool();
    return ret;
}
