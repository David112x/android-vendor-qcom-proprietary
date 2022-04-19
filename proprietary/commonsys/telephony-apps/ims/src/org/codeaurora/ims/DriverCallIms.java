/*
 * Copyright (c) 2012, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Confidential and Proprietary.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2006 The Android Open Source Project
 * Copyright (c) 2012 Code Aurora Forum. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.codeaurora.ims;

import android.telephony.ims.ImsReasonInfo;

import com.android.internal.telephony.PhoneConstants;

import java.lang.Comparable;
import java.util.Objects;

/**
 * {@hide}
 */
public class DriverCallIms implements Comparable<DriverCallIms> {
    static final String LOG_TAG = "DRIVERCALL-IMS";

    /**
     * Bit-mask values to indicate the {@link DriverCallIms} properties which changed during a
     * call to {@link #update(DriverCallIms)}.
     */
    public static final int UPDATE_NONE = 0x00;
    public static final int UPDATE_STATE = 0x01;
    public static final int UPDATE_INDEX = 0x02;
    public static final int UPDATE_NUMBER = 0x04;
    public static final int UPDATE_IS_MT = 0x08;
    public static final int UPDATE_IS_MPTY = 0x10;
    public static final int UPDATE_CALL_DETAILS = 0x20;
    public static final int UPDATE_CALL_TRANSFER = 0x40;
    public static final int UPDATE_ENCRYPTION = 0x80;
    public static final int UPDATE_HISTORY_INFO = 0x100;
    public static final int UPDATE_CONF_SUPPORT = 0x200;

    public enum State {
        ACTIVE,
        HOLDING,
        DIALING, // MO call only
        ALERTING, // MO call only
        INCOMING, // MT call only
        WAITING, // MT call only
        END;
    }

    /**
     * Bit-mask values to indicate the conference support information
     * for this call instance, as received from lower layers.
     */
    /* Default disable mask */
    public static final int CONF_SUPPORT_NONE = 0x00;
    /* Enabled when there is a valid conference support information from lower layers */
    public static final int CONF_SUPPORT_INDICATED = 0x01;
    /* Enabled when VT conference support is indicated as supported */
    public static final int CONF_VIDEO_SUPPORTED = 0x02;

    public CallDetails callDetails;
    public State state;
    public ImsReasonInfo callFailCause;
    /* A mask of the possible call transfers supported by this call instance */
    public int mEctMask;
    public boolean isEncrypted;
    public String historyInfo;
    public int mCallFailReason;
    public int index;
    public boolean isMT;
    public boolean isMpty;
    public String number;
    public int TOA;
    public boolean isVoice;
    public boolean isVoicePrivacy;
    public int als;
    public int numberPresentation;
    public String name;
    public int namePresentation;
    /* A mask indicating the conference support information of this call instance */
    public int mConfSupported = CONF_SUPPORT_NONE;
    //VerstatInfo Shoudn't be modified once set. No need to check for an update in the
    //update() method
    private final VerstatInfo mVerstatInfo;

    public MultiIdentityLineInfo mMtMultiLineInfo;
    // Used for MT calls to indicate whether to display additional answer options to
    // the user to overwrite TIR presentation
    public boolean isTirOverwriteAllowed;

    // Copy Constructor
    public DriverCallIms(DriverCallIms dc) {
        callDetails = new CallDetails(dc.callDetails);
        callFailCause = new ImsReasonInfo(dc.callFailCause.getCode(),
                dc.callFailCause.getExtraCode(),
                dc.callFailCause.getExtraMessage());
        state = dc.state;
        index = dc.index;
        number = dc.number;
        isMT = dc.isMT;
        TOA = dc.TOA;
        isMpty = dc.isMpty;
        mEctMask = dc.mEctMask;
        als = dc.als;
        isVoice = dc.isVoice;
        isVoicePrivacy = dc.isVoicePrivacy;
        numberPresentation = dc.numberPresentation;
        name = dc.name;
        namePresentation = dc.namePresentation;
        isEncrypted = dc.isEncrypted;
        historyInfo = dc.historyInfo;
        mConfSupported = dc.mConfSupported;
        mVerstatInfo = dc.getVerstatInfo();
        mMtMultiLineInfo = dc.mMtMultiLineInfo;
        isTirOverwriteAllowed = dc.isTirOverwriteAllowed;
    }

    public DriverCallIms() {
        callDetails = new CallDetails();
        mEctMask = 0;
        mVerstatInfo = new VerstatInfo();
    }

    public DriverCallIms(VerstatInfo verstatInfo) {
        callDetails = new CallDetails();
        mEctMask = 0;
        mVerstatInfo = verstatInfo;
    }

    /**
     * Updates members of the {@link DriverCallIms} with the update.  Bitmask describes
     * which attributes have changed.
     *
     * @param update The update.
     * @return Bit-mask describing the attributes of the {@link DriverCallIms} which changed.
     */
    public int update(DriverCallIms update) {
        if (update == null) {
            return UPDATE_NONE;
        }
        int changed = UPDATE_NONE;
        if (state != update.state) {
            state = update.state;
            changed |= UPDATE_STATE;
        }
        if (index != update.index) {
            index = update.index;
            changed |= UPDATE_INDEX;
        }
        if (number != update.number) {
            number = update.number;
            changed |= UPDATE_NUMBER;
        }
        if (isMT != update.isMT) {
            isMT = update.isMT;
            changed |= UPDATE_IS_MT;
        }
        if (isMpty != update.isMpty) {
            isMpty = update.isMpty;
            changed |= UPDATE_IS_MPTY;
        }
        if (update.callFailCause != null) {
            if (callFailCause == null) {
                callFailCause = new ImsReasonInfo(update.callFailCause.getCode(),
                        update.callFailCause.getExtraCode(),
                        update.callFailCause.getExtraMessage());
            } else {
                int imsReasonCode = callFailCause.getCode();
                int imsReasonExtraCode = callFailCause.getExtraCode();
                String imsReasonExtraMessage = callFailCause.getExtraMessage();
                if (callFailCause.getCode() != update.callFailCause.getCode()) {
                    imsReasonCode = update.callFailCause.getCode();
                }
                if (callFailCause.getExtraCode() != update.callFailCause.getExtraCode()) {
                    imsReasonExtraCode = update.callFailCause.getExtraCode();
                }
                if (callFailCause.getExtraMessage() != update.callFailCause.getExtraMessage()) {
                    imsReasonExtraMessage = update.callFailCause.getExtraMessage();
                }
                callFailCause = new ImsReasonInfo(imsReasonCode, imsReasonExtraCode,
                        imsReasonExtraMessage);
            }
        }
        if(callDetails.update(update.callDetails)) {
            changed |= UPDATE_CALL_DETAILS;
        }
        if (mEctMask != update.mEctMask) {
            mEctMask = update.mEctMask;
            changed |= UPDATE_CALL_TRANSFER;
        }
        if (isEncrypted != update.isEncrypted) {
            isEncrypted = update.isEncrypted;
            changed |= UPDATE_ENCRYPTION;
        }
        if (!Objects.equals(update.historyInfo, historyInfo)) {
            historyInfo = update.historyInfo;
            changed |= UPDATE_HISTORY_INFO;
        }
        if (mConfSupported != update.mConfSupported) {
            mConfSupported = update.mConfSupported;
            changed |= UPDATE_CONF_SUPPORT;
        }
        return changed;
    }

    public VerstatInfo getVerstatInfo() {
        return mVerstatInfo;
    }

    public boolean isConfSupportIndicated() {
        return (mConfSupported & CONF_SUPPORT_INDICATED) == CONF_SUPPORT_INDICATED;
    }

    public boolean isVideoConfSupported() {
        return (mConfSupported & CONF_VIDEO_SUPPORTED) == CONF_VIDEO_SUPPORTED;
    }

    public static int
    presentationFromCLIP(int cli) throws RuntimeException
    {
        switch(cli) {
            case 0: return PhoneConstants.PRESENTATION_ALLOWED;
            case 1: return PhoneConstants.PRESENTATION_RESTRICTED;
            case 2: return PhoneConstants.PRESENTATION_UNKNOWN;
            case 3: return PhoneConstants.PRESENTATION_PAYPHONE;
            default:
                throw new RuntimeException("illegal presentation " + cli);
        }
    }

    /** For sorting by index */
    @Override
    public int
    compareTo(DriverCallIms dc) {

        if (index < dc.index) {
            return -1;
        } else if (index == dc.index) {
            return 0;
        } else { /*index > dc.index*/
            return 1;
        }
    }

    public String toString() {
        return "id=" + index + "," + state + "," + "toa=" + TOA + ","
                + (isMpty ? "conf" : "norm") + "," + (isMT ? "mt" : "mo") + ","
                + als + "," + (isVoice ? "voc" : "nonvoc") + ","
                + (isVoicePrivacy ? "evp" : "noevp") + ","
                /* + "number=" + number */+ ",cli=" + numberPresentation + ","
                /* + "name="+ name */+ "," + namePresentation
                + "Call Details =" + callDetails + "," + "CallFailCause Code= "
                + callFailCause.getCode() + "," + "CallFailCause String= "
                + callFailCause.getExtraMessage()
                + ", ECT mask: " + mEctMask
                + ", isEncrypted=" + isEncrypted
                + ", historyInfo=" + historyInfo
                + ", Conf. Support=" + mConfSupported
                + ", " + mMtMultiLineInfo
                + ", isTirOverwriteAllowed=" + isTirOverwriteAllowed;
    }
}
