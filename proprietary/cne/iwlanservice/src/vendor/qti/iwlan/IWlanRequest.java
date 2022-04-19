/*
 *   Copyright (c) 2018 Qualcomm Technologies, Inc.
 *   All Rights Reserved.
 *   Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.iwlan;

import android.os.Message;

import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;

public class IWlanRequest {
    //***** Class Variables
    static AtomicInteger sNextSerial = new AtomicInteger(0);
    String mClientId;
    int mSerial;
    int mRequest;
    Message mResult;
    int mWakeLockType;

    IWlanRequest(int request, Message result){
        mSerial = sNextSerial.getAndIncrement();
        mClientId = Integer.toString(mSerial);
        mRequest = request;
        mResult = result;
        mWakeLockType = IWlanProxy.INVALID_WAKELOCK;
    }
}