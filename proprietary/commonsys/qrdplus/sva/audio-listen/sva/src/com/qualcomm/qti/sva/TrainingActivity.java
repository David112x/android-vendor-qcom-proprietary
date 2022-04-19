/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.qualcomm.listen.ListenSoundModel;
import com.qualcomm.listen.ListenTypes;
import com.qualcomm.qti.sva.controller.ExtendedSmMgr;
import com.qualcomm.qti.sva.controller.Global;
import com.qualcomm.qti.sva.controller.RecordingsMgr;
import com.qualcomm.qti.sva.data.IExtendedSmModel;
import com.qualcomm.qti.sva.data.ISettingsModel;
import com.qualcomm.qti.sva.data.ISmModel;
import com.qualcomm.qti.sva.data.SettingsModel;
import com.qualcomm.qti.sva.data.ISmModel.MODEL_VERSION;
import com.qualcomm.qti.sva.utils.FileUtils;
import com.qualcomm.qti.sva.utils.LogUtils;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ShortBuffer;
import java.util.Timer;
import java.util.TimerTask;

public class TrainingActivity extends Activity {
    public static final String KEY_EXTRA_PREVIOUS_SM_NAME = "previous_sm_name";
    public static final String KEY_EXTRA_KEYPHRASE_OR_NEW_SM_NAME = "keyphrase_or_new_sm_name";
    public static final String KEY_EXTRA_USER_NAME = "user_name";
    public static final String KEY_EXTRA_IS_ADD_USER_TO_PREVIOUS_MODEL
            = "is_add_user_to_previous_sm";
    private final static int SNR_THRESHOLD = 16;
    private final static int DURATION_OF_TRAINING_RECORDING = 3000; // 3s
    private final static int MSG_STOP_TRAINING_RECORDING = 1;

    private final String TAG = TrainingActivity.class.getSimpleName();

    private ExtendedSmMgr mExtendedSmMgr;
    private RecordingsMgr mRecordingMgr;
    private String mPreviousSmName;
    private String mKeyphraseOrNewSmName;
    private String mUserName;
    private boolean mIsAddUserToPreviousSm;
    private ISmModel.MODEL_VERSION mUdkVersion;
    private RecordingCounter mRecordingCounter;
    private ImageView[] mImageViewArray;

    // view variable define
    private TextView mTvUserName;
    private View mViewRecordingBackground;
    private RelativeLayout mLayoutRecording;
    private TextView mTvRecordingStatus;
    private ImageButton mIbTerminateTraining;
    private TextView mTvKeyphrase;
    private TextView mTvRecordingResult;
    private Button mButtonContinue;
    private RelativeLayout mLayoutMic;
    private ImageButton mIbMic;

    private Timer mRecordingTimer;
    private boolean mIsTraining = false;
    private VerifyRecordingTask mVerifyRecordingTask;
    private CreateSmTask mCreateSmTask;
    private AlertDialog mProgressDialog;
    private AlertDialog mCreateSmFailDialog;
    private AlertDialog mTerminateTrainingDialog;

    private Handler mHandler = new Handler(new Handler.Callback() {
        @Override
        public boolean handleMessage(Message msg) {
            LogUtils.d(TAG, "handleMessage: what = " + msg.what);
            switch (msg.what) {
                case MSG_STOP_TRAINING_RECORDING:
                    onReceivedStopTrainingRecordingMsg();
                    break;
                default:
                    break;
            }
            return true;
        }
    });

    private void onReceivedStopTrainingRecordingMsg() {
        mRecordingMgr.stopTrainingRecording(
                mRecordingMgr.getLastUserRecordingFilePath());
        mTvRecordingStatus.setText(getString(R.string.processing));
        mIbMic.setImageResource(R.drawable.speaker_off);

        // verify recording
        mVerifyRecordingTask = new VerifyRecordingTask();
        mVerifyRecordingTask.execute();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Intent intent = getIntent();
        mPreviousSmName = intent.getStringExtra(KEY_EXTRA_PREVIOUS_SM_NAME);
        mKeyphraseOrNewSmName = intent.getStringExtra(KEY_EXTRA_KEYPHRASE_OR_NEW_SM_NAME);
        mUserName = intent.getStringExtra(KEY_EXTRA_USER_NAME);
        mIsAddUserToPreviousSm = intent.getBooleanExtra(KEY_EXTRA_IS_ADD_USER_TO_PREVIOUS_MODEL,
                false);
        mUdkVersion = getUdkVersion();
        LogUtils.d(TAG, "onCreate: mPreviousSmName = " + mPreviousSmName
                + " mKeyphraseOrNewSmName = " + mKeyphraseOrNewSmName
                + " mUserName = " + mUserName
                + " mIsAddUserToPreviousSm = " + mIsAddUserToPreviousSm);

        mExtendedSmMgr = Global.getInstance().getExtendedSmMgr();
        mRecordingMgr = Global.getInstance().getRecordingsMgr();

        setContentView(R.layout.activity_training);
        initializeUI();

        mImageViewArray = new ImageView[]{
                findViewById(R.id.iv_recording_one),
                findViewById(R.id.iv_recording_two),
                findViewById(R.id.iv_recording_three),
                findViewById(R.id.iv_recording_four),
                findViewById(R.id.iv_recording_five)
        };
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        setIntent(intent);
        LogUtils.d(TAG, "onNewIntent: intent = " + intent.toString());
    }

    @Override
    protected void onRestart() {
        super.onRestart();

        // stop ongoing training if has
        stopTraining();

        // Get the new intent
        Intent intent = getIntent();
        mPreviousSmName = intent.getStringExtra(KEY_EXTRA_PREVIOUS_SM_NAME);
        mKeyphraseOrNewSmName = intent.getStringExtra(KEY_EXTRA_KEYPHRASE_OR_NEW_SM_NAME);
        mUserName = intent.getStringExtra(KEY_EXTRA_USER_NAME);
        mIsAddUserToPreviousSm = intent.getBooleanExtra(KEY_EXTRA_IS_ADD_USER_TO_PREVIOUS_MODEL,
                false);
        mUdkVersion = getUdkVersion();

        LogUtils.d(TAG, "onRestart: mPreviousSmName = " + mPreviousSmName
                + " mKeyphraseOrNewSmName = " + mKeyphraseOrNewSmName
                + " mUserName = " + mUserName
                + " mIsAddUserToPreviousSm = " + mIsAddUserToPreviousSm);

        // set keyphrase/userName to new keyphrase/UserName
        mTvKeyphrase.setText(getKeyphrase());
        mTvUserName.setText(mUserName);
    }

    @Override
    protected void onStop() {
        super.onStop();

        LogUtils.d(TAG, "onStop: enter");
        stopTraining();
    }

    @Override
    protected void onDestroy() {
        stopRecordingTimer();

        // clear the user recording mem data
        mRecordingMgr.removeUserRecordings();

        if (mHandler.hasMessages(MSG_STOP_TRAINING_RECORDING)) {
            mHandler.removeMessages(MSG_STOP_TRAINING_RECORDING);
        }

        if (null != mVerifyRecordingTask && mVerifyRecordingTask.isCancelled()) {
            mVerifyRecordingTask.cancel(true);
            mVerifyRecordingTask = null;
        }

        if (null != mCreateSmTask && mCreateSmTask.isCancelled()) {
            mCreateSmTask.cancel(true);
            mCreateSmTask = null;
        }

        if (null != mProgressDialog && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
            mProgressDialog = null;
        }

        if (null != mCreateSmFailDialog && mCreateSmFailDialog.isShowing()) {
            mCreateSmFailDialog.dismiss();
            mCreateSmFailDialog = null;
        }

        if (null != mTerminateTrainingDialog && mTerminateTrainingDialog.isShowing()) {
            mTerminateTrainingDialog.dismiss();
            mTerminateTrainingDialog = null;
        }

        if (null != mHandler) {
            mHandler.removeCallbacksAndMessages(null);
        }

        super.onDestroy();
    }

    private void initializeUI() {
        mTvUserName = findViewById(R.id.tv_user_name);
        mTvUserName.setText(mUserName);

        mViewRecordingBackground = findViewById(R.id.view_recording_background);
        mLayoutRecording = findViewById(R.id.layout_recording);
        mTvRecordingStatus = findViewById(R.id.tv_recording_status);

        mIbTerminateTraining = findViewById(R.id.ib_terminate_training);
        mIbTerminateTraining.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                LogUtils.d(TAG, "initializeUI:onClick: close training button is clicked");
                showTerminateTrainingDialog();
            }
        });

        mTvKeyphrase = findViewById(R.id.tv_keyphrase);
        mTvKeyphrase.setText(getKeyphrase());

        mTvRecordingResult = findViewById(R.id.tv_recording_result);

        mButtonContinue = findViewById(R.id.b_continue);
        mButtonContinue.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                LogUtils.d(TAG, "initializeUI:onClick: continue button is clicked");
                mCreateSmTask = new CreateSmTask();
                mCreateSmTask.execute();
                showProgressDialog(true);
            }
        });

        mLayoutMic = findViewById(R.id.layout_mic);
        mIbMic = findViewById(R.id.ib_mic);
        mIbMic.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                LogUtils.d(TAG, "initializeUI:onClick: mic image button is clicked");
                enterRecording();
            }
        });
    }

    private void updateUIVisible(boolean bRecording) {
        if (bRecording) {
            mLayoutRecording.setVisibility(View.VISIBLE);
            mViewRecordingBackground.setVisibility(View.VISIBLE);
            mTvRecordingResult.setVisibility(View.INVISIBLE);
            mButtonContinue.setVisibility(View.INVISIBLE);
            mIbMic.setImageResource(R.drawable.speaker_on);
            mIbMic.setClickable(false);
        } else {
            mLayoutRecording.setVisibility(View.INVISIBLE);
            mViewRecordingBackground.setVisibility(View.INVISIBLE);

            mIbMic.setImageResource(R.drawable.speaker_off);
            mIbMic.setClickable(true);
            mLayoutMic.setVisibility(View.VISIBLE);
        }
    }

    private void stopTraining(){
        if (mIsTraining) {
            // reset the training
            mIsTraining = false;
            reset();
            updateUIVisible(false);
            mRecordingMgr.stopTrainingRecording(
                    mRecordingMgr.getLastUserRecordingFilePath());
            stopRecordingTimer();
        }
    }

    private void enterRecording() {
        LogUtils.d(TAG, "enterRecording: enter");
        mIsTraining = true;
        reset();
        updateUIVisible(true);
        mRecordingMgr.startTrainingRecording();
        startRecordingTimer();
        mTvRecordingStatus.setText(getString(R.string.recording));
        mIbMic.setImageResource(R.drawable.speaker_on);
    }

    private void reset() {
        mRecordingMgr.removeAllTrainingRecordingFiles();
        mRecordingMgr.removeUserRecordings();
        try {
            mRecordingCounter = new RecordingCounter(TrainingActivity.this,
                    mImageViewArray);
        } catch (RecordingExceedException e) {
            e.printStackTrace();
        }
    }

    private void startRecordingTimer() {
        LogUtils.d(TAG, "startRecordingTimer: enter");
        stopRecordingTimer();

        mRecordingTimer = new Timer();
        mRecordingTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                mHandler.sendEmptyMessage(MSG_STOP_TRAINING_RECORDING);
            }
        }, DURATION_OF_TRAINING_RECORDING);
    }

    private void stopRecordingTimer() {
        LogUtils.d(TAG, "stopRecordingTimer: enter");
        if (null != mRecordingTimer) {
            mRecordingTimer.cancel();
            mRecordingTimer = null;
        }
    }

    private ByteBuffer readPreviousSmData() {
        String filePath = Global.PATH_ROOT + "/" + mPreviousSmName;
        if (FileUtils.isExist(filePath)) {
            try {
                ByteBuffer smData = FileUtils.readFileToByteBuffer(filePath);
                return smData;
            } catch (IOException e) {
                e.printStackTrace();
                return null;
            }
        } else {
            LogUtils.d(TAG, "readPreviousSmData: file " + filePath + " not exist");
            return null;
        }
    }

    private String getUserName() {
        return mUserName;
    }

    private boolean isUdk() {
        if (mIsAddUserToPreviousSm) {      //needs to check what this means
            return false;
        }
        return getString(R.string.create_your_own_3_0).equals(mPreviousSmName) 
                || getString(R.string.create_your_own_4_0).equals(mPreviousSmName);
    }

    private ISmModel.MODEL_VERSION getUdkVersion() {
        if (getString(R.string.create_your_own_3_0).equals(mPreviousSmName)) {
            return ISmModel.MODEL_VERSION.VERSION_3_0;
        } else if(getString(R.string.create_your_own_4_0).equals(mPreviousSmName)){
            return ISmModel.MODEL_VERSION.VERSION_4_0;
        } else {
            return MODEL_VERSION.VERSION_UNKNOWN;
        }
    }

    private String getKeyphrase() {
        String keyphrase;
        boolean bUdk = isUdk();
        if (!bUdk) {
            if (mIsAddUserToPreviousSm) {
                keyphrase = mKeyphraseOrNewSmName;
            } else {
                // get keyphrase by previous sm name
                IExtendedSmModel extendedSmModel = mExtendedSmMgr.getSoundModel(mPreviousSmName);
                keyphrase = extendedSmModel.getSoundModelPrettyKeyphrase();
            }
        } else {
            keyphrase = mKeyphraseOrNewSmName;
        }

        LogUtils.d(TAG, "getKeyphrase: keyphrase = " + keyphrase);
        return keyphrase;
    }

    private String getCreateSmFilePath() {
        String smFilePath;
        if (isUdk()) {
            smFilePath = Global.PATH_ROOT + "/" + getKeyphrase()
                    + ISmModel.SUFFIX_TRAINED_SOUND_MODEL;
            if (FileUtils.isExist(smFilePath)) {
                long timestamp = System.currentTimeMillis();
                smFilePath = Global.PATH_ROOT + "/" + getKeyphrase() + "_"
                        + String.valueOf(timestamp) + ISmModel.SUFFIX_TRAINED_SOUND_MODEL;
            }
        } else {
            if (mIsAddUserToPreviousSm) {
                smFilePath = Global.PATH_ROOT + "/" + mPreviousSmName;
            } else {
                smFilePath = Global.PATH_ROOT + "/" + mKeyphraseOrNewSmName;
            }
        }

        LogUtils.d(TAG, "getCreateSmFilePath: smFilePath = " + smFilePath);
        return smFilePath;
    }

    private void showTerminateTrainingDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(TrainingActivity.this);
        builder.setTitle(R.string.friendly_tips)
                .setMessage(R.string.terminate_training)
                .setCancelable(false)
                .setNegativeButton(R.string.cancel, null)
                .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        LogUtils.d(TAG, "showTerminateTrainingDialog:onClick: enter");
                        mIsTraining = false;
                        reset();
                        updateUIVisible(false);
                        mRecordingMgr.stopTrainingRecording(
                                mRecordingMgr.getLastUserRecordingFilePath());
                        stopRecordingTimer();

                        mTerminateTrainingDialog.dismiss();
                        mTerminateTrainingDialog = null;
                    }
                });

        if (!(TrainingActivity.this).isFinishing()) {
            mTerminateTrainingDialog = builder.show();
        }
    }

    private void showCreateSmFailDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(TrainingActivity.this);
        builder.setTitle(R.string.friendly_tips)
                .setMessage(R.string.create_sm_failure)
                .setCancelable(false)
                .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        LogUtils.d(TAG, "showCreateSmFailDialog:onClick: enter");
                        enterRecording();
                        mLayoutMic.setVisibility(View.VISIBLE);
                    }
                });

        if (!(TrainingActivity.this).isFinishing()) {
            mCreateSmFailDialog = builder.show();
        }
    }

    private class VerifyRecordingTask extends AsyncTask<String, Void, Integer> {
        @Override
        protected Integer doInBackground(String... strings) {
            LogUtils.d(TAG, "VerifyRecordingTask: doInBackground enter");
            int result;
            ShortBuffer lastRecordingData;
            try {
                lastRecordingData = mRecordingMgr.readWavFile(
                        mRecordingMgr.getLastUserRecordingFilePath());
            } catch (IOException e) {
                e.printStackTrace();
                return -1;
            }

            if (isUdk()) {
                int snr = ListenSoundModel.verifyUdkRecording(
                        mExtendedSmMgr.getLanguageModel(mUdkVersion), lastRecordingData);
                if (snr >= SNR_THRESHOLD) {
                    result = 0;
                } else {
                    result = -1;
                }
            } else {
                int confidenceLevel = ListenSoundModel.verifyUserRecording(
                        readPreviousSmData(), getKeyphrase(), lastRecordingData);
                ISettingsModel settingsModel = new SettingsModel(getApplicationContext(),
                        null);
                if (confidenceLevel >= settingsModel.getGlobalGMMTrainingConfidenceLevel()) {
                    result = 0;
                } else {
                    result = -1;
                }
            }

            return result;
        }

        @Override
        protected void onPostExecute(Integer result) {
            boolean bSuccess = (result.intValue() == 0);
            if (mIsTraining) {
                mTvRecordingResult.setVisibility(View.VISIBLE);
                mRecordingCounter.updateRecordingResult(bSuccess);
                if (bSuccess) {
                    mTvRecordingResult.setTextColor(getColor(R.color.bg_recording_good));
                    mTvRecordingResult.setText(getString(R.string.recording_good));

                    // keep recording data to mem
                    mRecordingMgr.addUserRecording();
                } else {
                    mTvRecordingResult.setTextColor(getColor(R.color.bg_recording_bad));
                    mTvRecordingResult.setText(getString(R.string.recording_bad));
                }

                if (mRecordingCounter.isFinished()) {
                    mIsTraining = false;
                    mLayoutMic.setVisibility(View.INVISIBLE);
                    mTvRecordingResult.setVisibility(View.INVISIBLE);
                    mButtonContinue.setVisibility(View.VISIBLE);
                    mTvRecordingStatus.setText(getString(R.string.complete));
                } else {
                    mRecordingMgr.startTrainingRecording();
                    startRecordingTimer();
                    mTvRecordingStatus.setText(getString(R.string.recording));
                    mIbMic.setImageResource(R.drawable.speaker_on);
                }
            }
        }
    }

    private class CreateSmTask extends AsyncTask<String, Void, Integer> {
        ListenTypes.ConfidenceData mConfidenceData;
        ByteBuffer mSmData;

        @Override
        protected Integer doInBackground(String... strings) {
            int smSize;
            if (isUdk()) {
                smSize = ListenSoundModel.getUdkSmSize(getKeyphrase(), getUserName(),
                        mRecordingMgr.getUserRecordings(),
                        mExtendedSmMgr.getLanguageModel(mUdkVersion));
            } else {
                smSize = ListenSoundModel.getSizeWhenExtended(readPreviousSmData(),
                        getKeyphrase(), getUserName());
            }

            if (smSize <= 0) {
                return -1;
            }

            mSmData = ByteBuffer.allocate(smSize);
            mConfidenceData = new ListenTypes.ConfidenceData();
            int result;
            if (isUdk()) {
                result = ListenSoundModel.createUdkSm(getKeyphrase(), getUserName(),
                        RecordingsMgr.NUM_RECORDINGS_REQUIRED,
                        mRecordingMgr.getUserRecordings(),
                        mExtendedSmMgr.getLanguageModel(mUdkVersion),
                        mSmData, mConfidenceData);
            } else {
                result = ListenSoundModel.extend(readPreviousSmData(), getKeyphrase(),
                        getUserName(), RecordingsMgr.NUM_RECORDINGS_REQUIRED,
                        mRecordingMgr.getUserRecordings(), mSmData, mConfidenceData);
            }

            LogUtils.d(TAG, "CreateSmTask:doInBackground: result = " + result
                    + " userMatch = " + mConfidenceData.userMatch);
            if (ListenTypes.STATUS_SUCCESS == result) {
                return 0;
            }

            return -1;
        }

        @Override
        protected void onPostExecute(Integer result) {
            LogUtils.d(TAG, "CreateSmTask:onPostExecute: result = " + result);

            // remove user recording mem data
            mRecordingMgr.removeUserRecordings();

            showProgressDialog(false);

            if (result.intValue() == 0) {
                // save the sm data to new file
                String smFilePath = getCreateSmFilePath();
                boolean bSuccess = FileUtils.saveByteBufferToFile(
                        mSmData, smFilePath);
                if (bSuccess) {
                    // update sm mgr
                    String[] strings = smFilePath.split("/");
                    String createdSmName = strings[strings.length - 1];
                    LogUtils.d(TAG, "CreateSmTask:onPostExecute: createdSmName = "
                            + createdSmName);
                    mExtendedSmMgr.addSoundModel(getApplicationContext(), createdSmName);
                    // finish training activity
                    finish();
                } else {
                    // failure case
                    showCreateSmFailDialog();
                }
            } else {
                // failure case
                showCreateSmFailDialog();
            }
        }
    }

    private class RecordingExceedException extends Exception {
        public RecordingExceedException(String msg){
            super(msg);
        }
    }

    private class RecordingCounter {
        private final String TAG = RecordingCounter.class.getSimpleName();
        private final int MAX_ITEM_COUNT = 5;
        private ImageView[] mViewArray;
        private int mCounter = 0;
        private Context mContext;

        public RecordingCounter(Context context, ImageView[] views) throws RecordingExceedException {
            mContext = context;
            mViewArray = views;
            if (MAX_ITEM_COUNT != views.length) {
                throw new RecordingExceedException("Invalid argument");
            }

            initCounter();
        }

        private void initCounter() {
            mCounter = 0;
            mViewArray[0].setBackgroundResource(R.drawable.recording_step_off);
            mViewArray[0].setImageResource(R.drawable.recording_1_gray);
            mViewArray[1].setBackgroundResource(R.drawable.recording_step_off);
            mViewArray[1].setImageResource(R.drawable.recording_2_gray);
            mViewArray[2].setBackgroundResource(R.drawable.recording_step_off);
            mViewArray[2].setImageResource(R.drawable.recording_3_gray);
            mViewArray[3].setBackgroundResource(R.drawable.recording_step_off);
            mViewArray[3].setImageResource(R.drawable.recording_4_gray);
            mViewArray[4].setBackgroundResource(R.drawable.recording_step_off);
            mViewArray[4].setImageResource(R.drawable.recording_5_gray);

            setImageFocusChange();
        }

        // Sets the current training image to blue when training
        private void setImageFocusChange() {
            if (MAX_ITEM_COUNT <= mCounter) {
                return;
            }

            int resourceId = 0;
            switch (mCounter) {
                case 0:
                    resourceId = R.drawable.recording_1_blue;
                    break;
                case 1:
                    resourceId = R.drawable.recording_2_blue;
                    break;
                case 2:
                    resourceId = R.drawable.recording_3_blue;
                    break;
                case 3:
                    resourceId = R.drawable.recording_4_blue;
                    break;
                case 4:
                    resourceId = R.drawable.recording_5_blue;
                    break;
            }

            mViewArray[mCounter].setBackgroundResource(R.drawable.recording_step_focused);
            mViewArray[mCounter].setImageResource(resourceId);
        }

        // Sets the current training image to white when done training
        private void setImageDone() {
            if (MAX_ITEM_COUNT <= mCounter) {
                return;
            }

            int resourceId = 0;
            switch (mCounter) {
                case 0:
                    resourceId = R.drawable.recording_1_white;
                    break;
                case 1:
                    resourceId = R.drawable.recording_2_white;
                    break;
                case 2:
                    resourceId = R.drawable.recording_3_white;
                    break;
                case 3:
                    resourceId = R.drawable.recording_4_white;
                    break;
                case 4:
                    resourceId = R.drawable.recording_5_white;
                    break;
            }

            mViewArray[mCounter].setBackgroundResource(R.drawable.recording_step_done);
            mViewArray[mCounter].setImageResource(resourceId);
        }

        // Sets the current training image to white when training fails
        private void setImageFailed() {
            if (MAX_ITEM_COUNT <= mCounter) {
                return;
            }

            int resourceId = 0;
            switch (mCounter) {
                case 0:
                    resourceId = R.drawable.recording_1_white;
                    break;
                case 1:
                    resourceId = R.drawable.recording_2_white;
                    break;
                case 2:
                    resourceId = R.drawable.recording_3_white;
                    break;
                case 3:
                    resourceId = R.drawable.recording_4_white;
                    break;
                case 4:
                    resourceId = R.drawable.recording_5_white;
                    break;
            }

            mViewArray[mCounter].setBackgroundResource(R.drawable.recording_step_fail);
            mViewArray[mCounter].setImageResource(resourceId);
        }

        // Updates the UI if training was successful
        public void updateRecordingResult(boolean isGoodRecording) {
            if (isGoodRecording) {
                setImageDone();
                mCounter++;
                setImageFocusChange();
            } else {
                setImageFailed();
            }
        }

        public boolean isFinished() {
            LogUtils.d(TAG, "isFinished: mCounter = " + mCounter);
            return MAX_ITEM_COUNT <= mCounter;
        }
    }

    private void showProgressDialog(boolean bShow){
        if (null == mProgressDialog) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setView(R.layout.progress_dialog);
            mProgressDialog = builder.create();
        }

        if (bShow) {
            mProgressDialog.show();
        } else {
            if (mProgressDialog.isShowing()) {
                mProgressDialog.dismiss();
                mProgressDialog = null;
            } else {
                LogUtils.d(TAG, "showProgressDialog: mProgressDialog is not showing");
            }
        }
    }
}
