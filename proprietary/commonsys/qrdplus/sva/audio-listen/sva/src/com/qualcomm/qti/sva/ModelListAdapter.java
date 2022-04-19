/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.qti.sva.data.IExtendedSmModel;
import com.qualcomm.qti.sva.utils.LogUtils;

import java.util.ArrayList;

public class ModelListAdapter extends BaseAdapter {
    private final String TAG = ModelListAdapter.class.getSimpleName();

    private Context mContext;
    private ArrayList<IExtendedSmModel> mListData;
    private OnListenerCallback mCallback;
    private String mCheckedSmFullName;

    public ModelListAdapter(Context context, ArrayList<IExtendedSmModel> listData,
                            OnListenerCallback callback) {
        mContext = context;
        mListData = listData;
        mCallback = callback;
    }

    @Override
    public int getCount() {
        return mListData.size();
    }

    @Override
    public Object getItem(int i) {
        return mListData.get(i);
    }

    @Override
    public long getItemId(int i) {
        return i;
    }

    @Override
    public View getView(int i, View view, ViewGroup viewGroup) {
        ModelListAdapter.RowViewHolder rowViewHolder;
        if (null == view) {
            LayoutInflater inflater
                    = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            if (null == inflater) {
                LogUtils.d(TAG, "getView: inflater is null");
                return null;
            }
            view = inflater.inflate(R.layout.listitem_sound_model, null);
            rowViewHolder = new ModelListAdapter.RowViewHolder();
            view.setTag(rowViewHolder);
            View layoutLeft = view.findViewById(R.id.layout_left);
            rowViewHolder.mCheckBox = view.findViewById(R.id.cb_sm_check);
            rowViewHolder.mTvSmName = view.findViewById(R.id.tv_sm_name);
            layoutLeft.setTag(rowViewHolder);
            layoutLeft.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    LogUtils.d(TAG, "left side is clicked");
                    ModelListAdapter.RowViewHolder holder
                            = (ModelListAdapter.RowViewHolder) view.getTag();

                    // bChecked present the checked status after click
                    boolean bChecked = !holder.mCheckBox.isChecked();
                    if (bChecked) {
                        String clickedSmName = holder.mItemData.getSoundModelFullFileName();
                        if (null != mCheckedSmFullName
                                && !mCheckedSmFullName.equalsIgnoreCase(clickedSmName)) {
                            LogUtils.d(TAG, "already has one checked item");
                            Toast.makeText(mContext,
                                    mContext.getString(R.string.cannot_checked_one_more),
                                    Toast.LENGTH_LONG).show();
                            return;
                        } else {
                            holder.mCheckBox.setChecked(bChecked);
                            mCheckedSmFullName = clickedSmName;
                        }
                    } else {
                        mCheckedSmFullName = null;
                        holder.mCheckBox.setChecked(bChecked);
                    }

                    mCallback.onLeftClicked(holder);
                }
            });

            View layoutRight = view.findViewById(R.id.layout_right);
            rowViewHolder.mIvSettings = view.findViewById(R.id.iv_settings);
            layoutRight.setTag(rowViewHolder);
            layoutRight.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    LogUtils.d(TAG, "right side is clicked");
                    ModelListAdapter.RowViewHolder holder
                            = (ModelListAdapter.RowViewHolder) view.getTag();
                    mCallback.onRightClicked(holder);
                }
            });
        } else {
            rowViewHolder = (ModelListAdapter.RowViewHolder) view.getTag();
        }

        rowViewHolder.mItemData = mListData.get(i);
        String itemSmName = rowViewHolder.mItemData.getSoundModelFullFileName();
        rowViewHolder.mTvSmName.setText(itemSmName);
        if (itemSmName.equalsIgnoreCase(mCheckedSmFullName)) {
            rowViewHolder.mCheckBox.setChecked(true);
        } else {
            rowViewHolder.mCheckBox.setChecked(false);
        }

        return view;
    }

    public void updateData(ArrayList<IExtendedSmModel> data) {
        mListData = data;
        notifyDataSetChanged();
    }

    public interface OnListenerCallback {
        void onLeftClicked(RowViewHolder rowViewHolder);

        void onRightClicked(RowViewHolder rowViewHolder);
    }

    public class RowViewHolder {
        public CheckBox mCheckBox;
        public TextView mTvSmName;
        public ImageView mIvSettings;
        public IExtendedSmModel mItemData;
    }
}
