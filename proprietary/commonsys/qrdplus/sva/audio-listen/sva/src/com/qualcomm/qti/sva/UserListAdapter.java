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
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.qualcomm.qti.sva.data.IUserModel;
import com.qualcomm.qti.sva.utils.LogUtils;

import java.util.ArrayList;

public class UserListAdapter extends BaseAdapter {
    private final String TAG = UserListAdapter.class.getSimpleName();
    private Context mContext;
    private ArrayList<IUserModel> mListData;
    private OnListenerCallback mCallback;

    public UserListAdapter(Context context, ArrayList<IUserModel> listData,
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
        RowViewHolder rowViewHolder;
        if (null == view) {
            LayoutInflater inflater
                    = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            if (null == inflater) {
                LogUtils.d(TAG, "getView: inflater is null");
                return null;
            }
            view = inflater.inflate(R.layout.listitem_user, null);
            rowViewHolder = new RowViewHolder();
            view.setTag(rowViewHolder);
            rowViewHolder.mLayoutUserItem = view.findViewById(R.id.layout_user_item);
            rowViewHolder.mLayoutUserItem.setTag(rowViewHolder);
            rowViewHolder.mLayoutUserItem.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    LogUtils.d(TAG, "onClick: click user item");
                    RowViewHolder holder = (RowViewHolder) view.getTag();
                    mCallback.onUserItemClicked(holder);
                }
            });

            rowViewHolder.mTvUserName = view.findViewById(R.id.tv_user_name);
        } else {
            rowViewHolder = (RowViewHolder) view.getTag();
        }

        rowViewHolder.mItemData = mListData;
        rowViewHolder.mIndex = i;
        rowViewHolder.mTvUserName.setText(mListData.get(i).getUserName());

        return view;
    }

    public void updateData(ArrayList<IUserModel> data) {
        mListData = data;
        notifyDataSetChanged();
    }

    public interface OnListenerCallback {
        void onUserItemClicked(RowViewHolder rowViewHolder);
    }

    public class RowViewHolder {
        public RelativeLayout mLayoutUserItem;
        public TextView mTvUserName;
        public ArrayList<IUserModel> mItemData;
        public int mIndex;
    }
}
