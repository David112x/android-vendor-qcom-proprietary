/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */
/*
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

package com.qualcomm.qti.networksetting;

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceManager;
import android.preference.TwoStatePreference;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.Toast;

import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.TelephonyCapabilities;
import com.android.internal.telephony.util.NotificationChannelController;

import org.codeaurora.internal.IExtTelephony;
import org.codeaurora.telephony.utils.RILConstants;

/**
 * "Networks" settings UI for the Phone app.
 */
public class NetworkOperators extends PreferenceCategory
        implements Preference.OnPreferenceChangeListener {

    private static final String LOG_TAG = "NetworkOperators";
    private static final boolean DBG = true;

    private static final int EVENT_AUTO_SELECT_DONE = 100;
    private static final int EVENT_GET_NETWORK_SELECTION_MODE_DONE = 200;

    private static final int SELECTED_OPERATOR_FAIL_NOTIFICATION = 6;
    private static final int ALREADY_IN_AUTO_SELECTION = 1;

    //String keys for preference lookup
    public static final String BUTTON_NETWORK_SELECT_KEY = "button_network_select_key";
    public static final String BUTTON_AUTO_SELECT_KEY = "button_auto_select_key";
    public static final String BUTTON_CHOOSE_NETWORK_KEY = "button_choose_network_key";
    public static final String CATEGORY_NETWORK_OPERATORS_KEY = "network_operators_category_key";

    public static final String KEY_SUBSCRIPTION_ID = "subscription_id";

    int mPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;

    //preference objects
    private NetworkSelectListPreference mNetworkSelect;
    private TwoStatePreference mAutoSelect;
    private Preference mChooseNetwork;
    private ProgressDialog mProgressDialog;
    private Toast mToast;

    // used to track the notification of selected network unavailable
    private boolean mSelectedUnavailableNotify = false;
    private int mSubId;
    private NotificationManager mNotificationManager;
    private TelephonyManager mTelephonyManager;

    // There are two sets of Network Select UI in this class.
    // If persist.vendor.radio.enableadvancedscan is set as true
    // mChooseNetwork will be used, otherwise mNetworkSelect will be used.
    boolean mEnableNewManualSelectNetworkUI = false;

    public NetworkOperators(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public NetworkOperators(Context context) {
        super(context);
    }

    private boolean isAdvancedPlmnScanUIEnabled() {
        boolean propVal = false;
        IExtTelephony extTelephony = IExtTelephony.Stub
                .asInterface(ServiceManager.getService("extphone"));
        try {
            propVal = extTelephony
                    .getPropertyValueBool("persist.vendor.radio.enableadvancedscan", false);
        } catch (RemoteException | NullPointerException ex) {
            loge("Exception: " + ex);
        }
        return propVal;
    }

    /**
     * Initialize NetworkOperators instance.
     */
    public void initialize() {
        mEnableNewManualSelectNetworkUI = isAdvancedPlmnScanUIEnabled();
        mNetworkSelect =
                (NetworkSelectListPreference) findPreference(BUTTON_NETWORK_SELECT_KEY);
        mAutoSelect =
                (TwoStatePreference) findPreference(BUTTON_AUTO_SELECT_KEY);
        mChooseNetwork = findPreference(BUTTON_CHOOSE_NETWORK_KEY);
        if (mEnableNewManualSelectNetworkUI) {
            removePreference(mNetworkSelect);
        } else {
            removePreference(mChooseNetwork);
        }
        mProgressDialog = new ProgressDialog(getContext());
        try {
            Context context = getContext().createPackageContext("com.android.phone", 0);
            mNotificationManager =
                    (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        } catch (NameNotFoundException e) {
            loge("NameNotFoundException for com.android.phone");
            mNotificationManager = (NotificationManager)
                    getContext().getSystemService(Context.NOTIFICATION_SERVICE);
        }
        mTelephonyManager =
                (TelephonyManager) getContext().getSystemService(Context.TELEPHONY_SERVICE);
    }

    /**
     * Update NetworkOperators instance if like subId or queryService are updated.
     *
     * @param subId Corresponding subscription ID of this network.
     * @param queryService The service to do network queries.
     */
    protected void update(final int subId, INetworkQueryService queryService) {
        TelephonyManager currentSubTelephonyManager;
        mSubId = subId;
        mPhoneId = SubscriptionManager.getSlotIndex(mSubId);

        if (mAutoSelect != null) {
            mAutoSelect.setOnPreferenceChangeListener(this);
        }

        if (mEnableNewManualSelectNetworkUI) {
            if (mChooseNetwork != null) {
                ServiceState ss = mTelephonyManager.getServiceStateForSubscriber(mSubId);
                currentSubTelephonyManager = mTelephonyManager.createForSubscriptionId(mSubId);
                if (ss != null && ss.getState() == ServiceState.STATE_IN_SERVICE) {
                    mChooseNetwork.setSummary(currentSubTelephonyManager.getNetworkOperatorName());
                } else {
                    mChooseNetwork.setSummary(R.string.network_disconnected);
                }
            }
        } else {
            if (mNetworkSelect != null) {
                mNetworkSelect.initialize(mSubId, queryService, this, mProgressDialog);
            }
        }
        getNetworkSelectionMode();
    }

    /**
     * Implemented to support onPreferenceChangeListener to look for preference
     * changes specifically on auto select button.
     *
     * @param preference is the preference to be changed, should be auto select button.
     * @param newValue should be the value of whether autoSelect is checked.
     */
    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (preference == mAutoSelect) {
            boolean autoSelect = (Boolean) newValue;
            if (DBG) logd("onPreferenceChange autoSelect: " + String.valueOf(autoSelect));
            selectNetworkAutomatic(autoSelect);
            return true;
        }
        return false;
    }

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
                case EVENT_AUTO_SELECT_DONE:
                    mAutoSelect.setEnabled(true);
                    dismissProgressBar();

                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        if (DBG) logd("automatic network selection: failed!");
                        displayNetworkSelectionFailed(ar.exception);
                    } else {
                        if (DBG) logd("automatic network selection: succeeded!");
                        displayNetworkSelectionSucceeded(msg.arg1);
                    }

                    break;
                case EVENT_GET_NETWORK_SELECTION_MODE_DONE:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        if (DBG) logd("get network selection mode: failed!");
                    } else if (ar.result != null) {
                        try {
                            int[] modes = (int[]) ar.result;
                            boolean autoSelect = (modes[0] == 0);
                            if (DBG) {
                                logd("get network selection mode: "
                                        + (autoSelect ? "auto" : "manual") + " selection");
                            }
                            if (mAutoSelect != null) {
                                mAutoSelect.setChecked(autoSelect);
                            }
                            if (mEnableNewManualSelectNetworkUI) {
                                if (mChooseNetwork != null) {
                                    mChooseNetwork.setEnabled(!autoSelect);
                                }
                            } else {
                                if (mNetworkSelect != null) {
                                    mNetworkSelect.setEnabled(!autoSelect);
                                }
                            }

                        } catch (Exception e) {
                            if (DBG) loge("get network selection mode: unable to parse result.");
                        }
                    }
            }

            return;
        }
    };

    /* package */ void postTransientNotification(CharSequence msg) {
        if (mToast != null) {
            mToast.cancel();
        }

        mToast = Toast.makeText(getContext(), msg, Toast.LENGTH_LONG);
        mToast.show();
    }

    // Used by both mAutoSelect and mNetworkSelect buttons.
    protected void displayNetworkSelectionFailed(Throwable ex) {
        String status;

        if ((ex != null && ex instanceof CommandException)
                && ((CommandException) ex).getCommandError()
                        == CommandException.Error.ILLEGAL_SIM_OR_ME) {
            status = getContext().getResources().getString(R.string.not_allowed);
        } else {
            status = getContext().getResources().getString(R.string.connect_later);
        }

        postTransientNotification(status);

        Phone phone = PhoneFactory.getPhone(mPhoneId);
        if (phone != null) {
            ServiceState ss = mTelephonyManager.getServiceStateForSubscriber(phone.getSubId());
            if (ss != null) {
                updateNetworkSelection(ss.getState(), phone.getSubId());
            }
        }
    }

    // Used by both mAutoSelect and mNetworkSelect buttons.
    protected void displayNetworkSelectionSucceeded(int msgArg1) {
        String status = null;
        if (msgArg1 == ALREADY_IN_AUTO_SELECTION) {
            status = getContext().getResources().getString(R.string.already_auto);
        } else {
            status = getContext().getResources().getString(R.string.registration_done);
        }
        postTransientNotification(status);
    }

    private void selectNetworkAutomatic(boolean autoSelect) {
        if (DBG) logd("selectNetworkAutomatic: " + String.valueOf(autoSelect));

        if (mEnableNewManualSelectNetworkUI) {
            if (mChooseNetwork != null) {
                mChooseNetwork.setEnabled(!autoSelect);
            }
        } else {
            if (mNetworkSelect != null) {
                mNetworkSelect.setEnabled(!autoSelect);
            }
        }

        if (autoSelect) {
            if (DBG) logd("select network automatically...");
            showAutoSelectProgressBar();
            mAutoSelect.setEnabled(false);
            Message msg = mHandler.obtainMessage(EVENT_AUTO_SELECT_DONE);
            Phone phone = PhoneFactory.getPhone(mPhoneId);
            if (phone != null) {
                phone.setNetworkSelectionModeAutomatic(msg);
            }
        } else {
            if (mEnableNewManualSelectNetworkUI) {
                if (mChooseNetwork != null) {
                    //Open the choose Network page automatically
                    //when user turns off the auto-select
                    openChooseNetworkPage();
                }
            } else {
                if (mNetworkSelect != null) {
                    mNetworkSelect.onClick();
                }
            }
        }
    }

    protected void getNetworkSelectionMode() {
        if (DBG) logd("getting network selection mode...");
        Message msg = mHandler.obtainMessage(EVENT_GET_NETWORK_SELECTION_MODE_DONE);
        Phone phone = PhoneFactory.getPhone(mPhoneId);
        if (phone != null) {
            phone.getNetworkSelectionMode(msg);
        }
    }

    private void dismissProgressBar() {
        if (!isParentWindowFinished(getContext()) && mProgressDialog != null
                && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }
    }

    private void showAutoSelectProgressBar() {
        mProgressDialog.setMessage(
                getContext().getResources().getString(R.string.register_automatically));
        mProgressDialog.setCanceledOnTouchOutside(false);
        mProgressDialog.setCancelable(false);
        mProgressDialog.setIndeterminate(true);
        mProgressDialog.show();
    }

    /**
     * Open the Choose network page via {@link NetworkSelectSettingActivity}
     */
    private void openChooseNetworkPage() {
        Intent intent = NetworkSelectSettingActivity.getIntent(getContext(), mSubId);
        try {
            getContext().startActivity(intent);
        } catch (ActivityNotFoundException e) {
            loge("Couldn't find Activity for intent " + intent);
        }
    }

    protected boolean preferenceTreeClick(Preference preference) {
        if (mEnableNewManualSelectNetworkUI) {
            if (DBG) logd("enable New AutoSelectNetwork UI");
            if (preference == mChooseNetwork) {
                openChooseNetworkPage();
            }
            return (preference == mAutoSelect || preference == mChooseNetwork);
        } else {
            return (preference == mAutoSelect || preference == mNetworkSelect);
        }
    }


    /**
     * Display the network selection "no service" notification
     * @param operator is the numeric operator number
     * @param subId is the subscription ID
     */
    private void showNetworkSelection(Context context, String operator, int subId) {
        if (DBG) logd("showNetworkSelection(" + operator + ")...");

        Notification.Builder builder = new Notification.Builder(context)
                .setSmallIcon(android.R.drawable.stat_sys_warning)
                .setContentTitle(
                        getContext().getString(R.string.notification_network_selection_title))
                .setContentText(
                        getContext().getString(R.string.notification_network_selection_text,
                        operator))
                .setShowWhen(false)
                .setOngoing(true)
                .setChannelId(NotificationChannelController.CHANNEL_ID_ALERT);

        // create the target network operators settings intent
        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK |
                Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
        // Use MobileNetworkSettings to handle the selection intent
        intent.setComponent(new ComponentName(
                getContext().getString(R.string.mobile_network_settings_package),
                getContext().getString(R.string.mobile_network_settings_class)));
        intent.putExtra(GsmUmtsOptions.EXTRA_SUB_ID, subId);
        builder.setContentIntent(PendingIntent.getActivity(getContext(), 0, intent, 0));
        if (mNotificationManager != null) {
            mNotificationManager.notifyAsUser(
                    Integer.toString(subId) /* tag */,
                    SELECTED_OPERATOR_FAIL_NOTIFICATION,
                    builder.build(),
                    UserHandle.ALL);
        }
    }

    /**
     * Turn off the network selection "no service" notification
     */
    private void cancelNetworkSelection(int subId) {
        if (DBG) logd("cancelNetworkSelection()...");
        if (mNotificationManager != null) {
            mNotificationManager.cancelAsUser(
                    Integer.toString(subId) /* tag */,
                    SELECTED_OPERATOR_FAIL_NOTIFICATION, UserHandle.ALL);
        }
    }

    /**
     * Update notification about no service of user selected operator
     *
     * @param serviceState Phone service state
     * @param subId The subscription ID
     */
    void updateNetworkSelection(int serviceState, int subId) {
        int phoneId = SubscriptionManager.getPhoneId(subId);
        TelephonyManager tm =  mTelephonyManager.createForSubscriptionId(subId);
        boolean isPhoneTypeGsm = (tm.getPhoneType() == RILConstants.GSM_PHONE);
        if (isPhoneTypeGsm) {
            if (SubscriptionManager.isValidSubscriptionId(subId)) {
                // get the shared preference of network_selection.
                // empty is auto mode, otherwise it is the operator alpha name
                // in case there is no operator name, check the operator numeric
                try {
                    Context context = getContext().createPackageContext("com.android.phone", 0);
                    SharedPreferences sp =
                            PreferenceManager.getDefaultSharedPreferences(context);
                    String networkSelection =
                            sp.getString(Phone.NETWORK_SELECTION_NAME_KEY + subId, "");
                    if (TextUtils.isEmpty(networkSelection)) {
                        networkSelection =
                                sp.getString(Phone.NETWORK_SELECTION_KEY + subId, "");
                    }

                    if (DBG) logd("updateNetworkSelection()..." + "state = " +
                            serviceState + " new network " + networkSelection);

                    if (serviceState == ServiceState.STATE_OUT_OF_SERVICE
                            && !TextUtils.isEmpty(networkSelection)
                            && isUiccCardProvisioned(subId)) {
                        showNetworkSelection(context, networkSelection, subId);
                        mSelectedUnavailableNotify = true;
                    } else {
                        if (mSelectedUnavailableNotify) {
                            cancelNetworkSelection(subId);
                            mSelectedUnavailableNotify = false;
                        }
                    }
                } catch (NameNotFoundException e) {
                    loge("NameNotFoundException for com.android.phone");
                }
            } else {
                if (DBG) logd("updateNetworkSelection()..." + "state = " +
                        serviceState + " not updating network due to invalid subId " + subId);
            }
        }
    }

    private boolean isUiccCardProvisioned(int subId) {
        final int PROVISIONED = 1;
        final int INVALID_STATE = -1;
        int provisionStatus = INVALID_STATE;
        IExtTelephony mExtTelephony = IExtTelephony.Stub
                .asInterface(ServiceManager.getService("extphone"));
        int slotId = SubscriptionController.getInstance().getSlotIndex(subId);
        try {
            //get current provision state of the SIM.
            provisionStatus = mExtTelephony.getCurrentUiccCardProvisioningStatus(slotId);
        } catch (RemoteException ex) {
            provisionStatus = INVALID_STATE;
            if (DBG) loge("Failed to get status for slotId: "+ slotId +" Exception: " + ex);
        } catch (NullPointerException ex) {
            provisionStatus = INVALID_STATE;
            if (DBG) loge("Failed to get status for slotId: "+ slotId +" Exception: " + ex);
        }
        return provisionStatus == PROVISIONED;
   }

    public static boolean isParentWindowFinished(Context context) {
        boolean finished = false;
        if (context != null) {
            boolean isActivity = context instanceof Activity;
            if (isActivity) {
                Activity parent = (Activity) context;
                finished = parent.isDestroyed() || parent.isFinishing();
            } else {
                Log.d(LOG_TAG, "Unexpected case!");
            }
        }
        return finished;
    }

    private void logd(String msg) {
        Log.d(LOG_TAG, "[NetworksList] " + msg);
    }

    private void loge(String msg) {
        Log.e(LOG_TAG, "[NetworksList] " + msg);
    }
}
