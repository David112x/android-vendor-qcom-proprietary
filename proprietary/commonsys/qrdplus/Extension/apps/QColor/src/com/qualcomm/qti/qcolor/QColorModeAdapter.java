/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.qcolor;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.RadioButton;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

public class QColorModeAdapter extends BaseAdapter {
    private final static String TAG = "QColorModeAdapter";
    private LayoutInflater mInflater;
    private int mSelectedPosition;
    private ArrayList<QColorMode> mModeList;

    public QColorModeAdapter(Context context){
        mInflater = LayoutInflater.from(context);
        mModeList = new ArrayList<>();
    }
    public void setQcolorModeData(List<QColorMode> modes) {
        mModeList.clear();
        mModeList.addAll(modes);
    }

    public void setSelectedItemPosition(int position) {
        mSelectedPosition = position;
        QColorLog.d(TAG, "Select the item: " + mSelectedPosition);
        notifyDataSetChanged();
    }

    @Override
    public int getCount() {
        return mModeList.size();
    }

    @Override
    public QColorMode getItem(int position) {
       return mModeList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View item;
        if (convertView == null) {
            item = mInflater.inflate(R.layout.qcolor_item, null);
        } else {
            item = convertView;
        }
        TextView name =  item.findViewById(R.id.qcolor_mode_name);
        name.setText(mModeList.get(position).getModeName());

        RadioButton button = item.findViewById(R.id.qcolor_radio_button);
        if(position == mSelectedPosition){
            button.setChecked(true);
        }else{
            button.setChecked(false);
        }

        return item;
    }
}
