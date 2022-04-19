/*
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.internal.telephony;

import android.content.Context;
import android.util.Log;
import android.os.Looper;
import android.os.Handler;
import android.os.IDeviceIdleController;
import android.os.PowerManager;
import android.os.ServiceManager;
import android.provider.Settings;
import android.telephony.Rlog;
import android.telephony.AccessNetworkConstants.TransportType;

import com.android.ims.ImsManager;
import com.android.internal.telephony.cdma.CdmaSubscriptionSourceManager;
import com.android.internal.telephony.cdma.EriManager;
import com.android.internal.telephony.dataconnection.DcTracker;
import com.android.internal.telephony.imsphone.ImsPhone;
import com.android.internal.telephony.imsphone.ImsPhoneCallTracker;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneNotifier;
import com.android.internal.telephony.PhoneSwitcher;
import com.android.internal.telephony.RIL;
import com.android.internal.telephony.SubscriptionController;

import com.android.internal.telephony.TelephonyComponentFactory;
import com.android.internal.telephony.*;
import com.qualcomm.qti.internal.nrNetworkService.MainServiceImpl;
import com.qualcomm.qti.internal.telephony.dataconnection.QtiDcTracker;

import com.qualcomm.qti.internal.telephony.primarycard.QtiPrimaryCardController;
import com.qualcomm.qti.internal.telephony.QtiCarrierInfoManager;

import java.lang.RuntimeException;

public class QtiTelephonyComponentFactory extends TelephonyComponentFactory {
    private static String LOG_TAG = "QtiTelephonyComponentFactory";

    private static QtiTelephonyComponentFactory sInstance;
    private QtiRIL mRil[] = new QtiRIL[]{null, null};

    public QtiTelephonyComponentFactory() {
        sInstance = this;
    }

    public static QtiTelephonyComponentFactory getInstance() {
        return sInstance;
    }


    @Override
    public GsmCdmaCallTracker makeGsmCdmaCallTracker(GsmCdmaPhone phone) {
        Rlog.d(LOG_TAG, "makeGsmCdmaCallTracker");
        return super.makeGsmCdmaCallTracker(phone);
    }

    @Override
    public SmsStorageMonitor makeSmsStorageMonitor(Phone phone) {
        Rlog.d(LOG_TAG, "makeSmsStorageMonitor");
        return super.makeSmsStorageMonitor(phone);
    }

    @Override
    public SmsUsageMonitor makeSmsUsageMonitor(Context context) {
        Rlog.d(LOG_TAG, "makeSmsUsageMonitor");
        /* TODO: P Upgrade */
        // return new QtiSmsUsageMonitor(context, getSmsSecurityService(context));
        return super.makeSmsUsageMonitor(context);
    }

    @Override
    public ServiceStateTracker makeServiceStateTracker(GsmCdmaPhone phone, CommandsInterface ci) {
        Rlog.d(LOG_TAG, "makeQtiServiceStateTracker");
        return new QtiServiceStateTracker(phone, ci);
    }

    @Override
    public DcTracker makeDcTracker(Phone phone, int transportType) {
        Rlog.d(LOG_TAG, "makeQtiDcTracker");
        return new QtiDcTracker(phone, transportType);
    }

    @Override
    public IccPhoneBookInterfaceManager makeIccPhoneBookInterfaceManager(Phone phone) {
        Rlog.d(LOG_TAG, "makeQtiIccPhoneBookInterfaceManager");
        return new QtiIccPhoneBookInterfaceManager(phone);
    }

    @Override
    public IccSmsInterfaceManager makeIccSmsInterfaceManager(Phone phone) {
        Rlog.d(LOG_TAG, "makeIccSmsInterfaceManager");
        return super.makeIccSmsInterfaceManager(phone);
    }

    @Override
    public EriManager makeEriManager(Phone phone, int eriFileSource) {
        Rlog.d(LOG_TAG, "makeEriManager");
        return super.makeEriManager(phone, eriFileSource);
    }

    @Override
    public WspTypeDecoder makeWspTypeDecoder(byte[] pdu) {
        Rlog.d(LOG_TAG, "makeWspTypeDecoder");
        return super.makeWspTypeDecoder(pdu);
    }

    @Override
    public InboundSmsTracker makeInboundSmsTracker(byte[] pdu, long timestamp, int destPort,
            boolean is3gpp2, boolean is3gpp2WapPdu, String address, String displayAddr,
            String msgBody, boolean isClass0, int subId) {
        Rlog.d(LOG_TAG, "makeInboundSmsTracker");
        return super.makeInboundSmsTracker(pdu, timestamp, destPort, is3gpp2,
                is3gpp2WapPdu, address, displayAddr, msgBody, isClass0, subId);
    }

    @Override
    public InboundSmsTracker makeInboundSmsTracker(byte[] pdu, long timestamp, int destPort,
            boolean is3gpp2, String address, String displayAddr, int referenceNumber,
            int sequenceNumber, int messageCount, boolean is3gpp2WapPdu, String msgBody,
            boolean isClass0, int subId) {
        Rlog.d(LOG_TAG, "makeInboundSmsTracker");
        return super.makeInboundSmsTracker(pdu, timestamp, destPort, is3gpp2,
                address, displayAddr, referenceNumber, sequenceNumber, messageCount,
                is3gpp2WapPdu, msgBody, isClass0, subId);
    }

    @Override
    public ImsPhoneCallTracker makeImsPhoneCallTracker(ImsPhone imsPhone) {
        Rlog.d(LOG_TAG, "makeImsPhoneCallTracker");
        return super.makeImsPhoneCallTracker(imsPhone);
    }

    @Override
    public CdmaSubscriptionSourceManager
    getCdmaSubscriptionSourceManagerInstance(Context context, CommandsInterface ci, Handler h,
                                             int what, Object obj) {
        Rlog.d(LOG_TAG, "getCdmaSubscriptionSourceManagerInstance");
        return super.getCdmaSubscriptionSourceManagerInstance(context, ci, h, what, obj);
    }

    @Override
    public IDeviceIdleController getIDeviceIdleController() {
        Rlog.d(LOG_TAG, "getIDeviceIdleController");
        return super.getIDeviceIdleController();
    }

    @Override
    public Phone makePhone(Context context, CommandsInterface ci, PhoneNotifier notifier,
            int phoneId, int precisePhoneType,
            TelephonyComponentFactory telephonyComponentFactory) {
        Rlog.d(LOG_TAG, "makePhone");
        return new QtiGsmCdmaPhone(context, ci, notifier, phoneId, precisePhoneType,
                telephonyComponentFactory);
    }

    @Override
    public SubscriptionController initSubscriptionController(Context c, CommandsInterface[] ci) {
        Rlog.d(LOG_TAG, "initSubscriptionController");
        return QtiSubscriptionController.init(c, ci);
    }

    @Override
    public SubscriptionInfoUpdater makeSubscriptionInfoUpdater(Looper looper, Context context,
            Phone[] phones, CommandsInterface[] ci) {
        Rlog.d(LOG_TAG, "makeSubscriptionInfoUpdater");
        return QtiSubscriptionInfoUpdater.init(looper, context, phones, ci);
    }

    @Override
    public void makeExtTelephonyClasses(Context context,
            Phone[] phones, CommandsInterface[] commandsInterfaces) {
        Rlog.d(LOG_TAG, " makeExtTelephonyClasses ");

        // Settings application uses the "settings_network_and_internet_v2" config value
        // to control the "SIM Cards" menu and "Networks & Internel" application menu.

        // Currently this "settings_network_and_internet_v2" config value is set to TRUE @
        // FeatureFlagUtils due to that "SIM Cards" menu not displayed, modifying the config
        // value below to false to display the "SIM Cards" menu in settings app.
        String value = Settings.Global.getString(context.getContentResolver(),
                "settings_network_and_internet_v2");
        if ((value == null) || !value.equals("false")) {
            Settings.Global.putString(context.getContentResolver(),
                    "settings_network_and_internet_v2", "false");
        }

        QtiPhoneUtils.init(context);
        QtiUiccCardProvisioner.make(context, commandsInterfaces);
        QtiDepersoSupplier.make(context);
        QtiRadioCapabilityController.make(context, phones, commandsInterfaces);
        QtiPrimaryCardController.init(context, phones, commandsInterfaces);
        QtiCallStateNotifier.init(phones);
        try {
            /* Init 5G network service */
            MainServiceImpl.init(context);
            ExtTelephonyServiceImpl.init(context);
        } catch (NoClassDefFoundError e) {
            e.printStackTrace();
            Rlog.e(LOG_TAG, "Error creating ExtTelephonyServiceImpl");
        }
    }

    @Override
    public PhoneSwitcher makePhoneSwitcher(int maxActivePhones, int numPhones, Context context,
            SubscriptionController sc, Looper looper, ITelephonyRegistry tr,
            CommandsInterface[] cis, Phone[] phones) {
        Rlog.d(LOG_TAG, "makeQtiPhoneSwitcher");
        return QtiPhoneSwitcher.make(maxActivePhones, numPhones, context, sc,
                looper, tr, cis, phones);
    }

    @Override
    public MultiSimSettingController initMultiSimSettingController(Context c,
            SubscriptionController sc) {
        Rlog.i(LOG_TAG, "initMultiSimSettingController");
        return QtiMultiSimSettingController.init(c, sc);
    }

    @Override
    public RIL makeRIL(Context context, int preferredNetworkType,
            int cdmaSubscription, Integer instanceId) {
        Rlog.d(LOG_TAG, "makeQtiRIL");
        if (instanceId < mRil.length) {
            mRil[instanceId] = new QtiRIL(context, preferredNetworkType, cdmaSubscription, instanceId);
        } else {
            throw new RuntimeException("RilInstance = " + instanceId + " not allowed!");
        }

        return mRil[instanceId];
    }

    @Override
    public CarrierInfoManager makeCarrierInfoManager(Phone phone) {
        Rlog.d(LOG_TAG, "makeCarrierInfoManager");
        return new QtiCarrierInfoManager(phone);
    }

    public QtiRIL getRil(int slotId) {
        if (slotId < mRil.length) {
            return mRil[slotId];
        } else {
            return null;
        }
    }
}
