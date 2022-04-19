/*=============================================================================
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.example.connmgr;

import android.util.Log;

import com.qualcomm.qti.imscmservice.V2_0.IImsCMConnectionListener;
import com.qualcomm.qti.imscmservice.V2_0.IImsCMConnection;
import com.qualcomm.qti.imscmservice.V2_0.connectionEvent;
import com.qualcomm.qti.imscmservice.V2_0.connectionEventData;
import com.qualcomm.qti.imscmservice.V2_0.incomingMessage;
import com.qualcomm.qti.imscmservice.V2_0.incomingMessageKeys;
import com.qualcomm.qti.imscmservice.V2_0.keyValuePairStringType;
import com.qualcomm.qti.imscmservice.V2_0.keyValuePairBufferType;


public class ConnectionListenerImpl extends IImsCMConnectionListener.Stub {
  final static String TAG = "UI_ConnMgr_ConnectionListenerImpl";
  int sub = 0;
  private long listenerToken = 0;

  public void setListenerToken(long listenerToken) {
    this.listenerToken = listenerToken;
  }
  public long getListenerToken() {
    return this.listenerToken;
  }
  public ConnectionListenerImpl(int activeSub) {
      sub = activeSub;
  }
  /*handleEventReceived: Callback function to inform clients about a
    registration status change, changes in service allowed by policy
    manager because of a RAT change,and any forceful terminations
    of the connection object by the QTI framework because of PDP
    status changes.
  */
  @Override
  public void onEventReceived(connectionEventData event) {
    Log.d(TAG, "onEventReceived event is "+event.toString());
    if(event.eEvent == connectionEvent.SERVICE_REGISTERED) {
      Log.d(TAG, "onEventReceived ConnectionSm is initialized: on Sub" + sub);
      ConnectionGlobalData.connSmInitialisedArray[sub] = 1;
    }
  }

  /*handleIncomingMessage: This callback indicates the incoming message to the client.*/
  @Override
  public void handleIncomingMessage(incomingMessage data) {
    Log.d(TAG, "handleIncomingMessage: on Sub" + sub);
    if(data.data.size() > 0) {
      for(keyValuePairStringType var : data.data) {
        Log.d(TAG, "handleIncomingMessage: key[" + incomingMessageKeys.toString(var.key) +
                    "] Value[" + var.value +"]");
      }
    }
    else {
      Log.d(TAG, "handleIncomingMessage: no String Type data received");
    }

    if(data.bufferData.size() > 0) {
      for(keyValuePairBufferType var : data.bufferData) {
        String sipMessage = "";
        for(int i=0; i< var.value.size(); i++)
        {
          sipMessage = sipMessage + var.value.get(i).toString();
        }
        Log.d(TAG, "handleIncomingMessage: key[" + incomingMessageKeys.toString(var.key) +
        "] Value[" + sipMessage +"]");
      }
    }
    else {
      Log.d(TAG, "handleIncomingMessage: no Buffer Type data received");
    }

  }

  /*onCommandStatus: Status of the sendMessage
    (whether or not the message was transmitted to network)
    is returned asynchronously via
    the onCommandStatus callback with messageID as a parameter.
  */
  @Override
  public void onCommandStatus(int status, int userdata) {
    for(int i=0; i<=1; i++) {
       if(ConnectionGlobalData.userDataArray[i] == userdata) {
           Log.d(TAG, "onCommandStatus: on Sub" + i);
           Log.d(TAG, "onCommandStatus status is "+status + "userdata " + userdata);
       }
    }
  }
}
