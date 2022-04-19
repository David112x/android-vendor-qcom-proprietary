/*
 * Copyright (C) 2017, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 */
/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.bluetooth.avrcp;

import java.util.Timer;
import java.util.TimerTask;
import android.util.Log;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;
import android.media.session.MediaController;
import android.media.session.MediaSessionManager;
import android.content.Context;
import android.bluetooth.BluetoothDevice;
import com.android.bluetooth.avrcp.AvrcpPlayerAppSettingsRspInterface;
import com.android.bluetooth.avrcp.AvrcpConstants_ext;
import android.content.BroadcastReceiver;
import com.android.bluetooth.Utils;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Set;
import java.util.Iterator;
import android.os.SystemProperties;

public class AvrcpPlayerAppSettings {
    private static final boolean DEBUG = true;
    private static final String TAG = "AvrcpPlayerAppSettings";

    private boolean mIsRegisterd;
    private long mPlayerAppSettingsCmdDelay;
    private ArrayList <Integer> mPendingCmds;
    private ArrayList <Integer> mPendingSetAttributes;
    private ArrayList <Byte> mPlayerSettingCmds;
    private Context mContext;
    private AvrcpPlayerAppSettingsRspInterface mAvrcpPlayerAppSettingsRspInterface;
    private class PlayerSettings {
        public byte [] attrIds;
        public String path;
    };

    private PlayerSettings mPlayerSettings = new PlayerSettings();
    private class localPlayerSettings {
        public byte eq_value = 0x01;
        public byte repeat_value = 0x01;
        public byte shuffle_value = 0x01;
        public byte scan_value = 0x01;
    };
    private localPlayerSettings settingValues = new localPlayerSettings();
    private static final String COMMAND = "command";
    private static final String CMDGET = "get";
    private static final String CMDSET = "set";
    private static final String EXTRA_GET_COMMAND = "commandExtra";
    private static final String EXTRA_GET_RESPONSE = "Response";

    private static final String EXTRA_ATTRIBUTE_ID = "Attribute";
    private static final String EXTRA_VALUE_STRING_ARRAY = "ValueStrings";
    private static final String EXTRA_ATTRIB_VALUE_PAIRS = "AttribValuePairs";
    private static final String EXTRA_ATTRIBUTE_STRING_ARRAY = "AttributeStrings";
    private static final String EXTRA_VALUE_ID_ARRAY = "Values";
    private static final String EXTRA_ATTIBUTE_ID_ARRAY = "Attributes";

    public static final int VALUE_SHUFFLEMODE_OFF = 1;
    public static final int VALUE_SHUFFLEMODE_ALL = 2;
    public static final int VALUE_REPEATMODE_OFF = 1;
    public static final int VALUE_REPEATMODE_SINGLE = 2;
    public static final int VALUE_REPEATMODE_ALL = 3;
    public static final int VALUE_INVALID = 0;
    public static final int ATTRIBUTE_NOTSUPPORTED = -1;

    public static final int ATTRIBUTE_EQUALIZER = 1;
    public static final int ATTRIBUTE_REPEATMODE = 2;
    public static final int ATTRIBUTE_SHUFFLEMODE = 3;
    public static final int ATTRIBUTE_SCANMODE = 4;
    public static final int NUMPLAYER_ATTRIBUTE = 2;

    private static final String BLUETOOTH_ADMIN_PERM = android.Manifest.permission.BLUETOOTH_ADMIN;
    private static final String BLUETOOTH_PERM = android.Manifest.permission.BLUETOOTH;

    private byte [] def_attrib = new byte [] {ATTRIBUTE_REPEATMODE, ATTRIBUTE_SHUFFLEMODE};
    private byte [] value_repmode = new byte [] { VALUE_REPEATMODE_OFF,
                                                  VALUE_REPEATMODE_SINGLE,
                                                  VALUE_REPEATMODE_ALL };

    private byte [] value_shufmode = new byte [] { VALUE_SHUFFLEMODE_OFF,
                                                  VALUE_SHUFFLEMODE_ALL };
    private byte [] value_default = new byte [] {0};
    private final String UPDATE_ATTRIBUTES = "UpdateSupportedAttributes";
    private final String UPDATE_VALUES = "UpdateSupportedValues";
    private final String UPDATE_ATTRIB_VALUE = "UpdateCurrentValues";
    private final String UPDATE_ATTRIB_TEXT = "UpdateAttributesText";
    private final String UPDATE_VALUE_TEXT = "UpdateValuesText";

    // Property for response delay for Player App Settings commands
    private static final String AVRCP_PLAYERAPP_SETTINGS_PROPERTY =
            "persist.vendor.btstack.avrcp.playerappsettings_time";

    //Intents for PlayerApplication Settings
    private static final String PLAYERSETTINGS_REQUEST =
            "org.codeaurora.music.playersettingsrequest";
    private static final String PLAYERSETTINGS_RESPONSE =
           "org.codeaurora.music.playersettingsresponse";

    public AvrcpPlayerAppSettings(Context context,
            AvrcpPlayerAppSettingsRspInterface playerAppSettings) {
        mIsRegisterd = false;
        mPendingCmds = null;
        mPendingSetAttributes = null;
        mPlayerSettingCmds = null;
        mPlayerAppSettingsCmdDelay =
                SystemProperties.getLong(AVRCP_PLAYERAPP_SETTINGS_PROPERTY, 100L);
        mContext = context;
        mAvrcpPlayerAppSettingsRspInterface = playerAppSettings;
    }

    public void start() {
        Log.v(TAG,"Start ");
        mPendingCmds = new ArrayList<Integer>();
        mPendingSetAttributes = new ArrayList<Integer>();
        mPlayerSettingCmds = new ArrayList<Byte>();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(PLAYERSETTINGS_RESPONSE);
        if (!mIsRegisterd) {
            try {
                mIsRegisterd = true;
                mContext.registerReceiver(mIntentReceiver, intentFilter);
            } catch (Exception e) {
                Log.e(TAG,"Unable to register Avrcp receiver", e);
            }
        } else {
            Log.v(TAG,"Player Settings intent already registered");
        }
    }

    public void cleanup() {
        Log.v(TAG,"Stop");
        if (mIsRegisterd) {
            try {
                mIsRegisterd = false;
                mContext.unregisterReceiver(mIntentReceiver);
            } catch (Exception e) {
                Log.e(TAG,"Unable to register Avrcp receiver", e);
            }
        } else {
            Log.d(TAG, "already stopped, returning");
        }
    }

    //Listen to intents from MediaPlayer and Audio Manager and update data structures
    private BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "Enter onReceive()" +intent);
            String action = intent.getAction();
            if (action.equals(PLAYERSETTINGS_RESPONSE)) {
            int getResponse = intent.getIntExtra(EXTRA_GET_RESPONSE,
                                                      AvrcpConstants_ext.GET_INVALID);
                byte [] data;
                String [] text;
                boolean isSetAttrValRsp = false;
                BluetoothDevice device = null;

                synchronized (mPendingCmds) {
                    Integer val = new Integer(getResponse);
                    if (mPendingCmds.contains(val)) {
                        if (getResponse == AvrcpConstants_ext.SET_ATTRIBUTE_VALUES) {
                            isSetAttrValRsp = true;
                            if (DEBUG) Log.v(TAG,"Response received for SET_ATTRIBUTE_VALUES");
                        }
                        mPendingCmds.remove(val);
                    }
                }

                device = mAvrcpPlayerAppSettingsRspInterface.
                        GetPlayerSettingCmdPendingDevice(getResponse);

                if (DEBUG)
                    Log.v(TAG,"getResponse" + getResponse);
                switch (getResponse) {
                    case AvrcpConstants_ext.GET_ATTRIBUTE_IDS:
                        if (device == null) {
                            Log.e(TAG,"ERROR!!! device is null");
                            return;
                        }
                        data = intent.getByteArrayExtra(EXTRA_ATTIBUTE_ID_ARRAY);
                        byte numAttr = (byte) data.length;
                        if (DEBUG)
                            Log.v(TAG,"GET_ATTRIBUTE_IDS");
                        mAvrcpPlayerAppSettingsRspInterface.getListPlayerappAttrRsp(numAttr ,
                                data ,getByteAddress(device));

                    break;
                    case AvrcpConstants_ext.GET_VALUE_IDS:
                        if (device == null) {
                            Log.e(TAG,"ERROR!!! device is null");
                            return;
                        }

                        if (!mPlayerSettingCmds.isEmpty()) {
                            mPlayerSettingCmds.remove(0);
                        } else {
                            Log.e(TAG, "No cmds in the queue");
                            break;
                        }

                        data = intent.getByteArrayExtra(EXTRA_VALUE_ID_ARRAY);
                        numAttr = (byte) data.length;
                        if (DEBUG)
                            Log.v(TAG,"GET_VALUE_IDS" + numAttr);
                        mAvrcpPlayerAppSettingsRspInterface.getPlayerAppValueRsp(numAttr, data,
                                getByteAddress(device));
                    break;
                    case AvrcpConstants_ext.GET_ATTRIBUTE_VALUES:
                        if (device == null) {
                            Log.e(TAG,"ERROR!!! device is null");
                            return;
                        }
                        data = intent.getByteArrayExtra(EXTRA_ATTRIB_VALUE_PAIRS);
                        updateLocalPlayerSettings(data);
                        numAttr = (byte) data.length;
                        if (DEBUG)
                            Log.v(TAG,"GET_ATTRIBUTE_VALUES" + numAttr);
                        mAvrcpPlayerAppSettingsRspInterface.SendCurrentPlayerValueRsp(numAttr ,
                                data, getByteAddress(device));
                    break;
                    case AvrcpConstants_ext.SET_ATTRIBUTE_VALUES:
                        boolean send_change_rsp_only = true;
                        data = intent.getByteArrayExtra(EXTRA_ATTRIB_VALUE_PAIRS);
                        updateLocalPlayerSettings(data);
                        if (isSetAttrValRsp && device != null) {
                            isSetAttrValRsp = false;
                            send_change_rsp_only = false;
                            Log.v(TAG,"Respond to SET_ATTRIBUTE_VALUES request");
                            if (checkPlayerAttributeResponse(data)) {
                                mAvrcpPlayerAppSettingsRspInterface.SendSetPlayerAppRsp(
                                        AvrcpConstants_ext.RSP_NO_ERROR, getByteAddress(device));
                                } else {
                                mAvrcpPlayerAppSettingsRspInterface.SendSetPlayerAppRsp(
                                        AvrcpConstants_ext.RSP_INTERNAL_ERR, getByteAddress(device));
                            }
                            mPendingSetAttributes.clear();
                        }
                        if (send_change_rsp_only) {
                            mAvrcpPlayerAppSettingsRspInterface.SendSetPlayerAppRsp(
                                    AvrcpConstants_ext.RSP_NO_ERROR, null);
                        }
                    break;
                    case AvrcpConstants_ext.GET_ATTRIBUTE_TEXT:
                        text = intent.getStringArrayExtra(EXTRA_ATTRIBUTE_STRING_ARRAY);
                        if (device == null) {
                            Log.e(TAG," ERROR!!! device is null");
                            return;
                        }
                        mAvrcpPlayerAppSettingsRspInterface.sendSettingsTextRsp(
                                mPlayerSettings.attrIds.length, mPlayerSettings.attrIds,
                                text.length, text, getByteAddress(device));
                        if (DEBUG)
                            Log.v(TAG,"mPlayerSettings.attrIds"
                                    + mPlayerSettings.attrIds.length);
                    break;
                    case AvrcpConstants_ext.GET_VALUE_TEXT:
                        text = intent.getStringArrayExtra(EXTRA_VALUE_STRING_ARRAY);
                        if (device == null) {
                            Log.e(TAG,"ERROR!!! device is null");
                            return;
                        }
                        mAvrcpPlayerAppSettingsRspInterface.sendValueTextRsp(
                                mPlayerSettings.attrIds.length, mPlayerSettings.attrIds,
                                text.length, text, getByteAddress(device));
                    break;
                }
            }
        }
    };

    public void handlerMsgTimeout(int cmd, BluetoothDevice device) {
        Log.v(TAG,"handlerMsgTimeout");
        synchronized (mPendingCmds) {
            Integer val = new Integer(cmd);
            if (!mPendingCmds.contains(val)) {
                return;
            }
            mPendingCmds.remove(val);
        }
        switch (cmd) {
            case AvrcpConstants_ext.GET_ATTRIBUTE_IDS:
                mAvrcpPlayerAppSettingsRspInterface.getListPlayerappAttrRsp(
                        (byte)def_attrib.length ,
                        def_attrib, getByteAddress(device));
                break;
            case AvrcpConstants_ext.GET_VALUE_IDS:
                byte attrib = 0;
                if (!mPlayerSettingCmds.isEmpty()) {
                    attrib = mPlayerSettingCmds.get(0);
                    mPlayerSettingCmds.remove(0);
                } else {
                    Log.e(TAG, "No cmds in queue");
                    break;
                }

                switch (attrib) {
                    case ATTRIBUTE_REPEATMODE:
                        mAvrcpPlayerAppSettingsRspInterface.getPlayerAppValueRsp(
                                (byte)value_repmode.length,
                                value_repmode, getByteAddress(device));
                    break;
                    case ATTRIBUTE_SHUFFLEMODE:
                        mAvrcpPlayerAppSettingsRspInterface.getPlayerAppValueRsp(
                                (byte)value_shufmode.length,
                                value_shufmode, getByteAddress(device));
                    break;
                    default:
                        mAvrcpPlayerAppSettingsRspInterface.getPlayerAppValueRsp(
                                (byte)value_default.length,
                                value_default, getByteAddress(device));
                    break;
                }
                break;
            case AvrcpConstants_ext.GET_ATTRIBUTE_VALUES:
                int j = 0;
                byte [] retVal = new byte [mPlayerSettings.attrIds.length*2];
                for (int i = 0; i < mPlayerSettings.attrIds.length; i++) {
                    retVal[j++] = mPlayerSettings.attrIds[i];
                    if (mPlayerSettings.attrIds[i] == ATTRIBUTE_REPEATMODE) {
                        retVal[j++] = settingValues.repeat_value;
                    } else if (mPlayerSettings.attrIds[i] == ATTRIBUTE_SHUFFLEMODE) {
                        retVal[j++] = settingValues.shuffle_value;
                    } else {
                        retVal[j++] = 0x0;
                    }
                 }
                 mAvrcpPlayerAppSettingsRspInterface.SendCurrentPlayerValueRsp((byte)retVal.length,
                        retVal, getByteAddress(device));
                break;
            case AvrcpConstants_ext.SET_ATTRIBUTE_VALUES :
                 mAvrcpPlayerAppSettingsRspInterface.SendSetPlayerAppRsp(
                        AvrcpConstants_ext.RSP_INTERNAL_ERR, getByteAddress(device));
                break;
            case AvrcpConstants_ext.GET_ATTRIBUTE_TEXT:
                String [] attribText = new String [mPlayerSettings.attrIds.length];
                for (int i = 0; i < mPlayerSettings.attrIds.length; i++) {
                    attribText[i] = "";
                }
                mAvrcpPlayerAppSettingsRspInterface.sendSettingsTextRsp(
                        mPlayerSettings.attrIds.length, mPlayerSettings.attrIds,
                        attribText.length, attribText, getByteAddress(device));
                break;
            case AvrcpConstants_ext.GET_VALUE_TEXT:
                String [] valueText = new String [mPlayerSettings.attrIds.length];
                for (int i = 0; i < mPlayerSettings.attrIds.length; i++) {
                    valueText[i] = "";
                }
                mAvrcpPlayerAppSettingsRspInterface.sendValueTextRsp(
                        mPlayerSettings.attrIds.length, mPlayerSettings.attrIds,
                        valueText.length, valueText, getByteAddress(device));
                break;
                default :
                    Log.e(TAG,"in default case");
                break;
        }
    }


    public void onListPlayerAttributeRequest(byte[] address) {
        Intent intent = new Intent(PLAYERSETTINGS_REQUEST);
        intent.putExtra(COMMAND, CMDGET);
        intent.putExtra(EXTRA_GET_COMMAND, AvrcpConstants_ext.GET_ATTRIBUTE_IDS);
        mContext.sendBroadcast(intent, BLUETOOTH_PERM);
        mPendingCmds.add(new Integer(AvrcpConstants_ext.GET_ATTRIBUTE_IDS));
    }

    public void onListPlayerAttributeValues (byte attr, byte[] address) {
        Intent intent = new Intent(PLAYERSETTINGS_REQUEST);
        intent.putExtra(COMMAND, CMDGET);
        intent.putExtra(EXTRA_GET_COMMAND, AvrcpConstants_ext.GET_VALUE_IDS);
        intent.putExtra(EXTRA_ATTRIBUTE_ID, attr);
        mContext.sendBroadcast(intent, BLUETOOTH_PERM);
        mPlayerSettingCmds.add(attr);
        mPendingCmds.add(new Integer(AvrcpConstants_ext.GET_VALUE_IDS));
    }

    public void onGetPlayerAttributeValues (byte attr ,int[] arr ,
            byte[] address) {
        int i ;
        byte[] barray = new byte[attr];
        for(i = 0; i<attr ; ++i)
            barray[i] = (byte)arr[i];
        mPlayerSettings.attrIds = new byte [attr];
        for ( i = 0; i < attr; i++)
            mPlayerSettings.attrIds[i] = barray[i];
        Intent intent = new Intent(PLAYERSETTINGS_REQUEST);
        intent.putExtra(COMMAND, CMDGET);
        intent.putExtra(EXTRA_GET_COMMAND, AvrcpConstants_ext.GET_ATTRIBUTE_VALUES);
        intent.putExtra(EXTRA_ATTIBUTE_ID_ARRAY, barray);
        mContext.sendBroadcast(intent, BLUETOOTH_PERM);
        mPendingCmds.add(new Integer(AvrcpConstants_ext.GET_ATTRIBUTE_VALUES));
    }

    public void setPlayerAppSetting( byte num, byte [] attr_id, byte [] attr_val,
            byte[] address) {
        byte[] array = new byte[num*2];
        for ( int i = 0; i < num; i++)
        {
            array[i] = attr_id[i] ;
            array[i+1] = attr_val[i];
            mPendingSetAttributes.add(new Integer(attr_id[i]));
        }
        Intent intent = new Intent(PLAYERSETTINGS_REQUEST);
        intent.putExtra(COMMAND, CMDSET);
        intent.putExtra(EXTRA_ATTRIB_VALUE_PAIRS, array);
        mContext.sendBroadcast(intent, BLUETOOTH_PERM);
        mPendingCmds.add(new Integer(AvrcpConstants_ext.SET_ATTRIBUTE_VALUES));
    }

    public void getplayerattribute_text(byte attr , byte [] attrIds,
            byte[] address) {
        Intent intent = new Intent(PLAYERSETTINGS_REQUEST);
        intent.putExtra(COMMAND, CMDGET);
        intent.putExtra(EXTRA_GET_COMMAND, AvrcpConstants_ext.GET_ATTRIBUTE_TEXT);
        intent.putExtra(EXTRA_ATTIBUTE_ID_ARRAY, attrIds);
        mPlayerSettings.attrIds = new byte [attr];
        for (int i = 0; i < attr; i++)
            mPlayerSettings.attrIds[i] = attrIds[i];
        mContext.sendBroadcast(intent, BLUETOOTH_PERM);
        mPendingCmds.add(new Integer(AvrcpConstants_ext.GET_ATTRIBUTE_TEXT));
   }

    public void getplayervalue_text(byte attr_id , byte num_value , byte [] value,
            byte[] address) {
        Intent intent = new Intent(PLAYERSETTINGS_REQUEST);
        intent.putExtra(COMMAND, CMDGET);
        intent.putExtra(EXTRA_GET_COMMAND, AvrcpConstants_ext.GET_VALUE_TEXT);
        intent.putExtra(EXTRA_ATTRIBUTE_ID, attr_id);
        intent.putExtra(EXTRA_VALUE_ID_ARRAY, value);
        mPlayerSettings.attrIds = new byte [num_value];
        for (int i = 0; i < num_value; i++)
            mPlayerSettings.attrIds[i] = value[i];
        mContext.sendBroadcast(intent, BLUETOOTH_PERM);
        mPendingCmds.add(new Integer(AvrcpConstants_ext.GET_VALUE_TEXT));
    }

    public void sendPlayerAppChangedRsp(int rsptype, BluetoothDevice device) {
        int j = 0;
        byte i = NUMPLAYER_ATTRIBUTE*2;
        byte [] retVal = new byte [i];
        retVal[j++] = ATTRIBUTE_REPEATMODE;
        retVal[j++] = settingValues.repeat_value;
        retVal[j++] = ATTRIBUTE_SHUFFLEMODE;
        retVal[j++] = settingValues.shuffle_value;
        mAvrcpPlayerAppSettingsRspInterface.registerNotificationPlayerAppRsp(rsptype,
                i, retVal, getByteAddress(device));
    }

    private void updateLocalPlayerSettings( byte[] data) {
        if (DEBUG) Log.v(TAG, "updateLocalPlayerSettings");
        for (int i = 0; i < data.length; i += 2) {
            if (DEBUG) Log.v(TAG, "ID: " + data[i] + " Value: " + data[i+1]);
            switch (data[i]) {
                case ATTRIBUTE_EQUALIZER:
                    settingValues.eq_value = data[i+1];
                break;
                case ATTRIBUTE_REPEATMODE:
                    settingValues.repeat_value = data[i+1];
                break;
                case ATTRIBUTE_SHUFFLEMODE:
                    settingValues.shuffle_value = data[i+1];
                break;
                case ATTRIBUTE_SCANMODE:
                    settingValues.scan_value = data[i+1];
                break;
            }
        }
    }

    private boolean checkPlayerAttributeResponse( byte[] data) {
        boolean ret = false;
        if (DEBUG) Log.v(TAG, "checkPlayerAttributeResponse");
        for (int i = 0; i < data.length; i += 2) {
            if (DEBUG) Log.v(TAG, "ID: " + data[i] + " Value: " + data[i+1]);
            switch (data[i]) {
                case ATTRIBUTE_EQUALIZER:
                    if (mPendingSetAttributes.contains(new Integer(ATTRIBUTE_EQUALIZER))) {
                        Log.v(TAG, "Pending SetAttribute contains Equalizer");
                        if(data[i+1] == ATTRIBUTE_NOTSUPPORTED) {
                            ret = false;
                        } else {
                            ret = true;
                        }
                    }
                break;
                case ATTRIBUTE_REPEATMODE:
                    if (mPendingSetAttributes.contains(new Integer(ATTRIBUTE_REPEATMODE))) {
                        Log.v(TAG, "Pending SetAttribute contains Repeat");
                        if(data[i+1] == ATTRIBUTE_NOTSUPPORTED) {
                            ret = false;
                        } else {
                            ret = true;
                        }
                    }
                break;
                case ATTRIBUTE_SHUFFLEMODE:
                    if (mPendingSetAttributes.contains(new Integer(ATTRIBUTE_SHUFFLEMODE))) {
                        Log.v(TAG, "Pending SetAttribute contains Shuffle");
                        if(data[i+1] == ATTRIBUTE_NOTSUPPORTED) {
                            ret = false;
                        } else {
                            ret = true;
                        }
                    }
                break;
            }
        }
        return ret;
    }

    public long getPlayerAppSettingsCmdDelay() {
        return mPlayerAppSettingsCmdDelay;
    }

    private byte[] getByteAddress(BluetoothDevice device) {
        return Utils.getBytesFromAddress(device.getAddress());
    }
}
