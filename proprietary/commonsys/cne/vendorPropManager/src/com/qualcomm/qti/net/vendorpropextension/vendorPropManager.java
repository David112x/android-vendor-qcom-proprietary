/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.net.vendorpropextension;

import android.util.Log;
import android.os.Message;
import android.os.Handler;
import android.os.HwBinder;
import android.os.RemoteException;

import vendor.qti.hardware.data.connection.V1_1.IDataConnection;

public class vendorPropManager {
    private static final String LOGTAG = "vendorPropManager";

    private static IDataConnection mdataConnection = null;

    static final int EVENT_SERVICE_DIED = 1;

    private static VendorPropDeathRecipient mDeathRecipient = null;
    private static VendorPropHandler mHandler = null;

    private final static long mDeathBinderCookie = Integer.MAX_VALUE;

    public vendorPropManager() {
        mDeathRecipient = new VendorPropDeathRecipient();
        mHandler = new VendorPropHandler();
        createDataConnectionClient();
    }

   /** The handler used to handle the internal events */
    public class VendorPropHandler extends Handler {
        //***** Handler implementation
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_SERVICE_DIED:
                    mdataConnection = null;
                    break;
                default:
                    break;
            }
        }
    }

    final class VendorPropDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            Log.e(LOGTAG, "HIDL service Died");
            mHandler.sendMessage(mHandler.obtainMessage(EVENT_SERVICE_DIED, cookie));
        }
    }

    private static boolean createDataConnectionClient() {
        if (mdataConnection == null) {
            try {
                mdataConnection = IDataConnection.getService("slot1");
                if(mdataConnection == null) {
                    Log.d(LOGTAG, "getService for DataConnection on slot1 is null. Trying slot2");
                    mdataConnection = IDataConnection.getService("slot2");
                }
            } catch (RemoteException e) {
                Log.e(LOGTAG, "IDataConnection service not supported: " + e);
                return false;
            }
            if (mdataConnection == null) {
                Log.e(LOGTAG, "IDataConnection::GetService returned null");
                return false;
            }

            try {
                mdataConnection.linkToDeath(mDeathRecipient, mDeathBinderCookie);
            } catch(RemoteException | RuntimeException e) {
                mdataConnection = null;
                Log.e(LOGTAG, "Failed to linkToDeath ");
            }
        }
        return true;
    }

    private static String getConfigValue(String config, String defaultValue) {
        String propResult = defaultValue;
        if (mdataConnection == null) {
            Log.e(LOGTAG, "DataConnection Client not initialized");
            return propResult;
        }

        try {
           propResult = mdataConnection.getConfig(config, defaultValue);
        } catch (RemoteException e) {
            Log.e(LOGTAG, "Remote exception during getConfig " + e);
            return propResult;
        }
        return propResult;
    }

    /*
     * Returns the vendor Property value
     */
    public String getConfig(String key, String defaultValue) {
        if(createDataConnectionClient()) {
            return getConfigValue(key, defaultValue);
        }
        return defaultValue;
    }
}
