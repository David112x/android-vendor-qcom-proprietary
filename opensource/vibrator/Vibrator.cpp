/*
 * Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
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

#define LOG_TAG "VibratorService.qti"

#include <hardware/hardware.h>
#include <hardware/vibrator.h>
#include <inttypes.h>
#include <linux/input.h>
#include <log/log.h>
#include <sys/ioctl.h>

#include "Vibrator.h"

namespace android {
namespace hardware {
namespace vibrator {
namespace V1_2 {
namespace implementation {

#define STRONG_MAGNITUDE        0x7fff
#define MEDIUM_MAGNITUDE        0x5fff
#define LIGHT_MAGNITUDE         0x3fff

using Status = ::android::hardware::vibrator::V1_0::Status;

Vibrator::Vibrator(int vibraFd, bool supportGain, bool supportEffects) :
    mVibraFd(vibraFd), mSupportGain(supportGain),
    mSupportEffects(supportEffects) {
    mCurrAppId = -1;
    mCurrEffectId = -1;
    mCurrMagnitude = 0x7fff;
    mPlayLengthMs = 0;
}

/** Play vibration
 *
 *  @param timeoutMs: playing length, non-zero means playing; zero means stop playing.
 *
 *  If the request is playing with a predefined effect, the timeoutMs value is
 *  ignored, and the real playing length is required to be returned from the kernel
 *  driver for userspace service to wait until the vibration done.
 *
 *  The custom_data in periodic is reused for this usage. It's been defined with
 *  following format: <effect-ID, play-time-in-seconds, play-time-in-milliseconds>.
 *  The effect-ID is used for passing down the predefined effect to kernel driver,
 *  and play-time-xxx is used for return back the real playing length from kernel
 *  driver.
 */
Return<Status> Vibrator::play(uint32_t timeoutMs) {
    struct ff_effect effect;
    struct input_event play;
    #define CUSTOM_DATA_LEN    3
    int16_t data[CUSTOM_DATA_LEN] = {0, 0, 0};
    int ret;

    if (timeoutMs != 0) {
        if (mCurrAppId != -1) {
            ret = TEMP_FAILURE_RETRY(ioctl(mVibraFd, EVIOCRMFF, mCurrAppId));
            if (ret == -1) {
                ALOGE("ioctl EVIOCRMFF failed, errno = %d", -errno);
                goto errout;
            }
            mCurrAppId = -1;
        }

        memset(&effect, 0, sizeof(effect));
        if (mCurrEffectId != -1) {
            data[0] = mCurrEffectId;
            effect.type = FF_PERIODIC;
            effect.u.periodic.waveform = FF_CUSTOM;
            effect.u.periodic.magnitude = mCurrMagnitude;
            effect.u.periodic.custom_data = data;
            effect.u.periodic.custom_len = sizeof(int16_t) * CUSTOM_DATA_LEN;
        } else {
            effect.type = FF_CONSTANT;
            effect.u.constant.level = mCurrMagnitude;
            effect.replay.length = timeoutMs;
        }

        effect.id = mCurrAppId;
        effect.replay.delay = 0;

        ret = TEMP_FAILURE_RETRY(ioctl(mVibraFd, EVIOCSFF, &effect));
        if (ret == -1) {
            ALOGE("ioctl EVIOCSFF failed, errno = %d", -errno);
            goto errout;
        }

        mCurrAppId = effect.id;
        if (mCurrEffectId != -1) {
            mPlayLengthMs = data[1] * 1000 + data[2];
            mCurrEffectId = -1;
        }

        play.value = 1;
        play.type = EV_FF;
        play.code = mCurrAppId;
        play.time.tv_sec = 0;
        play.time.tv_usec = 0;
        ret = TEMP_FAILURE_RETRY(write(mVibraFd, (const void*)&play, sizeof(play)));
        if (ret == -1) {
            ALOGE("write failed, errno = %d", -errno);
            ret = TEMP_FAILURE_RETRY(ioctl(mVibraFd, EVIOCRMFF, mCurrAppId));
            if (ret == -1)
                ALOGE("ioctl EVIOCRMFF failed, errno = %d", -errno);
            goto errout;
        }
    } else if (mCurrAppId != -1) {
        ret = TEMP_FAILURE_RETRY(ioctl(mVibraFd, EVIOCRMFF, mCurrAppId));
        if (ret == -1) {
            ALOGE("ioctl EVIOCRMFF failed, errno = %d", -errno);
            goto errout;
        }
        mCurrAppId = -1;
        mPlayLengthMs = 0;
    }
    return Status::OK;

errout:
    mCurrAppId = -1;
    mCurrEffectId = -1;
    mPlayLengthMs = 0;
    return Status::UNSUPPORTED_OPERATION;
}

Return<Status> Vibrator::on(uint32_t timeoutMs) {
    return play(timeoutMs);
}

Return<Status> Vibrator::off() {
    return play(0);
}

Return<bool> Vibrator::supportsAmplitudeControl() {
    return mSupportGain ? true : false;
}

Return<Status> Vibrator::setAmplitude(uint8_t amplitude) {
    int tmp, ret;
    struct input_event ie;

    if (!mSupportGain)
        return Status::UNSUPPORTED_OPERATION;

    if (amplitude == 0)
        return Status::BAD_VALUE;

    tmp = amplitude * (STRONG_MAGNITUDE - LIGHT_MAGNITUDE) / 255;
    tmp += LIGHT_MAGNITUDE;
    ie.type = EV_FF;
    ie.code = FF_GAIN;
    ie.value = tmp;

    ret = TEMP_FAILURE_RETRY(write(mVibraFd, &ie, sizeof(ie)));
    if (ret == -1) {
        ALOGE("write FF_GAIN failed, errno = %d", -errno);
        return Status::UNSUPPORTED_OPERATION;
    }

    mCurrMagnitude = tmp;
    return Status::OK;
}

using EffectStrength = ::android::hardware::vibrator::V1_0::EffectStrength;
static int16_t convertEffectStrength(EffectStrength es) {
    int16_t magnitude;

    switch (es) {
    case EffectStrength::LIGHT:
        magnitude = LIGHT_MAGNITUDE;
        break;
    case EffectStrength::MEDIUM:
        magnitude = MEDIUM_MAGNITUDE;
        break;
    case EffectStrength::STRONG:
        magnitude = STRONG_MAGNITUDE;
        break;
    default:
        return 0;
    }

    return magnitude;
}

using Effect_1_0 = ::android::hardware::vibrator::V1_0::Effect;
Return<void> Vibrator::perform(Effect_1_0 effect, EffectStrength es, perform_cb _hidl_cb) {
    Status status;

    if (!mSupportEffects) {
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
        return Void();
    }

    mCurrEffectId = static_cast<int16_t>(effect);
    mCurrMagnitude = convertEffectStrength(es);
    if (mCurrEffectId < (static_cast<int16_t>(Effect_1_0::CLICK)) ||
            mCurrEffectId > (static_cast<int16_t>(Effect_1_0::DOUBLE_CLICK)) ||
            mCurrMagnitude == 0) {
            _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
            return Void();
    }

    status = play(-1);
    if (status == Status::OK)
        _hidl_cb(Status::OK, mPlayLengthMs);
    else
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);

    return Void();
}

using Effect_1_1 = ::android::hardware::vibrator::V1_1::Effect_1_1;
Return<void> Vibrator::perform_1_1(Effect_1_1 effect, EffectStrength es, perform_1_1_cb _hidl_cb) {
    Status status;

    if (!mSupportEffects) {
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
        return Void();
    }

    mCurrEffectId = static_cast<int16_t>(effect);
    mCurrMagnitude = convertEffectStrength(es);
    if (mCurrEffectId < (static_cast<int16_t>(Effect_1_1::CLICK)) ||
            mCurrEffectId > (static_cast<int16_t>(Effect_1_1::TICK)) ||
            mCurrMagnitude == 0) {
            _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
            return Void();
    }

    status = play(-1);
    if (status == Status::OK)
        _hidl_cb(Status::OK, mPlayLengthMs);
    else
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);

    return Void();
}

using Effect_1_2 = ::android::hardware::vibrator::V1_2::Effect;
Return<void> Vibrator::perform_1_2(Effect_1_2 effect, EffectStrength es, perform_1_2_cb _hidl_cb) {
    Status status;

    if (!mSupportEffects) {
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
        return Void();
    }

    mCurrEffectId = static_cast<int16_t>(effect);
    mCurrMagnitude = convertEffectStrength(es);
    if (mCurrEffectId < (static_cast<int16_t>(Effect_1_2::CLICK)) ||
            mCurrEffectId > (static_cast<int16_t>(Effect_1_2::RINGTONE_15)) ||
            mCurrMagnitude == 0) {
            _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
            return Void();
    }

    status = play(-1);
    if (status == Status::OK)
        _hidl_cb(Status::OK, mPlayLengthMs);
    else
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);

    return Void();
}

}  // namespace implementation
}  // namespace V1_2
}  // namespace vibrator
}  // namespace hardware
}  // namespace android
