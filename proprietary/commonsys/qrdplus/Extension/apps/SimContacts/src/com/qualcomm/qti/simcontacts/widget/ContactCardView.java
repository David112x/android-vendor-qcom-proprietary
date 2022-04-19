/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts.widget;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.ColorFilter;
import android.graphics.drawable.Drawable;
import android.support.v7.widget.CardView;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.qualcomm.qti.simcontacts.R;

import java.util.ArrayList;
import java.util.List;

public class ContactCardView extends CardView {

    private LinearLayout mEntryCardView;
    private List<List<Entry>> mEntries;
    private ArrayList<List<View>> mEntryViews;

    public final static class Entry {

        private final String mHint;
        private final String mContent;
        private final int mMainIconRes;
        private final Intent mMainIntent;
        private final int mSubIconRes;
        private final Intent mSubIntent;

        public Entry(String hint, String content, int mainIconRes,
                     Intent mainIntent, int subIconRes, Intent subIntent){
            mHint = hint;
            mContent = content;
            mMainIconRes = mainIconRes;
            mMainIntent = mainIntent;
            mSubIconRes = subIconRes;
            mSubIntent = subIntent;
        }

        public String getHint(){
            return mHint;
        }

        public String getContent(){
            return mContent;
        }

        public int getMainIconRes(){
            return mMainIconRes;
        }

        public Intent getMainIntent(){
            return mMainIntent;
        }

        public int getSubIconRes(){
            return mSubIconRes;
        }

        public Intent getSubIntent(){
            return mSubIntent;
        }
    }

    public ContactCardView(Context context) {
        this(context, null);
    }

    public ContactCardView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public ContactCardView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public void setColorFilter(ColorFilter colorFilter){
        for (List<View> viewList : mEntryViews){
            for (View view : viewList){
                ImageView mainIcon = view.findViewById(R.id.icon);
                Drawable mainDrawable = mainIcon.getDrawable();
                if (mainDrawable != null){
                    mainDrawable.setColorFilter(colorFilter);
                    mainIcon.setImageDrawable(mainDrawable);
                }
                ImageView subIcon = view.findViewById(R.id.third_icon);
                Drawable subDrawable = subIcon.getDrawable();
                if (subDrawable != null){
                    subDrawable.setColorFilter(colorFilter);
                    subIcon.setImageDrawable(subDrawable);
                }
            }
        }
    }

    public void setupViews(List<List<Entry>> entries){
        removeAllViews();
        LayoutInflater inflater = LayoutInflater.from(getContext());
        mEntryCardView = (LinearLayout) inflater.inflate(R.layout.entry_card_view, this, false);
        addView(mEntryCardView);

        mEntries = entries;
        mEntryViews = new ArrayList<>(entries.size());
        for (int i = 0; i < entries.size(); i++ ){
            mEntryViews.add(new ArrayList<View>());
        }

        inflateAllEntries(inflater);

        addAllViews();
    }

    private void addAllViews(){

        for (int i = 0; i < mEntryViews.size(); i++){
            List<View> viewList = mEntryViews.get(i);
            if (i > 0){
                mEntryCardView.addView(generateDivider(viewList.get(0)));
            }
            for (View view : viewList){
                mEntryCardView.addView(view);
            }
        }
    }

    private void inflateAllEntries(LayoutInflater inflater){
        for (int i = 0; i < mEntries.size(); i++) {
            List<Entry> entryList = mEntries.get(i);
            List<View> viewList = mEntryViews.get(i);
            for (int j = 0; j < entryList.size(); j++) {
                viewList.add(createView(inflater, entryList.get(j), j));
            }
        }
    }

    private View createView(LayoutInflater inflater, final Entry entry, int index){
        View entryView = inflater.inflate(R.layout.card_entry_item_layout, this, false);
        TextView content = entryView.findViewById(R.id.header);
        ImageView mainIcon = entryView.findViewById(R.id.icon);
        ImageView subIcon = entryView.findViewById(R.id.third_icon);
        if (index == 0){
            mainIcon.setVisibility(VISIBLE);
            mainIcon.setImageResource(entry.getMainIconRes());
        }else {
            mainIcon.setVisibility(INVISIBLE);
        }
        if (entry.getSubIconRes() != 0){
            subIcon.setVisibility(VISIBLE);
            subIcon.setImageResource(entry.getSubIconRes());
        }
        if (!TextUtils.isEmpty(entry.getHint())){
            content.setText(entry.getHint());
        }else if (!TextUtils.isEmpty(entry.getContent())){
            content.setText(entry.getContent());
        }

        if (entry.getMainIntent() != null){
            entryView.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View view) {
                    ((Activity)getContext()).startActivityForResult(entry.getMainIntent(), 0);
                }
            });
        }

        if (entry.getSubIntent() != null){
            subIcon.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View view) {
                    ((Activity)getContext()).startActivityForResult(entry.getSubIntent(), 0);
                }
            });
        }
        return entryView;
    }

    private View generateDivider(View entry) {
        View divider = new View(getContext());

        divider.setBackgroundColor(getResources().getColor(
                R.color.divider_line_color_light));
        LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                getResources().getDimensionPixelSize(R.dimen.divider_line_height));
        int marginStart = getResources().getDimensionPixelSize(R.dimen.entry_card_item_padding_start);
        ImageView entryIcon = entry.findViewById(R.id.icon);
        if (entryIcon.getVisibility() == View.VISIBLE) {
            int imageWidthAndMargin =
                    getResources().getDimensionPixelSize(R.dimen.entry_card_item_icon_width) +
                            getResources().getDimensionPixelSize(R.dimen.entry_card_item_image_spacing);
            marginStart += imageWidthAndMargin;
        }
        layoutParams.setMarginStart(marginStart);
        divider.setLayoutParams(layoutParams);
        return divider;
    }
}
