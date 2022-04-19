/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.izat.izatconfig;

import java.io.StringReader;
import java.util.Properties;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import android.util.Log;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.RemoteException;
import java.io.IOException;
import vendor.qti.gnss.V3_0.ILocHidlGnss;
import vendor.qti.gnss.V3_0.ILocHidlIzatConfig;
import vendor.qti.gnss.V3_0.ILocHidlIzatConfigCallback;

public class IzatConfigService {

    private static final String TAG = "IzatConfig";
    private static final boolean DEBUG = Log.isLoggable (TAG, Log.DEBUG);

    private ILocHidlIzatConfig mIzatConfigIface;
    private IzatConfigCb mIzatConfigCb;
    private Context mContext;

    private int mNlpMode;
    private String mOsNpPackageName;
    private String mRegionalNpPackageName;

    private static IzatConfigService sInstance;

    private static final String PREFERENCES_FILE = "izatconfig";
    private static final String NLP_MODE_KEY = "nlp_mode";
    private static final String OSNLP_PACKAGE_KEY = "osnlp_package";
    private static final String REGIONAL_NLP_PACKAGE_KEY = "regional_nlp_package";
    private static final String IZAT_CONFIG_AVAIL_KEY = "izat_config_avail";

    private static final long WAIT_CALLBACK_TIME_MS = 100;

    public static int getNlpMode(Context ctx) {
        return getInstance(ctx).getNlpMode();
    }

    public static String getOsnpPackagename(Context ctx) {
        return getInstance(ctx).getOsNpPackageName();
    }

    public static String getRegionalNlpPackagename(Context ctx) {
        return getInstance(ctx).getRegionalNpPackageName();
    }

    class IzatConfigCb extends ILocHidlIzatConfigCallback.Stub {

        private CountDownLatch mCountDownLatch;

        public IzatConfigCb() {
            mCountDownLatch = new CountDownLatch(1);
        }

        @Override
        public void izatConfigCallback(String izatConfigContent) {
            try {
                StringReader reader = new StringReader(izatConfigContent);
                Properties prop = new Properties();
                prop.load(reader);
                mNlpMode = Integer.parseInt(prop.getProperty("NLP_MODE"));
                mOsNpPackageName = prop.getProperty("OSNLP_PACKAGE");
                mRegionalNpPackageName = prop.getProperty("REGION_OSNLP_PACKAGE");

                SharedPreferences.Editor editor =mContext.getSharedPreferences(
                        PREFERENCES_FILE, Context.MODE_PRIVATE).edit();
                editor.putInt(NLP_MODE_KEY, mNlpMode);
                editor.putString(OSNLP_PACKAGE_KEY, mOsNpPackageName);
                editor.putString(REGIONAL_NLP_PACKAGE_KEY, mRegionalNpPackageName);
                editor.putBoolean(IZAT_CONFIG_AVAIL_KEY, true);
                editor.commit();

                reader.close();
            } catch (IOException ioe) {
            }
            mCountDownLatch.countDown();
        }

        public void await() {
            try {
                mCountDownLatch.await(WAIT_CALLBACK_TIME_MS, TimeUnit.MILLISECONDS);
            } catch (InterruptedException e){
            }
        }

    }

    private ILocHidlIzatConfig getIzatConfigIface() {
        if (null == mIzatConfigIface) {
            ILocHidlGnss service = null;
            try {
                service = ILocHidlGnss.getService("gnss_vendor", false);
            } catch (RemoteException e) {
            }

            if (null != service) {
                try {
                    mIzatConfigIface = service.getExtensionLocHidlIzatConfig();
                } catch (RemoteException e) {
                }
            }
        }
        return mIzatConfigIface;
    }

    private IzatConfigService(Context ctx) {
        mContext = ctx.createDeviceProtectedStorageContext();
        SharedPreferences izatSharedPref =
                mContext.getSharedPreferences(PREFERENCES_FILE, Context.MODE_PRIVATE);
        if (izatSharedPref.getBoolean(IZAT_CONFIG_AVAIL_KEY, false)) {
            Log.d(TAG, "get izat config from shared preference");
            mNlpMode = izatSharedPref.getInt(NLP_MODE_KEY, -1);
            mOsNpPackageName = izatSharedPref.getString(OSNLP_PACKAGE_KEY,"");
            mRegionalNpPackageName = izatSharedPref.getString(REGIONAL_NLP_PACKAGE_KEY, "");
        } else {
            Log.d(TAG, "get izat config via HIDL API");
            mIzatConfigIface = getIzatConfigIface();
            mIzatConfigCb = new IzatConfigCb();
            try {
                mIzatConfigIface.init(mIzatConfigCb);
                mIzatConfigIface.readConfig();
            } catch (RemoteException e) {
            }
        }
    }

    private static IzatConfigService getInstance(Context ctx) {
        if (null == sInstance) {
            sInstance = new IzatConfigService(ctx);
        }
        return sInstance;
    }

    private void await() {
        if (mIzatConfigCb != null) {
            mIzatConfigCb.await();
        }
    }

    private int getNlpMode() {
        await();
        return mNlpMode;
    }

    private String getOsNpPackageName() {
        await();
        return mOsNpPackageName;
    }

    private String getRegionalNpPackageName() {
        await();
        return mRegionalNpPackageName;
    }
}

