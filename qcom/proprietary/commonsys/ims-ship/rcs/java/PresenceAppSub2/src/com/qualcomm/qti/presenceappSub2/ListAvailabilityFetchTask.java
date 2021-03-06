/*************************************************************************
 Copyright (c)2012-2013,2017 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
*************************************************************************/

package com.qualcomm.qti.presenceappSub2;

import java.util.ArrayList;

import com.qualcomm.qti.presenceappSub2.R;
import com.android.ims.internal.uce.common.StatusCode;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Looper;
import android.os.RemoteException;
import android.util.Log;
import android.widget.Toast;

public class ListAvailabilityFetchTask extends AsyncTask<Void, Integer, Integer> {
    final String TAG = "Presence_UI2: ListSubscribePollingTask";

    Context mContext;
    ProgressDialog dialog;

    ListAvailabilityFetchTask me;

    public ListAvailabilityFetchTask() {
    }

    public ListAvailabilityFetchTask(Context appContext) {

        mContext = appContext;
        me = this;
    }

    @Override
    protected void onPreExecute() {
        super.onPreExecute();

        initProgressBar();
    }

    private void initProgressBar() {
        dialog = new ProgressDialog(mContext);
        dialog.setMessage("API Request in progress...");
        dialog.setCancelable(false);

        AppGlobalState.setProgressDialog(dialog);

        dialog.show();

    }

    @Override
    protected Integer doInBackground(Void... params) {
        Log.d(TAG, "doInBackground(), Thread=" + Thread.currentThread().getName());

        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        ArrayList<Contact> contacts = AppGlobalState.getContacts();
        ArrayList<String> uriList = new ArrayList<String>();
        for (Contact c : contacts) {
            Log.d(TAG, "c = " + contacts);
            Log.d(TAG, "c.isMultiSelected() = " + c.isMultiSelected());
            if (c.isMultiSelected()) {
                uriList.clear();
                Log.d(TAG, "uriList = " + uriList);
                String phone = c.getPhone();
                Log.d(TAG, "phone = " + phone);
                uriList.add(prepareCompleteUri(phone));
                Log.d(TAG, "uriList = " + uriList);

                sendListSubscribePollingRequest(uriList);
            }
        }

        return 0;
    }

    private ArrayList<String> getSelectedContactsList() {
        ArrayList<Contact> contacts = AppGlobalState.getContacts();
        ArrayList<String> uriList = new ArrayList<String>();
        for (Contact c : contacts) {
            Log.d(TAG, "c = " + contacts);
            if (c.isMultiSelected()) {
                Log.d(TAG, "c.isMultiSelected() = " + c.isMultiSelected());
                String phone = c.getPhone();
                Log.d(TAG, "phone = " + phone);
                if (AppGlobalState.getOperatorMode() == AppGlobalState.VZW_MODE
                  || AppGlobalState.getOperatorMode() == AppGlobalState.TMO_MODE)
                {
                    uriList.add(prepareCompleteUri(phone));
                }
                else
                {
                    uriList.add(prepareCompleteUriForAtt(c));
                }
                Log.d(TAG, "uriList = " + uriList);
            }
        }
        return uriList;
    }

    private SharedPreferences getSharedPrefHandle(String imsPresencePref) {
        SharedPreferences settings = mContext
                .getSharedPreferences(imsPresencePref, 0);

        return settings;
    }

    private String prepareCompleteUri(String phone) {
        SharedPreferences setting = getSharedPrefHandle(AppGlobalState.IMS_PRESENCE_MY_INFO);

        String uri1Value = setting.getString(
                mContext.getString(R.string.uri1text), "");
        String uri2Value = setting.getString(
                mContext.getString(R.string.uri2text), "");

        phone = "" + (uri1Value != null ? uri1Value : "") + phone;
        phone = phone + (uri2Value != null ? uri2Value : "");

        Log.d(TAG, "prepareCompleteUri = " + phone);
        return phone;
    }

    private String prepareCompleteUriForAtt(Contact contact) {

        String phone = contact.getContactUri();
        if((phone == null) || (phone != null && phone.length() == 0))
        {
            phone = contact.getResourceUri();

        }
        if(phone != null && phone.length() > 0)
        {
            if(phone.equals("<Not Subscribed>"))
            {
                if(!contact.getIndividualContactURI().equals(""))
                {
                    phone = contact.getIndividualContactURI();
                }
                else
                {
                    phone = "tel:" + contact.getPhone();
                }
            }
        }
        else
        {
            if(!contact.getIndividualContactURI().equals(""))
            {
                phone = contact.getIndividualContactURI();
            }
            else
            {
                phone = "tel:" + contact.getPhone();
            }
        }
        return phone;
    }

    private Integer sendListSubscribePollingRequest(ArrayList<String> contactList) {
        Log.d(TAG, "sendListSubscribePollingRequest for " + contactList.get(0));

        try {
            if(AppGlobalState.getPresenceService()!= null && AppGlobalState.isCdByOption() == false)
            {
                Log.d(TAG, "ListAvailabilityFetchTask : sendListSubscribePollingRequest : AppGlobalState.getPresenceSerrviceHandle() "+AppGlobalState.getPresenceSerrviceHandle());
                Log.d(TAG, "ListAvailabilityFetchTask : sendListSubscribePollingRequest : contactList.get(0) "+contactList.get(0));
                RequestInfo requestinfoObject = new RequestInfo();
                requestinfoObject.URI = new String[1];
                requestinfoObject.URI[0] = contactList.get(0);
                requestinfoObject.userData = AppGlobalState.getpUserDataValue();
                AppGlobalState.requestinfo.add(requestinfoObject);
                StatusCode status = AppGlobalState.getPresenceService().getContactCap(AppGlobalState.getPresenceSerrviceHandle(), contactList.get(0), AppGlobalState.getpUserData());
                Log.d(TAG, "ListAvailabilityFetchTask : sendListSubscribePollingRequest :  status.getStatusCode() "+ status.getStatusCode());
                return status.getStatusCode();
            }
            else if(AppGlobalState.getCdService() != null && AppGlobalState.isCdByOption())
            {
                String [] stringContactList = new String[contactList.size()];
                for(int i = 0; i < contactList.size(); i++)
                {
                stringContactList[i] = contactList.get(i);
                }
                StatusCode status = AppGlobalState.getCdService().getContactListCap(AppGlobalState.getCdServiceHandle(), stringContactList, AppGlobalState.getpUserData());
                Log.d(TAG, "ListAvailabilityFetchTask : QCDService_RequestGivenContactsCDInfo :  status.getStatusCode() "+ status.getStatusCode());
                return status.getStatusCode();
            }
            else
            {
                Log.d(TAG, "ListAvailabilityFetchTask : sendListSubscribePollingRequest : AppGlobalState.getPresenceService() = NULL");
                return -1;
            }
        } catch (RemoteException e) {
            e.printStackTrace();
            return -2;
        } catch (Exception e) {
            e.printStackTrace();
            return -3;
        }
    }

    @Override
    protected void onProgressUpdate(Integer... values) {
        super.onProgressUpdate(values);
    }

    @Override
    protected void onPostExecute(Integer result) {
        super.onPostExecute(result);
        Log.d(TAG, "onPostExecute(), Thread=" +
                Thread.currentThread().getName());

        dialog.dismiss();

        if (getSelectedContactsList().size() == 0) {
            Toast.makeText(mContext,
                    "None of the contact selected for list subscription.",
                    Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(mContext, "List Subscribe Polling Result =" +
                    result, Toast.LENGTH_SHORT).show();
        }
    }
}
