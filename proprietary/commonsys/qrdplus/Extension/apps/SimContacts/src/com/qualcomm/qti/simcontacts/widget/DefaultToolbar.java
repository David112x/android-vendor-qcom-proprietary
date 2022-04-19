/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts.widget;

import android.app.Activity;
import android.content.Context;
import android.content.res.ColorStateList;
import android.graphics.drawable.ColorDrawable;
import android.support.v7.widget.Toolbar;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;

import com.qualcomm.qti.simcontacts.R;

public class DefaultToolbar extends Toolbar {
    private boolean mSelectionMode;
    private int mActionBarAnimationDuration;
    private View mSelectionContainer;
    private int mDefaultContentInsetStart;
    private Activity mActivity;
    private Listener mListener;

    public interface Listener{
        void onBackPressed();
        void onSelectionModeBeginStopping();
        void onSelectionModeStopped();
        void onSelectionModeStarted();
    }

    public DefaultToolbar(Context context) {
        super(context);
    }

    public DefaultToolbar(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public DefaultToolbar(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public void setListener(Listener listener){
        mListener = listener;
    }

    public void setSelectionMode(boolean flag){
        if (mSelectionMode != flag) {
            mSelectionMode = flag;
            update();
        }
    }

    public boolean isSelectionMode(){
        return mSelectionMode;
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mActivity = (Activity) getContext();
        mDefaultContentInsetStart = getContentInsetStart();
        mActionBarAnimationDuration =
                getResources().getInteger(R.integer.action_bar_animation_duration);
        setElevation(4 * mActivity.getResources().getDisplayMetrics().density);
        final LayoutInflater inflater = (LayoutInflater) getContext().getSystemService(
                Context.LAYOUT_INFLATER_SERVICE);
        mSelectionContainer = inflater.inflate(R.layout.sim_selection_bar, this, false);
        addView(mSelectionContainer);
        mSelectionContainer.findViewById(R.id.selection_close).
                setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        if (mListener != null){
                            mListener.onBackPressed();
                        }
                    }
                });
        mSelectionContainer.setVisibility(GONE);
    }

    public void updateSelectionCount(int count){
        TextView selectionCountText = mSelectionContainer.findViewById(R.id.selection_count_text);
        if (count > 0){
            selectionCountText.setVisibility(View.VISIBLE);
        }else {
            selectionCountText.setVisibility(View.GONE);
        }
        selectionCountText.setText(String.valueOf(count));
    }

    public void updateMenuIconTintList(){
        Menu menu = getMenu();
        for (int index = 0 ; index < menu.size(); index++){
            MenuItem menuItem = menu.getItem(index);
            if (!mSelectionMode) {
                menuItem.setIconTintList(ColorStateList.valueOf(mActivity.getColor(
                        android.R.color.white)));
            } else {
                menuItem.setIconTintList(ColorStateList.valueOf(mActivity.getColor(
                        R.color.quantum_black_secondary_text)));
            }
        }
    }

    private void update() {
        if (mSelectionMode){
            mSelectionContainer.setVisibility(VISIBLE);
            mSelectionContainer.setAlpha(1);
            updateDisplayOptions();
        }else {
            mListener.onSelectionModeBeginStopping();
            mSelectionContainer.setAlpha(1);
            mSelectionContainer.animate().alpha(0).setDuration(mActionBarAnimationDuration)
                    .withEndAction(new Runnable() {
                        @Override
                        public void run() {
                            updateDisplayOptions();
                            mSelectionContainer.setVisibility(GONE);
                        }
                    });
        }
    }

    private void updateDisplayOptions(){
        if (mSelectionMode){
            mListener.onSelectionModeStarted();
        }else {
            mListener.onSelectionModeStopped();
        }
        updateActionBarBackground(mSelectionMode ? R.color.contextual_selection_bar_color :
                R.color.primary_color);
        updateStatusBarColor(mSelectionMode ? R.color.contextual_selection_bar_status_bar_color :
                R.color.primary_color_dark);
        updateContentInsets(mSelectionMode ? 0 : mDefaultContentInsetStart);
    }

    private void updateContentInsets(int contentInsetStart){
        setContentInsetsRelative(contentInsetStart, getContentInsetEnd());
    }

    private void updateStatusBarColor(int color){
        mActivity.getWindow().setStatusBarColor(mActivity.getColor(color));
    }

    private void updateActionBarBackground(int color){
        setBackgroundDrawable(new ColorDrawable(mActivity.getColor(color)));
    }

}
