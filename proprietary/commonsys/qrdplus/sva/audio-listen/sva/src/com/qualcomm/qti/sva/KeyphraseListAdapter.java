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
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;

import com.qualcomm.qti.sva.data.IExtendedSmModel;
import com.qualcomm.qti.sva.data.ISettingsModel;
import com.qualcomm.qti.sva.data.ISmModel;
import com.qualcomm.qti.sva.data.SettingsModel;
import com.qualcomm.qti.sva.utils.LogUtils;

import java.util.ArrayList;

public class KeyphraseListAdapter extends BaseAdapter {
    private final String TAG = KeyphraseListAdapter.class.getSimpleName();

    private Context mContext;
    private ArrayList<IExtendedSmModel> mListData;
    private OnListenerCallback mCallback;
    private static final String DESC_SWITCH_SUFFIX = "_switch";
    private static final String VERSION_2_SUFFIX = "2.0";
    private static final String VERSION_3_SUFFIX = "3.0";
    private static final String VERSION_4_SUFFIX = "4.0";
    private static final String VERSION_UNKNOW_SUFFIX = "unknown";

    public KeyphraseListAdapter(Context context, ArrayList<IExtendedSmModel> listData,
                                OnListenerCallback callback) {
        mContext = context;
        mListData = listData;
        mCallback = callback;
    }

    public void updateData(ArrayList<IExtendedSmModel> data) {
        mListData = data;
        notifyDataSetChanged();
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
        RowViewHolder rowViewHolder;
        if (null == view) {
            LayoutInflater inflater
                    = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            if (null != inflater) {
                view = inflater.inflate(R.layout.listitem_keyphrase, null);
            } else {
                LogUtils.d(TAG, "getView: inflater is null");
                return null;
            }
            rowViewHolder = new RowViewHolder();
            view.setTag(rowViewHolder);
            View layoutLeft = view.findViewById(R.id.layout_left);
            rowViewHolder.mKeyphraseName = view.findViewById(R.id.list_item_keyphrase_name);
            rowViewHolder.mSettings = view.findViewById(R.id.list_item_settings);
            rowViewHolder.mSwitch = view.findViewById(R.id.list_item_switch);
            layoutLeft.setTag(rowViewHolder);
            rowViewHolder.mSwitch.setTag(rowViewHolder);
            layoutLeft.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    RowViewHolder holder = (RowViewHolder) view.getTag();
                    mCallback.onLeftClicked(holder);
                }
            });

            rowViewHolder.mSwitch.setOnCheckedChangeListener(
                    new CompoundButton.OnCheckedChangeListener() {
                        @Override
                        public void onCheckedChanged(CompoundButton compoundButton, boolean checked) {
                            RowViewHolder switchHolder = (RowViewHolder) compoundButton.getTag();
                            LogUtils.d(TAG, "switch check status change = " + checked + ", "
                                    + switchHolder.mItemData.getSoundModelFullFileName());
                            mCallback.onRightClicked(switchHolder);
                        }
                    });
            rowViewHolder.mSwitch.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    RowViewHolder switchHolder = (RowViewHolder) view.getTag();
                    LogUtils.d(TAG, "switch is clicked "
                            + switchHolder.mItemData.getSoundModelFullFileName());
                }
            });

        } else {
            rowViewHolder = (RowViewHolder) view.getTag();
        }

        rowViewHolder.mItemData = mListData.get(i);
        rowViewHolder.mKeyphraseName.setText(
                rowViewHolder.mItemData.getSoundModelPrettyKeyphrase());
        rowViewHolder.mSwitch.setContentDescription(
                rowViewHolder.mItemData.getSoundModelFullFileName() + DESC_SWITCH_SUFFIX);
        String settings = mContext.getString(R.string.action)
                + rowViewHolder.mItemData.getSoundModelActionName();
        if (rowViewHolder.mItemData.isUserTrainedSoundModel()) {
            ISettingsModel settingsModel
                    = new SettingsModel(mContext,
                    rowViewHolder.mItemData.getSoundModelFullFileName());
            String onOff = settingsModel.getUserVerificationEnabled() ? "On" : "Off";
            settings += " " + mContext.getString(R.string.user_verification_colon) + onOff;
        }

        ISmModel.MODEL_VERSION version = rowViewHolder.mItemData.getSoundModelVersion();
        switch (version) {
            case VERSION_2_0:
                settings += " " + mContext.getString(R.string.model) + VERSION_2_SUFFIX;
                break;
            case VERSION_3_0:
                settings += " " + mContext.getString(R.string.model) + VERSION_3_SUFFIX;
                break;
            case VERSION_4_0:
                settings += " " + mContext.getString(R.string.model) + VERSION_4_SUFFIX;
                break;
            default:
                settings += " " + mContext.getString(R.string.model) + VERSION_UNKNOW_SUFFIX;
                break;
        }

        rowViewHolder.mSettings.setText(settings);
        boolean checked = rowViewHolder.mItemData.getSessionStatus()
                == IExtendedSmModel.SessionStatus.STARTED;
        rowViewHolder.mSwitch.setChecked(checked);

        return view;
    }

    public interface OnListenerCallback {
        void onLeftClicked(RowViewHolder rowViewHolder);

        void onRightClicked(RowViewHolder rowViewHolder);
    }

    public class RowViewHolder {
        public TextView mKeyphraseName;
        public TextView mSettings;
        public Switch mSwitch;
        public IExtendedSmModel mItemData;
    }
}
