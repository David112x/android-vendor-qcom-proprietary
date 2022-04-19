/**
 **    Copyright (c) 2019 Qualcomm Technologies, Inc.
 **    All Rights Reserved.
 **    Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/

package com.qualcomm.qti.modemtestmode;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.view.MenuItem;
import android.view.WindowManager;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

public class MbnModeSelect extends PreferenceActivity {
    private final String TAG = "MbnModeSelect";
    private Preference mButtonManualSelection;
    private Preference mButtonAutoSelection;
    private Preference mButtonAutoCtSelection;
    private static final int MANUAL_LOAD = 1;
    private final int EVENT_QCRIL_HOOK_READY = 2;
    private final int EVENT_SET_AUTO_MODE_DONE = 3;
    private final int EVENT_LOAD_CONFIG_FILE = 4;
    private static final String OEM_MBN_LIST_FILE =
            "/data/vendor/modem_config/mcfg_sw/oem_sw.txt";
    private static final String SW_MBN_LIST_FILE =
            "/data/vendor/modem_config/mcfg_sw/mbn_sw.txt";
    private static final String CONFIG_TO_GO_DIR =
            "/data/vendor/modem_config/";
    private List<String> mOemConfigList = new ArrayList<String>();
    private List<String> mSwConfigList = new ArrayList<String>();
    private List<String> mConfigList = new ArrayList<String>();
    private int mCTConfigListSize = 0;
    private int mConfigFileIndex = 0;
    private ProgressDialog mProgressDialog;
    private boolean mRegistered = false;

    //private static MbnModeSelect mInstance = null;
    private Handler mHandler = new Handler() {

        @Override
        public void dispatchMessage(Message msg) {
            switch(msg.what) {
            case EVENT_QCRIL_HOOK_READY:
                log("EVENT_QCRIL_HOOK_READY");
                MbnAppGlobals.getInstance().unregisterQcRilHookReady(mHandler);
                break;

            case EVENT_SET_AUTO_MODE_DONE:
                log("EVENT_SET_AUTO_MODE_DONE");
                MbnAppGlobals.getInstance().setSelectMode((String)msg.obj);

                if(mRegistered) {
                    unregisterReceiver(mBroadcastReceiver);
                    mRegistered = false;
                }

                String message;
                if (msg.arg1 == 0) {
                    message = getString(R.string.succeed_to_set_auto_mode);
                } else {
                    message = getString(R.string.fail_to_set_auto_mode);
                }

                if (mProgressDialog != null && mProgressDialog.isShowing()) {
                    mProgressDialog.dismiss();
                }
                showCompleteDialog(message);
                break;

            case EVENT_LOAD_CONFIG_FILE:
                log("EVENT_LOAD_CONFIG_FILE");
                loadMbnFiles();
                break;

            default:
                log("Unexpected event:" + msg.what);
                break;
            }
        }
    };

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            log("Received: " + action);

            if (MbnTestUtils.PDC_DATA_ACTION.equals(action)) {
                byte[] result = intent.getByteArrayExtra(MbnTestUtils.PDC_ACTIVATED);
                int sub = intent.getIntExtra(MbnTestUtils.SUB_ID, MbnTestUtils.DEFAULT_SUB);
                ByteBuffer payload = ByteBuffer.wrap(result);
                payload.order(ByteOrder.nativeOrder());
                int error = payload.get();
                log("Sub:" + sub + ", activated:" + new String(result) + ", error code:" + error);

                mConfigFileIndex ++;
                mProgressDialog.incrementProgressBy(1);

                Message msg = mHandler.obtainMessage(EVENT_LOAD_CONFIG_FILE);
                msg.sendToTarget();
            } else if (MbnTestUtils.PDC_CONFIG_CLEARED_ACTION.equals(action)) {
                byte[] result = intent.getByteArrayExtra(MbnTestUtils.PDC_ACTIVATED);
                int sub = intent.getIntExtra(MbnTestUtils.SUB_ID, MbnTestUtils.DEFAULT_SUB);
                ByteBuffer payload = ByteBuffer.wrap(result);
                payload.order(ByteOrder.nativeOrder());
                int error = payload.get();
                log("Sub:" + sub + " cleared:" + new String(result) + " error code:" + error);
                switch (error) {
                case MbnTestUtils.CLEANUP_SUCCESS:
                case MbnTestUtils.CLEANUP_ALREADY:
                    startLoadMbn();
                    break;
                case MbnTestUtils.CLEANUP_FAILED:
                    log("Failed to clean up configs");
                    sendAutoModeEvent(1,
                        mConfigList.size() > mCTConfigListSize ? MbnTestUtils.AUTO_MODE : MbnTestUtils.CT_MODE);
                    break;
                default:
                    break;
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //mInstance = this;

        setTitle(getResources().getString(R.string.mbn_file_load_sw));
        addPreferencesFromResource(R.xml.mbn_mode_select);
        mButtonManualSelection = findPreference("manual_select_sw_mbn");
        mButtonAutoSelection = findPreference("auto_select_sw_mbn_key");
        mButtonAutoCtSelection = findPreference("auto_select_ct_sw_mbn_key");

        MbnAppGlobals.getInstance().registerQcRilHookReady(mHandler,
                EVENT_QCRIL_HOOK_READY, null);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        // TODO Auto-generated method stub
        if (preference.getKey().equals("manual_select_sw_mbn_key")) {
            startActivityForResult(new Intent(this, MbnFileLoad.class), MANUAL_LOAD);
        } else if (preference.getKey().equals("auto_select_sw_mbn_key")) {
            String mode = MbnAppGlobals.getInstance().getSelectMode();
            switch(mode) {
            case MbnTestUtils.AUTO_MODE:
                sendAutoModeEvent(0, MbnTestUtils.AUTO_MODE);
                break;

            case MbnTestUtils.DEFAULT_MODE:
            case MbnTestUtils.MANUAL_MODE:
            case MbnTestUtils.CT_MODE:
                if ((!mOemConfigList.isEmpty() && !mSwConfigList.isEmpty())
                    || (getSwMbnFileList(OEM_MBN_LIST_FILE, mOemConfigList)
                    && getSwMbnFileList(SW_MBN_LIST_FILE, mSwConfigList))) {
                    mConfigList.clear();
                    mConfigFileIndex = 0;
                    mConfigList.addAll(mOemConfigList);
                    mConfigList.retainAll(mSwConfigList);

                    startCleanupMbn();
                } else {
                    sendAutoModeEvent(1, MbnTestUtils.AUTO_MODE);
                }
                break;
            }
        } else if (preference.getKey().equals("auto_select_ct_sw_mbn_key")) {
            String mode = MbnAppGlobals.getInstance().getSelectMode();
            switch(mode) {
            case MbnTestUtils.DEFAULT_MODE:
            case MbnTestUtils.MANUAL_MODE:
            case MbnTestUtils.AUTO_MODE:
                 mConfigList.clear();
                 mConfigFileIndex = 0;
                 mConfigList.add("mcfg_sw/generic/China/CMCC/Commercial/Volte_OpenMkt/mcfg_sw.mbn");
                 mConfigList.add("mcfg_sw/generic/China/CT/Commercial/VoLTE_OpenMkt/mcfg_sw.mbn");
                 mConfigList.add("mcfg_sw/generic/China/CU/Commercial/VoLTE/mcfg_sw.mbn");
                 mConfigList.add("mcfg_sw/generic/common/ROW/Commercial/mcfg_sw.mbn");
                 mCTConfigListSize = mConfigList.size();

                 startCleanupMbn();
                break;

            case MbnTestUtils.CT_MODE:
                sendAutoModeEvent(0, MbnTestUtils.CT_MODE);
                break;
            }
        }
        return true;
    }

    @Override
    protected void onActivityResult(final int requestCode,
            final int resultCode, final Intent intent) {
        log("resultCode = " + resultCode + ", requestCode = " + requestCode);
        super.onActivityResult(requestCode, resultCode, intent);

        if(requestCode == MANUAL_LOAD) {
            if (resultCode == MbnFileLoad.RESULT_CODE_FINISH) {
                MbnModeSelect.this.finish();
            }
        }
    }

    private void showCompleteDialog(String msg){
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.exit)
        .setMessage(msg)
        .setPositiveButton(R.string.quit,
                new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
                MbnModeSelect.this.finish();
            }
        })
        .setNegativeButton(R.string.cancel,
                new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
            }
        });

        builder.create().show();
    }

    private boolean getSwMbnFileList(String fileName, List<String> configList) {
        File file = new File(fileName);
        if (file.isFile() && file.exists()) {
            try {
                InputStreamReader inputreader = new InputStreamReader(new FileInputStream(file));
                BufferedReader buffreader = new BufferedReader(inputreader);
                String line = null;
                while (( line = buffreader.readLine()) != null) {
                    if (!"".equals(line) && line.toLowerCase().contains("commerci")) {
                        configList.add(line);
                    }
                }
                inputreader.close();
                buffreader.close();
                return true;
            } catch (Exception e) {
                e.printStackTrace();
            }
        } else {
            log("Can not find file: " + SW_MBN_LIST_FILE);
        }

        return false;
    }

    private void startCleanupMbn() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(MbnTestUtils.PDC_CONFIG_CLEARED_ACTION);
        registerReceiver(mBroadcastReceiver, filter);
        mRegistered = true;

        if (MbnAppGlobals.getInstance().cleanUpConfigs() == false) {
            log("Respond failure to cleanup configs");
            sendAutoModeEvent(1,
                mConfigList.size() > mCTConfigListSize ? MbnTestUtils.AUTO_MODE : MbnTestUtils.CT_MODE);
        }
    }

    private void startLoadMbn() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(MbnTestUtils.PDC_DATA_ACTION);
        registerReceiver(mBroadcastReceiver, filter);
        mRegistered = true;

        showProgressDialog();
        loadMbnFiles();
    }

    private void showProgressDialog() {
        mProgressDialog = new ProgressDialog(this);
        mProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
        mProgressDialog.setMessage(getResources().getString(R.string.Loading_hint));
        mProgressDialog.setCancelable(false);
        mProgressDialog.setProgress(0);
        mProgressDialog.setMax(mConfigList.size());
        mProgressDialog.show();
    }

    private void loadMbnFiles() {
        log("Total size:" + mConfigList.size() + " index:" + mConfigFileIndex);

        if (mConfigFileIndex >= mConfigList.size()) {
            sendAutoModeEvent(
                MbnAppGlobals.getInstance().enableAutoMode() ? 0 : 1,
                mConfigList.size() > mCTConfigListSize ? MbnTestUtils.AUTO_MODE : MbnTestUtils.CT_MODE);
            mConfigList.clear();
            mConfigFileIndex = 0;

            return;
        }

        while (mConfigFileIndex < mConfigList.size()) {
            String fileName = CONFIG_TO_GO_DIR + mConfigList.get(mConfigFileIndex);
            String configId = fileName.replace(MbnTestUtils.SW_MBN_BASE_PATH, "");
            if (fileName.endsWith(MbnTestUtils.MBN_FILE_SUFFIX)) {
                if(MbnAppGlobals.getInstance().setupMbnConfig(fileName,
                        MbnTestUtils.APP_MBN_ID_PREFIX+configId, 0) == false) {
                    log("Load falied, try next" );
                    Message msg = mHandler.obtainMessage(EVENT_LOAD_CONFIG_FILE);
                    msg.sendToTarget();

                    mConfigFileIndex ++;
                    mProgressDialog.incrementProgressBy(1);
                }
                break;
            } else {
                mConfigFileIndex ++;
                mProgressDialog.incrementProgressBy(1);
            }
        }
    }

    private void sendAutoModeEvent(int result, String mode) {
        Message msg = mHandler.obtainMessage(EVENT_SET_AUTO_MODE_DONE);
        msg.arg1 = result;
        msg.obj = mode;
        msg.sendToTarget();
    }

    private void log(String msg) {
        Log.d(TAG, "MbnTest_" + msg);
    }
}
