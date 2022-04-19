/**
 * Copyright (c) 2020, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierservice;

public interface ICarrierSwitcher{
    /**
     * For first SIM insert
     */
    boolean onFirstCarrierIdChange(int oldId, int newId);
    /**
     * For each SIM insert
     */
    boolean onEveryCarrierIdChange(int oldId, int newId);
    /**
     * For first SIM insert
     */
    boolean onFirstMccMncChange(String oldValue, int newVale);
    /**
     * For each SIM insert
     */
    boolean onEveryMccMncChange(int oldValue, int newValue);
}
