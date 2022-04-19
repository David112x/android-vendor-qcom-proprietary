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

package com.android.bluetooth.ba;

import android.app.Service;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattServerCallback;

import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertiseData;
import android.bluetooth.le.AdvertiseSettings;
import android.bluetooth.le.AdvertisingSet;
import android.bluetooth.le.AdvertisingSetCallback;
import android.bluetooth.le.AdvertisingSetParameters;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.BluetoothLeAdvertiser;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanRecord;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;

import com.android.bluetooth.ba.BluetoothBAEncryptionKey;
import com.android.bluetooth.ba.BluetoothBAStreamServiceRecord;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import android.os.Handler;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.os.Message;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.RemoteException;
import android.os.Binder;
import android.util.Log;

import com.android.internal.util.IState;
import com.android.internal.util.State;
import com.android.internal.util.StateMachine;
import com.android.bluetooth.btservice.ProfileService;

import java.util.List;
import java.util.UUID;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Map;
import java.util.concurrent.CountDownLatch;

public class GattBroadcastService  {
    private static final String TAG = "GattBroadcastService";

    private final UUID GATT_BROADCAST_SERVICE_UUID = UUID.fromString
            ("0000FE06-0000-1000-8000-00805F9B34FB");
    private final UUID BROADCAST_VERSION_UUID = UUID.fromString
            ("0000BCA4-d102-11e1-9b23-00025b00a5a5");
    private final UUID BROADCAST_ADDRESS_UUID = UUID.fromString
            ("0000BCA7-d102-11e1-9b23-00025b00a5a5");
    private final UUID BROADCAST_STATUS_UUID = UUID.fromString
            ("0000BCA5-d102-11e1-9b23-00025b00a5a5");
    private final UUID BROADCAST_SECKEY_UUID = UUID.fromString
            ("0000BCAC-d102-11e1-9b23-00025b00a5a5");
    private final UUID BROADCAST_STREAM_SERVICE_RECORDS_UUID = UUID.fromString
            ("0000BCA6-d102-11e1-9b23-00025b00a5a5");
    private final UUID BROADCAST_IDENTIFIER_UUID = UUID.fromString
            ("0000BCA8-d102-11e1-9b23-00025b00a5a5");

    // Status Codes
    private final int BRA_ENABLED_SUCCESS = 0;
    private final int BRA_ENABLED_FAILED = 1;
    private final int BRA_DISABLED_SUCCESS = 2;
    private final int BRA_DISABLED_FAILED = 3;
    private final int ASSOCIATE_BCA_RECEIVER_SUCCESS = 4;
    private final int ASSOCIATE_BCA_RECEIVER_FAILED = 5;

    private final int FETCH_OWN_RANDOM_ADDRESS = 0;
    private final int NOTIFY_CB_ASSOCIATE_BCA_RECEIVER = 1;
    private final int NOTIFY_CB_CONFIGURE_BRA = 2;
    private final int NOTIFY_CB_ONLOST_ONFOUND_RECEIVER = 3;
    private final int HANDLER_ARG_NOT_USED = -1;
    private final int LE_DEVICE_NAME_MAX_LENGTH = 15;

    private BluetoothManager mBluetoothManager;
    private BATService mBATService;
    private BleScanner mBleScanner;
    private BleAdvertiser mBleAdvertiser;
    private BluetoothAdapter mBluetoothAdapter;
    private GattBroadcastServiceReceiver mReceiver;
    private GattBroadcastServiceStateMachine mStateMachine;
    private boolean mIsBAPairing = false;
    private BluetoothDevice mDevice = null;
    private Context mContext;
    private GattBroadcastServiceMessageHandler mSessionHandler = null;

    private void configureBroadcastReceiverAssociation(boolean enable) {
        Log.i(TAG, "configureBroadcastReceiverAssociation :" + enable);
        //Do not forward BRA Enable/Disable request to state machine, if BT is not on.
        if (isBluetoothLeOn() && mStateMachine != null) {
            mStateMachine.sendMessage(GattBroadcastServiceStateMachine
                    .CONFIGURE_BROADCAST_RECEIVER_ASSOCIATION, enable);
        } else if (mSessionHandler != null) {
            int status = enable ? BRA_ENABLED_FAILED: BRA_DISABLED_SUCCESS;
            mSessionHandler.sendMessage(
                    mSessionHandler.obtainMessage(NOTIFY_CB_CONFIGURE_BRA, status));
        } else {
            Log.i(TAG, "mSessionHandler is null");
        }
    }

    private void associateBCAReceiver(BluetoothDevice device) {
        Log.i(TAG, "associateBCAReceiver :" + device);
         //Do not forward associate BCA Receiver request to state machine, if BT is not on.
        if (isBluetoothLeOn() && mStateMachine != null) {
            mStateMachine.sendMessage(GattBroadcastServiceStateMachine
                    .ASSOCIATE_BCA_RECEIVER, device);
        } else if (mSessionHandler != null) {
            mSessionHandler.sendMessage(
                    mSessionHandler.obtainMessage(NOTIFY_CB_ASSOCIATE_BCA_RECEIVER,
                            ASSOCIATE_BCA_RECEIVER_FAILED, HANDLER_ARG_NOT_USED, device));
        } else {
            Log.i(TAG, "mSessionHandler is null");
        }
    }

    public void start(Context context) {

        mContext = context;
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        mBluetoothManager = (BluetoothManager) mContext.getSystemService(Context.BLUETOOTH_SERVICE);
        mBleScanner = new BleScanner();
        mBleAdvertiser = new BleAdvertiser();

        HandlerThread thread = new HandlerThread("BluetoothGattBroadcastServiceHandler");
        thread.start();
        Looper looper = thread.getLooper();

        mSessionHandler = new GattBroadcastServiceMessageHandler(looper);
        mBATService = BATService.getBATService();
        mStateMachine = new GattBroadcastServiceStateMachine();
        mStateMachine.start();
        mReceiver = new GattBroadcastServiceReceiver();

        IntentFilter filter = new IntentFilter(BATService.ACTION_BAT_STATE_CHANGED);
        filter.addAction(BATService.ACTION_BAT_ENCRYPTION_KEY_CHANGED);
        filter.addAction(BATService.ACTION_BAT_DIV_CHANGED);
        filter.addAction(BATService.ACTION_BAT_STREAMING_ID_CHANGED);
        filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
        filter.addAction(BluetoothDevice.ACTION_ACL_DISCONNECTED);
        filter.addAction(BATService.ACTION_BAT_BRA_ENABLE);
        filter.addAction(BATService.ACTION_BAT_BRA_DISABLE);
        filter.addAction(BATService.ACTION_BAT_ASSOCIATE_BCA_RECEIVER);
        mContext.registerReceiver(mReceiver, filter);
    }

    public void stop() {
        Log.i(TAG, "Stop");
        if (mStateMachine != null) {
            mStateMachine.doQuit();
        }

        if (mSessionHandler != null) {
            // Perform cleanup in Handler running on Worker Thread
            mSessionHandler.removeCallbacksAndMessages(null);
            Looper looper = mSessionHandler.getLooper();
            if (looper != null) {
                looper.quit();
                Log.i(TAG, "Quit looper");
            }
            mSessionHandler = null;
            Log.i(TAG, "Remove Handler");
        }

        if (mReceiver != null) {
            mContext.unregisterReceiver(mReceiver);
        }
    }

    private boolean isBluetoothLeOn() {
        if (mBluetoothAdapter != null) {
            if (mBluetoothAdapter.isLeEnabled()) {
                Log.i(TAG, "isBluetoothLeOn: true");
                return true;
            } else {
                Log.i(TAG, "Bluetooth LE state " + mBluetoothAdapter.getLeState());
            }
        } else {
            Log.i(TAG, " mBluetoothAdapter is null");
        }
        return false;
    }


    private final class GattBroadcastServiceMessageHandler extends Handler {
        private static final String TAG = "GattBroadcastServiceMessageHandler";

        private GattBroadcastServiceMessageHandler(Looper looper) {
            super(looper);
            Log.i(TAG, "GattBroadcastServiceMessageHandler ");
        }

        @Override
        public void handleMessage(Message msg) {
            Log.i(TAG, "Handler(): got msg=" + msg.what);
            int status;
            BluetoothDevice dev;
            switch (msg.what) {
                case FETCH_OWN_RANDOM_ADDRESS:
                    if (mBleAdvertiser.mAdvertisingSet != null) {
                        mBleAdvertiser.mAdvertisingSet.getOwnAddress();
                    }
                    break;
                case NOTIFY_CB_ASSOCIATE_BCA_RECEIVER:
                    status = msg.arg1;
                    dev = (BluetoothDevice) msg.obj;
                    Intent notify_on_associated_BCA_Receiver = new Intent(
                            BATService.ACTION_BAT_ON_ASSOCIATED_BCA_RECEIVER);
                    notify_on_associated_BCA_Receiver.putExtra(BATService.EXTRA_STATUS, status);
                    notify_on_associated_BCA_Receiver.putExtra(BluetoothDevice.EXTRA_DEVICE, dev);
                    mContext.sendBroadcast(notify_on_associated_BCA_Receiver,
                            BATService.BLUETOOTH_PERM_ADMIN);
                    break;
                case NOTIFY_CB_CONFIGURE_BRA:
                    status = msg.arg1;
                    Intent notify_configure_bra = new Intent(
                            BATService.ACTION_BAT_BRA_STATE_CHANGED);
                    notify_configure_bra.putExtra(BATService.EXTRA_STATUS, status);
                    mContext.sendBroadcast(notify_configure_bra,
                            BATService.BLUETOOTH_PERM_ADMIN);
                    break;
                case NOTIFY_CB_ONLOST_ONFOUND_RECEIVER:
                    boolean isFound;
                    int callBackType = msg.arg1;
                    ScanResult result = (ScanResult) msg.obj;

                    if (callBackType == ScanSettings.CALLBACK_TYPE_FIRST_MATCH) {
                        isFound = true;
                    } else if (callBackType == ScanSettings.CALLBACK_TYPE_MATCH_LOST) {
                        isFound = false;
                    } else {
                        return;
                    }

                    Intent notify_onlost_onfound_receiver = new Intent(
                            BATService.ACTION_BAT_ONFOUND_ONLOST_BCA_RECEIVER);
                    notify_onlost_onfound_receiver.putExtra(BATService.EXTRA_SCAN_RESULT,
                            result);
                    notify_onlost_onfound_receiver.putExtra(BATService.EXTRA_ONFOUND, isFound);
                    mContext.sendBroadcast(notify_onlost_onfound_receiver,
                            BATService.BLUETOOTH_PERM_ADMIN);
                    break;
                default:
                    break;
            }
        }
    };

    private class BleScanner {
        private static final String TAG = "GattBroadcastService: BleScanner";
        private BluetoothLeScanner mScanner;
        private ScanCallback mCallback;
        private boolean mScanning = false;

        private BleScanner() {
            Log.d(TAG, "BleScanner ");
            mCallback = new GattBroadcastReceiverScanCallback();
            mScanner = mBluetoothAdapter.getBluetoothLeScanner();
        }

        public void discoverBroadcastAudioReceiver(boolean enableScan) {
            Log.i(TAG, "discoverBroadcastAudioReceiver mScanner: " + mScanner
                    + " mScanning: " + mScanning + " enableScan: " + enableScan);
            if (isBluetoothLeOn() && mScanner != null) {
                if (enableScan) {
                    if (mScanning == false) {
                        List<ScanFilter> filters = new ArrayList<ScanFilter>();
                        ScanSettings.Builder settingBuilder = new ScanSettings.Builder();
                        filters.add(new ScanFilter.Builder()
                                .setServiceSolicitationUuid(new ParcelUuid(
                                        GATT_BROADCAST_SERVICE_UUID))
                                .build());
                        settingBuilder.setScanMode(ScanSettings.SCAN_MODE_BALANCED);
                        settingBuilder.setCallbackType(ScanSettings.CALLBACK_TYPE_FIRST_MATCH |
                                ScanSettings.CALLBACK_TYPE_MATCH_LOST);
                        mScanner.startScan(filters, settingBuilder.build(), mCallback);
                        mScanning = true;
                    }
                } else {
                    if (mScanning == true) {
                        mScanning = false;
                        mScanner.stopScan(mCallback);
                    }
                }
            }
        }

        private class GattBroadcastReceiverScanCallback extends ScanCallback {
            @Override
            public void onScanResult(int callBackType, ScanResult result) {
                Log.i(TAG, "onScanResult callBackType : " + callBackType);
                if (mSessionHandler != null) {
                    mSessionHandler.sendMessage(
                            mSessionHandler.obtainMessage(NOTIFY_CB_ONLOST_ONFOUND_RECEIVER,
                            callBackType, HANDLER_ARG_NOT_USED, result));
                 } else {
                    Log.e(TAG, "onScanResult mSessionHandler is null");
                 }
            }

            public void onScanFailed(int errorCode) {
                Log.e(TAG, "Scan fail. Error code: " + new Integer(errorCode).toString());
                mScanning = false;
            }
        }
    }


    private class BleAdvertiser {

        private static final String TAG = "GattBroadcastService: BleAdvertiser";
        private final int DIV_LENGTH = 2;
        public static final int BRA_ENABLED_STREAMING_PAUSED_ADV_INTERVAL = 160;// 100ms
        public static final int BRA_ENABLED_STREAMING_ACTIVE_ADV_INTERVAL = 160;// 100ms
        public static final int BRA_DISABLED_STREAMING_PAUSED_ADV_INTERVAL = 1600;// 1s
        public static final int BRA_DISABLED_STREAMING_ACTIVE_ADV_INTERVAL = 160;// 100ms
        private BluetoothLeAdvertiser mAdvertiser;
        private BluetoothGattServer mGattServer;
        private AdvertisingSetCallback mCallback;
        private int mState;

        //characterstic related object
        private BroadcastStreamServiceRecords mBroadcastStreamServiceRecords;
        private BroadcastIdentifier mBroadcastIdentifier;
        private BroadcastAddress mBroadcastAddress;
        private BroadcastStatus mBroadcastStatus;
        private BroadcastSecurityKey mBroadcastSecurityKey;
        private BroadcastVersion mBroadcastVersion;
        private byte[] mDiv;
        private AdvertisingSet mAdvertisingSet;
        private CountDownLatch mAdvertisingSignal;
        private boolean mIsAdvertising = false;
        private boolean isCharactersticExposed = false;


        /**
         * GATT callbacks
         */
        private final BluetoothGattServerCallback mGattCallbacks = new
            BluetoothGattServerCallback() {

            @Override
            public void onConnectionStateChange(BluetoothDevice device, int status, int newState) {

                Log.i(TAG, "onConnectionStateChange device :" + device + " status :" + status +
                        " newState :" + newState);
                mState = newState;

                if (mDevice == null || device == null) {
                    Log.e(TAG, "onConnectionStateChange:Unexpected error! mstate: " +  mState);
                    return;
                }

                int bondState = mDevice.getBondState();

                if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    int associationStatus = ASSOCIATE_BCA_RECEIVER_SUCCESS;

                    Log.i(TAG, "onConnectionStateChange:DISCONNECTED bond state: "
                            + bondState + " address: " + mDevice.getAddress()
                            + " remoteDevice: " + device.getAddress()
                            + " mIsBAPairing: " + mIsBAPairing);

                    if (mDevice.equals(device)) {
                        if (mIsBAPairing){
                            if (bondState == BluetoothDevice.BOND_BONDED) {
                                Log.i(TAG, "removing bond");
                                mDevice.removeBond();
                            } else {
                                associationStatus = ASSOCIATE_BCA_RECEIVER_FAILED;
                            }
                            mIsBAPairing = false;
                        } else {
                            if (bondState != BluetoothDevice.BOND_BONDED) {
                                 associationStatus = ASSOCIATE_BCA_RECEIVER_FAILED;
                            }
                        }

                        if (mSessionHandler != null) {
                            mSessionHandler.sendMessage(
                                mSessionHandler.obtainMessage(NOTIFY_CB_ASSOCIATE_BCA_RECEIVER,
                                associationStatus, HANDLER_ARG_NOT_USED, device));
                        } else {
                            Log.e(TAG, "onConnectionStateChange mSessionHandler is null");
                        }
                    }

                } else if (newState == BluetoothProfile.STATE_CONNECTED) {
                    Log.i(TAG, "onConnectionStateChange:CONNECTED bond state: "
                            + bondState + " address: " + mDevice.getAddress()
                            + " remoteDevice: " + device.getAddress());
                    if (mDevice.equals(device)) {
                        if (bondState == BluetoothDevice.BOND_BONDED) {
                            mIsBAPairing = false;
                        } else {
                            mIsBAPairing = true;
                        }
                    }
                }
            }

            @Override
            public void onCharacteristicReadRequest(BluetoothDevice device, int requestId,
                    int offset, BluetoothGattCharacteristic characteristic) {

                if (mState == BluetoothProfile.STATE_CONNECTED) {
                    UUID uuid = characteristic.getUuid();
                    byte[] value = {0};
                    int status = 0;

                    if (uuid == BROADCAST_VERSION_UUID) {
                        value = mBroadcastVersion.getBroadcastVersion();
                    } else if (uuid == BROADCAST_IDENTIFIER_UUID) {
                        value = mBroadcastIdentifier.getBroadcastIdentifier();
                    } else if (uuid == BROADCAST_STATUS_UUID) {
                        value = mBroadcastStatus.getBroadcastStatus();
                    } else if (uuid == BROADCAST_SECKEY_UUID) {
                        value = mBroadcastSecurityKey.getBroadcastSecuritykey();
                    } else if (uuid == BROADCAST_ADDRESS_UUID) {
                        value = mBroadcastAddress.getBroadcastAddress();
                    } else if (uuid == BROADCAST_STREAM_SERVICE_RECORDS_UUID) {
                        byte[] temp = mBroadcastStreamServiceRecords
                                .getBroadcastStreamServiceRecords();
                        value = java.util.Arrays.copyOfRange(temp, offset, temp.length);
                    } else {
                        return;
                    }

                    if (uuid != null)
                        Log.i(TAG, Arrays.toString(value) + " UUID " + uuid.toString() + offset);
                    if (mGattServer != null) {
                        mGattServer.sendResponse(device, requestId, status, offset, value);
                    }
                }

            }

            @Override
            public void onServiceAdded(final int status, BluetoothGattService service) {
                Log.i(TAG, "Service added");
            }
        };

        private BleAdvertiser() {
            Log.i(TAG, "BleAdvertiser");
            mCallback = new GattBroadcastAdvertiseCallback();
            mAdvertiser = mBluetoothAdapter.getBluetoothLeAdvertiser();

            //Initialize characterstic objects
            mBroadcastVersion = new BroadcastVersion();
            mBroadcastIdentifier = new BroadcastIdentifier();
            mBroadcastStatus = new BroadcastStatus();
            mBroadcastSecurityKey = new BroadcastSecurityKey();
            mBroadcastAddress = new BroadcastAddress();
            mBroadcastStreamServiceRecords = new BroadcastStreamServiceRecords();
            mDiv = new byte[DIV_LENGTH];
        }

        public byte[] getDiv() {
            Log.i(TAG, "getDiv " + Arrays.toString(mDiv));
            return mDiv;
        }

        public void setDiv(int div) {
            mDiv[1] = (byte) div;
            mDiv[0] = (byte) ((div >> 8) & 0xFF);
        }

        public void startAdvertising(boolean isConnectable, int interval) {
            Log.i(TAG, "startAdvertising:");
            if (isBluetoothLeOn() == false) return;

            AdvertiseData data = generateAdvertiseData(GATT_BROADCAST_SERVICE_UUID,
                    getDiv());

            AdvertisingSetParameters.Builder parameters = new AdvertisingSetParameters.Builder();
            int duration = 0;

            parameters.setLegacyMode(true);
            parameters.setConnectable(isConnectable);
            parameters.setScannable(true); // legacy advertisements we support are always scannable
            parameters.setInterval(interval);
            parameters.setTxPowerLevel(1);

            if (mIsAdvertising == false) {
                mAdvertisingSignal = new CountDownLatch(1);
                mAdvertiser.startAdvertisingSet(parameters.build(), data, null, null, null,
                        duration, 0, mCallback);
                mIsAdvertising = true;
                Log.i(TAG, "Waiting to start Advertisement");
                try {
                    mAdvertisingSignal.await();
                } catch (InterruptedException e) {
                    Log.e(TAG, "Interrupt received while waiting to start Advertisement", e);
                }
            } else {
                Log.i(TAG, "startAdvertising: Advertising is going on");
            }

            if (isConnectable && isCharactersticExposed == false) {
                isCharactersticExposed = openServer();
            }
        }

        public void stopAdvertising() {
            Log.i(TAG, "stopAdvertising: mIsAdvertising: " + mIsAdvertising);
            if (isBluetoothLeOn() == false) return;

            if (mIsAdvertising) {
                mAdvertisingSignal = new CountDownLatch(1);
                mAdvertiser.stopAdvertisingSet(mCallback);
                mIsAdvertising = false;
                Log.i(TAG, "Waiting to stop Advertisement");
                try {
                    mAdvertisingSignal.await();
                } catch (InterruptedException e) {
                    Log.e(TAG, "Interrupt received while waiting to stop Advertisement", e);
                }
            }
        }

        private AdvertiseData generateAdvertiseData(UUID uuid, byte[] data) {
            if (mBluetoothAdapter !=null && mBluetoothAdapter.getName() != null
                && mBluetoothAdapter.getName().length() <= LE_DEVICE_NAME_MAX_LENGTH) {
                return new AdvertiseData.Builder()
                        .addServiceUuid(new ParcelUuid(uuid))
                        .addServiceData(new ParcelUuid(uuid), data)
                        .setIncludeDeviceName(true)
                        .build();
           } else {
                return new AdvertiseData.Builder()
                        .addServiceUuid(new ParcelUuid(uuid))
                        .addServiceData(new ParcelUuid(uuid), data)
                        .build();
           }
        }

        private boolean openServer() {
            Log.i(TAG, "openServer");

            mGattServer = mBluetoothManager.openGattServer(mContext, mGattCallbacks,
                    BluetoothDevice.TRANSPORT_LE);
            if (mGattServer == null) {
                Log.e(TAG, "mBluetoothGattServer is NULL");
                return false;
            }

            BluetoothGattCharacteristic broadcastVersion = new BluetoothGattCharacteristic(
                    BROADCAST_VERSION_UUID,
                    BluetoothGattCharacteristic.PROPERTY_READ,
                    BluetoothGattCharacteristic.PERMISSION_READ);

            BluetoothGattCharacteristic broadcastAddress = new BluetoothGattCharacteristic(
                    BROADCAST_ADDRESS_UUID,
                    BluetoothGattCharacteristic.PROPERTY_READ,
                    BluetoothGattCharacteristic.PERMISSION_READ);

            BluetoothGattCharacteristic broadcastStatus = new BluetoothGattCharacteristic(
                    BROADCAST_STATUS_UUID,
                    BluetoothGattCharacteristic.PROPERTY_READ,
                    BluetoothGattCharacteristic.PERMISSION_READ);

            BluetoothGattCharacteristic broadcastSecKey = new BluetoothGattCharacteristic(
                    BROADCAST_SECKEY_UUID,
                    BluetoothGattCharacteristic.PROPERTY_READ,
                    BluetoothGattCharacteristic.PERMISSION_READ);

            BluetoothGattCharacteristic broadcastStreamServiceRecords = new
                    BluetoothGattCharacteristic(
                    BROADCAST_STREAM_SERVICE_RECORDS_UUID,
                    BluetoothGattCharacteristic.PROPERTY_READ,
                    BluetoothGattCharacteristic.PERMISSION_READ);

            BluetoothGattCharacteristic broadcastIdentifier = new BluetoothGattCharacteristic(
                    BROADCAST_IDENTIFIER_UUID,
                    BluetoothGattCharacteristic.PROPERTY_READ,
                    BluetoothGattCharacteristic.PERMISSION_READ);

            BluetoothGattService gattBroadcastService = new BluetoothGattService(
                    GATT_BROADCAST_SERVICE_UUID, BluetoothGattService.SERVICE_TYPE_PRIMARY);

            gattBroadcastService.addCharacteristic(broadcastVersion);
            gattBroadcastService.addCharacteristic(broadcastAddress);
            gattBroadcastService.addCharacteristic(broadcastStatus);
            gattBroadcastService.addCharacteristic(broadcastSecKey);
            gattBroadcastService.addCharacteristic(broadcastStreamServiceRecords);
            gattBroadcastService.addCharacteristic(broadcastIdentifier);

            mGattServer.addService(gattBroadcastService);
            return true;
        }

        private void closeServer() {
            Log.i(TAG, "closeServer isCharactersticExposed: " + isCharactersticExposed);
            if (isCharactersticExposed) {
                isCharactersticExposed = false;
            } else {
                return;
            }
            if (mGattServer != null) {
                if (mDevice != null) mGattServer.cancelConnection(mDevice);
                mGattServer.close();
            }
        }

        public void connectDevice(final BluetoothDevice device) {
            mGattServer.connect(device, false);
            mDevice = device;
        }

        private class GattBroadcastAdvertiseCallback extends AdvertisingSetCallback {

            @Override
            public void onAdvertisingSetStarted(AdvertisingSet advertisingSet, int txPower,
                                                int status) {
                Log.i(TAG, "onAdvertisingSetStarted status " + status
                        + " advertisingSet: " + advertisingSet + " txPower " + txPower);
                if (status == BluetoothGatt.GATT_SUCCESS) {
                    mAdvertisingSet = advertisingSet;
                } else {
                    mIsAdvertising = false;
                }
                mAdvertisingSignal.countDown();
            }

            @Override
            public void onAdvertisingSetStopped(AdvertisingSet advertisingSet) {
                Log.i(TAG, "onAdvertisingSetStopped advertisingSet: " + advertisingSet);
                mAdvertisingSignal.countDown();
            }

            @Override
            public void onAdvertisingEnabled(AdvertisingSet advertisingSet, boolean enable,
                                             int status) {
                Log.i(TAG, "onAdvertisingEnabled advertisingSet: " + advertisingSet
                        + " status " + status + " enable: " + enable);
            }

            @Override
            public void onAdvertisingDataSet(AdvertisingSet advertisingSet, int status) {
                Log.i(TAG, "onAdvertisingDataSet advertisingSet: " + advertisingSet
                        + " status " + status);
            }

            public void onAdvertisingParametersUpdated(AdvertisingSet advertisingSet,
                                                       int txPower, int status) {
                Log.i(TAG, "onAdvertisingParametersUpdated  advertisingSet: " + advertisingSet
                        + " status " + status  + " txPower " + txPower);
            }

            @Override
            public void onOwnAddressRead(AdvertisingSet advertisingSet, int addressType,
                                         String address) {
                Log.i(TAG, "onOwnAddressRead advertisingSet: " + advertisingSet
                        + " address " + address + " addressType " + addressType);
            }

        }

        private class BroadcastVersion {
            public static final int MAJOR_VERSION_OFFSET = 0;
            public static final int MINOR_VERSION_OFFSET = 1;
            private byte mVersionMajor;
            private byte mVersionMinor;
            private byte[] mVersion;

            private BroadcastVersion() {
                mVersionMajor = 0x03;
                mVersionMinor = 0x00;
                mVersion = new byte[2];
            }

            public byte getMajorVersion() {
                return mVersionMajor;
            }

            public byte getMinorVersion() {
                return mVersionMinor;
            }

            public byte[] getBroadcastVersion() {

                mVersion[MAJOR_VERSION_OFFSET] = getMajorVersion();
                mVersion[MINOR_VERSION_OFFSET] = getMinorVersion();
                return mVersion;
            }

        }

        private class BroadcastAddress {
            public static final int BROADCAST_ADDRESS_LENGTH = 7;
            private byte[] mAddress;

            private BroadcastAddress() {
                mAddress = new byte[BROADCAST_ADDRESS_LENGTH];
                mAddress[0] = 0x00;
                if (mBluetoothAdapter != null) {
                    String bdAddr = mBluetoothAdapter.getAddress();
                    if (bdAddr != null) {
                        setBroadcastAddress(bdAddr);
                    } else {
                        Log.e(TAG, "Adapter address is null");
                    }
                }
            }

            public byte[] strToBdAddr(String address) {
                byte[] addr = new byte[6];
                int i = 0;
                String[] words = address.split(":");
                for (String w : words) {
                    addr[i++] = (byte) Integer.parseInt(w, 16);
                }
                return addr;
            }

            public byte[] getBroadcastAddress() {
                return mAddress;
            }

            public void setBroadcastAddress(String address) {
                Log.i(TAG, "BroadcastAddress string " + address);
                byte[] addr = strToBdAddr(address);
                mAddress[1] = addr[3];
                mAddress[2] = addr[4];
                mAddress[3] = addr[5];
                mAddress[4] = addr[2];
                mAddress[5] = addr[0];
                mAddress[6] = addr[1];
                Log.i(TAG, "**BroadcastAddress bytes" + Arrays.toString(mAddress));
            }
        }

        private class BroadcastStatus {
            private final byte mReserved = 0x00;
            private final int RESERVED_OFFSET = 0;
            private final int ACTIVE_STREAM_ID_OFFSET = 1;
            private final int BROADCAST_STATUS_LENGTH = 2;
            private byte mActiveStreamId;
            private byte[] mBroadcastStatus;

            private BroadcastStatus() {
                mActiveStreamId = 0x00;
                mBroadcastStatus = new byte[BROADCAST_STATUS_LENGTH];
            }

            public byte getReserved() {
                return mReserved;
            }

            public byte getActiveStreamId() {
                return mActiveStreamId;
            }

            public void setActiveStreamId(byte activeStreamId) {
                mActiveStreamId = activeStreamId;
            }

            public byte[] getBroadcastStatus() {
                mBroadcastStatus[RESERVED_OFFSET] = getReserved();
                mBroadcastStatus[ACTIVE_STREAM_ID_OFFSET] = getActiveStreamId();

                return mBroadcastStatus;
            }
        }

        private class BroadcastSecurityKey {
            public static final int RESERVED_OFFSET = 0;
            public static final int RESERVED_LENGTH = 1;
            public static final int ENCRYPTION_KEY_TYPE_OFFSET = 1;
            public static final int ENCRYPTION_KEY_OFFSET = 2;
            public final int ENCRYPTION_KEY_TYPE_LENGTH = 1;
            public final int ENCRYPTION_KEY_LENGTH = BluetoothBAEncryptionKey.ENCRYPTION_KEY_LENGTH;
            private final byte mReserved = 0x02;
            private byte[] mEncryptionKeyType;
            private byte[] mEncryptionKey;
            private byte[] mBroadcastSecurityKey;

            private BroadcastSecurityKey() {
                mEncryptionKeyType = new byte[ENCRYPTION_KEY_TYPE_LENGTH];
                mEncryptionKey = new byte[ENCRYPTION_KEY_LENGTH];
                mBroadcastSecurityKey = new byte[RESERVED_LENGTH +
                        ENCRYPTION_KEY_TYPE_LENGTH + ENCRYPTION_KEY_LENGTH];
            }

            public byte getReserved() {
                return mReserved;
            }

            public byte[] getEncryptionKeyType() {
                return mEncryptionKeyType;
            }

            public void setEncryptionKeyType(int encryptionKeyType) {
                mEncryptionKeyType[0] = (byte) encryptionKeyType;
            }

            public byte[] getEncryptionKey() {
                return mEncryptionKey;
            }

            public void setEncryptionKey(byte[] encryptionKey) {
                System.arraycopy(encryptionKey, 0, mEncryptionKey, 0,
                        BluetoothBAEncryptionKey.ENCRYPTION_KEY_LENGTH);
                Log.i(TAG, "setEncryptionKey " + Arrays.toString(mEncryptionKey));
            }

            public byte[] getBroadcastSecuritykey() {
                mBroadcastSecurityKey[RESERVED_OFFSET] = getReserved();
                System.arraycopy(getEncryptionKeyType(), 0,
                        mBroadcastSecurityKey, ENCRYPTION_KEY_TYPE_OFFSET,
                        ENCRYPTION_KEY_TYPE_LENGTH);
                System.arraycopy(getEncryptionKey(), 0,
                        mBroadcastSecurityKey, ENCRYPTION_KEY_OFFSET,
                        ENCRYPTION_KEY_LENGTH);
                return mBroadcastSecurityKey;
            }
        }

        private class BroadcastStreamServiceRecords {
            public static final int BSSR_TAG_ID_LEN = 1;
            public static final int BSSR_LENGTH_LEN = 1;
            public static final int BSSR_STREAM_ID_LEN = 1;
            public static final int BSSR_STREAM_ID_OFFSET = BSSR_TAG_ID_LEN + BSSR_STREAM_ID_LEN;
            public static final int BSSR_TYPE_SECURITY_ID_OFFSET
                    = BSSR_STREAM_ID_OFFSET + BSSR_STREAM_ID_LEN;
            public static final int BSSR_TYPE_CODEC_TYPE_ID_OFFSET
                    = BSSR_TYPE_SECURITY_ID_OFFSET + BSSR_TAG_ID_LEN + BSSR_LENGTH_LEN
                    + BluetoothBAStreamServiceRecord.BSSR_TYPE_SECURITY_ID_LEN;
            public static final int BSSR_TYPE_CODEC_CONFIG_CELT_ID_OFFSET
                    = BSSR_TYPE_CODEC_TYPE_ID_OFFSET + BSSR_TAG_ID_LEN + BSSR_LENGTH_LEN
                    + BluetoothBAStreamServiceRecord.BSSR_TYPE_CODEC_TYPE_ID_LEN;
            public static final int BSSR_TYPE_ERASURE_CODE_ID_OFFSET
                    = BSSR_TYPE_CODEC_CONFIG_CELT_ID_OFFSET + BSSR_TAG_ID_LEN + BSSR_LENGTH_LEN
                    + BluetoothBAStreamServiceRecord.BSSR_TYPE_CODEC_CONFIG_CELT_ID_LEN;
            public static final int BSSR_TYPE_CHANNELS_ID_OFFSET
                    = BSSR_TYPE_ERASURE_CODE_ID_OFFSET + BSSR_TAG_ID_LEN + BSSR_LENGTH_LEN
                    + BluetoothBAStreamServiceRecord.BSSR_TYPE_ERASURE_CODE_ID_LEN;
            public static final int BSSR_TYPE_SAMPLE_SIZE_ID_OFFSET
                    = BSSR_TYPE_CHANNELS_ID_OFFSET + BSSR_TAG_ID_LEN + BSSR_LENGTH_LEN
                    + BluetoothBAStreamServiceRecord.BSSR_TYPE_CHANNELS_ID_LEN;
            public static final int BSSR_TYPE_AFH_UPDATE_METHOD_ID_OFFSET
                    = BSSR_TYPE_SAMPLE_SIZE_ID_OFFSET + BSSR_TAG_ID_LEN + BSSR_LENGTH_LEN
                    + BluetoothBAStreamServiceRecord.BSSR_TYPE_SAMPLE_SIZE_ID_LEN;
            public static final int BSSR_TOTAL_LEN
                    = BSSR_TYPE_AFH_UPDATE_METHOD_ID_OFFSET + BSSR_TAG_ID_LEN + BSSR_LENGTH_LEN
                    + BluetoothBAStreamServiceRecord.BSSR_TYPE_AFH_UPDATE_METHOD_ID_LEN;

            //celt related offset
            public static final int BSSR_TYPE_CODEC_CONFIG_CELT_FREQ_ID_OFFSET
                    = BSSR_TYPE_CODEC_CONFIG_CELT_ID_OFFSET + BSSR_TAG_ID_LEN + BSSR_TAG_ID_LEN;
            public static final int BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SIZE_ID_OFFSET
                    = BSSR_TYPE_CODEC_CONFIG_CELT_FREQ_ID_OFFSET
                    + BluetoothBAStreamServiceRecord.BSSR_TYPE_CODEC_CONFIG_CELT_FREQ_ID_LEN;
            public static final int BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SAMPLES_ID_OFFSET
                    = BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SIZE_ID_OFFSET
                    + BluetoothBAStreamServiceRecord.BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SIZE_ID_LEN;
            private byte[] mBroadcastStreamServiceRecords;
            private boolean mIsRecordSet = false;

            BroadcastStreamServiceRecords() {
            }

            public byte[] getBroadcastStreamServiceRecords() {
                return mBroadcastStreamServiceRecords;
            }

            public void setBroadcastStreamServiceRecords(BluetoothBAStreamServiceRecord record) {
                if (record != null && mIsRecordSet == false) {
                    Long[] streamIDs = record.getStreamIds();
                    int start_offset_of_cur_rec;
                    int numofRecords = streamIDs.length;

                    if (mBroadcastStreamServiceRecords == null)
                        mBroadcastStreamServiceRecords = new byte[numofRecords * BSSR_TOTAL_LEN];

                    if (streamIDs.length != 0) {
                        for (int i = 0; i < streamIDs.length; i++) {
                            start_offset_of_cur_rec = i * BSSR_TOTAL_LEN;

                            Map<Integer, Long> mServiceRecordData = record.getServiceRecord
                                    (streamIDs[i]);
                            for (Map.Entry<Integer, Long> entry : mServiceRecordData.entrySet()) {

                                Log.i(TAG, " Key< " + entry.getKey() + " >" + " value <"
                                        + entry.getValue() + ">");

                                if (entry.getKey() == BluetoothBAStreamServiceRecord
                                        .BSSR_TYPE_SECURITY_ID) {
                                    long entryValue = entry.getValue().longValue();
                                    byte[] secValue = new byte[BluetoothBAStreamServiceRecord
                                            .BSSR_TYPE_SECURITY_ID_LEN];

                                    secValue[1] = (byte) entryValue;
                                    secValue[0] = (byte) ((entryValue >> 8) & 0xFF);

                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec +
                                            BSSR_TYPE_SECURITY_ID_OFFSET]
                                            = (byte) BluetoothBAStreamServiceRecord
                                                .BSSR_TYPE_SECURITY_ID;
                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec +
                                            BSSR_TYPE_SECURITY_ID_OFFSET + BSSR_TAG_ID_LEN]
                                            = (byte) BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_SECURITY_ID_LEN;
                                    System.arraycopy(secValue, 0, mBroadcastStreamServiceRecords,
                                            start_offset_of_cur_rec + BSSR_TYPE_SECURITY_ID_OFFSET
                                                    + BSSR_TAG_ID_LEN + BSSR_LENGTH_LEN,
                                            BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_SECURITY_ID_LEN);
                                } else if (entry.getKey() == BluetoothBAStreamServiceRecord
                                        .BSSR_TYPE_CODEC_TYPE_ID) {
                                    long entryValue = entry.getValue().longValue();
                                    byte[] codecValue = new byte[BluetoothBAStreamServiceRecord
                                            .BSSR_TYPE_CODEC_TYPE_ID_LEN];

                                    codecValue[0] = (byte) entryValue;

                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec +
                                            BSSR_TYPE_CODEC_TYPE_ID_OFFSET]
                                            = (byte) BluetoothBAStreamServiceRecord
                                                .BSSR_TYPE_CODEC_TYPE_ID;
                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec +
                                            BSSR_TYPE_CODEC_TYPE_ID_OFFSET + BSSR_TAG_ID_LEN]
                                            = (byte) BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_CODEC_TYPE_ID_LEN;
                                    System.arraycopy(codecValue, 0, mBroadcastStreamServiceRecords,
                                            start_offset_of_cur_rec + BSSR_TYPE_CODEC_TYPE_ID_OFFSET
                                                    + BSSR_TAG_ID_LEN + BSSR_LENGTH_LEN,
                                            BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_CODEC_TYPE_ID_LEN);
                                } else if (entry.getKey() == BluetoothBAStreamServiceRecord
                                        .BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SIZE_ID) {
                                    long entryValue = entry.getValue().longValue();
                                    byte[] configCeltFrameSize
                                            = new byte[BluetoothBAStreamServiceRecord
                                            .BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SIZE_ID_LEN];

                                    configCeltFrameSize[1] = (byte) entryValue;
                                    configCeltFrameSize[0] = (byte) ((entryValue >> 8) & 0xFF);

                                    System.arraycopy(configCeltFrameSize, 0,
                                            mBroadcastStreamServiceRecords,
                                            start_offset_of_cur_rec +
                                            BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SIZE_ID_OFFSET,
                                            BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SIZE_ID_LEN);

                                } else if (entry.getKey() == BluetoothBAStreamServiceRecord
                                        .BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SAMPLES_ID) {
                                    long entryValue = entry.getValue().longValue();
                                    byte[] configCeltFrameSamples
                                            = new byte[BluetoothBAStreamServiceRecord
                                                .BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SIZE_ID_LEN];

                                    configCeltFrameSamples[1] = (byte) entryValue;
                                    configCeltFrameSamples[0] = (byte) ((entryValue >> 8) & 0xFF);

                                    System.arraycopy(configCeltFrameSamples, 0,
                                            mBroadcastStreamServiceRecords,
                                            start_offset_of_cur_rec +
                                            BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SAMPLES_ID_OFFSET,
                                            BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SIZE_ID_LEN);

                                } else if (entry.getKey() == BluetoothBAStreamServiceRecord
                                        .BSSR_TYPE_CODEC_CONFIG_CELT_FREQ_ID) {
                                    long entryValue = entry.getValue().longValue();
                                    byte[] configCeltFreqValue
                                            = new byte[BluetoothBAStreamServiceRecord
                                            .BSSR_TYPE_CODEC_CONFIG_CELT_FREQ_ID];

                                    configCeltFreqValue[1] = (byte) entryValue;
                                    configCeltFreqValue[0] = (byte) ((entryValue >> 8) & 0xFF);

                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec +
                                            BSSR_TYPE_CODEC_CONFIG_CELT_ID_OFFSET]
                                            = (byte) BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_CODEC_CONFIG_CELT_ID;
                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec +
                                            BSSR_TYPE_CODEC_CONFIG_CELT_ID_OFFSET
                                            + BSSR_TAG_ID_LEN] = (byte)
                                            BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_CODEC_CONFIG_CELT_ID_LEN;
                                    System.arraycopy(configCeltFreqValue, 0,
                                            mBroadcastStreamServiceRecords,
                                            start_offset_of_cur_rec +
                                                    BSSR_TYPE_CODEC_CONFIG_CELT_ID_OFFSET
                                                    + BSSR_TAG_ID_LEN + BSSR_LENGTH_LEN,
                                            BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_CODEC_CONFIG_CELT_FREQ_ID_LEN);

                                } else if (entry.getKey() == BluetoothBAStreamServiceRecord
                                        .BSSR_TYPE_ERASURE_CODE_ID) {
                                    long entryValue = entry.getValue().longValue();
                                    byte[] erasureCodeValue
                                            = new byte[BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_ERASURE_CODE_ID_LEN];

                                    erasureCodeValue[0] = (byte) entryValue;
                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec +
                                            BSSR_TYPE_ERASURE_CODE_ID_OFFSET]
                                            = (byte) BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_ERASURE_CODE_ID;
                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec +
                                            BSSR_TYPE_ERASURE_CODE_ID_OFFSET + BSSR_TAG_ID_LEN]
                                            = (byte) BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_ERASURE_CODE_ID_LEN;
                                    System.arraycopy(erasureCodeValue, 0,
                                            mBroadcastStreamServiceRecords, start_offset_of_cur_rec
                                                    + BSSR_TYPE_ERASURE_CODE_ID_OFFSET
                                                    + BSSR_TAG_ID_LEN + BSSR_LENGTH_LEN,
                                            BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_ERASURE_CODE_ID_LEN);
                                } else if (entry.getKey() == BluetoothBAStreamServiceRecord
                                        .BSSR_TYPE_CHANNELS_ID) {
                                    long entryValue = entry.getValue().longValue();
                                    byte[] typeChannelsValue
                                            = new byte[BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_CHANNELS_ID_LEN];

                                    typeChannelsValue[1] = (byte) entryValue;
                                    typeChannelsValue[0] = (byte) ((entryValue >> 8) & 0xFF);

                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec +
                                            BSSR_TYPE_CHANNELS_ID_OFFSET]
                                            = (byte) BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_CHANNELS_ID;
                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec +
                                            BSSR_TYPE_CHANNELS_ID_OFFSET + BSSR_TAG_ID_LEN]
                                            = (byte) BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_CHANNELS_ID_LEN;
                                    System.arraycopy(typeChannelsValue, 0,
                                            mBroadcastStreamServiceRecords,
                                            start_offset_of_cur_rec + BSSR_TYPE_CHANNELS_ID_OFFSET
                                                    + BSSR_TAG_ID_LEN + BSSR_LENGTH_LEN,
                                            BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_CHANNELS_ID_LEN);
                                } else if (entry.getKey() == BluetoothBAStreamServiceRecord
                                        .BSSR_TYPE_SAMPLE_SIZE_ID) {
                                    long entryValue = entry.getValue().longValue();
                                    byte[] sampleSizeValue
                                            = new byte[BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_SAMPLE_SIZE_ID_LEN];

                                    sampleSizeValue[0] = (byte) entryValue;
                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec +
                                            BSSR_TYPE_SAMPLE_SIZE_ID_OFFSET]
                                            = (byte) BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_SAMPLE_SIZE_ID;
                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec +
                                            BSSR_TYPE_SAMPLE_SIZE_ID_OFFSET
                                            + BSSR_TAG_ID_LEN] = (byte)
                                            BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_SAMPLE_SIZE_ID_LEN;
                                    System.arraycopy(sampleSizeValue, 0,
                                            mBroadcastStreamServiceRecords,
                                            start_offset_of_cur_rec +
                                                    BSSR_TYPE_SAMPLE_SIZE_ID_OFFSET
                                                    + BSSR_TAG_ID_LEN + BSSR_LENGTH_LEN,
                                            BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_SAMPLE_SIZE_ID_LEN);
                                } else if (entry.getKey() == BluetoothBAStreamServiceRecord
                                        .BSSR_TYPE_AFH_UPDATE_METHOD_ID) {
                                    long entryValue = entry.getValue().longValue();
                                    byte[] afhUpdateValue
                                            = new byte[BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_AFH_UPDATE_METHOD_ID_LEN];

                                    afhUpdateValue[0] = (byte) entryValue;
                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec +
                                            BSSR_TYPE_AFH_UPDATE_METHOD_ID_OFFSET]
                                            = (byte) BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_AFH_UPDATE_METHOD_ID;
                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec +
                                            BSSR_TYPE_AFH_UPDATE_METHOD_ID_OFFSET
                                            + BSSR_TAG_ID_LEN] = (byte)
                                            BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_AFH_UPDATE_METHOD_ID_LEN;
                                    System.arraycopy(afhUpdateValue, 0,
                                            mBroadcastStreamServiceRecords, start_offset_of_cur_rec
                                                    + BSSR_TYPE_AFH_UPDATE_METHOD_ID_OFFSET
                                                    + BSSR_TAG_ID_LEN + BSSR_LENGTH_LEN,
                                            BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_AFH_UPDATE_METHOD_ID_LEN);
                                } else if (entry.getKey() == BluetoothBAStreamServiceRecord
                                        .BSSR_TYPE_STREAM_ID) {
                                    long entryValue = entry.getValue().longValue();

                                    byte[] streamId = new byte[BluetoothBAStreamServiceRecord
                                            .BSSR_TYPE_STREAM_ID_LEN];
                                    streamId[0] = (byte) entryValue;

                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec]
                                            = (byte) BluetoothBAStreamServiceRecord
                                                    .BSSR_TYPE_STREAM_ID;
                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec +
                                            BSSR_TAG_ID_LEN] = (byte)
                                            BluetoothBAStreamServiceRecord.BSSR_TYPE_STREAM_ID_LEN;
                                    mBroadcastStreamServiceRecords[start_offset_of_cur_rec +
                                            BSSR_STREAM_ID_OFFSET] = streamId[0];

                                }
                            }
                        }
                    }
                    Log.i(TAG, Arrays.toString(mBroadcastStreamServiceRecords));
                    mIsRecordSet = true;
                } else {
                    Log.w(TAG, mIsRecordSet ? "record is already set" :
                            "BroadcastStreamServiceRecords is null");
                }

            }
        }

        private class BroadcastIdentifier {
            private final int BROADCAST_IDENTIFIER_LENGTH = 4;
            byte[] mBroadcastIdentifierValue;

            private BroadcastIdentifier() {
                mBroadcastIdentifierValue = new byte[BROADCAST_IDENTIFIER_LENGTH];
                mBroadcastIdentifierValue[0] = 0x1D;
                mBroadcastIdentifierValue[1] = 0x00;
                mBroadcastIdentifierValue[2] = 0x1E;
                mBroadcastIdentifierValue[3] = 0x00;
            }

            public byte[] getBroadcastIdentifier() {
                return mBroadcastIdentifierValue;
            }
        }

    }

    private class GattBroadcastServiceReceiver extends BroadcastReceiver {
        private static final String TAG = "GattBroadcastServiceReceiver";
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.i(TAG, action);
            if (action.equals(BluetoothDevice.ACTION_BOND_STATE_CHANGED)) {
                BluetoothDevice remoteDevice = intent
                        .getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                int state = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE,
                        BluetoothDevice.ERROR);
                int prevState = intent.getIntExtra(BluetoothDevice.EXTRA_PREVIOUS_BOND_STATE,
                        BluetoothDevice.ERROR);
                if (remoteDevice != null) {
                    Log.i(TAG, "prev bond state: " + prevState + " curr bond state: " + state
                            + " mIsBAPairing: " + mIsBAPairing
                            + " remoteDevice: Address: " + remoteDevice.getAddress());
                } else {
                    Log.e(TAG, "remoteDevice is null");
                    return;
                }

                if (state == BluetoothDevice.BOND_NONE) {
                    if (mDevice != null && mDevice.equals(remoteDevice)) {
                        if (prevState == BluetoothDevice.BOND_BONDING) {
                            //Notify association failure
                            if (mSessionHandler != null) {
                                mSessionHandler.sendMessage(
                                        mSessionHandler.obtainMessage(
                                                NOTIFY_CB_ASSOCIATE_BCA_RECEIVER,
                                                ASSOCIATE_BCA_RECEIVER_FAILED,
                                                HANDLER_ARG_NOT_USED,
                                                remoteDevice));
                            } else {
                                Log.e(TAG, "mSessionHandler is null");
                            }
                        }
                        mIsBAPairing = false;
                    }
                }
            } else if (action.equals(BluetoothDevice.ACTION_ACL_DISCONNECTED)) {
                BluetoothDevice remoteDevice = intent.getParcelableExtra(BluetoothDevice
                        .EXTRA_DEVICE);

                if (mDevice == null || remoteDevice == null) {
                    Log.e(TAG, "Unexpected error!");
                    return;
                }

                Log.i(TAG, "ACL disconnected for " + remoteDevice.getAddress());

                if (mDevice.equals(remoteDevice)) {
                    Log.i(TAG, "bond state: " + mDevice.getBondState()
                            + " mIsBAPairing: " + mIsBAPairing);
                    if (mIsBAPairing){
                        if (mDevice.getBondState() == BluetoothDevice.BOND_BONDED) {
                            Log.i(TAG, "removing bond");
                            mDevice.removeBond();
                        }
                        mIsBAPairing = false;
                    }
                    mDevice = null;
                }
            } else if (action.equals(BATService.ACTION_BAT_BRA_ENABLE)) {
                configureBroadcastReceiverAssociation(true);
            } else if (action.equals(BATService.ACTION_BAT_BRA_DISABLE)) {
                configureBroadcastReceiverAssociation(false);
            } else if (action.equals(BATService.ACTION_BAT_ASSOCIATE_BCA_RECEIVER)) {
                BluetoothDevice device = (BluetoothDevice) intent.getParcelableExtra(
                    BluetoothDevice.EXTRA_DEVICE);
                associateBCAReceiver(device);
            } else {
                if (action.equals(BluetoothAdapter.ACTION_BLE_STATE_CHANGED)
                    || isBluetoothLeOn()) {
                    mStateMachine.sendMessage(GattBroadcastServiceStateMachine.INTENT, intent);
                } else {
                    Log.w(TAG, "ignoring " + action);
                }
            }
        }
    }

    class GattBroadcastServiceStateMachine extends StateMachine {

        public static final int INTENT = 1;
        public static final int CONFIGURE_BROADCAST_RECEIVER_ASSOCIATION = 2;
        public static final int ASSOCIATE_BCA_RECEIVER = 3;

        // Gatt Broadcast Service states.
        private BRADisabledStreamingDisabled mBRADisabledStreamingDisabled;
        private BRADisabledStreamingPaused mBRADisabledStreamingPaused;
        private BRADisabledStreamingActive mBRADisabledStreamingActive;
        private BRAEnabledStreamingPaused mBRAEnabledStreamingPaused;
        private BRAEnabledStreamingActive mBRAEnabledStreamingActive;
        private BluetoothAdapter mAdapter;

        private GattBroadcastServiceStateMachine() {
            super("GattBroadcastServiceStateMachine");
            mAdapter = BluetoothAdapter.getDefaultAdapter();

            mBRADisabledStreamingDisabled = new BRADisabledStreamingDisabled();
            mBRADisabledStreamingPaused = new BRADisabledStreamingPaused();
            mBRADisabledStreamingActive = new BRADisabledStreamingActive();
            mBRAEnabledStreamingPaused = new BRAEnabledStreamingPaused();
            mBRAEnabledStreamingActive = new BRAEnabledStreamingActive();

            addState(mBRADisabledStreamingDisabled);
            addState(mBRADisabledStreamingPaused);
            addState(mBRADisabledStreamingActive);
            addState(mBRAEnabledStreamingPaused);
            addState(mBRAEnabledStreamingActive);
            if (mBATService != null) {
                if (mBATService.getBATState() == BATService.STATE_PLAYING) {
                    setInitialState(mBRADisabledStreamingActive);
                    int div = mBATService.getDIV();
                    Log.d("GattBroadcastServiceStateMachine:", "div = " + div);
                    if (div != -1 && mBleAdvertiser != null) {
                        mBleAdvertiser.setDiv(div);
                    }
                    if (mBleAdvertiser != null) {
                        mBleAdvertiser.stopAdvertising();
                        Log.d("GattBroadcastServiceStateMachine:",
                            " starting advertising");
                        mBleAdvertiser.startAdvertising(false,
                            BleAdvertiser.BRA_DISABLED_STREAMING_ACTIVE_ADV_INTERVAL);
                    }
                }
                else if (mBATService.getBATState() == BATService.STATE_PAUSED)
                    setInitialState(mBRADisabledStreamingPaused);
                else
                    setInitialState(mBRADisabledStreamingDisabled);
            }
            else
                Log.e(TAG, "BA Service is null");

        }


        public void doQuit() {
            Log.i("GattBroadcastServiceStateMachine", "Quit");
            synchronized (GattBroadcastServiceStateMachine.this) {
                quitNow();
            }
        }

        private class BRADisabledStreamingDisabled extends State {
            private static final String TAG = "GattBroadcastService: BRADisabledStreamingDisabled";

            @Override
            public void enter() {
                Log.i(TAG, "Enter: " + getCurrentMessage().what);
            }

            @Override
            public void exit() {
                Log.i(TAG, "Exit: " + getCurrentMessage().what);
            }

            @Override
            public boolean processMessage(Message message) {
                Log.i(TAG, "processMessage: " + message.what);

                boolean retValue = HANDLED;
                switch (message.what) {
                    case INTENT:
                        Intent intent = (Intent) message.obj;

                        String action = intent.getAction();
                        Log.i(TAG, action);

                        if (action.equals(BATService.ACTION_BAT_STATE_CHANGED)) {
                            int state = intent.getIntExtra(BATService.EXTRA_STATE, -1);
                            int prevState = intent.getIntExtra(
                                    BATService.EXTRA_PREVIOUS_STATE, -1);
                            processBATStateChangedEvent(state, prevState);
                        } else if (action.equals(
                                    BATService.ACTION_BAT_ENCRYPTION_KEY_CHANGED)) {
                            BluetoothBAEncryptionKey encKey = intent.getParcelableExtra
                                    (BATService.EXTRA_ECNRYPTION_KEY);
                            processBATEncryptionKeyChangedEvent(encKey);
                        } else if (action.equals(BATService.ACTION_BAT_DIV_CHANGED)) {
                            int div = intent.getIntExtra(BATService.EXTRA_DIV_VALUE, -1);
                            processBATDivChangedEvent(div);
                        } else if (action.equals(BATService.ACTION_BAT_STREAMING_ID_CHANGED)) {
                            int streamingId = intent.getIntExtra(
                                    BATService.EXTRA_STREAM_ID, -1);
                            processBATStreamingIdChangedEvent(streamingId);
                        }
                        break;

                    case CONFIGURE_BROADCAST_RECEIVER_ASSOCIATION:
                        boolean enable = (Boolean) message.obj;
                        processConfigureBroadcastReceiverAssociation(enable);
                        break;

                    case ASSOCIATE_BCA_RECEIVER:
                        BluetoothDevice device = (BluetoothDevice) message.obj;
                        processAssociateBCAReceiver(device);
                        break;

                    default:
                        return NOT_HANDLED;
                }
                return retValue;
            }

            private void processBATStateChangedEvent(int state, int prevState) {
                Log.i(TAG, "processBATStateChangedEvent prevState: " + prevState
                        + " state: " + state);
                if (prevState == state) {
                    return;
                }
                switch (state) {
                    case BATService.STATE_PAUSED:
                        if (mBATService != null && mBleAdvertiser != null) {
                            mBleAdvertiser.mBroadcastStreamServiceRecords
                                .setBroadcastStreamServiceRecords(mBATService.getBAServiceRecord());

                            processBATDivChangedEvent(mBATService.getDIV());

                            mBleAdvertiser.startAdvertising(false,
                                BleAdvertiser.BRA_DISABLED_STREAMING_PAUSED_ADV_INTERVAL);

                            transitionTo(mBRADisabledStreamingPaused);
                        } else {
                            Log.e(TAG, "mBATService: " + mBATService + " mBleAdvertiser: "
                                    + mBleAdvertiser);
                        }
                        break;
                    default:
                        Log.w(TAG, "state " + state + " not handled");
                        break;
                }
            }

            private void processBATEncryptionKeyChangedEvent(BluetoothBAEncryptionKey encKey) {
                Log.i(TAG, "processBATEncryptionKeyChangedEvent encKey:" + encKey);
                if (encKey != null) {
                    mBleAdvertiser.mBroadcastSecurityKey.setEncryptionKeyType(
                            encKey.getFlagType());
                    mBleAdvertiser.mBroadcastSecurityKey.setEncryptionKey(
                            encKey.getEncryptionKey());
                }
            }

            private void processBATDivChangedEvent(int div) {
                Log.i(TAG, "processBATDivChangedEvent div:" + div);
                if (div != -1) {
                    mBleAdvertiser.setDiv(div);
                }
            }

            private void processBATStreamingIdChangedEvent(int streamingId) {
                Log.i(TAG, "processBATStreamingIdChangedEvent streamingId:" + streamingId);
                if (streamingId != -1) {
                    mBleAdvertiser.mBroadcastStatus.setActiveStreamId((byte) streamingId);
                }
            }

            private void processConfigureBroadcastReceiverAssociation(boolean enable) {
                Log.i(TAG, "Ignoring processConfigureBroadcastReceiverAssociation enable:" +
                        enable);
            }

            private void processAssociateBCAReceiver(BluetoothDevice device) {
                Log.i(TAG, "Ignoring associateBCAReceiver BD address: " + device.getAddress());
            }

        }

        private class BRADisabledStreamingPaused extends State {
            private static final String TAG = "GattBroadcastService:BRADisabledStreamingPaused";

            @Override
            public void enter() {
                Log.i(TAG, "Enter: " + getCurrentMessage().what);
            }

            @Override
            public void exit() {
                Log.i(TAG, "Exit: " + getCurrentMessage().what);
            }

            @Override
            public boolean processMessage(Message message) {
                Log.i(TAG, "processMessage: " + message.what);

                boolean retValue = HANDLED;
                switch (message.what) {
                    case INTENT:
                        Intent intent = (Intent) message.obj;

                        String action = intent.getAction();
                        Log.i(TAG, action);

                        if (action.equals(BATService.ACTION_BAT_STATE_CHANGED)) {
                            int state = intent.getIntExtra(BATService.EXTRA_STATE, -1);
                            int prevState = intent.getIntExtra(
                                BATService.EXTRA_PREVIOUS_STATE, -1);
                            processBATStateChangedEvent(state, prevState);
                        } else if (action.equals(BATService.ACTION_BAT_ENCRYPTION_KEY_CHANGED)) {
                            BluetoothBAEncryptionKey encKey = intent.getParcelableExtra
                                    (BATService.EXTRA_ECNRYPTION_KEY);
                            processBATEncryptionKeyChangedEvent(encKey);
                        } else if (action.equals(BATService.ACTION_BAT_DIV_CHANGED)) {
                            int div = intent.getIntExtra(BATService.EXTRA_DIV_VALUE, -1);
                            processBATDivChangedEvent(div);
                        } else if (action.equals(BATService.ACTION_BAT_STREAMING_ID_CHANGED)) {
                            int streamingId = intent.getIntExtra(BATService.EXTRA_STREAM_ID, -1);
                            processBATStreamingIdChangedEvent(streamingId);
                        }
                        break;

                    case CONFIGURE_BROADCAST_RECEIVER_ASSOCIATION:
                        boolean enable = (boolean) message.obj;
                        processConfigureBroadcastReceiverAssociation(enable);
                        break;
                    case ASSOCIATE_BCA_RECEIVER:
                        BluetoothDevice device = (BluetoothDevice) message.obj;
                        processAssociateBCAReceiver(device);
                        break;
                    default:
                        return NOT_HANDLED;
                }
                return retValue;
            }

            private void processBATStateChangedEvent(int state, int prevState) {
                Log.i(TAG, "processBATStateChangedEvent prevState: " + prevState
                        + " state: " + state);
                if (prevState == state) {
                    return;
                }
                switch (state) {
                    case BATService.STATE_DISABLED:
                        if (mBleAdvertiser != null) {
                            mBleAdvertiser.stopAdvertising();
                            transitionTo(mBRADisabledStreamingDisabled);
                        } else {
                            Log.e(TAG, "mBleAdvertiser is null");
                        }
                        break;
                    case BATService.STATE_PLAYING:
                        if (mBleAdvertiser != null) {
                            mBleAdvertiser.stopAdvertising();

                            mBleAdvertiser.startAdvertising(false,
                                BleAdvertiser.BRA_DISABLED_STREAMING_ACTIVE_ADV_INTERVAL);

                            transitionTo(mBRADisabledStreamingActive);

                        } else {
                            Log.e(TAG, "mBleAdvertiser is null");
                        }

                        break;
                    default:
                        Log.w(TAG, "state " + state + " not handled");
                        break;
                }
            }

            private void processBATEncryptionKeyChangedEvent(BluetoothBAEncryptionKey encKey) {
                Log.i(TAG, "processBATEncryptionKeyChangedEvent encKey:" + encKey);
                if (encKey != null) {
                    mBleAdvertiser.mBroadcastSecurityKey.setEncryptionKeyType(
                            encKey.getFlagType());
                    mBleAdvertiser.mBroadcastSecurityKey.setEncryptionKey(
                            encKey.getEncryptionKey());
                }
            }

            private void processBATDivChangedEvent(int div) {
                Log.i(TAG, "processBATDivChangedEvent div:" + div);
                if (div != -1) {
                    mBleAdvertiser.setDiv(div);
                }
            }

            private void processBATStreamingIdChangedEvent(long streamingId) {
                Log.i(TAG, "processBATStreamingIdChangedEvent streamingId:" + streamingId);
                if (streamingId != -1) {
                    mBleAdvertiser.mBroadcastStatus.setActiveStreamId((byte) streamingId);
                }
            }

            private void processConfigureBroadcastReceiverAssociation(boolean enable) {
                Log.i(TAG, "processConfigureBroadcastReceiverAssociation enable:" + enable);
                if (enable) {
                    if (mBATService != null && mSessionHandler != null
                        && mBleScanner != null && mBleAdvertiser != null) {

                        mBleAdvertiser.stopAdvertising();

                        processBATDivChangedEvent(mBATService.getDIV());
                        processBATStreamingIdChangedEvent(mBATService.getStreamId());
                        processBATEncryptionKeyChangedEvent(mBATService.getEncryptionKey());

                        mBleAdvertiser.startAdvertising(true,
                                BleAdvertiser.BRA_ENABLED_STREAMING_PAUSED_ADV_INTERVAL);

                        mBleScanner.discoverBroadcastAudioReceiver(true);

                        transitionTo(mBRAEnabledStreamingPaused);

                        mSessionHandler.sendMessage(
                                mSessionHandler.obtainMessage(NOTIFY_CB_CONFIGURE_BRA,
                                BRA_ENABLED_SUCCESS));

                    } else {
                        Log.e(TAG, "mSessionHandler: " + mSessionHandler
                                + " mBATService: " + mBATService + " mBleScanner: "
                                + mBleScanner + " mBleAdvertiser: " + mBleAdvertiser);
                        mSessionHandler.sendMessage(
                                mSessionHandler.obtainMessage(NOTIFY_CB_CONFIGURE_BRA,
                                BRA_ENABLED_FAILED));
                    }
                } else {
                    Log.i(TAG, "Unexpected, Ignoring BRA disable");
                }

            }

            private void processAssociateBCAReceiver(BluetoothDevice device) {
                Log.i(TAG, "Ignoring associateBCAReceiver BD address: " + device.getAddress());
            }

        }

        private class BRADisabledStreamingActive extends State {
            private static final String TAG = "GattBroadcastService:BRADisabledStreamingActive";

            @Override
            public void enter() {
                Log.i(TAG, "Enter: " + getCurrentMessage().what);
            }

            @Override
            public void exit() {
                Log.i(TAG, "Exit: " + getCurrentMessage().what);
            }

            @Override
            public boolean processMessage(Message message) {
                Log.i(TAG, "processMessage: " + message.what);

                boolean retValue = HANDLED;
                switch (message.what) {
                    case INTENT:
                        Intent intent = (Intent) message.obj;

                        String action = intent.getAction();
                        Log.i(TAG, action);

                        if (action.equals(BATService.ACTION_BAT_STATE_CHANGED)) {
                            int state = intent.getIntExtra(BATService.EXTRA_STATE, -1);
                            int prevState = intent.getIntExtra(
                                    BATService.EXTRA_PREVIOUS_STATE, -1);
                            processBATStateChangedEvent(state, prevState);
                        } else if (action.equals(BATService.ACTION_BAT_ENCRYPTION_KEY_CHANGED)) {
                            BluetoothBAEncryptionKey encKey = intent.getParcelableExtra
                                    (BATService.EXTRA_ECNRYPTION_KEY);
                            processBATEncryptionKeyChangedEvent(encKey);
                        } else if (action.equals(BATService.ACTION_BAT_DIV_CHANGED)) {
                            int div = intent.getIntExtra(BATService.EXTRA_DIV_VALUE, -1);
                            processBATDivChangedEvent(div);
                        } else if (action.equals(BATService.ACTION_BAT_STREAMING_ID_CHANGED)) {
                            int streamingId = intent.getIntExtra(BATService.EXTRA_STREAM_ID, -1);
                            processBATStreamingIdChangedEvent(streamingId);
                        }
                        break;

                    case CONFIGURE_BROADCAST_RECEIVER_ASSOCIATION:
                        boolean enable = (Boolean) message.obj;
                        processConfigureBroadcastReceiverAssociation(enable);
                        break;

                    case ASSOCIATE_BCA_RECEIVER:
                        BluetoothDevice device = (BluetoothDevice) message.obj;
                        processAssociateBCAReceiver(device);
                        break;

                    default:
                        return NOT_HANDLED;
                }
                return retValue;
            }

            private void processBATStateChangedEvent(int state, int prevState) {
                Log.i(TAG, "processBATStateChangedEvent prevState: " + prevState
                        + " state: " + state);
                if (prevState == state) {
                    return;
                }
                switch (state) {
                    case BATService.STATE_DISABLED:
                        if (mBleAdvertiser != null) {
                            mBleAdvertiser.stopAdvertising();
                            transitionTo(mBRADisabledStreamingDisabled);
                        } else {
                            Log.e(TAG, "mBleAdvertiser is null");
                        }
                        break;
                    case BATService.STATE_PAUSED:
                        if (mBleAdvertiser != null) {
                            mBleAdvertiser.stopAdvertising();

                            mBleAdvertiser.startAdvertising(false,
                                BleAdvertiser.BRA_DISABLED_STREAMING_PAUSED_ADV_INTERVAL);

                            transitionTo(mBRADisabledStreamingPaused);

                            } else {
                                Log.e(TAG, "mBleAdvertiser is null");
                            }

                        break;
                    default:
                        Log.w(TAG, "state " + state + " not handled");
                        break;
                }
            }

            private void processBATEncryptionKeyChangedEvent(BluetoothBAEncryptionKey encKey) {
                Log.i(TAG, "processBATEncryptionKeyChangedEvent encKey:" + encKey);
                if (encKey != null) {
                    mBleAdvertiser.mBroadcastSecurityKey.setEncryptionKeyType(
                            encKey.getFlagType());
                    mBleAdvertiser.mBroadcastSecurityKey.setEncryptionKey(
                            encKey.getEncryptionKey());
                }
            }

            private void processBATDivChangedEvent(int div) {
                Log.i(TAG, "processBATDivChangedEvent div:" + div);
                if (div != -1) {
                    mBleAdvertiser.setDiv(div);
                }
            }

            private void processBATStreamingIdChangedEvent(long streamingId) {
                Log.i(TAG, "processBATStreamingIdChangedEvent streamingId:" + streamingId);
                if (streamingId != -1) {
                    mBleAdvertiser.mBroadcastStatus.setActiveStreamId((byte) streamingId);
                }
            }

            private void processConfigureBroadcastReceiverAssociation(boolean enable) {
                Log.i(TAG, "processConfigureBroadcastReceiverAssociation enable:" + enable);
                if (enable) {
                    if (mBATService != null && mSessionHandler != null
                        && mBleScanner != null && mBleAdvertiser != null) {

                        mBleAdvertiser.stopAdvertising();

                        processBATDivChangedEvent(mBATService.getDIV());
                        processBATStreamingIdChangedEvent(mBATService.getStreamId());
                        processBATEncryptionKeyChangedEvent(mBATService.getEncryptionKey());

                        mBleAdvertiser.startAdvertising(true,
                                BleAdvertiser.BRA_ENABLED_STREAMING_ACTIVE_ADV_INTERVAL);

                        mBleScanner.discoverBroadcastAudioReceiver(true);

                        transitionTo(mBRAEnabledStreamingActive);

                        mSessionHandler.sendMessage(
                                mSessionHandler.obtainMessage(NOTIFY_CB_CONFIGURE_BRA,
                                BRA_ENABLED_SUCCESS));

                    } else {
                        Log.e(TAG, "mSessionHandler: " + mSessionHandler
                                + " mBATService: " + mBATService + " mBleScanner: "
                                + mBleScanner + " mBleAdvertiser: " + mBleAdvertiser);
                        mSessionHandler.sendMessage(
                                mSessionHandler.obtainMessage(NOTIFY_CB_CONFIGURE_BRA,
                                BRA_ENABLED_FAILED));
                    }
                } else {
                    Log.i(TAG, "Unexpected, Ignoring BRA disable");
                }
            }

            private void processAssociateBCAReceiver(BluetoothDevice device) {
                Log.i(TAG, "Ignoring associateBCAReceiver BD address: " + device.getAddress());
            }
        }

        private class BRAEnabledStreamingPaused extends State {
            private static final String TAG = "GattBroadcastService:BRAEnabledStreamingPaused";

            @Override
            public void enter() {
                Log.i(TAG, "Enter: " + getCurrentMessage().what);
            }

            @Override
            public void exit() {
                Log.i(TAG, "Exit: " + getCurrentMessage().what);
            }

            @Override
            public boolean processMessage(Message message) {
                Log.i(TAG, "processMessage: " + message.what);

                boolean retValue = HANDLED;
                switch (message.what) {
                    case INTENT:
                        Intent intent = (Intent) message.obj;

                        String action = intent.getAction();
                        Log.i(TAG, action);

                        if (action.equals(BATService.ACTION_BAT_STATE_CHANGED)) {
                            int state = intent.getIntExtra(BATService.EXTRA_STATE, -1);
                            int prevState = intent.getIntExtra(
                                    BATService.EXTRA_PREVIOUS_STATE, -1);
                            processBATStateChangedEvent(state, prevState);
                        } else if (action.equals(BATService.ACTION_BAT_ENCRYPTION_KEY_CHANGED)) {
                            BluetoothBAEncryptionKey encKey = intent.getParcelableExtra
                                    (BATService.EXTRA_ECNRYPTION_KEY);
                            processBATEncryptionKeyChangedEvent(encKey);
                        } else if (action.equals(BATService.ACTION_BAT_DIV_CHANGED)) {
                            int div = intent.getIntExtra(BATService.EXTRA_DIV_VALUE, -1);
                            processBATDivChangedEvent(div);
                        } else if (action.equals(BATService.ACTION_BAT_STREAMING_ID_CHANGED)) {
                            int streamingId = intent.getIntExtra(BATService.EXTRA_STREAM_ID, -1);
                            processBATStreamingIdChangedEvent(streamingId);
                        }
                        break;

                    case CONFIGURE_BROADCAST_RECEIVER_ASSOCIATION:
                        boolean enable = (boolean) message.obj;
                        processConfigureBroadcastReceiverAssociation(enable);
                        break;

                    case ASSOCIATE_BCA_RECEIVER:
                        BluetoothDevice device = (BluetoothDevice) message.obj;
                        processAssociateBCAReceiver(device);
                        break;

                    default:
                        return NOT_HANDLED;
                }
                return retValue;
            }

            private void processBATStateChangedEvent(int state, int prevState) {
                Log.i(TAG, "processBATStateChangedEvent prevState: " + prevState
                        + " state: " + state);
                if (prevState == state) {
                    return;
                }
                switch (state) {
                    case BATService.STATE_DISABLED:
                        if (mBleScanner != null) {
                            mBleScanner.discoverBroadcastAudioReceiver(false);
                        } else {
                            Log.i(TAG, "mBleScanner is null");
                        }

                        if (mBleAdvertiser != null) {
                            mBleAdvertiser.closeServer();
                            mBleAdvertiser.stopAdvertising();
                        } else {
                            Log.i(TAG, "mBleAdvertiser is null");
                        }

                        transitionTo(mBRADisabledStreamingDisabled);
                        break;
                    case BATService.STATE_PLAYING:
                        transitionTo(mBRAEnabledStreamingActive);
                        break;
                    default:
                        Log.w(TAG, "state " + state + " not handled");
                        break;
                }
            }

            private void processBATEncryptionKeyChangedEvent(BluetoothBAEncryptionKey encKey) {
                Log.i(TAG, "processBATEncryptionKeyChangedEvent encKey:" + encKey);
                if (encKey != null) {
                    mBleAdvertiser.mBroadcastSecurityKey.setEncryptionKeyType(
                            encKey.getFlagType());
                    mBleAdvertiser.mBroadcastSecurityKey.setEncryptionKey(
                            encKey.getEncryptionKey());
                }
            }

            private void processBATDivChangedEvent(int div) {
                Log.i(TAG, "processBATDivChangedEvent div:" + div);
                if (div != -1) {
                    mBleAdvertiser.setDiv(div);
                }
            }

            private void processBATStreamingIdChangedEvent(long streamingId) {
                Log.i(TAG, "processBATStreamingIdChangedEvent streamingId:" + streamingId);
                if (streamingId != -1) {
                    mBleAdvertiser.mBroadcastStatus.setActiveStreamId((byte) streamingId);
                }
            }

            private void processConfigureBroadcastReceiverAssociation(boolean enable) {
                Log.i(TAG, "processConfigureBroadcastReceiverAssociation enable:" + enable);
                if (enable) {
                    Log.i(TAG, "Unexpected, Ignoring BRA enable");
                } else {
                    int status = BRA_DISABLED_SUCCESS;
                    if (mBleScanner != null) {
                        mBleScanner.discoverBroadcastAudioReceiver(false);
                    } else {
                        Log.i(TAG, "mBleScanner is null");
                        status = BRA_DISABLED_FAILED;
                    }

                    if (mBleAdvertiser != null) {
                        mBleAdvertiser.closeServer();
                        mBleAdvertiser.stopAdvertising();

                        mBleAdvertiser.startAdvertising(false,
                                BleAdvertiser.BRA_DISABLED_STREAMING_PAUSED_ADV_INTERVAL);

                        transitionTo(mBRADisabledStreamingPaused);
                    } else {
                        Log.i(TAG, "mBleAdvertiser is null");
                        status = BRA_DISABLED_FAILED;
                    }

                    if (mSessionHandler != null) {
                        mSessionHandler.sendMessage(
                                mSessionHandler.obtainMessage(NOTIFY_CB_CONFIGURE_BRA, status));
                    } else {
                        Log.e(TAG, "mSessionHandler is null");
                    }

                }
            }

            private void processAssociateBCAReceiver(BluetoothDevice device) {
                Log.i(TAG, "processAssociateBCAReceiver BD address:" + device.getAddress());
                mBleAdvertiser.connectDevice(device);
            }

        }

        private class BRAEnabledStreamingActive extends State {
            private static final String TAG = "GattBroadcastService:BRAEnabledStreamingActive";

            @Override
            public void enter() {
                Log.i(TAG, "Enter: " + getCurrentMessage().what);
            }

            @Override
            public void exit() {
                Log.i(TAG, "Exit: " + getCurrentMessage().what);
            }

            @Override
            public boolean processMessage(Message message) {
                Log.i(TAG, "processMessage: " + message.what);

                boolean retValue = HANDLED;
                switch (message.what) {
                    case INTENT:
                        Intent intent = (Intent) message.obj;

                        String action = intent.getAction();
                        Log.i(TAG, action);

                        if (action.equals(BATService.ACTION_BAT_STATE_CHANGED)) {
                            int state = intent.getIntExtra(BATService.EXTRA_STATE, -1);
                            int prevState = intent.getIntExtra(
                                    BATService.EXTRA_PREVIOUS_STATE, -1);
                            processBATStateChangedEvent(state, prevState);
                        } else if (action.equals(BATService.ACTION_BAT_ENCRYPTION_KEY_CHANGED)) {
                            BluetoothBAEncryptionKey encKey = intent.getParcelableExtra
                                    (BATService.EXTRA_ECNRYPTION_KEY);
                            processBATEncryptionKeyChangedEvent(encKey);
                        } else if (action.equals(BATService.ACTION_BAT_DIV_CHANGED)) {
                            int div = intent.getIntExtra(BATService.EXTRA_DIV_VALUE, -1);
                            processBATDivChangedEvent(div);
                        } else if (action.equals(BATService.ACTION_BAT_STREAMING_ID_CHANGED)) {
                            int streamingId = intent.getIntExtra(BATService.EXTRA_STREAM_ID, -1);
                            processBATStreamingIdChangedEvent(streamingId);
                        }
                        break;

                    case CONFIGURE_BROADCAST_RECEIVER_ASSOCIATION:
                        boolean enable = (boolean) message.obj;
                        processConfigureBroadcastReceiverAssociation(enable);
                        break;
                    case ASSOCIATE_BCA_RECEIVER:
                        BluetoothDevice device = (BluetoothDevice) message.obj;
                        processAssociateBCAReceiver(device);
                        break;
                    default:
                        return NOT_HANDLED;
                }
                return retValue;
            }

            private void processBATStateChangedEvent(int state, int prevState) {
                Log.i(TAG, "processBATStateChangedEvent prevState: " + prevState
                        + " state: " + state);
                if (prevState == state) {
                    return;
                }
                switch (state) {
                    case BATService.STATE_DISABLED:
                        if (mBleScanner != null) {
                            mBleScanner.discoverBroadcastAudioReceiver(false);
                        } else {
                            Log.i(TAG, "mBleScanner is null");
                        }

                        if (mBleAdvertiser != null) {
                            mBleAdvertiser.closeServer();
                            mBleAdvertiser.stopAdvertising();
                        } else {
                            Log.i(TAG, "mBleAdvertiser is null");
                        }

                        transitionTo(mBRADisabledStreamingDisabled);
                        break;
                    case BATService.STATE_PAUSED:
                        transitionTo(mBRAEnabledStreamingPaused);
                        break;
                    default:
                        Log.w(TAG, "state " + state + " not handled");
                        break;
                }
            }

            private void processBATEncryptionKeyChangedEvent(BluetoothBAEncryptionKey encKey) {
                Log.i(TAG, "processBATEncryptionKeyChangedEvent encKey:" + encKey);
                if (encKey != null) {
                    mBleAdvertiser.mBroadcastSecurityKey.setEncryptionKeyType(
                            encKey.getFlagType());
                    mBleAdvertiser.mBroadcastSecurityKey.setEncryptionKey(
                            encKey.getEncryptionKey());
                }
            }

            private void processBATDivChangedEvent(int div) {
                Log.i(TAG, "processBATDivChangedEvent div:" + div);
                if (div != -1) {
                    mBleAdvertiser.setDiv(div);
                }
            }

            private void processBATStreamingIdChangedEvent(long streamingId) {
                Log.i(TAG, "processBATStreamingIdChangedEvent streamingId:" + streamingId);
                if (streamingId != -1) {
                    mBleAdvertiser.mBroadcastStatus.setActiveStreamId((byte) streamingId);
                }
            }

            private void processConfigureBroadcastReceiverAssociation(boolean enable) {
                Log.i(TAG, "processConfigureBroadcastReceiverAssociation enable:" + enable);
                if (enable) {
                    Log.i(TAG, "Unexpected, Ignoring BRA enable");
                } else {
                    int status = BRA_DISABLED_SUCCESS;
                    if (mBleScanner != null) {
                        mBleScanner.discoverBroadcastAudioReceiver(false);
                    } else {
                        Log.i(TAG, "mBleScanner is null");
                        status = BRA_DISABLED_FAILED;
                    }

                    if (mBleAdvertiser != null) {
                        mBleAdvertiser.closeServer();
                        mBleAdvertiser.stopAdvertising();

                        mBleAdvertiser.startAdvertising(false,
                                BleAdvertiser.BRA_DISABLED_STREAMING_ACTIVE_ADV_INTERVAL);

                        transitionTo(mBRADisabledStreamingActive);
                    } else {
                        Log.i(TAG, "mBleAdvertiser is null");
                        status = BRA_DISABLED_FAILED;
                    }

                    if (mSessionHandler != null) {
                        mSessionHandler.sendMessage(
                                mSessionHandler.obtainMessage(NOTIFY_CB_CONFIGURE_BRA, status));
                    } else {
                        Log.e(TAG, "mSessionHandler is null");
                    }
                }
            }

            private void processAssociateBCAReceiver(BluetoothDevice device) {
                Log.i(TAG, "processAssociateBCAReceiver BD address:" + device.getAddress());
                mBleAdvertiser.connectDevice(device);
            }
        }
    }
}
