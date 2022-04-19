/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
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

package com.android.bluetooth.map;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.NotificationChannel;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;
import android.util.SparseArray;
import com.android.bluetooth.R;
import com.android.bluetooth.Utils;
import com.android.bluetooth.btservice.AdapterService;
import com.android.bluetooth.btservice.AbstractionLayer;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.HashMap;

public class BluetoothMapFixes {

    private static final String TAG = "BluetoothMapFixes";

    public static final boolean DEBUG = BluetoothMapService.DEBUG;

    public static final boolean VERBOSE = BluetoothMapService.VERBOSE;

    private static final int SDP_MAP_MAS_FEATURES_ADV = 0x603ff,
                             SDP_MAP_MAS_VERSION_ADV = 0x0103;
    // Stores map of BD address to isRebonded for the given BT Session
    private static HashMap<String,String> mapSdpResponse = new HashMap <String,String>();
    // Stores map of BD address to MCE version for the given BT Session
    private static HashMap<String,Integer> remoteVersion = new HashMap <String,Integer>();
    private static final String MAP_NOTIFICATION_ID = "map_notification",
                                MAP_NOTIFICATION_NAME = "BT_MAP_ADVANCE_SUPPORT";
    private static final int RECORD_LENGTH = 6,
                             VERSION_LENGTH = 2,
                             MAP_ADV_NOTIFICATION_ID = -1000003,
                             ADDRESS_LENGTH = 3;
    private static NotificationManager mNotificationManager;

    /*to trigger MNS OFF to handle MCE that dosen't issue MNS OFF before MAS DISC*/
    public static void handleMnsShutdown(SparseArray<BluetoothMapMasInstance>
            mMasInstances, int masId) {
        BluetoothMapMasInstance masInst = mMasInstances.get(masId);
        if (masInst != null && masInst.mObserver != null) {
            try {
                masInst.mObserver.setNotificationRegistration(BluetoothMapAppParams
                    .NOTIFICATION_STATUS_NO);
            } catch (RemoteException e) {
                Log.e(TAG,"setNoficationRegistarion OFF Failed: "+ e);
            }
        }
    }

    /*
     * Checks if its the last line of the BMessage
     */
    public static boolean isLastLine(byte [] line) {
        if (line == null || line.length == 0)
            return true;
        return false;
    }

    /**
      * Create both MAP SMS/MMS and EMAIL SDP in a handler thread.
      */
    public static void sendCreateMasInstances(BluetoothMapService mapService,
            int sendCreateMsg) {
        Handler mSessionStatusHandler = mapService.getHandler();
        if (mSessionStatusHandler != null && !mSessionStatusHandler
                .hasMessages(sendCreateMsg)) {
            Log.d(TAG, "mSessionStatusHandler CREATE_MAS_INSTANCES ");
            Message msg = mSessionStatusHandler.obtainMessage(sendCreateMsg);
            /* We add a small delay here to ensure the call returns true before this message is
             * handled. It seems wrong to add a delay, but the alternative is to build a lock
             * system to handle synchronization, which isn't nice either... */
            mSessionStatusHandler.sendMessage(msg);
        } else if(mSessionStatusHandler != null ) {
            Log.w(TAG, "mSessionStatusHandler START_MAPEMAIL message already in Queue");
        }
    }

    /* Check if Map App Observer is null. */
    public static boolean checkMapAppObserver(
            BluetoothMapAppObserver mAppObserver) {
        if (mAppObserver == null) {
            Log.w( TAG, "updateMasInstancesHandler: NoAppObeserver Found");
            return true;
        }
        return false;
    }

    /* to create app observers and get enabled accounts to create MAS instances */
    public static void createMasInstances(BluetoothMapService mService) {
        mService.mAppObserver = new BluetoothMapAppObserver(mService,
                (BluetoothMapService) mService);
        if (mService.mAppObserver != null ) {
            mService.mEnabledAccounts = mService.mAppObserver
                    .getEnabledAccountItems();
        }
       /** Uses mEnabledAccounts, hence getEnabledAccountItems()
          * must be called before this. */
        mService.createMasInstances();
    }

    static boolean isRemoteSupportsAdvMap(BluetoothDevice device) {
        Log.d(TAG, "isRemoteSupportsAdvMap ");
        try {
            final String filePath = "/data/misc/bluedroid/mce_peer_entries.conf";
            File file = new File(filePath);
            Log.d(TAG, "file length = " + (int)file.length());
            byte[] fileData = new byte[(int) file.length()];
            DataInputStream dis = new DataInputStream(new FileInputStream(file));
            dis.readFully(fileData);
            dis.close();
            return readRecord(fileData, device);
        } catch (IOException io) {
            Log.e(TAG, "File Read Failed: " + io);
        }
        return false;
    }

    /* Read all records and check if entry for remote device is found
     * Returns true if 1.4 support is stored for remote else false */
    static boolean readRecord(byte[] fileData, BluetoothDevice device) {
        for (int j = 0 ; (j + RECORD_LENGTH) <= fileData.length;) {
            // Read Version from MCE Entry
            byte[] versionBytes = Arrays.copyOfRange(fileData, j, j + VERSION_LENGTH);
            int version = byteArrayToInt(versionBytes);
            j += VERSION_LENGTH;

            // Read BD ADDRESS from MCE Entry
            StringBuilder address = new StringBuilder();
            address.append(String.format("%02X", fileData[j]) + ":"
                    + String.format("%02X", fileData[j+1]) + ":"
                    + String.format("%02X", fileData[j+2]));
            j += ADDRESS_LENGTH;

            // Read rebonded from MCE Entry
            char isRebonded = (char)fileData[j++];
            Log.d(TAG, "version: " + version + ", address = " + address.toString()
                    + ", isRebonded = "+ isRebonded +" Remote Address = "
                    + device.getAddress());

            boolean isMatched = device.getAddress().toLowerCase()
                    .startsWith(address.toString().toLowerCase());
            mapSdpResponse.put(address.toString(), Character.toString(isRebonded));
            remoteVersion.put(address.toString(), Integer.valueOf(version));
            if (isMatched) {
                return (version >= SDP_MAP_MAS_VERSION_ADV ? true : false);
            }
        }
        return false;
    }

    /* Convert 2 bytes of data to integer */
    static int byteArrayToInt(byte[] b)
    {
        return   b[0] & 0xFF |
                (b[1] & 0xFF) << 8 ;
    }

    /*Creates Notification for MAP version upgrade/downgrade */
   static void createNotification(BluetoothMapService context, boolean isUpgrade) {
        if (VERBOSE) Log.v(TAG, "Create MAP Notification for Upgrade/Downgrade");
        // create Notification channel.
        if (mNotificationManager == null) {
            mNotificationManager = (NotificationManager)
                    context.getSystemService(Context.NOTIFICATION_SERVICE);
            NotificationChannel mChannel = new NotificationChannel(MAP_NOTIFICATION_ID,
                    MAP_NOTIFICATION_NAME, NotificationManager.IMPORTANCE_HIGH);
            mNotificationManager.createNotificationChannel(mChannel);
        }

        // create notification
        String title = isUpgrade ?
                context.getString(R.string.bluetooth_map_remote_advance_feature_support):
                context.getString(R.string.bluetooth_map_remote_message_access_feature_downgrade);
        String contentText = isUpgrade ?
                context.getString(R.string.bluetooth_map_repair_for_adv_message_access_feature):
                context.getString(R.string.bluetooth_map_repair_for_message_access_version_comp);
        Notification notification = new Notification.Builder(context,MAP_NOTIFICATION_ID)
            .setContentTitle(title)
            .setContentText(contentText)
            .setSmallIcon(android.R.drawable.stat_sys_data_bluetooth)
            .setAutoCancel(true)
            .build();
        if (mNotificationManager != null ) {
            mNotificationManager.notify(MAP_ADV_NOTIFICATION_ID, notification);
        } else {
            Log.e(TAG,"mNotificationManager is null");
        }
    }

    /* Checks if notification for Version Upgrade is required */
    static void showNotification(BluetoothMapService service,
            BluetoothDevice remoteDevice) {
        if (service == null || remoteDevice == null || Utils.isPtsTestMode()
                || isMapAdvDisabled()) {
            Log.d(TAG, "Ignore showNotification , service: " + service + " remoteDevice: "
                    + remoteDevice +" isPtsTestMode :" + Utils.isPtsTestMode()
                    + " isMapAdvDisabled :" + isMapAdvDisabled());
            return;
        }
        boolean hasMap14Support = isRemoteSupportsAdvMap(remoteDevice);
        /* check if remote devices is rebonded by looking into its entry in hashmap using key
         * (0,8) i.e. first 3 bytes of bd addr in string format including colon (XX:XX:XX) */
        String isRebonded = mapSdpResponse.get(
                remoteDevice.getAddress().substring(0,8));
        /* fetch MCE version of remote (if present) in hashmap 'remoteVersion' from key
         * (0,8) i.e. first 3 bytes of bd addr in string format including colon (XX:XX:XX) */
        Integer remoteMceVersion = remoteVersion.get(
                remoteDevice.getAddress().substring(0,8));
        if (hasMap14Support && (isRebonded.equals("N"))) {
            Log.d(TAG, "Remote Supports MAP 1.4 Notify user");
            createNotification(service, true);
        } else if (!hasMap14Support
                && (remoteMceVersion != null &&
                    remoteMceVersion.intValue() < SDP_MAP_MAS_VERSION_ADV)
                && (isRebonded != null && isRebonded.equals("N"))) {
            Log.d(TAG, "Remote MAP profile support downgraded");
            createNotification(service, false);
        } else {
            Log.d(TAG, "Notification Not Required.");
            if (mNotificationManager != null)
                mNotificationManager.cancel(MAP_ADV_NOTIFICATION_ID);
        }
    }

    public static boolean isMapAdvDisabled(){
        boolean ismap14Enabled = false;
        int MAP_0104_SUPPORT = 7;
        try {
            AdapterService adapterService = AdapterService.getAdapterService();
            if (adapterService != null) {
                ismap14Enabled = adapterService.getProfileInfo(AbstractionLayer.MAP,
                        MAP_0104_SUPPORT);
            }
        } catch(Exception e) {
            Log.w(TAG," isMapadvEnabled : " + e);
        }
        return !ismap14Enabled;
    }

    static boolean isRebonded(BluetoothDevice remoteDevice) {
        String isRebonded = mapSdpResponse.get(
            remoteDevice.getAddress().substring(0,8));
        return ((isRebonded != null) && isRebonded.equalsIgnoreCase("Y"));
    }
}
