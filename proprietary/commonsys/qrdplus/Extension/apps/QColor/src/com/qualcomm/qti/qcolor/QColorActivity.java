/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.qcolor;

import android.app.Activity;
import android.hardware.display.ColorDisplayManager;
import android.os.Bundle;
import android.os.RemoteException;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;

import java.util.ArrayList;
import java.util.List;
import java.util.NoSuchElementException;

import vendor.display.color.V1_4.IDisplayColor;
import vendor.display.color.V1_0.Result;

public class QColorActivity extends Activity implements AdapterView.OnItemClickListener {
    private final static String TAG = "QColorActivity";

    private QColorModeAdapter mColorModeAdapter;
    private ColorDisplayManager mColorDisplayManager;
    private final ArrayList<QColorMode> mColorModes = new ArrayList<>();
    private int[] mAvailableColorModes;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.qcolor_activity);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        mColorDisplayManager = getSystemService(ColorDisplayManager.class);
        mAvailableColorModes = getResources().getIntArray(
                com.android.internal.R.array.config_availableColorModes);
        initQcolorModeListView();
    }

    private void initQcolorModeListView(){
        ListView list = findViewById(R.id.color_mode_list);
        list.setOnItemClickListener(this);

        mColorModeAdapter = new QColorModeAdapter(this);
        mColorModeAdapter.setQcolorModeData(getQColorModes());
        mColorModeAdapter.setSelectedItemPosition(getCurrentColorModeIndex());

        list.setAdapter(mColorModeAdapter);
    }

    private List<QColorMode> getQColorModes(){
        mColorModes.clear();
        final ArrayList<String> names = new ArrayList<>();
        final ArrayList<Integer> intents = new ArrayList<>();
        try {
            IDisplayColor mDisplayColor = IDisplayColor.getService(true);
            if (mDisplayColor == null) {
                QColorLog.e(TAG, "mDisplayColor is null");
                return null;
            }
            mDisplayColor.getRenderIntentsMap(0, (int result, ArrayList<String> tempNames,
                                                  ArrayList<Integer> tempIntents) -> {
                QColorLog.w(TAG, "getRenderIntentsMap callback");
                if (result == Result.OK){
                    names.addAll(tempNames);
                    intents.addAll(tempIntents);
                }
            });
        } catch(RemoteException e) {
            QColorLog.e(TAG, "RemoteException:" + e);
        } catch (NoSuchElementException e){
            QColorLog.e(TAG, "NoSuchElementException:" + e);
        }
        if (names.size() != intents.size()){
            throw new IllegalArgumentException("names is not matched with intents");
        }

        for (int i = 0; i < names.size(); i++){
            QColorMode mode = new QColorMode(names.get(i), intents.get(i));
            if (mode.isColorModeAvailable(mAvailableColorModes)){
                mColorModes.add(mode);
            }
        }
        return mColorModes;
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        mColorModeAdapter.setSelectedItemPosition(position);
        setColorMode(mColorModeAdapter.getItem(position).getColorMode());
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                break;

            default:
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    private void setColorMode(int colorMode) {
        mColorDisplayManager.setColorMode(colorMode);
    }

    private int getCurrentColorModeIndex() {
        final int colorMode = mColorDisplayManager.getColorMode();
        for (int i = 0; i < mColorModes.size(); i++){
            if (colorMode == mColorModes.get(i).getColorMode()){
                return i;
            }
        }
        return 0;
    }

}
