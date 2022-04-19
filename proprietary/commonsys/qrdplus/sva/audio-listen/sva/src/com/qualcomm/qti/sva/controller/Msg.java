/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.controller;

public class Msg {
    // Req&Rsp Message ID define
    // for value < 0x1000, used for client internal
    public static final int MSG_REGISTER_CLIENT_REQ = 0x1000;
    public static final int MSG_REGISTER_CLIENT_RSP = 0x1001;
    public static final int MSG_DE_REGISTER_CLIENT_REQ = 0x1002;
    public static final int MSG_DE_REGISTER_CLIENT_RSP = 0x1003;
    public static final int MSG_ESTABLISH_SESSION_REQ = 0x1004;
    public static final int MSG_ESTABLISH_SESSION_RSP = 0x1005;
    public static final int MSG_TERMINATE_SESSION_REQ = 0x1006;
    public static final int MSG_TERMINATE_SESSION_RSP = 0x1007;
    public static final int MSG_LOAD_SOUND_MODEL_REQ = 0x1008;
    public static final int MSG_LOAD_SOUND_MODEL_RSP = 0x1009;
    public static final int MSG_START_RECOGNITION_REQ = 0x1010;
    public static final int MSG_START_RECOGNITION_RSP = 0x1011;
    public static final int MSG_STOP_RECOGNITION_REQ = 0x1012;
    public static final int MSG_STOP_RECOGNITION_RSP = 0x1013;
    public static final int MSG_UNLOAD_SOUND_MODEL_REQ = 0x1014;
    public static final int MSG_UNLOAD_SOUND_MODEL_RSP = 0x1015;
    public static final int MSG_RELEASE_ALL_SESSIONS_REQ = 0x1016;
    public static final int MSG_RELEASE_ALL_SESSIONS_RSP = 0x1017;

    // ind msg
    public static final int MSG_VOICE_REQUEST_IND = 0x1100;
    public static final int MSG_DETECTION_IND = 0x1101;
    public static final int MSG_AUDIO_SERVER_DIED_IND = 0x1102;

    public enum ClientID {
        CLIENT_ID_MAIN_ACTIVITY,
        CLIENT_ID_DEBUG_MAIN_ACTIVITY,
        CLIENT_ID_DEBUG_MODEL_LIST_ACTIVITY,
    }
}
