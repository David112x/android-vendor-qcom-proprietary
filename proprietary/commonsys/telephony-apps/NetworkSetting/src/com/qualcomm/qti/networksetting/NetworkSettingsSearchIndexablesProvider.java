/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.networksetting;

import static android.provider.SearchIndexablesContract.COLUMN_INDEX_NON_INDEXABLE_KEYS_KEY_VALUE;
import static android.provider.SearchIndexablesContract.INDEXABLES_RAW_COLUMNS;
import static android.provider.SearchIndexablesContract.INDEXABLES_XML_RES_COLUMNS;
import static android.provider.SearchIndexablesContract.NON_INDEXABLES_KEYS_COLUMNS;
import static android.provider.SearchIndexablesContract.XmlResource;

import android.content.Context;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.os.UserManager;
import android.provider.SearchIndexableResource;
import android.provider.SearchIndexablesProvider;
import android.util.Log;

import java.util.ArrayList;

public class NetworkSettingsSearchIndexablesProvider extends SearchIndexablesProvider {
    private static final String TAG = "NetworkSettingsSearchIndexablesProvider";
    private UserManager mUserManager;

    private static SearchIndexableResource[] INDEXABLE_RES = new SearchIndexableResource[] {
            new SearchIndexableResource(0, R.xml.network_setting_fragment,
                    MobileNetworkSettings.class.getName(),
                    0),
    };

    @Override
    public boolean onCreate() {
        mUserManager = (UserManager) getContext().getSystemService(Context.USER_SERVICE);
        return true;
    }

    @Override
    public Cursor queryXmlResources(String[] projection) {
        MatrixCursor cursor = new MatrixCursor(INDEXABLES_XML_RES_COLUMNS);
        for (int i = 0, length = INDEXABLE_RES.length; i < length; i++) {
            cursor.newRow()
                    .add(XmlResource.COLUMN_RANK, INDEXABLE_RES[i].rank)
                    .add(XmlResource.COLUMN_XML_RESID, INDEXABLE_RES[i].xmlResId)
                    .add(XmlResource.COLUMN_CLASS_NAME, null)
                    .add(XmlResource.COLUMN_ICON_RESID, INDEXABLE_RES[i].iconResId)
                    .add(XmlResource.COLUMN_INTENT_ACTION, "android.intent.action.MAIN")
                    .add(XmlResource.COLUMN_INTENT_TARGET_PACKAGE,
                            "com.qualcomm.qti.networksetting")
                    .add(XmlResource.COLUMN_INTENT_TARGET_CLASS, INDEXABLE_RES[i].className);
        }
        return cursor;
    }

    @Override
    public Cursor queryRawData(String[] projection) {
        MatrixCursor cursor = new MatrixCursor(INDEXABLES_RAW_COLUMNS);
        return cursor;
    }

    @Override
    public Cursor queryNonIndexableKeys(String[] projection) {
        MatrixCursor cursor = new MatrixCursor(NON_INDEXABLES_KEYS_COLUMNS);

        ArrayList<String> nonIndexableRows = new ArrayList<>();

        if (!mUserManager.isAdminUser()) {
            Log.d(TAG, "Not an admin user");
            nonIndexableRows.add("preferred_network_mode_key");
            nonIndexableRows.add("button_roaming_key");
            nonIndexableRows.add("cdma_lte_data_service_key");
            nonIndexableRows.add("enhanced_4g_lte");
            nonIndexableRows.add("mobile_data_enable");
            nonIndexableRows.add("data_usage_summary");
            nonIndexableRows.add("button_uplmn_key");
            nonIndexableRows.add("carrier_settings_euicc_key");
            nonIndexableRows.add("enable_4g_settings");
            nonIndexableRows.add("network_operators_category_key");
            nonIndexableRows.add("button_auto_select_key");
            nonIndexableRows.add("button_network_select_key");
            nonIndexableRows.add("button_choose_network_key");
            nonIndexableRows.add("category_gsm_apn_key");
            nonIndexableRows.add("button_gsm_apn_key");
            nonIndexableRows.add("carrier_settings_key");
            nonIndexableRows.add("cdma_system_select_key");
            nonIndexableRows.add("cdma_subscription_key");
            nonIndexableRows.add("category_cdma_apn_key");
            nonIndexableRows.add("button_cdma_apn_key");
            nonIndexableRows.add("carrier_settings_key");
        }

        nonIndexableRows.add("carrier_settings_euicc_key");
        nonIndexableRows.add("enhanced_4g_lte");
        nonIndexableRows.add("enabled_networks_key");
        nonIndexableRows.add("preferred_network_mode_cmcc_key");

        for (String key : nonIndexableRows) {
            final Object[] ref = new Object[NON_INDEXABLES_KEYS_COLUMNS.length];
            ref[COLUMN_INDEX_NON_INDEXABLE_KEYS_KEY_VALUE] = key;
            cursor.addRow(ref);
        }
        return cursor;
    }
}
