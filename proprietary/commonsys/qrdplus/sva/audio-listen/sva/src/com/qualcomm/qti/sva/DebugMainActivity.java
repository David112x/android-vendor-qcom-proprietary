/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva;

import android.content.Context;
import android.content.Intent;
import android.graphics.Typeface;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.listen.ListenTypes;
import com.qualcomm.qti.sva.controller.DetectionEventContainer;
import com.qualcomm.qti.sva.controller.Global;
import com.qualcomm.qti.sva.controller.Msg;
import com.qualcomm.qti.sva.controller.MsgResponse;
import com.qualcomm.qti.sva.data.IExtendedSmModel;
import com.qualcomm.qti.sva.data.IKeyphraseModel;
import com.qualcomm.qti.sva.data.ISettingsModel;
import com.qualcomm.qti.sva.data.IUserModel;
import com.qualcomm.qti.sva.data.SettingsModel;
import com.qualcomm.qti.sva.service.WakeupService;
import com.qualcomm.qti.sva.utils.LogUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;

public class DebugMainActivity extends ServiceActivity {
    private final String TAG = DebugMainActivity.class.getSimpleName();

    private Messenger mClientMessenger;

    // UI view variable define
    private LinearLayout mLayoutRoot;
    private LinearLayout mLayoutSelectSoundModel;
    private ImageView mIvMic;
    private LinearLayout mLayoutAdvancedDetail;
    private ImageView mIvReset;
    private TextView mTvDetectionCount;
    private TextView mTvKeyphraseCount;
    private TextView mTvUserCount;
    private TextView mTvVoiceRequestsCount;
    private LinearLayout mLayoutSessionInfo;

    // detection statistics variables
    private int mDetectionSuccessCounter = 0;
    private int mDetectionFailureCounter = 0;
    private int mKeyphraseDetectionCounter = 0;
    private int mUserDetectionCounter = 0;
    private int mVoiceRequestsCounter = 0;

    //advanced details table layout variables
    private Map<String, View> mKeyphraseUserViewMap = new HashMap<>();

    private Timer mTimer;
    private String mLastVoiceRequestFilePath;

    private Handler mHandler = new Handler(new Handler.Callback() {
        @Override
        public boolean handleMessage(Message msg) {
            LogUtils.d(TAG, "handleMessage: msg.what = " + msg.what);
            switch (msg.what) {
                case Msg.MSG_REGISTER_CLIENT_RSP:
                    break;
                case Msg.MSG_LOAD_SOUND_MODEL_RSP:
                    onReceivedLoadSmRsp((MsgResponse.Result) msg.obj);
                    break;
                case Msg.MSG_START_RECOGNITION_RSP:
                    onReceivedStartRecognitionRsp((MsgResponse.Result) msg.obj);
                    break;
                case Msg.MSG_UNLOAD_SOUND_MODEL_RSP:
                    onReceivedUnloadSmRsp((MsgResponse.Result) msg.obj);
                    break;
                case Msg.MSG_STOP_RECOGNITION_RSP:
                    onReceivedStopRecognitionRsp((MsgResponse.Result) msg.obj);
                    break;
                case Msg.MSG_VOICE_REQUEST_IND:
                    onReceivedVoiceRequestInd((String) msg.obj);
                    break;
                case Msg.MSG_DETECTION_IND:
                    onReceivedDetectionInd((DetectionEventContainer) msg.obj);
                    break;
                default:
                    break;
            }
            return true;
        }
    });

    private void onReceivedStopRecognitionRsp(MsgResponse.Result result) {
        LogUtils.d(TAG, "onReceivedStopRecognitionRsp: result = " + result);
        if (MsgResponse.Result.SUCCESS == result) {
            mIvMic.setImageResource(R.drawable.mic_off);
        }
    }

    private void onReceivedUnloadSmRsp(MsgResponse.Result result) {
        LogUtils.d(TAG, "onReceivedUnloadSmRsp: result = " + result);
        if (MsgResponse.Result.SUCCESS == result) {
            initSessionTableLayout();
        }
    }

    private void onReceivedDetectionInd(DetectionEventContainer container) {
        LogUtils.d(TAG, "onReceivedDetectionInd: enter");
        updateSessionTableLayout(container);
    }

    private void onReceivedStartRecognitionRsp(MsgResponse.Result result) {
        LogUtils.d(TAG, "onReceivedStartRecognitionRsp: result = " + result);
        if (MsgResponse.Result.SUCCESS == result) {
            mIvMic.setImageResource(R.drawable.mic_on);
        }
    }

    private void onReceivedLoadSmRsp(MsgResponse.Result result) {
        LogUtils.d(TAG, "onReceivedLoadSmRsp: result = " + result);
        if (MsgResponse.Result.SUCCESS == result) {
            resetProgressBarsAndCounters();
            initSessionTableLayout();
        }
    }

    private void onReceivedVoiceRequestInd(String filePath) {
        LogUtils.d(TAG, "onReceivedVoiceRequestInd: filePath = " + filePath);
        mLastVoiceRequestFilePath = filePath;
        mVoiceRequestsCounter++;
        mTvVoiceRequestsCount.setText(String.valueOf(mVoiceRequestsCounter));
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mClientMessenger = new Messenger(mHandler);
        setContentView(R.layout.activity_debug_main);

        Intent intent = new Intent(this, WakeupService.class);
        bindService(intent, getServiceConnection(), Context.BIND_AUTO_CREATE);

        // no service request for UI, so can initialize UI here
        initializeUI();
    }

    @Override
    protected void onResume() {
        super.onResume();

        // update visibility for advanced detail
        setAdvanceDetailVisibility();

        // update mic image
        if (Global.getInstance().getExtendedSmMgr().hasActiveSessions()) {
            mIvMic.setImageResource(R.drawable.mic_on);
        } else {
            mIvMic.setImageResource(R.drawable.mic_off);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        if (isServiceConnected()) {
            deRegisterClient(Msg.ClientID.CLIENT_ID_DEBUG_MAIN_ACTIVITY);
            unbindService(getServiceConnection());
        }

        Global.getInstance().getRecordingsMgr().stopPlaybackRecording();

        if (null != mHandler) {
            mHandler.removeCallbacksAndMessages(null);
        }

        super.onDestroy();
    }

    @Override
    protected void onServiceConnectedCallback() {
        registerClient(Msg.ClientID.CLIENT_ID_DEBUG_MAIN_ACTIVITY);
    }

    @Override
    protected void onServiceDisconnectedCallback() {
        deRegisterClient(Msg.ClientID.CLIENT_ID_DEBUG_MAIN_ACTIVITY);
    }

    @Override
    protected Messenger getClientMessenger() {
        return mClientMessenger;
    }

    private void initializeUI() {
        mLayoutRoot = findViewById(R.id.layout_root);
        mLayoutSelectSoundModel = findViewById(R.id.layout_select_sound_model);
        mLayoutSelectSoundModel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                LogUtils.d(TAG, "onClick: select a sound model is clicked");
                Intent intent = new Intent(getApplicationContext(),
                        DebugModelListActivity.class);
                startActivity(intent);
            }
        });

        mIvMic = findViewById(R.id.image_microphone);
        mLayoutAdvancedDetail = findViewById(R.id.layout_advanced_details);
        setAdvanceDetailVisibility();
        mIvReset = findViewById(R.id.image_reset_detections);
        mIvReset.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                LogUtils.d(TAG, "onClick: reset is clicked");
                resetProgressBarsAndCounters();
            }
        });
        mIvReset.setVisibility(View.INVISIBLE);

        mTvDetectionCount = findViewById(R.id.tv_detection_count);
        mTvKeyphraseCount = findViewById(R.id.tv_keyphrase_count);
        mTvUserCount = findViewById(R.id.tv_user_count);
        mTvVoiceRequestsCount = findViewById(R.id.tv_voice_request_count);
        mLayoutSessionInfo = findViewById(R.id.layout_session_info);
        initSessionTableLayout();
    }

    public void playLastVoiceRequest(View view) {
        LogUtils.d(TAG, "playLastVoiceRequest: enter");
        if (null == mLastVoiceRequestFilePath) {
            Toast.makeText(getApplicationContext(),
                    getString(R.string.no_voice_request), Toast.LENGTH_LONG).show();
            return;
        }

        Global.getInstance().getRecordingsMgr().playbackRecording(
                getApplicationContext(), mLastVoiceRequestFilePath);
    }

    private void setAdvanceDetailVisibility() {
        ISettingsModel settingsModel = new SettingsModel(this, null);
        boolean bDisplay = settingsModel.getGlobalIsDisplayAdvancedDetails();
        if (bDisplay) {
            mLayoutAdvancedDetail.setVisibility(View.VISIBLE);
        } else {
            mLayoutAdvancedDetail.setVisibility(View.INVISIBLE);
        }

        mLayoutRoot.setBackgroundColor(bDisplay
                ? getResources().getColor(R.color.bg_home, null)
                : getResources().getColor(R.color.bg_content, null));
    }

    private void resetProgressBarsAndCounters() {
        LogUtils.d(TAG, "resetProgressBarsAndCounters: enter");
        mDetectionSuccessCounter = 0;
        mDetectionFailureCounter = 0;
        mKeyphraseDetectionCounter = 0;
        mUserDetectionCounter = 0;
        mVoiceRequestsCounter = 0;

        mTvDetectionCount.setText(String.valueOf(mDetectionSuccessCounter));
        mTvKeyphraseCount.setText(String.valueOf(mKeyphraseDetectionCounter));
        mTvUserCount.setText(String.valueOf(mUserDetectionCounter));
        mTvVoiceRequestsCount.setText(String.valueOf(mVoiceRequestsCounter));

        View view;
        for (Map.Entry<String, View> item : mKeyphraseUserViewMap.entrySet()) {
            view = item.getValue();
            TextView tvDetectedCount = view.findViewById(R.id.tv_detected_count);
            tvDetectedCount.setText(getString(R.string.zero));

            TextView tvConfidenceLevel = view.findViewById(R.id.tv_confidence_level);
            tvConfidenceLevel.setText(getString(R.string.zero));
        }
    }

    private void initSessionTableLayout() {
        mLayoutSessionInfo.removeAllViews();
        mKeyphraseUserViewMap.clear();
        mIvReset.setVisibility(View.VISIBLE);

        ArrayList<IExtendedSmModel> loadedList
                = Global.getInstance().getExtendedSmMgr().getAllLoadedSoundModelList();
        int size = loadedList.size();
        IExtendedSmModel soundModel;
        for (int i = 0; i < size; i++) {
            soundModel = loadedList.get(i);
            View titleTableRow = getLayoutInflater().inflate(
                    R.layout.table_row_advanceddetail_title, null);
            TextView titleView = titleTableRow.findViewById(R.id.tv_sm_pretty_name);
            String title = getString(R.string.session) + String.valueOf(i + 1) + ":"
                    + soundModel.getSoundModelPrettyFileName();
            titleView.setText(title);
            titleView.setTypeface(null, Typeface.BOLD);
            mLayoutSessionInfo.addView(titleTableRow);

            // set session id
            soundModel.setSessionId(i + 1);

            RelativeLayout.LayoutParams layoutParams = null;
            for (IKeyphraseModel keyphraseItem : soundModel.getSoundModelKeyphraseList()) {
                View keyphraseTableRow = getLayoutInflater().inflate(
                        R.layout.table_row_advanceddetail_info, null);

                TextView tvKeyphrase = keyphraseTableRow.findViewById(
                        R.id.tv_keyphrase_or_user);
                TextView tvKeyphraseFlag = keyphraseTableRow.findViewById(
                        R.id.tv_keyphrase_or_user_flag);
                tvKeyphrase.setText(keyphraseItem.getKeyphraseFullName());
                tvKeyphraseFlag.setText(R.string.keyphrase);
                mLayoutSessionInfo.addView(keyphraseTableRow);

                String keyphraseViewKey = generateViewKey(soundModel.getSessionId(),
                        keyphraseItem.getKeyphraseFullName(), null);
                mKeyphraseUserViewMap.put(keyphraseViewKey, keyphraseTableRow);

                for (IUserModel userItem : keyphraseItem.getUserList()) {
                    View userTableRow = getLayoutInflater().inflate(
                            R.layout.table_row_advanceddetail_info, null);
                    TextView tvUser = userTableRow.findViewById(R.id.tv_keyphrase_or_user);
                    TextView tvUserFlag = userTableRow.findViewById(R.id.tv_keyphrase_or_user_flag);
                    tvUser.setText(userItem.getUserName());
                    tvUserFlag.setText(R.string.user);
                    mLayoutSessionInfo.addView(userTableRow);

                    String userViewKey = generateViewKey(soundModel.getSessionId(),
                            keyphraseItem.getKeyphraseFullName(), userItem.getUserName());
                    mKeyphraseUserViewMap.put(userViewKey, userTableRow);
                }
            }
        }
    }

    private void updateSessionTableLayout(DetectionEventContainer container) {
        if (null == container) {
            LogUtils.d(TAG, "updateSessionTableLayout: invalid input param");
            return;
        }
        ListenTypes.VoiceWakeupDetectionDataV2 detectionData = container.getDetectionData();
        boolean bSuccess = (ListenTypes.STATUS_SUCCESS == detectionData.status);
        if (bSuccess) {
            ++mDetectionSuccessCounter;
        } else {
            ++mDetectionFailureCounter;
        }
        mTvDetectionCount.setText(
                String.valueOf(mDetectionSuccessCounter + mDetectionFailureCounter));

        int sessionId = container.getSessionId();
        for (ListenTypes.VWUKeywordConfLevel keyphraseItem : detectionData.nonzeroKWConfLevels) {
            updateTableRow(bSuccess, sessionId, keyphraseItem.keyword,
                    null, keyphraseItem.confLevel);
        }

        for (ListenTypes.VWUUserKeywordPairConfLevel
                userItem : detectionData.nonzeroUserKWPairConfLevels) {
            updateTableRow(bSuccess, sessionId, userItem.keyword,
                    userItem.user, userItem.confLevel);
        }

        // update keyphrase and user detection counter
        mTvKeyphraseCount.setText(String.valueOf(mKeyphraseDetectionCounter));
        mTvUserCount.setText(String.valueOf(mUserDetectionCounter));
    }

    private void updateTableRow(boolean bSuccess, int sessionId,
                                String keyphrase, String userName, int confidenceLevel) {

        // for MKU only one keyphrase report the confidence level value
        if (0 == confidenceLevel) {
            LogUtils.d(TAG, "updateTableRow: return for confidence level is zero");
            return;
        }

        if (bSuccess) {
            if (null == userName) {
                mKeyphraseDetectionCounter++;
            } else {
                mUserDetectionCounter++;
            }
        }

        String key = generateViewKey(sessionId, keyphrase, userName);
        View view = mKeyphraseUserViewMap.getOrDefault(key, null);
        if (null == view) {
            LogUtils.d(TAG, "updateTableRow: get view failed");
            return;
        }

        TextView tvDetectedCount = view.findViewById(R.id.tv_detected_count);
        int count = Integer.parseInt(tvDetectedCount.getText().toString());
        tvDetectedCount.setText(Integer.toString(++count));
        TextView tvConfidenceLevel = view.findViewById(R.id.tv_confidence_level);
        tvConfidenceLevel.setText(String.valueOf(confidenceLevel));
    }

    private String generateViewKey(int sessionId, String keyphrase, String userName) {
        return sessionId + ":" + keyphrase + ":" + userName;
    }
}
