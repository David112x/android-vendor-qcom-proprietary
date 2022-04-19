/*
 * Copyright (C) 2016-2017 The Linux Foundation. All rights reserved
 * Not a Contribution.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 * * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (C) 2012 The Android Open Source Project
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

package com.android.bluetooth.btservice;

import android.util.Log;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothClass;
import android.bluetooth.BluetoothQualityReport;
import com.android.bluetooth.Utils;

import android.content.Intent;
import android.content.Context;
final class Vendor {
    private static final String TAG = "BluetoothVendorService";
    private AdapterService mService;
    private boolean isQtiStackEnabled;
    private String socName;
    private String a2dpOffloadCap;
    // Split A2dp will be enabled by default
    private boolean splitA2dpEnabled = true;
    private static boolean PowerbackoffStatus = false;
    // SWB will be enabled by default
    private boolean isSwbEnabled = true;
    // SWB-PM will be enabled by default
    private boolean isSwbPmEnabled = true;

    static {
        classInitNative();
    }

    public Vendor(AdapterService service) {
        mService = service;
    }

    public void init(){
        initNative();
        isQtiStackEnabled = getQtiStackStatusNative();
        Log.d(TAG,"Qti Stack enabled status: " + isQtiStackEnabled);
        socName = getSocNameNative();
        Log.d(TAG,"socName: " + socName);
        a2dpOffloadCap = getA2apOffloadCapabilityNative();
        Log.d(TAG,"a2dpOffloadCap: " + a2dpOffloadCap);
        splitA2dpEnabled = isSplitA2dpEnabledNative();
        Log.d(TAG,"splitA2dpEnabled: " + splitA2dpEnabled);
        isSwbEnabled = isSwbEnabledNative();
        Log.d(TAG,"isSwbEnabled: " + isSwbEnabled);
        isSwbPmEnabled = isSwbPmEnabledNative();
        Log.d(TAG,"isSwbPmEnabled: " + isSwbPmEnabled);
    }

    public void bredrCleanup() {
        bredrcleanupNative();
    }

    public void bredrStartup() {
        bredrstartupNative();
    }

    public void setWifiState(boolean status) {
        Log.d(TAG,"setWifiState to: " + status);
        setWifiStateNative(status);
    }

   public void setPowerBackoff(boolean status) {

        if (getPowerBackoff() == status)
            return;
        Log.d(TAG,"setPowerBackoff to: " + status);
        PowerbackoffStatus = status;
        setPowerBackoffNative(status);
    }

   public boolean getPowerBackoff() {
        Log.d(TAG,"getPowerBackoff " );
        return PowerbackoffStatus;
    }

    public void HCIClose() {
        hcicloseNative();
    }

    public boolean getProfileInfo(int profile_id , int profile_info) {
        Log.d(TAG,"getProfileInfo profile_id: " + profile_id);
        return getProfileInfoNative(profile_id, profile_info);
    }

    public boolean getQtiStackStatus() {
        return isQtiStackEnabled;
    }

    public void cleanup() {
        cleanupNative();
    }

   private void onBredrCleanup(boolean status) {
        Log.d(TAG,"BREDR cleanup done");
        mService.startBluetoothDisable();
    }

   public void voipNetworkWifiInformation(boolean isVoipStarted, boolean isNetworkWifi) {
        Log.d(TAG,"In voipNetworkWifiInformation, isVoipStarted: " + isVoipStarted +
                     ", isNetworkWifi: " + isNetworkWifi);
        voipNetworkWifiInfoNative(isVoipStarted, isNetworkWifi);
   }

    private void iotDeviceBroadcast(byte[] remoteAddr,
                int error, int error_info, int event_mask, int lmpVer, int lmpSubVer,
                int manufacturerId,int pwr_level, int rssi, int linkQuality,
                int glitchCount) {
        String mRemoteAddr = Utils.getAddressStringFromByte(remoteAddr);
        BluetoothDevice mBluetoothDevice = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(mRemoteAddr);
        String mRemoteName = mService.getRemoteName(mBluetoothDevice);
        int mRemoteCoD = mService.getRemoteClass(mBluetoothDevice);
        Log.d(TAG,"iotDeviceBroadcast " + mRemoteName + " address: " + mRemoteAddr + " error: " + error
                    + " error info: " + error_info + " event mask: " + event_mask + "Class of Device: " + mRemoteCoD
                    + " lmp version: " + lmpVer + " lmp subversion: " + lmpSubVer + " manufacturer: " + manufacturerId
                    + " power level: " + pwr_level + " rssi: " + rssi + " link quality: " + linkQuality
                    + " glitch count: " + glitchCount);

        Intent intent = new Intent(BluetoothDevice.ACTION_REMOTE_ISSUE_OCCURRED);
        intent.putExtra(BluetoothDevice.EXTRA_DEVICE, mRemoteAddr);
        intent.putExtra(BluetoothDevice.EXTRA_NAME, mRemoteName);
        intent.putExtra(BluetoothDevice.EXTRA_CLASS, mRemoteCoD);
        intent.putExtra(BluetoothDevice.EXTRA_ISSUE_TYPE, error);
        intent.putExtra(BluetoothDevice.EXTRA_ERROR_CODE, error_info);
        intent.putExtra(BluetoothDevice.EXTRA_ERROR_EVENT_MASK, event_mask);
        intent.putExtra(BluetoothDevice.EXTRA_LMP_VERSION, lmpVer);
        intent.putExtra(BluetoothDevice.EXTRA_LMP_SUBVER, lmpSubVer);
        intent.putExtra(BluetoothDevice.EXTRA_MANUFACTURER, manufacturerId);
        intent.putExtra(BluetoothDevice.EXTRA_POWER_LEVEL, pwr_level);
        intent.putExtra(BluetoothDevice.EXTRA_RSSI, rssi);
        intent.putExtra(BluetoothDevice.EXTRA_LINK_QUALITY, linkQuality);
        intent.putExtra(BluetoothDevice.EXTRA_GLITCH_COUNT, glitchCount);
        mService.sendBroadcast(intent, AdapterService.BLUETOOTH_PERM);
    }

    private void bqrDeliver(byte[] remoteAddr,
            int lmpVer, int lmpSubVer, int manufacturerId, byte[] bqrRawData) {
        String remoteName = "";
        int remoteCoD = 0;
        String addr = Utils.getAddressStringFromByte(remoteAddr);
        if (addr != null) {
            BluetoothDevice device = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(addr);
            remoteName = mService.getRemoteName(device);
            remoteCoD = mService.getRemoteClass(device);
        }

        BluetoothQualityReport bqr;
        try {
            bqr = new BluetoothQualityReport(addr, lmpVer, lmpSubVer,
                    manufacturerId, remoteName, remoteCoD, bqrRawData);
        } catch (Exception e) {
            Log.e(TAG, "bqrDeliver: failed to create bqr", e);
            return;
        }
        Log.d(TAG, "" + bqr);
        Intent intent = new Intent(BluetoothDevice.ACTION_REMOTE_ISSUE_OCCURRED);
        intent.putExtra(BluetoothDevice.EXTRA_BQR, bqr);
        mService.sendBroadcast(intent, AdapterService.BLUETOOTH_PERM);
    }

    void ssr_cleanup_callback() {
        Log.e(TAG,"ssr_cleanup_callback");
        mService.ssrCleanupCallback();
    }

    void devicePropertyChangedCallback(byte[] address, int[] types, byte[][] values) {
        byte[] val;
        int type;
        short twsPlusType;
        boolean autoConnect;
        byte[] mPeerAddress;
        if (types.length <= 0) {
            Log.e(TAG, "No properties to update");
            return;
        }

        for (int j = 0; j < types.length; j++) {
            type = types[j];
            val = values[j];
            if (val.length > 0) {
                Log.d(TAG, "Property type: " + type);
                switch (type) {
                    case AbstractionLayer.BT_VENDOR_PROPERTY_TWS_PLUS_DEVICE_TYPE:
                        twsPlusType = val[0];
                        mService.setTwsPlusDevType(address, twsPlusType);
                        //debugLog("Remote Device name is: " + device.mName);
                        break;
                    case AbstractionLayer.BT_VENDOR_PROPERTY_TWS_PLUS_PEER_ADDR:
                        mPeerAddress = val;
                        mService.setTwsPlusPeerEbAddress(address, mPeerAddress);
                        break;
                    case AbstractionLayer.BT_VENDOR_PROPERTY_TWS_PLUS_AUTO_CONNECT:
                        if(val[0] > 0) {
                          autoConnect = true;
                        } else {
                          autoConnect = false;
                        }
                        mService.setTwsPlusAutoConnect(address, autoConnect);
                        break;
                }
            }
        }
    }

    void adapterPropertyChangedCallback(int[] types, byte[][] values) {
        byte[] val;
        int type;

        if (types.length <= 0) {
            Log.e(TAG, "No properties to update");
            return;
        }

        for (int j = 0; j < types.length; j++) {
            type = types[j];
            val = values[j];
            Log.d(TAG, "Property type: " + type);
            switch (type) {
                case AbstractionLayer.BT_VENDOR_PROPERTY_HOST_ADD_ON_FEATURES:
                    mService.updateHostFeatureSupport(val);
                    break;
                case AbstractionLayer.BT_VENDOR_PROPERTY_SOC_ADD_ON_FEATURES:
                    mService.updateSocFeatureSupport(val);
                    break;
            }
        }
    }

    public String getSocName() {
        return socName;
    }

    public String getA2apOffloadCapability() {
        return a2dpOffloadCap;
    }

    public boolean isSplitA2dpEnabled() {
        return splitA2dpEnabled;
    }
    public boolean isSwbEnabled() {
        return isSwbEnabled;
    }
    public boolean isSwbPmEnabled() {
        return isSwbPmEnabled;
    }

    public boolean setClockSyncConfig(boolean enable, int mode, int adv_interval,
          int channel, int jitter, int offset) {
        if (mode != 0 && mode != 1) {
            Log.e(TAG, "invalid mode setting(0: GPIO, 1: VSC) " + mode);
            return false;
        }
        Log.d(TAG, "enable: " + enable + "mode: " + mode + "adv_interval: " +
                adv_interval + "channel: " + channel + "jitter: " + jitter +
                "offset: " + offset);
        return setClockSyncConfigNative(enable, mode, adv_interval, channel,
                jitter, offset);
    }

    public boolean startClockSync() {
        return startClockSyncNative();
    }

    private native void bredrcleanupNative();
    private native void bredrstartupNative();
    private native void initNative();
    private native static void classInitNative();
    private native void cleanupNative();
    private native void setWifiStateNative(boolean status);
    private native void setPowerBackoffNative(boolean status);
    private native boolean getProfileInfoNative(int profile_id , int profile_info);
    private native boolean getQtiStackStatusNative();
    private native boolean voipNetworkWifiInfoNative(boolean isVoipStarted, boolean isNetworkWifi);
    private native void hcicloseNative();
    private native String getSocNameNative();
    private native String getA2apOffloadCapabilityNative();
    private native boolean isSplitA2dpEnabledNative();
    private native boolean isSwbEnabledNative();
    private native boolean isSwbPmEnabledNative();
    private native boolean setClockSyncConfigNative(boolean enable, int mode, int adv_interval,
        int channel, int jitter, int offset);
    private native boolean startClockSyncNative();
}
