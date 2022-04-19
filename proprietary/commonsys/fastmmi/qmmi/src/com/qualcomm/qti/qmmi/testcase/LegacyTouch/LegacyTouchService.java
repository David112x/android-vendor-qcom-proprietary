/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.qmmi.testcase.LegacyTouch;


import android.os.Binder;

import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;

import com.qualcomm.qti.qmmi.bean.TestCase;
import com.qualcomm.qti.qmmi.framework.BaseService;
import com.qualcomm.qti.qmmi.utils.LogUtils;
import com.qualcomm.qti.qmmi.utils.Utils;
import com.qualcomm.qti.qmmi.R;
import android.view.InputDevice;

public class LegacyTouchService extends BaseService {
    private final IBinder binder = new MyBinder();
    private TestCase mTestCase = null;
    private String CASE_NAME = null;
    private String testCaseOutcome;
    public InputDevice inputDevice;

    public class MyBinder extends Binder{
        LegacyTouchService getService() {
            return LegacyTouchService.this;
        }
    };

    public int onStartCommand(Intent intent, int flags, int startId) {
        LogUtils.logi("onStartCommand");
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO Auto-generated method stub
        return binder;
    }

    @Override
    public void register() { }

    public int stop(TestCase testCase) {
        return 0;
    }

    @Override
    public int run(TestCase testCase) {
        mTestCase = testCase;
        boolean test_result = false;
        int size = 0;

        LogUtils.logi("Enter");
        int[] ids = InputDevice.getDeviceIds();
        size = ids.length;
        LogUtils.logi("size is " + size);
        for(int j=0;j<size;j++){
            inputDevice = InputDevice.getDevice(ids[j]);
            if((inputDevice.getSources() & InputDevice.SOURCE_TOUCHSCREEN) == InputDevice.SOURCE_TOUCHSCREEN)
            {
                final int numAxes = inputDevice.getMotionRanges().size();
                test_result = true;
                LogUtils.logi("Found, the No. "+ ids[j]+ " Touchscreen "+inputDevice.getName() + " has "+ numAxes + " Axes.");
                for (int i = 0; i < numAxes; i++) {
                    if(inputDevice.getMotionRanges().get(i).getSource() !=InputDevice.SOURCE_TOUCHSCREEN){
                        LogUtils.loge("The No. " + i + " is the other NOT touchscreen source.");
                        test_result = false;
                        break;
                    }
                }
                if(!test_result)
                    break;
            }
        }
        if(test_result){
            LogUtils.logi("pass");
            testCaseOutcome = getApplicationContext().getResources().getString(R.string.legacy_touch_pass);
            updateResultForCase(testCase.getName(), TestCase.STATE_PASS);
        }else{
            LogUtils.logi("Fail");
            testCaseOutcome = getApplicationContext().getResources().getString(R.string.legacy_touch_fail);
            updateResultForCase(testCase.getName(), TestCase.STATE_FAIL);
        }
        updateView(testCase.getName(), testCaseOutcome);
        return 0;
    }
}
