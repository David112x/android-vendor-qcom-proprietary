/*
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.dualstaapp;

import vendor.qti.data.factory.V2_1.IFactory;
import vendor.qti.data.slm.V1_0.ISlmToken;
import vendor.qti.data.slm.V1_0.ISlmService;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.os.Bundle;
import android.app.Activity;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.os.RemoteException;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.NoSuchElementException;
import java.util.Scanner;

import static android.net.ConnectivityManager.TYPE_WIFI;
import static android.net.NetworkCapabilities.NET_CAPABILITY_INTERNET ;
import static android.net.NetworkCapabilities.NET_CAPABILITY_OEM_PAID;
import static android.net.NetworkCapabilities.NET_CAPABILITY_VALIDATED;
import static android.net.NetworkCapabilities.TRANSPORT_WIFI;

public class MainActivity extends Activity {

    private final String TAG = "DualSTA";
    private String appName = "DualSTA";

    final Context context = this;

    Button requestNetworkButton, registerNetworkButton, enableSlmButton, disableSlmButton, disableSlmforAllUidsButton;

    private NetworkRequest mOemWifiRequest;
    private ConnectivityManager cm;

    private IFactory mFactory;
    private ISlmService mSlmService;

    private static long testNetworkHandle;

    public Network getNetwork() {
        return Network.fromNetworkHandle(testNetworkHandle);
    }

    private NetworkCallback mWifiOemNetworkCallback = new NetworkCallback() {

        @Override
        public void onAvailable(Network network) {
            testNetworkHandle = network.getNetworkHandle();
            Log.e(TAG, "mWifiOemNetworkCallback network available: cellNetId " + testNetworkHandle);
            Toast.makeText(getApplicationContext(),"mWifiOemNetworkCallback network available: cellNetId "
                    + testNetworkHandle, Toast.LENGTH_SHORT).show();
        }

        @Override
        public void onCapabilitiesChanged(Network network, NetworkCapabilities networkCapabilities) {
            if (network.equals(getNetwork()) && networkCapabilities.hasCapability(NET_CAPABILITY_VALIDATED)) {
                Log.e(TAG, "mWifiOemNetworkCallback network validated: cellNetId " + testNetworkHandle);
                Toast.makeText(getApplicationContext(),"mWifiOemNetworkCallback network validated: cellNetId "
                    + testNetworkHandle, Toast.LENGTH_SHORT).show();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.MainActivity);

        requestNetworkButton = (Button) findViewById(R.id.requestNetwork);
        registerNetworkButton = (Button) findViewById(R.id.registerNetwork);
        enableSlmButton = (Button) findViewById(R.id.enableSlm);
        disableSlmButton = (Button) findViewById(R.id.disableSlm);
        disableSlmforAllUidsButton = (Button) findViewById(R.id.disableSlmforAllUids);

        mSlmService = null;

        mOemWifiRequest = new NetworkRequest.Builder()
                .addCapability(NET_CAPABILITY_OEM_PAID)
                .addTransportType(TRANSPORT_WIFI)
                .build();

        cm = (ConnectivityManager)getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);

        requestNetworkButton.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                Log.e(TAG, "requestNetwork called");
                cm.requestNetwork(mOemWifiRequest , mWifiOemNetworkCallback);
            }
        });

        registerNetworkButton.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                final CbResults results = new CbResults();
                try {
                    Log.e(TAG, "Call Ifactory getService");
                    mFactory = IFactory.getService();
                } catch (RemoteException | NoSuchElementException e) {
                    Log.e(TAG, "cne factory not supported: " + e);
                }

                if (mFactory == null) {
                    Log.e(TAG, "cne IFactory.getService() returned null");
                        return;
                }
                try {
                    Log.e(TAG, "Call createISlmService");
                    mFactory.createISlmService(
                            new ISlmToken.Stub() {},
                            (int status, ISlmService service) -> {
                                results.status = status;
                                results.service = service;
                    });
                    mSlmService = results.service;
                    Log.e(TAG, "createISlmService success");
                } catch (RemoteException e) {
                    Log.e(TAG, "Call createISlmService but in catch");
                }
            }
        });

        enableSlmButton.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {

                LayoutInflater layoutInflater = LayoutInflater.from(context);
                View enableSlmPromptView = layoutInflater.inflate(R.layout.EnableSlmPrompt, null);

                AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(context);
                alertDialogBuilder.setView(enableSlmPromptView);

                final EditText pref1UidsEditText = (EditText) enableSlmPromptView.findViewById(R.id.pref1Uids);
                final EditText pref2UidsEditText = (EditText) enableSlmPromptView.findViewById(R.id.pref2Uids);

                alertDialogBuilder
                    .setCancelable(false)
                    .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            ArrayList<Integer> pref1_uids, pref2_uids;
                            pref1_uids = new ArrayList<Integer> ();
                            pref2_uids = new ArrayList<Integer> ();
                            Integer pref1 = new Integer(1);
                            Integer pref2 = new Integer(2);

                            Scanner scPref1 = new Scanner(pref1UidsEditText.getText().toString());
                            while(scPref1.hasNextInt()){
                                pref1_uids.add(scPref1.nextInt());
                            }

                            Scanner scPref2 = new Scanner(pref2UidsEditText.getText().toString());
                            while(scPref2.hasNextInt()){
                                pref2_uids.add(scPref2.nextInt());
                            }

                            try {
                                if (mSlmService != null) {
                                    mSlmService.enableSlm(
                                        pref1_uids, pref1.byteValue());
                                    mSlmService.enableSlm(
                                        pref2_uids, pref2.byteValue());
                                }
                                else {
                                    Log.e(TAG, "Please register Wifi OEM network first");
                                }
                            } catch (RemoteException e) {
                                Log.e(TAG, "enableSlm in catch");
                            }
                        }
                    })
                    .setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            dialog.cancel();
                        }
                    });

                    AlertDialog alertEnableSlm = alertDialogBuilder.create();
                    alertEnableSlm.show();
            }
        });

        disableSlmButton.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {

                LayoutInflater layoutInflater = LayoutInflater.from(context);
                View disableSlmPromptView = layoutInflater.inflate(R.layout.DisableSlmPrompt, null);

                AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(context);
                alertDialogBuilder.setView(disableSlmPromptView);

                final EditText uidsEditText = (EditText) disableSlmPromptView.findViewById(R.id.uids);

                alertDialogBuilder
                    .setCancelable(false)
                    .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            ArrayList<Integer> uids;
                            uids = new ArrayList<Integer> ();

                            Scanner sc = new Scanner(uidsEditText.getText().toString());
                            while(sc.hasNextInt()){
                                uids.add(sc.nextInt());
                            }

                            try {
                                if (mSlmService != null) {
                                    mSlmService.disableSlm(uids);
                                }
                                else {
                                    Log.e(TAG, "Please register Wifi OEM network first");
                                }
                            } catch (RemoteException e) {
                                Log.e(TAG, "disableSlm in catch");
                            }
                        }
                    })
                    .setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            dialog.cancel();
                        }
                    });

                    AlertDialog alertDisableSlm = alertDialogBuilder.create();
                    alertDisableSlm.show();
            }
        });

        disableSlmforAllUidsButton.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                try {
                    if (mSlmService != null) {
                        mSlmService.disableSlmforAllUids();
                    }
                    else {
                        Log.e(TAG, "Please register Wifi OEM network first");
                    }
                } catch (RemoteException e) {
                    Log.e(TAG, "disableSlmforAllUids in catch");
                }
            }
        });

    }

    private static class CbResults {
        int status;
        ISlmService service;
    }
}
