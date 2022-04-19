/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;

import com.qualcomm.listen.ListenSoundModel;
import com.qualcomm.listen.ListenTypes;
import com.qualcomm.qti.sva.controller.ExtendedSmMgr;
import com.qualcomm.qti.sva.controller.Global;
import com.qualcomm.qti.sva.controller.Msg;
import com.qualcomm.qti.sva.controller.MsgResponse;
import com.qualcomm.qti.sva.data.IExtendedSmModel;
import com.qualcomm.qti.sva.data.ISmModel;
import com.qualcomm.qti.sva.service.WakeupService;
import com.qualcomm.qti.sva.utils.FileUtils;
import com.qualcomm.qti.sva.utils.LogUtils;
import com.qualcomm.qti.sva.utils.Utils;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;

public class DebugModelListActivity extends ServiceActivity {
    private final String TAG = DebugModelListActivity.class.getSimpleName();

    private int mSelectedIndex = -1;
    private ArrayList<IExtendedSmModel> mTrainableSmList;
    private MergeModelTask mMergeModelTask;
    private ArrayList<IExtendedSmModel> mCanDeleteSmList;
    private Messenger mClientMessenger;
    private Button mButtonLoad;
    private Button mButtonStart;
    private Button mButtonStop;
    private Button mButtonUnload;
    private ListView mListView;
    private ArrayList<IExtendedSmModel> mListData = new ArrayList<>();
    private String mSelectedSmFullName;
    private AlertDialog mDialogSelectKeyphrase;
    private AlertDialog mDialogNext;
    private AlertDialog mDialogAddUser;
    private AlertDialog mMergeModelDialog;
    private ArrayList<String> mCheckedSmNameList = new ArrayList<>();
    private AlertDialog mNamingMergedSmDialog;
    private AlertDialog mMergeSmFailDialog;
    private AlertDialog mDialogDeleteModels;

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
                case Msg.MSG_TERMINATE_SESSION_RSP:
                    onReceivedTerminateSessionRsp((MsgResponse) msg.obj);
                    break;
                case Msg.MSG_RELEASE_ALL_SESSIONS_RSP:
                    onReceivedReleaseAllSessionRsp();
                    break;
                default:
                    break;
            }
            return true;
        }
    });

    private ModelListAdapter.OnListenerCallback mListItemCallback
            = new ModelListAdapter.OnListenerCallback() {
        @Override
        public void onLeftClicked(ModelListAdapter.RowViewHolder rowViewHolder) {
            LogUtils.d(TAG, "onLeftClicked: clicked smName = "
                    + rowViewHolder.mItemData.getSoundModelFullFileName());
            if (rowViewHolder.mCheckBox.isChecked()) {
                mSelectedSmFullName = rowViewHolder.mItemData.getSoundModelFullFileName();
            } else {
                mSelectedSmFullName = null;
            }

            enterState(rowViewHolder.mItemData.getSessionStatus());

        }

        @Override
        public void onRightClicked(ModelListAdapter.RowViewHolder rowViewHolder) {
            final String smName = rowViewHolder.mItemData.getSoundModelFullFileName();
            LogUtils.d(TAG, "onRightClicked: clicked smName = " + smName);
            ExtendedSmMgr extendedSmMgr = Global.getInstance().getExtendedSmMgr();
            IExtendedSmModel.SessionStatus sessionStatus
                    = extendedSmMgr.getSoundModel(smName).getSessionStatus();
            if (IExtendedSmModel.SessionStatus.UNLOADED != sessionStatus) {
                AlertDialog.Builder builder = new AlertDialog.Builder(
                        DebugModelListActivity.this);
                builder.setTitle(R.string.friendly_tips)
                        .setMessage(R.string.close_session_first)
                        .setCancelable(false)
                        .setNegativeButton(R.string.cancel, null)
                        .setPositiveButton(R.string.confirm,
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        sendRequest(Msg.MSG_TERMINATE_SESSION_REQ, smName);
                                        startModelSettingsActivity(smName);
                                    }
                                });

                if (!((DebugModelListActivity.this).isFinishing())) {
                    builder.show();
                }
            } else {
                startModelSettingsActivity(smName);
            }
        }
    };

    private void onReceivedReleaseAllSessionRsp() {
        LogUtils.d(TAG, "onReceivedReleaseAllSessionRsp: enter");
        if (null != mSelectedSmFullName) {
            enterState(Global.getInstance().getExtendedSmMgr()
                    .getSoundModel(mSelectedSmFullName).getSessionStatus());
        }
    }

    private void onReceivedTerminateSessionRsp(MsgResponse response) {
        LogUtils.d(TAG, "onTerminateSessionRsp: response = " + response);
        String smName = response.getSmFullName();
        enterState(Global.getInstance().getExtendedSmMgr()
                .getSoundModel(smName).getSessionStatus());
    }

    private void onReceivedStopRecognitionRsp(MsgResponse.Result result) {
        LogUtils.d(TAG, "onReceivedStopRecognitionRsp: result = " + result);
        String toastMsg;
        if (MsgResponse.Result.SUCCESS == result) {
            enterState(IExtendedSmModel.SessionStatus.STOPPED);
            toastMsg = getString(R.string.stop_success);
        } else {
            toastMsg = getString(R.string.stop_un_success);
        }
        Toast.makeText(getApplicationContext(), toastMsg, Toast.LENGTH_SHORT).show();
    }

    private void onReceivedUnloadSmRsp(MsgResponse.Result result) {
        LogUtils.d(TAG, "onReceivedUnloadSmRsp: result = " + result);
        String toastMsg;
        if (MsgResponse.Result.SUCCESS == result) {
            enterState(IExtendedSmModel.SessionStatus.UNLOADED);
            toastMsg = getString(R.string.unload_success);
        } else {
            toastMsg = getString(R.string.unload_un_success);
        }
        Toast.makeText(getApplicationContext(), toastMsg, Toast.LENGTH_SHORT).show();
    }

    private void onReceivedStartRecognitionRsp(MsgResponse.Result result) {
        LogUtils.d(TAG, "onReceivedStartRecognitionRsp: result = " + result);
        String toastMsg;
        if (MsgResponse.Result.SUCCESS == result) {
            enterState(IExtendedSmModel.SessionStatus.STARTED);
            toastMsg = getString(R.string.start_success);
        } else {
            toastMsg = getString(R.string.start_un_success);
        }
        Toast.makeText(getApplicationContext(), toastMsg, Toast.LENGTH_SHORT).show();
    }

    private void onReceivedLoadSmRsp(MsgResponse.Result result) {
        LogUtils.d(TAG, "onReceivedLoadSmRsp: result = " + result);
        String toastMsg;
        if (MsgResponse.Result.SUCCESS == result) {
            enterState(IExtendedSmModel.SessionStatus.LOADED);
            toastMsg = getString(R.string.load_success);
        } else {
            toastMsg = getString(R.string.load_un_success);
        }
        Toast.makeText(getApplicationContext(), toastMsg, Toast.LENGTH_SHORT).show();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mClientMessenger = new Messenger(mHandler);
        setContentView(R.layout.activity_model_list);

        Intent intent = new Intent(this, WakeupService.class);
        bindService(intent, getServiceConnection(), Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateUI();
    }

    @Override
    protected void onDestroy() {
        if (isServiceConnected()) {
            deRegisterClient(Msg.ClientID.CLIENT_ID_DEBUG_MODEL_LIST_ACTIVITY);
            unbindService(getServiceConnection());
        }

        if (null != mDialogSelectKeyphrase) {
            mDialogSelectKeyphrase.dismiss();
            mDialogSelectKeyphrase = null;
        }

        if (null != mDialogNext) {
            mDialogNext.dismiss();
            mDialogNext = null;
        }

        if (null != mDialogAddUser) {
            mDialogAddUser.dismiss();
            mDialogAddUser = null;
        }

        if (null != mMergeModelDialog) {
            mMergeModelDialog.dismiss();
            mMergeModelDialog = null;
        }

        if (null != mNamingMergedSmDialog) {
            mNamingMergedSmDialog.dismiss();
            mNamingMergedSmDialog = null;
        }

        if (null != mMergeModelTask && mMergeModelTask.isCancelled()) {
            mMergeModelTask.cancel(true);
            mMergeModelTask = null;
        }

        if (null != mMergeSmFailDialog) {
            mMergeSmFailDialog.dismiss();
            mMergeSmFailDialog = null;
        }

        if (null != mHandler) {
            mHandler.removeCallbacksAndMessages(null);
        }

        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_debug_model_list, menu);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);

        // set merge model to invisible when list is one
        if (Global.getInstance().getExtendedSmMgr().getCanMergeSoundModelList().size() <= 1) {
            menu.findItem(R.id.menu_item_merge_model).setVisible(false);
        } else {
            menu.findItem(R.id.menu_item_merge_model).setVisible(true);
        }

        // set delete model to invisible when there is not trained models
        if (Global.getInstance().getExtendedSmMgr().getCanDeleteSoundModelList().size() <= 0) {
            menu.findItem(R.id.menu_item_delete_model).setVisible(false);
        } else {
            menu.findItem(R.id.menu_item_delete_model).setVisible(true);
        }

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        LogUtils.d(TAG, "onOptionsItemSelected: selected item " + item.getTitle());
        switch (id) {
            case R.id.menu_item_training:
                handleTrainingMenuItemSelected();
                break;
            case R.id.menu_item_merge_model:
                showMergeModelDialog();
                break;
            case R.id.menu_item_delete_model:
                handleDeleteModelMenuItemSelected();
                break;
            case R.id.menu_item_general_settings:
                Intent intent = new Intent(this,
                        GeneralSettingsActivity.class);
                startActivity(intent);
                break;
            case R.id.menu_item_toggle:
                boolean bToggle = Global.getInstance().getIsToggleXmlParam();
                Global.getInstance().setIsToggleXmlParam(!bToggle);
                if (!bToggle) {
                    Toast.makeText(getApplicationContext(), getString(R.string.params_from_xml),
                            Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(getApplicationContext(),
                            getString(R.string.params_not_from_xml),
                            Toast.LENGTH_SHORT).show();
                }
                break;
            case R.id.menu_item_version:
                try {
                    String versionNumber = getString(R.string.version_number)
                            + " " + getPackageManager().getPackageInfo(
                            getPackageName(), 0).versionName;
                    Utils.openAlertDialog(DebugModelListActivity.this,
                            null, versionNumber);
                } catch (PackageManager.NameNotFoundException e) {
                    e.printStackTrace();
                }
                break;
        }
        return true;
    }

    private void handleDeleteModelMenuItemSelected() {
        LogUtils.d(TAG, "handleDeleteModelMenuItemSelected: enter");
        if (Global.getInstance().getExtendedSmMgr().hasLoadedOrActiveSessions()) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.friendly_tips)
                    .setMessage(R.string.close_all_sessions_first)
                    .setCancelable(false)
                    .setNegativeButton(R.string.cancel, null)
                    .setPositiveButton(R.string.confirm,
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    sendRequest(Msg.MSG_RELEASE_ALL_SESSIONS_REQ, null);
                                    ShowDeleteModelDialog();
                                }
                            });

            if (!((DebugModelListActivity.this).isFinishing())) {
                builder.show();
            }
        } else {
            ShowDeleteModelDialog();
        }
    }

    private void ShowDeleteModelDialog() {
        LogUtils.d(TAG, "ShowDeleteModelDialog: enter");
        final ExtendedSmMgr smMgr = Global.getInstance().getExtendedSmMgr();
        ArrayList<String> smNameList = new ArrayList<>();
        mCanDeleteSmList = smMgr.getCanDeleteSoundModelList();
        for (IExtendedSmModel item : mCanDeleteSmList) {
            smNameList.add(item.getSoundModelFullFileName());
        }

        if (smNameList.isEmpty()) {
            LogUtils.e(TAG, "ShowDeleteModelDialog: no available sound models");
            return;
        }

        final CharSequence[] nameList = smNameList.toArray(
                new CharSequence[smNameList.size()]);
        int size = smNameList.size();
        boolean[] initCheckedItems = new boolean[size];
        final boolean[] checkedItems = new boolean[size];
        mCheckedSmNameList.clear();
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.delete_model)
                .setCancelable(true)
                .setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        dialogInterface.dismiss();
                        mDialogDeleteModels = null;
                    }
                })
                .setMultiChoiceItems(nameList, initCheckedItems,
                        new DialogInterface.OnMultiChoiceClickListener() {
                            @Override
                            public void onClick(DialogInterface
                                                        dialogInterface, int which, boolean bChecked) {
                                checkedItems[which] = bChecked;
                            }
                        })
                .setPositiveButton(R.string.delete, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int which) {
                        for (int i = 0; i < checkedItems.length; i++) {
                            if (checkedItems[i]) {
                                mCheckedSmNameList.add(nameList[i].toString());
                            }
                        }

                        for (String item : mCheckedSmNameList) {
                            LogUtils.d(TAG, "ShowDeleteModelDialog: delete model = " + item);

                            // delete sm memory
                            smMgr.deleteSoundModel(item);

                            // delete sm file
                            String smPath = smMgr.getSoundModelFullPath(item);
                            FileUtils.deleteFile(smPath);
                        }

                        dialogInterface.dismiss();
                        mDialogDeleteModels = null;

                        // update list data
                        updateUI();
                    }
                });

        if (!DebugModelListActivity.this.isFinishing()) {
            mDialogDeleteModels = builder.show();
        }
    }

    private void showMergeModelDialog() {
        ArrayList<IExtendedSmModel> canMergeSmList
                = Global.getInstance().getExtendedSmMgr().getCanMergeSoundModelList();
        ArrayList<String> smNameList = new ArrayList<>();
        for (IExtendedSmModel item : canMergeSmList) {
            smNameList.add(item.getSoundModelFullFileName());
        }

        final CharSequence[] nameList = smNameList.toArray(
                new CharSequence[smNameList.size()]);
        int size = smNameList.size();
        boolean[] initCheckedItems = new boolean[size];
        final boolean[] checkedItems = new boolean[size];
        mCheckedSmNameList.clear();
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.merge_model)
                .setCancelable(true)
                .setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        dialogInterface.dismiss();
                        mMergeModelDialog = null;
                    }
                })
                .setMultiChoiceItems(nameList, initCheckedItems,
                        new DialogInterface.OnMultiChoiceClickListener() {
                            @Override
                            public void onClick(DialogInterface
                                                        dialogInterface, int which, boolean bChecked) {
                                checkedItems[which] = bChecked;
                            }
                        })
                .setPositiveButton(R.string.next, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int which) {
                        for (int i = 0; i < checkedItems.length; i++) {
                            if (checkedItems[i]) {
                                mCheckedSmNameList.add(nameList[i].toString());
                            }
                        }

                        // show naming dialog
                        showNamingMergedSmDialog();

                        dialogInterface.dismiss();
                        mMergeModelDialog = null;
                    }
                });

        if (!DebugModelListActivity.this.isFinishing()) {
            mMergeModelDialog = builder.show();
        }
    }

    private void showNamingMergedSmDialog() {
        LogUtils.d(TAG, "showNamingMergedSmDialog: enter");

        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(this);
        View view = getLayoutInflater().inflate(R.layout.dialog_edit_text, null);
        String title = getString(R.string.new_sm_dialog_title);
        String message = getString(R.string.new_sm_dialog_message);
        dialogBuilder.setView(view)
                .setTitle(title)
                .setMessage(message)
                .setCancelable(false)
                .setNegativeButton(R.string.cancel, null)
                .setPositiveButton(R.string.merge, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        EditText editText = mNamingMergedSmDialog.findViewById(R.id.et_param);
                        String inputText = editText.getText().toString();

                        String title = getString(R.string.friendly_tips);
                        String message;

                        // check whether is empty or not
                        if ("".equals(inputText)) {
                            LogUtils.d(TAG, "showNamingMergedSmDialog:invalid input value");
                            message = getString(R.string.sm_name_empty);
                            Utils.openAlertDialog(DebugModelListActivity.this,
                                    title, message);
                            return;
                        }

                        // check if conflict or not
                        String newSmName = inputText + ISmModel.SUFFIX_TRAINED_SOUND_MODEL;
                        String newSmPath = Global.PATH_ROOT + "/" + newSmName;
                        if (FileUtils.isExist(newSmPath)) {
                            message = getString(R.string.name_conflict_message);
                            Utils.openAlertDialog(DebugModelListActivity.this,
                                    title, message);
                            return;
                        }

                        LogUtils.d(TAG, "showNamingMergedSmDialog: input new sm name = "
                                + newSmName);

                        // do merge task
                        mMergeModelTask = new MergeModelTask();
                        String[] strings = new String[]{newSmName};
                        mMergeModelTask.execute(strings);

                        // dismiss this dialog
                        mNamingMergedSmDialog.dismiss();
                        mNamingMergedSmDialog = null;
                    }
                });

        mNamingMergedSmDialog = dialogBuilder.show();
        EditText editText = mNamingMergedSmDialog.findViewById(R.id.et_param);
        editText.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                if (hasFocus) {
                    Window window = mNamingMergedSmDialog.getWindow();
                    if (null != window) {
                        window.setSoftInputMode(
                                WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);
                    }
                }
            }
        });
        editText.requestFocus();
    }

    private void showMergeSmFailDialog(int errorCode) {
        AlertDialog.Builder builder = new AlertDialog.Builder(DebugModelListActivity.this);
        String message = getString(R.string.merge_sm_failure) + " " + errorCode;
        builder.setTitle(R.string.friendly_tips)
                .setMessage(message)
                .setCancelable(false)
                .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        LogUtils.d(TAG, "showMergeSmFailDialog:onClick: enter");
                        mMergeSmFailDialog.dismiss();
                        mMergeSmFailDialog = null;
                    }
                });

        if (!(DebugModelListActivity.this).isFinishing()) {
            mMergeSmFailDialog = builder.show();
        }
    }

    @Override
    protected void onServiceConnectedCallback() {
        registerClient(Msg.ClientID.CLIENT_ID_DEBUG_MODEL_LIST_ACTIVITY);
        initializeUI();
    }

    @Override
    protected void onServiceDisconnectedCallback() {
        deRegisterClient(Msg.ClientID.CLIENT_ID_DEBUG_MODEL_LIST_ACTIVITY);
    }

    private void initializeUI() {
        mButtonLoad = findViewById(R.id.b_load_sm);
        mButtonStart = findViewById(R.id.b_start_recognition);
        mButtonStop = findViewById(R.id.b_stop_recognition);
        mButtonUnload = findViewById(R.id.b_unload_sm);
        mListView = findViewById(R.id.lv_sound_models);

        mListData = Global.getInstance().getExtendedSmMgr().getAllSoundModelList();
        ModelListAdapter adapter = new ModelListAdapter(this, mListData,
                mListItemCallback);
        mListView.setAdapter(adapter);
    }

    private void updateUI() {
        // update list view data
        if (null != mListView) {
            mListData = Global.getInstance().getExtendedSmMgr().getAllSoundModelList();
            ((ModelListAdapter) mListView.getAdapter()).updateData(mListData);
        }
    }

    private void startModelSettingsActivity(String smName) {
        Intent intent = new Intent(DebugModelListActivity.this,
                ModelSettingsActivity.class);
        intent.putExtra(ModelSettingsActivity.KEY_EXTRA_PARAM_SM_NAME, smName);
        startActivity(intent);
    }

    @Override
    protected Messenger getClientMessenger() {
        return mClientMessenger;
    }

    public void onStateButtonClicked(View view) {
        LogUtils.d(TAG, "onStateButtonClicked: click button "
                + ((Button) view).getText() + " mSelectedSmFullName = "
                + mSelectedSmFullName);
        if (null == mSelectedSmFullName) {
            LogUtils.d(TAG, "onStateButtonClicked: no selected sound model");
            Toast.makeText(getApplicationContext(), getString(R.string.select_one_model),
                    Toast.LENGTH_SHORT).show();
            return;
        }

        switch (view.getId()) {
            case R.id.b_load_sm:
                sendRequest(Msg.MSG_LOAD_SOUND_MODEL_REQ, mSelectedSmFullName);
                break;
            case R.id.b_start_recognition:
                sendRequest(Msg.MSG_START_RECOGNITION_REQ, mSelectedSmFullName);
                break;
            case R.id.b_stop_recognition:
                sendRequest(Msg.MSG_STOP_RECOGNITION_REQ, mSelectedSmFullName);
                break;
            case R.id.b_unload_sm:
                sendRequest(Msg.MSG_UNLOAD_SOUND_MODEL_REQ, mSelectedSmFullName);
                break;
            default:
                LogUtils.e(TAG, "onStateButtonClicked: unrecognized button pressed");
                break;
        }
    }

    private void enterState(IExtendedSmModel.SessionStatus state) {
        LogUtils.d(TAG, "enterState: state = " + state);
        switch (state) {
            case UNLOADED:
                mButtonLoad.setEnabled(true);
                mButtonStart.setEnabled(false);
                mButtonStop.setEnabled(false);
                mButtonUnload.setEnabled(false);
                break;
            case LOADED:
                mButtonLoad.setEnabled(false);
                mButtonStart.setEnabled(true);
                mButtonStop.setEnabled(false);
                mButtonUnload.setEnabled(true);
                break;
            case STARTED:
                mButtonLoad.setEnabled(false);
                mButtonStart.setEnabled(false);
                mButtonStop.setEnabled(true);
                mButtonUnload.setEnabled(false);
                break;
            case STOPPED:
                mButtonLoad.setEnabled(false);
                mButtonStart.setEnabled(true);
                mButtonStop.setEnabled(false);
                mButtonUnload.setEnabled(true);
                break;
            default:
                LogUtils.e(TAG, "enterState: unrecognized state= " + state.name());
                break;
        }
    }

    private void showSelectKeyphraseDialog() {
        LogUtils.d(TAG, "showSelectKeyphraseDialog: enter");
        ExtendedSmMgr smMgr = Global.getInstance().getExtendedSmMgr();
        ArrayList<String> keyphraseList = new ArrayList<>();
        mTrainableSmList = smMgr.getTrainableSoundModelList();
        for (IExtendedSmModel item : mTrainableSmList) {
            String versionString = null;
            ISmModel.MODEL_VERSION smVersion = item.getSoundModelVersion();
            switch(smVersion) {
                case VERSION_2_0:
                    versionString = "(2.0)";
                    break;
                case VERSION_3_0:
                    versionString = "(3.0)";
                    break;
                case VERSION_4_0:
                    versionString = "(4.0)";
                    break;
                default:
                    versionString = "(unknown)";
                    break;
            }
            keyphraseList.add(item.getSoundModelPrettyKeyphrase() +  versionString);
        }

        if (keyphraseList.isEmpty()) {
            LogUtils.d(TAG, "showSelectKeyphraseDialog: no available sound models");
            return;
        }

        keyphraseList.add(getString(R.string.create_your_own_3_0));
        keyphraseList.add(getString(R.string.create_your_own_4_0));
        final CharSequence[] items = keyphraseList.toArray(
                new CharSequence[keyphraseList.size()]);
        final AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(this);
        dialogBuilder.setTitle(getString(R.string.select_one_keyphrase))
                .setCancelable(true)
                .setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        dialogInterface.dismiss();
                        mDialogSelectKeyphrase = null;
                    }
                })
                .setSingleChoiceItems(items, mSelectedIndex, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int which) {
                        mSelectedIndex = which;
                        LogUtils.d(TAG, "showSelectKeyphraseDialog:onClick: single choice "
                                + "mSelectedIndex = " + mSelectedIndex);
                    }
                })
                .setPositiveButton(R.string.next, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int which) {
                        // get sound model name or keyphrase
                        if (-1 == mSelectedIndex || mSelectedIndex > (mTrainableSmList.size() + 1)) {
                            Toast.makeText(getApplicationContext(),
                                    getString(R.string.select_one_item_first),
                                    Toast.LENGTH_LONG).show();
                            return;
                        }

                        String smName;
                        boolean isUdk;
                        if (mSelectedIndex < mTrainableSmList.size()) {
                            smName = mTrainableSmList.get(mSelectedIndex).getSoundModelFullFileName();
                            isUdk = false;
                        } else {
                            smName = items[mSelectedIndex].toString();
                            isUdk = true;
                        }

                        // dismiss this dialog
                        dialogInterface.dismiss();
                        mDialogSelectKeyphrase = null;
                        mSelectedIndex = -1;

                        LogUtils.d(TAG, "showSelectKeyphraseDialog:onClick: next button "
                                + "selected smName = " + smName);

                        // show name sound model or create keyphrase dialog
                        showNextDialog(smName, isUdk);

                    }
                });

        mDialogSelectKeyphrase = dialogBuilder.show();
    }

    private void showNextDialog(final String smName, final boolean isUdk) {
        LogUtils.d(TAG, "showNextDialog: smName = " + smName);

        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(this);
        View view = getLayoutInflater().inflate(R.layout.dialog_edit_text, null);
        String message;
        String title;
        if (isUdk) {
            title = getString(R.string.new_keyphrase_dialog_title);
            message = getString(R.string.new_keyphrase_dialog_message);
        } else {
            title = getString(R.string.new_sm_dialog_title);
            message = getString(R.string.new_sm_dialog_message);
        }

        dialogBuilder.setView(view)
                .setTitle(title)
                .setMessage(message)
                .setCancelable(false)
                .setNegativeButton(R.string.cancel, null)
                .setPositiveButton(R.string.next, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        EditText editText = mDialogNext.findViewById(R.id.et_param);
                        String inputText = editText.getText().toString();
                        String title = getString(R.string.friendly_tips);
                        String message;

                        // check whether is empty or not
                        if ("".equals(inputText)) {
                            LogUtils.d(TAG, "showNextDialog: invalid input value");
                            if (isUdk) {
                                message = getString(R.string.keyphrase_empty);
                            } else {
                                message = getString(R.string.sm_name_empty);
                            }
                            Utils.openAlertDialog(DebugModelListActivity.this, title, message);
                            return;
                        }

                        String keyphraseOrNewSmName = inputText;

                        // check if conflict or not
                        if (!isUdk) {
                            keyphraseOrNewSmName = inputText + ISmModel.SUFFIX_TRAINED_SOUND_MODEL;
                            String newSmPath = Global.PATH_ROOT + "/" + keyphraseOrNewSmName;
                            if (FileUtils.isExist(newSmPath)) {
                                message = getString(R.string.name_conflict_message);
                                Utils.openAlertDialog(DebugModelListActivity.this, title,
                                        message);
                                return;
                            }
                        } else {
                            // for UDK, do not allowed input the UIM keyphrase
                            ArrayList<String> keyphraseList = Global.getInstance()
                                    .getExtendedSmMgr().getUIMKeyphraseList();
                            for (String item : keyphraseList) {
                                if (keyphraseOrNewSmName.equalsIgnoreCase(item)) {
                                    message = getString(R.string.keyphrase_conflict_message);
                                    Utils.openAlertDialog(DebugModelListActivity.this,
                                            title, message);
                                    return;
                                }
                            }
                        }

                        if (!isUdk) {
                            LogUtils.d(TAG, "showNextDialog: input new sm name = "
                                    + keyphraseOrNewSmName);
                        } else {
                            LogUtils.d(TAG, "showNextDialog: input keyphrase = "
                                    + keyphraseOrNewSmName);
                        }

                        // show add new user dialog
                        showAddUserDialog(smName, keyphraseOrNewSmName);

                        // dismiss this dialog
                        mDialogNext.dismiss();
                        mDialogNext = null;
                    }
                });

        mDialogNext = dialogBuilder.show();
        EditText editText = mDialogNext.findViewById(R.id.et_param);
        editText.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                if (hasFocus) {
                    Window window = mDialogNext.getWindow();
                    if (null != window) {
                        window.setSoftInputMode(
                                WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);
                    }
                }
            }
        });
        editText.requestFocus();
    }

    private void showAddUserDialog(final String oldSmName, final String keyphraseOrNewSmName) {
        LogUtils.d(TAG, "showAddUserDialog: oldSmName = " + oldSmName
                + " keyphraseOrNewSmName = " + keyphraseOrNewSmName);
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
                            Utils.openAlertDialog(DebugModelListActivity.this, title, message);
                            return;
                        }

                        // start training activity
                        startTrainingActivity(oldSmName, keyphraseOrNewSmName, userName);

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

    private void startTrainingActivity(String oldSmName,
                                       String keyphraseOrNewSmName, String userName) {
        LogUtils.d(TAG, "startTrainingActivity: oldSmName = " + oldSmName
                + " keyphraseOrNewSmName = " + keyphraseOrNewSmName
                + " userName = " + userName);

        Intent intent = new Intent(this, TrainingActivity.class);
        intent.putExtra(TrainingActivity.KEY_EXTRA_PREVIOUS_SM_NAME, oldSmName);
        intent.putExtra(TrainingActivity.KEY_EXTRA_KEYPHRASE_OR_NEW_SM_NAME, keyphraseOrNewSmName);
        intent.putExtra(TrainingActivity.KEY_EXTRA_USER_NAME, userName);
        intent.putExtra(TrainingActivity.KEY_EXTRA_IS_ADD_USER_TO_PREVIOUS_MODEL, false);
        startActivity(intent);
    }

    private void handleTrainingMenuItemSelected() {
        LogUtils.d(TAG, "handleTrainingMenuItemSelected: enter");
        ExtendedSmMgr smMgr = Global.getInstance().getExtendedSmMgr();
        if (smMgr.hasActiveSessions()) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.friendly_tips)
                    .setMessage(R.string.close_all_sessions_first)
                    .setCancelable(false)
                    .setNegativeButton(R.string.cancel, null)
                    .setPositiveButton(R.string.confirm,
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    sendRequest(Msg.MSG_RELEASE_ALL_SESSIONS_REQ, null);
                                    showSelectKeyphraseDialog();
                                }
                            });

            if (!((DebugModelListActivity.this).isFinishing())) {
                builder.show();
            }
        } else {
            showSelectKeyphraseDialog();
        }
    }

    private class MergeModelTask extends AsyncTask<String, Void, Integer> {

        @Override
        protected Integer doInBackground(String... strings) {
            String mergedSmName = strings[0];
            LogUtils.d(TAG, "MergeModelTask:doInBackground: mergedSmName = "
                    + mergedSmName);
            ArrayList<ByteBuffer> arrayList = new ArrayList<>();
            for (String item : mCheckedSmNameList) {
                try {
                    ByteBuffer byteBuffer = FileUtils.readFileToByteBuffer(
                            Global.getInstance().getExtendedSmMgr().getSoundModelFullPath(item));
                    arrayList.add(byteBuffer);
                } catch (IOException e) {
                    e.printStackTrace();
                    return -1;
                }
            }

            ByteBuffer[] byteBuffers = arrayList.toArray(
                    new ByteBuffer[mCheckedSmNameList.size()]);
            int sizeAfterMerged = ListenSoundModel.getSizeWhenMerged(byteBuffers);
            if (sizeAfterMerged <= 0) {
                LogUtils.d(TAG, "MergeModelTask:doInBackground: sizeAfterMerged exception");
                return -1;
            }

            ByteBuffer mergedSmData = ByteBuffer.allocate(sizeAfterMerged);
            int result = ListenSoundModel.merge(byteBuffers, mergedSmData);
            LogUtils.d(TAG, "MergeModelTask:doInBackground: merge result = " + result);
            if (ListenTypes.STATUS_SUCCESS == result) {
                // save to file
                String savePath = Global.getInstance().getExtendedSmMgr()
                        .getSoundModelFullPath(mergedSmName);
                boolean bSuccess = FileUtils.saveByteBufferToFile(mergedSmData, savePath);
                if (bSuccess) {
                    // add sm mem data
                    Global.getInstance().getExtendedSmMgr().addSoundModel(getApplicationContext(),
                            mergedSmName);
                    return 0;
                } else {
                    return -1;
                }
            }

            return result;
        }

        @Override
        protected void onPostExecute(Integer result) {
            LogUtils.d(TAG, "MergeModelTask:onPostExecute: result = " + result);

            // failure case
            if (0 == result.intValue()) {
                Toast.makeText(getApplicationContext(), getString(R.string.merge_success),
                        Toast.LENGTH_SHORT).show();
            } else {
                showMergeSmFailDialog(result.intValue());
            }
        }
    }
}
