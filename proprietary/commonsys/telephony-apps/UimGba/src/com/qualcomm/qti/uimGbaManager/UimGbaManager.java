/******************************************************************************
  @file    UimGbaManager.java
  @brief   Manager class which provides interface to communicate with uim
           GBA service.

  ---------------------------------------------------------------------------
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

package com.qualcomm.qti.uimGbaManager;

import android.util.Log;
import android.os.Message;
import android.content.Context;
import android.os.IBinder;
import android.os.Handler;
import android.os.RemoteException;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.ComponentName;

import com.qualcomm.qti.uimGba.IUimGbaService;

/**
 * This class provides UimGbaService Interface
 * for the clients.
 *
 * Client needs to instantiate this class to
 * use the IUimGbaService.aidl functionality
 */

public class UimGbaManager {
    private final String LOG_TAG = "UimGbaManager";

    private Context context;

    /* used internally to check the status of serviceConnection */
    private Boolean serviceConnected;

    private GbaManagerServiceConnection mConnection;
    private IUimGbaService gbaService = null;

    //List of handlers to store with ids
    private Handler serviceConnectionStatusHandler = null;
    private int serviceConnectionStatusId;

    /**
     * Constructor
     * @param context context in which the bindService will be
     *                initiated.
     */
    public UimGbaManager(Context context) {
        this.context = context;
        serviceConnected = false;
    }

    /**
     * Initiate connection with the service.
     *
     * @param callbackHandler Handler to which the result message
     *                        needs to be sent.
     *
     *                        Boolean object will be put
     *                        in "msg.obj" with true for
     *                        service connected and false
     *                        for service disconnected.
     *
     * @param serviceConnectionStatusId This integer id will be
     *                                  put in the "what" variable
     *                                  of the message that is sent
     *                                  to the handler i.e. "msg.what".
     *                                  It can be used as an identifier.
     *
     * @return boolean Immediate result of the operation. true if
     *                 successful.
     *                 NOTE: This does not guarantee a successful
     *                 connection. The client needs to use handler
     *                 to listen to the Result.
     *
     */
    public boolean connectService(Handler callbackHandler,
                                  int serviceConnectionStatusId) {
        serviceConnectionStatusHandler = callbackHandler;
        this.serviceConnectionStatusId = serviceConnectionStatusId;

        Log.e(LOG_TAG, "Creating uim GBA Service, if not started start");
        Intent intent = new Intent();
        intent.setComponent(new ComponentName("com.qualcomm.qti.uimGbaApp",
                                    "com.qualcomm.qti.uimGbaApp.GbaService"));
        mConnection = new GbaManagerServiceConnection();
        boolean success = context.bindService(intent, mConnection,
                                              Context.BIND_AUTO_CREATE);
        Log.e(LOG_TAG, "bind Service result: " + success);
        return success;
    }

    /**
     * Disconnect the connection with the Service.
     *
     */
    public void disconnectService() {
        Log.i(LOG_TAG, "release uim GBA Service");
        context.unbindService(mConnection);
        mConnection = null;
    }

    /**
     * registerCallback will be used by uim GBA client
     * to register a callback to be notified asynchronously
     *
     * @param callback
     *    Defines the callback interface
     *
     * @return
     *    SUCCESS 0
     *       Successfully registered call back with uim GBA service.
     *    INTERNAL_FAILURE 8
     *       Failed to register callback due to internal error.
     */
    public int registerCallback(UimGbaManagerCallback callback) {
        int ret = GbaStatus.UIM_GBA_SUCCESS;

        if(!serviceConnected){
            Log.e(LOG_TAG, "service not connected!");
            return GbaStatus.SERVICE_NOT_CONNECTED;
        }

        Log.i(LOG_TAG, "registerCallback");

        try {
            ret = gbaService.registerCallback(callback);
        } catch(RemoteException e) {
            Log.e(LOG_TAG, "registerCallback, remote exception");
            e.printStackTrace();
            ret = GbaStatus.UIM_GBA_ERROR;
        }
        return ret;
    }

    /**
     * deregisterCallback will be used by
     * uim GBA client to deregister callback
     *
     * @param callback
     *    Defines the callback interface
     *
     * @return
     *    UIM_GBA_SUCCESS 0
     *       Successfully deregistered callback with uimGba service.

     *    UIM_GBA_ERROR 1
     *       Failed to deregister call back due to internal error.
     *
     *    SERVICE_NOT_CONNECTED 2
     *       Not connected to uimGba service
     *
     */
    public int deregisterCallback(UimGbaManagerCallback callback) {
        int ret = GbaStatus.UIM_GBA_ERROR;

        if(!serviceConnected){
            Log.e(LOG_TAG, "service not connected!");
            return GbaStatus.SERVICE_NOT_CONNECTED;
        }

        Log.i(LOG_TAG, "deregisterCallback");

        try {
            ret = gbaService.deregisterCallback(callback);
        } catch(RemoteException e){

            Log.e(LOG_TAG, "deregisterCallback, remote exception");
            e.printStackTrace();
            ret = GbaStatus.UIM_GBA_ERROR;
        }

        return ret;
    }

    /**
     * gbaInitialize will be used by client to perform bootstrapping procedure(GBA)
     * to generate keys.
     *
     * @param token
     *    This is used for identification of the request.
     *
     * @param securityProtocol
     *    Security protocol identifier NULL terminated string of 5 octets.
     *    See 3GPP TS 33.220 Annex H.
     *
     * @param nafFullyQualifiedDomainName
     *    NAF fully qualified domain name with maximum size of 255 bytes.
     *
     * @param slotId
     *    Slot identifier in which SIM is inserted.
     *    Value 0(Slot 1) 1(Slot 2) 3(Slot 3)
     *
     * @param applicationType
     *    Application Type of the card application.
     *    Value 0(UNKNOWN) 1(SIM) 2(USIM) 3(RUIM) 4(CSIM) 5(ISIM)
     *
     * @param forceBootStrapping
     *    true=force bootstrapping, false=do not force bootstrapping.
     *
     *  @param apn
     *     Access point name
     *
     * @return
     *     UIM_GBA_SUCCESS         = 0;
     *     UIM_GBA_GENERIC_FAILURE = 1;
     */
    public int gbaInitialize(int token, byte[] securityProtocol, String nafFullyQualifiedDomainName,
                             int slotId, byte applicationType, boolean forceBootStrapping,
                             String apn) {

        int ret = GbaStatus.UIM_GBA_ERROR;

        if (!serviceConnected) {
            Log.e(LOG_TAG, "service not connected!");
            return GbaStatus.SERVICE_NOT_CONNECTED;
        }

        Log.i(LOG_TAG, "uimGbaInit");

        try {
            ret = gbaService.gbaInit(token, securityProtocol, nafFullyQualifiedDomainName,
                                     slotId, applicationType, forceBootStrapping, apn);

            if (ret == 0)
            {
                Log.i(LOG_TAG, "uimGbaInit return " + ret);
            }
        } catch(RemoteException e){

            Log.e(LOG_TAG, "uimGbaInit, remote exception");
            e.printStackTrace();
            ret = GbaStatus.UIM_GBA_ERROR;
        }

        return ret;
    }

    /**
     * getImpiData will be used by client to read IMPI.
     *
     * @param token
     *    This is used for identification of the request.
     *
     * @param slotId
     *    Slot identifier in which SIM is inserted.
     *    Value 0(Slot 1) 1(Slot 2) 3(Slot 3)
     *
     * @param applicationType
     *    Application Type of the card application.
     *    Value 0(UNKNOWN) 1(SIM) 2(USIM) 3(RUIM) 4(CSIM) 5(ISIM)
     *
     * @param secure
     *    true=return encrypted IMPI value, false=return plain text IMPI value
     *
     * @return
     *     UIM_GBA_SUCCESS         = 0;
     *     UIM_GBA_GENERIC_FAILURE = 1;
     */
    public int getImpiData(int token, int slotId, byte applicationType,
                           boolean secure) {
        int ret = GbaStatus.UIM_GBA_ERROR;

        if(!serviceConnected){
            Log.e(LOG_TAG, "service not connected!");
            return GbaStatus.SERVICE_NOT_CONNECTED;
        }

        Log.i(LOG_TAG, "getImpiData");

        try {
            ret = gbaService.getImpi(token, slotId, applicationType, secure);

            if (ret == 0)
            {
                Log.i(LOG_TAG, "getImpiData return " + ret);
            }
        } catch(RemoteException e){

            Log.e(LOG_TAG, "getImpiData, remote exception");
            e.printStackTrace();
            ret = GbaStatus.UIM_GBA_ERROR;
        }

        return ret;
    }

    public class GbaManagerServiceConnection implements ServiceConnection {

        public void onServiceConnected(ComponentName name, IBinder boundService) {

            Log.e(LOG_TAG, "On service connected, get the binder object");
            gbaService = IUimGbaService.Stub.asInterface((IBinder) boundService);
            Log.e(LOG_TAG, "gbaService obtained from the binder!!!!");
            serviceConnected = true;
            if(serviceConnectionStatusHandler == null) {
                Log.e(LOG_TAG, "ServiceStatusHandler is NULL");
            } else {
                Message msg = serviceConnectionStatusHandler.obtainMessage(
                                serviceConnectionStatusId, serviceConnected);
                msg.sendToTarget();
            }
        }

        public void onServiceDisconnected(ComponentName name) {
        Log.i(LOG_TAG, "onServiceDisconnected, release the binder object");
            gbaService       = null;
            serviceConnected = false;
            if(serviceConnectionStatusHandler == null){
                Log.e(LOG_TAG, "ServiceStatusHandler is NULL");
            } else {
                Message msg = serviceConnectionStatusHandler.obtainMessage
                            (serviceConnectionStatusId, serviceConnected);
                msg.sendToTarget();
            }
        }
    }

    public class GbaStatus {
        public static final int UIM_GBA_SUCCESS       = 0;
        public static final int UIM_GBA_ERROR         = 1;
        public static final int SERVICE_NOT_CONNECTED = 2;
   }
}
