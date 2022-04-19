/*
 * Copyright (c) 2016, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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

package org.codeaurora.bluetooth.bttestapp;

import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.IBluetooth;
import android.bluetooth.IBluetoothHidHost;
import android.bluetooth.BluetoothHidHost;
import android.bluetooth.BluetoothProfile.ServiceListener;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
// Activity for hid host qualification.
public class HidTestApp extends Activity implements OnClickListener {
    private static final String TAG = "HidTestApp";
    private static BluetoothHidHost mService = null;
    private BluetoothAdapter mAdapter;
    private BluetoothDevice mRemoteDevice;
    private Context mContext;

    private EditText mEtBtAddress, mEtBtCommand;
    private Button mBtnExecute;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hidtestapp);
        Log.i(TAG, "On Create ");
        mEtBtAddress = (EditText) findViewById(R.id.id_et_btaddress);
        mEtBtCommand = (EditText) findViewById(R.id.id_et_btcommand);
        mBtnExecute = (Button) findViewById(R.id.id_btn_execute);
        mBtnExecute.setOnClickListener(this);
        mContext = getApplicationContext();
        mAdapter = BluetoothAdapter.getDefaultAdapter();
        init();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        deinit();
    }

    @Override
    public void onClick(View v) {
        if (v == mBtnExecute) {
            executeCommand();
        }

    }

    private boolean executeCommand() {
        boolean isValid = true;
        String address = mEtBtAddress.getText().toString().trim();
        String command = mEtBtCommand.getText().toString().trim();
        Log.e(TAG, "address = " + address + ", command = " + command);
        if (address.length() != 17) {
            mEtBtAddress.setError("Enter Proper Address");
            isValid = false;
        } else if (command.length() == 0) {
            mEtBtCommand.setError("Enter proper command");
            isValid = false;
        }

        if (isValid) {
            /* Make sure that all the address bytes are in upper case */
            String addSplit[] = address.split(":");
            address = addSplit[0].toUpperCase() + ":" + addSplit[1].toUpperCase() + ":" +
                addSplit[2].toUpperCase() + ":" + addSplit[3].toUpperCase() + ":" +
                addSplit[4].toUpperCase() + ":" + addSplit[5].toUpperCase();
            Log.e(TAG, "address = " + address);
            try {
                mRemoteDevice = mAdapter.getRemoteDevice(address);
            } catch (IllegalArgumentException e) {
                toast("Invalid BD Address");
                return false;
            }
            String[] separated = command.split(" ");
            int len = separated.length;

            if (len > 0) {
                if (separated[0].equals("getreport")) {
                    if (len != 4) {
                        toast("Invalid getreport command");
                        isValid = false;
                    } else {
                        try {
                            int reportType = Integer.parseInt(separated[1]);
                            int reportId = Integer.parseInt(separated[2]);
                            int bufferSize = Integer.parseInt(separated[3]);
                            Log.v(TAG," reportType " + reportType+ ", reportId " + reportId
                                    + ", bufferSize " + bufferSize);
                            getReport(mRemoteDevice, (byte)reportType, (byte)reportId, bufferSize);
                            toast("getreport command sent");
                        } catch(NumberFormatException e) {
                            toast("getreport input parameters exception");
                        }
                    }
                } else if (separated[0].equals("setreport")) {
                    if (len != 3) {
                        toast("Invalid setreport command");
                        isValid = false;
                    } else {
                        try {
                            int reportType = Integer.parseInt(separated[1]);
                            Log.v(TAG," reportType " + reportType + ", report " + separated[2]);
                            setReport(mRemoteDevice, (byte)reportType, separated[2]);
                            toast("setreport command sent");
                        } catch(NumberFormatException e) {
                            toast("setreport input parameters exception");
                        }
                    }
                }  else if (separated[0].equals("virtualunplug")) {
                    if (len != 1) {
                        toast("Invalid virtualunplug command");
                        isValid = false;
                    } else {
                        virtualUnplug(mRemoteDevice);
                        toast("virtualunplug command sent");
                    }
                } else if (separated[0].equals("getprotocolmode")) {
                    if (len != 1) {
                        toast("Invalid getprotocolmode command");
                        isValid = false;
                    } else {
                        getProtocolMode(mRemoteDevice);
                        toast("getprotocolmode command sent");
                    }
                } else if (separated[0].equals("setprotocolmode")) {
                    if (len != 2) {
                        toast("Invalid setprotocolmode command");
                        isValid = false;
                    } else {
                        try {
                            int mode = Integer.parseInt(separated[1]);
                            Log.v(TAG," mode " + mode);
                            setProtocolMode(mRemoteDevice, mode);
                            toast("setprotocolmode command sent");
                        } catch(NumberFormatException e) {
                            toast("setprotocolmode input parameters exception");
                        }
                    }
                } else if (separated[0].equals("getidle")) {
                    if (len != 1) {
                        toast("Invalid getidle command");
                        isValid = false;
                    } else {
                        getIdleTime(mRemoteDevice);
                        toast("getidle command sent");
                    }
                } else if (separated[0].equals("setidle")) {
                    if (len != 2) {
                        toast("Invalid setidle command");
                        isValid = false;
                    } else {
                        try {
                            int idleTime = Integer.parseInt(separated[1]);
                            Log.v(TAG," idleTime " + idleTime);
                            setIdleTime(mRemoteDevice, (byte)idleTime);
                            toast("setidle command sent");
                        } catch(NumberFormatException e) {
                            toast("setidle input parameters exception");
                        }
                    }
                } else if (separated[0].equals("senddata")) {
                    if (len != 2) {
                        toast("Invalid senddata command");
                        isValid = false;
                    } else {
                        Log.v(TAG," report " + separated[1]);
                        sendData(mRemoteDevice, separated[1]);
                        toast("senddata command sent");
                    }
                } else {
                    toast("Invalid command");
                    Log.v(TAG," Invalid command " + separated[0]);
                    isValid = false;
                }
            }
        }
        return isValid;
    }

    private void toast(String msg){
        Toast.makeText(mContext, msg, Toast.LENGTH_LONG).show();
    }

    public void init() {
        if (!mAdapter.getProfileProxy(mContext, mServiceListener,
                BluetoothProfile.HID_HOST)) {
            Log.w(TAG, "Cannot obtain profile proxy");
            return;
        }
    }

    public void deinit() {
        if (mService != null) {
            mAdapter.closeProfileProxy(BluetoothProfile.HID_HOST, mService);
        }
    }

    boolean getProtocolMode(BluetoothDevice device) {
        if (mService == null) return false;
        return mService.getProtocolMode(device);
    }

    boolean setProtocolMode(BluetoothDevice device, int mode) {
        if (mService == null) return false;
        return mService.setProtocolMode(device, mode);
    }

    boolean virtualUnplug(BluetoothDevice device) {
        if (mService == null) return false;
        return mService.virtualUnplug(device);
    }

    boolean getReport(BluetoothDevice device, byte reportType, byte reportId, int bufferSize) {
        if (mService == null) return false;
        return mService.getReport(device, reportType, reportId, bufferSize);
    }

    boolean setReport(BluetoothDevice device, byte reportType, String report) {
        if (mService == null) return false;
        return mService.setReport(device, reportType, report);
    }

    boolean sendData(BluetoothDevice device, String report) {
        if (mService == null) return false;
        return mService.sendData(device, report);
    }

    boolean getIdleTime(BluetoothDevice device) {
        if (mService == null) return false;
        return mService.getIdleTime(device);
    }

    boolean setIdleTime(BluetoothDevice device, byte idleTime) {
        if (mService == null) return false;
        return mService.setIdleTime(device, idleTime);
    }

    private final ServiceListener mServiceListener = new ServiceListener() {

        @Override
        public void onServiceConnected(int profile, BluetoothProfile proxy) {
            if (profile != BluetoothProfile.HID_HOST) return;

            Log.i(TAG, "Profile proxy connected");

            mService = (BluetoothHidHost) proxy;

        }

        @Override
        public void onServiceDisconnected(int profile) {
            if (profile != BluetoothProfile.HID_HOST) return;

            Log.i(TAG, "Profile proxy disconnected");

            mService = null;
        }
    };
}
