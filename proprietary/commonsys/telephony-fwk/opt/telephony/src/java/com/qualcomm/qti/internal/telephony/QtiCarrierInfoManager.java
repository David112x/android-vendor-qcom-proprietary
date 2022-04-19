/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

package com.qualcomm.qti.internal.telephony;

import android.content.Context;
import com.android.internal.telephony.CarrierInfoManager;
import com.android.internal.telephony.Phone;
import android.telephony.ImsiEncryptionInfo;
import android.util.Log;

import com.qualcomm.qti.internal.telephony.QtiTelephonyComponentFactory;
import vendor.qti.hardware.radio.qtiradio.V1_0.QtiRadioResponseInfo;

public class QtiCarrierInfoManager extends CarrierInfoManager {

    private static final String LOG_TAG = "QtiCarrierInfoManager";
    private final Phone mPhone;

    public class QtiCarrierInfoResponse {

        public void setCarrierInfoForImsiEncryptionResponse(
                QtiRadioResponseInfo responseInfo) {
            Log.i(LOG_TAG,"CarrierInfoForImsiEncryptionResponse");
        }
    }

    public QtiCarrierInfoManager(Phone phone) {
        super();
        mPhone = phone;
        QtiTelephonyComponentFactory.getInstance().getRil(mPhone.getPhoneId())
                .setImsiEncryptionResponseCallback(new QtiCarrierInfoResponse());
    }

}
