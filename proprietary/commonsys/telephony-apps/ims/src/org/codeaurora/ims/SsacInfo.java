/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

public final class SsacInfo {

    /*
     * Message sent as response for REQUEST_GET_SSAC_INFO and UNSOL_SSAC_CHANGED
     * As per 3GPP TS 36.331, Range for
     * barring factor - 0 to 100. Value 100 is used when the UE goes to the LTE Connected state.
     *                  Value 0xFF indicates Invalid.
     * barring time(in seconds) -  0 to 512. Value 0 is used when the UE goes to the LTE
     *                  Connected state. Value 0xFFFF indicates invalid
     *
     */
    private int mBarringFactorVoice;
    private int mBarringTimeVoice;
    private int mBarringFactorVideo;
    private int mBarringTimeVideo;
    private int mBarringFactorVoiceSib;
    private int mBarringTimeVoiceSib;
    private int mBarringFactorVideoSib;
    private int mBarringTimeVideoSib;

    public SsacInfo() {
        mBarringFactorVoice = 0;
        mBarringTimeVoice = 0;
        mBarringFactorVideo = 0;
        mBarringTimeVideo = 0;
        mBarringFactorVoiceSib = 0;
        mBarringTimeVoiceSib = 0;
        mBarringFactorVideoSib = 0;
        mBarringTimeVideoSib = 0;
    }

    public int getBarringFactorVoice() {
        return mBarringFactorVoice;
    }

    public String toString() {
        return "SsacInfo barringFactorVoice: " + mBarringFactorVoice
                + " barringTimeVoice: " + mBarringTimeVoice
                + " barringFactorVideo: " + mBarringFactorVideo
                + " barringTimeVideo: " + mBarringTimeVideo
                + " barringFactorVoiceSib: " + mBarringFactorVoiceSib
                + " barringTimeVoiceSib: " + mBarringTimeVoiceSib
                + " barringFactorVideoSib: " + mBarringFactorVideoSib
                + " barringTimeVideoSib: " + mBarringTimeVideoSib;
    }
}
