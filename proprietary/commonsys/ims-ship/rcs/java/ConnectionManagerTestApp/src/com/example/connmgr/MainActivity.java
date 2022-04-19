/*=============================================================================
  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.example.connmgr;

import java.util.*;
import java.text.SimpleDateFormat;
import java.text.*;

import java.net.InetAddress;
import java.net.Socket;
import javax.net.SocketFactory;

import android.net.ConnectivityManager;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.os.AsyncTask;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.widget.Button;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;

import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionInfo;

import com.qualcomm.qti.imscmservice.V2_2.IImsCmService;
import com.qualcomm.qti.imscmservice.V2_2.IImsCmServiceListener;
import com.qualcomm.qti.imscmservice.V2_0.IImsCMConnectionListener;
import com.qualcomm.qti.imscmservice.V2_0.IImsCMConnection;
import com.qualcomm.qti.imscmservice.V2_0.outgoingMessage;
import com.qualcomm.qti.imscmservice.V2_0.outgoingMessageKeys;
import com.qualcomm.qti.imscmservice.V2_0.messageType;
import com.qualcomm.qti.imscmservice.V2_0.sipProtocolType;
import com.qualcomm.qti.imscmservice.V2_0.userConfig;
import com.qualcomm.qti.imscmservice.V2_0.userConfigKeys;
import com.qualcomm.qti.imscmservice.V2_0.deviceConfig;
import com.qualcomm.qti.imscmservice.V2_2.deviceConfigKeys;
import com.qualcomm.qti.imscmservice.V2_0.configData;
import com.qualcomm.qti.imscmservice.V2_0.autoConfig;
import com.qualcomm.qti.imscmservice.V2_0.keyValuePairBufferType;
import com.qualcomm.qti.imscmservice.V2_0.keyValuePairStringType;
import com.qualcomm.qti.imscmservice.V2_0.methodResponseData;
import com.qualcomm.qti.imscmservice.V2_0.methodResponseDataKeys;
import android.widget.Toast;

import com.example.connmgr.ConnectionGlobalData;



public class MainActivity extends Activity {
    final static String TAG = "UI_ConnMgr_MainActivity";
    Button button;
    Button buttonCreatecon;
    static int nSetStatus = 1;
    private String requestNwAction = "android.cne.action.REQUEST_NETWORK";

    private String mIccId = null;
    private int status=0;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initButtons();
    }

    private void getDefaultIccId() {
        //check if this is multiSIM.
        SubscriptionManager subMgr =  SubscriptionManager.from(getApplicationContext());
        if(subMgr != null){
        List<SubscriptionInfo> subInfoList = subMgr.getActiveSubscriptionInfoList();
        if(subInfoList != null){
        if (subInfoList.size() > 1) {
            Log.i(TAG, "RCSStartRCSService Detected MultiSim");
            //return;
        }
        mIccId = subInfoList.get(0).getIccId();
        Log.i(TAG, "getDefaultIccId default mIccId [" + mIccId +"]");
        }
        else {
        Log.i(TAG, "subinfolist is null, SIM is not inserted");
        }
        }
        else {
        Log.i(TAG, "subMgr is null");
        }
    }

    public void initButtons()
    {
        getDefaultIccId();
        initCreateConnectionManagerButton();
        initActivateSub0Button();
        initActivateSub1Button();
        initCreateSMConnectionButton();
        initCreateIMConnectionButton();
        initCreateInvalidConnectionButton();
        initTriggerRegistrationButton();
        initTriggerDeRegistrationButton();
        initSendSMMessageButton();
        initSendIMMessageButton();
        initCloseSMConnectionButton();
        initCloseIMConnectionButton();
        initCloseConnectionManagerButton();
        initGetConfigurationManagerButton();
        initMethodResponseButton();
        initRegRestorationButton();
        initTestSendingPacketsViaSocket();
        initAutoconfigButton();
        initsetStatusButton();
        initsendMsgButton();
        initTriggerAutoConfigButton();
    }
    private void initsendMsgButton() {
    // TODO Auto-generated method stub
    Button sendMsgButton = (Button) findViewById(R.id.sendMsg);
    handlesendMsgButtonClick(sendMsgButton);
    }

    private void initAutoconfigButton() {
    Button autoconfigButton = (Button) findViewById(R.id.autoconfig);
    handleAutoconfigButtonClick(autoconfigButton);
    }

    private void initsetStatusButton() {
    // TODO Auto-generated method stub
    Button setStatusButton = (Button) findViewById(R.id.setStatus);
    handlesetStatusButtonClick(setStatusButton);
    }

    private void handlesendMsgButtonClick(Button sendMsgButton) {
    sendMsgButton.setOnClickListener(new OnClickListener() {
    @Override
      public void onClick(View arg0) {
        Log.d(TAG, "sendMsgButton click");
        try{
            if (ConnectionGlobalData.connSmMap.get(ConnectionGlobalData.activeWorkingSub).connection != null )
            {
               if ( ConnectionGlobalData.connMgrInitialised == 1 &&
                    ConnectionGlobalData.connSmInitialisedArray[ConnectionGlobalData.activeWorkingSub] == 1)
               {
                   int iRetValue ;
                   Log.d(TAG, "sendmsg");
                   /*Fill in the message related params listed below*/
                   String pstringMsg = "MESSAGE sip:310008984630745@ims.vz.net "+
                                       "SIP/2.0\r\n"+
                                       "From: \"TestUE1\" "+
                                       "<sip:11111@ims.cingularme.com>;"+
                                       "tag=3476455352\r\n"+
                                       "To: <sip:11111@ims.cingularme.com>\r\n"+
                                       "CSeq: 255229831 MESSAGE\r\n"+
                                       "Call-ID: 3476455304_1570329456@10.242.114.221\r\n"+
                                       "Via: SIP/2.0/UDP 10.242.114.221:5060;"+
                                       "branch=z9hG4bK504163233\r\n"+
                                       "Max-Forwards: 70\r\n"+
                                       "Route: <sip:10.252.42.41:5060;lr>\r\n"+
                                       "P-Access-Network-Info: 3GPP-E-UTRAN-FDD; "+
                                       "utran-cell-id-3gpp=3114800000011C000\r\n"+
                                       "Accept-Contact: *;+g.3gpp.iari-ref=\""+
                                       "urn:urn-7:3gpp-service.ims.icsi.oma.cpm.msg\"\r\n"+
                                       "Content-Type: text/plain\r\n"+
                                       "Expires: 2345\r\nContent"+
                                       "-Length: 8\r\n\r\nSM_MESSAGE\r\n\r\n";

                   outgoingMessage msg= new outgoingMessage();

                   {
                       // the SIP packet is a of buffer type
                       keyValuePairBufferType bufferType = new keyValuePairBufferType();
                       bufferType.key = outgoingMessageKeys.Message;
                       byte[] byteArray = pstringMsg.getBytes();
                       for(int i = 0; i < byteArray.length; i++)
                       {
                        bufferType.value.add(byteArray[i]);
                        }
                        msg.bufferData.add(bufferType);
                   }
                   /* Refers to the SIP request/response call ID value as per RFC 3261 */
                   keyValuePairStringType callIdType = new keyValuePairStringType();
                   callIdType.key = outgoingMessageKeys.CallId;
                   callIdType.value = "3476455304_1570329456@10.242.114.221";
                   msg.data.add(callIdType);


                   /*Message destination;
                     for SIP requests, an outbound proxy address can be specified;
                     for SIP responses, a header address can be used
                   */
                   keyValuePairStringType outboundProxyType = new keyValuePairStringType();
                   outboundProxyType.key = outgoingMessageKeys.OutboundProxy;
                   outboundProxyType.value = "10.242.166.1";
                   msg.data.add(outboundProxyType);

                   /* protocol can be TCP,UDP,NONE */
                   keyValuePairStringType protocolType = new keyValuePairStringType();
                   protocolType.key = outgoingMessageKeys.Protocol;
                   protocolType.value = Integer.toString(sipProtocolType.UDP);
                   msg.data.add(protocolType);

                   keyValuePairStringType remotePortType = new keyValuePairStringType();
                   remotePortType.key = outgoingMessageKeys.RemotePort;
                   remotePortType.value = Integer.toString(5060);
                   msg.data.add(remotePortType);

                   /* Message Type : REQUEST,RESPONSE,NONE */
                   keyValuePairStringType msgType = new keyValuePairStringType();
                   msgType.key = outgoingMessageKeys.MessageType;
                   msgType.value = Integer.toString(messageType.TYPE_REQUEST);
                   msg.data.add(msgType);

                   /* 0 -request ; 1-response; 2-invalidmax*/
                   iRetValue=ConnectionGlobalData.connSmMap.get(
                       ConnectionGlobalData.activeWorkingSub).connection.sendMessage(
                           msg,
                           ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);

                   Log.d(TAG, "sendMsgButton RetValue "+ iRetValue);
               }
               else
               {
                   Log.d(TAG, "setStatus Connection Manager/ SM connection is not REGISTERED ");
               }
            }
            else
            {
               Log.d(TAG, " setStatus Connection Manager/ SM connection is NULL");
            }
        }catch (Exception e){
            Log.d(TAG, "Exception raised" + e);
        }
    }});
    }

    private void handleAutoconfigButtonClick(Button autoconfigButton) {
        autoconfigButton.setOnClickListener(new OnClickListener() {
        @Override
        public void onClick(View arg0) {
            Log.d(TAG, "autoconfigButton click");
            try
            {

                if (ConnectionGlobalData.connMgrInitialised == 1)
                {
                  Log.d(TAG, "handleAutoconfigButtonClick : Calling Autoconfig: iccid "
                        + ConnectionGlobalData.activeWorkingSub);
                  /* autoConfigType = IMSCM_ConfigType.IMSCM_AUTO_CONFIG */
                  int status;
                  if(ConnectionGlobalData.connMgr != null)
                  {
                    status = ConnectionGlobalData.connMgr.getConfiguration(
                                    ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                    ConnectionGlobalData.autoConfigType,
                                    ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                  }
                  else if(ConnectionGlobalData.oldConnMgrV2_1 != null)
                  {
                    status = ConnectionGlobalData.oldConnMgrV2_1.getConfiguration(
                                    ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                    ConnectionGlobalData.autoConfigType,
                                    ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                  }
                  else
                  {
                    status = ConnectionGlobalData.oldConnMgrV2_0.getConfiguration(
                                    ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                    ConnectionGlobalData.autoConfigType,
                                    ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                  }
                }
                else
                {
                  Log.d(TAG, "iccid " + ConnectionGlobalData.activeWorkingSub +
                        " :handleAutoconfigButtonClick error connection manager is not initilised");
                }
            } catch (Exception e){
               Log.d(TAG, "Exception raised" + e);
            }
        }});
    }

    private void handlesetStatusButtonClick(Button setStatusButton) {
        setStatusButton.setOnClickListener(new OnClickListener() {

        @Override
        public void onClick(View arg0) {
            Log.d(TAG, "setStatusButton click : iccid "+ ConnectionGlobalData.activeWorkingSub);
            try {
                if (ConnectionGlobalData.connSmMap.get(ConnectionGlobalData.activeWorkingSub) != null )
                {
                   if ( ConnectionGlobalData.connMgrInitialised == 1 )
                   {
                       int iRetValue =0;
                       Log.d(TAG, "iccid" + ConnectionGlobalData.activeWorkingSub);
                       Log.d(TAG, ":setStatus = " + nSetStatus);
                       /*Sets the current status of the service(feature tag of the service),
                         which is session based and is using the current connection.
                         0 means none ; 1 means active ;2 means hold ; 3 means inactive
                       */
                       iRetValue=ConnectionGlobalData.connSmMap.get(
                           ConnectionGlobalData.activeWorkingSub).connection.setStatus("+g.3gpp.iari-ref=\"urn:"+
                                                                       "urn-7:3gpp-service."+
                                                                       "ims.icsi.oma.cpm.msg\"",
                                                                       nSetStatus);
                       if( nSetStatus == 1 )
                           nSetStatus = 3;
                       else
                           nSetStatus = 1;
                       Log.d(TAG, "setstatus RetValue "+ iRetValue);
                   }
                   else
                   {
                       Log.d(TAG, "setStatus Connection Manager/ SM connection "+
                                  "is not REGISTERED 222");
                   }
               }
               else
               {
                   Log.d(TAG, " setStatus Connection Manager/ SM connection is NULL");
               }
           } catch (Exception e) {
               Log.d(TAG, "Exception raised" + e);
           }
        }});
    }

    private void initRegRestorationButton() {
        // TODO Auto-generated method stub
        Button connMgrButton = (Button) findViewById(R.id.regRestoration);
        //handleRegRestorationButtonClick(connMgrButton);
    }

/*
    private void handleRegRestorationButtonClick(Button connMgrButton) {
      connMgrButton.setOnClickListener(new OnClickListener() {
      @Override
      public void onClick(View arg0) {
        Log.d(TAG, "handleRegRestorationButtonClick click");
          if (ConnectionGlobalData.connMgr!= null)
          {
           if (ConnectionGlobalData.connMgrInitialised == 1)
           {
             int iRetValue ;
             Log.d(TAG, "RegRestoration");
             //iRetValue = ConnectionGlobalData.connMgr.RegRestoration("LTE", 200);
             //Call the function with method and responce code
             iRetValue = ConnectionGlobalData.connMgr.regRestoration("INVITE", 403);
             Log.d(TAG, "RegRestoration RetValue "+ iRetValue);
           }
           else
           {
             Log.d(TAG, "create IM connection error connection manager is not initilised");
           }
          } else {
           Log.d(TAG, "handleRegRestorationButtonClick ConnectionGlobalData.connMgr is NULL");
          }
      }});
  }*/

    private void initCreateConnectionManagerButton()
    {
        Button connMgrButton = (Button) findViewById(R.id.CreateManager);
        handleCreateConnectionManagerButtonClick(connMgrButton);
    }

    private void initActivateSub0Button()
    {
        Button connMgrButton = (Button) findViewById(R.id.ActivateSub0);
        handleActivateSub0ButtonClick(connMgrButton);
    }

    private void initActivateSub1Button()
    {
        Button connMgrButton = (Button) findViewById(R.id.ActivateSub1);
        handleActivateSub1ButtonClick(connMgrButton);
    }

    private void initCreateSMConnectionButton()
    {
        Button connButton = (Button) findViewById(R.id.CreateSMConn);
        handleCreateSMConnectionButtonClick(connButton);
    }

    private void initCreateIMConnectionButton()
    {
        Button connButton = (Button) findViewById(R.id.CreateIMConn);
        handleCreateIMConnectionButtonClick(connButton);
    }

    private void initCreateInvalidConnectionButton()
    {
        Button connButton = (Button) findViewById(R.id.CreateInvalidConn);
        handleCreateInvalidConnectionButtonClick(connButton);
    }

    private void initTriggerRegistrationButton()
    {
        Button connButton = (Button) findViewById(R.id.TriggerRegistration);
        handleTriggerRegistrationButtonClick(connButton);
    }

    private void initTriggerDeRegistrationButton()
    {
        Button connButton = (Button) findViewById(R.id.TriggerDeRegistration);
        handleTriggerDeRegistrationButtonClick(connButton);
    }

    private void initSendSMMessageButton()
    {
        Button connButton = (Button) findViewById(R.id.SendSMMessage);
        handleSendSMMessageButtonClick(connButton);
    }

    private void initSendIMMessageButton()
    {
        Button connButton = (Button) findViewById(R.id.SendIMMessage);
        handleSendIMMessageButtonClick(connButton);
    }

    private void initCloseSMConnectionButton()
    {
        Button connButton = (Button) findViewById(R.id.CloseSMConn);
        handleCloseSMConnectionButtonClick(connButton);
    }

    private void initCloseIMConnectionButton()
    {
        Button connButton = (Button) findViewById(R.id.CloseIMConn);
        handleCloseIMConnectionButtonClick(connButton);
    }

    private void initCloseConnectionManagerButton()
    {
        Button connMgrButton = (Button) findViewById(R.id.CloseManager);
        handleCloseConnectionManagerButtonClick(connMgrButton);
    }

    private void initGetConfigurationManagerButton()
    {
        Button getConfigurationButton = (Button) findViewById(R.id.GetConfiguration);
        handleGetConfigurationButtonClick(getConfigurationButton);
    }

    private void initMethodResponseButton()
    {
        Button methodResponseButton = (Button) findViewById(R.id.MethodResponse);
        handleMethodResponseButtonClick(methodResponseButton);
    }

    private void initTestSendingPacketsViaSocket()
    {
        Button socketSendButton = (Button) findViewById(R.id.socketSend);
        handleTestSendingPacketsViaSocket(socketSendButton);
    }
    private void initTriggerAutoConfigButton()
    {
        Button triggerAcsButton = (Button) findViewById(R.id.triggerAcsButton);
        handleTriggerAcsButtonClick(triggerAcsButton);
    }
    private void handleTestSendingPacketsViaSocket(Button socketSendButton)
    {
        socketSendButton.setOnClickListener(new OnClickListener() {
           public void onClick(View v) {
               Log.d(TAG, "handleTestSendingPacketsViaSocket entered ");
               /*click getConfigurationbutton before sending packets via socket and
                 assign value directly from onConfigChange Callback
                 Callback populates: IMSCM_USER_CONFIG,
                                     IMSCM_DEVICE_CONFIG,
                                     IMSCM_AUTO_CONFIG
               */
               String localHost = ConnectionGlobalData.userConfigData.get(userConfigKeys.LocalHostIPAddress);

               int localport = 54321;
               String packet = "Data Packet to be sent to remote.";
               Log.d(TAG, "localHost Ip address " + localHost);
               /*Got from the SIP Message. -- for testing use ice server address */
               String remoteIP = "10.242.166.1";
               int remoteport = 60450;  // Got from the SIP Message.
               //InetAddress remoteInetIp = InetAddress.getByName(remoteIP);
               //byte[] bytes = remoteInetIp.getAddress();
               ConnectivityManager mConnectivityMgr;
               try{
                   if( ConnectionGlobalData.mContext == null)
                   {
                       Log.d(TAG, "ConnectionGlobalData.mContext is null");
                   }
                   mConnectivityMgr =
                       (ConnectivityManager) ConnectionGlobalData.mContext.getSystemService(
                                                                     Context.CONNECTIVITY_SERVICE);
                   if( mConnectivityMgr != null){
                        /* if(! ((Object) mConnectivityMgr)
                           .requestRouteToHostAddress(
                                     ConnectivityManager.TYPE_MOBILE,
                                     InetAddress.getByName(remoteIP)) )
                        {
                            Log.d(TAG, "Cannot establish route to " +
                                       InetAddress.getByName(remoteIP));
                        }
                        else
                        {
                            Log.d(TAG, "Route Established to " +
                                       InetAddress.getByName(remoteIP));
                        }*/

                       Log.d(TAG, "Creating and sending message to remoteIP " +
                             InetAddress.getByName(remoteIP) +
                             " from host "+
                             InetAddress.getByName(localHost));

                       Socket soc = SocketFactory.getDefault()
                                               .createSocket(InetAddress.getByName(remoteIP),
                                                             remoteport,
                                                             InetAddress.getByName(localHost),
                                                             localport);
                       soc.getOutputStream().write(packet.getBytes());
                   }
                   else
                   {
                       Log.d(TAG, "Connectivity Manager is NULL");
                   }
               }catch(Exception ex)
               {
                   ex.printStackTrace();
               }
           }});
    }

    private void handleGetConfigurationButtonClick(Button getConfigurationButton)
    {
        getConfigurationButton.setOnClickListener(new OnClickListener() {
           public void onClick(View v) {
               try {
                   /* returns asynchronous callback onConfigChange in the listener */
                   int status;
                   if(ConnectionGlobalData.connMgr != null){
                   status =  ConnectionGlobalData.connMgr.getConfiguration(
                                                               ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                                               ConnectionGlobalData.configType,
                                                               ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                   }
                   else if(ConnectionGlobalData.oldConnMgrV2_1 != null)
                   {
                    status =  ConnectionGlobalData.oldConnMgrV2_1.getConfiguration(
                                                                ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                                                ConnectionGlobalData.configType,
                                                                ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                   }
                   else
                   {
                    status =  ConnectionGlobalData.oldConnMgrV2_0.getConfiguration(
                                                               ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                                               ConnectionGlobalData.configType,
                                                               ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                   }
                   Log.d(TAG, "ImsCmServiceImpl.getConfiguration called: iccid:"+
                         ConnectionGlobalData.activeWorkingSub);
               }catch(Exception e){
                   Log.d(TAG, "Exception raised" + e);
               }
           }});
    }

    private void handleMethodResponseButtonClick(Button methodResponseButton)
    {
        methodResponseButton.setOnClickListener(new OnClickListener() {
          public void onClick(View v) {
            try {
              if (ConnectionGlobalData.connMgrInitialised == 1){
                  int iRetValue;
                  Log.d(TAG, "MethodResponse: iccid: " +
                                 ConnectionGlobalData.activeWorkingSub);
                  methodResponseData data = new methodResponseData();
                  keyValuePairStringType methodType = new keyValuePairStringType();
                  methodType.key = methodResponseDataKeys.method;
                  methodType.value = "INVITE";
                  data.data.add(methodType);

                  keyValuePairStringType responseCodeType = new keyValuePairStringType();
                  responseCodeType.key = methodResponseDataKeys.responseCode;
                  responseCodeType.value = Integer.toString(ConnectionGlobalData.responseCode);
                  data.data.add(responseCodeType);
                  if(ConnectionGlobalData.connMgr != null){
                  iRetValue = ConnectionGlobalData.connMgr.methodResponse(
                                                           ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                                           data,
                                                           ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                  }
                  else if(ConnectionGlobalData.oldConnMgrV2_1 != null){
                  iRetValue = ConnectionGlobalData.oldConnMgrV2_1.methodResponse(
                                                            ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                                            data,
                                                            ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                  }
                  else{
                  iRetValue = ConnectionGlobalData.oldConnMgrV2_0.methodResponse(
                                                           ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                                           data,
                                                           ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                  }
                  Log.d(TAG, "MethodResponse RetValue "+ iRetValue);

              } else
              {
                 Log.d(TAG, "create MethodResponse error connection manager "+
                            "is not initilised : iccid: " +
                            ConnectionGlobalData.activeWorkingSub);
              }
            } catch (Exception e) {
                Log.d(TAG, "Exception raised" + e);
            }
          }});
    }

    private void handleCreateConnectionManagerButtonClick(Button connMgrButton) {
        connMgrButton.setOnClickListener(new OnClickListener() {
           public void onClick(View v) {
               try 
               {
                 /*startService---- Starts the native Android service */
                 try
                 {
                   ConnectionGlobalData.connMgr = IImsCmService.getService("qti.ims.connectionmanagerservice");
                 }
                 catch(Exception e)
                 {
                   Log.d(TAG, "ImsCmServiceImpl.InitializeService old V_2_1 modem, fallback to previousclient");
                   ConnectionGlobalData.connMgr = null;
                   try
                   {
                    ConnectionGlobalData.oldConnMgrV2_1 = com.qualcomm.qti.imscmservice.V2_1.IImsCmService.getService(
                                                                                   "qti.ims.connectionmanagerservice");
                   } catch(Exception ex)
                   {
                     Log.d(TAG, "ImsCmServiceImpl.InitializeService old V_2_0 modem, fallback to oldclient");
                     ConnectionGlobalData.oldConnMgrV2_1 = null;
                     ConnectionGlobalData.oldConnMgrV2_0 = com.qualcomm.qti.imscmservice.V2_0.IImsCmService.getService(
                                                                                    "qti.ims.connectionmanagerservice"); 
                   }
                  }
                  if(ConnectionGlobalData.connMgr !=null){
                   Log.d(TAG, "ImsCmServiceImpl.InitializeService called newclient V2_2");
                   ConnectionGlobalData.currVersion = ConnectionGlobalData.newClient;
                  }
                  else if(ConnectionGlobalData.oldConnMgrV2_1 !=null){
                   Log.d(TAG, "ImsCmServiceImpl.InitializeService called oldclient V2_1");
                   ConnectionGlobalData.currVersion = ConnectionGlobalData.oldClientV2_1;
                  }
                  else{
                   Log.d(TAG, "ImsCmServiceImpl.InitializeService called oldclient V2_0");
                   ConnectionGlobalData.currVersion = ConnectionGlobalData.oldClientV2_0;
                  }
                  Log.d(TAG, "ImsCmServiceImpl.InitializeService called");
                  if(ConnectionGlobalData.oldConnMgrV2_0 == null &&
                     ConnectionGlobalData.oldConnMgrV2_1 == null &&
                     ConnectionGlobalData.connMgr ==null){
                   Log.d(TAG, "ERROR: ImsCmServiceImpl.InitializeService called | oldclients V2_0,V2_1,newclient are null, return");
                   //Toast.makeText(getActivity().getContext(),"GetService fails, please try again!!",Toast.LENGTH_SHORT).show();
                   return;
                  }
                  SubscriptionManager subMgr =  SubscriptionManager.from(getApplicationContext());
                  //create CM for all active subs
                  if(subMgr != null){
                  List<SubscriptionInfo> subInfoList = subMgr.getActiveSubscriptionInfoList();
                  if(subInfoList != null){
                  for( int i=0; i < subInfoList.size(); i++) {
                      mIccId = subInfoList.get(i).getIccId();
                      if(ConnectionGlobalData.currVersion == ConnectionGlobalData.newClient &&
                         ConnectionGlobalData.connMgr != null){
                      ConnectionManagerListenerImpl connMgrListener = new ConnectionManagerListenerImpl();
                      ConnectionGlobalData.connMgrListenerMap.put(i, connMgrListener);
                      /* calls onServiceReady Callback and populates serviceHandle */
                       //inal int initStatus;
                      ConnectionGlobalData.connMgr.InitializeService_2_2(
                                                              mIccId,
                                                              connMgrListener,
                                                              ConnectionGlobalData.userDataArray[i],
                                                              (int status, long listenerId) -> {
                                                                   setCmServiceStatus(status);
                                                                   connMgrListener.setListenerId(listenerId);
                                                              });
                      Log.d(TAG+mIccId, "ImsCmServiceImpl connMgr status: " + getCmServiceStatus());
                      }
                      if(getCmServiceStatus() == ConnectionGlobalData.unsupportedStatusCode)
                      {
			           ConnectionGlobalData.connMgr = null;
			           ConnectionGlobalData.oldConnMgrV2_1 = com.qualcomm.qti.imscmservice.V2_1.IImsCmService.getService(
                                                                                       "qti.ims.connectionmanagerservice");
			           ConnectionGlobalData.currVersion = ConnectionGlobalData.oldClientV2_1;
		              }
                      if(ConnectionGlobalData.currVersion == ConnectionGlobalData.oldClientV2_1 &&
                         ConnectionGlobalData.oldConnMgrV2_1 != null)
                      {
                        ConnectionManagerListenerImpl_2_1 connMgrListener_2_1 = new ConnectionManagerListenerImpl_2_1();
                        ConnectionGlobalData.connMgrListenerMap_2_1.put(i, connMgrListener_2_1);
                        ConnectionGlobalData.oldConnMgrV2_1.InitializeService_2_1(
                                                              mIccId,
                                                              connMgrListener_2_1,
                                                              ConnectionGlobalData.userDataArray[i],
                                                              (int status, long listenerId) -> {
                                                                    setCmServiceStatus(status);
                                                                    connMgrListener_2_1.setListenerId(listenerId);
                                                              });
                        Log.d(TAG+mIccId, "ImsCmServiceImpl connMgrListener_2_1 connMgr status: " + getCmServiceStatus());
                      }
                      if(getCmServiceStatus() == ConnectionGlobalData.unsupportedStatusCode)
		              {
			            ConnectionGlobalData.oldConnMgrV2_1 = null;
			            ConnectionGlobalData.oldConnMgrV2_0 = com.qualcomm.qti.imscmservice.V2_0.IImsCmService.getService(
                                                                                       "qti.ims.connectionmanagerservice");
			            ConnectionGlobalData.currVersion = ConnectionGlobalData.oldClientV2_0;
		              }
                      if(ConnectionGlobalData.currVersion == ConnectionGlobalData.oldClientV2_0 &&
                         ConnectionGlobalData.oldConnMgrV2_0 != null)
                      {
                        ConnectionManagerListenerImpl_2_0 connMgrListener_2_0 = new ConnectionManagerListenerImpl_2_0();
                        ConnectionGlobalData.connMgrListenerMap_2_0.put(i, connMgrListener_2_0);

                        int initStatus2 = 0;
                        ConnectionGlobalData.oldConnMgrV2_0.InitializeService(
                                                              mIccId,
                                                              connMgrListener_2_0,
                                                              ConnectionGlobalData.userDataArray[i],
                                                              (int status, long listenerId) -> {
                                                                    //initStatus2 = status;
                                                                    connMgrListener_2_0.setListenerId(listenerId);
                                                              });
                        Log.d(TAG+mIccId, "ImsCmServiceImpl connMgrListener_2_0 connMgr status: " + initStatus2);
                      }
                  }
                  }
                  }
               }
               catch (Exception e) {
                  Log.d(TAG, "Exception raised" + e);
               }
           }});
    }
    private void setCmServiceStatus(int status){
	this.status = status;
	}
	private int getCmServiceStatus(){
	return this.status;
	}
    private void handleActivateSub0ButtonClick(Button connMgrButton) {
        connMgrButton.setOnClickListener(new OnClickListener() {
           public void onClick(View v) {
               try {
                   Log.d(TAG, "ActivateSub0 called");
                   ConnectionGlobalData.activeWorkingSub = 0;
               } catch (Exception e) {
                   Log.d(TAG, "Exception raised" + e);
               }
           }});
    }

    private void handleActivateSub1ButtonClick(Button connMgrButton) {
        connMgrButton.setOnClickListener(new OnClickListener() {
           public void onClick(View v) {
               try {
                   Log.d(TAG, "ActivateSub1 called");
                   ConnectionGlobalData.activeWorkingSub = 1;
               } catch (Exception e) {
                   Log.d(TAG, "Exception raised" + e);
               }
           }});
    }

    private void handleCreateSMConnectionButtonClick(Button connButton) {
        connButton.setOnClickListener(new OnClickListener() {
           public void onClick(View v) {
               try{
                       if ( ConnectionGlobalData.connMgrInitialised == 1)
                       {
                           Log.d(TAG, "createconnection: working sub: " +
                                 ConnectionGlobalData.activeWorkingSub + ", iccid:" + mIccId);
                           /*Creates new native IMS Connection object.
                             In this case SM feature tag is used */
                           ConnectionListenerImpl connListener = new ConnectionListenerImpl(ConnectionGlobalData.activeWorkingSub);
                           ConnectionGlobalData.connSmListenerMap.put(ConnectionGlobalData.activeWorkingSub,
                                                                      connListener);
                           ConnectionData connSm = new ConnectionData();
                           if(ConnectionGlobalData.connMgr != null) 
                           {
                             ConnectionGlobalData.connMgr.createConnection(
                                ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                ConnectionGlobalData.connSmListenerMap.get(ConnectionGlobalData.activeWorkingSub),
                                ConnectionGlobalData.SM_FEATURE_TAG,
                                   (IImsCMConnection connection, long connectionHandle, long listenerToken) -> {
                                        connSm.connection = connection;
                                        connSm.connectionHandle = connectionHandle;
                                        connListener.setListenerToken(listenerToken);
                                    });
                           }
                           else if(ConnectionGlobalData.oldConnMgrV2_1 != null)
                           {
                             ConnectionGlobalData.oldConnMgrV2_1.createConnection(
                                ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                ConnectionGlobalData.connSmListenerMap.get(ConnectionGlobalData.activeWorkingSub),
                                ConnectionGlobalData.SM_FEATURE_TAG,
                                    (IImsCMConnection connection, long connectionHandle, long listenerToken) -> {
                                        connSm.connection = connection;
                                        connSm.connectionHandle = connectionHandle;
                                        connListener.setListenerToken(listenerToken);
                                    });
                           }
                           else
                           {
                            ConnectionGlobalData.oldConnMgrV2_0.createConnection(
                                ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                ConnectionGlobalData.connSmListenerMap.get(ConnectionGlobalData.activeWorkingSub),
                                ConnectionGlobalData.SM_FEATURE_TAG,
                                    (IImsCMConnection connection, long connectionHandle, long listenerToken) -> {
                                        connSm.connection = connection;
                                        connSm.connectionHandle = connectionHandle;
                                        connListener.setListenerToken(listenerToken);
                                    });

                           }
                           ConnectionGlobalData.connSmMap.put(ConnectionGlobalData.activeWorkingSub, connSm);
                       }else{
                           Log.d(TAG, "createconnection is not initialised ");
                       }
                       }catch (Exception e){
                       Log.d(TAG,"Exception raised" + e);
                   }}});
    }

    private void handleCreateIMConnectionButtonClick(Button connButton) {
        connButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                try {

                        if ( ConnectionGlobalData.connMgrInitialised == 1){
                           Log.d(TAG, "create IM connection : iccid : "+
                                 ConnectionGlobalData.activeWorkingSub);
                           /*Creates new native IMS Connection object.
                             In this case IM feature tag is used*/
                           ConnectionListenerImpl connListener = new ConnectionListenerImpl(ConnectionGlobalData.activeWorkingSub);
                           ConnectionGlobalData.connImListenerMap.put(ConnectionGlobalData.activeWorkingSub,
                                                                      connListener );
                           ConnectionData connIm = new ConnectionData();
                           if(ConnectionGlobalData.connMgr != null)
                           {
                             ConnectionGlobalData.connMgr.createConnection(
                                ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                ConnectionGlobalData.connImListenerMap.get(ConnectionGlobalData.activeWorkingSub),
                                ConnectionGlobalData.IM_FEATURE_TAG,
                                   (IImsCMConnection connection, long connectionHandle, long listenerToken) -> {
                                        connIm.connection = connection;
                                        connIm.connectionHandle = connectionHandle;
                                        connListener.setListenerToken(listenerToken);
                                   });
                           }
                           else if(ConnectionGlobalData.oldConnMgrV2_1 != null)
                           {
                             ConnectionGlobalData.oldConnMgrV2_1.createConnection(
                                ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                ConnectionGlobalData.connImListenerMap.get(ConnectionGlobalData.activeWorkingSub),
                                ConnectionGlobalData.IM_FEATURE_TAG,
                                   (IImsCMConnection connection, long connectionHandle, long listenerToken) -> {
                                        connIm.connection = connection;
                                        connIm.connectionHandle = connectionHandle;
                                        connListener.setListenerToken(listenerToken);
                                    });
                           }
                           else
                           {
                             ConnectionGlobalData.oldConnMgrV2_0.createConnection(
                                ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                ConnectionGlobalData.connImListenerMap.get(ConnectionGlobalData.activeWorkingSub),
                                ConnectionGlobalData.IM_FEATURE_TAG,
                                   (IImsCMConnection connection, long connectionHandle, long listenerToken) -> {
                                        connIm.connection = connection;
                                        connIm.connectionHandle = connectionHandle;
                                        connListener.setListenerToken(listenerToken);
                                    });

                           }
                           ConnectionGlobalData.connImMap.put(ConnectionGlobalData.activeWorkingSub, connIm);
                        }else{
                           Log.d(TAG, "create IM connection Connection Manager is not initialised");
                        }
                    } catch (Exception e) {
                       Log.d(TAG, "Exception raised" + e);
                   }}});
    }

    private void handleCreateInvalidConnectionButtonClick(Button connButton) {
        connButton.setOnClickListener(new OnClickListener() {
           public void onClick(View v) {
                try {
                    if (ConnectionGlobalData.connMgr!= null){
                        if ( ConnectionGlobalData.connMgrInitialised == 1){
                           Log.d(TAG, "create Invalidconnection: iccid: " +
                                 ConnectionGlobalData.activeWorkingSub);
                           /*Creates new native IMS Connection object.
                             In this case SM feature tag is used*/
                           ConnectionListenerImpl connListener = new ConnectionListenerImpl(ConnectionGlobalData.activeWorkingSub);
                           ConnectionGlobalData.connInvalidListenerMap.put(ConnectionGlobalData.activeWorkingSub,
                                                                           connListener);
                           ConnectionData connInvalid = new ConnectionData();
                           ConnectionGlobalData.connMgr.createConnection(
                                   ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                   ConnectionGlobalData.connInvalidListenerMap.get(ConnectionGlobalData.activeWorkingSub),
                                   ConnectionGlobalData.INVALID_FEATURE_TAG3,
                                   (IImsCMConnection connection, long connectionHandle, long listenerToken) -> {
                                            connInvalid.connection = connection;
                                            connInvalid.connectionHandle = connectionHandle;
                                            connListener.setListenerToken(listenerToken);
                                   });
                           ConnectionGlobalData.connInvalidMap.put(ConnectionGlobalData.activeWorkingSub, connInvalid);

                        }else{
                           Log.d(TAG, "create Invalid connection - Connection Manager "+
                                      "is not initialised");
                        }
                    }else{
                       Log.d(TAG, "connMgr is NULL");
                    }} catch (Exception e) {
                       Log.d(TAG, "Exception raised" + e);
                    }
            }});
    }

    private void handleTriggerRegistrationButtonClick(Button connButton) {
        connButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                try{

                        if (ConnectionGlobalData.connMgrInitialised == 1)
                        {
                            int iRetValue ;
                            Log.d(TAG, "TriggerRegistration: iccid:" +
                                  ConnectionGlobalData.activeWorkingSub);
                            /*triggerRegistration---- Triggers registration.
                              This must be done after all the connections are created,
                              enabling registration triggering with all the required
                              feature tags simultaneously*/
                            if(ConnectionGlobalData.connMgr != null)
                            {
                              iRetValue = ConnectionGlobalData.connMgr.triggerRegistration(
                              ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                              ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                            }
                            else if(ConnectionGlobalData.oldConnMgrV2_1 != null)
                            {
                              iRetValue = ConnectionGlobalData.oldConnMgrV2_1.triggerRegistration(
                              ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                              ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                            }
                            else
                            {
                              iRetValue = ConnectionGlobalData.oldConnMgrV2_0.triggerRegistration(
                              ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                              ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                            }
                            Log.d(TAG, "TriggerRegistration RetValue "+ iRetValue);
                        }
                        else
                        {
                            Log.d(TAG, "create IM connection error connection manager "+
                                       "is not initilised : iccid: " +
                                  ConnectionGlobalData.activeWorkingSub);
                        }
                    }catch (Exception e){
                        Log.d(TAG, "Exception raised" + e);
                    }}});
    }

    private void handleTriggerDeRegistrationButtonClick(Button connButton) {
        connButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                try{

                        if (ConnectionGlobalData.connMgrInitialised == 1)
                        {
                            int iRetValue ;
                            Log.d(TAG, "TriggerDeRegistration: iccid:"+
                                  ConnectionGlobalData.activeWorkingSub);
                            /*triggerDeRegistration---- Triggers a deregistration.
                              This API removes all FTs, performs a PDN release,
                              and brings up the PDN*/
                            if(ConnectionGlobalData.connMgr != null) {
                              iRetValue = ConnectionGlobalData.connMgr.triggerDeRegistration(
                              ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                              ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                            }
                            else if(ConnectionGlobalData.oldConnMgrV2_1 != null)
                            {
                              iRetValue = ConnectionGlobalData.oldConnMgrV2_1.triggerDeRegistration(
                              ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                              ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                            }
                            else
                            {
                              iRetValue = ConnectionGlobalData.oldConnMgrV2_0.triggerDeRegistration(
                              ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                              ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                            }
                            Log.d(TAG, "TriggerDeRegistration RetValue "+ iRetValue);
                        }
                        else{
                            Log.d(TAG, "create IM connection error connection manager "+
                                       "is not initilised: iccid :"+
                                  ConnectionGlobalData.activeWorkingSub);
                        }
                    }catch (Exception e){
                        Log.d(TAG, "Exception raised" + e);
                    }}});
    }

    private void handleSendSMMessageButtonClick(Button connButton) {
        connButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                try{
                    if (ConnectionGlobalData.connMgrInitialised == 1 &&
                        ConnectionGlobalData.connSmMap.get(ConnectionGlobalData.activeWorkingSub) != null )
                    {
                        if ( ConnectionGlobalData.connMgrInitialised == 1 &&
                             ConnectionGlobalData.connSmInitialisedArray[ConnectionGlobalData.activeWorkingSub] == 1)
                        {
                            if(ConnectionGlobalData.deviceConfigData.isEmpty()) {
                                Log.d(TAG, "ERROR:Send SM Message: deviceConfigData is empty");
                                return;
                            }
                            if(ConnectionGlobalData.userConfigData.isEmpty()) {
                                Log.d(TAG, "ERROR:Send SM Message: userConfigData is empty");
                                return;
                            }
                            int messageId = (int) (new Date().getTime()/1000);
                            Log.d(TAG, "Send SM Message: iccid :"+
                                  ConnectionGlobalData.activeWorkingSub);
                            String strOutboundProxy = ConnectionGlobalData.deviceConfigData.get(deviceConfigKeys.StrSipOutBoundProxyName);
                            String strOutboundProxyPort =  ConnectionGlobalData.deviceConfigData.get(deviceConfigKeys.SipOutBoundProxyPort);
                            String strDomainName = ConnectionGlobalData.userConfigData.get(userConfigKeys.SipHomeDomain);
                            String strSipPublicUserId = ConnectionGlobalData.userConfigData.get(userConfigKeys.SipPublicUserId);
                            String strLocalIpaddress = ConnectionGlobalData.userConfigData.get(userConfigKeys.LocalHostIPAddress);
                            String msgString = "MESSAGE sip:22222@"+ strDomainName +" SIP/2.0\r\n"+
                                               "From: \"TestUE1\" <" +
                                               strSipPublicUserId +
                                               ">;tag=3486455352\r\n"+
                                               "To: <sip:22222@"+ strDomainName +">\r\n"+
                                               "CSeq: 255229832 MESSAGE\r\n"+
                                               "Call-ID: 3486455304_1570329456@"+
                                               "["+ strOutboundProxy+ "]\r\n"+
                                               "Via: SIP/2.0/UDP ["+ strLocalIpaddress + "]:"+strOutboundProxyPort +";"+
                                               "branch=z9hG4bK504163233\r\n"+
                                               "Max-Forwards: 70\r\n"+
                                               "Route: <sip:["+ strOutboundProxy+ "]:"+strOutboundProxyPort +";lr>\r\n"+
                                               "P-Access-Network-Info: 3GPP-E-UTRAN-FDD; "+
                                               "utran-cell-id-3gpp=3114800000011C000\r\n"+
                                               "Accept-Contact: *;+g.3gpp.icsi-ref=\"urn:urn-7:"+
                                               "3gpp-service.ims.icsi.oma.cpm.session\"\r\n"+
                                               "Content-Type: text/plain\r\n"+
                                               "Expires: 2345\r\n"+
                                               "Content-Length: 8\r\n\r\nSM_MESSAGE\r\n\r\n";

                            outgoingMessage msg = new outgoingMessage();
                            {   //Marshall KeyValueStringType data
                                /* Refers to the SIP request/response call ID value as per RFC 3261 */
                                keyValuePairStringType callIdType = new keyValuePairStringType();
                                callIdType.key = outgoingMessageKeys.CallId;
                                callIdType.value = "3486455304_1570329456@["+ strOutboundProxy+ "]";
                                msg.data.add(callIdType);

                                /* Message Type : REQUEST,RESPONSE,NONE */
                                keyValuePairStringType messagetypeVal = new keyValuePairStringType();
                                messagetypeVal.key = outgoingMessageKeys.MessageType;
                                messagetypeVal.value = Integer.toString(messageType.TYPE_REQUEST);
                                msg.data.add(messagetypeVal);

                                /* protocol can be TCP,UDP,NONE */
                                keyValuePairStringType protocolType = new keyValuePairStringType();
                                protocolType.key = outgoingMessageKeys.Protocol;
                                protocolType.value = Integer.toString(sipProtocolType.UDP);
                                msg.data.add(protocolType);
                                /*Message destination; for SIP requests, an outbound
                                  proxy address can be specified;
                                  for SIP responses, a header address can be used*/
                                keyValuePairStringType outboundProxyVal = new keyValuePairStringType();
                                outboundProxyVal.key = outgoingMessageKeys.OutboundProxy;
                                outboundProxyVal.value = "["+ strOutboundProxy+ "]";
                                msg.data.add(outboundProxyVal);

                                keyValuePairStringType remotePortVal = new keyValuePairStringType();
                                remotePortVal.key = outgoingMessageKeys.RemotePort;
                                remotePortVal.value = Integer.toString(5060);
                                msg.data.add(remotePortVal);
                            }
                            {   //Marshall KeyValueBuffer Type data
                                keyValuePairBufferType messageData = new keyValuePairBufferType();
                                messageData.key = outgoingMessageKeys.Message;
                                byte[] messageByteArray = msgString.getBytes();
                                for(int i = 0; i < messageByteArray.length; i++) {
                                    messageData.value.add(messageByteArray[i]);
                                }
                                msg.bufferData.add(messageData);
                            }
                            int ret_status = ConnectionGlobalData.connSmMap.get(
                                ConnectionGlobalData.activeWorkingSub).connection.sendMessage(
                                    msg,ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                            Log.d(TAG, "Send SM Message : status is " + ret_status);
                        }
                        else
                        {
                            Log.d(TAG, "SendSM Connection Manager/ SM connection "+
                                       "is not REGISTERED : iccid:"+
                                  ConnectionGlobalData.activeWorkingSub);
                        }
                    }else{
                        Log.d(TAG, " SendSM Connection Manager/ SM connection is NULL");
                    }}catch (Exception e){
                        Log.d(TAG, "Exception raised" + e);
                    }}});
    }

    private void handleSendIMMessageButtonClick(Button connButton) {
        connButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                try{
                    if (ConnectionGlobalData.connMgrInitialised == 1 &&
                        ConnectionGlobalData.connImMap.get(ConnectionGlobalData.activeWorkingSub) != null )
                    {
                        if ( ConnectionGlobalData.connMgrInitialised == 1)
                        {
                            String msgString = "MESSAGE sip:22222@ims.cingularme.com SIP/2.0\r\n"+
                                               "From: \"TestUE1\" <sip:11111@ims.cingularme.com>;"+
                                               "tag=3486455352\r\n"+
                                               "To: <sip:22222@ims.cingularme.com>\r\n"+
                                               "CSeq: 255229832 MESSAGE\r\n"+
                                               "Call-ID: 3486455304_1570329456@"+
                                               "[2002:c023:9c17:401e::3]\r\n"+
                                               "Via: SIP/2.0/UDP[2002:c023:9c17:401e::3]:5060;"+
                                               "branch=z9hG4bK504163233\r\n"+
                                               "Max-Forwards: 70\r\n"+
                                               "Route: <sip:[2002:c023:9c17:401e::3]:5060;"+
                                               "lr>\r\nP-Access-Network-Info: 3GPP-E-UTRAN-FDD;"+
                                               " utran-cell-id-3gpp=3114800000011C000\r\n"+
                                               "Accept-Contact: *;+g.3gpp.icsi-ref=\"urn:"+
                                               "urn-7:3gpp-service.ims.icsi.oma.cpm.session\"\r\n"+
                                               "Content-Type: text/plain\r\n"+
                                               "Expires: 2345\r\nContent-Length: 8\r\n\r\n"+
                                               "IM_MESSAGE\r\n\r\n";

                            outgoingMessage msg = new outgoingMessage();
                            {   //marshall keyValuePair string type data
                                keyValuePairStringType callIdVal = new keyValuePairStringType();
                                callIdVal.key = outgoingMessageKeys.CallId;
                                callIdVal.value = "3486455304_1570329456@[2002:c023:9c17:401e::3]";
                                msg.data.add(callIdVal);

                                keyValuePairStringType messageTypeVal = new keyValuePairStringType();
                                messageTypeVal.key = outgoingMessageKeys.MessageType;
                                messageTypeVal.value = Integer.toString(messageType.TYPE_REQUEST);
                                msg.data.add(messageTypeVal);

                                keyValuePairStringType protocolVal = new keyValuePairStringType();
                                protocolVal.key = outgoingMessageKeys.Protocol;
                                protocolVal.value = Integer.toString(sipProtocolType.UDP);
                                msg.data.add(protocolVal);

                                keyValuePairStringType outBoundProxyVal = new keyValuePairStringType();
                                outBoundProxyVal.key = outgoingMessageKeys.OutboundProxy;
                                outBoundProxyVal.value = "[2002:c023:9c17:401e::3]";
                                msg.data.add(outBoundProxyVal);

                                keyValuePairStringType remotePortVal = new keyValuePairStringType();
                                remotePortVal.key = outgoingMessageKeys.RemotePort;
                                remotePortVal.value = Integer.toString(5060);
                                msg.data.add(remotePortVal);
                            }
                            {
                                keyValuePairBufferType messageData = new keyValuePairBufferType();
                                messageData.key = outgoingMessageKeys.Message;
                                byte[] messageArray = msgString.getBytes();
                                for(int i = 0; i < messageArray.length; i++) {
                                    messageData.value.add(messageArray[i]);
                                }
                                msg.bufferData.add(messageData);
                            }
                            ConnectionGlobalData.connImMap.get(
                                ConnectionGlobalData.activeWorkingSub).connection.sendMessage(
                                    msg,
                                    ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                            Log.d(TAG, "Send IM Message: iccid: "+
                                  ConnectionGlobalData.activeWorkingSub);
                        }else{
                            Log.d(TAG, "Send IM Message");
                        }
                    }
                }catch (Exception e){
                    Log.d(TAG, "Exception raised" + e);
                }
            }});
    }

    private void handleCloseConnectionManagerButtonClick(Button connMgrButton) {
        connMgrButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                try{
                    Log.d(TAG, "CloseConnectionManagerButtonClick: iccid:"+
                          ConnectionGlobalData.activeWorkingSub);
                    if (ConnectionGlobalData.connMgr!= null){
                        if (ConnectionGlobalData.connMgrListenerMap.get(ConnectionGlobalData.activeWorkingSub)!= null )
                        {
                            Log.d(TAG, "ImsCmServiceImpl removeListener called: iccid"+
                                  ConnectionGlobalData.activeWorkingSub);
                            Log.d(TAG, "ImsCmServiceImpl.removeListener"+
                                  ConnectionGlobalData.connMgrListenerMap.get(ConnectionGlobalData.activeWorkingSub));
                            ConnectionGlobalData.connMgr.removeListener(
                                ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                ConnectionGlobalData.connMgrListenerMap.get(ConnectionGlobalData.activeWorkingSub).
                                getListenerId());
                        }
                        Log.d(TAG, "ConnectionManagerNativeImpl.close  called: iccid :"+
                              ConnectionGlobalData.activeWorkingSub);
                        /* Closes the Connection Manager.
                           Closing the manager forces pending connection objects to be
                           immediately deleted regardless of what state they are in*/
                        ConnectionGlobalData.connMgr.closeService(
                            ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub]);
                    }
                    else if(ConnectionGlobalData.oldConnMgrV2_1 != null)
                    {
                      if (ConnectionGlobalData.connMgrListenerMap_2_1.get(ConnectionGlobalData.activeWorkingSub)!= null )
                      {
                        Log.d(TAG, "ImsCmServiceImpl removeListener called: iccid"+
                               ConnectionGlobalData.activeWorkingSub);
                        Log.d(TAG, "ImsCmServiceImpl.removeListener"+
                               ConnectionGlobalData.connMgrListenerMap_2_1.get(ConnectionGlobalData.activeWorkingSub));
                        ConnectionGlobalData.oldConnMgrV2_1.removeListener(
                        ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                        ConnectionGlobalData.connMgrListenerMap_2_1.get(ConnectionGlobalData.activeWorkingSub).
                                getListenerId());
                      }
                      Log.d(TAG, "ConnectionManagerNativeImpl.close_2_1  called: iccid :"+
                              ConnectionGlobalData.activeWorkingSub);
                      /* Closes the Connection Manager.
                         Closing the manager forces pending connection objects to be
                         immediately deleted regardless of what state they are in*/
                      ConnectionGlobalData.oldConnMgrV2_1.closeService(
                        ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub]);
                    }
                    else if(ConnectionGlobalData.oldConnMgrV2_0 != null)
                    {
                      if (ConnectionGlobalData.connMgrListenerMap_2_0.get(ConnectionGlobalData.activeWorkingSub)!= null )
                      {
                        Log.d(TAG, "ImsCmServiceImpl removeListener called: iccid"+
                               ConnectionGlobalData.activeWorkingSub);
                        Log.d(TAG, "ImsCmServiceImpl.removeListener"+
                               ConnectionGlobalData.connMgrListenerMap_2_0.get(ConnectionGlobalData.activeWorkingSub));
                        ConnectionGlobalData.oldConnMgrV2_0.removeListener(
                        ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                        ConnectionGlobalData.connMgrListenerMap_2_0.get(ConnectionGlobalData.activeWorkingSub).
                                getListenerId());
                      }
                      Log.d(TAG, "ConnectionManagerNativeImpl.close_2_0  called: iccid :"+
                              ConnectionGlobalData.activeWorkingSub);
                      /* Closes the Connection Manager.
                         Closing the manager forces pending connection objects to be
                         immediately deleted regardless of what state they are in*/
                      ConnectionGlobalData.oldConnMgrV2_0.closeService(
                        ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub]);
                    }
                    else
                    {
                      Log.d(TAG, "CloseConnectionManagerButtonClick/connection Mgr is NULL: iccid :"+
                              ConnectionGlobalData.activeWorkingSub);
                    }
                    Log.d(TAG, "CloseConnectionManagerButtonClick end: iccid"+
                          ConnectionGlobalData.activeWorkingSub);
                }catch (Exception e){
                    Log.d(TAG, "Exception raised" + e);
                }}});
    }

    private void handleCloseSMConnectionButtonClick(Button connButton) {
        connButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                try{
                    if (ConnectionGlobalData.connSmMap.get(ConnectionGlobalData.activeWorkingSub) != null )
                    {
                        if ( ConnectionGlobalData.connMgrInitialised == 1)
                        {
                            Log.d(TAG, "SM closeAllTransactions: iccid:"+
                                  ConnectionGlobalData.activeWorkingSub);
                            IImsCMConnection connection = ConnectionGlobalData.connSmMap.get(
                                                             ConnectionGlobalData.activeWorkingSub).connection;
                            long listenerToken = ConnectionGlobalData.connSmListenerMap.get(
                                ConnectionGlobalData.activeWorkingSub).getListenerToken();
                            /*closeAllTransactions---- Terminates all transactions
                              being handled by the connection object*/
                            connection.closeAllTransactions(
                                    ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                            Log.d(TAG, "Remove SM listener: iccid:"+
                                  ConnectionGlobalData.activeWorkingSub);
                            if (listenerToken != 0 )
                                connection.removeListener(listenerToken);

                            Log.d(TAG, "Close SM connection: iccid:"+
                                  ConnectionGlobalData.activeWorkingSub);
                            /*ConnectionNativeImpl.close---- Closes the connection and
                              triggers deregistration of the associated URI*/
                            if(ConnectionGlobalData.connMgr != null){
                              ConnectionGlobalData.connMgr.closeConnection(
                                ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                ConnectionGlobalData.connSmMap.get(ConnectionGlobalData.activeWorkingSub).connectionHandle);
                            }
                            else if(ConnectionGlobalData.oldConnMgrV2_1 != null)
                            {
                              ConnectionGlobalData.oldConnMgrV2_1.closeConnection(
                                ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                ConnectionGlobalData.connSmMap.get(ConnectionGlobalData.activeWorkingSub).connectionHandle);
                            }
                            else
                            {
                              ConnectionGlobalData.oldConnMgrV2_0.closeConnection(
                                ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                ConnectionGlobalData.connSmMap.get(ConnectionGlobalData.activeWorkingSub).connectionHandle);
                            }
                        }
                        else{
                            Log.d(TAG, "CloseSMConnectionButtonClick Connection Manager "+
                                       "is not initialised: iccid:"+
                                  ConnectionGlobalData.activeWorkingSub);
                        }
                    }else{
                        Log.d(TAG, "CloseSMConnectionButtonClick/connection Mgr or "+
                                   "SM Conn is NULL: iccid:"+
                              ConnectionGlobalData.activeWorkingSub);
                    }
                }catch (Exception e){
                    Log.d(TAG, "Exception raised" + e);
                }}});
    }

    private void handleCloseIMConnectionButtonClick(Button connButton) {
        connButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                try{
                    if (ConnectionGlobalData.connImMap.get(ConnectionGlobalData.activeWorkingSub) != null )
                    {
                        if ( ConnectionGlobalData.connMgrInitialised == 1)
                        {
                            Log.d(TAG, "Close IM closeAllTransactions: iccid:"+
                                  ConnectionGlobalData.activeWorkingSub);

                            /*closeAllTransactions---- Terminates all transactions
                              being handled by the connection object*/
                            IImsCMConnection connection = ConnectionGlobalData.connImMap.get(
                                ConnectionGlobalData.activeWorkingSub).connection;
                            long connectionHandle = ConnectionGlobalData.connImMap.get(
                                ConnectionGlobalData.activeWorkingSub).connectionHandle;
                            long listenerToken = ConnectionGlobalData.connImListenerMap.get(
                                                        ConnectionGlobalData.activeWorkingSub).getListenerToken();
                            //closeTransaction
                            final String str = new String("3486455304_1570329456@[2002:c023:9c17:401e::3]");
                            connection.closeTransaction(str,ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                            connection. closeAllTransactions(
                                ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);

                            Log.d(TAG, "Remove IM listener: iccid:"+
                                  ConnectionGlobalData.activeWorkingSub);
                            if ( listenerToken != 0 ){
                                connection.removeListener(listenerToken);
                            }
                            Log.d(TAG, "Close IM connection: iccid:"+
                                  ConnectionGlobalData.activeWorkingSub);
                            if(ConnectionGlobalData.connMgr != null)
                            {
                              ConnectionGlobalData.connMgr.closeConnection(
                                ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                connectionHandle);
                            }
                            else if(ConnectionGlobalData.oldConnMgrV2_1 != null)
                            {
                              ConnectionGlobalData.oldConnMgrV2_1.closeConnection(
                                ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                connectionHandle);
                            }
                            else{
                              ConnectionGlobalData.oldConnMgrV2_0.closeConnection(
                                ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                connectionHandle);
                            }
                        }else{
                            Log.d(TAG, "Close IM connection");
                        }
                    }
                }catch (Exception e){
                    Log.d(TAG, "Exception raised" + e);
                }}});
    }

private void handleTriggerAcsButtonClick(Button triggerAcsButton) {
    triggerAcsButton.setOnClickListener(new OnClickListener() {
    @Override
    public void onClick(View arg0) {
    Log.d(TAG,"TriggerAcsButton click");
    try
    {
        if (ConnectionGlobalData.connMgr!= null)
        {
            if (ConnectionGlobalData.connMgrInitialised == 1)
            {
                Log.d(TAG, "TriggerAcsButton : Triggering Acs Request : iccid: " + ConnectionGlobalData.activeWorkingSub);

                int status = ConnectionGlobalData.connMgr.triggerACSRequest(
                                                 ConnectionGlobalData.serviceHandleArray[ConnectionGlobalData.activeWorkingSub],
                                                 ConnectionGlobalData.acsTriggerReason,
                                                 ConnectionGlobalData.userDataArray[ConnectionGlobalData.activeWorkingSub]);
                Log.d(TAG, "status: " + String.valueOf(status));
            }
            else
            {
                Log.d(TAG, "create IM connection error connection manager is not initilised");
            }
        }
        else
        {
            Log.d(TAG, "TriggerAcsButton ConnectionGlobalData.connMgr is NULL");
        }
    }
    catch (Exception e)
    {
        Log.d(TAG, "Exception raised" + e);
    }
    }});

    }
}
