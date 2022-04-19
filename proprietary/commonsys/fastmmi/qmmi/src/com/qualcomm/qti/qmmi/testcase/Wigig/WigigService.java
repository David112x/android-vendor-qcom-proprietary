/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

package com.qualcomm.qti.qmmi.testcase.Wigig;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.os.Handler;
import android.util.Log;
import android.widget.Toast;
import android.os.SystemProperties;

import com.qualcomm.qti.qmmi.R;
import com.qualcomm.qti.qmmi.bean.TestCase;
import com.qualcomm.qti.qmmi.framework.BaseService;
import com.qualcomm.qti.qmmi.utils.LogUtils;
import com.qualcomm.qti.qmmi.utils.Utils;
import com.qualcomm.qti.wigig.WigigManager;
import com.qualcomm.qti.wigig.ScanResult;

import java.util.List;

public class WigigService extends BaseService {
    private static WigigManager mWigigManager = null;
    private static List<ScanResult> mWigigScanResult;

    private final static int SCAN_INTERVAL = 10000;

    private TestCase mTestCase;
    private volatile boolean isRegisteredReceiver = false;

    public int onStartCommand(Intent intent, int flags, int startId) {
        LogUtils.logi("onStartCommand");
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void register() {
    }

    @Override
    public int stop(TestCase testCase) {
        finish();
        return 0;
    }

    @Override
    public int run(TestCase testCase) {
        LogUtils.logi("SensorService service run");
        mTestCase = testCase;
        start();
        return 0;
    }

    private void start() {
        mWigigManager = WigigManager.getInstance(this);
        if (mWigigManager == null){
            updateView(mTestCase.getName(), this.getResources().getString(R.string.wigig_service_notloaded));
            updateResultForCase(mTestCase.getName(), TestCase.STATE_FAIL);
            return;
        }

        IntentFilter mFilter = new IntentFilter();
        mFilter.addAction(WigigManager.SCAN_RESULTS_AVAILABLE_ACTION);
        this.getApplicationContext().registerReceiver(mReceiver, mFilter);
        isRegisteredReceiver = true;

        if (mWigigManager.isWigigEnabled()) {
            mWigigManager.startScan();
        } else {
            boolean ret = mWigigManager.setWigigEnabled(true);
            if (!ret) {
                Toast.makeText(this, "Could not enable wigig!", Toast.LENGTH_SHORT).show();
                LogUtils.loge("Could not enable wigig!");
            }

            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    if (mWigigManager.isWigigEnabled()) {
                        mWigigManager.startScan();
                    }
                }
            }, SCAN_INTERVAL);
        }
    }

    public void finish() {
        if(isRegisteredReceiver){
            try {
                this.getApplicationContext().unregisterReceiver(mReceiver);
                isRegisteredReceiver = false;
            } catch (Exception e) {
                LogUtils.logi("Error" + e);
            }
        }
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {

        public void onReceive(Context c, Intent intent) {
            LogUtils.logi(intent.getAction() + " WigigState="
                    + mWigigManager.getWigigApState());

            if (WigigManager.SCAN_RESULTS_AVAILABLE_ACTION.equals(intent.getAction())) {
                final List<ScanResult> scanResults = mWigigManager.getScanResults();
                if (scanResults != null && scanResults.size() > 0) {
                    StringBuffer sb = new StringBuffer();
                    Resources resources = getApplicationContext().getResources();
                    sb.append(resources.getString(R.string.wifi_device_list));
                    sb.append("\n");

                    for (ScanResult sr : scanResults) {
                        LogUtils.logi(sr.toString());

                        sb.append("\n")
                                .append(sr.SSID).append("\n")
                                .append(resources.getString(R.string.wifi_level))
                                .append(sr.level).append(", ")
                                .append(resources.getString(R.string.wifi_freq))
                                .append(sr.frequency).append("\n");

                        mTestCase.addTestData(sr.SSID, "Level:" + sr.level + ",  Frep:" + sr.frequency);
                    }

                    updateView(mTestCase.getName(), sb.toString());
                    updateResultForCase(mTestCase.getName(), TestCase.STATE_PASS);

                } else {
                    updateResultForCase(mTestCase.getName(), TestCase.STATE_FAIL);
                }
            }
        }
    };
}
*/
