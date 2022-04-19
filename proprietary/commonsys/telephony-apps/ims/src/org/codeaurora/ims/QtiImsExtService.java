/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package org.codeaurora.ims;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

import com.qualcomm.ims.utils.Log;

/** Service to handle side car API's
  * Returns QtiImsExt binder to clients.
  */
public class QtiImsExtService extends Service {
    private QtiImsExt mQtiImsExt;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(this, "QtiImsExtService created!");
        // Initialize the QtiImsExt
        createQtiImsExt();
    }

    private void createQtiImsExt() {
        ImsServiceSub[] imsServiceSubs = ImsService.getServiceSubs();
        if (imsServiceSubs != null) {
            mQtiImsExt = new QtiImsExt(this, imsServiceSubs);
        } else {
            Log.e(this, "QtiImsExtService, ImsService is not yet started retry.");
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(this, "onBind " + mQtiImsExt);
        if (mQtiImsExt == null) {
            createQtiImsExt();
        }
        if (mQtiImsExt == null) {
            Log.w(this, "onBind returned null");
            return null;
        }
        return mQtiImsExt.getBinder();
    }
}
