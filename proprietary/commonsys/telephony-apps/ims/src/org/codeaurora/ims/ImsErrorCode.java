/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

public class ImsErrorCode {

    private ImsErrorCode() {}

    public static final int SUCCESS = 0;

    /* If radio did not start or is resetting */
    public static final int RADIO_NOT_AVAILABLE = 1;
    public static final int GENERIC_FAILURE = 2;
    public static final int REQUEST_NOT_SUPPORTED = 6;
    public static final int CANCELLED = 7;
    public static final int UNUSED = 16;

    /* Invalid parameter given in a
       REQUEST_MODIFY_CALL_INITIATE */
    public static final int INVALID_PARAMETER = 27;

    /* Remote end rejected a change started by
       REQUEST_MODIFY_CALL_INITIATE */
    public static final int REJECTED_BY_REMOTE = 28;
    public static final int IMS_DEREGISTERED = 29;

    /* request not supported by network */
    public static final int NETWORK_NOT_SUPPORTED = 30;

    public static final int HOLD_RESUME_FAILED = 31;
    public static final int HOLD_RESUME_CANCELED = 32;
    public static final int REINVITE_COLLISION = 33;
    public static final int FDN_CHECK_FAILURE = 34;

    /* STK Call Control */
    public static final int SS_MODIFIED_TO_DIAL = 35;
    public static final int SS_MODIFIED_TO_USSD = 36;
    public static final int SS_MODIFIED_TO_SS = 37;
    public static final int SS_MODIFIED_TO_DIAL_VIDEO = 38;
    public static final int DIAL_MODIFIED_TO_USSD = 39;
    public static final int DIAL_MODIFIED_TO_SS = 40;
    public static final int DIAL_MODIFIED_TO_DIAL = 41;
    public static final int DIAL_MODIFIED_TO_DIAL_VIDEO = 42;
    public static final int DIAL_VIDEO_MODIFIED_TO_USSD = 43;
    public static final int DIAL_VIDEO_MODIFIED_TO_SS = 44;
    public static final int DIAL_VIDEO_MODIFIED_TO_DIAL = 45;
    public static final int DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO = 46;
}
