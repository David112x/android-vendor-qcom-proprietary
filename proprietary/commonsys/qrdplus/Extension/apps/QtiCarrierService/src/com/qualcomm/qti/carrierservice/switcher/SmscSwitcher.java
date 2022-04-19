/**
 * Copyright (c) 2020, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierservice.switcher;


import android.util.Log;

import com.qualcomm.qti.carrierservice.AbstractSwitcher;
import com.qualcomm.qti.carrierservice.CarrierConfigService;

public class SmscSwitcher extends AbstractSwitcher {

    @Override
    public boolean onFirstCarrierIdChange(int oldId, int newId) {
        Log.d(CarrierConfigService.TAG, "SmscSwitcher onFirstCarrierIdChange oldId: "
                + oldId + ", newId: " + newId);
        return false;
    }

    @Override
    public boolean onFirstMccMncChange(String oldValue, int newVale) {
        Log.d(CarrierConfigService.TAG, "SmscSwitcher onFirstMccMncChange oldValue: "
                + oldValue + ", newVale: " + newVale);
        return false;
    }
}
