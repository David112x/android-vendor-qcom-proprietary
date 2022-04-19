/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.listen.ListenSoundModel;
import com.qualcomm.listen.ListenTypes;
import com.qualcomm.qti.sva.controller.ExtendedSmMgr;
import com.qualcomm.qti.sva.controller.Global;
import com.qualcomm.qti.sva.data.IExtendedSmModel;
import com.qualcomm.qti.sva.data.IKeyphraseModel;
import com.qualcomm.qti.sva.data.ISettingsModel;
import com.qualcomm.qti.sva.data.ISmModel;
import com.qualcomm.qti.sva.data.IUserModel;
import com.qualcomm.qti.sva.data.SettingsModel;
import com.qualcomm.qti.sva.utils.FileUtils;
import com.qualcomm.qti.sva.utils.LogUtils;
import com.qualcomm.qti.sva.utils.Utils;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

public class ModelSettingsActivity extends Activity {
    public final static String KEY_EXTRA_PARAM_SM_NAME = "smName";
    private final String TAG = ModelSettingsActivity.class.getSimpleName();

    private final String FLAG_ADD_NEW_USER = "YES";
    private final String FLAG_NO_ADD_NEW_USER = "NO";

    // view variable define
    private EditText mEt1stKeyphrase;
    private RelativeLayout mLayout1stUser;
    private EditText mEt1stUser;
    private RelativeLayout mLayout2ndKeyphrase;
    private EditText mEt2ndKeyphrase;
    private RelativeLayout mLayout2ndUser;
    private EditText mEt2ndUser;
    private RelativeLayout mLayoutUserVerification;
    private Switch mSwitchUserVerification;
    private View mSeparatorUnderUserVerification;
    private Switch mSwitchVoiceRequests;
    private RelativeLayout mLayoutVoiceRequests;
    private Spinner mSpinnerVoiceRequestLen;
    private RelativeLayout mLayoutOpaqueDataTransfer;
    private Switch mSwitchOpaqueDataTransfer;
    private EditText mEtHistBufferTime;
    private EditText mEtPreRollDuration;
    private LinearLayout mLayoutSmOperations;
    private LinearLayout mLayoutSmInfo;
    private DeleteUserTask mDeleteUserTask;
    private DeleteKeyphraseTask mDeleteKeyphraseTask;
    private ISettingsModel mSettingModel;
    private String mSmName;
    private boolean mIsTrainedModel;
    private boolean mIsUdkSm;
    private IExtendedSmModel mExtendedSmModel;
    private ISmModel.MODEL_VERSION mSmVersion;
    private AlertDialog mDialogAddUser;

    //sm info layout variables
    private Map<String, View> mKeyphraseUserViewMap = new HashMap<>();
    private static final String VERSION_2_SUFFIX = "(2.0)";
    private static final String VERSION_3_SUFFIX = "(3.0)";
    private static final String VERSION_4_SUFFIX = "(4.0)";
    private static final String VERSION_UNKNOWN_SUFFIX = "(unknown)";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        mSmName = getIntent().getStringExtra(KEY_EXTRA_PARAM_SM_NAME);
        if (null == mSmName) {
            LogUtils.d(TAG, "onCreate: invalid param, directly exit");
            return;
        }

        super.onCreate(savedInstanceState);

        mExtendedSmModel = Global.getInstance().getExtendedSmMgr().getSoundModel(mSmName);
        mIsTrainedModel = mExtendedSmModel.isUserTrainedSoundModel();
        mIsUdkSm = mExtendedSmModel.isUdkSm();
        mSmVersion = mExtendedSmModel.getSoundModelVersion();
        mSettingModel = new SettingsModel(getApplicationContext(), mSmName);

        String title = getTitle().toString();
        String appendString;
        switch (mSmVersion) {
            case VERSION_2_0:
                appendString = VERSION_2_SUFFIX;
                break;
            case VERSION_3_0:
                appendString = VERSION_3_SUFFIX;
                break;
            case VERSION_4_0:
                appendString = VERSION_4_SUFFIX;
                break;
            default:
                appendString = VERSION_UNKNOWN_SUFFIX;
                break;
        }
        setTitle(title + appendString);
        setContentView(R.layout.activity_model_settings);
        initializeUI();
    }

    @Override
    protected void onResume() {
        super.onResume();

        // update UI
        updateUI();
    }

    @Override
    protected void onPause() {
        super.onPause();
        saveVoiceRequestLen();
    }

    @Override
    protected void onDestroy() {
        if (null != mDialogAddUser) {
            mDialogAddUser.dismiss();
            mDialogAddUser = null;
        }

        if (null != mDeleteUserTask && mDeleteUserTask.isCancelled()) {
            mDeleteUserTask.cancel(true);
            mDeleteUserTask = null;
        }

        if (null != mDeleteKeyphraseTask && mDeleteKeyphraseTask.isCancelled()) {
            mDeleteKeyphraseTask.cancel(true);
            mDeleteKeyphraseTask = null;
        }

        super.onDestroy();
    }

    private void updateUI() {
        if (mIsTrainedModel) {
            initSmInfoLayout();
        }
    }

    private void initializeUI() {
        mEt1stKeyphrase = findViewById(R.id.et_first_stage_keyphrase);
        mEt1stKeyphrase.setText(String.valueOf(mSettingModel.get1stKeyphraseConfidenceLevel()));
        mEt1stKeyphrase.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                try {
                    mSettingModel.set1stKeyphraseConfidenceLevel(Integer.valueOf(s.toString()));
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
            }
        });

        mLayout2ndKeyphrase = findViewById(R.id.layout_second_stage_keyphrase_confidence_level);
        mEt2ndKeyphrase = findViewById(R.id.et_second_stage_keyphrase);
        mEt2ndKeyphrase.setText(String.valueOf(mSettingModel.get2ndKeyphraseConfidenceLevel()));
        mEt2ndKeyphrase.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                try {
                    mSettingModel.set2ndKeyphraseConfidenceLevel(Integer.valueOf(s.toString()));
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
            }
        });

        if (ISmModel.MODEL_VERSION.VERSION_2_0 == mSmVersion) {
            mLayout2ndKeyphrase.setVisibility(View.GONE);
        } else {
            mLayout2ndKeyphrase.setVisibility(View.VISIBLE);
        }

        mLayout1stUser = findViewById(R.id.layout_first_stage_user_confidence_level);
        mEt1stUser = findViewById(R.id.et_first_stage_user);
        mEt1stUser.setText(String.valueOf(mSettingModel.get1stUserConfidenceLevel()));
        mEt1stUser.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                try {
                    mSettingModel.set1stUserConfidenceLevel(Integer.valueOf(s.toString()));
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
            }
        });

        if (!mIsTrainedModel) {
            mLayout1stUser.setVisibility(View.GONE);
        }

        mLayout2ndUser = findViewById(R.id.layout_second_stage_user_confidence_level);
        mEt2ndUser = findViewById(R.id.et_second_stage_user);
        mEt2ndUser.setText(String.valueOf(mSettingModel.get2ndUserConfidenceLevel()));
        mEt2ndUser.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                try {
                    mSettingModel.set2ndUserConfidenceLevel(Integer.valueOf(s.toString()));
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
            }
        });
        if (ISmModel.MODEL_VERSION.VERSION_2_0 == mSmVersion) {
            mLayout2ndUser.setVisibility(View.GONE);
        } else {
            if (mIsTrainedModel) {
                mLayout2ndUser.setVisibility(View.VISIBLE);
            } else {
                mLayout2ndUser.setVisibility(View.GONE);
            }
        }

        // user verification
        mSwitchUserVerification = findViewById(R.id.switch_user_verification);
        mSwitchUserVerification.setChecked(mSettingModel.getUserVerificationEnabled());
        mLayoutUserVerification = findViewById(R.id.layout_user_verification);
        mLayoutUserVerification.setTag(mSwitchUserVerification);
        mLayoutUserVerification.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Switch switchUserVerification = (Switch) view.getTag();
                boolean bChecked = !switchUserVerification.isChecked();
                switchUserVerification.setChecked(bChecked);
                mSettingModel.setUserVerificationEnabled(bChecked);
                LogUtils.d(TAG, "onClick: user verification bChecked = " + bChecked);
            }
        });

        mSeparatorUnderUserVerification = findViewById(R.id.separator_under_user_verification);
        if (mIsUdkSm || !mIsTrainedModel) {
            mLayoutUserVerification.setVisibility(View.GONE);
            mSeparatorUnderUserVerification.setVisibility(View.GONE);
        }

        // voice requests
        mSwitchVoiceRequests = findViewById(R.id.switch_voice_requests);
        mSwitchVoiceRequests.setChecked(mSettingModel.getVoiceRequestEnabled());
        mLayoutVoiceRequests = findViewById(R.id.layout_voice_requests);
        mLayoutVoiceRequests.setTag(mSwitchVoiceRequests);
        mLayoutVoiceRequests.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Switch switchVoiceRequests = (Switch) view.getTag();
                boolean bChecked = !switchVoiceRequests.isChecked();
                switchVoiceRequests.setChecked(bChecked);
                mSettingModel.setVoiceRequestEnabled(bChecked);
                LogUtils.d(TAG, "onClick: voice requests bChecked = " + bChecked);
            }
        });

        // voice request len
        mSpinnerVoiceRequestLen = findViewById(R.id.spinner_voice_requests_len);
        int[] optionItems = getResources().getIntArray(
                R.array.voice_request_entry_value);
        ArrayList<VoiceRequestSpinnerItem> voiceRequestLengthOptions = new ArrayList<>();
        for (int i = 0; i < optionItems.length; i++) {
            voiceRequestLengthOptions.add(new VoiceRequestSpinnerItem(optionItems[i]));
        }
        ArrayAdapter<VoiceRequestSpinnerItem> voiceRequestArrayAdapter = new ArrayAdapter<>(
                this, android.R.layout.simple_spinner_item,
                voiceRequestLengthOptions);
        voiceRequestArrayAdapter.setDropDownViewResource(
                android.R.layout.simple_spinner_dropdown_item);
        mSpinnerVoiceRequestLen.setAdapter(voiceRequestArrayAdapter);
        mSpinnerVoiceRequestLen.setSelection(getItemPosition(optionItems,
                mSettingModel.getVoiceRequestLength()));

        // opaque data transfer
        mSwitchOpaqueDataTransfer = findViewById(R.id.switch_opaque_data_transfer);
        mSwitchOpaqueDataTransfer.setChecked(mSettingModel.getOpaqueDataTransferEnabled());
        mLayoutOpaqueDataTransfer = findViewById(R.id.layout_opaque_data_transfer);
        mLayoutOpaqueDataTransfer.setTag(mSwitchOpaqueDataTransfer);
        mLayoutOpaqueDataTransfer.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Switch switchOpaque = (Switch) view.getTag();
                boolean bChecked = !switchOpaque.isChecked();
                switchOpaque.setChecked(bChecked);
                mSettingModel.setOpaqueDataTransferEnabled(bChecked);
                LogUtils.d(TAG, "onClick: opaque data transfer bChecked = " + bChecked);
            }
        });

        // hist buffer time
        mEtHistBufferTime = findViewById(R.id.et_hist_buffer_time);
        mEtHistBufferTime.setText(String.valueOf(mSettingModel.getHistBufferTime()));
        mEtHistBufferTime.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {

            }

            @Override
            public void afterTextChanged(Editable s) {
                try {
                    mSettingModel.setHistBufferTime(Integer.valueOf(s.toString()));
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
            }
        });

        // pre-roll duration
        mEtPreRollDuration = findViewById(R.id.et_pre_roll_duration);
        mEtPreRollDuration.setText(String.valueOf(mSettingModel.getPreRollDuration()));
        mEtPreRollDuration.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {

            }

            @Override
            public void afterTextChanged(Editable s) {
                try {
                    mSettingModel.setPreRollDuration(Integer.valueOf(s.toString()));
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
            }
        });

        // sound model layout
        mLayoutSmOperations = findViewById(R.id.layout_sm_operations);
        mLayoutSmInfo = findViewById(R.id.layout_sm_info);
        if (mIsTrainedModel) {
            initSmInfoLayout();
        } else {
            mLayoutSmOperations.setVisibility(View.GONE);
            mLayoutSmInfo.setVisibility(View.GONE);
        }
    }

    private void initSmInfoLayout() {
        LogUtils.d(TAG, "initSmInfoLayout: enter");
        mLayoutSmInfo.removeAllViews();
        mKeyphraseUserViewMap.clear();
        for (IKeyphraseModel keyphraseModel : mExtendedSmModel.getSoundModelKeyphraseList()) {
            // add keyphrase row
            View rowKeyphrase = getLayoutInflater().inflate(
                    R.layout.row_keyphrase, null);
            TextView tvKeyphrase = rowKeyphrase.findViewById(R.id.tv_keyphrase_name);
            tvKeyphrase.setText(keyphraseModel.getKeyphraseFullName());
            RelativeLayout layoutKeyphrase = rowKeyphrase.findViewById(R.id.layout_keyphrase);
            layoutKeyphrase.setTag(keyphraseModel);
            layoutKeyphrase.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    IKeyphraseModel keyphraseData = (IKeyphraseModel) view.getTag();
                    String keyphrase = keyphraseData.getKeyphraseFullName();
                    LogUtils.d(TAG, "onClick: delete keyphrase "
                            + keyphrase + " is clicked");
                    showDeleteKeyphraseDialog(mSmName, keyphrase);
                }
            });
            mLayoutSmInfo.addView(rowKeyphrase);
            String rowKeyphraseKey = generateViewKey(mSmName,
                    keyphraseModel.getKeyphraseFullName(), null,
                    FLAG_NO_ADD_NEW_USER);
            mKeyphraseUserViewMap.put(rowKeyphraseKey, rowKeyphrase);

            // add all user rows for this keyphrase
            for (IUserModel userModel : keyphraseModel.getUserList()) {
                View rowUser = getLayoutInflater().inflate(
                        R.layout.row_user, null);
                RelativeLayout layoutUser = rowUser.findViewById(R.id.layout_user);
                String rowUserKey = generateViewKey(mSmName,
                        keyphraseModel.getKeyphraseFullName(), userModel.getUserName(),
                        FLAG_NO_ADD_NEW_USER);
                layoutUser.setTag(rowUserKey);
                layoutUser.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        String keyData = (String) view.getTag();
                        String[] arrayList = keyData.split(":");
                        String smName = arrayList[0];
                        String keyphraseName = arrayList[1];
                        String userName = arrayList[2];
                        LogUtils.d(TAG, "onClick: delete user name "
                                + userName + " is clicked");
                        showDeleteUserDialog(smName, keyphraseName, userName);
                    }
                });
                TextView tvUserName = rowUser.findViewById(R.id.tv_user_name);
                tvUserName.setText(userModel.getUserName());
                mLayoutSmInfo.addView(rowUser);
                mKeyphraseUserViewMap.put(rowUserKey, rowUser);
            }

            // + add user row
            if (!mIsUdkSm) {
                View rowAddUser = getLayoutInflater().inflate(
                        R.layout.row_add_user, null);
                LinearLayout mLayoutAddUser = rowAddUser.findViewById(R.id.layout_add_user);
                mLayoutAddUser.setTag(keyphraseModel);
                mLayoutAddUser.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        IKeyphraseModel keyphraseData = (IKeyphraseModel) view.getTag();
                        LogUtils.d(TAG, "onClick: + add user for keyphrase "
                                + keyphraseData.getKeyphraseFullName() + " is clicked");
                        if (mExtendedSmModel.canAddUser()) {
                            showAddUserDialog(mSmName, keyphraseData.getKeyphraseFullName());
                        } else {
                            Toast.makeText(getApplicationContext(),
                                    getString(R.string.cannot_add_user), Toast.LENGTH_LONG).show();
                        }
                    }
                });
                mLayoutSmInfo.addView(rowAddUser);
                String tableRowAddUserKey = generateViewKey(mSmName,
                        keyphraseModel.getKeyphraseFullName(), null, FLAG_ADD_NEW_USER);
                mKeyphraseUserViewMap.put(tableRowAddUserKey, rowAddUser);
            }

            // add separator view
            View separatorView = getLayoutInflater().inflate(R.layout.row_separator, null);
            mLayoutSmInfo.addView(separatorView);
        }
    }

    private String generateViewKey(String smName, String keyphrase, String userName, String flag) {
        return smName + ":" + keyphrase + ":" + userName + ":" + flag;
    }

    private void saveVoiceRequestLen() {
        VoiceRequestSpinnerItem item = (VoiceRequestSpinnerItem)
                (mSpinnerVoiceRequestLen.getSelectedItem());
        mSettingModel.setVoiceRequestLength(item.getValue());
    }

    private void startTrainingActivity(String previousSmName,
                                       String keyphrase, String userName) {
        LogUtils.d(TAG, "startTrainingActivity: previousSmName = " + previousSmName
                + " keyphrase = " + keyphrase
                + " userName = " + userName);

        Intent intent = new Intent(this, TrainingActivity.class);
        intent.putExtra(TrainingActivity.KEY_EXTRA_PREVIOUS_SM_NAME, previousSmName);
        intent.putExtra(TrainingActivity.KEY_EXTRA_KEYPHRASE_OR_NEW_SM_NAME, keyphrase);
        intent.putExtra(TrainingActivity.KEY_EXTRA_USER_NAME, userName);
        intent.putExtra(TrainingActivity.KEY_EXTRA_IS_ADD_USER_TO_PREVIOUS_MODEL, true);
        startActivity(intent);
    }

    private void showAddUserDialog(final String previousSmName, final String keyphrase) {
        LogUtils.d(TAG, "showAddUserDialog: previousSmName = " + previousSmName
                + " keyphrase = " + keyphrase);
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        View view = getLayoutInflater().inflate(R.layout.dialog_edit_text, null);
        builder.setView(view)
                .setTitle(R.string.add_user_dialog_title)
                .setMessage(R.string.add_user_dialog_message)
                .setCancelable(false)
                .setNegativeButton(R.string.cancel, null)
                .setPositiveButton(R.string.train, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        EditText editText = mDialogAddUser.findViewById(R.id.et_param);
                        String userName = editText.getText().toString();
                        LogUtils.d(TAG, "showAddUserDialog: userName = " + userName);
                        String title = getString(R.string.friendly_tips);
                        String message;

                        // check whether is empty or not
                        if ("".equals(userName)) {
                            LogUtils.d(TAG, "showAddUserDialog: invalid user name");
                            message = getString(R.string.user_name_empty);
                            Utils.openAlertDialog(ModelSettingsActivity.this,
                                    title, message);
                            return;
                        }

                        // start training activity
                        startTrainingActivity(previousSmName, keyphrase, userName);

                        // dismiss the dialog
                        mDialogAddUser.dismiss();
                        mDialogAddUser = null;
                    }
                });

        mDialogAddUser = builder.show();
        EditText editText = mDialogAddUser.findViewById(R.id.et_param);
        editText.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                if (hasFocus) {
                    Window window = mDialogAddUser.getWindow();
                    if (null != window) {
                        window.setSoftInputMode(
                                WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);
                    }
                }
            }
        });
        editText.requestFocus();
    }


    private void showDeleteUserDialog(final String smName, final String keyphrase, final String userName) {
        LogUtils.d(TAG, "showDeleteUserDialog: smName = " + smName
                + " keyphrase = " + keyphrase + " userName = " + userName);

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        String message = getString(R.string.delete_user) + " " + userName + "?";
        if (mIsUdkSm) {
            message = getString(R.string.delete_udk_user);
        }
        builder.setTitle(R.string.friendly_tips)
                .setMessage(message)
                .setCancelable(false)
                .setNegativeButton(R.string.cancel, null)
                .setPositiveButton(R.string.confirm,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                LogUtils.d(TAG, "showDeleteUserDialog:onClick: enter");
                                mDeleteUserTask = new DeleteUserTask();
                                String[] strings = new String[]{smName, keyphrase, userName};
                                mDeleteUserTask.execute(strings);
                            }
                        });

        if (!((ModelSettingsActivity.this).isFinishing())) {
            builder.show();
        }
    }

    private void showDeleteKeyphraseDialog(final String smName, final String keyphrase) {
        LogUtils.d(TAG, "showDeleteKeyphraseDialog: smName = " + smName
                + " keyphrase = " + keyphrase);

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        String message = getString(R.string.delete_keyphrase) + " " + keyphrase + "?";
        if (mIsUdkSm) {
            message = getString(R.string.delete_udk_keyphrase);
        }
        builder.setTitle(R.string.friendly_tips)
                .setMessage(message)
                .setCancelable(false)
                .setNegativeButton(R.string.cancel, null)
                .setPositiveButton(R.string.confirm,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                LogUtils.d(TAG, "showDeleteKeyphraseDialog:onClick: enter");
                                mDeleteKeyphraseTask = new DeleteKeyphraseTask();
                                String[] strings = new String[]{smName, keyphrase};
                                mDeleteKeyphraseTask.execute(strings);
                            }
                        });

        if (!((ModelSettingsActivity.this).isFinishing())) {
            builder.show();
        }
    }

    // delete user async task
    private class DeleteUserTask extends AsyncTask<String, Void, Integer> {
        ByteBuffer mNewSmData;
        String mSmName;
        String mKeyphrase;
        String mUserName;

        @Override
        protected Integer doInBackground(String... strings) {
            mSmName = strings[0];
            mKeyphrase = strings[1];
            mUserName = strings[2];
            ByteBuffer previousSmData;

            ExtendedSmMgr extendedSmMgr = Global.getInstance().getExtendedSmMgr();
            if (mIsUdkSm) {
                LogUtils.d(TAG, "DeleteUserTask:doInBackground: UDK");

                // delete this sm file
                FileUtils.deleteFile(extendedSmMgr.getSoundModelFullPath(mSmName));

                // delete sm mgr mem data
                extendedSmMgr.deleteSoundModel(mSmName);
                return 0;
            } else {
                // delete user data for PDK+UV
                LogUtils.d(TAG, "DeleteUserTask:doInBackground: PDK UV");
                try {
                    previousSmData = FileUtils.readFileToByteBuffer(
                            Global.getInstance().getExtendedSmMgr().getSoundModelFullPath(mSmName));
                } catch (IOException e) {
                    e.printStackTrace();
                    return -1;
                }

                int sizeAfterDelete = ListenSoundModel.getSizeAfterDelete(
                        previousSmData, mKeyphrase, mUserName);
                if (sizeAfterDelete < 0) {
                    LogUtils.d(TAG, "DeleteUserTask:doInBackground: get size fail");
                    return -1;
                }

                mNewSmData = ByteBuffer.allocate(sizeAfterDelete);
                int result = ListenSoundModel.deleteData(previousSmData,
                        mKeyphrase, mUserName, mNewSmData);
                if (ListenTypes.STATUS_SUCCESS == result) {
                    LogUtils.d("DeleteUserTask:doInBackground:delete success");

                    // save data to new previous sm file
                    boolean bSuccess = FileUtils.saveByteBufferToFile(mNewSmData,
                            extendedSmMgr.getSoundModelFullPath(mSmName));

                    // update sm mgr data
                    if (bSuccess) {
                        IExtendedSmModel extendedSmModel = extendedSmMgr.getSoundModel(mSmName);
                        extendedSmModel.removeUser(mKeyphrase, mUserName);

                        // need update sm and re-allocate user id too
                        extendedSmMgr.updateSoundModel(getApplicationContext(), extendedSmModel);

                        return 0;
                    } else {
                        return -1;
                    }
                }
            }

            return -1;
        }

        @Override
        protected void onPostExecute(Integer result) {
            if (0 == result.intValue()) {
                // success
                if (mIsUdkSm) {
                    finish();
                } else {
                    // delete view from view array
                    String rowUserKey = generateViewKey(mSmName, mKeyphrase,
                            mUserName, FLAG_NO_ADD_NEW_USER);
                    View view = mKeyphraseUserViewMap.get(rowUserKey);
                    mLayoutSmInfo.removeView(view);
                    if (mKeyphraseUserViewMap.containsValue(view)) {
                        mKeyphraseUserViewMap.remove(view);
                    }
                }
            } else {
                Toast.makeText(getApplicationContext(),
                        getString(R.string.user_delete_failed), Toast.LENGTH_LONG).show();
            }
        }
    }

    // delete keyphrase async task
    private class DeleteKeyphraseTask extends AsyncTask<String, Void, Integer> {
        ByteBuffer mNewSmData;
        String mSmName;
        String mKeyphrase;
        ArrayList<String> mUserList = new ArrayList<>();

        @Override
        protected Integer doInBackground(String... strings) {
            mSmName = strings[0];
            mKeyphrase = strings[1];
            ByteBuffer previousSmData;
            mUserList.clear();
            ExtendedSmMgr extendedSmMgr = Global.getInstance().getExtendedSmMgr();
            if (mIsUdkSm) {
                LogUtils.d(TAG, "DeleteKeyphraseTask:doInBackground: UDK");

                // delete this sm file
                FileUtils.deleteFile(extendedSmMgr.getSoundModelFullPath(mSmName));

                ArrayList<IUserModel> userModels = extendedSmMgr.getSoundModel(mSmName)
                        .getKeyphraseModel(mKeyphrase).getUserList();
                for (IUserModel item : userModels) {
                    mUserList.add(item.getUserName());
                }

                // delete sm mgr mem data
                extendedSmMgr.deleteSoundModel(mSmName);
                return 0;
            } else {
                // delete keyphrase data for PDK+UV
                LogUtils.d(TAG, "DeleteKeyphraseTask:doInBackground: PDK UV");
                try {
                    previousSmData = FileUtils.readFileToByteBuffer(
                            Global.getInstance().getExtendedSmMgr().getSoundModelFullPath(mSmName));
                } catch (IOException e) {
                    e.printStackTrace();
                    return -1;
                }

                int sizeAfterDelete = ListenSoundModel.getSizeAfterDelete(
                        previousSmData, mKeyphrase, null);
                if (sizeAfterDelete < 0) {
                    LogUtils.d(TAG, "DeleteKeyphraseTask:doInBackground: get size fail");
                    return -1;
                }

                mNewSmData = ByteBuffer.allocate(sizeAfterDelete);
                int result = ListenSoundModel.deleteData(previousSmData,
                        mKeyphrase, null, mNewSmData);
                if (ListenTypes.STATUS_SUCCESS == result) {
                    LogUtils.d("DeleteKeyphraseTask:doInBackground:delete success");

                    IExtendedSmModel extendedSmModel = extendedSmMgr.getSoundModel(mSmName);
                    ArrayList<IUserModel> userModels = extendedSmModel.getKeyphraseModel(
                            mKeyphrase).getUserList();
                    for (IUserModel item : userModels) {
                        mUserList.add(item.getUserName());
                    }

                    // save data to new previous sm file
                    boolean bSuccess = FileUtils.saveByteBufferToFile(mNewSmData,
                            extendedSmMgr.getSoundModelFullPath(mSmName));

                    // update sm mgr data
                    if (bSuccess) {
                        extendedSmModel.removeKeyphrase(mKeyphrase);

                        // need update sm and re-allocate keyphrase id too
                        extendedSmMgr.updateSoundModel(getApplicationContext(), extendedSmModel);

                        return 0;
                    } else {
                        return -1;
                    }
                }
            }

            return -1;
        }

        @Override
        protected void onPostExecute(Integer result) {
            if (0 == result.intValue()) {
                // success
                if (mIsUdkSm) {
                    finish();
                } else {
                    // delete view from view array
                    for (String item : mUserList) {
                        String rowUserKey = generateViewKey(mSmName, mKeyphrase,
                                item, FLAG_NO_ADD_NEW_USER);
                        View view = mKeyphraseUserViewMap.get(rowUserKey);
                        mLayoutSmInfo.removeView(view);
                        if (mKeyphraseUserViewMap.containsValue(view)) {
                            mKeyphraseUserViewMap.remove(view);
                        }
                    }
                }
            } else {
                Toast.makeText(getApplicationContext(),
                        getString(R.string.keyphrase_delete_failed), Toast.LENGTH_LONG).show();
            }
        }
    }

    private class VoiceRequestSpinnerItem {
        private int mValue;

        VoiceRequestSpinnerItem(int v) {
            mValue = v;
        }

        @Override
        public String toString() {
            return "" + ((double)mValue) / 1000;
        }

        public int getValue() {
            return mValue;
        }
    }

    private int getItemPosition(int[] options, int selected) {
        for (int i = 0; i < options.length; i++) {
            if (options[i] == selected) {
                return i;
            }
        }
        return 0;
    }
}
