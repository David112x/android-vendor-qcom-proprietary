/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva;

import android.app.ListActivity;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.qualcomm.qti.sva.controller.Global;
import com.qualcomm.qti.sva.data.IExtendedSmModel;
import com.qualcomm.qti.sva.data.ISettingsModel;
import com.qualcomm.qti.sva.data.SettingsModel;
import com.qualcomm.qti.sva.utils.LogUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class SelectActionActivity extends ListActivity {
    public static final String KEY_EXTRA_KEYPHRASE_NAME = "keyphraseName";
    public static final String KEY_EXTRA_SM_NAME = "smName";

    private final String TAG = SelectActionActivity.class.getSimpleName();
    private TextView mTvKeyphraseName;

    private String mKeyphraseName;
    private String mSmName;
    private ArrayList<String> mActionNameArray = new ArrayList<>();
    private ArrayList<Intent> mActionIntentArray = new ArrayList<>();
    private ArrayList<Drawable> mAppIconArray = new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        mKeyphraseName = getIntent().getStringExtra(KEY_EXTRA_KEYPHRASE_NAME);
        mSmName = getIntent().getStringExtra(KEY_EXTRA_SM_NAME);
        if (null == mKeyphraseName || null == mSmName) {
            LogUtils.d(TAG, "onCreate: invalid intent param");
            return;
        }
        LogUtils.d(TAG, "onCreate: mKeyphraseName = " + mKeyphraseName
                + " mSmName = " + mSmName);
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_select_action);
        initializeUI();
    }

    private void initializeUI() {
        mTvKeyphraseName = findViewById(R.id.tv_keyphrase_name);
        mTvKeyphraseName.setText(mKeyphraseName);

        mActionNameArray.clear();
        mAppIconArray.clear();
        mActionIntentArray.clear();

        mActionNameArray.add(getString(R.string.none));
        mActionIntentArray.add(null);
        mAppIconArray.add(new ColorDrawable(Color.TRANSPARENT));

        PackageManager packageManager = getPackageManager();
        Intent intent = new Intent(Intent.ACTION_MAIN, null);
        intent.addCategory(Intent.CATEGORY_LAUNCHER);
        List<ResolveInfo> resolveInfoList = packageManager.queryIntentActivities(intent, 0);
        Collections.sort(resolveInfoList, new ResolveInfo.DisplayNameComparator(packageManager));

        List<ApplicationInfo> applicationInfoList = new ArrayList<>();
        for (ResolveInfo item : resolveInfoList) {
            try {
                applicationInfoList.add(packageManager.getApplicationInfo(
                        item.activityInfo.packageName, 0));
            } catch (PackageManager.NameNotFoundException e) {
                e.printStackTrace();
            }
        }

        for (ApplicationInfo item : applicationInfoList) {
            mActionNameArray.add(packageManager.getApplicationLabel(item).toString());
            mActionIntentArray.add(packageManager.getLaunchIntentForPackage(item.packageName));
            mAppIconArray.add(packageManager.getApplicationIcon(item));
        }

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                R.layout.listitem_select_action, R.id.tv_app_name, mActionNameArray.toArray(
                new String[mActionNameArray.size()])) {
            @Override
            public View getView(int position, View convertView, ViewGroup parent) {
                View view = super.getView(position, convertView, parent);
                ImageView appIcon = view.findViewById(R.id.iv_app_icon);
                appIcon.setImageDrawable(mAppIconArray.get(position));

                TextView appName = view.findViewById(R.id.tv_app_name);
                appName.setText(mActionNameArray.get(position));

                return view;
            }
        };

        setListAdapter(adapter);

        ListView listView = getListView();
        listView.setTextFilterEnabled(true);
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int position, long id) {
                String actionName = mActionNameArray.get(position);
                Intent actionIntent = mActionIntentArray.get(position);

                IExtendedSmModel model = Global.getInstance().getExtendedSmMgr().getSoundModel(
                        mSmName);
                model.setSoundModelActionName(actionName);
                model.setSoundModelActionIntent(actionIntent);

                // save to preferences
                ISettingsModel settingsModel = new SettingsModel(getApplicationContext(), mSmName);
                settingsModel.setActionName(actionName);
                settingsModel.setActionIntent(actionIntent);

                finish();
            }
        });
    }
}
