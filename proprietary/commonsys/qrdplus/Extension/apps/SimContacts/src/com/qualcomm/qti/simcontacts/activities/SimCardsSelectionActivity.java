/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts.activities;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.android.contacts.activities.RequestPermissionsActivity;
import com.android.contacts.database.SimContactDao;
import com.android.contacts.model.SimCard;
import com.qualcomm.qti.simcontacts.R;
import com.qualcomm.qti.simcontacts.SimUpdateMonitor;
import com.qualcomm.qti.simcontacts.util.Constants;

import java.util.ArrayList;
import java.util.List;

public class SimCardsSelectionActivity extends AppCompatActivity
        implements AdapterView.OnItemClickListener, SimUpdateMonitor.SimStateListener{

    private SimContactDao mSimDao;
    private TextView mEmptyMessage;
    private ListView mSimCardList;
    private SimCardListAdapter mSimCardListAdapter;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        RequestPermissionsActivity.startPermissionActivityIfNeeded(this);
        setContentView(R.layout.sim_cards_activity);
        setTitle(getResources().getString(R.string.sim_cards_selection_title));

        mSimDao = SimContactDao.create(getApplicationContext());

        SimUpdateMonitor.getInstance(getApplicationContext()).registerSimStateListener(this);

        mEmptyMessage = findViewById(R.id.empty_message);
        mSimCardList = findViewById(R.id.sim_card_list);
        mSimCardListAdapter = new SimCardListAdapter(SimCardsSelectionActivity.this);
        mSimCardList.setAdapter(mSimCardListAdapter);
        mSimCardList.setOnItemClickListener(this);

        updateSimCards();
    }

    @Override
    public void onDestroy(){
        super.onDestroy();
        SimUpdateMonitor.getInstance(getApplicationContext()).unRegisterSimStateListener(this);
    }

    @Override
    public void onSimStateChanged(int subscriptionId, int state) {
        updateSimCards();
    }

    private void updateSimCards(){
        List<SimCard> simCards = mSimDao.getSimCards();
        mSimCardListAdapter.setData(simCards);
        mEmptyMessage.setVisibility(simCards.size() > 0 ? View.GONE : View.VISIBLE);
        mSimCardList.setVisibility(simCards.size() > 0 ? View.VISIBLE : View.GONE);
    }

    @Override
    public void onItemClick(AdapterView<?> adapterView, View view, int position, long l) {
        final SimCard simCard = (SimCard)mSimCardListAdapter.getItem(position);
        final Intent intent = new Intent(getApplicationContext(),
                SimContactBrowseListActivity.class);
        intent.putExtra(Constants.SIM_CARD_SUBSCRIPTION_ID, simCard.getSubscriptionId());
        intent.putExtra(Constants.SIM_CARD_DISPLAY_NAME,
                simCard.getDisplayName()+" "+(simCard.getSimSlotIndex()+1));
        startActivity(intent);
    }


    private static class SimCardListAdapter extends BaseAdapter {

        private List<SimCard> mSimCards = new ArrayList<>();
        private LayoutInflater inflater;

        public SimCardListAdapter(Context context){
            this.inflater = LayoutInflater.from(context);
        }

        public void setData(List<SimCard> simCards){
            mSimCards.clear();
            mSimCards.addAll(simCards);
            notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            return mSimCards.size();
        }

        @Override
        public Object getItem(int i) {
            return mSimCards.get(i);
        }

        @Override
        public long getItemId(int i) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup viewGroup) {
            ViewHolder holder = null;
            if(convertView == null){
                holder = new ViewHolder();
                convertView = inflater.inflate(R.layout.sim_card_item, null);
                holder.name = convertView.findViewById(R.id.sim_card_name);
                holder.phone = convertView.findViewById(R.id.sim_card_phone);
                convertView.setTag(holder);
            }
            else{
                holder = (ViewHolder) convertView.getTag();
            }
            updateSimCardDisplay(holder, position);
            return convertView;
        }

        private void updateSimCardDisplay(ViewHolder holder, int position){
            final SimCard simCard = mSimCards.get(position);
            holder.name.setText(simCard.getDisplayName()+" "+(simCard.getSimSlotIndex()+1));
            holder.phone.setText(simCard.getPhone());
        }

        private class ViewHolder{
            public TextView name;
            public TextView phone;
        }
    }
}
