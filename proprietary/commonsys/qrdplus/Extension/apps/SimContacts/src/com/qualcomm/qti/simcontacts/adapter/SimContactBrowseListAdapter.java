/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts.adapter;

import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import com.android.contacts.model.SimContact;
import com.android.contacts.util.ContactsSectionIndexer;
import com.qualcomm.qti.simcontacts.R;
import com.qualcomm.qti.simcontacts.util.SimContactsSortAlgorithm;

import java.util.ArrayList;

public class SimContactBrowseListAdapter extends MultiSelectContactListAdapter<SimContact> {


    public SimContactBrowseListAdapter(Context context) {
        super(context);
    }

    @Override
    public void setData(ArrayList<SimContact> simContacts, Bundle bundle) {
        mContacts.clear();
        mContacts.addAll(simContacts);

        if (bundle.containsKey(SimContactsSortAlgorithm.EXTRA_CONTACTS_INDEX_TITLES)
                && bundle.containsKey(SimContactsSortAlgorithm.EXTRA_CONTACTS_INDEX_COUNTS)) {
            String sections[] = bundle.getStringArray(
                    SimContactsSortAlgorithm.EXTRA_CONTACTS_INDEX_TITLES);
            int counts[] = bundle.getIntArray(
                    SimContactsSortAlgorithm.EXTRA_CONTACTS_INDEX_COUNTS);
            setIndexer(new ContactsSectionIndexer(sections, counts));
        } else {
            setIndexer(null);
        }
    }

    @Override
    public ArrayList<SimContact> getData() {
        return mContacts;
    }

    @Override
    public void bindView(View itemView, int position) {
        CheckBox checkBox = itemView.findViewById(R.id.contact_list_item_checkbox);
        ImageView photoView = itemView.findViewById(R.id.contact_list_item_photo);
        TextView name = itemView.findViewById(R.id.contact_list_item_name);
        TextView section = itemView.findViewById(R.id.section_index);
        itemView.findViewById(R.id.contact_list_item_number).setVisibility(View.GONE);

        bindCheckBox(checkBox, mContacts.get(position).getId());
        bindPhotoView(photoView,mContacts.get(position).getItemLabel(),
                mContacts.get(position).getLookupKey());
        bindContactName(name, mContacts.get(position).getItemLabel());
        bindSectionHeader(section, position);
    }
}
