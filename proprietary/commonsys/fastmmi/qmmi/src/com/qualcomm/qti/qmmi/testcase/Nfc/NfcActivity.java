/*
 * Copyright (c) 2017, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.qmmi.testcase.Nfc;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nfc.NfcAdapter;
import android.nfc.NfcManager;
import android.nfc.Tag;
import android.os.Bundle;

import com.qualcomm.qti.qmmi.R;
import com.qualcomm.qti.qmmi.bean.TestCase;
import com.qualcomm.qti.qmmi.framework.BaseActivity;
import com.qualcomm.qti.qmmi.utils.LogUtils;
import com.qualcomm.qti.qmmi.utils.Utils;

public class NfcActivity extends BaseActivity {

    private NfcAdapter mNfcAdapter;
    private PendingIntent mPendingIntent;
    private boolean isHistoryEnabled;
    private Tag mTag;
    private volatile boolean isRegisteredReceiver = false;

    private BroadcastReceiver mReceiver = new BroadcastReceiver(){
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (NfcAdapter.ACTION_ADAPTER_STATE_CHANGED.equals(action)){
                handleNfcStateChanged(intent.getIntExtra(NfcAdapter.EXTRA_ADAPTER_STATE,
                        NfcAdapter.STATE_OFF));
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        loadData();
    }

    private void loadData(){
        NfcManager nfcManager = (NfcManager) getSystemService(NFC_SERVICE);
        mNfcAdapter = nfcManager.getDefaultAdapter();
        if ( mNfcAdapter != null){
            isHistoryEnabled = mNfcAdapter.isEnabled();
            LogUtils.logd("NFC History Enable State is: " + isHistoryEnabled);
            mPendingIntent = PendingIntent.getActivity(this, 0, new Intent(this, getClass())
                    .addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP), 0);
        }
    }

    @Override
    protected void onResume() {

        // TODO Auto-generated method stub
        LogUtils.logd("onResume");
        super.onResume();

        if ( mNfcAdapter == null ){
            mTextView.setText(R.string.NFC_not_support);
            mTestCase.addTestData("nfc", "not support");
            updateResult(TestCase.STATE_FAIL);
            return;
        } else {
            if (mNfcAdapter.isEnabled() == false){
                //Register if need
                if(!isRegisteredReceiver){
                    IntentFilter intentFilter = new IntentFilter(NfcAdapter.ACTION_ADAPTER_STATE_CHANGED);
                    registerReceiver(mReceiver, intentFilter);
                    isRegisteredReceiver = true;
                }
                enableNFC(true);
            } else {
                readyForScan();
                if (mTag == null){
                    mTextView.setText(R.string.NFC_place_info);
                    mTextView.setBackgroundResource(0);
                }
            }
        }
    }

    private void enableNFC( boolean enable){
        if (enable){
            mNfcAdapter.enable();
            mTextView.setText(R.string.NFC_enabling);
            mTextView.setBackgroundResource(0);
        } else {
            mNfcAdapter.disable();
            mTextView.setBackgroundResource(0);
        }
    }

    @Override
    protected void onNewIntent(Intent intent) {

        LogUtils.logd("onNewIntent");

        Tag tag = intent.getParcelableExtra(NfcAdapter.EXTRA_TAG);
        updateTag(tag);
    }

    private void handleNfcStateChanged(int state){
        LogUtils.logd("state:" + state);
        switch(state) {
            case NfcAdapter.STATE_ON:
                readyForScan();
                mTextView.setText(R.string.NFC_place_info);
                break;
            }
    }

    private void readyForScan(){
        try {
            mNfcAdapter.enableForegroundDispatch(this, mPendingIntent, null, null);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void updateTag(Tag tag) {
        mTag = tag;
        byte[] id = tag.getId();
        LogUtils.logd("Tag id:" + id);

        if (id == null) {
            mTextView.setText(R.string.NFC_null_tag);
            updateResult(TestCase.STATE_FAIL);
            return;
        }

        String result = (Utils.byteTo16(id)).toString();
        LogUtils.logd("Tag Parse result:" + result);
        mTextView.setText(result);
        mTestCase.addTestData("nfc", "id:" + result);
        updateResult(TestCase.STATE_PASS);
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        super.onPause();
        if ( mNfcAdapter != null ){
            mNfcAdapter.disableForegroundDispatch(this);
            if (isRegisteredReceiver){
                unregisterReceiver(mReceiver);
                isRegisteredReceiver = false;
            }
        }
    }

    @Override
    protected void onDestroy() {
        LogUtils.logd("onDestory");
        if ( mNfcAdapter != null && mNfcAdapter.isEnabled() && isHistoryEnabled == false){
            LogUtils.logd("disable NFC");
            enableNFC(false);
        }
        super.onDestroy();
    }

    @Override
    protected int getLayoutId() {
        return R.layout.case_act;
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
    }
}
