/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

package com.qualcomm.qti.internal.nrNetworkService.hidl;

import android.hardware.radio.V1_0.RadioResponseInfo;
import android.hardware.radio.V1_0.SendSmsResult;
import android.hidl.manager.V1_0.IServiceManager;
import android.hidl.manager.V1_0.IServiceNotification;
import android.os.AsyncResult;
import android.os.HwBinder;
import android.os.RemoteException;
import android.util.Log;
import android.telephony.TelephonyManager;

import java.util.stream.IntStream;

import com.android.internal.telephony.TelephonyComponentFactory;
import com.qualcomm.qti.internal.telephony.QtiTelephonyComponentFactory;
import org.codeaurora.internal.BearerAllocationStatus;
import org.codeaurora.internal.DcParam;
import org.codeaurora.internal.NrConfigType;
import org.codeaurora.internal.SignalStrength;
import org.codeaurora.internal.Status;
import org.codeaurora.internal.Token;
import org.codeaurora.internal.UpperLayerIndInfo;
import org.codeaurora.internal.NrIconType;

import java.util.concurrent.ConcurrentHashMap;
import java.lang.NullPointerException;

import vendor.qti.hardware.radio.qtiradio.V1_0.QtiRadioResponseInfo;
import vendor.qti.hardware.radio.qtiradio.V1_0.QtiRadioError;

import vendor.qti.hardware.radio.qtiradio.V2_0.IQtiRadio;
import vendor.qti.hardware.radio.qtiradio.V2_3.IQtiRadioResponse;
import vendor.qti.hardware.radio.qtiradio.V2_2.IQtiRadioIndication;

public class QtiRadioHidlClient implements IHidlConnectionInterface {
    private static final String TAG = "QtiRadioHidlClient";

    private final int MAX_SLOTS = 2;
    private int mSerial = -1;

    private IHidlConnectionCallback mCallback;
    private final Token UNSOL = new Token(-1);
    private ConcurrentHashMap<Integer, Token> mInflightRequests = new ConcurrentHashMap<Integer,
            Token>();

    private int getPhoneCount() {
        return TelephonyManager.getDefault().getPhoneCount();
    }

    /* Overloaded adapters to translate HIDL data types to AIDL data types. */
    private DcParam convertHidl2Aidl(vendor.qti.hardware.radio.qtiradio.V2_0.DcParam dcParam) {
        return new DcParam(dcParam.endc, dcParam.dcnr);
    }

    private UpperLayerIndInfo convertHidl2Aidl(
            vendor.qti.hardware.radio.qtiradio.V2_1.UpperLayerIndInfo ulInfo) {
        return new UpperLayerIndInfo(ulInfo.plmnInfoList, ulInfo.upplerLayerInd);
    }

    private NrConfigType convertHidlConfigType2Aidl(int configType) {
        return new NrConfigType(configType);
    }

    private NrIconType convertHidlNrIconType2Aidl(int iconType) {
        return new NrIconType(iconType);
    }

    private SignalStrength convertHidl2Aidl(vendor.qti.hardware.radio.qtiradio.V2_0.SignalStrength
            signalStrength) {
        return new SignalStrength(signalStrength.rsrp, signalStrength.snr);
    }

    private Status convertHidl2Aidl(int rilErrorCode) {
        return new Status((rilErrorCode == 0)? Status.SUCCESS : Status.FAILURE);
    }

    private BearerAllocationStatus convertHidlBearerStatus2Aidl(int bearerStatus) {
        return new BearerAllocationStatus(bearerStatus);
    }

    private boolean isEnableOrDisableSucess(int errorCode) {
        return errorCode == QtiRadioError.NONE ? true : false;
    }

    public class QtiRadioResponse extends
            vendor.qti.hardware.radio.qtiradio.V2_3.IQtiRadioResponse.Stub {
        static final String TAG = "QtiRadioResponse";

        int mSlotId;

        public QtiRadioResponse(int slotId) {
            Log.d(TAG, "[" + slotId + "] Constructor: ");
            mSlotId = slotId;
        }

        @Override
        public void onEnable5gResponse(int serial, int errorCode, int status) {
            Log.d(TAG,"onEnable5gResponse: serial = " + serial + " errorCode = " + errorCode +
                    " " + "status = " + status);
            if (mInflightRequests.containsKey(serial)) {
                Token token = mInflightRequests.get(serial);
                boolean enabled = isEnableOrDisableSucess(errorCode);
                Log.d(TAG, "onEnable5gResponse: enabled = " + enabled);

                mCallback.on5gStatus(mSlotId, token, convertHidl2Aidl(errorCode), enabled);

                mInflightRequests.remove(serial);
            } else {
                Log.d(TAG, "onEnable5gResponse: No previous request found for serial = " +
                        serial);
            }
        }

        @Override
        public void onDisable5gResponse(int serial, int errorCode, int status) {
            Log.d(TAG, "onDisable5gResponse: serial = " + serial + " errorCode = " + errorCode +
                    " " + "status = " + status);
            if (mInflightRequests.containsKey(serial)) {
                Token token = mInflightRequests.get(serial);

                boolean enabled = isEnableOrDisableSucess(errorCode);
                Log.d(TAG, "onDisable5gResponse: enabled = " + !enabled);
                mCallback.on5gStatus(mSlotId, token, convertHidl2Aidl(errorCode), !enabled);

                mInflightRequests.remove(serial);
            } else {
                Log.d(TAG, "onDisable5gResponse: No previous request found for serial = " +
                        serial);
            }
        }

        @Override
        public void onEnable5gOnlyResponse(int serial, int errorCode, int status) {
            Log.d(TAG, "onEnable5gOnlyResponse: serial = " + serial + " errorCode = " + errorCode +
                    "" + " " + "status = " + status);
            if (mInflightRequests.containsKey(serial)) {
                Token token = mInflightRequests.get(serial);

                boolean enabled = isEnableOrDisableSucess(errorCode);
                Log.d(TAG, "onEnable5gOnlyResponse: enabled = " + enabled);
                mCallback.on5gStatus(mSlotId, token, convertHidl2Aidl(errorCode), enabled);

                mInflightRequests.remove(serial);
            } else {
                Log.d(TAG, "onEnable5gOnlyResponse: No previous request found for serial = " +
                        serial);
            }
        }

        @Override
        public void on5gStatusResponse(int serial, int errorCode, int enabled) {
            Log.d(TAG, "on5gStatusResponse: serial = " + serial + " errorCode = " + errorCode +
                    " " + "enabled = " + enabled);
            if (mInflightRequests.containsKey(serial)) {
                Token token = mInflightRequests.get(serial);

                boolean isEnabled = (enabled == vendor.qti.hardware.radio.qtiradio
                        .V2_0.EnableStatus.ENABLED);
                Log.d(TAG, "on5gStatusResponse: enabled = " + isEnabled);
                mCallback.on5gStatus(mSlotId, token, convertHidl2Aidl(errorCode), isEnabled);

                mInflightRequests.remove(serial);
            } else {
                Log.d(TAG, "on5gStatusResponse: No previous request found for serial = " +
                        serial);
            }
        }

        @Override
        public void onNrDcParamResponse(int serial, int errorCode,
                vendor.qti.hardware.radio.qtiradio.V2_0.DcParam dcParam) {
            Log.d(TAG, "onNrDcParamResponse: serial = " + serial + " errorCode = "
                    + errorCode + " dcParam = " + dcParam);
            if (mInflightRequests.containsKey(serial)) {
                Token token = mInflightRequests.get(serial);

                DcParam aidlDcParam = convertHidl2Aidl(dcParam);
                Log.d(TAG, "onNrDcParamResponse:  " + aidlDcParam);
                mCallback.onNrDcParam(mSlotId, token, convertHidl2Aidl(errorCode), aidlDcParam);

                mInflightRequests.remove(serial);
            } else {
                Log.d(TAG, "onNrDcParamResponse: No previous request found for serial = " +
                        serial);
            }
        }

        @Override
        public void onNrBearerAllocationResponse_2_1(int serial, int errorCode, int bearerStatus) {
            Log.d(TAG, "onNrBearerAllocationResponse: serial = " + serial + " errorCode = " +
                    errorCode + " bearerStatus = " + bearerStatus);
            if (mInflightRequests.containsKey(serial)) {
                Token token = mInflightRequests.get(serial);

                BearerAllocationStatus bStatus = convertHidlBearerStatus2Aidl(bearerStatus);

                Log.d(TAG, "onNrBearerAllocationResponse:  allocated = " + bStatus);
                mCallback.onAnyNrBearerAllocation(mSlotId, token, convertHidl2Aidl(errorCode),
                        bStatus);

                mInflightRequests.remove(serial);
            } else {
                Log.d(TAG, "onNrBearerAllocationResponse: No previous request found for serial = " +
                        serial);
            }
        }

        @Override
        public void onNrBearerAllocationResponse(int serial, int errorCode, int bearerStatus) {
            Log.d(TAG, "onNrBearerAllocationResponse: serial = " + serial + " errorCode = " +
                    errorCode + " bearerStatus = " + bearerStatus);
            onNrBearerAllocationResponse_2_1(serial, errorCode, bearerStatus);
        }


        @Override
        public void onUpperLayerIndInfoResponse(int serial, int errorCode,
                vendor.qti.hardware.radio.qtiradio.V2_1.UpperLayerIndInfo uliInfo) {
            Log.d(TAG, "onUpperLayerIndInfoResponse: serial = " + serial + " errorCode = " +
                    errorCode + " UpperLayerIndInfo = " + uliInfo);
            if (mInflightRequests.containsKey(serial)) {
                Token token = mInflightRequests.get(serial);

                UpperLayerIndInfo upperLayerInfo = convertHidl2Aidl(uliInfo);

                Log.d(TAG, "onUpperLayerIndInfoResponse:  upperLayerInfo = " + upperLayerInfo);
                mCallback.onUpperLayerIndInfo(mSlotId, token, convertHidl2Aidl(errorCode),
                        upperLayerInfo);
                mInflightRequests.remove(serial);
            } else {
                Log.d(TAG, "onUpperLayerIndInfoResponse: No previous request found for serial = " +
                        serial);
            }

        }

        @Override
        public void on5gConfigInfoResponse(int serial, int errorCode, int configType) {
            Log.d(TAG, "on5gConfigInfoResponse: serial = " + serial + " errorCode = " +
                    errorCode + " ConfigType = " + configType);
            if (mInflightRequests.containsKey(serial)) {
                Token token = mInflightRequests.get(serial);

                NrConfigType nrConfigType = convertHidlConfigType2Aidl(configType);

                Log.d(TAG, "on5gConfigInfoResponse:  NrConfigType = " + nrConfigType);
                mCallback.on5gConfigInfo(mSlotId, token, convertHidl2Aidl(errorCode),
                        nrConfigType);
                mInflightRequests.remove(serial);
            } else {
                Log.d(TAG, "on5gConfigInfoResponse: No previous request found for serial = " +
                        serial);
            }

        }

        @Override
        public void onNrIconTypeResponse(int serial, int errorCode, int iconType) {
            Log.d(TAG, "onNrIconTypeResponse: serial = " + serial + " errorCode = " +
                    errorCode + " iconType = " + iconType);
            if (mInflightRequests.containsKey(serial)) {
                Token token = mInflightRequests.get(serial);

                NrIconType nrIconType = convertHidlNrIconType2Aidl(iconType);

                Log.d(TAG, "onNrIconTypeResponse:  NrIconType = " + nrIconType);
                mCallback.onNrIconType(mSlotId, token, convertHidl2Aidl(errorCode),
                        nrIconType);
                mInflightRequests.remove(serial);
            } else {
                Log.d(TAG, "onNrIconTypeResponse: No previous request found for serial = " +
                        serial);
            }

        }

        @Override
        public void onSignalStrengthResponse(int serial, int errorCode,
                vendor.qti.hardware.radio.qtiradio.V2_0.SignalStrength signalStrength) {
            Log.d(TAG, "onSignalStrengthResponse: serial = " + serial + " errorCode = " +
                    errorCode + " signalStrength = " + signalStrength);
            if (mInflightRequests.containsKey(serial)) {
                Token token = mInflightRequests.get(serial);

                SignalStrength aidlSignalStrength = convertHidl2Aidl(signalStrength);

                Log.d(TAG, "onSignalStrengthResponse:  " + aidlSignalStrength);
                mCallback.onSignalStrength(mSlotId, token, convertHidl2Aidl(errorCode),
                        aidlSignalStrength);

                mInflightRequests.remove(serial);
            } else {
                Log.d(TAG, "onSignalStrengthResponse: No previous request found for serial = " +
                        serial);
            }
        }

        @Override
        public void onEnableEndcResponse(int serial, int errorCode, int status) {
            Log.d(TAG, "onEnableEndcResponse: serial = " + serial + " errorCode = " + errorCode +
                    " " + "status = " + status);
            if (mInflightRequests.containsKey(serial)) {
                Token token = mInflightRequests.get(serial);
                Log.d(TAG, "onEnableEndcResponse: status = " + status);
                mCallback.onEnableEndc(mSlotId, token, convertHidl2Aidl(errorCode));

                mInflightRequests.remove(serial);
            } else {
                Log.d(TAG, "onEnableEndcResponse: No previous request found for serial = " +
                        serial);
            }
        }

        @Override
        public void onEndcStatusResponse(int serial, int errorCode, int enabled) {
            Log.d(TAG, "onEndcStatusResponse: serial = " + serial + " errorCode = " + errorCode +
                    " " + "enabled = " + enabled);
            if (mInflightRequests.containsKey(serial)) {
                Token token = mInflightRequests.get(serial);

                boolean isEnabled = (enabled == vendor.qti.hardware.radio.qtiradio
                        .V2_3.EndcStatus.ENABLED);
                Log.d(TAG, "onEndcStatusResponse: enabled = " + isEnabled);
                mCallback.onEndcStatus(mSlotId, token, convertHidl2Aidl(errorCode), isEnabled);

                mInflightRequests.remove(serial);
            } else {
                Log.d(TAG, "onEndcStatusResponse: No previous request found for serial = " +
                        serial);
            }
        }

        @Override
        public void getAtrResponse(QtiRadioResponseInfo qtiResponseInfo, String atr) {
            Log.d(TAG, "getAtrResponse: NOP!!");

        }

        @Override
        public void sendCdmaSmsResponse(QtiRadioResponseInfo qtiResponseInfo, SendSmsResult sms) {
            Log.d(TAG, "sendCdmaSmsResponse: NOP!!");
        }

    }

    public class QtiRadioIndication extends
            vendor.qti.hardware.radio.qtiradio.V2_2.IQtiRadioIndication.Stub {
        static final String TAG = "QtiRadioIndication";

        int mSlotId;

        public QtiRadioIndication(int slotId) {
            Log.d(TAG, "[" + slotId + "]Constructor: ");
            mSlotId = slotId;
        }

        @Override
        public void on5gStatusChange(int enableStatus) {
            Log.d(TAG, "on5gStatusChange: slotId = " + mSlotId);
            if (mCallback != null) {
                boolean enabled = (enableStatus == vendor.qti.hardware.radio.qtiradio
                        .V2_0.EnableStatus.ENABLED);
                Log.d(TAG, "on5gStatusChange: enabled = " + enabled);
                mCallback.on5gStatus(mSlotId, UNSOL, new Status(Status.SUCCESS), enabled);
            }
        }

        @Override
        public void onNrDcParamChange(
                vendor.qti.hardware.radio.qtiradio.V2_0.DcParam dcParam) {
            Log.d(TAG, "onNrDcParamChange: slotId = " + mSlotId);
            DcParam aidlDcParam = convertHidl2Aidl(dcParam);
            Log.d(TAG, "onNrDcParamChange: " + aidlDcParam);
            mCallback.onNrDcParam(mSlotId, UNSOL, new Status(Status.SUCCESS), aidlDcParam);
        }

        @Override
        public void onNrBearerAllocationChange_2_1(int bearerStatus) {
            Log.d(TAG, "onNrBearerAllocationChange: slotId = " + mSlotId);
            BearerAllocationStatus bStatus = convertHidlBearerStatus2Aidl(bearerStatus);
            Log.d(TAG, "onNrBearerAllocationChange: bStatus = " + bStatus);
            mCallback.onAnyNrBearerAllocation(mSlotId, UNSOL, new Status(Status.SUCCESS), bStatus);
        }

        @Override
        public void onNrBearerAllocationChange(int bearerStatus) {
            onNrBearerAllocationChange_2_1(bearerStatus);
        }

        @Override
        public void onSignalStrengthChange(
                vendor.qti.hardware.radio.qtiradio.V2_0.SignalStrength signalStrength) {
            Log.d(TAG, "onSignalStrengthChange: slotId = " + mSlotId);
            SignalStrength aidlSignalStrength = convertHidl2Aidl(signalStrength);
            Log.d(TAG, "onSignalStrengthChange: " + signalStrength);
            mCallback.onSignalStrength(mSlotId, UNSOL, new Status(Status.SUCCESS),
                    aidlSignalStrength);
        }

        @Override
        public void onUpperLayerIndInfoChange(
                vendor.qti.hardware.radio.qtiradio.V2_1.UpperLayerIndInfo uliInfo) {
            Log.d(TAG, "onUpperLayerIndInfoChange: slotId = " + mSlotId);
            UpperLayerIndInfo upperLayerInfo = convertHidl2Aidl(uliInfo);
            Log.d(TAG, "onUpperLayerIndInfoChange:  upperLayerInfo = " + upperLayerInfo);
            mCallback.onUpperLayerIndInfo(mSlotId, UNSOL, new Status(Status.SUCCESS),
                    upperLayerInfo);
        }

        @Override
        public void on5gConfigInfoChange(int configType) {
            Log.d(TAG, "on5gConfigInfoChange: slotId = " + mSlotId);
            NrConfigType nrConfigType = convertHidlConfigType2Aidl(configType);
            Log.d(TAG, "on5gConfigInfoChange:  5gConfigType = " + nrConfigType);
            mCallback.on5gConfigInfo(mSlotId, UNSOL, new Status(Status.SUCCESS),
                    nrConfigType);
        }

        @Override
        public void onNrIconTypeChange(int iconType) {
            Log.d(TAG, "onNrIconTypeChange: slotId = " + mSlotId);
            NrIconType nrIconType = convertHidlNrIconType2Aidl(iconType);
            Log.d(TAG, "onNrIconTypeChange:  NrIconType = " + nrIconType);
            mCallback.onNrIconType(mSlotId, UNSOL, new Status(Status.SUCCESS),
                    nrIconType);
        }


        @Override
        public void qtiRadioIndication(int value) {
            Log.d(TAG, "qtiRadioIndication: NOP!!");
        }

    }


    private QtiTelephonyComponentFactory getQtiTelephonyComponentFactory() {
        return QtiTelephonyComponentFactory.getInstance();
    }

    private void setCallbacks(int slotId) {
        QtiTelephonyComponentFactory factory = getQtiTelephonyComponentFactory();
        Log.d(TAG, "ril[" + slotId + "]: " + factory.getRil(slotId));
        factory.getRil(slotId).setCallbacks(new QtiRadioResponse(slotId),
                new QtiRadioIndication(slotId));
    }

    private void register() {
        Log.d(TAG, "Register");
        int phones = getPhoneCount();
        Log.d(TAG, "Phone count = " + phones);
        IntStream.range(0, phones).forEach(i -> setCallbacks(i));
    }
    public QtiRadioHidlClient() {
        Log.d(TAG, "constructor");
        register();
    }

    private Token getNextToken() {
        return new Token(++mSerial);
    }

    @Override
    public Token enable5g(int slotId) throws RemoteException {
        Token token = getNextToken();
        Log.d(TAG, "enable5g: slotId = " + slotId + token);
        int serial = token.get();
        mInflightRequests.put(serial, token);
        try {
            getQtiTelephonyComponentFactory().getRil(slotId).enable5g(serial);
        } catch (RemoteException | NullPointerException e) {
            mInflightRequests.remove(serial, token);
            Log.d(TAG, "enable5g: slotId = " + slotId + " Exception = " + e);
            e.printStackTrace();
            //Rethrow same exception.
            throw e;
        }
        return token;
    }

    @Override
    public Token disable5g(int slotId) throws RemoteException {
        Token token = getNextToken();
        Log.d(TAG, "disable5g: slotId = " + slotId + token);
        int serial = token.get();
        mInflightRequests.put(serial, token);
        try {
            getQtiTelephonyComponentFactory().getRil(slotId).disable5g(serial);
        } catch (RemoteException | NullPointerException  e) {
            mInflightRequests.remove(serial, token);
            Log.d(TAG, "disable5g: slotId = " + slotId + " Exception = " + e);
            e.printStackTrace();
            //Rethrow same exception.
            throw e;
        }
        return token;
    }

    @Override
    public Token enable5gOnly(int slotId) throws RemoteException {
        Token token = getNextToken();
        Log.d(TAG, "enable5gOnly: slotId = " + slotId + token);
        int serial = token.get();
        mInflightRequests.put(serial, token);
        try {
            getQtiTelephonyComponentFactory().getRil(slotId).enable5gOnly(serial);
        } catch (RemoteException | NullPointerException  e) {
            mInflightRequests.remove(serial, token);
            Log.d(TAG, "enable5gOnly: slotId = " + slotId + " Exception = " + e);
            e.printStackTrace();
            //Rethrow same exception.
            throw e;
        }
        return token;
    }

    @Override
    public Token query5gStatus(int slotId) throws RemoteException {
        Token token = getNextToken();
        Log.d(TAG, "query5gStatus: slotId = " + slotId + token);
        int serial = token.get();
        mInflightRequests.put(serial, token);
        try {
            getQtiTelephonyComponentFactory().getRil(slotId).query5gStatus(serial);
        } catch (RemoteException | NullPointerException  e) {
            mInflightRequests.remove(serial, token);
            Log.d(TAG, "query5gStatus: slotId = " + slotId + " Exception = " + e);
            e.printStackTrace();
            //Rethrow same exception.
            throw e;
        }
        return token;
    }

    @Override
    public Token queryNrDcParam(int slotId) throws RemoteException {
        Token token = getNextToken();
        Log.d(TAG, "queryNrDcParam: slotId = " + slotId + token);
        int serial = token.get();
        mInflightRequests.put(serial, token);
        try {
            getQtiTelephonyComponentFactory().getRil(slotId).queryNrDcParam(serial);
        } catch (RemoteException | NullPointerException  e) {
            mInflightRequests.remove(serial, token);
            Log.d(TAG, "queryNrDcParam: slotId = " + slotId + " Exception = " + e);
            e.printStackTrace();
            //Rethrow same exception.
            throw e;
        }
        return token;
    }

    @Override
    public Token queryNrBearerAllocation(int slotId) throws RemoteException {
        Token token = getNextToken();
        Log.d(TAG, "queryNrBearerAllocation: slotId = " + slotId + token);
        int serial = token.get();
        mInflightRequests.put(serial, token);
        try {
            getQtiTelephonyComponentFactory().getRil(slotId).queryNrBearerAllocation(serial);
        } catch (RemoteException | NullPointerException  e) {
            mInflightRequests.remove(serial, token);
            Log.d(TAG, "queryNrBearerAllocation: slotId = " + slotId + " Exception = " + e);
            e.printStackTrace();
            //Rethrow same exception.
            throw e;
        }
        return token;
    }

    @Override
    public Token queryNrSignalStrength(int slotId) throws RemoteException {
        Token token = getNextToken();
        Log.d(TAG, "queryNrSignalStrength: slotId = " + slotId + token);
        int serial = token.get();
        mInflightRequests.put(serial, token);
        try {
            getQtiTelephonyComponentFactory().getRil(slotId).queryNrSignalStrength(serial);
        } catch (RemoteException | NullPointerException  e) {
            mInflightRequests.remove(serial, token);
            Log.d(TAG, "queryNrSignalStrength: slotId = " + slotId + " Exception = " + e);
            e.printStackTrace();
            //Rethrow same exception.
            throw e;
        }
        return token;
    }

    @Override
    public Token queryUpperLayerIndInfo(int slotId) throws RemoteException {
        Token token = getNextToken();
        Log.d(TAG, "queryUpperLayerIndInfo: slotId = " + slotId + " token " + token);
        int serial = token.get();
        mInflightRequests.put(serial, token);
        try {
            getQtiTelephonyComponentFactory().getRil(slotId).queryUpperLayerIndInfo(serial);
        } catch (RemoteException | NullPointerException  e) {
            mInflightRequests.remove(serial, token);
            Log.d(TAG, "queryUpperLayerIndInfo: slotId = " + slotId + " Exception = " + e);
            e.printStackTrace();
            //Rethrow same exception.
            throw e;
        }
        return token;
    }

    @Override
    public Token query5gConfigInfo(int slotId) throws RemoteException {
        Token token = getNextToken();
        Log.d(TAG, "query5gConfigInfo: slotId = " + slotId + " token " + token);
        int serial = token.get();
        mInflightRequests.put(serial, token);
        try {
            getQtiTelephonyComponentFactory().getRil(slotId).query5gConfigInfo(serial);
        } catch (RemoteException | NullPointerException  e) {
            mInflightRequests.remove(serial, token);
            Log.d(TAG, "query5gConfigInfo: slotId = " + slotId + " Exception = " + e);
            e.printStackTrace();
            //Rethrow same exception.
            throw e;
        }
        return token;
    }

    @Override
    public Token queryNrIconType(int slotId) throws RemoteException {
        Token token = getNextToken();
        Log.d(TAG, "queryNrIconType: slotId = " + slotId + " token " + token);
        int serial = token.get();
        mInflightRequests.put(serial, token);
        try {
            getQtiTelephonyComponentFactory().getRil(slotId).queryNrIconType(serial);
        } catch (RemoteException | NullPointerException  e) {
            mInflightRequests.remove(serial, token);
            Log.d(TAG, "queryNrIconType: slotId = " + slotId + " Exception = " + e);
            e.printStackTrace();
            //Rethrow same exception.
            throw e;
        }
        return token;
    }

    @Override
    public Token enableEndc(int slotId, boolean enable) throws RemoteException {
        Token token = getNextToken();
        Log.d(TAG, "enableEndc: slotId = " + slotId + " token " + token);
        int serial = token.get();
        mInflightRequests.put(serial, token);
        try {
            getQtiTelephonyComponentFactory().getRil(slotId).enableEndc(serial, enable);
        } catch (RemoteException | NullPointerException  e) {
            mInflightRequests.remove(serial, token);
            Log.d(TAG, "enableEndc: slotId = " + slotId + " Exception = " + e);
            e.printStackTrace();
            //Rethrow same exception.
            throw e;
        }
        return token;
    }

    @Override
    public Token queryEndcStatus(int slotId) throws RemoteException {
        Token token = getNextToken();
        Log.d(TAG, "queryEndcStatus: slotId = " + slotId + " token " + token);
        int serial = token.get();
        mInflightRequests.put(serial, token);
        try {
            getQtiTelephonyComponentFactory().getRil(slotId).queryEndcStatus(serial);
        } catch (RemoteException | NullPointerException  e) {
            mInflightRequests.remove(serial, token);
            Log.d(TAG, "queryEndcStatus: slotId = " + slotId + " Exception = " + e);
            e.printStackTrace();
            //Rethrow same exception.
            throw e;
        }
        return token;
    }



    @Override
    public void registerCallback(IHidlConnectionCallback callback) {
        Log.d(TAG, "registerCallback: callback = " + callback);
        mCallback = callback;
    }

    @Override
    public void unRegisterCallback(IHidlConnectionCallback callback) {
        Log.d(TAG, "unRegisterCallback: callback = " + callback);
        if (mCallback == callback) {
            mCallback = null;
        }
    }
}
