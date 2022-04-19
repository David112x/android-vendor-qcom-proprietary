/**
 * Copyright (c) 2020, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierservice;


import android.util.Log;

public class AbstractSwitcher implements ICarrierSwitcher {

    @Override
    public boolean onFirstCarrierIdChange(int oldId, int newId) {
        Log.d(CarrierConfigService.TAG, "onFirstCarrierIdChange oldId: "
                + oldId + ", newId: " + newId);
        return false;
    }

    @Override
    public boolean onEveryCarrierIdChange(int oldId, int newId) {
        Log.d(CarrierConfigService.TAG, "onEveryCarrierIdChange oldId: "
                + oldId + ", newId: " + newId);
        return false;
    }

    @Override
    public boolean onFirstMccMncChange(String oldValue, int newVale) {
        Log.d(CarrierConfigService.TAG, "onFirstMccMncChange oldValue: "
                + oldValue + ", newVale: " + newVale);
        return false;
    }

    @Override
    public boolean onEveryMccMncChange(int oldValue, int newValue) {
        Log.d(CarrierConfigService.TAG, "onEveryMccMncChange oldValue: "
                + oldValue + ", newValue: " + newValue);
        return false;
    }
}
