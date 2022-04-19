/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.simcontacts;

import android.content.Context;
import android.util.Log;

import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import com.android.internal.telephony.IIccPhoneBook;

import java.util.Arrays;

/**
 * Shared static SIM contact methods.
 */
public class ContactUtils {

    private static final String TAG = "ContactUtils";
    private static final int ADN_COUNT_POS = 0;
    private static final int ADN_USED_POS = 1;
    private static final int EMAIL_COUNT_POS = 2;
    private static final int EMAIL_USED_POS = 3;
    private static final int ANR_COUNT_POS = 4;
    private static final int ANR_USED_POS = 5;
    public static final int NAME_LENGTH_POS = 6;
    public static final int NUMBER_LENGTH_POS = 7;
    public static final int EMAIL_LENGTH_POS = 8;
    public static final int ANR_LENGTH_POS = 9;


    public static int[] getAdnRecordsCapacity(int subId) {
        return getAdnRecordsCapacityForSubscriber(subId);
    }

    /**
     * Returns the subscription's card can save anr or not.
     */
    public static boolean canSaveAnr(int subId) {
        int adnCapacity[] = getAdnRecordsCapacity(subId);
        return adnCapacity[ANR_COUNT_POS] > 0;
    }

    /**
     * Returns the subscription's card can save email or not.
     */
    public static boolean canSaveEmail(int subId) {
        int adnCapacity[] = getAdnRecordsCapacity(subId);
        return adnCapacity[EMAIL_COUNT_POS] > 0;
    }

    public static int getOneSimAnrCount(int subId) {
        int count = 0;
        int adnCapacity[] = getAdnRecordsCapacity(subId);
        int anrCount = adnCapacity[ANR_COUNT_POS];
        int adnCount = adnCapacity[ADN_COUNT_POS];
        if (adnCount > 0) {
            count = anrCount % adnCount != 0 ? (anrCount / adnCount + 1)
                    : (anrCount / adnCount);
        }
        return count;
    }

    public static int getOneSimEmailCount(int subId) {
        int count = 0;
        int adnCapacity[] = getAdnRecordsCapacity(subId);
        int emailCount = adnCapacity[EMAIL_COUNT_POS];
        int adnCount = adnCapacity[ADN_COUNT_POS];
        if (adnCount > 0) {
            count = emailCount % adnCount != 0 ? (emailCount / adnCount + 1)
                    : (emailCount / adnCount);
        }
        return count;
    }

    public static int getSimFreeCount(int subId) {
        int adnCapacity[] = getAdnRecordsCapacity(subId);
        int count = adnCapacity[ADN_COUNT_POS]-adnCapacity[ADN_USED_POS];
        if (Log.isLoggable(TAG, Log.DEBUG)) {
            Log.d(TAG, "spare adn:" + count);
        }
        return count;
    }

    public static int getSpareAnrCount(int subId) {
        int adnCapacity[] = getAdnRecordsCapacity(subId);
        int spareCount = adnCapacity[ANR_COUNT_POS]-adnCapacity[ANR_USED_POS];
        if (Log.isLoggable(TAG, Log.DEBUG)) {
            Log.d(TAG, "spare anr:" + spareCount);
        }
        return spareCount;
    }

    public static int getSpareEmailCount(int subId) {
        int adnCapacity[] = getAdnRecordsCapacity(subId);
        int spareCount = adnCapacity[EMAIL_COUNT_POS]-adnCapacity[EMAIL_USED_POS];
        if (Log.isLoggable(TAG, Log.DEBUG)) {
            Log.d(TAG, "spare email:" + spareCount);
        }
        return spareCount;
    }

    /*      capacity[0]  is the max count of ADN
            capacity[1]  is the used count of ADN
            capacity[2]  is the max count of EMAIL
            capacity[3]  is the used count of EMAIL
            capacity[4]  is the max count of ANR
            capacity[5]  is the used count of ANR
            capacity[6]  is the max length of name
            capacity[7]  is the max length of number
            capacity[8]  is the max length of email
            capacity[9]  is the max length of anr
    */
    private static int[] getAdnRecordsCapacityForSubscriber(int subId) {
        int defaultCapacity[] = { 500, 0, 200, 0, 500, 0, 14, 42, 40, 15};
        try {
            IIccPhoneBook iccIpb = IIccPhoneBook.Stub
                    .asInterface(ServiceManager.getService("simphonebook"));
            if (iccIpb != null) {
                int[] capacity = iccIpb.getAdnRecordsCapacityForSubscriber(subId);
                if (capacity != null)
                    return capacity;
            }
        }catch (RemoteException e){
            Log.e(TAG," getAdnRecordsCapacityForSubscriber error = "+e.toString());
        }
        return defaultCapacity;
    }
}
