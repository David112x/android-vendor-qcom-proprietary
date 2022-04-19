/*
 * Copyright (c) 2015-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import com.qualcomm.ims.vt.ImsVideoGlobals;

import org.codeaurora.ims.QtiImsExtManager;
import org.codeaurora.ims.utils.QtiCarrierConfigHelper;
import org.codeaurora.ims.utils.QtiImsExtUtils;

import com.qualcomm.ims.utils.Log;

import android.telephony.ims.stub.ImsConfigImplBase;
import android.telephony.ims.stub.ImsFeatureConfiguration;
import android.telephony.ims.stub.ImsRegistrationImplBase;

import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.ServiceManager;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.MmTelFeature;

import java.util.ArrayList;
import java.util.List;

public class ImsService extends android.telephony.ims.ImsService {

    private static final String LOG_TAG = "ImsService";
    private static final int INVALID_SLOT_ID = -1;
    private static final int UNINITIALIZED_VALUE = -1;

    private static ImsServiceSub mServiceSubs[];
    private ImsSenderRxr mSenderRxrs[];

    private int mNumPhonesCache = UNINITIALIZED_VALUE;
    private ImsSubController mSubController;

    private int getNumSlots() {
        if (mNumPhonesCache == UNINITIALIZED_VALUE) {
            mNumPhonesCache = ((TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE)).
                    getPhoneCount();
        }
        return mNumPhonesCache;
    }

    private void setup() {
        QtiCarrierConfigHelper.getInstance().setup(this);
        migrateDb();
        final int numSlots = getNumSlots();
        mServiceSubs = new ImsServiceSub[numSlots];
        mSenderRxrs = new ImsSenderRxr[numSlots];
        for (int i = 0; i < numSlots; i++) {
            mSenderRxrs[i] = new ImsSenderRxr(this, i);
        }
        mSubController = new ImsSubController(this, mSenderRxrs);
        for (int i = 0; i < numSlots; i++) {
            mServiceSubs[i] = new ImsServiceSub(this, i, mSenderRxrs[i], mSubController);
        }

        final int defaultSub = 0;
        ImsVideoGlobals.init(mServiceSubs, this);
    }

    static public ImsServiceSub[] getServiceSubs() {
        return mServiceSubs;
    }

    // Migrate db keys from vendor to AOSP
    private void migrateDb() {
        final String QTI_IMS_RTT_MODE = "rtt_mode";
        final int RTT_MODE_INVALID =  -1;
        //Read rtt mode from old db key and save in Settings.Secure.RTT_CALLING_MODE key.
        //This needs to be performed only once when new s/w with updated key is run first time.
        int rttMode = android.provider.Settings.Global.getInt(this.getContentResolver(),
                QTI_IMS_RTT_MODE, RTT_MODE_INVALID);
        if (rttMode != RTT_MODE_INVALID) {
            Log.v(this, "upgradeDb: migrate to new db key for rtt. mode=" + rttMode);
            android.provider.Settings.Secure.putInt(this.getContentResolver(),
                    Settings.Secure.RTT_CALLING_MODE, rttMode);
            android.provider.Settings.Global.putInt(this.getContentResolver(),
                    QTI_IMS_RTT_MODE, RTT_MODE_INVALID);
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i (this, "ImsService created!");
        setup();
    }

    @Override
    public IBinder onBind(Intent intent) {
        if (android.telephony.ims.ImsService.SERVICE_INTERFACE.equals(intent.getAction())) {
            Log.d(this, "Returning mImsServiceController for ImsService binding");
            return mImsServiceController;
        }
        Log.e(this, "Invalid Intent action in onBind: " + intent.getAction());
        return null;
    }

    /**
     * When called, provide the {@link ImsFeatureConfiguration} that this {@link ImsService}
     * currently supports. This will trigger the framework to set up the {@link ImsFeature}s that
     * correspond to the {@link ImsFeature}s configured here.
     *
     * Use {@link #onUpdateSupportedImsFeatures(ImsFeatureConfiguration)} to change the supported
     * {@link ImsFeature}s.
     *
     * @return an {@link ImsFeatureConfiguration} containing Features this ImsService supports.
     */
    @Override
    public ImsFeatureConfiguration querySupportedImsFeatures() {
        // NOTE: This method returns the same features that are declared in the AndroidManifest
        // file, as supported.
        ImsFeatureConfiguration.Builder features = new ImsFeatureConfiguration.Builder();
        for (int i = 0; i < getNumSlots(); i++) {
            features.addFeature(i, ImsFeature.FEATURE_MMTEL)
                    .addFeature(i, ImsFeature.FEATURE_EMERGENCY_MMTEL);
        }
        return features.build();
    }


    /**
     * The ImsService has been bound and is ready for ImsFeature creation based on the Features that
     * the ImsService has registered for with the framework, either in the manifest or via
     * {@link #querySupportedImsFeatures()}.
     *
     * The ImsService should use this signal instead of onCreate/onBind or similar to perform
     * feature initialization because the framework may bind to this service multiple times to
     * query the ImsService's {@link ImsFeatureConfiguration} via
     * {@link #querySupportedImsFeatures()}before creating features.
     */
    @Override
    public void readyForFeatureCreation() {
        Log.i(this, "readyForFeatureCreation :: No-op");
        // NOTE: This method is a no-op. IMS Service initializaiton will have to move
        // into this method if/when AOSP starts binding to IMS Service multiple times,
    }

    /**
     * The framework has enabled IMS for the slot specified, the ImsService should register for IMS
     * and perform all appropriate initialization to bring up all ImsFeatures.
     */
    @Override
    public void enableIms(int slotId) {
        Log.i(this, "enableIms :: slotId=" + slotId);
        if (slotId > INVALID_SLOT_ID && slotId < getNumSlots()) {
            mServiceSubs[slotId].turnOnIms();
        } else {
            Log.e(this, "enableIms :: Invalid slotId " + slotId);
        }
    }

    /**
     * The framework has disabled IMS for the slot specified. The ImsService must deregister for IMS
     * and set capability status to false for all ImsFeatures.
     */
    @Override
    public void disableIms(int slotId) {
        Log.i(this, "disableIms :: slotId=" + slotId);
        if (slotId > INVALID_SLOT_ID && slotId < getNumSlots()) {
            mServiceSubs[slotId].turnOffIms();
        } else {
            Log.e(this, "disableIms :: Invalid slotId " + slotId);
        }
    }

    /**
     * When called, the framework is requesting that a new {@link MmTelFeature} is created for the
     * specified slot.
     *
     * @param slotId The slot ID that the MMTEL Feature is being created for.
     * @return The newly created {@link MmTelFeature} associated with the slot or null if the
     * feature is not supported.
     */
    @Override
    public MmTelFeature createMmTelFeature(int slotId) {
        // NOTE: Since ImsServiceSub/MmTelFeature instances are created independently (as
        // part of this IMS Service's initialization), we simply return a created
        // instance, corresponding to the slotId.
        Log.d(this, "createMmTelFeature :: slotId=" + slotId);
        if (slotId > INVALID_SLOT_ID && slotId < getNumSlots()) {
            return mServiceSubs[slotId];
        }
        Log.e(this, "createMmTelFeature :: Invalid slotId " + slotId);
        return null;
    }

    /**
     * Return the {@link ImsConfigImplBase} implementation associated with the provided slot. This
     * will be used by the platform to get/set specific IMS related configurations.
     *
     * @param slotId The slot that the IMS configuration is associated with.
     * @return ImsConfig implementation that is associated with the specified slot.
     */
    @Override
    public ImsConfigImplBase getConfig(int slotId) {
        Log.d(this, "getConfig :: slotId=" + slotId);
        if (slotId > INVALID_SLOT_ID && slotId < getNumSlots()) {
            return (ImsConfigImplBase)mServiceSubs[slotId].getConfigInterface();
        }
        Log.e(this, "getConfig :: invalid slotId=" + slotId);
        return null;
    }

    /**
     * Return the {@link ImsRegistrationImplBase} implementation associated with the provided slot.
     *
     * @param slotId The slot that is associated with the IMS Registration.
     * @return the ImsRegistration implementation associated with the slot.
     */
    @Override
    public ImsRegistrationImplBase getRegistration(int slotId) {
        Log.d(this, "getRegistration :: slotId=" + slotId);
        if (slotId > INVALID_SLOT_ID && slotId < getNumSlots()) {
            return mServiceSubs[slotId].getImsRegistrationInterface();
        }
        Log.e(this, "getRegistration :: invalid slotId=" + slotId);
        return null;
    }

    @Override
    public void onDestroy() {
        Log.i(this, "Ims Service Destroyed Successfully...");
        ImsVideoGlobals.getInstance().dispose();
        for (int i = 0; i < getNumSlots(); i++) {
            mServiceSubs[i].dispose();
            mServiceSubs[i] = null;
            mSenderRxrs[i] = null;
        }
        mServiceSubs = null;
        mSenderRxrs = null;
        mSubController = null;
        QtiCarrierConfigHelper.getInstance().teardown();
        super.onDestroy();
    }
}
