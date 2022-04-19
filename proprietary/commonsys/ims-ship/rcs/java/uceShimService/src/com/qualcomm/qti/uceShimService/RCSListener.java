/*********************************************************************
 Copyright (c) 2017 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

package com.qualcomm.qti.uceShimService;

import com.android.ims.internal.uce.uceservice.ImsUceManager;
import com.qualcomm.qti.uceservice.V2_0.IUceListener;
import com.qualcomm.qti.uceservice.V2_0.UceStatus;

import android.util.Log;
import android.os.RemoteException;

public class RCSListener extends IUceListener.Stub { //extends HIDL IUceListener

    public static boolean isRCSRegistered = false;
    com.android.ims.internal.uce.uceservice.IUceListener mAidlUceListener = null;
    private final static String logTAG = "RCSService Listener";

    public void onStatusChange(UceStatus status, String iccId) {
        //ToDo Notify Status change Ind
        //AIDL-UCE Listener currently does not have Subcription Specific Callback
        Log.d(logTAG,"onStatusChange_1_1 callback received");
    }

    public void setAidlUceListener(com.android.ims.internal.uce.uceservice.IUceListener aidlUceListener) {
	mAidlUceListener = aidlUceListener;
    }
}
