/* Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import android.telephony.ims.ImsStreamMediaProfile;

public class ImsMediaUtils {
    private ImsMediaUtils() {
    }

    public static ImsStreamMediaProfile newImsStreamMediaProfile() {
        return newImsStreamMediaProfile(ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.RTT_MODE_DISABLED);
    }

    public static ImsStreamMediaProfile newImsStreamMediaProfile(int audioDirection,
            int rttMode) {
        return newImsStreamMediaProfile(ImsStreamMediaProfile.AUDIO_QUALITY_NONE, audioDirection,
                rttMode);
    }

    public static ImsStreamMediaProfile newImsStreamMediaProfile(int audioQuality,
               int audioDirection, int rttMode) {
        return newImsStreamMediaProfile(audioQuality, audioDirection,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE, ImsStreamMediaProfile.DIRECTION_INVALID,
                rttMode);
    }

    public static ImsStreamMediaProfile newImsStreamMediaProfile(int audioQuality,
            int videoQuality, int videoDirection, int rttMode) {
        return newImsStreamMediaProfile(audioQuality, ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                videoQuality, videoDirection, rttMode);
    }

    public static ImsStreamMediaProfile newImsStreamMediaProfile(int audioQuality,
            int audioDirection, int videoQuality, int videoDirection, int rttMode) {
        return new ImsStreamMediaProfile(audioQuality, audioDirection, videoQuality, videoDirection,
                rttMode);
    }
}
