/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

package org.codeaurora.qti.nrNetworkSettingApp;

import android.content.ComponentName;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.app.Activity;
import android.util.Log;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;
import android.telephony.TelephonyManager;
import android.provider.Settings;
import android.content.ContentResolver;
import android.content.Intent;
import android.database.ContentObserver;
import android.net.Uri;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.AdapterView.OnItemSelectedListener;
import java.util.ArrayList;
import java.util.List;

import org.codeaurora.internal.BearerAllocationStatus;
import org.codeaurora.internal.Client;
import org.codeaurora.internal.DcParam;
import org.codeaurora.internal.IExtTelephony;
import org.codeaurora.internal.INetworkCallback;
import org.codeaurora.internal.NetworkCallbackBase;
import org.codeaurora.internal.NrConfigType;
import org.codeaurora.internal.ServiceUtil;
import org.codeaurora.internal.SignalStrength;
import org.codeaurora.internal.Status;
import org.codeaurora.internal.Token;
import org.codeaurora.internal.UpperLayerIndInfo;
import org.codeaurora.internal.NrIconType;

public class MainActivity extends Activity {
    private static final String TAG = "MainActivity";

    private static final int EVENT_GET_NR_DUAL_CONNECTIVITY_PARAM = 0;
    private static final int EVENT_SET_ENDC_AVAILABLE_STATUS = 1;
    private static final int EVENT_ACTIVATE_ENABLE_5G_BUTTON = 2;
    private static final int EVENT_DEACTIVATE_ENABLE_5G_BUTTON = 3;
    private static final int EVENT_SET_5G_ENABLE_STATUS = 4;
    private static final int EVENT_ON_5G_ENABLE_STATUS = 5;
    private static final int EVENT_ON_5G_SIGNAL_STRENGTH_CHANGE_IND = 6;
    private static final int EVENT_ON_NR_DUAL_CONNECTIVITY_CHANGE_IND = 7;
    private static final int EVENT_ON_RESTRICT_DCNR_CHANGE = 8;
    private static final int EVENT_SET_NR_DUAL_CONNECTIVITY_PARAM = 9;
    private static final int EVENT_ON_BEARER_ALLOCATION_STATUS_CHANGE_IND = 10;
    private static final int EVENT_ON_UPPER_LAYER_STATUS_CHANGE_IND = 11;
    private static final int EVENT_ON_5G_CONFIG_INFO_CHANGE_IND = 12;
    private static final int EVENT_ON_NR_ICON_TYPE_CHANGE_IND = 13;
    private static final int EVENT_ACTIVATE_ENABLE_ENDC_BUTTON = 14;
    private static final int EVENT_DEACTIVATE_ENABLE_ENDC_BUTTON = 15;
    private static final int EVENT_ON_ENDC_STATUS = 16;

    public static final String FIVEG_UWB_INDICATOR_CONFIG = "5gUwbIndicatorConfig";
    public static final String FIVEG_BASIC_INDICATOR_CONFIG = "5gBasicIndicatorConfig";

    private static final int PRIMARY_SLOT = 0;
    private static final int SECONDARY_SLOT = 1;
    private static final int SINGLE_SIM = 1;

    private boolean mIsServiceBound;
    private IExtTelephony mNetworkService;
    private String mPackageName;
    private Client mClient;
    public ContentResolver resolver;
    private int mPhoneCount;
    FiveGIndicatorContentObserver[] mObserver;
    FiveGSpinner fiveGSpinner;

    private Handler mMainThreadHandler = new Handler() {
        private static final String TAG = "MainThreadHandler: ";

        private Switch getEnable5gOnSwitchForSlotId(int slotId) {
            int resourceId = slotId == 0 ? R.id.enable5gOnSlot1 : R.id.enable5gOnSlot2;
            return (Switch) findViewById(resourceId);
        }

        private Switch getEnableEndcOnSwitchForSlotId(int slotId) {
            int resourceId = slotId == 0 ? R.id.enableEndcOnSlot1 : R.id.enableEndcOnSlot2;
            return (Switch) findViewById(resourceId);
        }

        private TextView get5gDataConnectionStatusTextViewForSlotId(int slotId) {
            int resourceId
                    = slotId == 0 ? R.id.value5gDataConnectionStatusSlot1 : R.id
                    .value5gDataConnectionStatusSlot2;
            return (TextView) findViewById(resourceId);
        }

        private TextView getEndcAvailableTextViewForSlotId(int slotId) {
            int resourceId
                    = slotId == 0 ? R.id.valueEndcAvailableSlot1 : R.id.valueEndcAvailableSlot2;
            return (TextView) findViewById(resourceId);
        }

        private TextView get5gSignalStrengthTextViewForSlotId(int slotId) {
            int resourceId
                    = slotId == 0 ? R.id.value5gSignalStrengthSlot1 : R.id
                    .value5gSignalStrengthSlot2;
            return (TextView) findViewById(resourceId);
        }

        private TextView getRestrictDcnrTextViewForSlotId(int slotId) {
            int resourceId
                    = slotId == 0 ? R.id.valueRestrictDcnrSlot1 : R.id.valueRestrictDcnrSlot2;
            return (TextView) findViewById(resourceId);
        }

        private TextView getPlmnInfoListSlotId(int slotId) {
            int resourceId
                    = slotId == 0 ? R.id.valuePlmnInfoListSlot1 : R.id.valuePlmnInfoListSlot2;
            return (TextView) findViewById(resourceId);
        }

        private TextView getUpperLayerIndStatusSlotId(int slotId) {
            int resourceId = (slotId == 0) ? R.id.valueUpperLayerIndStatusSlot1 :
                    R.id.valueUpperLayerIndStatusSlot2;
            return (TextView) findViewById(resourceId);
        }

        private TextView get5gConfigInfoTextViewForSlotId(int slotId) {
            int resourceId
                    = slotId == 0 ? R.id.valueConfigTypeSlot1 : R.id
                    .valueConfigTypeSlot2;
            return (TextView) findViewById(resourceId);
        }

        private TextView getNrIconTypeTextViewForSlotId(int slotId) {
            int resourceId
                    = slotId == 0 ? R.id.valueIconTypeSlot1 : R.id
                    .valueIconTypeSlot2;
            return (TextView) findViewById(resourceId);
        }

        private TextView getEndcStatusTextViewForSlotId(int slotId) {
            int resourceId
                    = slotId == 0 ? R.id.valueEndcStatusSlot1 : R.id
                    .valueEndcStatusSlot2;
            return (TextView) findViewById(resourceId);
        }


        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "handleMessage msg.what = " + msg.what);
            switch (msg.what) {
                case EVENT_ACTIVATE_ENABLE_5G_BUTTON: {
                    int slotId = msg.arg1;
                    Log.d(TAG, "EVENT_ACTIVATE_ENABLE_5G_BUTTON: slotId = " + slotId);
                    Switch enable5gOnSwitch = getEnable5gOnSwitchForSlotId(slotId);
                    enable5gOnSwitch.setEnabled(true);
                    Log.d(TAG, "App package: " + getApplicationContext().getPackageName());
                    break;
                }

                case EVENT_DEACTIVATE_ENABLE_5G_BUTTON: {
                    int slotId = msg.arg1;
                    Log.d(TAG, "EVENT_DEACTIVATE_ENABLE_5G_BUTTON: slotId = " + slotId);
                    Switch enable5gOnSwitch = getEnable5gOnSwitchForSlotId(slotId);
                    enable5gOnSwitch.setEnabled(false);
                    break;
                }

                case EVENT_ACTIVATE_ENABLE_ENDC_BUTTON: {
                    int slotId = msg.arg1;
                    Log.d(TAG, "EVENT_ACTIVATE_ENABLE_ENDC_BUTTON: slotId = " + slotId);
                    Switch enableEndcOnSwitch = getEnableEndcOnSwitchForSlotId(slotId);
                    enableEndcOnSwitch.setEnabled(true);
                    break;
                }

                case EVENT_DEACTIVATE_ENABLE_ENDC_BUTTON: {
                    int slotId = msg.arg1;
                    Log.d(TAG, "EVENT_DEACTIVATE_ENABLE_ENDC_BUTTON: slotId = " + slotId);
                    Switch enableEndcOnSwitch = getEnableEndcOnSwitchForSlotId(slotId);
                    enableEndcOnSwitch.setEnabled(false);
                    break;
                }

                case EVENT_SET_5G_ENABLE_STATUS:
                case EVENT_ON_5G_ENABLE_STATUS: {
                    int slotId = msg.arg1;
                    boolean enabled = msg.arg2 == 1;
                    Log.d(TAG, "EVENT_SET_5G_ENABLE_STATUS: slotId = " + slotId);

                    Switch enable5gOnSwitch = getEnable5gOnSwitchForSlotId(slotId);
                    enable5gOnSwitch.setChecked(enabled);
                    break;
                }

                case EVENT_SET_NR_DUAL_CONNECTIVITY_PARAM:
                case EVENT_ON_NR_DUAL_CONNECTIVITY_CHANGE_IND: {
                    Log.d(TAG, "EVENT NR_DUAL_CONNECTIVITY PARAM : slotId = " + msg.arg1);
                    DcParam dcParam = (DcParam) msg.obj;
                    int slotId = msg.arg1;
                    TextView valueEndcAvailable = getEndcAvailableTextViewForSlotId(slotId);

                    String value = (dcParam
                            .getEndc() == DcParam.ENDC_AVAILABLE) ? "AVAILABLE" : "UNAVAILABLE";
                    valueEndcAvailable.setText(value);

                    TextView valueRestrictDcnr = getRestrictDcnrTextViewForSlotId(msg.arg1);
                    value = (dcParam
                            .getDcnr() == DcParam.DCNR_UNRESTRICTED) ? "UNRESTRICTED" :
                            "RESTRICTED";
                    valueRestrictDcnr.setText(value);
                    break;
                }

                case EVENT_ON_BEARER_ALLOCATION_STATUS_CHANGE_IND: {
                    Log.d(TAG,
                            "EVENT_ON_BEARER_ALLOCATION_STATUS_CHANGE_IND: slotId = " + msg.arg1 +
                                    " " + "allocated = " + msg.obj);
                    TextView value5gDataConnectionStatus
                            = get5gDataConnectionStatusTextViewForSlotId(msg.arg1);
                    BearerAllocationStatus bStatus = (BearerAllocationStatus) msg.obj;
                    String value = "INVALID";
                    switch (bStatus.get()){
                        case BearerAllocationStatus.NOT_ALLOCATED: {
                            value = "NOT-ALLOCATED";
                            break;
                        }
                        case BearerAllocationStatus.ALLOCATED: {
                            value = "ALLOCATED";
                            break;
                        }
                        case BearerAllocationStatus.MMW_ALLOCATED: {
                            value = "MMW-ALLOCATED";
                            break;
                        }
                        default:
                            Log.e(TAG, "Invalid bearer status");
                    }
                    value5gDataConnectionStatus.setText(value);
                    break;
                }

                case EVENT_ON_UPPER_LAYER_STATUS_CHANGE_IND: {
                    UpperLayerIndInfo uliInfo = (UpperLayerIndInfo) msg.obj;
                    int slotId = msg.arg1;
                    Log.d(TAG, "EVENT_ON_UPPER_LAYER_STATUS_CHANGE_IND = "+ msg.arg1 + " "
                            + "UpperLayerIndInfo = " + uliInfo);
                    TextView plmnView = getPlmnInfoListSlotId(slotId);

                    String value = (uliInfo
                            .getPlmnInfoListAvailable() == UpperLayerIndInfo.
                            PLMN_INFO_LIST_AVAILABLE) ? "AVAILABLE" : "UNAVAILABLE";
                    plmnView.setText(value);

                    TextView upperLayerIndView = getUpperLayerIndStatusSlotId(msg.arg1);
                    value = (uliInfo.getUpperLayerIndInfoAvailable() == UpperLayerIndInfo.
                            UPPER_LAYER_IND_INFO_AVAILABLE) ? "AVAILABLE" : "UNAVAILABLE";
                    upperLayerIndView.setText(value);
                    break;
                }

                case EVENT_ON_5G_SIGNAL_STRENGTH_CHANGE_IND: {
                    SignalStrength signalStrength = (SignalStrength) msg.obj;
                    Log.d(TAG,
                            "EVENT_ON_5G_SIGNAL_STRENGTH_CHANGE_IND: slotId = " + msg.arg1 + " "
                                    + "signaStrength = " + signalStrength);
                    TextView value5gSignalStrength = get5gSignalStrengthTextViewForSlotId(msg.arg1);
                    String text = (signalStrength != null) ? signalStrength.toString() : "";
                    value5gSignalStrength.setText(text);
                    break;
                }

                case EVENT_ON_5G_CONFIG_INFO_CHANGE_IND: {
                    NrConfigType nrConfigType = (NrConfigType) msg.obj;
                    Log.d(TAG,
                            "EVENT_ON_5G_CONFIG_INFO_CHANGE_IND: slotId = " + msg.arg1 + " " +
                                    "networkconfgitype = " + nrConfigType);
                    TextView value5gConfigType
                            = get5gConfigInfoTextViewForSlotId(msg.arg1);
                    String value = (nrConfigType
                            .get() == NrConfigType.NSA_CONFIGURATION) ?
                            "NSA" : "SA";
                    value5gConfigType.setText(value);
                    break;
                }

                case EVENT_ON_NR_ICON_TYPE_CHANGE_IND: {
                    NrIconType iconType = (NrIconType) msg.obj;
                    Log.d(TAG,
                            "EVENT_ON_NR_ICON_TYPE_CHANGE_IND: slotId = " + msg.arg1 + " " +
                                    "icontype = " + iconType);
                    TextView valueIconType
                            = getNrIconTypeTextViewForSlotId(msg.arg1);
                    String value;
                    if(iconType.get() == NrIconType.TYPE_NONE){
                        value = "NONE";
                    } else if (iconType.get() == NrIconType.TYPE_5G_BASIC) {
                        value = "BASIC";
                    } else if(iconType.get() == NrIconType.TYPE_5G_UWB){
                        value = "UWB";
                    } else {
                        value = "INVALID";
                    }
                    valueIconType.setText(value);
                    break;
                }

                case EVENT_ON_ENDC_STATUS: {
                    boolean endcStatus = (boolean) msg.obj;
                    int slotId = msg.arg1;
                    Log.d(TAG, "EVENT_ON_ENDC_STATUS = "+ msg.arg1 + " "
                            + "endcstatus = " + endcStatus);
                    TextView endcStatusView = getEndcStatusTextViewForSlotId(slotId);
                    String value = (endcStatus) ? "ENABLED" : "DISABLED";
                    endcStatusView.setText(value);
                    break;
                }
            }
        }
    };

    private ServiceConnection mMainServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "onServiceConnected:");

            try {
                mNetworkService = IExtTelephony.Stub.asInterface(service);
                mClient = mNetworkService.registerCallback(mPackageName, mCallback);
                Log.d(TAG, "Client = " + mClient);
                mIsServiceBound = true;
                for (int slot = 0; slot < mPhoneCount; slot++) {
                    mMainThreadHandler.sendMessage(mMainThreadHandler
                            .obtainMessage(EVENT_ACTIVATE_ENABLE_5G_BUTTON, slot, -1));
                    mMainThreadHandler.sendMessage(mMainThreadHandler
                            .obtainMessage(EVENT_ACTIVATE_ENABLE_ENDC_BUTTON, slot, -1));
                    mNetworkService.query5gStatus(slot, mClient);
                    mNetworkService.queryNrIconType(slot, mClient);
                }
            } catch (RemoteException e) {
                Log.d(TAG, "onServiceConnected: Exception = " + e);
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "onServiceDisconnected:");
            cleanup();
        }

        @Override
        public void onBindingDied(ComponentName name) {
            Log.d(TAG, "onBindingDied:");
            cleanup();
        }

        private void cleanup() {
            Log.d(TAG, "cleanup");
            mIsServiceBound = false;
            mNetworkService = null;
            mClient = null;
            mMainThreadHandler.sendMessage(
                    mMainThreadHandler.obtainMessage(EVENT_DEACTIVATE_ENABLE_5G_BUTTON));
        }
    };

    private INetworkCallback mCallback = new NetworkCallbackBase() {
        @Override
        public void on5gStatus(int slotId, Token token, Status status, boolean enableStatus) throws
                RemoteException {
            Log.d(TAG, "on5gStatus: slotId = " + slotId + " token = " + token + " status" +
                    status + " enableStatus = " + enableStatus);

            mMainThreadHandler.sendMessage(mMainThreadHandler
                    .obtainMessage(EVENT_ACTIVATE_ENABLE_5G_BUTTON, slotId, -1));
        }

        @Override
        public void onNrDcParam(int slotId, Token token, Status status, DcParam dcParam) throws
                RemoteException {
            Log.d(TAG, "onNrDcParam: slotId = " + slotId + " token = " + token + " status" +
                    status + " dcParam = " + dcParam);
            if (status.get() == Status.SUCCESS) {
                mMainThreadHandler.sendMessage(mMainThreadHandler
                        .obtainMessage(EVENT_ON_NR_DUAL_CONNECTIVITY_CHANGE_IND, slotId, -1,
                                dcParam));
            }
        }

        @Override
        public void onAnyNrBearerAllocation(int slotId, Token token, Status status,
                BearerAllocationStatus bearerStatus) throws RemoteException {
            Log.d(TAG,
                    "onNrBearerAllocationChange: slotId = " + slotId + " token = " + token + " " +
                            "status" + status + " bearerStatus = " + bearerStatus);

            if (status.get() == Status.SUCCESS) {
                mMainThreadHandler.sendMessage(mMainThreadHandler
                        .obtainMessage(EVENT_ON_BEARER_ALLOCATION_STATUS_CHANGE_IND,
                                slotId, -1, bearerStatus));
            }
        }

        @Override
        public void onUpperLayerIndInfo(int slotId, Token token, Status status,
                UpperLayerIndInfo uilInfo) throws RemoteException {
            Log.d(TAG,
                    "onUpperLayerIndInfo: slotId = " + slotId + " token = " + token + " " +
                            "status" + status + " UpperLayerIndInfo = " + uilInfo);

            if (status.get() == Status.SUCCESS) {
                mMainThreadHandler.sendMessage(mMainThreadHandler.
                        obtainMessage(EVENT_ON_UPPER_LAYER_STATUS_CHANGE_IND,
                        slotId, -1, uilInfo));
            }
        }

        @Override
        public void onSignalStrength(int slotId, Token token, Status status, SignalStrength
                signalStrength) throws RemoteException {
            Log.d(TAG,
                    "onSignalStrength: slotId = " + slotId + " token = " + token + " " + "status"
                            + status + " signalStrength = " + signalStrength);

            if (status.get() == Status.SUCCESS) {
                mMainThreadHandler.sendMessage(mMainThreadHandler
                        .obtainMessage(EVENT_ON_5G_SIGNAL_STRENGTH_CHANGE_IND, slotId, -1,
                                signalStrength));
            }
        }


        @Override
        public void on5gConfigInfo(int slotId, Token token, Status status, NrConfigType
                nrConfigType) throws RemoteException {
            Log.d(TAG,
                    "on5gConfigInfo: slotId = " + slotId + " token = " + token + " " + "status"
                            + status + " NrConfigType = " + nrConfigType);

            if (status.get() == Status.SUCCESS) {
                mMainThreadHandler.sendMessage(mMainThreadHandler
                        .obtainMessage(EVENT_ON_5G_CONFIG_INFO_CHANGE_IND, slotId, -1,
                                nrConfigType));
            }
        }

        @Override
        public void onNrIconType(int slotId, Token token, Status status, NrIconType
                nrIconType) throws RemoteException {
            Log.d(TAG,
                    "onNrIconType: slotId = " + slotId + " token = " + token + " " + "status"
                            + status + " NrIconType = " + nrIconType);

            if (status.get() == Status.SUCCESS) {
                mMainThreadHandler.sendMessage(mMainThreadHandler
                        .obtainMessage(EVENT_ON_NR_ICON_TYPE_CHANGE_IND, slotId, -1,
                                nrIconType));
            }
        }

        @Override
        public void onEnableEndc(int slotId, Token token, Status status) throws RemoteException {
            Log.d(TAG,
                    "onEnableEndc: slotId = " + slotId + " token = " + token + " " + "status"
                            + status);

            mMainThreadHandler.sendMessage(mMainThreadHandler
                    .obtainMessage(EVENT_ACTIVATE_ENABLE_ENDC_BUTTON, slotId, -1));
            //After user enable/disable endc, query the status and display on UI
            if (status.get() == Status.SUCCESS) {
                mNetworkService.queryEndcStatus(slotId, mClient);
            }
        }

        @Override
        public void onEndcStatus(int slotId, Token token, Status status, boolean endcStatus) throws RemoteException {
            Log.d(TAG,
                    "onEndcStatus: slotId = " + slotId + " token = " + token + " " + "status"
                            + status + " endcStatus = " + endcStatus);

            if (status.get() == Status.SUCCESS) {
                mMainThreadHandler.sendMessage(mMainThreadHandler
                        .obtainMessage(EVENT_ON_ENDC_STATUS, slotId, -1,
                                endcStatus));
            }
        }
    };

    private void enableContentObservers(Uri uwbUri, Uri basicUri,
            FiveGIndicatorContentObserver mObserver) {
        resolver.registerContentObserver(uwbUri, false, mObserver);
        resolver.registerContentObserver(basicUri, false, mObserver);
    }

    private Uri formUri(String uriStr, int slotId) {
        return Settings.Global.getUriFor(uriStr + slotId);
    }

    public class FiveGIndicatorContentObserver extends ContentObserver {
        private int mSlotId;
        public FiveGIndicatorContentObserver(int slotId) {
            super(new Handler());
            mSlotId = slotId;
        }
        @Override
        public void onChange(final boolean selfChange, Uri uri) {
            String uwbIndConf;
            String basicIndConf;
            try {
                uwbIndConf = Settings.Global.getString(resolver,
                            FIVEG_UWB_INDICATOR_CONFIG + mSlotId);
                basicIndConf = Settings.Global.getString(resolver,
                            FIVEG_BASIC_INDICATOR_CONFIG + mSlotId);

                Log.i(TAG, "5G Indicator Config updated on SlotId "+ mSlotId+ " uwbIndConf: "+
                        uwbIndConf + " basicIndConf: " + basicIndConf);
            }catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public class FiveGSpinner implements OnItemSelectedListener {
        @Override
        public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
            Spinner spin = (Spinner) parent;
            String item = parent.getItemAtPosition(position).toString();
            int slot = (spin.getId() == R.id.uwbSpinnerSlot2 || spin.getId() ==
                    R.id.basicSpinnerSlot2) ? SECONDARY_SLOT : PRIMARY_SLOT;
            String config = (spin.getId() == R.id.uwbSpinnerSlot1 || spin.getId() == R.id.
                    uwbSpinnerSlot2) ? FIVEG_UWB_INDICATOR_CONFIG : FIVEG_BASIC_INDICATOR_CONFIG;
            Log.i(TAG, "setting " + config + ": " + item + " on Slot Id:" + slot);
            Settings.Global.putString(resolver, config + slot, item);
        }

        public void onNothingSelected(AdapterView<?> arg0) {
            // TODO Auto-generated method stub
        }
    }


   private void processIntent(final Intent newIntent) {
        Switch enable5gOnSlot1Switch = (Switch) findViewById(R.id.enable5gOnSlot1);
        Switch enable5gOnSlot2Switch = (Switch) findViewById(R.id.enable5gOnSlot2);
        int intentSlot = newIntent.getIntExtra("SLOT", 0);
        boolean intentStatus = (newIntent.getIntExtra("STATUS", 0) == 1)? true: false;

        if (intentSlot == 0) {
            Log.d(TAG, "enable5gOnSlot1Switch: Intent requested status =  " + intentStatus);
            enable5gOnSlot1Switch.setChecked(intentStatus);
        } else {
            Log.d(TAG, "enable5gOnSlot2Switch: Intent requested status =  " + intentStatus);
            enable5gOnSlot2Switch.setChecked(intentStatus);
        }
   }

    private void setUpObserver(int slot) {
        /** Observer to get notified when Mobile data enabled status changes */
        mObserver[slot] = new FiveGIndicatorContentObserver(slot);
        Uri uwbUri = formUri(FIVEG_UWB_INDICATOR_CONFIG, slot);
        Uri basicUri = formUri(FIVEG_BASIC_INDICATOR_CONFIG, slot);
        enableContentObservers(uwbUri, basicUri, mObserver[slot]);
    }

    private List<String> getUwbCategories() {
        // uwbSpinner Drop down elements
        List<String> uwbCategories = new ArrayList<String>();
        uwbCategories.add("Configuration1");
        uwbCategories.add("Configuration2");
        uwbCategories.add("spare1");
        uwbCategories.add("spare2");
        return uwbCategories;
    }

    private List<String> getBasicCategories() {
        // basicSpinner Drop down elements
        List<String> basicCategories = new ArrayList<String>();
        basicCategories.add("r15Enabled");
        basicCategories.add("r15Disabled");
        basicCategories.add("spare1");
        basicCategories.add("spare2");
        return basicCategories;
    }

    private void setUpSpinner(Spinner spinner, String spinnerType, int slot) {
        //Spinner Drop down elements
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, (spinnerType.equals(
                FIVEG_UWB_INDICATOR_CONFIG)) ? getUwbCategories() : getBasicCategories());
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);
        String indConf = Settings.Global.getString(resolver,
                spinnerType + slot);
        if(indConf != null){
            Log.d(TAG, "Read previous " + spinnerType + " " + indConf + " on slot " +  slot);
            String currentItem;
            for(int i = 0; i < adapter.getCount(); i++){
                currentItem = adapter.getItem(i);
                if(currentItem.equals(indConf)){
                    spinner.setSelection(i);
                }
            }
        }
        spinner.setOnItemSelectedListener(fiveGSpinner);
    }

    @Override
    public void onNewIntent(final Intent newIntent) {
        super.onNewIntent(newIntent);
        processIntent(newIntent);

    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate: mIsServiceBound = " + mIsServiceBound);

        super.onCreate(savedInstanceState);
        mPackageName = getPackageName();
        setContentView(R.layout.main);

        resolver = getApplicationContext().getContentResolver();

        TelephonyManager tm = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
        //TODO: getActiveSubscriptions instead of phone count and handle NO SIM scenarios
        mPhoneCount = tm.getPhoneCount();

        // Use the ServiceUtil.bindService to connect to correct service.
        ServiceUtil.bindService(this, mMainServiceConnection);

        // Setup UI for primary slot
        mObserver = new FiveGIndicatorContentObserver[mPhoneCount];
        setUpObserver(PRIMARY_SLOT);
        fiveGSpinner = new FiveGSpinner();
        // uwbSpinner element
        Spinner uwbSpinner1 = (Spinner) findViewById(R.id.uwbSpinnerSlot1);
        setUpSpinner(uwbSpinner1, FIVEG_UWB_INDICATOR_CONFIG, PRIMARY_SLOT);
        // basicSpinner element
        Spinner basicSpinner1 = (Spinner) findViewById(R.id.basicSpinnerSlot1);
        setUpSpinner(basicSpinner1, FIVEG_BASIC_INDICATOR_CONFIG, PRIMARY_SLOT);
        // enable5gOnSlotXSwitch would be active only if we have a service connection.
        Switch enable5gOnSlot1Switch = (Switch) findViewById(R.id.enable5gOnSlot1);
        CompoundButton.OnCheckedChangeListener listenerSlot1 = new CompoundButton.
                OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                handle5gButton(isChecked, PRIMARY_SLOT);
            }
        };
        enable5gOnSlot1Switch.setOnCheckedChangeListener(listenerSlot1);

        // Enable/Disable Endc
        Switch enableEndcOnSlot1Switch = (Switch) findViewById(R.id.enableEndcOnSlot1);
        CompoundButton.OnCheckedChangeListener EndcListenerSlot1 = new CompoundButton.
                OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                handleEndcButton(isChecked, PRIMARY_SLOT);
            }
        };
        enableEndcOnSlot1Switch.setOnCheckedChangeListener(EndcListenerSlot1);

        if (mPhoneCount > SINGLE_SIM) {
            loadUiForSecondarySlot();
        }

        //Process any intent related extra actions.
        processIntent(getIntent());
    }

    private void loadUiForSecondarySlot() {
        setUpObserver(SECONDARY_SLOT);
        // uwbSpinner element
        Spinner uwbSpinner2 = (Spinner) findViewById(R.id.uwbSpinnerSlot2);
        setUpSpinner(uwbSpinner2, FIVEG_UWB_INDICATOR_CONFIG, SECONDARY_SLOT);
        // basicSpinner element
        Spinner basicSpinner2 = (Spinner) findViewById(R.id.basicSpinnerSlot2);
        setUpSpinner(basicSpinner2, FIVEG_BASIC_INDICATOR_CONFIG, SECONDARY_SLOT);
        Switch enable5gOnSlot2Switch = (Switch) findViewById(R.id.enable5gOnSlot2);
        CompoundButton.OnCheckedChangeListener listenerSlot2 = new CompoundButton.
                OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                handle5gButton(isChecked, SECONDARY_SLOT);
            }
        };
        enable5gOnSlot2Switch.setOnCheckedChangeListener(listenerSlot2);
    }

    private void handle5gButton(boolean isChecked, int slotId) {
        if (mNetworkService != null) {
            try {
                Log.d(TAG, "enable5gOnSwitch: requested " + "" + isChecked +
                        " on slot" + slotId);
                Token token = (isChecked) ?
                        mNetworkService.enable5g(slotId, mClient) :
                        mNetworkService.disable5g(slotId, mClient);
                Log.d(TAG, "enable5gOnSwitch: " + token);
                mMainThreadHandler.sendMessage(mMainThreadHandler
                        .obtainMessage(EVENT_DEACTIVATE_ENABLE_5G_BUTTON, slotId,
                                -1));
            } catch (RemoteException e) {
                Log.d(TAG, "enable5gOnSwitch: Remote exception = " + e);
            }
        }
    }

    private void handleEndcButton(boolean isChecked, int slotId) {
        if (mNetworkService != null) {
            try {
                Log.d(TAG, "enableEndcOnSwitch: requested " + "" + isChecked +
                        " on slot" + slotId);
                Token token = mNetworkService.enableEndc(slotId, isChecked, mClient);
                Log.d(TAG, "enableEndcOnSwitch: " + token);
                mMainThreadHandler.sendMessage(mMainThreadHandler
                        .obtainMessage(EVENT_DEACTIVATE_ENABLE_ENDC_BUTTON, slotId,
                                -1));
            } catch (RemoteException e) {
                Log.d(TAG, "enableEndcOnSwitch: Remote exception = " + e);
            }
        }
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        if (mIsServiceBound) {
            try {
                mNetworkService.unRegisterCallback(mCallback);
            } catch (RemoteException e) {
                Log.d(TAG, "onDestroy: Exception = " + e);
            }
            ServiceUtil.unbindService(mMainServiceConnection);
            mIsServiceBound = false;
            mNetworkService = null;
            mClient = null;
        }
        super.onDestroy();
    }
}
