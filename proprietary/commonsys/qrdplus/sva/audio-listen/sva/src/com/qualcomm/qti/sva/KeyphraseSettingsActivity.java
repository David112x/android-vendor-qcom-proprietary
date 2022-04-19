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
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.listen.ListenSoundModel;
import com.qualcomm.listen.ListenTypes;
import com.qualcomm.qti.sva.controller.ExtendedSmMgr;
import com.qualcomm.qti.sva.controller.Global;
import com.qualcomm.qti.sva.data.IExtendedSmModel;
import com.qualcomm.qti.sva.data.ISettingsModel;
import com.qualcomm.qti.sva.data.IUserModel;
import com.qualcomm.qti.sva.data.SettingsModel;
import com.qualcomm.qti.sva.utils.FileUtils;
import com.qualcomm.qti.sva.utils.LogUtils;
import com.qualcomm.qti.sva.utils.Utils;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;

public class KeyphraseSettingsActivity extends Activity
        implements UserListAdapter.OnListenerCallback {
    public final static String KEY_EXTRA_PARAM_SM_NAME = "smName";
    private final String TAG = KeyphraseSettingsActivity.class.getSimpleName();

    private DeleteUserTask mDeleteUserTask;
    private DeleteKeyphraseTask mDeleteKeyphraseTask;

    // View item variable
    private LinearLayout mLayoutSetAction;
    private TextView mTvActionName;
    private RelativeLayout mLayoutUserVerification;
    private Switch mSwitchUserVerification;
    private ListView mListViewUsers;
    private LinearLayout mLayoutAddUser;
    private ArrayList<IUserModel> mListData = new ArrayList<>();
    private String mSmName;
    private boolean mIsTrainedModel;
    private boolean mIsUdkSm;
    private IExtendedSmModel mExtendedSmModel;
    private View mSeparatorUnderUserVerification;
    private View mSeparatorUnderUserList;
    private View mSeparatorUnderAddUser;
    private RelativeLayout mLayoutKeyphrase;
    private TextView mTvKeyphrase;
    private View mSeparatorAboveListView;
    private AlertDialog mDialogAddUser;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        mSmName = getIntent().getStringExtra(KEY_EXTRA_PARAM_SM_NAME);
        if (null == mSmName) {
            LogUtils.d(TAG, "onCreate: invalid param, directly exit");
            return;
        }

        mExtendedSmModel = Global.getInstance().getExtendedSmMgr().getSoundModel(mSmName);

        // trained model & not udk
        mIsTrainedModel = mExtendedSmModel.isUserTrainedSoundModel();
        mIsUdkSm = mExtendedSmModel.isUdkSm();

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_keyphrase_settings);
        initializeUI();
    }

    @Override
    protected void onResume() {
        updateUI();
        super.onResume();
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

        super.onDestroy();
    }

    private void updateUI() {
        // update action txt
        mTvActionName.setText(
                Global.getInstance().getExtendedSmMgr()
                        .getSoundModel(mSmName).getSoundModelActionName());

        // update add user line
        if (!mExtendedSmModel.canAddUser()) {
            mLayoutAddUser.setVisibility(View.GONE);
            mSeparatorUnderAddUser.setVisibility(View.GONE);
        } else {
            mLayoutAddUser.setVisibility(View.VISIBLE);
            mSeparatorUnderAddUser.setVisibility(View.VISIBLE);
        }

        // update user list
        mListData = Global.getInstance().getExtendedSmMgr().getSoundModel(mSmName).getAllUsers();
        ((UserListAdapter) mListViewUsers.getAdapter()).updateData(mListData);
    }

    private void initializeUI() {
        mLayoutSetAction = findViewById(R.id.layout_set_action);
        mLayoutSetAction.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                LogUtils.d(TAG, "onClick: set action");
                Intent intent = new Intent(getApplicationContext(),
                        SelectActionActivity.class);
                intent.putExtra(SelectActionActivity.KEY_EXTRA_KEYPHRASE_NAME,
                        Global.getInstance().getExtendedSmMgr()
                                .getSoundModel(mSmName).getSoundModelPrettyKeyphrase());
                intent.putExtra(SelectActionActivity.KEY_EXTRA_SM_NAME, mSmName);
                startActivity(intent);
            }
        });
        mTvActionName = findViewById(R.id.tv_action_name);
        mSwitchUserVerification = findViewById(R.id.switch_user_verification);
        ISettingsModel settingsModel = new SettingsModel(getApplicationContext(),
                mSmName);
        mSwitchUserVerification.setChecked(settingsModel.getUserVerificationEnabled());
        mLayoutUserVerification = findViewById(R.id.layout_user_verification);
        mLayoutUserVerification.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                LogUtils.d(TAG, "onClick: set user verification");
                boolean isChecked = mSwitchUserVerification.isChecked();
                mSwitchUserVerification.setChecked(!isChecked);

                // update global variables values
                ISettingsModel settingsModel = new SettingsModel(getApplicationContext(),
                        mSmName);
                settingsModel.setUserVerificationEnabled(!isChecked);
            }
        });

        mSeparatorUnderUserVerification = findViewById(R.id.separator_under_user_verification);
        mSeparatorUnderUserList = findViewById(R.id.separator_under_user_list);

        mLayoutAddUser = findViewById(R.id.layout_add_user);
        mLayoutAddUser.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                LogUtils.d(TAG, "onClick: add user");
                if (mExtendedSmModel.canAddUser()) {
                    showAddUserDialog(mSmName, mExtendedSmModel.getSoundModelPrettyKeyphrase());
                } else {
                    Toast.makeText(getApplicationContext(),
                            getString(R.string.cannot_add_user),
                            Toast.LENGTH_LONG).show();
                }
            }
        });
        mSeparatorUnderAddUser = findViewById(R.id.separator_under_add_user);

        mListViewUsers = findViewById(R.id.lv_user_list);
        mListData = Global.getInstance().getExtendedSmMgr().getSoundModel(mSmName).getAllUsers();
        UserListAdapter adapter = new UserListAdapter(this, mListData, this);
        mListViewUsers.setAdapter(adapter);
        mSeparatorUnderUserList = findViewById(R.id.separator_under_user_list);

        mLayoutKeyphrase = findViewById(R.id.layout_keyphrase);
        mTvKeyphrase = findViewById(R.id.tv_keyphrase_name);
        mTvKeyphrase.setText(mExtendedSmModel.getSoundModelPrettyKeyphrase());
        mSeparatorAboveListView = findViewById(R.id.separator_above_list_view);
        mLayoutKeyphrase.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                // delete keyphrase, means delete the sound model
                showDeleteKeyphraseDialog(mSmName,
                        mExtendedSmModel.getSoundModelPrettyKeyphrase());

            }
        });

        // update visibility
        if (mIsUdkSm) {
            mLayoutUserVerification.setVisibility(View.GONE);
            mSeparatorUnderUserVerification.setVisibility(View.GONE);
            mLayoutAddUser.setVisibility(View.GONE);
            mSeparatorUnderAddUser.setVisibility(View.GONE);
        } else if (!mIsTrainedModel) {
            mLayoutUserVerification.setVisibility(View.GONE);
            mSeparatorUnderUserVerification.setVisibility(View.GONE);
            mSeparatorUnderUserList.setVisibility(View.GONE);
            mLayoutKeyphrase.setVisibility(View.GONE);
            mSeparatorAboveListView.setVisibility(View.GONE);
        }
    }

    @Override
    public void onUserItemClicked(UserListAdapter.RowViewHolder rowViewHolder) {
        String userName
                = (rowViewHolder.mItemData.get(rowViewHolder.mIndex)).getUserName();
        LogUtils.d(TAG, "onUserItemClicked: delete userName = " + userName);

        // delete user
        showDeleteUserDialog(mSmName, mExtendedSmModel.getSoundModelPrettyKeyphrase(), userName);
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
                            Utils.openAlertDialog(KeyphraseSettingsActivity.this,
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

    private void showDeleteUserDialog(final String smName, final String keyphrase,
                                      final String userName) {
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

        if (!((KeyphraseSettingsActivity.this).isFinishing())) {
            builder.show();
        }
    }

    private void showDeleteKeyphraseDialog(final String smName, final String keyphrase) {
        LogUtils.d(TAG, "showDeleteKeyphraseDialog: smName = " + smName
                + " keyphrase = " + keyphrase);

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        String message = getString(R.string.delete_single_keyphrase_sm);
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

        if (!((KeyphraseSettingsActivity.this).isFinishing())) {
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
                    updateUI();
                }
            } else {
                Toast.makeText(getApplicationContext(),
                        getString(R.string.user_delete_failed), Toast.LENGTH_LONG).show();
            }
        }
    }

    // delete keyphrase async task
    private class DeleteKeyphraseTask extends AsyncTask<String, Void, Integer> {
        String mSmName;
        String mKeyphrase;

        @Override
        protected Integer doInBackground(String... strings) {
            LogUtils.d(TAG, "DeleteKeyphraseTask:doInBackground: enter");
            mSmName = strings[0];
            mKeyphrase = strings[1];
            ExtendedSmMgr extendedSmMgr = Global.getInstance().getExtendedSmMgr();

            // delete this sm file
            FileUtils.deleteFile(extendedSmMgr.getSoundModelFullPath(mSmName));

            // delete sm mgr mem data
            extendedSmMgr.deleteSoundModel(mSmName);
            return 0;
        }

        @Override
        protected void onPostExecute(Integer result) {
            if (0 == result.intValue()) {
                finish();
            } else {
                Toast.makeText(getApplicationContext(),
                        getString(R.string.keyphrase_delete_failed), Toast.LENGTH_LONG).show();
            }
        }
    }


}
