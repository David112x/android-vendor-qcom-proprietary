/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts.adapter;

import android.content.Context;
import android.net.Uri;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SectionIndexer;
import android.widget.TextView;

import com.android.contacts.ContactPhotoManager;
import com.qualcomm.qti.simcontacts.R;

import java.util.ArrayList;
import java.util.TreeSet;


public abstract class MultiSelectContactListAdapter<T> extends BaseAdapter {
    protected final Context mContext;
    private final LayoutInflater mInflater;
    private Placement mPlacement = new Placement();
    private SectionIndexer mIndexer;
    private ContactPhotoManager mContactPhotoManager;
    private TreeSet<Long> mSelectedContactIds = new TreeSet<>();
    private boolean mDisplayCheckBox;
    private ContactSelectionListener mContactSelectionListener;

    protected ArrayList<T> mContacts = new ArrayList<>();

    public interface ContactSelectionListener{
        void onSelectedContactsChanged();
    }

    public MultiSelectContactListAdapter(Context context){
        mContext = context;
        mInflater = LayoutInflater.from(context);
        mContactPhotoManager = ContactPhotoManager.getInstance(context);
    }

    public void setContactSelectionListener(ContactSelectionListener listener){
        mContactSelectionListener = listener;
    }

    public void setDisplayCheckBox(boolean displayCheckBox){
        mDisplayCheckBox = displayCheckBox;
        notifyDataSetChanged();
    }

    public boolean isDisplayCheckBox(){
        return mDisplayCheckBox;
    }

    public void clearSelectedContactIds(){
        mSelectedContactIds.clear();
        notifyDataSetChanged();
        if (mContactSelectionListener != null){
            mContactSelectionListener.onSelectedContactsChanged();
        }
    }

    public TreeSet<Long> getSelectedContactIds(){
        return mSelectedContactIds;
    }

    public void toggleSelectedContactIds(long contactId){
        if (mDisplayCheckBox){
            if (mSelectedContactIds.contains(contactId)){
                mSelectedContactIds.remove(contactId);
            }else {
                mSelectedContactIds.add(contactId);
            }
            notifyDataSetChanged();
            if (mContactSelectionListener != null){
                mContactSelectionListener.onSelectedContactsChanged();
            }
        }
    }

    public void setSelectedContactIds(TreeSet<Long> selectedContactIds){
        mSelectedContactIds.clear();
        mSelectedContactIds.addAll(selectedContactIds);
        notifyDataSetChanged();
        if (mContactSelectionListener != null){
            mContactSelectionListener.onSelectedContactsChanged();
        }
    }

    public void setIndexer(SectionIndexer index) {
        mIndexer = index;
        mPlacement.invalidate();
    }

    public abstract void setData(ArrayList<T> simContacts, Bundle bundle);

    public abstract ArrayList<T> getData();

    @Override
    public int getCount() {
        return mContacts.size();
    }

    @Override
    public T getItem(int i) {
        return mContacts.get(i);
    }

    @Override
    public long getItemId(int i) {
        return 0;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View v;
        if (convertView != null && convertView.getTag() != null) {
            v = convertView;
        } else {
            v = newView(parent);
        }
        bindView(v, position);
        return v;
    }

    private View newView(ViewGroup parent) {
        View itemView = mInflater.inflate(R.layout.contact_list_item, parent, false);
        itemView.setTag(new Object());
        return itemView;
    }

    public abstract void bindView(View itemView, int position);

    protected void bindContactName(TextView textView, String displayName){
        textView.setText(displayName);
    }

    protected void bindCheckBox(CheckBox checkBox, long contactId){
        checkBox.setVisibility(mDisplayCheckBox ? View.VISIBLE : View.GONE);
        checkBox.setChecked(mSelectedContactIds.contains(contactId));
    }

    protected void bindPhotoView(ImageView photoView, String name, String lookupKey) {
        bindPhotoView(photoView, name, lookupKey, 0, null);
    }

    protected void bindPhotoView(ImageView photoView, String name, String lookupKey,
                                 long photoId, Uri photoUri) {
        photoView.setVisibility(View.VISIBLE);

        if (photoId != 0) {
            mContactPhotoManager.loadThumbnail(photoView, photoId, false, true,
                    null);
        } else {
            ContactPhotoManager.DefaultImageRequest request = null;
            if (photoUri == null) {
                request = new ContactPhotoManager.DefaultImageRequest(name, lookupKey, true);
            }
            mContactPhotoManager.loadDirectoryPhoto(photoView, photoUri, false, true,
                    request);
        }
    }

    protected void bindSectionHeader(TextView section, int position) {
        section.setVisibility(View.VISIBLE);
        Placement placement = getItemPlacementInSection(position);
        section.setText(placement.sectionHeader);
        section.setTextAppearance(mContext, R.style.SectionHeaderStyle);
    }

    private int getPositionForSection(int sectionIndex) {
        if (mIndexer != null) {
            return mIndexer.getPositionForSection(sectionIndex);
        }
        return -1;
    }

    private Object[] getSections() {
        if (mIndexer != null) {
            return mIndexer.getSections();
        } else {
            return new String[] {" "};
        }
    }

    private int getSectionForPosition(int position) {
        if (mIndexer != null) {
            return mIndexer.getSectionForPosition(position);
        }
        return -1;
    }

    /**
     * Computes the item's placement within its section and populates the {@code placement}
     * object accordingly. Please note that the returned object is volatile and should be copied
     * if the result needs to be used later.
     */
    private Placement getItemPlacementInSection(int position) {
        if (mPlacement.position == position) {
            return mPlacement;
        }
        mPlacement.position = position;
        int section = getSectionForPosition(position);
        if (section != -1 && getPositionForSection(section) == position) {
            mPlacement.sectionHeader = (String) getSections()[section];
        } else {
            mPlacement.sectionHeader = null;
        }
        return mPlacement;
    }

    private static final class Placement {
        private int position = ListView.INVALID_POSITION;
        private String sectionHeader;

        void invalidate() {
            position = ListView.INVALID_POSITION;
        }
    }
}

