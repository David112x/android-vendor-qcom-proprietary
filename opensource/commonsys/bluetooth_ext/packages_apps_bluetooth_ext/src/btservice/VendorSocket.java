/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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


package com.android.bluetooth.btservice;

import android.util.Log;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothClass;

final class VendorSocket {
    private static final String TAG = "BluetoothVendorSocketService";
    private AdapterService mService;

    static {
        classInitNative();
    }

    public VendorSocket(AdapterService service) {
        mService = service;
    }

    public void init(){
        initNative();
    }

    public void cleanup() {
        cleanupNative();
    }

    public int setSocketOpt(int type, int channel, int optionName, byte [] optionVal,
            int optionLen) {
       return setSocketOptNative(type, channel, optionName, optionVal, optionLen);
    }

    public int getSocketOpt(int type, int channel, int optionName, byte [] optionVal) {
       return getSocketOptNative(type, channel, optionName, optionVal);
    }

    private native void initNative();
    private native static void classInitNative();
    private native void cleanupNative();
    private native int setSocketOptNative(int fd, int type, int optionName,
                            byte [] optionVal, int optionLen);
    private native int  getSocketOptNative(int fd, int type, int optionName,
                            byte [] optionVal);

}
