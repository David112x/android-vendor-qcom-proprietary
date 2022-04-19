/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts.widget;

import android.content.Context;
import android.content.res.ColorStateList;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.graphics.Rect;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.ActionBar;
import android.support.v7.widget.Toolbar;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.TextView;

import com.qualcomm.qti.simcontacts.R;

import java.util.ArrayList;
import java.util.List;

public class ContactToolbar extends Toolbar implements View.OnFocusChangeListener{
    private List<SelectionBarAction> mActions = new ArrayList<>();
    private ViewGroup mSearchViewContainer;
    private EditText mSearchView;
    private ViewGroup mSelectionContainer;
    private Button mSelectionButton;
    private PopupWindow mPopupWindow;
    private Listener mListener;
    private boolean mSearchMode;
    private ListView mActionList;
    private int mPopupWidth;
    private int mPopupHeight;
    private int mPopupOffsetX;
    private int mPopupOffsetY;

    private final PopupWindow.OnDismissListener mOnDismissListener =
            new PopupWindow.OnDismissListener() {
                @Override
                public void onDismiss() {
                    mPopupWindow = null;
                }
            };

    private final AdapterView.OnItemClickListener mOnItemClickListener =
            new AdapterView.OnItemClickListener() {
                @Override
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                    if (mPopupWindow != null) {
                        mPopupWindow.dismiss();
                        if (SelectionBarAction.SELECT_OR_DESELECT_ACTION  == id
                                && mListener != null){
                            mListener.onAllContactsSelectedOrDeselected();
                        }
                    }
                }
            };

    public interface Listener{
        void onSearchKeywordChanged(String keyword);
        void onAllContactsSelectedOrDeselected();
        void onSearchModeStopped();
    }

    public ContactToolbar(Context context) {
        super(context);
    }

    public ContactToolbar(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public ContactToolbar(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public void setListener(Listener listener){
        mListener = listener;
    }

    public void setSearchMode(boolean flag) {
        if (mSearchMode != flag) {
            mSearchMode = flag;
            update();
        }
    }

    public boolean isSearchMode(){
        return mSearchMode;
    }

    public void updateSelectionCount(int count, int total){
        String countTitle = getContext().getString(R.string.contacts_selected,
                count);
        mSelectionButton.setText(countTitle);
        updateSelectionBarActions(countTitle, count, total);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        setElevation(4 * getContext().getResources().getDisplayMetrics().density);
        LayoutInflater inflater = LayoutInflater.from(getContext());
        mSearchViewContainer =  (ViewGroup)inflater.inflate(R.layout.selection_search_bar, null);
        mSearchView = mSearchViewContainer.findViewById(R.id.search_view);
        mSearchViewContainer.setLayoutParams(new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT));
        mSelectionContainer = (ViewGroup)inflater.inflate(R.layout.contact_selection_bar, null);
        mSelectionButton = mSelectionContainer.findViewById(R.id.selection_menu);
        mSelectionButton.setLayoutParams(new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.WRAP_CONTENT, FrameLayout.LayoutParams.WRAP_CONTENT));
        String countTitle = getContext().getString(R.string.contacts_selected, 0);
        mSelectionButton.setText(countTitle);
        mSelectionButton.setElevation(4 * getContext().getResources().getDisplayMetrics().density);
        addView(mSelectionContainer);
        addView(mSearchViewContainer);
        // Setup selection bar
        mActions.add(new SelectionBarAction(
                SelectionBarAction.SELECT_OR_DESELECT_ACTION,
                getContext().getString(R.string.menu_select_all)));
        mSelectionButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                showSelectionBarActions();
            }
        });

        mSearchView.setFocusable(true);
        mSearchView.setOnFocusChangeListener(this);
        mSearchView.addTextChangedListener(new SearchTextWatcher());
        update();
    }

    @Override
    public void onFocusChange(View view, boolean hasFocus) {
        switch (view.getId()) {
            case R.id.search_view: {
                if (hasFocus) {
                    showSoftKeyboard();
                    if (mListener != null){
                        mListener.onSearchKeywordChanged(null);
                    }
                }else {
                    hideSoftKeyboard();
                    if (mSearchView != null
                            && !TextUtils.isEmpty(mSearchView.getText().toString())) {
                        mSearchView.setText(null);
                    }
                }
            }
        }
    }

    private class SearchTextWatcher implements TextWatcher {
        @Override
        public void onTextChanged(CharSequence queryString, int start, int before, int count) {
            if (mListener != null){
                mListener.onSearchKeywordChanged(queryString.toString());
            }
        }

        @Override
        public void afterTextChanged(Editable s) {
        }

        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        }
    }

    private static class SelectionBarAction {

        public static final int SELECTED_STATE = 0;
        public static final int SELECT_OR_DESELECT_ACTION =1;
        private final int mId;
        private String mTitle;

        public SelectionBarAction(int id, String title) {
            mId = id;
            mTitle = title;
        }

        private int getId(){
            return mId;
        }

        public String getTitle(){
            return mTitle;
        }
    }

    private class SelectionBarActionAdapter extends BaseAdapter {
        @Override
        public int getCount() {
            return mActions.size();
        }

        @Override
        public Object getItem(int position) {
            return mActions.get(position);
        }

        @Override
        public long getItemId(int position) {
            return mActions.get(position).getId();
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                convertView = LayoutInflater.from(getContext())
                        .inflate(R.layout.selection_bar_action_item, null);
            }
            TextView actionText = convertView.findViewById(R.id.selection_bar_action_title);
            actionText.setText(mActions.get(position).getTitle());
            return convertView;
        }
    }

    private void showSoftKeyboard(){
        final InputMethodManager inputMethodManager = (InputMethodManager) getContext().
                getSystemService(Context.INPUT_METHOD_SERVICE);
        inputMethodManager.showSoftInput(mSearchView.findFocus(), 0);
    }

    private void hideSoftKeyboard() {
        // Hide soft keyboard, if visible
        InputMethodManager inputMethodManager = (InputMethodManager) getContext().getSystemService(
                Context.INPUT_METHOD_SERVICE);
        inputMethodManager.hideSoftInputFromWindow(mSearchView.getWindowToken(), 0);
    }

    private void update() {
        updateToolbarContainerVisibility();
//        setHomeAsUpIndicator(mSearchMode ? R.color.primary_color : android.R.color.white);
        updateActionBarBackground(mSearchMode ? android.R.color.white : R.color.primary_color);
        if (mSearchMode) {
            mSearchView.clearFocus();
            mSearchView.requestFocus();
        }else {
            if (mListener != null && !mSearchMode){
                mListener.onSearchModeStopped();
            }
            mSearchView.clearFocus();
        }
    }

    private void updateToolbarContainerVisibility(){
        mSearchViewContainer.setVisibility(mSearchMode ? View.VISIBLE : View.GONE);
        mSelectionContainer.setVisibility(mSearchMode ? View.GONE : View.VISIBLE);
    }

    public void updateHomeAsUpIndicator(ActionBar actionBar){
        setHomeAsUpIndicator(mSearchMode ? R.color.primary_color
                : android.R.color.white, actionBar);
    }

    private void setHomeAsUpIndicator(int color, ActionBar actionBar){
        Drawable upArrow = ContextCompat.getDrawable(getContext(),
                R.drawable.quantum_ic_arrow_back_vd_theme_24);
        upArrow.setColorFilter(getContext().getColor(color), PorterDuff.Mode.SRC_ATOP);
        actionBar.setHomeAsUpIndicator(upArrow);
    }

    private void updateActionBarBackground(int color){
        setBackgroundDrawable(new ColorDrawable(getContext().getColor(color)));
    }

    private void showSelectionBarActions(){
        mPopupWindow = createPopupWindow();
        updatePopupLayoutParams();
        mPopupWindow.setWidth(mPopupWidth);
        mPopupWindow.setHeight(mPopupHeight);
        mPopupWindow.showAsDropDown(mSelectionButton, mPopupOffsetX, mPopupOffsetY);
    }

    private PopupWindow createPopupWindow() {
        PopupWindow popup = new PopupWindow(getContext());
        popup.setOnDismissListener(mOnDismissListener);
        popup.setBackgroundDrawable(new ColorDrawable(Color.WHITE));

        mActionList = new ListView(getContext(), null,
                android.R.attr.dropDownListViewStyle);
        mActionList.setBackgroundResource(R.color.selection_bar_action_list_background);
        mActionList.setAdapter(new SelectionBarActionAdapter());
        mActionList.setOnItemClickListener(mOnItemClickListener);
        popup.setElevation(24);
        popup.setContentView(mActionList);
        popup.setFocusable(true);
        popup.setOutsideTouchable(true);
        return popup;
    }

    private void updatePopupLayoutParams() {
        ListView content = mActionList;
        PopupWindow popup = mPopupWindow;

        Rect p = new Rect();
        popup.getBackground().getPadding(p);

        int maxHeight = mPopupWindow.getMaxAvailableHeight(mSelectionButton) - p.top - p.bottom;
        mActionList.measure(
                View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED),
                View.MeasureSpec.makeMeasureSpec(maxHeight, View.MeasureSpec.AT_MOST));
        mPopupWidth = content.getMeasuredWidth() + p.top + p.bottom;
        mPopupHeight = Math.min(maxHeight, content.getMeasuredHeight() + p.left + p.right);
        mPopupOffsetX = -p.left;
        mPopupOffsetY = -mSelectionButton.getHeight()-p.top;
    }

    private void updateSelectionBarActions(String countTitle, int selectedCount, int total) {
        mActions.clear();
        mActions.add(new SelectionBarAction(SelectionBarAction.SELECTED_STATE, countTitle));
        boolean selectAll = true;
        if (selectedCount == total && total > 0) {
            selectAll = false;
        }
        mActions.add(new SelectionBarAction(
                SelectionBarAction.SELECT_OR_DESELECT_ACTION,
                getContext().getString(selectAll ? R.string.menu_select_all
                        : R.string.menu_deselect_all)));
    }
}
