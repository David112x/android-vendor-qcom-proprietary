/*
 * Copyright (c) 2017,2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 *
 * Copyright (C) 2017 The Android Open Source Project
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

package com.qualcomm.qti.server.wigig;

import android.os.HwRemoteBinder;
import android.os.RemoteException;

import android.hidl.manager.V1_0.IServiceManager;
import android.hidl.manager.V1_0.IServiceNotification;
import android.util.Log;

import java.util.NoSuchElementException;

import vendor.qti.hardware.capabilityconfigstore.V1_0.ICapabilityConfigStore;
import vendor.qti.hardware.capabilityconfigstore.V1_0.CommandResult;
import vendor.qti.hardware.capabilityconfigstore.V1_0.Result;

/**
 * Provides access to the capability config store via
 * HIDL, and supports recovery if service is restarted/crashes
 * In theory it is also possible to access ICapabilityConfigStore directly
 * without this class and much less code, however:
 * 1. If accessing once and storing the reference for all future calls, it
 * will not recover from service death
 * 2. If getting a new reference for each new call, it can recover from service
 * death, but will be less efficient since it has to establish connection
 * every call.
 * TODO we have several such HAL access class which have duplicated code,
 * move common code to ancestor class.
 * TODO this should be common class since there can be multiple users of the
 * capability config store HAL. Switch to common class once available.
 *
 * {@hide}
 */
public class CapConfigStoreHal {

    private static final String TAG = "CapConfigStoreHal";
    private static boolean DBG = true;
    private static final String WIGIG_CONFIG_STORE_AREA = "wigig";

    // lock to serialize HAL operations
    private final Object mLock = new Object();

    // HAL interface objects
    private IServiceManager mIServiceManager = null;
    private ICapabilityConfigStore mICapabilityConfigStore;
    private final IServiceNotification mServiceNotificationCallback =
            new IServiceNotification.Stub() {
        public void onRegistration(String fqName, String name, boolean preexisting) {
            synchronized (mLock) {
                Log.i(TAG, "IServiceNotification.onRegistration for: " + fqName
                        + ", " + name + " preexisting=" + preexisting);
                if (!initCapConfigStore()) {
                    Log.e(TAG, "initalizing capability config store interface failed.");
                    capConfigStoreServiceDiedHandler();
                } else {
                    Log.i(TAG, "Completed initialization of ICapabilityConfigStore interface.");
                }
            }
        }
    };
    private final HwRemoteBinder.DeathRecipient mServiceManagerDeathRecipient =
            cookie -> {
                Log.w(TAG, "IServiceManager died: cookie=" + cookie);
                synchronized (mLock) {
                    capConfigStoreServiceDiedHandler();
                    mIServiceManager = null; // Will need to register a new ServiceNotification
                }
            };
    private final HwRemoteBinder.DeathRecipient mCapConfigStoreDeathRecipient =
            cookie -> {
                Log.w(TAG, "ICapabilityConfigStore died: cookie=" + cookie);
                synchronized (mLock) {
                    capConfigStoreServiceDiedHandler();
                }
            };

    private static CapConfigStoreHal sInstance;

    public static CapConfigStoreHal getInstance() {
        if (sInstance == null) {
            sInstance = new CapConfigStoreHal();
        }
        return sInstance;
    }

    public String getConfig(String area, String key) {
        synchronized(mLock) {
            return getConfigLocked(area, key);
        }
    }

    public String getConfigString(String key, String defValue) {
        String value = getConfig(WIGIG_CONFIG_STORE_AREA, key);
        if (value == null) {
            value = defValue;
        }
        return value;
    }

    public boolean getConfigBoolean(String key, boolean defValue) {
        boolean bvalue;
        String value = getConfig(WIGIG_CONFIG_STORE_AREA, key);
        if (value != null) {
            if (value.equals("true") || value.equals("1")) {
                bvalue = true;
            } else if (value.equals("false") || value.equals("0")) {
                bvalue = false;
            } else {
                Log.i(TAG, "invalid boolean value: " + value + " using default");
                bvalue = defValue;
            }
        } else {
            bvalue = defValue;
        }

        return bvalue;
    }

    public int getConfigInteger(String key, int defValue) {
        int ivalue;
        String value = getConfig(WIGIG_CONFIG_STORE_AREA, key);
        if (value != null) {
            try {
                ivalue = Integer.parseInt(value);
            } catch (NumberFormatException e) {
                Log.i(TAG, "invalid integer value: " + value + " using default");
                ivalue = defValue;
            }
        } else {
            ivalue = defValue;
        }

        return ivalue;
    }

    private CapConfigStoreHal() {
        if (!initServiceManager()) {
            Log.e(TAG, "failed to initialize HIDL service manager");
        }
        if (!initCapConfigStore()) {
            Log.e(TAG, "initalizing capability config store interface failed.");
            capConfigStoreServiceDiedHandler();
        } else {
            Log.i(TAG, "Completed initialization of ICapabilityConfigStore interface.");
        }
    }

    void enableVerboseLogging(int verbose) {
        if (verbose > 0) {
            DBG = true;
        } else {
            DBG = false;
        }
    }

    private boolean linkToServiceManagerDeath() {
        if (mIServiceManager == null) return false;
        try {
            if (!mIServiceManager.linkToDeath(mServiceManagerDeathRecipient, 0)) {
                Log.wtf(TAG, "Error on linkToDeath on IServiceManager");
                capConfigStoreServiceDiedHandler();
                mIServiceManager = null; // Will need to register a new ServiceNotification
                return false;
            }
        } catch (RemoteException e) {
            Log.e(TAG, "IServiceManager.linkToDeath exception", e);
            return false;
        }
        return true;
    }

    /**
     * Registers a service notification for the ICapabilityConfigStore
     * service.
     * This will help in case wigig HAL service is killed and
     * restarted.
     * @return true if the service notification was successfully registered
     */
    private boolean initServiceManager() {
        Log.i(TAG, "Registering ICapabilityConfigStore service ready callback.");
        synchronized (mLock) {
            mICapabilityConfigStore = null;
            if (mIServiceManager != null) {
                // Already have an IServiceManager and serviceNotification registered,
                // don't register another.
                return true;
            }
            try {
                mIServiceManager = IServiceManager.getService();
                if (mIServiceManager == null) {
                    Log.e(TAG, "Failed to get HIDL Service Manager");
                    return false;
                }
                if (!linkToServiceManagerDeath()) {
                    return false;
                }
                if (!mIServiceManager.registerForNotifications(
                        ICapabilityConfigStore.kInterfaceName, "", mServiceNotificationCallback)) {
                    Log.e(TAG, "Failed to register for notifications to "
                            + ICapabilityConfigStore.kInterfaceName);
                    mIServiceManager = null; // Will need to register a new ServiceNotification
                    return false;
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception while trying to register a listener for ICapabilityConfigStore service: "
                        + e);
                capConfigStoreServiceDiedHandler();
            }
            return true;
        }
    }

    private boolean linkToCapConfigStoreDeath() {
        if (mICapabilityConfigStore == null) return false;
        try {
            if (!mICapabilityConfigStore.linkToDeath(mCapConfigStoreDeathRecipient, 0)) {
                Log.wtf(TAG, "Error on linkToDeath on ICapabilityConfigStore");
                capConfigStoreServiceDiedHandler();
                return false;
            }
        } catch (RemoteException e) {
            Log.e(TAG, "ICapabilityConfigStore.linkToDeath exception", e);
            return false;
        }
        return true;
    }

    private boolean initCapConfigStore() {
        synchronized (mLock) {
            try {
                mICapabilityConfigStore = ICapabilityConfigStore.getService();
            } catch (RemoteException e) {
                Log.e(TAG, "ICapabilityConfigStore.getService exception: " + e);
                return false;
            } catch (NoSuchElementException e) {
                Log.e(TAG, "ICapabilityConfigStore.getService exception: " + e);
                return false;
            } catch (Throwable e) {
                // any exception from here may cause HwBinder to exit, which
                // will kill the system server, so catch any other exception.
                Log.e(TAG, "ICapabilityConfigStore.getService generic exception: " + e);
                return false;
            }

            if (mICapabilityConfigStore == null) {
                Log.e(TAG, "Got null ICapabilityConfigStore service. Stopping HIDL startup");
                return false;
            }
            if (!linkToCapConfigStoreDeath()) {
                return false;
            }
        }
        return true;
    }

    private static class Mutable<E> {
        public E value;

        Mutable() {
            value = null;
        }

        Mutable(E value) {
            this.value = value;
        }
    }

    private void capConfigStoreServiceDiedHandler() {
        synchronized (mLock) {
            mICapabilityConfigStore = null;
            // TODO should we notify upper layers??
        }
    }

    private String getConfigLocked(String area, String key) {
        if (mICapabilityConfigStore == null) {
            Log.e(TAG, "capability config store service not available");
            return null;
        }
        try {
            CommandResult cres = mICapabilityConfigStore.getConfig(area, key);
            if (cres.result_type != Result.SUCCESS) {
                Log.d(TAG, "getConfig call failed for area: " + area + " key: " + key +
                      " result: " + cres.result_type);
                return null;
            }

            return cres.value;
        } catch (RemoteException e) {
            Log.e(TAG, "getConfig remote call failed: " + e);
            return null;
        }
    }
}
