/*=============================================================================
  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.example.connmgr;

import java.net.InetAddress;
import java.net.Socket;
import javax.net.SocketFactory;

import android.util.Log;
import android.net.ConnectivityManager;
import android.content.Context;
import android.net.NetworkInfo;

import com.qualcomm.qti.imscmservice.V2_2.IImsCmServiceListener;
import com.qualcomm.qti.imscmservice.V2_0.configData;
import com.qualcomm.qti.imscmservice.V2_0.keyValuePairStringType;
import com.qualcomm.qti.imscmservice.V2_0.keyValuePairBufferType;
import com.qualcomm.qti.imscmservice.V2_0.userConfigKeys;
import com.qualcomm.qti.imscmservice.V2_2.deviceConfigKeys;
import com.qualcomm.qti.imscmservice.V2_2.AutoConfigResponse;
import com.example.connmgr.ConnectionGlobalData;

class ConnectionManagerListenerImpl_2_0 extends com.qualcomm.qti.imscmservice.V2_0.IImsCmServiceListener.Stub  {
    final static String TAG = "UI_ConnMgr_ConnectionManagerListenerImpl";
    private long listenerId = 0;

    public void setListenerId(long listenerId){
      this.listenerId = listenerId;
    }
    public long getListenerId() {
      return this.listenerId;
    }

    /*onStatusChange---- This callback is invoked when there
      is a change in the status of the IMS Connection Manager.
    */
    public void onStatusChange(int status)  {
        Log.d(TAG, "onConnectionManagerStatusChange status is "+status);
        if ( status == 2 )
        {
            ConnectionGlobalData.connMgrInitialised = 1 ;
            Log.d(TAG, "onConnectionManagerStatusChange ConnectionManager is initialised");
            Log.d(TAG, "testing - sending packets across device");
            Log.d(TAG, "handleTestSendingPacketsViaSocket entered ");
            String localHost = "10.242.114.49";
            int localport = 54321;
            String packet = "Data Packet to be sent to remote.";
            Log.d(TAG, "localHost Ip address " + localHost);
            String remoteIP = "10.242.166.1";
            int remoteport = 60450;  // Got from the SIP Message.
            ConnectivityManager mConnectivityMgr;
            try{

              if( ConnectionGlobalData.mContext == null)
              {
                Log.d(TAG, "ConnectionGlobalData.mContext is null");
              }
              mConnectivityMgr = (ConnectivityManager) ConnectionGlobalData.mContext
                                                   .getSystemService(Context.CONNECTIVITY_SERVICE);

              if( mConnectivityMgr != null) {
                NetworkInfo nInfo = mConnectivityMgr.getActiveNetworkInfo();
                if(nInfo != null)
                {
                  Log.d(TAG, "getActiveNetworkInfo " + nInfo.toString());
                }
                else
                {
                  Log.d(TAG, "getActiveNetworkInfo nInfo is null, no active network found");
                }
                if(ConnectionGlobalData.IMSPDNConnected == true)
                {
                  Log.d(TAG, "Creating and sending message to remoteIP " +
                  InetAddress.getByName(remoteIP) +
                  " from host "+
                  InetAddress.getByName(localHost));
                  Socket soc = SocketFactory.getDefault().createSocket(
                    InetAddress.getByName(remoteIP),
                    remoteport,
                    InetAddress.getByName(localHost),
                    localport);
                    soc.getOutputStream().write(packet.getBytes());
                }
                else
                {
                  Log.d(TAG, "IMS PDN is not connected");
                }
              }
              else
              {
                Log.d(TAG, "Connectivity Manager is NULL");
              }
            }
            catch(Exception ex)
            {
              ex.printStackTrace();
            }
        }
    }

    /*onConfigurationChange---- Callback invoked when the configurations are updated*/
    public void onConfigurationChange(configData config, int userdata) {

      if(config != null)
      {
        for (int i=0; i<=1; i++)
        {
          if(ConnectionGlobalData.userDataArray[i] == userdata)
          {
            Log.d(TAG, "onConfigurationChange_2_0: on Sub :"+ i);
            Log.d(TAG, "onConfigurationChange_2_0: userData :"+ userdata);
          }
        }
        ConnectionGlobalData.configdata = config;
        if (config.userConfigData != null)
        {
          if(!config.userConfigData.data.isEmpty())  {
            ConnectionGlobalData.userConfigData.clear();
            for(keyValuePairStringType var : config.userConfigData.data)
            {
              Log.d(TAG, "userConfig: Key[" + userConfigKeys.toString(var.key) +
                         "] Value[" + var.value + "]");
              ConnectionGlobalData.userConfigData.put(var.key, var.value);
            }

          }
          else {
            Log.d(TAG, "userConfig: no Data received");
          }
        }

        if (config.deviceConfigData != null)
        {
          if(!config.deviceConfigData.data.isEmpty())  {
            ConnectionGlobalData.deviceConfigData.clear();
            for(keyValuePairStringType var : config.deviceConfigData.data)
            {
              Log.d(TAG, "deviceConfigData: Key[" + com.qualcomm.qti.imscmservice.V2_0.deviceConfigKeys.toString(var.key) +
                         "] Value[" + var.value + "]");
              ConnectionGlobalData.deviceConfigData.put(var.key, var.value);
            }

          }
          else {
            Log.d(TAG, "deviceConfigData: no Data received");
          }
        }

        if (config.autoConfigData != null)
        {
          ConnectionGlobalData.autoConfigData = config.autoConfigData;
          Log.d(TAG, "autoConfig" +
                ConnectionGlobalData.autoConfigData.autoConfigRequestType + "\n" +
                ConnectionGlobalData.autoConfigData.autoConfigXml);
          Log.d(TAG, "autoConfig via toString()" +
                ConnectionGlobalData.autoConfigData.toString());
        }

      }
      else
      {
        Log.d(TAG, "configData is null");
      }

    Log.d(TAG, "onConfigurationChange_2_0 config data via toString()" +
          ConnectionGlobalData.configdata.toString());
    }

    /*onCommandStatus---- Callback invoked to notify the clients the status of request placed.*/
    public void onCommandStatus(int userData, int status) {
      for (int i=0; i<=1; i++)
      {
        if(ConnectionGlobalData.userDataArray[i] == userData)
        {
          Log.d(TAG, "onCommandStatus: on Sub :"+ i);
          Log.d(TAG, "onCommandStatus: userData :"+ userData);
          Log.d(TAG, "onCommandStatus: status :"+ status);
        }
      }
    }

    /*onServiceReady---- Callback invoked upon successful service creation.*/
    public void onServiceReady(long serviceHandle, int userData, int status) {
      //iterate through array userDataArray and based on index : map iccid to servicehandle
      for (int i=0; i<=1; i++)
      {
        if(ConnectionGlobalData.userDataArray[i] == userData)
        {
          ConnectionGlobalData.connMgrInitialised = 1 ;
          ConnectionGlobalData.serviceHandleArray[i] = serviceHandle;
          Log.d(TAG+i, "onServiceReady: serviceHandle :"+ ConnectionGlobalData.serviceHandleArray[i]);
          Log.d(TAG+i, "onServiceReady: userData :"+ userData);
          Log.d(TAG+i, "onServiceReady: status :"+ status);

        }
      }
    }
}
class ConnectionManagerListenerImpl_2_1 extends com.qualcomm.qti.imscmservice.V2_1.IImsCmServiceListener.Stub  {
    final static String TAG = "UI_ConnMgr_ConnectionManagerListenerImpl";
    private long listenerId = 0;

    public void setListenerId(long listenerId){
      this.listenerId = listenerId;
    }
    public long getListenerId() {
      return this.listenerId;
    }

    /*onStatusChange---- This callback is invoked when there
      is a change in the status of the IMS Connection Manager.
    */
    public void onStatusChange(int status)  {
        Log.d(TAG, "onConnectionManagerStatusChange status is "+status);
        if ( status == 2 )
        {
            ConnectionGlobalData.connMgrInitialised = 1 ;
            Log.d(TAG, "onConnectionManagerStatusChange ConnectionManager is initialised");
            Log.d(TAG, "testing - sending packets across device");
            Log.d(TAG, "handleTestSendingPacketsViaSocket entered ");
            String localHost = "10.242.114.49";
            int localport = 54321;
            String packet = "Data Packet to be sent to remote.";
            Log.d(TAG, "localHost Ip address " + localHost);
            String remoteIP = "10.242.166.1";
            int remoteport = 60450;  // Got from the SIP Message.
            ConnectivityManager mConnectivityMgr;
            try{

              if( ConnectionGlobalData.mContext == null)
              {
                Log.d(TAG, "ConnectionGlobalData.mContext is null");
              }
              mConnectivityMgr = (ConnectivityManager) ConnectionGlobalData.mContext
                                                   .getSystemService(Context.CONNECTIVITY_SERVICE);

              if( mConnectivityMgr != null) {
                NetworkInfo nInfo = mConnectivityMgr.getActiveNetworkInfo();
                if(nInfo != null)
                {
                  Log.d(TAG, "getActiveNetworkInfo " + nInfo.toString());
                }
                else
                {
                  Log.d(TAG, "getActiveNetworkInfo nInfo is null, no active network found");
                }
                if(ConnectionGlobalData.IMSPDNConnected == true)
                {
                  Log.d(TAG, "Creating and sending message to remoteIP " +
                  InetAddress.getByName(remoteIP) +
                  " from host "+
                  InetAddress.getByName(localHost));
                  Socket soc = SocketFactory.getDefault().createSocket(
                    InetAddress.getByName(remoteIP),
                    remoteport,
                    InetAddress.getByName(localHost),
                    localport);
                    soc.getOutputStream().write(packet.getBytes());
                }
                else
                {
                  Log.d(TAG, "IMS PDN is not connected");
                }
              }
              else
              {
                Log.d(TAG, "Connectivity Manager is NULL");
              }
            }
            catch(Exception ex)
            {
              ex.printStackTrace();
            }
        }
    }
    /*onConfigurationChange---- Callback invoked when the configurations are updated*/
    public void onConfigurationChange(configData config, int userdata) {

      if(config != null)
      {
        for (int i=0; i<=1; i++)
        {
          if(ConnectionGlobalData.userDataArray[i] == userdata)
          {
            Log.d(TAG, "onConfigurationChange cm2.1: on Sub :"+ i);
            Log.d(TAG, "onConfigurationChange: userData :"+ userdata);
          }
        }
        ConnectionGlobalData.configdata = config;
        if (config.userConfigData != null)
        {
          if(!config.userConfigData.data.isEmpty())  {
            ConnectionGlobalData.userConfigData.clear();
            for(keyValuePairStringType var : config.userConfigData.data)
            {
              Log.d(TAG, "userConfig: Key[" + userConfigKeys.toString(var.key) +
                         "] Value[" + var.value + "]");
              ConnectionGlobalData.userConfigData.put(var.key, var.value);
            }

          }
          else {
            Log.d(TAG, "userConfig: no Data received");
          }
        }

        if (config.deviceConfigData != null)
        {
          if(!config.deviceConfigData.data.isEmpty())  {
            ConnectionGlobalData.deviceConfigData.clear();
            for(keyValuePairStringType var : config.deviceConfigData.data)
            {
              Log.d(TAG, "deviceConfigData: Key[" + com.qualcomm.qti.imscmservice.V2_0.deviceConfigKeys.toString(var.key) +
                         "] Value[" + var.value + "]");
              ConnectionGlobalData.deviceConfigData.put(var.key, var.value);
            }

          }
          else {
            Log.d(TAG, "deviceConfigData: no Data received");
          }
        }

        if (config.autoConfigData != null)
        {
          ConnectionGlobalData.autoConfigData = config.autoConfigData;
          Log.d(TAG, "autoConfig" +
                ConnectionGlobalData.autoConfigData.autoConfigRequestType + "\n" +
                ConnectionGlobalData.autoConfigData.autoConfigXml);
          Log.d(TAG, "autoConfig via toString()" +
                ConnectionGlobalData.autoConfigData.toString());
        }

      }
      else
      {
        Log.d(TAG, "configData is null");
      }

    Log.d(TAG, "onConfigurationChange config data via toString()" +
          ConnectionGlobalData.configdata.toString());
    }
    /*onConfigurationChange_2_1---- Callback invoked when the configurations are updated*/
    public void onConfigurationChange_2_1(configData config, int userdata) {

      if(config != null)
      {
        for (int i=0; i<=1; i++)
        {
          if(ConnectionGlobalData.userDataArray[i] == userdata)
          {
            Log.d(TAG, "onConfigurationChange_2_1: on Sub :"+ i);
            Log.d(TAG, "onConfigurationChange_2_1: userData :"+ userdata);
          }
        }
        ConnectionGlobalData.configdata = config;
        if (config.userConfigData != null)
        {
          if(!config.userConfigData.data.isEmpty())  {
            ConnectionGlobalData.userConfigData.clear();
            for(keyValuePairStringType var : config.userConfigData.data)
            {
              Log.d(TAG, "userConfig: Key[" + userConfigKeys.toString(var.key) +
                         "] Value[" + var.value + "]");
              ConnectionGlobalData.userConfigData.put(var.key, var.value);
            }

          }
          else {
            Log.d(TAG, "userConfig: no Data received");
          }
        }

        if (config.deviceConfigData != null)
        {
          if(!config.deviceConfigData.data.isEmpty())  {
            ConnectionGlobalData.deviceConfigData.clear();
            for(keyValuePairStringType var : config.deviceConfigData.data)
            {
              Log.d(TAG, "deviceConfigData: Key[" + deviceConfigKeys.toString(var.key) +
                         "] Value[" + var.value + "]");
              ConnectionGlobalData.deviceConfigData.put(var.key, var.value);
            }

          }
          else {
            Log.d(TAG, "deviceConfigData: no Data received");
          }
        }

        if (config.autoConfigData != null)
        {
          ConnectionGlobalData.autoConfigData = config.autoConfigData;
          Log.d(TAG, "autoConfig" +
                ConnectionGlobalData.autoConfigData.autoConfigRequestType + "\n" +
                ConnectionGlobalData.autoConfigData.autoConfigXml);
          Log.d(TAG, "autoConfig via toString()" +
                ConnectionGlobalData.autoConfigData.toString());
        }

      }
      else
      {
        Log.d(TAG, "configData is null");
      }

    Log.d(TAG, "onConfigurationChange_2_1 config data via toString()" +
          ConnectionGlobalData.configdata.toString());
    }

    /*onCommandStatus---- Callback invoked to notify the clients the status of request placed.*/
    public void onCommandStatus(int userData, int status) {
      for (int i=0; i<=1; i++)
      {
        if(ConnectionGlobalData.userDataArray[i] == userData)
        {
          Log.d(TAG, "onCommandStatus: on Sub :"+ i);
          Log.d(TAG, "onCommandStatus: userData :"+ userData);
          Log.d(TAG, "onCommandStatus: status :"+ status);
        }
      }
    }

    /*onServiceReady---- Callback invoked upon successful service creation.*/
    public void onServiceReady(long serviceHandle, int userData, int status) {
      //iterate through array userDataArray and based on index : map iccid to servicehandle
      for (int i=0; i<=1; i++)
      {
        if(ConnectionGlobalData.userDataArray[i] == userData)
        {
          ConnectionGlobalData.connMgrInitialised = 1 ;
          ConnectionGlobalData.serviceHandleArray[i] = serviceHandle;
          Log.d(TAG+i, "onServiceReady: serviceHandle :"+ ConnectionGlobalData.serviceHandleArray[i]);
          Log.d(TAG+i, "onServiceReady: userData :"+ userData);
          Log.d(TAG+i, "onServiceReady: status :"+ status);

        }
      }
    }

    public void onAcsConnectionStatusChange(int userData, int status) {
    for (int i=0; i<=1; i++)
      {
        if(ConnectionGlobalData.userDataArray[i] == userData)
        {
          Log.d(TAG, "onCommandStatus: on Sub :"+ i);
          Log.d(TAG, "onAcsConnectionStatusChange: userData :"+ userData);
          Log.d(TAG, "onAcsConnectionStatusChange: status :"+ status);
        }
      }
    }
}
// V2.2 ConnectionManagerListenerImpl
public class ConnectionManagerListenerImpl extends IImsCmServiceListener.Stub  {
    final static String TAG = "UI_ConnMgr_ConnectionManagerListenerImpl";
    private long listenerId = 0;

    public void setListenerId(long listenerId){
      this.listenerId = listenerId;
    }
    public long getListenerId() {
      return this.listenerId;
    }
    /*onStatusChange---- This callback is invoked when there
      is a change in the status of the IMS Connection Manager.
    */
    public void onStatusChange(int status)  {
      Log.d(TAG, "onConnectionManagerStatusChange status is "+status);
      if ( status == 2 )
      {
          ConnectionGlobalData.connMgrInitialised = 1 ;
          Log.d(TAG, "onConnectionManagerStatusChange ConnectionManager is initialised");
          Log.d(TAG, "testing - sending packets across device");
          Log.d(TAG, "handleTestSendingPacketsViaSocket entered ");
          String localHost = "10.242.114.49";
          int localport = 54321;
          String packet = "Data Packet to be sent to remote.";
          Log.d(TAG, "localHost Ip address " + localHost);
          String remoteIP = "10.242.166.1";
          int remoteport = 60450;  // Got from the SIP Message.
          ConnectivityManager mConnectivityMgr;
          try{

            if( ConnectionGlobalData.mContext == null)
            {
              Log.d(TAG, "ConnectionGlobalData.mContext is null");
            }
            mConnectivityMgr = (ConnectivityManager) ConnectionGlobalData.mContext
                                                 .getSystemService(Context.CONNECTIVITY_SERVICE);

            if( mConnectivityMgr != null) {
              NetworkInfo nInfo = mConnectivityMgr.getActiveNetworkInfo();
              if(nInfo != null)
              {
                Log.d(TAG, "getActiveNetworkInfo " + nInfo.toString());
              }
              else
              {
                Log.d(TAG, "getActiveNetworkInfo nInfo is null, no active network found");
              }
              if(ConnectionGlobalData.IMSPDNConnected == true)
              {
                Log.d(TAG, "Creating and sending message to remoteIP " +
                InetAddress.getByName(remoteIP) +
                " from host "+
                InetAddress.getByName(localHost));
                Socket soc = SocketFactory.getDefault().createSocket(
                  InetAddress.getByName(remoteIP),
                  remoteport,
                  InetAddress.getByName(localHost),
                  localport);
                  soc.getOutputStream().write(packet.getBytes());
              }
              else
              {
                Log.d(TAG, "IMS PDN is not connected");
              }
            }
            else
            {
              Log.d(TAG, "Connectivity Manager is NULL");
            }
          }
          catch(Exception ex)
          {
            ex.printStackTrace();
          }
      }
  }
    /*onConfigurationChange---- Callback invoked when the configurations are updated*/
    public void onConfigurationChange(configData config, int userdata) {

      if(config != null)
      {
        for (int i=0; i<=1; i++)
        {
          if(ConnectionGlobalData.userDataArray[i] == userdata)
          {
            Log.d(TAG, "onConfigurationChange cm2.1: on Sub :"+ i);
            Log.d(TAG, "onConfigurationChange: userData :"+ userdata);
          }
        }
        ConnectionGlobalData.configdata = config;
        if (config.userConfigData != null)
        {
          if(!config.userConfigData.data.isEmpty())  {
            ConnectionGlobalData.userConfigData.clear();
            for(keyValuePairStringType var : config.userConfigData.data)
            {
              Log.d(TAG, "userConfig: Key[" + userConfigKeys.toString(var.key) +
                         "] Value[" + var.value + "]");
              ConnectionGlobalData.userConfigData.put(var.key, var.value);
            }

          }
          else {
            Log.d(TAG, "userConfig: no Data received");
          }
        }

        if (config.deviceConfigData != null)
        {
          if(!config.deviceConfigData.data.isEmpty())  {
            ConnectionGlobalData.deviceConfigData.clear();
            for(keyValuePairStringType var : config.deviceConfigData.data)
            {
              Log.d(TAG, "deviceConfigData: Key[" + com.qualcomm.qti.imscmservice.V2_0.deviceConfigKeys.toString(var.key) +
                         "] Value[" + var.value + "]");
              ConnectionGlobalData.deviceConfigData.put(var.key, var.value);
            }

          }
          else {
            Log.d(TAG, "deviceConfigData: no Data received");
          }
        }

        if (config.autoConfigData != null)
        {
          ConnectionGlobalData.autoConfigData = config.autoConfigData;
          Log.d(TAG, "autoConfig" +
                ConnectionGlobalData.autoConfigData.autoConfigRequestType + "\n" +
                ConnectionGlobalData.autoConfigData.autoConfigXml);
          Log.d(TAG, "autoConfig via toString()" +
                ConnectionGlobalData.autoConfigData.toString());
        }

      }
      else
      {
        Log.d(TAG, "configData is null");
      }

    Log.d(TAG, "onConfigurationChange config data via toString()" +
          ConnectionGlobalData.configdata.toString());
    }
    /*onConfigurationChange_2_1---- Callback invoked when the configurations are updated*/
    public void onConfigurationChange_2_1(configData config, int userdata) {

      if(config != null)
      {
        for (int i=0; i<=1; i++)
        {
          if(ConnectionGlobalData.userDataArray[i] == userdata)
          {
            Log.d(TAG, "onConfigurationChange_2_1: on Sub :"+ i);
            Log.d(TAG, "onConfigurationChange_2_1: userData :"+ userdata);
          }
        }
        ConnectionGlobalData.configdata = config;
        if (config.userConfigData != null)
        {
          if(!config.userConfigData.data.isEmpty())  {
            ConnectionGlobalData.userConfigData.clear();
            for(keyValuePairStringType var : config.userConfigData.data)
            {
              Log.d(TAG, "userConfig: Key[" + userConfigKeys.toString(var.key) +
                         "] Value[" + var.value + "]");
              ConnectionGlobalData.userConfigData.put(var.key, var.value);
            }

          }
          else {
            Log.d(TAG, "userConfig: no Data received");
          }
        }

        if (config.deviceConfigData != null)
        {
          if(!config.deviceConfigData.data.isEmpty())  {
            ConnectionGlobalData.deviceConfigData.clear();
            for(keyValuePairStringType var : config.deviceConfigData.data)
            {
              Log.d(TAG, "deviceConfigData: Key[" + deviceConfigKeys.toString(var.key) +
                         "] Value[" + var.value + "]");
              ConnectionGlobalData.deviceConfigData.put(var.key, var.value);
            }

          }
          else {
            Log.d(TAG, "deviceConfigData: no Data received");
          }
        }

        if (config.autoConfigData != null)
        {
          ConnectionGlobalData.autoConfigData = config.autoConfigData;
          Log.d(TAG, "autoConfig" +
                ConnectionGlobalData.autoConfigData.autoConfigRequestType + "\n" +
                ConnectionGlobalData.autoConfigData.autoConfigXml);
          Log.d(TAG, "autoConfig via toString()" +
                ConnectionGlobalData.autoConfigData.toString());
        }

      }
      else
      {
        Log.d(TAG, "configData is null");
      }

    Log.d(TAG, "onConfigurationChange_2_1 config data via toString()" +
          ConnectionGlobalData.configdata.toString());
    }
    /*onConfigurationChange_2_2---- Callback invoked when the configurations are updated*/
    public void onConfigurationChange_2_2(configData config, int userdata) {

      if(config != null)
      {
        for (int i=0; i<=1; i++)
        {
          if(ConnectionGlobalData.userDataArray[i] == userdata)
          {
            Log.d(TAG, "onConfigurationChange_2_2: on Sub :"+ i);
            Log.d(TAG, "onConfigurationChange_2_2: userData :"+ userdata);
          }
        }
        ConnectionGlobalData.configdata = config;
        if (config.userConfigData != null)
        {
          if(!config.userConfigData.data.isEmpty())  {
            ConnectionGlobalData.userConfigData.clear();
            for(keyValuePairStringType var : config.userConfigData.data)
            {
              Log.d(TAG, "userConfig: Key[" + userConfigKeys.toString(var.key) +
                         "] Value[" + var.value + "]");
              ConnectionGlobalData.userConfigData.put(var.key, var.value);
            }

          }
          else {
            Log.d(TAG, "userConfig: no Data received");
          }
        }

        if (config.deviceConfigData != null)
        {
          if(!config.deviceConfigData.data.isEmpty())  {
            ConnectionGlobalData.deviceConfigData.clear();
            for(keyValuePairStringType var : config.deviceConfigData.data)
            {
              Log.d(TAG, "deviceConfigData: Key[" + deviceConfigKeys.toString(var.key) +
                         "] Value[" + var.value + "]");
              ConnectionGlobalData.deviceConfigData.put(var.key, var.value);
            }

          }
          else {
            Log.d(TAG, "deviceConfigData: no Data received");
          }
        }

        if (config.autoConfigData != null)
        {
          ConnectionGlobalData.autoConfigData = config.autoConfigData;
          Log.d(TAG, "autoConfig" +
                ConnectionGlobalData.autoConfigData.autoConfigRequestType + "\n" +
                ConnectionGlobalData.autoConfigData.autoConfigXml);
          Log.d(TAG, "autoConfig via toString()" +
                ConnectionGlobalData.autoConfigData.toString());
        }

      }
      else
      {
        Log.d(TAG, "configData is null");
      }

    Log.d(TAG, "onConfigurationChange_2_2 config data via toString()" +
          ConnectionGlobalData.configdata.toString());
    }
    /*onCommandStatus---- Callback invoked to notify the clients the status of request placed.*/
    public void onCommandStatus(int userData, int status) {
      for (int i=0; i<=1; i++)
      {
        if(ConnectionGlobalData.userDataArray[i] == userData)
        {
          Log.d(TAG, "onCommandStatus: on Sub :"+ i);
          Log.d(TAG, "onCommandStatus: userData :"+ userData);
          Log.d(TAG, "onCommandStatus: status :"+ status);
        }
      }
    }
    /*onServiceReady---- Callback invoked upon successful service creation.*/
    public void onServiceReady(long serviceHandle, int userData, int status) {
      //iterate through array userDataArray and based on index : map iccid to servicehandle
      for (int i=0; i<=1; i++)
      {
        if(ConnectionGlobalData.userDataArray[i] == userData)
        {
          ConnectionGlobalData.connMgrInitialised = 1 ;
          ConnectionGlobalData.serviceHandleArray[i] = serviceHandle;
          Log.d(TAG+i, "onServiceReady: serviceHandle :"+ ConnectionGlobalData.serviceHandleArray[i]);
          Log.d(TAG+i, "onServiceReady: userData :"+ userData);
          Log.d(TAG+i, "onServiceReady: status :"+ status);

        }
      }
    }
    public void onAcsConnectionStatusChange(int userData, int status) {
      for (int i=0; i<=1; i++)
        {
          if(ConnectionGlobalData.userDataArray[i] == userData)
          {
            Log.d(TAG, "onCommandStatus: on Sub :"+ i);
            Log.d(TAG, "onAcsConnectionStatusChange: userData :"+ userData);
            Log.d(TAG, "onAcsConnectionStatusChange: status :"+ status);
          }
        }
    }
    public void onAutoConfigResponse(AutoConfigResponse acsResponse) {
      ConnectionGlobalData.acsResponse = acsResponse;
      Log.d(TAG, "onAutoConfigResponse: statusCode:"+ acsResponse.statusCode);
      Log.d(TAG, "onAutoConfigResponse: reasonPhrase :"+ acsResponse.reasonPhrase);
    }
}
