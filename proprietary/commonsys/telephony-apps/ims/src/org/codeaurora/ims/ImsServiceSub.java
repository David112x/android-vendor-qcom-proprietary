/*
 * Copyright (c) 2015-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained for attribution purposes only.
 *
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.codeaurora.ims;

import java.io.IOException;
import java.util.Iterator;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.RejectedExecutionException;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;

import com.android.ims.ImsConfig;
import com.android.ims.internal.IImsCallSessionListener;
import com.android.ims.internal.IImsConfig;
import com.android.ims.internal.IImsRegistrationListener;
import com.android.ims.internal.IImsEcbm;
import com.android.ims.internal.IImsMultiEndpoint;
import com.android.internal.annotations.VisibleForTesting;
import org.codeaurora.ims.QtiCallConstants;
import org.codeaurora.ims.QtiImsExtBase.QtiImsExtBinder;
import com.qualcomm.ims.utils.Log;
import com.qualcomm.ims.vt.ImsVideoGlobals;
import org.codeaurora.telephony.utils.AsyncResult;
import org.codeaurora.telephony.utils.RegistrantList;
import org.codeaurora.telephony.utils.CallForwardInfo;

import org.codeaurora.ims.internal.IQtiImsExtListener;
import org.codeaurora.ims.ImsPhoneCommandsInterface;
import org.codeaurora.ims.utils.QtiImsExtUtils;

import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.os.Parcelable;
import android.os.RemoteCallbackList;
import android.os.UserHandle;
import android.telephony.CarrierConfigManager;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.feature.CapabilityChangeRequest;
import android.telephony.ims.feature.CapabilityChangeRequest.CapabilityPair;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.telephony.ims.stub.ImsEcbmImplBase;
import android.telephony.ims.stub.ImsMultiEndpointImplBase;
import android.telephony.ims.stub.ImsSmsImplBase;
import android.app.PendingIntent;
import android.content.Context;
import android.location.Address;
import android.location.Geocoder;
import android.os.RemoteException;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Pair;
import android.widget.Toast;

public class ImsServiceSub extends MmTelFeature
        implements ImsSubController.ImsStackConfigListener,
        ICallListListener {
    private static final String LOG_TAG = "ImsServiceSub";
    protected ImsSenderRxr mCi = null; /* Commands Interface */
    private ImsSubController mSubController;
    private ImsSmsImpl mImsSms;
    private ImsServiceClassTracker mTracker;
    private Handler mHandler;
    private ImsRegistrationImpl mImsReg;
    private Handler mFeatureCallbackHandler;
    private HandlerThread mFeatureHandlerThread;
    private Context mContext;
    private ImsEcbmImpl mImsEcbmImpl;
    private ImsConfigImpl mImsConfigImpl;
    private ImsUtImpl mImsUtImpl;
    private ServiceStatus mServiceStatus[] = null;
    private ImsMultiEndpointImpl mImsMultiEndpointImpl = null;
    private ImsServiceStateReceiver mImsServiceStateReceiver =  null;
    private ImsMultiIdentityImpl mMultiIdentityImpl;
    private ImsScreenShareControllerImpl mScreenShareController;
    private HashSet<Uri> mSelfIndentityUris = null;

    // Determines if this ImsServiceSub is ready for requests.
    private boolean mFeatureIsOpen = false;

    private boolean mIsVopsEnabled = false;
    private boolean mIsSsacVoiceBarred = false;
    private final int SSAC_VOICE_BARRING_ZERO = 0;
    private boolean mModemSupportForWfcRoamingConfiguration = false;

    //This is the error that we get from the modem
    //for IKEv2 Authentication failure(24).
    private static final int IKEv2_AUTH_FAILURE = 5;

    // Used to report failures for changeEnabledCapabilities.
    // If not null, this variable holds the CapabilityCallbackProxy
    // instance of the latest changeEnabledCapabilities invocation.
    private CapabilityCallbackProxy mCapabilityCallback;
    // TODO: This may need to be changed to a Map or List if we need to
    // keep track of multiple listeners corresponding to multiple
    // changeEnabledCapabilities calls.

    // Thread pool for invoking runnable tasks, such as GeocoderTask.
    private static ExecutorService sTaskExecutor = Executors.newCachedThreadPool();

    //Cached list for IQtiImsExtListener listeners registered for vice.
    private CopyOnWriteArrayList<IQtiImsExtListener>
            mQtiImsInterfaceListeners = new CopyOnWriteArrayList<IQtiImsExtListener>();

    //Cached list for IQtiImsExtListener listeners registered for participant status.
    private CopyOnWriteArrayList<IQtiImsExtListener>
            mQtiImsParticipantStatusListeners = new CopyOnWriteArrayList<IQtiImsExtListener>();

    private List<ICallListListener> mCallListListeners =
            new CopyOnWriteArrayList<ICallListListener>();

    private Object mMultiIdentityLock = new Object();
    private Object mScreenShareLock = new Object();
    // VOIP, VT_TX, VT_RX, VT, UT, SMS
    private static final int SERVICE_TYPE_MAX = 6;
    private final int MAX_FEATURES_SUPPORTED = 3;

    private final int EVENT_CALL_STATE_CHANGE = 1;
    private final int EVENT_IMS_STATE_CHANGED = 2;
    private final int EVENT_IMS_STATE_DONE = 3;
    private final int EVENT_SRV_STATUS_UPDATE = 4;
    private final int EVENT_GET_SRV_STATUS = 5;
    private final int EVENT_SET_SRV_STATUS = 6;
    private final int EVENT_GET_CURRENT_CALLS = 7;
    private final int EVENT_SUPP_SRV_UPDATE = 8;
    private final int EVENT_SET_IMS_STATE = 9;
    private final int EVENT_TTY_STATE_CHANGED = 10;
    private final int EVENT_REFRESH_CONF_INFO = 11;
    //Event that gets triggered for intra RAT HandOver's
    private final int EVENT_HANDOVER_STATE_CHANGED = 12;
    private final int EVENT_CALL_MODIFY = 13;
    private final int EVENT_MWI = 14;
    private final int EVENT_CONFERENCE = 15;
    private final int EVENT_SET_CALL_FORWARD_TIMER = 16;
    private final int EVENT_GET_CALL_FORWARD_TIMER = 17;
    private final int EVENT_SET_CALL_BARRING = 18;
    private final int EVENT_GEOLOCATION_INFO_REQUEST = 21;
    private final int EVENT_GEOLOCATION_RESPONSE = 22;
    private final int EVENT_VOWIFI_CALL_QUALITY_UPDATE = 23;
    private final int EVENT_CALL_TRANSFER = 24;
    private final int EVENT_VOPS_CHANGED = 25;
    private final int EVENT_SSAC_CHANGED = 26;
    private final int EVENT_VOPS_RESPONSE = 27;
    private final int EVENT_SSAC_RESPONSE = 28;
    private final int EVENT_PARTICIPANT_STATUS_INFO = 29;
    private final int EVENT_SET_VOLTE_PREF = 30;
    private final int EVENT_GET_VOLTE_PREF = 31;
    private final int EVENT_GET_HANDOVER_CONFIG = 32;
    private final int EVENT_SET_HANDOVER_CONFIG = 33;
    private final int EVENT_REGISTRATION_BLOCK_STATUS = 34;
    private final int EVENT_CANCEL_MODIFY_CALL = 35;
    private final int EVENT_CALL_AUTO_REJECT = 36;
    private final int EVENT_WFC_ROAMING_CONFIGURATION = 37;
    private final int EVENT_USSD_INFO = 38;
    private final int EVENT_SEND_AUTO_REJECT = 39;

    static final int CF_REASON_UNCONDITIONAL    = 0;

    // TODO: Change this to an ArrayList/Map of service class trackers.
    private ImsServiceClassTracker mConferenceRequestedbyTracker;

    private static final int INVALID_FEATURE_VALUE = -2;
    private static final int INVALID_RAT_VALUE = -2;
    // NOTE: Since CAPABILITY_ERROR_GENERIC value is -1, the above
    //       values are for differentiation.

    protected int mPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
    private int mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    int[] mEnabledFeatures = {
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN
    };
    int[] mDisabledFeatures = {
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN
    };
    private MmTelCapabilities mMmtelCapabilities = new MmTelCapabilities(0);

    private Mwi mMwiResponse;
    private IQtiImsExtListener mUssdInfoListener = null;

    private RegistrantList mCapabilitiesChangedRegistrants = new RegistrantList();

    //Constructor
    public ImsServiceSub(Context context, int phoneId, ImsSenderRxr senderRxr,
            ImsSubController subController) {
        setFeatureState(ImsFeature.STATE_INITIALIZING);
        mPhoneId = phoneId;
        mContext = context;
        mCi = senderRxr;
        mSubController = subController;
        logi( "[phoneId=" + mPhoneId +
                "] Constructor :: Registering with Sub Controller.");
        mHandler = new ImsServiceSubHandler();

        mImsReg = new ImsRegistrationImpl();
        mFeatureHandlerThread = new HandlerThread(this + "FeatureHandlerThread");
        mFeatureHandlerThread.start();
        mFeatureCallbackHandler = new Handler(mFeatureHandlerThread.getLooper());
        mSubController.registerListener(this, phoneId);
        mCi.registerForImsNetworkStateChanged(mHandler, EVENT_IMS_STATE_CHANGED,
                null);
        // Query for registration state in case we have missed the UNSOL
    //    mCi.getImsRegistrationState(mHandler.obtainMessage(EVENT_IMS_STATE_DONE));
        mCi.registerForSrvStatusUpdate(mHandler, EVENT_SRV_STATUS_UPDATE, null);
        mCi.registerForCallStateChanged(mHandler, EVENT_CALL_STATE_CHANGE, null);
        mCi.registerForRefreshConfInfo(mHandler, EVENT_REFRESH_CONF_INFO, null);
        mCi.registerForHandoverStatusChanged(mHandler, EVENT_HANDOVER_STATE_CHANGED, null);
        mCi.registerForGeolocationRequest(mHandler, EVENT_GEOLOCATION_INFO_REQUEST, null);
        mCi.registerForVopsStatusChanged(mHandler, EVENT_VOPS_CHANGED, null);
        mCi.registerForSsacStatusChanged(mHandler, EVENT_SSAC_CHANGED, null);
        mCi.registerForParticipantStatusInfo(mHandler, EVENT_PARTICIPANT_STATUS_INFO, null);
        mCi.registerForRegistrationBlockStatus(mHandler, EVENT_REGISTRATION_BLOCK_STATUS, null);
        mCi.registerForCallAutoRejection(mHandler, EVENT_CALL_AUTO_REJECT, null);
        mCi.registerForUssdInfo(mHandler, EVENT_USSD_INFO, null);

        mCi.registerForVoWiFiCallQualityUpdate(mHandler,
                EVENT_VOWIFI_CALL_QUALITY_UPDATE, null);

        mCi.registerForWfcRoamingModeFeatureSupport(mHandler,
                EVENT_WFC_ROAMING_CONFIGURATION, null);

        mImsEcbmImpl = new ImsEcbmImpl(this, mCi);

        //Initialize the UT interface associated with the sub.
        mImsUtImpl = new ImsUtImpl(this, mCi, mContext);

        //Initialize the IMS Config interface associated with the sub.
        mImsConfigImpl =  new ImsConfigImpl(this, mCi, mContext);

        //Initialize the MultiEndpoint interface
        mImsMultiEndpointImpl = new ImsMultiEndpointImpl(mCi, mContext, this);

        //Initialize sms implementation
        mImsSms = new ImsSmsImpl(mContext, mPhoneId, mCi);

        // For listening to incoming (MT) Hold/Resume UNSOLs.
        mCi.setOnSuppServiceNotification(mHandler, EVENT_SUPP_SRV_UPDATE, null);

        // For listening to MWI UNSOLs.
        mCi.registerForMwi(mHandler, EVENT_MWI, null);
        mMwiResponse = new Mwi();

        // For listening to TTY mode change UNSOL.
        mCi.registerForTtyStatusChanged(mHandler, EVENT_TTY_STATE_CHANGED, null);

        mCi.registerForModifyCall(mHandler, EVENT_CALL_MODIFY, null);
        initServiceStatus();

        //Initialize the ImsServiceStateReceiver associated with this sub.
        mImsServiceStateReceiver = new ImsServiceStateReceiver(this, mContext, mPhoneId);
        final IntentFilter intentFilter =
                new IntentFilter(Intent.ACTION_SIM_STATE_CHANGED);
        intentFilter.addAction(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);
        mContext.registerReceiver(mImsServiceStateReceiver, intentFilter);
        // Explicitly remove the HD icon in notification bar to handle
        // phone process crash.
        mImsServiceStateReceiver.updateHDIcon(false, false);
        mCi.queryServiceStatus(mHandler.obtainMessage(EVENT_GET_SRV_STATUS));
    }

    /**
     * Used for turning on IMS for this feature instance.
     */
    public void turnOnIms() {
        mCi.sendImsRegistrationState(ImsRegistrationInfo.REGISTERED,
                mHandler.obtainMessage(EVENT_SET_IMS_STATE));
    }

    /**
     * Used for turning off IMS for this feature. When IMS is OFF, the device will behave
     * as CSFB'ed.
     */
    public void turnOffIms() {
        mCi.sendImsRegistrationState(ImsRegistrationInfo.NOT_REGISTERED,
                mHandler.obtainMessage(EVENT_SET_IMS_STATE));
    }

    public void dispose() {
        setFeatureState(ImsFeature.STATE_UNAVAILABLE);
        logd( "dispose");
        if (mFeatureHandlerThread != null) {
            mFeatureHandlerThread.quitSafely();
            mFeatureHandlerThread = null;
            mFeatureCallbackHandler = null;
        }
        mImsMultiEndpointImpl = null;
        mContext.unregisterReceiver(mImsServiceStateReceiver);
        mImsServiceStateReceiver = null;
        if (mTracker != null) {
            mTracker.removeListener(this);
            mTracker = null;
        }
    }

    @Override
    public void onCallSessionAdded(ImsCallSessionImpl callSession) {
        for (ICallListListener listener : mCallListListeners) {
            listener.onCallSessionAdded(callSession);
        }
    }

    @Override
    public void onCallSessionRemoved(ImsCallSessionImpl callSession) {
        for (ICallListListener listener : mCallListListeners) {
            listener.onCallSessionRemoved(callSession);
        }
    }

    @Override
    public void onStackConfigChanged(boolean[] activeStacks, int phoneId) {
        /* In non-Dsdv cases (eg. 7+1/7+5), we need to honor this callback
         * for both the subs else in use-cases such as DDS switch etc., the
         * other service sub feature state will not be set to unavailable
         */
        if(mPhoneId != phoneId && mSubController.isDsdv()) {
            logv("onStackConfigChanged safely ignore the indication");
            return;
        }

        try {
            logi( "onStackConfigChanged :: activeStacks["
                    + mPhoneId + "]=" + activeStacks[mPhoneId]);
            int featureState = activeStacks[mPhoneId] == true ?
                ImsFeature.STATE_READY : ImsFeature.STATE_UNAVAILABLE;
            int oldFeatureState = getFeatureState();
            logi( "oldFeatureState = " + oldFeatureState);
            if (oldFeatureState == ImsFeature.STATE_READY &&
                    featureState == ImsFeature.STATE_UNAVAILABLE) {
                resetFeatures();

                ImsRegistrationInfo imsRegInfo = new ImsRegistrationInfo();
                imsRegInfo.setState(ImsRegistrationInfo.NOT_REGISTERED);
                Message msg = mHandler.obtainMessage(EVENT_IMS_STATE_CHANGED);
                AsyncResult.forMessage(msg, imsRegInfo, null);
                msg.sendToTarget();
                if (mImsServiceStateReceiver != null) {
                    mImsServiceStateReceiver.updateHDIcon(false, false);
                } else {
                    loge("onStackConfigChanged :: mImsServiceStateReceiver null");
                }
            } else if (oldFeatureState != ImsFeature.STATE_READY &&
                    featureState == ImsFeature.STATE_READY){
                mCi.sendConfigRequest(MessageId.REQUEST_SET_IMS_CONFIG,
                        ImsConfigItem.AUTO_REJECT_CALL_MODE, false,
                        QtiImsExtUtils.getAutoReject(mContext.getContentResolver(), mPhoneId), null,
                        ImsConfigItem.NO_ERR, mHandler.obtainMessage(EVENT_SEND_AUTO_REJECT));
            }

            setFeatureState(featureState);
        } catch (ArrayIndexOutOfBoundsException ex) {
            loge("onStackConfigChanged :: Invalid activeStacks length!");
        }
    }

    /* Method to initialize the Service related objects */
    private void initServiceStatus() {
        mServiceStatus = new ServiceStatus[SERVICE_TYPE_MAX];
        for (int i = 0; i < SERVICE_TYPE_MAX; i++) {
            mServiceStatus[i] = new ServiceStatus();
            /*
             * By default, the assumption is the service is enabled on LTE - when RIL and modem
             * changes to update the availability of service on power up this will be removed
             */
            mServiceStatus[i].accessTechStatus = new ServiceStatus.StatusForAccessTech[1];
            mServiceStatus[i].accessTechStatus[0] = new ServiceStatus.StatusForAccessTech();
            mServiceStatus[i].accessTechStatus[0].networkMode = RadioTech.RADIO_TECH_LTE;
            mServiceStatus[i].accessTechStatus[0].status = ServiceStatus.STATUS_NOT_SUPPORTED;
            mServiceStatus[i].accessTechStatus[0].registered = ImsRegistrationInfo.NOT_REGISTERED;
            mServiceStatus[i].status = ServiceStatus.STATUS_NOT_SUPPORTED;
        }
    }

    public void sendConferenceRequest(ImsServiceClassTracker sct) {
        mConferenceRequestedbyTracker = sct;

        // Send the conference request to lower layers.
        mCi.conference(mHandler.obtainMessage(EVENT_CONFERENCE, this));
    }

    private void handleConferenceResponse(AsyncResult ar) {
        mConferenceRequestedbyTracker.handleConferenceResult(ar);
    }

    /**
     * Method to gather the currently available calls from the available trackers.
     */
    private Object getCallsListToClear() {
        Object mmTelCallsList = null;
        if (mTracker != null) {
            // Only IMS calls will go to tracker
            mmTelCallsList = mTracker.getCallsListToClear();
        }
        return mmTelCallsList;
    }

    /**
     * This method is invoked by Framework when it is ready to use this MmTel
     * feature. We initialize the ImsServiceClassTracker instance at this
     * time.
     */
    @Override
    public void onFeatureReady() {
        logv( "onFeatureReady");
        if (mTracker == null) {
            logv( "onFeatureReady :: creating ImsServiceClassTracker");
            mTracker = new ImsServiceClassTracker(mCi, mContext, this);
            mTracker.updateFeatureCapabilities(isVideoSupported(), isVoiceSupported());
            mTracker.addListener(this);
        }

        notifyFeatureCapabilityChange();
        mCi.getImsRegistrationState(mHandler.obtainMessage(EVENT_IMS_STATE_DONE));

        mFeatureIsOpen = true;
        // TODO: Gate all callbacks to Framework on mFeatureIsOpen?
    }

    @Override
    public void onFeatureRemoved() {
        logd( "onFeatureRemoved");
        mFeatureIsOpen = false;
    }

    /**
     * Creates a {@link ImsCallProfile} from the service capabilities & IMS registration state.
     *
     * @param callSessionType a service type that is specified in {@link ImsCallProfile}
     *        {@link ImsCallProfile#SERVICE_TYPE_NONE}
     *        {@link ImsCallProfile#SERVICE_TYPE_NORMAL}
     *        {@link ImsCallProfile#SERVICE_TYPE_EMERGENCY}
     * @param callType a call type that is specified in {@link ImsCallProfile}
     *        {@link ImsCallProfile#CALL_TYPE_VOICE}
     *        {@link ImsCallProfile#CALL_TYPE_VT}
     *        {@link ImsCallProfile#CALL_TYPE_VT_TX}
     *        {@link ImsCallProfile#CALL_TYPE_VT_RX}
     *        {@link ImsCallProfile#CALL_TYPE_VT_NODIR}
     *        {@link ImsCallProfile#CALL_TYPE_VS}
     *        {@link ImsCallProfile#CALL_TYPE_VS_TX}
     *        {@link ImsCallProfile#CALL_TYPE_VS_RX}
     * @return a {@link ImsCallProfile} object
     */
    @Override
    public ImsCallProfile createCallProfile(int callSessionType, int callType) {
        if (mTracker == null) {
            loge("createCallProfile :: Null ImsServiceClassTracker!");
            return null;
        }
        //TODO: Check if IMS is registered
        //TODO: Check if callType is supported i.e UNSOL_SRV_STATUS_UPDATE
        ImsCallProfile profile = new ImsCallProfile(callSessionType, callType);
        return profile;
    }

    /**
     * Creates an {@link ImsCallSession} with the specified call profile.
     *
     * @return ImsCallSessionImplBase object or null on failure
     */
    @Override
    public ImsCallSessionImplBase createCallSession(ImsCallProfile profile) {
        ImsCallSessionImplBase session = null;
        if (mTracker == null) {
            loge("createCallSession :: Null ImsServiceClassTracker!");
        } else {
            session = mTracker.createCallSession(profile);
        }
        return session;
    }

    /**
     * Method to return the IMS Registration interface instance associated with this
     * MMTEL feature.
     */
    public ImsRegistrationImpl getImsRegistrationInterface() {
        return mImsReg;
    }

    /**
     * Sets the current UI TTY mode for the MmTelFeature.
     * @param mode An integer containing the new UI TTY Mode, can consist of
     *         {@link TelecomManager#TTY_MODE_OFF},
     *         {@link TelecomManager#TTY_MODE_FULL},
     *         {@link TelecomManager#TTY_MODE_HCO},
     *         {@link TelecomManager#TTY_MODE_VCO}
     * @param onCompleteMessage A {@link Message} to be used when the mode has been set.
     */
    @Override
    public void setUiTtyMode(int mode, Message onCompleteMessage) {
        mCi.setUiTTYMode(mode, onCompleteMessage);
    }

    /**
     * Get vendor SMS Implementaion class object
     * @return ImsSmsImplBase class object.
     */
    @Override
    public ImsSmsImplBase getSmsImplementation() {
        return mImsSms;
    }

    /**
     * Get the UT interface handle.
     * @return ImsUtImpl interface handle.
     */
    @Override
    public ImsUtImpl getUt() {
        return mImsUtImpl;
    }


    /**
     * Get the Config interface handle.
     * @return ImsConfigImpl interface handle.
     */
    public ImsConfigImpl getConfigInterface() {
        return mImsConfigImpl;
    }

    /**
     * Get the ECBM interface handle.
     * @return ImsEcbmImplBase interface handle.
     */
    @Override
    public ImsEcbmImplBase getEcbm() {
        return mImsEcbmImpl;
    }

    /**
     * Provides the MmTelFeature with the ability to return the framework Capability Configuration
     * for a provided Capability. If the framework calls {@link #changeEnabledCapabilities} and
     * includes a capability A to enable or disable, this method should return the correct enabled
     * status for capability A.
     *
     * The method checks the for capability-radioTech combo in mEnabledFeatures, and returns
     * true if the feature value is found.
     *
     * @param capability The capability that we are querying the configuration for.
     * @return true if the capability is enabled, false otherwise.
     */
    @Override
    public boolean queryCapabilityConfiguration(@MmTelCapabilities.MmTelCapability int capability,
            @ImsRegistrationImplBase.ImsRegistrationTech int radioTech) {
        logi( "queryCapabilityConfiguration :: capability=" + capability
                + " radioTech=" + radioTech);
        if (radioTech == ImsRegistrationImplBase.REGISTRATION_TECH_NONE) {
            return false;
        }
        final int feature = getFeature(capability, radioTech);
        if (feature == INVALID_FEATURE_VALUE) {
            return false;
        }
        for (int enabledFeature : mEnabledFeatures) {
            if (feature == enabledFeature) {
                return true;
            }
        }
        // If not enabled, the feature is either disabled or the status is not known.
        // Return false in either case.
        return false;
    }

    private void updateMmTelCapabilities() {
        for (int enabledFeature : mEnabledFeatures) {
            switch (enabledFeature) {
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE :
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI:
                    mMmtelCapabilities.addCapabilities(MmTelCapabilities.CAPABILITY_TYPE_VOICE);
                    break;
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE:
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI:
                    mMmtelCapabilities.addCapabilities(MmTelCapabilities.CAPABILITY_TYPE_VIDEO);
                    break;
                case ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_LTE:
                case ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_WIFI:
                    mMmtelCapabilities.addCapabilities(MmTelCapabilities.CAPABILITY_TYPE_UT);
            }
        }
        for (int disabledFeature : mDisabledFeatures) {
            switch (disabledFeature) {
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE :
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI:
                    mMmtelCapabilities.removeCapabilities(MmTelCapabilities.CAPABILITY_TYPE_VOICE);
                    break;
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE:
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI:
                    mMmtelCapabilities.removeCapabilities(MmTelCapabilities.CAPABILITY_TYPE_VIDEO);
                    break;
                case ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_LTE:
                case ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_WIFI:
                    mMmtelCapabilities.removeCapabilities(MmTelCapabilities.CAPABILITY_TYPE_UT);
            }
        }
        logd( "updateMmTelCapabilities :: mMmtelCapabilities updated to: "
                + mMmtelCapabilities);
    }

    /**
     * Local utility to notify feature capability change to Framework in a new thread.
     * notifyCapabilitiesStatusChanged is implemented in the parent class.
     */
    private void notifyFeatureCapabilityChange() {
        final Runnable r = new Runnable() {
            @Override
            public void run() {
                logd( "notifyFeatureCapabilityChange :: mMmtelCapabilities="
                        + mMmtelCapabilities);
                notifyCapabilitiesStatusChanged(mMmtelCapabilities);
            }
        };
        if (mFeatureCallbackHandler != null) {
            mFeatureCallbackHandler.post(r);
        }
    }

    /**
     * The MmTelFeature should override this method to handle the enabling/disabling of
     * MmTel Features, defined in {@link MmTelCapabilities.MmTelCapability}. The framework assumes
     * the {@link CapabilityChangeRequest} was processed successfully. If a subset of capabilities
     * could not be set to their new values,
     * {@link CapabilityCallbackProxy#onChangeCapabilityConfigurationError} must be called
     * individually for each capability whose processing resulted in an error.
     *
     * Enabling/Disabling a capability here indicates that the capability should be registered or
     * deregistered (depending on the capability change) and become available or unavailable to
     * the framework.
     *
     * This method invokes ImsConfigImpl's setFeatureValue method for each CapabilityPair in the
     * request, and keeps a track of the status of each request via the corresponding
     * SetCapabilityListener instance.
     *
     * NOTE: This IMS Service implementation does not fully support the scenario of back to back
     *       calls to this method. To support such functionality, this class would need to
     *       implement a CapabilityCallbackProxy token system, where each SetCapabilityListener
     *       instance would have to get the appropriate CapabilityCallbackProxy instance to call
     *       its callbacks on.
     */
    @Override
    public void changeEnabledCapabilities(CapabilityChangeRequest request,
            CapabilityCallbackProxy c) {
        if (request == null || c == null) {
            loge("changeEnabledCapabilities :: Invalid argument(s).");
            return;
        }

        List<CapabilityPair> capsToEnable = request.getCapabilitiesToEnable();
        List<CapabilityPair> capsToDisable = request.getCapabilitiesToDisable();
        if (capsToEnable.isEmpty() && capsToDisable.isEmpty()) {
            loge("changeEnabledCapabilities :: No CapabilityPair objects to process!");
            return;
        }

        mCapabilityCallback = c;

        ArrayList<CapabilityStatus> capabilityStatusList = new ArrayList<>();
        for (CapabilityPair cp : capsToEnable) {
            CapabilityStatus cs = new CapabilityStatus(cp.getCapability(), cp.getRadioTech(),
                    ImsConfig.FeatureValueConstants.ON);
            capabilityStatusList.add(cs);
        }

        for (CapabilityPair cp : capsToDisable) {
            CapabilityStatus cs = new CapabilityStatus(cp.getCapability(), cp.getRadioTech(),
                    ImsConfig.FeatureValueConstants.OFF);
            capabilityStatusList.add(cs);

        }

        mImsConfigImpl.setCapabilityValue(capabilityStatusList,
                new SetCapabilityListener());
    }

    /**
     * Utility method to map MmTelCapabilities and ImsRegistrationBaseImpl
     * values for capabilities and RAT.
     *
     * @param capability Has one of the following values:
     * {@link MmTelFeature.MmTelCapabilities#CAPABILITY_TYPE_VOICE},
     * {@link MmTelFeature.MmTelCapabilities#CAPABILITY_TYPE_VIDEO},
     * {@link MmTelFeature.MmTelCapabilities#CAPABILITY_TYPE_UT}, or
     * {@link MmTelFeature.MmTelCapabilities#CAPABILITY_TYPE_SMS},
     *
     * @param radioTech Has one of the following values:
     * {@link ImsRegistrationImplBase#REGISTRATION_TECH_LTE} or
     * {@link ImsRegistrationImplBase#REGISTRATION_TECH_IWLAN}
     *
     * @return The appropriate ImsConfig.FeatureConstants value, if a mapping
     *         exists, else -2.
     */
    private static int getFeature(@MmTelCapabilities.MmTelCapability int capability,
            @ImsRegistrationImplBase.ImsRegistrationTech int radioTech) {
        if (radioTech != ImsRegistrationImplBase.REGISTRATION_TECH_LTE &&
                radioTech != ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN) {
            return INVALID_FEATURE_VALUE;
        }
        switch (capability) {
            case MmTelCapabilities.CAPABILITY_TYPE_VOICE:
                return radioTech == ImsRegistrationImplBase.REGISTRATION_TECH_LTE ?
                        ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE
                        : ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI;
            case MmTelCapabilities.CAPABILITY_TYPE_VIDEO:
                return radioTech == ImsRegistrationImplBase.REGISTRATION_TECH_LTE ?
                        ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE
                        : ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI;
            case MmTelCapabilities.CAPABILITY_TYPE_UT:
                return radioTech == ImsRegistrationImplBase.REGISTRATION_TECH_LTE ?
                        ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_LTE
                        : ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_WIFI;
            case MmTelCapabilities.CAPABILITY_TYPE_SMS:
                /* TODO: Add logic here, to support SMS capability.
                  queryCapabilityConfiguration() does not have SMS support
                  in AOSP as of now. So corsponding vendor support here will
                  be added later. */
                return INVALID_FEATURE_VALUE;
            default:
                return INVALID_FEATURE_VALUE;
        }
    }

    private class SetCapabilityListener implements ImsConfigImpl.SetCapabilityValueListener {
        @Override
        public void onSetCapabilityValueSuccess(int capability, int radioTech, int value) {
            logd( "onSetCapabilityValueSuccess :: capability=" + capability
                    + " radioTech=" + radioTech + " value=" + value);
            // NOTE: Framework expects IMS Service to only report failures, so we don't
            // report anything at this point.
        }

        @Override
        public void onSetCapabilityValueFailure(int capability, int radioTech,
                ImsConfigImpl.SetCapabilityFailCause reason) {
            logd( "onSetCapabilityValueFailure :: capability=" + capability
                    + " radioTech=" + radioTech + " reason=" + reason);
            if (mCapabilityCallback == null) {
                loge("onSetCapabilityValueFailure :: Null mCapabilityCallback!");
                return;
            }
            mCapabilityCallback.onChangeCapabilityConfigurationError(capability, radioTech,
                    getSetCapabilityFailError(reason));
        }
    }

    private static @ImsCapabilityError int getSetCapabilityFailError(
            ImsConfigImpl.SetCapabilityFailCause reason) {
        int error = ImsFeature.CAPABILITY_ERROR_GENERIC;
        switch (reason) {
            case ERROR_GENERIC:
                error = ImsFeature.CAPABILITY_ERROR_GENERIC;
                break;
            case ERROR_SUCCESS:
                error = ImsFeature.CAPABILITY_SUCCESS;
        }
        return error;
    }

    void onIccLoaded() {
        logd( "onIccLoaded...");
        notifyFeatureCapabilityChange();
    }

    private boolean isSrvTypeValid(int type) {
        // checking UT and SMS condition separately
        if (type == CallDetails.CALL_TYPE_UT || type == CallDetails.CALL_TYPE_SMS) {
            return true;
        } else {
            return ((type < CallDetails.CALL_TYPE_VOICE) ||
                     (type > CallDetails.CALL_TYPE_VT)) ? false
                    : true;
        }
    }

    private void resetFeatures() {
        for (int i = 0; i < mEnabledFeatures.length; i++) {
            mEnabledFeatures[i] = ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN;
            mDisabledFeatures[i] = ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN;
        }
        mMmtelCapabilities = new MmTelCapabilities(0);
    }

    private void handleSrvStatusUpdate(ArrayList<ServiceStatus> updateList) {
        boolean isVideoEnabled = false;
        resetFeatures();
        for (ServiceStatus update : updateList) {
            logi( "type = " + update.type + " status = " + update.status
                    + " isValid = " + update.isValid);
            if (update != null && update.isValid && isSrvTypeValid(update.type)) {
                ServiceStatus srvSt = null;
                if (update.type == CallDetails.CALL_TYPE_UT) {
                    srvSt = mServiceStatus[SERVICE_TYPE_MAX -1];
                } else {
                    srvSt = mServiceStatus[update.type];
                }
                srvSt.isValid = update.isValid;
                srvSt.type = update.type;
                if (update.userdata != null) {
                    srvSt.userdata = new byte[update.userdata.length];
                    srvSt.userdata = Arrays.copyOf(update.userdata, update.userdata.length);
                }
                if (update.accessTechStatus != null && update.accessTechStatus.length > 0) {
                    srvSt.accessTechStatus = new ServiceStatus.StatusForAccessTech[update.
                            accessTechStatus.length];
                    logi( "Call Type " + srvSt.type + "has num updates = "
                            + update.accessTechStatus.length);
                    ServiceStatus.StatusForAccessTech[] actSt = srvSt.accessTechStatus;

                    for (int i = 0; i < update.accessTechStatus.length; i++) {
                        ServiceStatus.StatusForAccessTech actUpdate =
                                update.accessTechStatus[i];
                        logi( "StatusForAccessTech networkMode = "
                                + actUpdate.networkMode
                                + " registered = " + actUpdate.registered
                                + " status = " + actUpdate.status
                                + " restrictCause = " + actUpdate.restrictCause);
                        actSt[i] = new ServiceStatus.StatusForAccessTech();
                        actSt[i].networkMode = actUpdate.networkMode;
                        actSt[i].registered = actUpdate.registered;
                        if (actUpdate.status == ServiceStatus.STATUS_ENABLED &&
                                actUpdate.restrictCause != CallDetails.CALL_RESTRICT_CAUSE_NONE) {
                            actSt[i].status = ServiceStatus.STATUS_PARTIALLY_ENABLED;
                        } else {
                            actSt[i].status = actUpdate.status;
                        }
                        srvSt.status = actSt[i].status;
                        actSt[i].restrictCause = actUpdate.restrictCause;
                        int feature;
                        boolean modeWifi = actSt[i].networkMode == RadioTech.RADIO_TECH_WIFI ||
                                actSt[i].networkMode == RadioTech.RADIO_TECH_IWLAN;
                        boolean modeLte = actSt[i].networkMode == RadioTech.RADIO_TECH_ANY ||
                                actSt[i].networkMode == RadioTech.RADIO_TECH_NR5G ||
                                actSt[i].networkMode == RadioTech.RADIO_TECH_LTE;
                        if (modeWifi || modeLte) {
                            if (update.type == CallDetails.CALL_TYPE_VOICE) {
                                feature = modeLte ?
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE :
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI;
                            } else if (update.type == CallDetails.CALL_TYPE_UT) {
                                 feature = modeLte ?
                                         ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_LTE:
                                         ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_WIFI;
                            } else if (update.type == CallDetails.CALL_TYPE_VT_TX ||
                                           update.type == CallDetails.CALL_TYPE_VT_RX ||
                                           update.type == CallDetails.CALL_TYPE_VT) {
                                feature = modeLte ?
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE :
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI;
                            } else {
                                feature = ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN;
                            }
                            if (actSt[i].status == ServiceStatus.STATUS_ENABLED ||
                                    actSt[i].status == ServiceStatus.STATUS_PARTIALLY_ENABLED) {
                                if (update.type == CallDetails.CALL_TYPE_SMS &&
                                        mCi.isSmsSupported()) {
                                    // For SMS over IMS, only STATUS_ENABLED is capability enabled
                                    logd("SMS service status = " + actSt[i].status);
                                    if (actSt[i].status == ServiceStatus.STATUS_ENABLED) {
                                        mMmtelCapabilities.addCapabilities(
                                            MmTelCapabilities.CAPABILITY_TYPE_SMS);
                                    } else {
                                        mMmtelCapabilities.removeCapabilities(
                                            MmTelCapabilities.CAPABILITY_TYPE_SMS);
                                    }
                                }
                                if (feature != ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN) {
                                    mEnabledFeatures[feature] = feature;
                                    mDisabledFeatures[feature] =
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN;
                                }
                                if (feature ==
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE ||
                                        feature ==
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI) {
                                    isVideoEnabled = true;
                                }
                                logi( "enabledFeature = " + feature);
                            } else if (actSt[i].status == ServiceStatus.STATUS_DISABLED ||
                                    actSt[i].status == ServiceStatus.STATUS_NOT_SUPPORTED) {
                                if (update.type == CallDetails.CALL_TYPE_SMS &&
                                        mCi.isSmsSupported()) {
                                    logd("SMS service status = " + actSt[i].status);
                                    mMmtelCapabilities.removeCapabilities(
                                        MmTelCapabilities.CAPABILITY_TYPE_SMS);
                                }
                                // VT is enabled if VT_TX, or VT_RX, or VT is enabled
                                if ((!(isVideoEnabled == true && (feature ==
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE ||
                                        feature ==
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI)))
                                        && feature !=
                                        ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN) {
                                    mDisabledFeatures[feature] = feature;
                                    mEnabledFeatures[feature] =
                                            ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN;
                                    logi( "disabledFeature = " + feature);
                                }
                            }
                        }
                    }
                }
            }
        }
        // NOTE: SMS over IMS feature-support information is not processed in this method.
        // Support for the same will need to be added from RIL upwards.
        updateMmTelCapabilities();
        notifyFeatureCapabilityChange();

        if (mTracker != null) {
            mTracker.updateFeatureCapabilities(isVideoSupported(), isVoiceSupported());
        } else {
            loge("handleSrvStatusUpdate :: tracker null; not updating global VT capability");
        }

        if (mImsServiceStateReceiver != null) {
            mImsServiceStateReceiver.updateHDIcon(isVideoSupported(), isVoiceSupported());
        } else {
            loge("handleSrvStatusUpdate :: mImsServiceStateReceiver null");
        }

        // Notify MultiEndPoint listeners on voice/video capability change.
        // This comes in form of service status change.
        if (mCapabilitiesChangedRegistrants != null) {
            Pair<Boolean, Boolean> result =
                     new Pair<Boolean, Boolean>(isVideoSupported(), isVoiceSupported());
            mCapabilitiesChangedRegistrants.notifyRegistrants(new AsyncResult(null, result, null));
        } else {
            loge("handleSrvStatusUpdate :: mCapabilitiesChangedRegistrants null");
        }
    }

    public boolean isVideoSupported() {
        return mEnabledFeatures[ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE]
                    == ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE
                || mEnabledFeatures[ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI]
                    == ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI;
    }

    public boolean isVoiceSupported() {
        return mEnabledFeatures[ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE]
                    == ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE
                || mEnabledFeatures[ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI]
                    == ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI;
    }

    public boolean isVideoSupportedOverWifi() {
        return mEnabledFeatures[ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI]
                == ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI;
    }

    public boolean isVoiceSupportedOverWifi() {
        return mEnabledFeatures[ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI]
                == ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI;
    }

    public void updateLowBatteryStatus() {
        if (mTracker != null) {
            mTracker.updateLowBatteryStatus();
        }
    }

    public void setCallForwardUncondTimer(int startHour, int startMinute, int endHour,
        int endMinute, int action, int condition, int serviceClass, String number,
                IQtiImsExtListener listener) {
        logi( "setCallForwardUncondTimer");
        mCi.setCallForwardUncondTimer(startHour, startMinute, endHour, endMinute,
                action, condition, serviceClass, number,
                mHandler.obtainMessage(EVENT_SET_CALL_FORWARD_TIMER, listener));
    }

    public void getCallForwardUncondTimer(int reason, int serviceClass,
            IQtiImsExtListener listener) {
        logi( "getCallForwardUncondTimer reason is"+ reason
                + "serviceClass = " + serviceClass);
        mCi.queryCallForwardStatus(reason, serviceClass, null,
                mHandler.obtainMessage(EVENT_GET_CALL_FORWARD_TIMER, listener));
    }

    public void setCallBarring(boolean operationType, String facility, String[] cbNumListInfo,
            String password, int serviceClass, IQtiImsExtListener listener) {
        logi( "setCallBarring operationType: " + operationType + " facility: " + facility);
        mCi.suppSvcStatus(operationType ? SuppSvcResponse.ACTIVATE : SuppSvcResponse.DEACTIVATE,
                getCBTypeFromFacility(facility), cbNumListInfo, password, serviceClass,
                mHandler.obtainMessage(EVENT_SET_CALL_BARRING, listener));
    }

    private static int getCBTypeFromFacility(String facility) {
        if (ImsPhoneCommandsInterface.CB_FACILITY_BAOC.equals(facility)) {
            return SuppSvcResponse.FACILITY_BAOC;
        } else if (ImsPhoneCommandsInterface.CB_FACILITY_BAOIC.equals(facility)) {
            return SuppSvcResponse.FACILITY_BAOIC;
        } else if (ImsPhoneCommandsInterface.CB_FACILITY_BAOICxH.equals(facility)) {
            return SuppSvcResponse.FACILITY_BAOICxH;
        } else if (ImsPhoneCommandsInterface.CB_FACILITY_BAIC.equals(facility)) {
            return SuppSvcResponse.FACILITY_BAIC;
        } else if (ImsPhoneCommandsInterface.CB_FACILITY_BAICr.equals(facility)) {
            return SuppSvcResponse.FACILITY_BAICr;
        } else if (ImsPhoneCommandsInterface.CB_FACILITY_BA_ALL.equals(facility)) {
            return SuppSvcResponse.FACILITY_BA_ALL;
        } else if (ImsPhoneCommandsInterface.CB_FACILITY_BA_MO.equals(facility)) {
            return SuppSvcResponse.FACILITY_BA_MO;
        } else if (ImsPhoneCommandsInterface.CB_FACILITY_BA_MT.equals(facility)) {
            return SuppSvcResponse.FACILITY_BA_MT;
        }
        return SuppSvcResponse.INVALID;
    }

    public void sendCancelModifyCall(IQtiImsExtListener listener) {
        /* Get the callsession for active call */
        List<ImsCallSessionImpl> sessionList =
                getCallSessionByState(DriverCallIms.State.ACTIVE);

        if (sessionList.isEmpty()) {
            /* No call to cancel, notify error */
            Log.i (this, "sendCancelModifyCall: no call is available to cancel modify call");
            if (listener != null) {
                try {
                    listener.receiveCancelModifyCallResponse(mPhoneId,
                            QtiImsExtUtils.QTI_IMS_REQUEST_ERROR);
                } catch (Throwable t) {
                    loge("sendCancelModifyCall exception!");
                }
            } else {
                Log.i (this, "sendCancelModifyCall: no listener is available");
            }
        } else {
            /* Retrieve the call id to be cancelled modify call */
            int nCallId = Integer.parseInt(sessionList.get(0).getCallId());
            Log.i (this, "sendCancelModifyCall: call ID " + nCallId);

            /* Send the cancel modify call request to lower layers */
            mCi.cancelModifyCall(mHandler.obtainMessage(EVENT_CANCEL_MODIFY_CALL,
                    listener), nCallId);
        }
    }

    public void setUssdInfoListener(IQtiImsExtListener listener) {
        logv( "setUssdInfoListener");
        mUssdInfoListener = listener;
    }

    public void resumePendingCall(int videoState) {
        /* Get the callsession for dialing call */
        List<ImsCallSessionImpl> sessionList =
                getCallSessionByState(DriverCallIms.State.DIALING);

        if (sessionList.isEmpty()) {
            logd( "resumePendingCall: no call is available");
            return;
        }
        sessionList.get(0).resumePendingCall(videoState);
    }

    private int convertQtiImsTransferTypeToImsQmiType(int type) {
        switch(type) {
            case QtiImsExtUtils.QTI_IMS_BLIND_TRANSFER:
                return EctTypeInfo.BLIND_TRANSFER;
            case QtiImsExtUtils.QTI_IMS_ASSURED_TRANSFER:
                return EctTypeInfo.ASSURED_TRANSFER;
            case QtiImsExtUtils.QTI_IMS_CONSULTATIVE_TRANSFER:
                return EctTypeInfo.CONSULTATIVE_TRANSFER;
        }
        return type;
    }

    public void sendCallTransferRequest(int type, String number,
                IQtiImsExtListener listener) {
        boolean returnError = true;

        switch(type) {
            /* Blind and assured transfer are the same, hence fall through */
            case QtiImsExtUtils.QTI_IMS_BLIND_TRANSFER:
            case QtiImsExtUtils.QTI_IMS_ASSURED_TRANSFER:
                if (number == null) {
                    /* Number cannot be null */
                    loge ("sendCallTransferRequest: no target number");
                } else {
                    /* Check for an active callsession */
                    List<ImsCallSessionImpl> sessionList = getCallSessionByState(
                            DriverCallIms.State.ACTIVE);
                    if (sessionList.isEmpty()) {
                        /* If no active call, then check for held callsession */
                        sessionList = getCallSessionByState(DriverCallIms.State.HOLDING);
                    }

                    if (sessionList.isEmpty()) {
                        /* No call to transfer, return error */
                        loge ("sendCallTransferRequest: no call available");
                    } else if (sessionList.get(0).canTransferToNumber()) {
                        /* Send the call transfer request to lower layers */
                        returnError = false;
                        mCi.explicitCallTransfer(
                                Integer.parseInt(sessionList.get(0).getCallId()),
                                convertQtiImsTransferTypeToImsQmiType(type), number, 0,
                                mHandler.obtainMessage(EVENT_CALL_TRANSFER, listener));
                    } else {
                        /* Calls are available, but not allowed to transfer */
                        loge ("sendCallTransferRequest: transfer not allowed " +
                                sessionList.get(0));
                    }
                }
                break;
            case QtiImsExtUtils.QTI_IMS_CONSULTATIVE_TRANSFER:
                {
                    /* Get the callsession for active call */
                    List<ImsCallSessionImpl> sessionList =
                            getCallSessionByState(DriverCallIms.State.ACTIVE);
                    if (sessionList.isEmpty()) {
                        /* There is no active, return error */
                        loge ("sendCallTransferRequest: no active call available");
                    } else if (sessionList.get(0).canTransferToId()) {
                        /* check for held call */
                        int nCallId = Integer.parseInt(sessionList.get(0).getCallId());
                        sessionList = getCallSessionByState(DriverCallIms.State.HOLDING);
                        if (sessionList.isEmpty()) {
                            /* There is no held call, return error */
                            loge ("sendCallTransferRequest: no held call available");
                        } else if (sessionList.get(0).canTransferToId()) {
                            /* Send transfer request */
                            returnError = false;
                            mCi.explicitCallTransfer(
                                    Integer.parseInt(sessionList.get(0).getCallId()),
                                    convertQtiImsTransferTypeToImsQmiType(type), null, nCallId,
                                    mHandler.obtainMessage(EVENT_CALL_TRANSFER, listener));
                        } else {
                            loge ("sendCallTransferRequest: held call transfer is" +
                                    "not allowed " + sessionList.get(0));
                        }
                    } else {
                        /* no support for conference call, return error */
                        loge ("sendCallTransferRequest: active call transfer is" +
                                "not allowed " + sessionList.get(0));
                    }
                }
                break;
            default:
                break;
        }

        /* if identified as error case, send error response */
        if (returnError) {
            /* Call transfer failure, notify error */
            loge("sendCallTransferRequest: Type " + type + " Failed");
            if (listener != null) {
                try {
                    listener.receiveCallTransferResponse(mPhoneId,
                            QtiImsExtUtils.QTI_IMS_REQUEST_ERROR);
                } catch (Throwable t) {
                    loge("sendCallTransferRequest: exception!");
                }
            } else {
                loge("sendCallTransferRequest: no listener is available");
            }
        }
    }

    public void getHandoverConfig(IQtiImsExtListener listener) {
        mCi.getHandoverConfig(mHandler.obtainMessage(EVENT_GET_HANDOVER_CONFIG,
                listener));
    }

    public void setHandoverConfig(int hoConfig, IQtiImsExtListener listener) {
        mCi.setHandoverConfig(hoConfig,
                mHandler.obtainMessage(EVENT_SET_HANDOVER_CONFIG, listener));
    }

    /**
     * Check if the exception is due to Radio not being available
     * @param e
     * @return true, if exception is due to RADIO_NOT_AVAILABLE
     */
    private boolean isImsExceptionRadioNotAvailable(Throwable e) {
        return e != null
                && e instanceof RuntimeException
                && ((RuntimeException) e).getMessage().equals(
                        ImsSenderRxr.errorIdToString(ImsErrorCode.RADIO_NOT_AVAILABLE));
    }

    /**
     * Check if given call can be transferred or not
     * @param dc, an instance of DriverCallIms
     * @return true, if given call is transferrable
     *         false, otherwise
     */
    private boolean canTransfer(DriverCallIms dc) {
        /**
         * For conference calls, transfer is not supported in any call state
         * For individual calls, transfer is possible only in active and held state
         */
        return (dc != null) && !dc.isMpty &&
                (dc.state == DriverCallIms.State.ACTIVE ||
                dc.state == DriverCallIms.State.HOLDING);
    }

    private void updateCallTransferInfo(ArrayList<DriverCallIms> dcList) {
        /* return if the corresponding config is disabled */
        if (!QtiImsExtUtils.isCallTransferEnabled(mContext)) {
            return;
        }

        int nTransferableCalls = 0;
        /* Get the count of the possible transferable calls */
        for (DriverCallIms dc: dcList) {
            if (canTransfer(dc)) {
                nTransferableCalls++;
            }
        }
        logi( "Transfer is possible for " + nTransferableCalls + " calls");

        /**
         * If number of transferable calls is
         * 1 - only one call is available to transfer, so can only transfer to number
         * 2 - two calls are available to transfer, so can transfer to both number and id
         * otherwise - not possible for any transfer.
         */
        switch (nTransferableCalls) {
            /* update that call transfer to a number is allowed */
            case 1:
                for (DriverCallIms dc: dcList) {
                    if (canTransfer(dc)) {
                        dc.mEctMask = QtiImsExtUtils.QTI_IMS_BLIND_TRANSFER |
                                QtiImsExtUtils.QTI_IMS_ASSURED_TRANSFER;
                    }
                }
                break;
            /* update that call transfer to a number & id are allowed */
            case 2:
                for (DriverCallIms dc: dcList) {
                    if (canTransfer(dc)) {
                        dc.mEctMask = QtiImsExtUtils.QTI_IMS_BLIND_TRANSFER |
                                QtiImsExtUtils.QTI_IMS_ASSURED_TRANSFER |
                                QtiImsExtUtils.QTI_IMS_CONSULTATIVE_TRANSFER;
                    }
                }
                break;
            /* No call transfer is possible */
            default:
                break;
        } /* end of switch */

        return;
    }

    /**
     * Handle the calls returned as part of UNSOL_CALL_STATE_CHANGED
     * @param ar - the AsyncResult object that contains the call list information
     */
    private void handleCalls(AsyncResult ar) {
        ArrayList<DriverCallIms> callList;
        logi( ">handleCalls");
        Map<Integer, DriverCallIms> dcList = new HashMap<Integer, DriverCallIms>();

        if (ar.exception == null) {
            callList = (ArrayList<DriverCallIms>) ar.result;
        } else if (isImsExceptionRadioNotAvailable(ar.exception)) {
            // just a dummy empty ArrayList to cause the loop
            // to hang up all the calls
            callList = new ArrayList<DriverCallIms> ();
        } else {
            // Radio probably wasn't ready--try again in a bit
            // But don't keep polling if the channel is closed
            return;
        }

        ArrayList<DriverCallIms> mmTelList = new ArrayList<DriverCallIms> ();

        if (callList != null) {
            for (DriverCallIms dc : callList) {
                logi( "handleCalls: dc = " + dc);
                mmTelList.add(dc);
            }
        }

        if (mTracker == null) {
            loge("handleCalls :: Null mTracker!");
        } else {
            /* Check upon the possibility of call transfer support */
            updateCallTransferInfo(mmTelList);
            // service class will be filtered here and only calls with service class MMTEL will go to tracker
            mTracker.handleCalls((ArrayList<DriverCallIms>) mmTelList);
        }
    }

    /**
     * Handle intra RAT Handovers as part of UNSOL_RESPONSE_HANDOVER
     * @param ar - the AsyncResult object that contains handover information
     */
    private void handleHandover(AsyncResult ar) {
        logi( "handleHandover");
        if (ar.exception == null) {
            HoInfo handover = (HoInfo) ar.result;
            if (mTracker != null) {
                mTracker.handleHandover(handover);
            }
        }
        else {
            loge("AsyncResult exception in handleHandover- " + ar.exception);
        }
    }

    /**
     * Handle the call state changes for incoming (MT) Hold/Resume as part of
     * the UNSOL_SUPP_SVC_NOTIFICATION message.
     * @param ar - the AsyncResult object that contains the call list information
     */
    private void handleSuppSvc(AsyncResult ar) {
        logi( "handleSuppSvc");
        if (ar.exception == null) {
            SuppNotifyInfo supp_svc_unsol = (SuppNotifyInfo) ar.result;
            if (mTracker != null) {
                mTracker.handleSuppSvcUnsol(supp_svc_unsol);
            }
        }
        else {
            loge("AsyncResult exception in handleSuppSvc.");
        }
    }

    /**
     * Handle the TTY mode changes as part of the UNSOL_TTY_NOTIFICATION message.
     * @param ar - the AsyncResult object that contains new TTY mode.
     */
    private void handleTtyModeChange(AsyncResult ar) {
        logi( "handleTtyModeChange");
        if (ar != null && ar.result != null && ar.exception == null) {
            int[] mode = (int[])ar.result;
            loge("Received EVENT_TTY_STATE_CHANGED mode= " + mode[0]);
            if (mTracker != null) {
                mTracker.handleTtyModeChangeUnsol(mode[0]);
            }
        } else {
            loge("Error EVENT_TTY_STATE_CHANGED AsyncResult ar= " + ar);
        }
     }

    /**
     * Gets a call session with give media id.
     * @param mediaId Media id of the session to be searched.
     * @return Call session with {@code mediaId}
     */
    public List<ImsCallSessionImpl> getCallSessionByState(DriverCallIms.State state) {
        return mTracker == null ? Collections.EMPTY_LIST : mTracker.getCallSessionByState(state);
    }

    /**
     * Gets a call session with give media id.
     * @param mediaId Media id of the session to be searched.
     * @return Call session with {@code mediaId}
     */
    public ImsCallSessionImpl findSessionByMediaId(int mediaId) {
        return mTracker == null ? null : mTracker.findSessionByMediaId(mediaId);
    }

    /**
     * @return This instance's Phone ID.
     */
    public int getPhoneId() {
        return mPhoneId;
    }

    /**
     * Registers call list listener.
     * Note: Only {@code ImsServiceClass.MMTEL} is supported.
     * @param listener Listener to registered
     * @see ICallListListener
     */
    public void addListener(ICallListListener listener) {
        if (listener == null) {
            throw new IllegalArgumentException("addListener error: listener is null.");
        }

        if (!mCallListListeners.contains(listener)) {
            mCallListListeners.add(listener);
        } else {
            Log.w(this,"addListener error: Duplicate listener, " + listener);
        }
    }

    /**
     * Unregisters call list listener.
     * Note: Only {@code ImsServiceClass.MMTEL} is supported.
     * @param listener Listener to unregistered
     * @see ICallListListener
     */
    public void removeListener(ICallListListener listener) {
        if (listener == null) {
            throw new IllegalArgumentException("removeListener error: listener is null.");
        }

        if (mCallListListeners.contains(listener)) {
            mCallListListeners.remove(listener);
        } else {
            Log.w(this,"removeListener error: Listener not found, " + listener);
        }
    }

    /**
     * Handles Conference refresh Info notified through UNSOL_REFRESH_CONF_INFO message
     * @param ar - the AsyncResult object that contains the refresh Info information
     */
    public void handleRefreshConfInfo(AsyncResult ar) {
        logi( "handleRefreshConfInfo");
        if ((ar != null) && (ar.exception == null) && (ar.result != null)) {
            ConfInfo confInfo = (ConfInfo) ar.result;
            if (mTracker != null) {
                mTracker.handleRefreshConfInfo(confInfo);
            }
        } else {
            if (ar != null) {
                loge("Failed @handleRefreshConfInfo --> " + ar.exception
                        + "with result = " + ar.result);
            } else {
                loge("Failed @handleRefreshConfInfo --> AsyncResult is null");
            }
        }
    }

    private void handleModifyCallRequest(CallModify cm) {
        logi( "handleCallModifyRequest(" + cm + ")");
        if (mTracker != null) {
            mTracker.handleModifyCallRequest(cm);
        }
    }

    private void sendBroadcastForDisconnected(ImsReasonInfo imsReasonInfo, int regState){
        Intent intent = new Intent("com.android.imsconnection.DISCONNECTED");
        intent.putExtra("result", (Parcelable)imsReasonInfo);
        intent.putExtra("stateChanged", regState);
        mContext.sendStickyBroadcastAsUser(intent, UserHandle.ALL);
        logi("sendBroadcastForDisconnected");
    }

    @VisibleForTesting
    public Handler getHandler() {
        return mHandler;
    }

    @VisibleForTesting
    public ImsServiceClassTracker getImsServiceClassTracker() {
        return mTracker;
    }

    @VisibleForTesting
    public boolean getFeatureIsOpen() {
        return mFeatureIsOpen;
    }

    //Handler for the events on response from ImsSenderRxr
    private class ImsServiceSubHandler extends Handler {
        ImsServiceSubHandler() {
            super();
        }

        /**
         * Local utility to call registration callbacks in new threads.
         */
        private void notifyRegChange(final int registrationState,
                final ImsReasonInfo imsReasonInfo, final int imsRadioTech) {
            final Runnable r = new Runnable() {
                @Override
                public void run() {
                    switch (registrationState) {
                        case ImsRegistrationInfo.REGISTERED:
                            mImsReg.registeredWithRadioTech(imsRadioTech);
                            break;
                        case ImsRegistrationInfo.NOT_REGISTERED:
                            mImsReg.registrationDisconnected(imsReasonInfo);
                            break;
                        case ImsRegistrationInfo.REGISTERING:
                            mImsReg.registeringWithRadioTech(imsRadioTech);
                            break;
                    }
                }
            };
            if (mFeatureCallbackHandler != null) {
                mFeatureCallbackHandler.post(r);
            }
        }

        private void notifyRegAssociatedUriChange(final Uri[] self_identity) {
            final Runnable r = new Runnable() {
                @Override
                public void run() {
                    mImsReg.registrationAssociatedUriChanged(self_identity);
                }
            };
            if (mFeatureCallbackHandler != null) {
                mFeatureCallbackHandler.post(r);
            }
        }

        private void handleImsStateChanged(AsyncResult ar) {
            logi("handleImsStateChanged");
            int errorCode = ImsReasonInfo.CODE_UNSPECIFIED;
            String errorMessage = null;
            String selfIdentityUrisCombined = null;
            int regState = ImsRegistrationInfo.NOT_REGISTERED;
            int imsRat = ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN;
            String[] selfIdentityUriStrings = null;
            Uri[] selfIdentityUris = null;
            if (ar != null && ar.exception == null && ar.result instanceof ImsRegistrationInfo) {
                ImsRegistrationInfo registration = (ImsRegistrationInfo) ar.result;

                errorCode = registration.getErrorCode();
                errorMessage = registration.getErrorMessage();
                selfIdentityUrisCombined = registration.getPAssociatedUris();
                regState = registration.getState();
                imsRat = getRilRadioTech(registration);
                if (regState != ImsRegistrationInfo.NOT_REGISTERED) {
                    mCi.queryServiceStatus(mHandler.obtainMessage(EVENT_GET_SRV_STATUS));
                }
                if (selfIdentityUrisCombined != null) {
                    selfIdentityUris =
                            extractUrisFromPipeSeparatedUriStrings(selfIdentityUrisCombined);
                }
            } else {
                loge("handleImsStateChanged error");
            }

            ImsReasonInfo imsReasonInfo = new ImsReasonInfo(
                    ImsReasonInfo.CODE_REGISTRATION_ERROR,
                    errorCode, errorMessage);

            int wfcRegState = regState;
            if (imsRat == ServiceState.RIL_RADIO_TECHNOLOGY_LTE) {
                wfcRegState = ImsRegistrationInfo.NOT_REGISTERED;
                logi( "VOLTE ims registered, WFC change to Not  registered");
            }
            sendBroadcastForDisconnected(imsReasonInfo, wfcRegState);

            notifyRegChange(regState, imsReasonInfo, imsRat);

            if (selfIdentityUris != null && selfIdentityUrisHaveChanged(selfIdentityUris)) {
                updateSelfIdentityUriCache(selfIdentityUris);
                notifyRegAssociatedUriChange(selfIdentityUris);
            }
        }

        private int getRilRadioTech(ImsRegistrationInfo registration) {
            if (registration.getRadioTech() == RadioTech.RADIO_TECH_INVALID) {
                return ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN;
            }

            int imsRat;
            switch (registration.getRadioTech()) {
                case RadioTech.RADIO_TECH_NR5G:
                case RadioTech.RADIO_TECH_LTE:
                    imsRat = ServiceState.RIL_RADIO_TECHNOLOGY_LTE;
                break;
                case RadioTech.RADIO_TECH_WIFI:
                case RadioTech.RADIO_TECH_IWLAN:
                    imsRat = ServiceState.RIL_RADIO_TECHNOLOGY_IWLAN;
                break;
                default:
                   imsRat = ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN;
            }

            return imsRat;
        }

        private Uri[] extractUrisFromPipeSeparatedUriStrings(String combinedUris) {
            if (combinedUris == null || combinedUris.length() <= 1) return null;
            String[] uriStrings = combinedUris.split("\\|");
            Uri[] uris = new Uri[uriStrings.length];
            for (int i = 0; i < uriStrings.length; i++) {
                uris[i] = Uri.parse(uriStrings[i]);
            }
            return uris;
        }

        private boolean selfIdentityUrisHaveChanged(Uri[] new_uris) {
            if (mSelfIndentityUris == null) return true;
            boolean changed = false;
            if ((new_uris.length != mSelfIndentityUris.size())
                    || (mSelfIndentityUris == null)) {
                changed = true;
            } else {
                for (int i = 0; i < new_uris.length; i++) {
                    if (!mSelfIndentityUris.contains(new_uris[i])) {
                        changed = true;
                        break;
                    }
                }
            }
            return changed;
        }

        private void updateSelfIdentityUriCache(Uri[] new_uris) {
            if (mSelfIndentityUris == null) {
                mSelfIndentityUris = new HashSet<Uri>();
            } else {
                mSelfIndentityUris.clear();
            }
            for (int i = 0; i < new_uris.length; i++) {
                mSelfIndentityUris.add(new_uris[i]);
                Log.i (this, "updateSelfIdentityUriCache :: "
                        + "new self-identity host URI=" + new_uris[i]);
            }
        }

        @Override
        public void handleMessage(Message msg) {
            Log.i (this, "Message received: what = " + msg.what);
            AsyncResult ar;

            switch (msg.what) {
                case EVENT_IMS_STATE_CHANGED:
                    ar = (AsyncResult) msg.obj;
                    if (ar.result == null) {
                        // Backward compatibility with old RIL API
                        mCi.getImsRegistrationState(this.obtainMessage(EVENT_IMS_STATE_DONE));
                    } else {
                        handleImsStateChanged(ar);
                    }
                    break;
                case EVENT_IMS_STATE_DONE:
                    ar = (AsyncResult) msg.obj;
                    handleImsStateChanged(ar);
                    break;
                case EVENT_SET_IMS_STATE:
                    ar = (AsyncResult) msg.obj;
                    if ( ar.exception != null ) {
                        logi( "Request turn on/off IMS failed");
                    }
                    break;
                case EVENT_SRV_STATUS_UPDATE:
                    logi( "Received event: EVENT_SRV_STATUS_UPDATE");
                    AsyncResult arResult = (AsyncResult) msg.obj;
                    if (arResult.exception == null && arResult.result != null) {
                        ArrayList<ServiceStatus> responseArray =
                                (ArrayList<ServiceStatus>) arResult.result;
                        handleSrvStatusUpdate(responseArray);
                    } else {
                        loge("IMS Service Status Update failed!");
                        initServiceStatus();
                    }
                    break;
                case EVENT_GET_SRV_STATUS:
                    logi( "Received event: EVENT_GET_STATUS_UPDATE");
                    AsyncResult arResultSrv = (AsyncResult) msg.obj;
                    if (arResultSrv.exception == null && arResultSrv.result != null) {
                        ArrayList<ServiceStatus> responseArray =
                                (ArrayList<ServiceStatus>) arResultSrv.result;
                        handleSrvStatusUpdate(responseArray);
                    } else {
                        loge("IMS Service Status Update failed!");
                        initServiceStatus();
                    }
                    break;
                case EVENT_SET_SRV_STATUS:
                    //TODO:
                    break;
                case EVENT_CALL_STATE_CHANGE:
                    ar = (AsyncResult) msg.obj;
                    if (ar.result != null) {
                        handleCalls(ar);
                    } else if (isImsExceptionRadioNotAvailable(ar.exception)) {
                        loge("EVENT_CALL_STATE_CHANGE when Radio is Unavailable");
                        ar.exception = null;
                        ar.result = getCallsListToClear();
                        if (ar.result != null) {
                            handleCalls(ar);
                        } else {
                            loge("EVENT_CALL_STATE_CHANGE with no calls ignored!");
                        }
                    } else {
                        loge("EVENT_CALL_STATE_CHANGE exception " + ar.exception);
                    }
                    break;
                case EVENT_GET_CURRENT_CALLS:
                    ar = (AsyncResult) msg.obj;
                    handleCalls(ar);
                    break;
                case EVENT_SUPP_SRV_UPDATE:
                    ar = (AsyncResult) msg.obj;
                    handleSuppSvc(ar);
                    break;
                case EVENT_TTY_STATE_CHANGED:
                    ar = (AsyncResult) msg.obj;
                    handleTtyModeChange(ar);
                    break;
                case EVENT_REFRESH_CONF_INFO:
                    ar = (AsyncResult) msg.obj;
                    handleRefreshConfInfo(ar);
                    break;
                case EVENT_HANDOVER_STATE_CHANGED:
                    ar = (AsyncResult) msg.obj;
                    handleHandover(ar);
                    break;
                case EVENT_CONFERENCE:
                    ar = (AsyncResult) msg.obj;
                    handleConferenceResponse(ar);
                    break;
                case EVENT_CALL_MODIFY:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null && ar.result != null && ar.exception == null) {
                        handleModifyCallRequest((CallModify) ar.result);
                    } else {
                        loge("Error EVENT_MODIFY_CALL AsyncResult ar= " + ar);
                    }
                    break;
                case EVENT_MWI:
                    handleMwiNotification(msg);
                    break;
                case EVENT_SET_CALL_FORWARD_TIMER:
                    ar = (AsyncResult) msg.obj;
                    onSetCallForwardTimerDone(getImsInterfaceListener(ar), ar);
                    break;
                case EVENT_GET_CALL_FORWARD_TIMER:
                    ar = (AsyncResult) msg.obj;
                    onGetCallForwardTimerDone(getImsInterfaceListener(ar), ar);
                    break;
                case EVENT_SET_CALL_BARRING:
                    ar = (AsyncResult) msg.obj;
                    onSetCallBarring(getImsInterfaceListener(ar), ar);
                    break;
                case EVENT_CANCEL_MODIFY_CALL:
                    ar = (AsyncResult) msg.obj;
                    handleCancelModifyCallResponse(ar);
                    break;
                case EVENT_GEOLOCATION_INFO_REQUEST:
                    ar = (AsyncResult) msg.obj;
                    handleGeolocationRequest(ar);
                    break;
                case EVENT_GEOLOCATION_RESPONSE:
                    ar = (AsyncResult) msg.obj;
                    handleGeolocationResponse(ar);
                    break;
                case EVENT_VOWIFI_CALL_QUALITY_UPDATE:
                    ar = (AsyncResult) msg.obj;
                    handleVoWiFiCallQuality(ar);
                    break;
                case EVENT_CALL_TRANSFER:
                    ar = (AsyncResult) msg.obj;
                    handleCallTransferResponse(ar);
                    break;
                case EVENT_VOPS_CHANGED:
                    ar = (AsyncResult) msg.obj;
                    handleVops(ar);
                    break;
                case EVENT_SSAC_CHANGED:
                    ar = (AsyncResult) msg.obj;
                    handleSsac(ar);
                    break;
                case EVENT_VOPS_RESPONSE:
                    ar = (AsyncResult) msg.obj;
                    handleVopsResponse(getImsInterfaceListener(ar), ar);
                    break;
                case EVENT_SSAC_RESPONSE:
                    ar = (AsyncResult) msg.obj;
                    handleSsacResponse(getImsInterfaceListener(ar), ar);
                    break;
                case EVENT_PARTICIPANT_STATUS_INFO:
                    ar = (AsyncResult) msg.obj;
                    handleParticipantStatusInfo(ar);
                    break;
                case EVENT_SET_VOLTE_PREF:
                    ar = (AsyncResult) msg.obj;
                    handleUpdateVoltePrefResponse(ar);
                    break;
                case EVENT_GET_VOLTE_PREF:
                    ar = (AsyncResult) msg.obj;
                    handleQueryVoltePrefResponse(ar);
                    break;
                case EVENT_GET_HANDOVER_CONFIG:
                    ar = (AsyncResult) msg.obj;
                    onGetHandoverConfigDone(getImsInterfaceListener(ar), ar);
                    break;
                case EVENT_SET_HANDOVER_CONFIG:
                    ar = (AsyncResult) msg.obj;
                    onSetHandoverConfigDone(getImsInterfaceListener(ar), ar);
                    break;
                case EVENT_REGISTRATION_BLOCK_STATUS:
                    ar = (AsyncResult) msg.obj;
                    handleRegistrationBlockStatus(ar);
                    break;
                case EVENT_CALL_AUTO_REJECT:
                    ar = (AsyncResult) msg.obj;
                    handleCallAutoReject(ar);
                    break;
                case EVENT_WFC_ROAMING_CONFIGURATION:
                    ar = (AsyncResult) msg.obj;
                    handleWfcRoamingConfiguration(ar);
                    break;
                case EVENT_USSD_INFO:
                    ar = (AsyncResult) msg.obj;
                    handleUssdInfo(ar);
                    break;
                case EVENT_SEND_AUTO_REJECT:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        loge("IMS Service auto reject update failed!" +
                                ar.exception);
                    }
                    break;
                default:
                    logi( "Unknown message = " + msg.what);
            }
        }
    }

    private int getOperationStatus(boolean status) {
        return status ? ImsConfig.OperationStatusConstants.SUCCESS :
                ImsConfig.OperationStatusConstants.FAILED;
    }

    private int getUtErrorCode(AsyncResult ar) {
         ImsReasonInfo err = mImsUtImpl.getImsReasonInfoFromResponseError(ar);
         return err == null ? ImsReasonInfo.CODE_UNSPECIFIED : err.getCode();
    }

    private void onSetCallForwardTimerDone(IQtiImsExtListener listener, AsyncResult ar) {
        if (listener == null || ar == null) {
            loge("onSetCallForwardTimerDone: listener : " + listener + " AsyncResult: " + ar);
            return;
        }
        int status = 0;

        if (ar.exception != null) {
            loge("set CF Timer error!");
            CallForwardStatusInfo cfStatusInfo = (CallForwardStatusInfo) ar.result;
            final ImsReasonInfo sipError = cfStatusInfo != null ? cfStatusInfo.getSipErrorInfo() :
                    null;
            String failCause = sipError != null ? sipError.getExtraMessage() : null;
            loge("onSetCallForwardTimerDone Failure cause: " + failCause);
            if (failCause != null && !(failCause.isEmpty())) {
                try {
                    listener.onUTReqFailed(mPhoneId, getUtErrorCode(ar), failCause);
                } catch (Throwable t) {
                    loge("onUTReqFailed exception!" +t);
                }
            } else {
                try {
                    listener.onUTReqFailed(mPhoneId, getUtErrorCode(ar), null);
                } catch (Throwable t) {
                    loge("onUTReqFailed exception!" +t);
                }
            }
        } else {
            logi("set CF Timer success!");
            try {
                listener.onSetCallForwardUncondTimer(mPhoneId, status);
            } catch (Throwable t) {
                loge("onSetCallForwardTimerDone exception!" +t);
            }
        }
    }

    private void onGetCallForwardTimerDone(IQtiImsExtListener listener, AsyncResult ar) {
        int startHour = 0;
        int endHour = 0;
        int startMinute = 0;
        int endMinute = 0;
        int reason = 0;
        int status = 0;
        String number = null;
        int serviceClass = 0;

        if (ar.exception != null) {
            loge("get CF Timer error!");
            try {
                listener.onUTReqFailed(mPhoneId,  getUtErrorCode(ar), null);
            } catch (Throwable t) {
                loge("onUTReqFailed exception!" +t);
            }
            return;
        }
        if (ar.result != null) {
            if(ar.result instanceof ImsCallForwardTimerInfo[]) {
                logi( "onGetCallForwardTimerDone ImsCallForwardTimerInfo instance! ");
                ImsCallForwardTimerInfo cfTimerInfo[] = (ImsCallForwardTimerInfo[]) ar.result;
                for (int i = 0; i < cfTimerInfo.length; i++) {
                   startHour = cfTimerInfo[i].startHour;
                   endHour = cfTimerInfo[i].endHour;
                   startMinute = cfTimerInfo[i].startMinute;
                   endMinute = cfTimerInfo[i].endMinute;
                   reason = cfTimerInfo[i].reason;
                   status = cfTimerInfo[i].status;
                   number = cfTimerInfo[i].number;
                   serviceClass = cfTimerInfo[i].serviceClass;
                }
            } else if (ar.result instanceof CallForwardInfo[]) {
                logi( "onGetCallForwardTimerDone CallForwardInfo instance! ");
                CallForwardInfo cfInfo[] = (CallForwardInfo[]) ar.result;
                for (int i = 0; i < cfInfo.length; i++) {
                    reason = cfInfo[i].reason;
                    status = cfInfo[i].status;
                    number = cfInfo[i].number;
                    serviceClass = cfInfo[i].serviceClass;
                }
            }
        }

        if (listener !=null) {
            if (reason == CF_REASON_UNCONDITIONAL) {
                try {
                    listener.onGetCallForwardUncondTimer(mPhoneId, startHour, endHour, startMinute,
                            endMinute, reason, status, number, serviceClass);
                } catch (Throwable t) {
                    loge("onGetCallForwardTimerDone exception!");
                }
            } else {
                try {
                    listener.onUTReqFailed(mPhoneId, getUtErrorCode(ar), null);
                } catch (Throwable t) {
                    loge("onUTReqFailed exception!"+t);
                }
            }
        }
    }

    private void handleCancelModifyCallResponse(AsyncResult ar) {
        IQtiImsExtListener listener = getImsInterfaceListener(ar);

        /* Default assume as successful */
        int nStatus = QtiImsExtUtils.QTI_IMS_REQUEST_SUCCESS;
        if (ar != null && ar.exception != null) {
            /* Request failed */
            nStatus = QtiImsExtUtils.QTI_IMS_REQUEST_ERROR;
            Toast.makeText(mContext, "Cancel upgrade failed", Toast.LENGTH_SHORT).show();
        }
        logi( "handleCancelModifyCallResponse: Result " + nStatus);

        /* If listener is available, notify the result */
        if (listener != null) {
            try {
                listener.receiveCancelModifyCallResponse(mPhoneId, nStatus);
            } catch (Throwable t) {
                loge("handleCancelModifyCallResponse exception!");
            }
        } else {
            logi( "handleCancelModifyCallResponse: no listener is available");
        }
    }

    private void handleGeolocationResponse(AsyncResult ar) {
        if (ar != null && ar.exception != null) {
            logi( "handleGeolocationResponse :: Error response!");
        }
        // NOTE: If we need to figure exactly which request's response
        //       failed, the Message's arg1 and arg2 fields will have to
        //       be used in GeoCoderTask. This is not needed as of now.
    }

    private void handleGeolocationRequest(AsyncResult ar) {
        GeoLocationInfo geoInfo = (GeoLocationInfo) ar.result;
        if (geoInfo == null) {
            loge("handleGeolocationRequest :: Null AsyncResult!");
            return;
        }
        try {
            sTaskExecutor.execute(
                    new GeocoderTask(geoInfo.getLat(), geoInfo.getLon()));
        } catch (RejectedExecutionException rex) {
            loge("handleGeolocationRequest :: Failure to start "
                    + "GeocoderTask thread for [lat=" + geoInfo.getLat()
                    + " | long=" + geoInfo.getLon() + "]!\n");
            rex.printStackTrace();
        }
    }

    private class GeocoderTask implements Runnable {
        private double latitude;
        private double longitude;
        final int MAX_RESULTS = 1; // Check NOTE in run method before changing value.

        public GeocoderTask(double lat, double lon) {
            latitude = lat;
            longitude = lon;
        }

        @Override
        public String toString() {
            return "GeocoderTask [lat=" + latitude + " | long=" + longitude + "]";
        }

        @Override
        public void run() {
            logi( "Executing " + this);
            Address address = null;
            List<Address> addresses = null;
            if (!Geocoder.isPresent()) {
                loge("handleGeolocationRequest :: Geocoder not present!");
            } else {
                Geocoder gc = new Geocoder(mContext);
                try {
                    addresses = gc.getFromLocation(latitude, longitude, MAX_RESULTS);
                } catch (IllegalArgumentException iex) {
                    loge(this + " :: Invalid latitude or longitude value!");
                } catch (IOException iox) {
                    loge(this + " :: Error getting Addresses from GeoCoder!");
                }
                if (addresses != null && !addresses.isEmpty()) {
                    // NOTE: Since the current rationale is to only query GeCoder API
                    //       for one Address, the logic below works. A for-loop will
                    //       be required if multiple Address-es need to be parsed.
                    //       This will be required if IMS Service has to select the
                    //       'most appropriate' Address information from multiple objects.
                    address = addresses.get(0);
                } else {
                    loge(this + " :: Error getting Addresses from GeoCoder!");
                }
            }
            mCi.sendGeolocationInfo(latitude, longitude, address,
                    mHandler.obtainMessage(EVENT_GEOLOCATION_RESPONSE));
        }
    }

    private void handleCallTransferResponse(AsyncResult ar) {
        IQtiImsExtListener listener = getImsInterfaceListener(ar);

        /* Default assume as successful */
        int nStatus = QtiImsExtUtils.QTI_IMS_REQUEST_SUCCESS;
        if (ar != null && ar.exception != null) {
            /* Request failed */
            nStatus = QtiImsExtUtils.QTI_IMS_REQUEST_ERROR;
        }
        logi( "handleCallTransferResponse: Result " + nStatus);

        /* If listener is available, notify the result */
        if (listener != null) {
            try {
                listener.receiveCallTransferResponse(mPhoneId, nStatus);
            } catch (Throwable t) {
                loge("handleCallTransferResponse exception!");
            }
        } else {
            logi( "handleCallTransferResponse: no listener is available");
        }
    }

    private IQtiImsExtListener getImsInterfaceListener(AsyncResult ar) {
        if (ar != null && ar.userObj instanceof IQtiImsExtListener) {
            return (IQtiImsExtListener)ar.userObj;
        }
        logi( "getImsConfigListener returns null");
        return null;
    }

    private void handleMwiNotification(Message msg) {
        if ((msg != null) && (msg.obj != null)) {
            AsyncResult arMwiUpdate = (AsyncResult) msg.obj;
            if (arMwiUpdate.exception == null) {
                if (arMwiUpdate.result != null) {
                    mMwiResponse = (Mwi) arMwiUpdate.result;
                    updateVoiceMail();
                } else {
                    loge("handleMwiNotification arMwiUpdate.result null");
                }
            } else {
                loge("handleMwiNotification arMwiUpdate exception");
            }
        } else {
            loge("handleMwiNotification msg null");
        }
    }

    private void updateVoiceMail() {
        int count = 0;
        for (Mwi.MwiMessageSummary msgSummary : mMwiResponse.mwiMsgSummary) {
            if (msgSummary.mMessageType == Mwi.MWI_MSG_VOICE) {
                count = count + msgSummary.mNewMessage
                        + msgSummary.mNewUrgent;
                break;
            }
        }
        // Holds the Voice mail count
        // .i.e.MwiMessageSummary.NewMessage + MwiMessageSummary.NewUrgent
        logi("updateVoiceMail count = " + count);

        notifyVoiceMessageCountUpdate(count);
    }

    public void registerForParticipantStatusInfo(IQtiImsExtListener listener) {
        mQtiImsParticipantStatusListeners.add(listener);
    }

    private void handleVoWiFiCallQuality(AsyncResult ar) {
        if (ar != null && ar.exception == null && ar.result != null) {
            int[] voWifiCallQuality = (int[]) ar.result;
            if (voWifiCallQuality[0] != VoWiFiQuality.QUALITY_NONE) {
                if (mTracker != null) {
                    mTracker.updateVoWiFiCallQuality(voWifiCallQuality[0]);
                } else {
                    loge("Wifi Quality Error -- tracker is null");
                }
            } else {
                loge("handleVoWiFiCallQuality received VoWiFIQuality : " +
                        voWifiCallQuality[0]);
            }
        } else {
            loge("handleVoWiFiCallQuality response is not valid");
        }
    }

    public void registerForCapabilitiesChanged(Handler h, int what, Object obj) {
        mCapabilitiesChangedRegistrants.addUnique(h, what, obj);
    }

    public void unregisterForCapabilitiesChanged(Handler h) {
        mCapabilitiesChangedRegistrants.remove(h);
    }

    private void handleVops(AsyncResult ar) {
        if ((ar != null) && (ar.exception == null) && (ar.result != null)) {
            mIsVopsEnabled = (boolean) ar.result;
            broadcastVopsSsacIntent();
        } else {
            loge("handleVops exception");
        }
    }

    private void handleSsac(AsyncResult ar) {
        if ((ar != null) && (ar.exception == null) && (ar.result != null)) {
            final SsacInfo ssacInd = (SsacInfo) ar.result;

            logi( "handleSsac voice = " + ssacInd.getBarringFactorVoice());
            mIsSsacVoiceBarred = (ssacInd.getBarringFactorVoice() == SSAC_VOICE_BARRING_ZERO);
            broadcastVopsSsacIntent();
        } else {
            loge("handleSsac exception");
        }
    }

    private void handleWfcRoamingConfiguration(AsyncResult ar) {
        if ((ar != null) && (ar.exception == null) && (ar.result != null)) {
            mModemSupportForWfcRoamingConfiguration = (boolean) ar.result;
        } else {
            loge("handleWfcRoamingConfiguration exception");
        }
        if (mModemSupportForWfcRoamingConfiguration) {
            mImsConfigImpl.updateWfcModeConfigurationsToModem();
        }
    }

    public boolean IsWfcRoamingConfigurationSupportedByModem() {
        return mModemSupportForWfcRoamingConfiguration;
    }

    private void handleUssdInfo(AsyncResult ar) {
        if ((ar != null) && (ar.exception == null) && (ar.result != null)) {
            final UssdInfo info = (UssdInfo) ar.result;

            if (mUssdInfoListener != null) {
                try {
                    mUssdInfoListener.onUssdFailed(mPhoneId, info.getType(),
                            info.getErrorCode(), info.getErrorMessage());
                } catch (Exception e) {
                    Log.e(this, "handleUssdInfo exception!");
                }
            }
        } else {
            loge("handleUssdInfo exception");
        }
    }

    private void broadcastVopsSsacIntent() {
        Intent intent = new Intent(QtiImsExtUtils.ACTION_VOPS_SSAC_STATUS);
        intent.putExtra(QtiImsExtUtils.EXTRA_VOPS, mIsVopsEnabled);
        intent.putExtra(QtiImsExtUtils.EXTRA_SSAC, mIsSsacVoiceBarred);
        intent.putExtra(QtiImsExtUtils.QTI_IMS_PHONE_ID_EXTRA_KEY, mPhoneId);
        logi( "broadcastVopsSsacIntent Vops = " + mIsVopsEnabled
                + " , Ssac = " + mIsSsacVoiceBarred + " , PhoneId = " + mPhoneId);
        mContext.sendBroadcastAsUser(intent, UserHandle.ALL);
    }

    public void queryVopsStatus(IQtiImsExtListener listener) {
        logi( "queryVopsStatus");
        mCi.queryVopsStatus(mHandler.obtainMessage(EVENT_VOPS_RESPONSE, listener));
    }

    public void querySsacStatus(IQtiImsExtListener listener) {
        logi( "querySsacStatus");
        mCi.querySsacStatus(mHandler.obtainMessage(EVENT_SSAC_RESPONSE, listener));
    }

    private void handleVopsResponse(IQtiImsExtListener listener, AsyncResult ar) {
        if ((ar != null) && (ar.exception == null) && (ar.result != null)) {
            final VopsInfo vops = (VopsInfo) ar.result;

            mIsVopsEnabled = vops.isVopsEnabled();
            logi( "Vops Response = " + mIsVopsEnabled);

            if (listener != null) {
                try {
                    listener.notifyVopsStatus(mPhoneId, mIsVopsEnabled);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        } else {
            loge("handleVopsResponse exception");
        }
    }

    private void handleSsacResponse(IQtiImsExtListener listener, AsyncResult ar) {
        if ((ar != null) && (ar.exception == null) && (ar.result != null)) {
            final SsacInfo ssacInd = (SsacInfo) ar.result;

            logi( "handleSsacResponse voice = " + ssacInd.getBarringFactorVoice());
            mIsSsacVoiceBarred = (ssacInd.getBarringFactorVoice() == SSAC_VOICE_BARRING_ZERO);

            if (listener != null) {
                try {
                    listener.notifySsacStatus(mPhoneId, mIsSsacVoiceBarred);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        } else {
            loge("handleSsacResponse exception");
        }
    }

    /**
     * Handles participant status Info notified through UNSOL_PARTICIPANT_STATUS_INFO message
     * @param listener - Ims extension listener
     * @param ar - the AsyncResult object that contains the participant status Info information
     */
    public void handleParticipantStatusInfo(AsyncResult ar) {
        if ((ar != null) && (ar.exception == null) && (ar.result != null)) {
            ParticipantStatusDetails participantInfo =
                    (ParticipantStatusDetails) ar.result;
            if (mQtiImsParticipantStatusListeners.size() == 0 || mTracker == null ||
                    mTracker.getCallSession(Integer.toString(participantInfo.getCallId())) == null) {
                logi( "participant listeners size= "+ mQtiImsParticipantStatusListeners.size()
                        + " isTrackerPresent = " + (mTracker != null));
                return;
            }
            logi( "handleParticipantStatusInfo operation = " + participantInfo.getOperation()
                   + " status = " + participantInfo.getSipStatus() + " participant = "
                   + participantInfo.getParticipantUri() + " ect = " + participantInfo.getIsEct());
            for (IQtiImsExtListener listener : mQtiImsParticipantStatusListeners) {
                try {
                    listener.notifyParticipantStatusInfo(mPhoneId,
                            participantInfo.getOperation(), participantInfo.getSipStatus(),
                            participantInfo.getParticipantUri(), participantInfo.getIsEct());
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        } else {
            loge("ParticipantStatusInfo exception");
        }
    }

    /**
     * Handles registration block status through UNSOL_RESPONSE_REGISTRATION_BLOCK_STATUS message
     * @param ar - the AsyncResult object that contains the registration block status information
     */
    public void handleRegistrationBlockStatus(AsyncResult ar) {

        if ((ar != null) && (ar.exception == null) && (ar.result != null)) {
            RegistrationBlockStatusInfo regBlockStatus =
                    (RegistrationBlockStatusInfo) ar.result;
            // We only check and report wlan failures for now. This can be
            // extended to Wwan when we have an usecase for it.
            if(regBlockStatus.getStatusOnWlan() != null) {
                BlockStatusInfo blockStatus =
                        regBlockStatus.getStatusOnWlan();
                //Check if it has block reason details. Block reason type is
                //ignored for now.
                if(isIKEv2Error(blockStatus)) {
                    final Runnable r = new Runnable() {
                        @Override
                        public void run() {
                            logi( "Permanent IWLAN reg failure (IKEv2 auth failure).");
                            ImsReasonInfo imsReasonInfo = new ImsReasonInfo(
                                    ImsReasonInfo.CODE_EPDG_TUNNEL_ESTABLISH_FAILURE,
                                    ImsReasonInfo.CODE_IKEV2_AUTH_FAILURE,
                                    null);
                            mImsReg.registrationChangeFailed(
                                    ServiceState.RIL_RADIO_TECHNOLOGY_IWLAN, imsReasonInfo);
                        }
                    };
                    if (mFeatureCallbackHandler != null) {
                        mFeatureCallbackHandler.post(r);
                    }
                }
            }
        } else {
            loge("Async result is null or exception is not null.");
        }
    }

    public void updateVoltePreference(int preference, IQtiImsExtListener listener) {
        mCi.updateVoltePref(preference, mHandler.obtainMessage(EVENT_SET_VOLTE_PREF, listener));
    }

    public void queryVoltePreference(IQtiImsExtListener listener) {
        mCi.queryVoltePref(mHandler.obtainMessage(EVENT_GET_VOLTE_PREF, listener));
    }

    private int getQtiImsExtOperationStatus(AsyncResult ar) {
        /* Default assume as successful */
        int status = QtiImsExtUtils.QTI_IMS_REQUEST_SUCCESS;
        if (ar != null && ar.exception != null) {
            /* Request failed */
            status = QtiImsExtUtils.QTI_IMS_REQUEST_ERROR;
        }
        return status;
    }

    private void handleUpdateVoltePrefResponse(AsyncResult ar) {
        IQtiImsExtListener listener = getImsInterfaceListener(ar);
        int result = getQtiImsExtOperationStatus(ar);
        logi( "handleUpdateVoltePrefResponse: result " + result);

        /* If listener is available, notify the result */
        if (listener != null) {
            try {
                listener.onVoltePreferenceUpdated(mPhoneId, result);
            } catch (Throwable t) {
                loge("handleUpdateVoltePrefResponse exception!");
            }
        } else {
            loge("handleUpdateVoltePrefResponse: no listener is available");
        }
    }

    private void handleQueryVoltePrefResponse(AsyncResult ar) {
        IQtiImsExtListener listener = getImsInterfaceListener(ar);
        int result = getQtiImsExtOperationStatus(ar);
        int preference = QtiImsExtUtils.QTI_IMS_VOLTE_PREF_UNKNOWN;
        if (result == QtiImsExtUtils.QTI_IMS_REQUEST_SUCCESS) {
            /* Received volte preference from lower layers */
            int[] pref = (int[])ar.result;
            preference = pref[0];
        }
        logi( "handleQueryVoltePrefResponse: result-" + result + " mode-" + preference);

        /* If listener is available, notify the result */
        if (listener != null) {
            try {
                listener.onVoltePreferenceQueried(mPhoneId, result, preference);
            } catch (Throwable t) {
                loge("handleQueryVoltePrefResponse exception!");
            }
        } else {
            loge("handleQueryVoltePrefResponse: no listener is available");
        }
    }

    private void onGetHandoverConfigDone(IQtiImsExtListener listener, AsyncResult ar) {
        if (listener != null) {
            try {
                int status = getOperationStatus(ar.exception == null);
                int result = ar.result == null ?
                             QtiImsExtUtils.QTI_IMS_HO_INVALID :
                             (int)ar.result;
                listener.onGetHandoverConfig(mPhoneId, status, result);
            } catch (Throwable t) {
                loge("onGetHandoverConfigDone " + t);
            }
        } else {
            loge("onGetHandoverConfigDone listener is not valid !!!");
        }
    }

    private void onSetHandoverConfigDone(IQtiImsExtListener listener, AsyncResult ar) {
        if (listener != null) {
            try {
                int status = getOperationStatus(ar.exception == null);
                listener.onSetHandoverConfig(mPhoneId, status);
            } catch (Throwable t) {
                loge("onSetHandoverConfigDone " + t);
            }
        } else {
            loge("onSetHandoverConfigDone listener is not valid !!!");
        }
    }

    private static boolean isIKEv2Error(BlockStatusInfo blockStatus) {
        //Check if it has block reason details. Block reason type is
        //ignored for now.
        if(blockStatus.getReasonDetails() != null) {
            BlockReasonDetailsInfo blockReasonDetails =
                blockStatus.getReasonDetails();
            //Check if it has registration failure reason type
            //and failure reason.
            return ((blockReasonDetails.getRegFailureReasonType() !=
                       BlockReasonDetailsInfo.REG_FAILURE_REASON_TYPE_INVALID) &&
                    (blockReasonDetails.getRegFailureReason() !=
                       Integer.MAX_VALUE) &&
                    //Check if there was an IKEv2 authentication failure while
                    //establishing tunnel for IWLAN ims registration.
                    (blockReasonDetails.getRegFailureReasonType() ==
                       BlockReasonDetailsInfo.REG_FAILURE_REASON_TYPE_IWLAN) &&
                    (blockReasonDetails.getRegFailureReason() == IKEv2_AUTH_FAILURE));
        }
        return false;
    }

    private void handleCallAutoReject(AsyncResult ar) {
        if (ar == null || ar.result == null) {
            Log.e(this, "handleCallAutoReject :: Error parsing DriverCallIms");
            return;
        }

        DriverCallIms rejectedCall = (DriverCallIms)ar.result;
        if (rejectedCall.callDetails == null) {
            Log.e(this, "handleCallAutoReject :: No call details in DriverCallIms");
            return;
        }
        ImsCallProfile rejCallProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                rejectedCall.callDetails.call_type);
        if (rejectedCall.number != null) {
            rejCallProfile.setCallExtra(ImsCallProfile.EXTRA_OI, rejectedCall.number);
        }
        rejCallProfile.setCallExtraInt(QtiImsExtUtils.QTI_IMS_PHONE_ID_EXTRA_KEY, mPhoneId);

        Log.i(this, "handleCallAutoReject :: rejCallProfile=" + rejCallProfile
                + " ImsReasonInfo=" + rejectedCall.callFailCause);
        notifyRejectedCall(rejCallProfile, rejectedCall.callFailCause);
    }

    /**
     * Get the MultiEndpoint interface handle.
     * @return ImsMultiEndpointImplBase interface handle.
    */
    @Override
    public ImsMultiEndpointImplBase getMultiEndpoint() {
        return mImsMultiEndpointImpl;
    }

    public ImsMultiIdentityImpl getMultiIdentityImpl() {
        synchronized (mMultiIdentityLock) {
            if ( mMultiIdentityImpl == null ) {
                 mMultiIdentityImpl = new ImsMultiIdentityImpl(mPhoneId, mCi, mContext,
                         mHandler.getLooper());
            }
        }
        return mMultiIdentityImpl;
    }

    public ImsScreenShareControllerImpl getScreenShareController() {
        synchronized (mScreenShareLock) {
            if ( mScreenShareController == null ) {
                 mScreenShareController = new ImsScreenShareControllerImpl(this, mContext);
            }
        }
        return mScreenShareController;
    }

    public int getImsFeatureState() {
        return getFeatureState();
    }

    private List<ImsCallSessionImpl> getIncomingOrWaitingCallSession() {
        List<ImsCallSessionImpl> session =
                getCallSessionByState(DriverCallIms.State.INCOMING);
        if (session.isEmpty()) {
            session = getCallSessionByState(DriverCallIms.State.WAITING);
        }
        return session;
    }

    public void setAnswerExtras(Bundle extras) {
        List<ImsCallSessionImpl> sessionList = getIncomingOrWaitingCallSession();
        if (sessionList.isEmpty()) {
            logd("setAnswerExtras: no incoming/waiting call available");
            return;
        }
        sessionList.get(0).setAnswerExtras(extras);
    }

    private void onSetCallBarring(IQtiImsExtListener listener, AsyncResult ar) {
        if (listener == null || ar == null) {
            loge("onSetCallBarring: listener : " + listener + " AsyncResult: " + ar);
            return;
        }

        if (ar.exception != null) {
            SuppSvcResponse response = (SuppSvcResponse) ar.result;
            final ImsReasonInfo sipError = response != null ? response.getErrorDetails() : null;
            String failCause = sipError != null ? sipError.getExtraMessage() : null;
            loge("onSetCallBarring failure cause: " + failCause);
            try {
                listener.onUTReqFailed(mPhoneId, getUtErrorCode(ar), failCause);
            } catch (Throwable t) {
                loge("onUTReqFailed exception" + t);
            }
        } else {
            logi("onSetCallBarring success");
            try {
                listener.onSetCallBarring();
            } catch (Throwable t) {
                loge("onSetCallBarring exception" + t);
            }
        }
    }

    /**
     * This method notifes this ServiceSub that it has a new call.
     */
    public void onNewCall() {
        ImsVideoGlobals.getInstance().setPhoneIdWithActiveCall(mPhoneId);
    }

    private void logi(String s) {
        Log.i(LOG_TAG, "[" + mPhoneId + "] " + s);
    }

    private void logv(String s) {
        Log.v(LOG_TAG, "[" + mPhoneId + "] " + s);
    }

    private void logd(String s) {
        Log.d(LOG_TAG, "[" + mPhoneId + "] " + s);
    }

    private void loge(String s) {
        Log.e(LOG_TAG, "[" + mPhoneId + "] " + s);
    }

}
