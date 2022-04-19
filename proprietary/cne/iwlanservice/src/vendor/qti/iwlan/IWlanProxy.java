/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 *
 * Copyright (C) 2006 The Android Open Source Project
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

package vendor.qti.iwlan;

import android.content.Context;
import android.hardware.radio.V1_4.DataProfileInfo;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.HwBinder;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.RemoteException;
import android.os.SystemClock;
import android.telephony.data.DataProfile;
import android.text.TextUtils;
import android.util.Log;
import android.util.SparseArray;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicLong;
import java.util.Random;

import vendor.qti.hardware.data.iwlan.V1_0.IIWlan;
import vendor.qti.hardware.data.iwlan.V1_0.IWlanResponseInfo;

public class IWlanProxy {
    public static final String TAG = IWlanProxy.class.getSimpleName();

    private static final String WAKELOCK_TAG = "IWLAN_PROXY_WL";
    private static final String RESP_WAKELOCK_TAG = "IWLAN_PROXY_RESP_WL";

    public IWlanLog iwlanLog;

    static final String[] HIDL_SERVICE_NAME = {"slot1", "slot2", "slot3"};
    private static IWlanProxy instances[] = new IWlanProxy[HIDL_SERVICE_NAME.length];

    volatile IIWlan mService;
    IWlanIndication mIndication;
    IWlanResponse mResponse;

    /* Modem defaults to supporting iwlan */
    static boolean iwlanModemSupport = true;

    final IWlanServiceDeathRecipient mDeathRecipient;
    final Handler mHandler;
    final Looper mLooper;
    final AtomicLong mServiceCookie = new AtomicLong(0);

    final Integer mSlotId;
    SparseArray<IWlanRequest> mRequestList = new SparseArray<IWlanRequest>();

    /**
     * Wake lock timeout should be longer than the longest timeout in
     * the vendor ril.
     */
    private static final int WAKE_LOCK_TIMEOUT_MS = 60000;
    private static final int RESP_WAKE_LOCK_TIMEOUT_MS = 200;

    // Variables used to identify releasing of WL on wakelock timeouts
    volatile int mWlSequenceNum = 0;
    volatile int mRespWlSequenceNum = 0;

    // Variables used to differentiate ack messages from request while calling clearWakeLock()
    public static final int INVALID_WAKELOCK = -1;
    public static final int FOR_WAKELOCK = 0;
    public static final int FOR_RESP_WAKELOCK = 1;

    final WakeLock mWakeLock;           // Wake lock associated with request/response
    final WakeLock mRespWakeLock;      // Wake lock associated with response up stack
    // The number of wakelock requests currently active.  Don't release the lock until dec'd to 0
    int mWakeLockCount;

    //***** Request Constants
    static final int REQUEST_SETUP_DATA_CALL         = 1;
    static final int REQUEST_DEACTIVATE_DATA_CALL    = 2;
    static final int REQUEST_DATA_CALL_LIST          = 3;
    static final int REQUEST_GET_DATA_REG_STATE      = 4;
    static final int REQUEST_GET_QUALIFIED_NETWORKS  = 5;

    //***** Response Constants
    static final int RESPONSE_ACKNOWLEDGEMENT        = 50;

    //***** Events
    static final int EVENT_WAKE_LOCK_TIMEOUT          = 2;
    static final int EVENT_RESP_WAKE_LOCK_TIMEOUT     = 4;
    static final int EVENT_PROXY_DEAD                 = 5;

    public static IWlanProxy getInstanceBySlotId(Context context, int slotId) {
        synchronized (instances) {
            if(instances[slotId] == null) {
                instances[slotId] = new IWlanProxy(context, slotId);
            }
        }

        return instances[slotId];
    }

    public IWlanProxy(Context context, Integer slotId) {
        mSlotId = slotId;

        iwlanLog = new IWlanLog();
        mIndication = new IWlanIndication(this);
        mResponse = new IWlanResponse(this);
        mDeathRecipient = new IWlanServiceDeathRecipient();
        HandlerThread thread = new HandlerThread(IWlanProxy.class.getSimpleName());
        thread.start();
        mLooper = thread.getLooper();
        mHandler = new Handler(mLooper) {

            //***** Handler implementation
            @Override
            public void handleMessage(Message msg) {
                IWlanRequest req;

                switch (msg.what) {
                    case EVENT_WAKE_LOCK_TIMEOUT:
                        // Haven't heard back from the last request.  Assume we're
                        // not getting a response and  release the wake lock.

                        // The timer of WAKE_LOCK_TIMEOUT is reset with each
                        // new send request. So when WAKE_LOCK_TIMEOUT occurs
                        // all requests in mRequestList already waited at
                        // least WAKE_LOCK_TIMEOUT_MS but no response.
                        //
                        // Note: Keep mRequestList so that delayed response
                        // can still be handled when response finally comes.

                        synchronized (mRequestList) {
                            if (msg.arg1 == mWlSequenceNum && clearWakeLock(FOR_WAKELOCK)) {
                                int count = mRequestList.size();
                                iwlanLog.w(TAG, "WAKE_LOCK_TIMEOUT " + " mRequestList = " + count);
                                for (int i = 0; i < count; i++) {
                                    req = mRequestList.valueAt(i);
                                    iwlanLog.w(TAG, i + ": [" + req.mSerial + "] " + requestToString(req.mRequest));
                                }
                            }
                        }
                        break;

                    case EVENT_RESP_WAKE_LOCK_TIMEOUT:
                        if (msg.arg1 == mRespWlSequenceNum && clearWakeLock(FOR_RESP_WAKELOCK)) {
                            iwlanLog.w(TAG, "RESP_WAKE_LOCK_TIMEOUT");
                        }
                        break;

                    case EVENT_PROXY_DEAD:
                        iwlanLog.w(TAG, "handleMessage: EVENT_PROXY_DEAD cookie = " + msg.obj +
                                   ", mServiceCookie = " + mServiceCookie.get());
                        if ((long) msg.obj == mServiceCookie.get()) {
                            resetProxyAndRequestList();
                        }
                        break;
                }
            }
        };

        PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, WAKELOCK_TAG);
        mWakeLock.setReferenceCounted(false);
        mRespWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, RESP_WAKELOCK_TAG);
        mRespWakeLock.setReferenceCounted(false);
        mWakeLockCount = 0;

        iwlanLog.w(TAG, "new IWlan Proxy on slot " + mSlotId);
        getService();
    }

    private IIWlan getService() {
        if(mService != null) {
            return mService;
        }

        try {
            mService = IIWlan.getService(HIDL_SERVICE_NAME[mSlotId == null ? 0 : mSlotId], false);

            if (mService != null) {
                mService.linkToDeath(mDeathRecipient, mServiceCookie.incrementAndGet());
                mService.setResponseFunctions(mResponse, mIndication);
                iwlanLog.w(TAG, "new service: IIWlan client connected on " +
                           HIDL_SERVICE_NAME[mSlotId == null ? 0 : mSlotId]);
            } else {
              iwlanLog.e(TAG, "getService: proxy = null");
            }
        } catch(RemoteException | RuntimeException e) {
            mService = null;
            iwlanLog.e(TAG, "getService error for slot " + mSlotId + ", error: " + e);
        }

        return mService;
    }

    final class IWlanServiceDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            iwlanLog.w(TAG, "serviceDied, cookie = " + cookie + ", slot = " +
                       HIDL_SERVICE_NAME[mSlotId == null ? 0 : mSlotId]);
            mHandler.sendMessage(mHandler.obtainMessage(EVENT_PROXY_DEAD, cookie));
        }
    }

    private void resetProxyAndRequestList() {
        mService = null;
        iwlanLog.w(TAG, "reset Proxy, IIWLAN service set to null");

        // increment the cookie so that death notification can be ignored
        mServiceCookie.incrementAndGet();

        IWlanRequest req;
        synchronized (mRequestList) {
            int count = mRequestList.size();
            iwlanLog.w(TAG, "mWakeLockCount=" + mWakeLockCount + ", mRequestList = " + count);

            // Clear all wakelocks in the request list
            for (int i = 0; i < count; i++) {
                req = mRequestList.valueAt(i);
                iwlanLog.w(TAG, i + ": [" + req.mSerial + "] " + requestToString(req.mRequest));

                if (req.mResult != null) {
                    iwlanLog.w(TAG, "Sending RADIO_NOT_AVAILABLE AsyncResult");
                    AsyncResult.forMessage(req.mResult, null, new CommandException(CommandException.Error.RADIO_NOT_AVAILABLE));
                    req.mResult.sendToTarget();
                }

                decrementWakeLock(req);
            }
            mRequestList.clear();
        }

        getService();
    }

    private String convertDpiToString(DataProfileInfo dpi) {
        StringBuilder b = new StringBuilder();
        b.append("Profile ID = " + dpi.profileId);
        b.append(", APN = " + dpi.apn);
        b.append(", Protocol = " + dpi.protocol);
        b.append(", Roaming Protocol = " + dpi.roamingProtocol);
        b.append(", Auth Type = " + dpi.authType);
        b.append(", User = " + dpi.user);
        b.append(", Passwrod = " + dpi.password);
        b.append(", Type = " + dpi.type);
        b.append(", Enabled = " + dpi.enabled);
        b.append(", Supported Apn Types bitmap = " + dpi.supportedApnTypesBitmap);
        b.append(", BearerBitmap = " + dpi.bearerBitmap);
        b.append(", MTU = " + dpi.mtu);
        return b.toString();
    }

    public void setupDataCall(DataProfile dataProfile, boolean isRoaming,
                              boolean allowRoaming, int reason, LinkProperties linkProperties,
                              Message result) {
        IIWlan service = getService();

        if (service != null) {
            IWlanRequest req = obtainRequest(REQUEST_SETUP_DATA_CALL, result);

            // Convert to HAL data profile
            DataProfileInfo dpi = convertToHalDataProfile(dataProfile);
            ArrayList<String> addresses = new ArrayList<>();
            ArrayList<String> dnses = new ArrayList<>();
            if (linkProperties != null) {
                for (LinkAddress address : linkProperties.getLinkAddresses()) {
                    InetAddress addr = address.getAddress();
                    addresses.add(addr.getHostAddress());
                }
                for (InetAddress dns : linkProperties.getDnsServers()) {
                    dnses.add(dns.getHostAddress());
                }
            }

            iwlanLog.v(IWlanDataService.TAG, req.mSerial + " > " + requestToString(req.mRequest) + ", reason = " + reason +
                       ", dpi: " + convertDpiToString(dpi) + ", addresses = [" + TextUtils.join(",", addresses) + "]" +
                       ", dnses = [" + TextUtils.join(",", dnses) + "]");

            try {
                service.setupDataCall(req.mSerial, dpi, allowRoaming, reason, addresses, dnses);
            } catch (RemoteException | RuntimeException e) {
                iwlanLog.e(IWlanDataService.TAG, req.mSerial + " > " + requestToString(req.mRequest) + " error : " + e);
            }
        }
    }

    public void deactivateDataCall(int cid, int reason, Message result) {
        IIWlan service = getService();

        if (service != null) {
            IWlanRequest req = obtainRequest(REQUEST_DEACTIVATE_DATA_CALL, result);
            Random serial = new Random();

            iwlanLog.d(IWlanDataService.TAG, req.mSerial + " > " + requestToString(req.mRequest) + ", reason = " + reason +
                       ", cid = " + cid);

            try {
                service.deactivateDataCall(req.mSerial, cid, reason);
            } catch (RemoteException | RuntimeException e) {
                iwlanLog.e(TAG, req.mSerial + " > " + requestToString(req.mRequest) + " error : " + e);
            }
        }
    }

    public void requestDataCallList(Message result) {
        IIWlan service = getService();

        if (service != null) {
            IWlanRequest req = obtainRequest(REQUEST_DATA_CALL_LIST, result);

            iwlanLog.d(IWlanDataService.TAG, req.mSerial + " > " +  requestToString(req.mRequest));

            try {
                service.getDataCallList(req.mSerial);
            } catch (RemoteException | RuntimeException e) {
                iwlanLog.e(TAG, req.mSerial + " > " + requestToString(req.mRequest) + " error : " + e);
            }
        }
    }

    public void getDataRegistrationState(Message result) {
        IIWlan service = getService();

        if (service != null) {
            IWlanRequest req = obtainRequest(REQUEST_GET_DATA_REG_STATE, result);

            iwlanLog.d(IWlanNetworkService.TAG, req.mSerial + " > " +  requestToString(req.mRequest));

            try {
                service.getDataRegistrationState(req.mSerial);
            } catch (RemoteException | RuntimeException e) {
                iwlanLog.e(TAG, req.mSerial + " > " + requestToString(req.mRequest) + " error : " + e);
            }
        }
    }

    public void getAllQualifiedNetworks(Message result) {
        IIWlan service = getService();

        if (service != null) {
            IWlanRequest req = obtainRequest(REQUEST_GET_QUALIFIED_NETWORKS, result);

            iwlanLog.d(QualifiedNetworksServiceImpl.TAG, req.mSerial + " > " +  requestToString(req.mRequest));

            try {
                service.getAllQualifiedNetworks(req.mSerial);
            } catch (RemoteException | RuntimeException e) {
                iwlanLog.e(TAG, req.mSerial + " > " + requestToString(req.mRequest) + " error : " + e);
            }
        }
    }

    private IWlanRequest obtainRequest(int request, Message result) {
        IWlanRequest req = new IWlanRequest(request, result);

        acquireWakeLock(req, FOR_WAKELOCK);
        synchronized (mRequestList) {
            mRequestList.append(req.mSerial, req);
        }

        return req;
    }

    private IWlanRequest findAndRemoveRequestFromList(int serial) {
        IWlanRequest req = null;
        synchronized (mRequestList) {
            req = mRequestList.get(serial);
            if (req != null) {
                mRequestList.remove(serial);
            }
        }

        return req;
    }

    /**
     * Function to send ack and acquire related wakelock
     */
    public void sendAck(String tag) {
        IWlanRequest temp_req = new IWlanRequest(RESPONSE_ACKNOWLEDGEMENT, null);
        acquireWakeLock(temp_req, FOR_RESP_WAKELOCK);

        IIWlan proxy = getService();
        if (proxy != null) {
            try {
                proxy.responseAcknowledgement();
            } catch (RemoteException | RuntimeException e) {
                resetProxyAndRequestList();
                iwlanLog.e(tag, "sendAck : " + e);
            }
        } else {
            iwlanLog.e(tag, "Error trying to send ack, proxy = null");
        }
    }

    /**
     * Sends explicit message to modem that iWlan should be disabled.
     * Disables IWlan when the first service exits.
     */
    public static void disableIWlan(IWlanProxy proxy) {
        int slotId = proxy.mSlotId;
        synchronized (instances) {
            if(instances[slotId] != null) {

                // If the service for this slot doesn't exist we don't need to disable
                if(proxy.mService != null) {
                    try{
                        proxy.mService.iwlanDisabled();
                    }catch(RemoteException | RuntimeException e){
                        Log.e(TAG, "disableIWlan: " + e);
                    }
                }
                instances[slotId] = null;
            }
        }
    }

    /**
     * This is a helper function to be called when a IWlanServiceResponse callback is called.
     * It takes care of acks, wakelocks, and finds and returns IWlanRequest corresponding to the
     * response if one is found.
     * @param responseInfo IWlanResponseInfo received in response callback
     * @return IWlanRequest corresponding to the response
     */
    public IWlanRequest processResponse(IWlanResponseInfo responseInfo, boolean sendAck, String tag) {
        int serial = responseInfo.serial;
        int error = responseInfo.error;

        IWlanRequest req = findAndRemoveRequestFromList(serial);

        if (req == null) {
            iwlanLog.e(tag, req.mSerial + " > processResponse: Unexpected response! error: " + error);
            return null;
        } else {
            iwlanLog.w(tag, req.mSerial + " > Response Processed");
            if(sendAck) {
                iwlanLog.w(tag, req.mSerial + " > send ACK for serial");
                sendAck(tag);
            }
        }

        return req;
    }

    void processRequestAck(int serial) {
        IWlanRequest req;
        synchronized (mRequestList) {
            req = mRequestList.get(serial);
        }
        if (req == null) {
            iwlanLog.e(TAG, "processRequestAck: request not found for serial = " + serial);
        } else {
            decrementWakeLock(req);
            iwlanLog.w(TAG, req.mSerial + " Recieved ACK for serial = " + serial);
        }
    }

    /**
     * Holds a PARTIAL_WAKE_LOCK whenever
     * a) There is outstanding RIL request sent to RIL deamon and no replied
     * b) There is a request pending to be sent out.
     *
     * There is a WAKE_LOCK_TIMEOUT to release the lock, though it shouldn't
     * happen often.
     */
    private void acquireWakeLock(IWlanRequest req, int wakeLockType) {
        synchronized (req) {
            if (req.mWakeLockType != INVALID_WAKELOCK) {
                iwlanLog.w(TAG, "Failed to aquire wakelock for " + req.mSerial);
                return;
            }

            switch(wakeLockType) {
                case FOR_WAKELOCK:
                    synchronized (mWakeLock) {
                        mWakeLock.acquire();
                        mWakeLockCount++;
                        mWlSequenceNum++;

                        try {
                            Message msg = mHandler.obtainMessage(EVENT_WAKE_LOCK_TIMEOUT);
                            msg.arg1 = mWlSequenceNum;
                            mHandler.sendMessageDelayed(msg, WAKE_LOCK_TIMEOUT_MS);
                        } catch (IllegalStateException e) {
                            /*
                            *  Handler thread might not exist if bound service crashed.
                            *  If Unable to send msg, release the wakelock
                            */
                            mWakeLock.release();
                            mWakeLockCount--;
                            mWlSequenceNum--;
                        }
                    }
                    break;
                case FOR_RESP_WAKELOCK:
                    synchronized (mRespWakeLock) {
                        mRespWakeLock.acquire();
                        mRespWlSequenceNum++;

                        try {
                            Message msg = mHandler.obtainMessage(EVENT_RESP_WAKE_LOCK_TIMEOUT);
                            msg.arg1 = mRespWlSequenceNum;
                            mHandler.sendMessageDelayed(msg, RESP_WAKE_LOCK_TIMEOUT_MS);
                        } catch (IllegalStateException e) {
                            /*
                            *  Handler thread might not exist if bound service crashed.
                            *  If Unable to send msg, release the wakelock
                            */
                            mWakeLock.release();
                            mRespWlSequenceNum--;
                        }
                    }
                    break;
                default:
                    iwlanLog.w(TAG, "Acquiring Invalid Wakelock type " + wakeLockType);
                    return;
            }
            req.mWakeLockType = wakeLockType;
        }
    }

    public void decrementWakeLock(IWlanRequest req) {
        synchronized (req) {
            switch(req.mWakeLockType) {
                case FOR_WAKELOCK:
                    synchronized (mWakeLock) {
                        if (mWakeLockCount > 1) {
                            mWakeLockCount--;
                        } else {
                            mWakeLockCount = 0;
                            mWakeLock.release();
                        }
                    }
                    break;
                case FOR_RESP_WAKELOCK:
                    //We do not decrement the ACK wakelock, we always wait for timeout
                    break;
                case INVALID_WAKELOCK:
                    break;
                default:
                    iwlanLog.w(TAG, "Decrementing Invalid Wakelock type " + req.mWakeLockType);
            }
            req.mWakeLockType = INVALID_WAKELOCK;
        }
    }

    private boolean clearWakeLock(int wakeLockType) {
        if (wakeLockType == FOR_WAKELOCK) {
            synchronized (mWakeLock) {
                if (mWakeLockCount == 0 && !mWakeLock.isHeld()) return false;
                iwlanLog.w(TAG, "NOTE: mWakeLockCount is " + mWakeLockCount + "at time of clearing");
                mWakeLockCount = 0;
                mWakeLock.release();
                return true;
            }
        } else {
            synchronized (mRespWakeLock) {
                if (!mRespWakeLock.isHeld()) return false;
                mRespWakeLock.release();
                return true;
            }
        }
    }

    /**
     * Convert to DataProfileInfo defined in types.hal
     * @param dp Data profile
     * @return A converted data profile
     */
    private static DataProfileInfo convertToHalDataProfile(DataProfile dp) {
        DataProfileInfo dpi = new DataProfileInfo();

        dpi.profileId = dp.getProfileId();
        dpi.apn = dp.getApn();
        dpi.protocol = dp.getProtocolType();
        dpi.roamingProtocol = dp.getRoamingProtocolType();
        dpi.authType = dp.getAuthType();
        dpi.user = dp.getUserName();
        dpi.password = dp.getPassword();
        dpi.type = dp.getType();
        //dpi.maxConnsTime = dp.getMaxConnectionsTime();
        //dpi.maxConns = dp.getMaxConnections();
        //dpi.waitTime = dp.getWaitTime();
        dpi.enabled = dp.isEnabled();
        dpi.supportedApnTypesBitmap = dp.getSupportedApnTypesBitmask();
        dpi.bearerBitmap = dp.getBearerBitmask();
        dpi.mtu = dp.getMtu();

        return dpi;
    }

    static String requestToString(int request) {
        switch(request) {
            case REQUEST_SETUP_DATA_CALL:
                return "REQUEST_SETUP_DATA_CALL";
            case REQUEST_DEACTIVATE_DATA_CALL:
                return "REQUEST_DEACTIVATE_DATA_CALL";
            case REQUEST_DATA_CALL_LIST:
                return "REQUEST_DATA_CALL_LIST";
            case REQUEST_GET_DATA_REG_STATE:
                return "REQUEST_GET_DATA_REG_STATE";
            case REQUEST_GET_QUALIFIED_NETWORKS:
                return "REQUEST_GET_QUALIFIED_NETWORKS";
            default: return "<unknown request>";
        }
    }

    /********** INDICATION FUNCTIONS ****************/
    protected RegistrantList mDataCallListChangedRegistrants = new RegistrantList();
    protected RegistrantList mDataRegistrationStateChangedRegistrants = new RegistrantList();
    protected RegistrantList mQualifiedNetworksChangedRegistrants = new RegistrantList();

    public void modemSupportNotPresent() {
        //Currently not used
        iwlanModemSupport = false;
    }

    public void registerForDataCallListChanged(Handler h, int what) {
        if(h != null) {
            mDataCallListChangedRegistrants.addUnique(h, what, null);
        }
    }

    public void unregisterForDataCallListChanged(Handler h) {
        if(h != null) {
            mDataCallListChangedRegistrants.remove(h);
        }
    }

    public void registerForDataRegistrationStateChanged(Handler h, int what) {
        if(h != null) {
            mDataRegistrationStateChangedRegistrants.addUnique(h, what, null);
        }
    }

    public void unregisterForDataRegistrationStateChanged(Handler h) {
        if(h != null) {
            mDataRegistrationStateChangedRegistrants.remove(h);
        }
    }

    public void registerForQualifiedNetworksChanged(Handler h, int what) {
        if(h != null) {
            mQualifiedNetworksChangedRegistrants.addUnique(h, what, null);
        }
    }

    public void unregisterForQualifiedNetworksChanged(Handler h) {
        if(h != null) {
            mQualifiedNetworksChangedRegistrants.remove(h);
        }
    }
}
