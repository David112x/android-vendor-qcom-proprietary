// Copyright (c) 2017-18 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
package com.qualcomm.qti.seccamservice;

import android.os.Bundle;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;

public class SecCamServiceVendorHandler {

    private final static String LOG_TAG = "SECCAM-SERVICE-VENDOR-HANDLER";

    public static final int JNI_OK = 0;
    public static final int JNI_EPERM = -1;
    public static final int JNI_EINVAL = -22;
    public static final int JNI_ETIMEDOUT = -110;

    private static final int MSG_VENDOR_EXCHANGE_TIMESTAMP     = 2000;
    private static final int MSG_VENDOR_INIT_BIOMETRICS_BUFFER = 2001;
    private static final int MSG_VENDOR_RUN_BIOMETRICS_TEST    = 2002;
    private static final int MSG_VENDOR_START_SECCAM_USECASE   = 2003;
    private static final int MSG_VENDOR_STOP_SECCAM_USECASE    = 2004;

    //=========================================================================
    // JNI API
    //=========================================================================
    private static native long exchangeTimestampWithTA(long timestamp);
    private static native long initBiometricsBufferAndShareWithTA(long bufferSize);
    private static native long runBiometricsTestOnTA(long test_num, long bufferSize);
    private static native long startSecCamUsecase();
    private static native long stopSecCamUsecase();

    //=========================================================================
    static void handleVendorMessage_MSG_VENDOR_EXCHANGE_TIMESTAMP(Message msg) {
        int ret = JNI_OK;

        Log.d(LOG_TAG, "::handleVendorMessage_MSG_VENDOR_EXCHANGE_TIMESTAMP");
        Messenger activityMessenger = msg.replyTo;
        Message replyMsg = Message.obtain();
        replyMsg.what = MSG_VENDOR_EXCHANGE_TIMESTAMP;
        Bundle in_bundle = msg.getData();
        long hlosTimestamp = in_bundle.getLong("hlosTimestamp");

        // Call the JNI function to handle the timestamp exchange command
        long tzTimestamp = exchangeTimestampWithTA(hlosTimestamp);

        Bundle out_bundle = new Bundle();
        out_bundle.putLong("tzTimestamp", tzTimestamp);
        replyMsg.setData(out_bundle);
        try {
            activityMessenger.send(replyMsg);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    //=========================================================================
    static void handleVendorMessage_MSG_VENDOR_INIT_BIOMETRICS_BUFFER(Message msg) {
        int ret = JNI_OK;

        Log.d(LOG_TAG, "::handleVendorMessage_MSG_VENDOR_INIT_BIOMETRICS_BUFFER");
        Messenger activityMessenger = msg.replyTo;
        Message replyMsg = Message.obtain();
        replyMsg.what = MSG_VENDOR_INIT_BIOMETRICS_BUFFER;
        Bundle in_bundle = msg.getData();
        long buffer_size = in_bundle.getLong("bufferSize");

        // Call the JNI function to handle the timestamp exchange command
        long result_code = initBiometricsBufferAndShareWithTA(buffer_size);

        Bundle out_bundle = new Bundle();
        out_bundle.putLong("result", result_code);
        replyMsg.setData(out_bundle);
        try {
            activityMessenger.send(replyMsg);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    //=========================================================================
    static void handleVendorMessage_MSG_VENDOR_RUN_BIOMETRICS_TEST(Message msg) {
        int ret = JNI_OK;

        Log.d(LOG_TAG, "::handleVendorMessage_MSG_VENDOR_RUN_BIOMETRICS_TEST");
        Messenger activityMessenger = msg.replyTo;
        Message replyMsg = Message.obtain();
        replyMsg.what = MSG_VENDOR_RUN_BIOMETRICS_TEST;
        Bundle in_bundle = msg.getData();
        long test_number = in_bundle.getLong("testNumber");
        long buffer_size = in_bundle.getLong("bufferSize");

        // Call the JNI function to handle the timestamp exchange command
        long result_code = runBiometricsTestOnTA(test_number, buffer_size);

        Bundle out_bundle = new Bundle();
        out_bundle.putLong("result", result_code);
        replyMsg.setData(out_bundle);
        try {
            activityMessenger.send(replyMsg);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    //=========================================================================
    static void handleVendorMessage_MSG_VENDOR_START_SECCAM_USECASE(Message msg) {
        int ret = JNI_OK;

        Log.d(LOG_TAG, "::handleVendorMessage_MSG_VENDOR_START_SECCAM_USECASE");
        Messenger activityMessenger = msg.replyTo;
        Message replyMsg = Message.obtain();
        replyMsg.what = MSG_VENDOR_START_SECCAM_USECASE;

        startSecCamUsecase();
        try {
            activityMessenger.send(replyMsg);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    //=========================================================================
    static void handleVendorMessage_MSG_VENDOR_STOP_SECCAM_USECASE(Message msg) {
        int ret = JNI_OK;

        Log.d(LOG_TAG, "::handleVendorMessage_MSG_VENDOR_STOP_SECCAM_USECASE");
        Messenger activityMessenger = msg.replyTo;
        Message replyMsg = Message.obtain();
        replyMsg.what = MSG_VENDOR_STOP_SECCAM_USECASE;

        stopSecCamUsecase();
        try {
            activityMessenger.send(replyMsg);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    //=========================================================================
    static boolean handleVendorMessage(Message msg) {
        Log.d(LOG_TAG, "::handleVendorMessage");
        boolean ret = false;

        switch(msg.what) {
            case MSG_VENDOR_EXCHANGE_TIMESTAMP: {
                handleVendorMessage_MSG_VENDOR_EXCHANGE_TIMESTAMP(msg);
                ret = true;
                break;
            }
            case MSG_VENDOR_INIT_BIOMETRICS_BUFFER: {
                handleVendorMessage_MSG_VENDOR_INIT_BIOMETRICS_BUFFER(msg);
                ret = true;
                break;
            }
            case MSG_VENDOR_START_SECCAM_USECASE: {
                handleVendorMessage_MSG_VENDOR_START_SECCAM_USECASE(msg);
                ret = true;
                break;
            }
            case MSG_VENDOR_STOP_SECCAM_USECASE: {
                handleVendorMessage_MSG_VENDOR_STOP_SECCAM_USECASE(msg);
                ret = true;
                break;
            }
            default:
                ret = false;
        }

        return ret;
    }
}
