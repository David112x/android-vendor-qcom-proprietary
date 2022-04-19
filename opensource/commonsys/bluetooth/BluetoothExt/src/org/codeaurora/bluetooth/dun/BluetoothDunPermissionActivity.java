/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *        * Redistributions of source code must retain the above copyright
 *            notice, this list of conditions and the following disclaimer.
 *        * Redistributions in binary form must reproduce the above copyright
 *            notice, this list of conditions and the following disclaimer in the
 *            documentation and/or other materials provided with the distribution.
 *        * Neither the name of The Linux Foundation nor
 *            the names of its contributors may be used to endorse or promote
 *            products derived from this software without specific prior written
 *            permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.    IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package org.codeaurora.bluetooth.dun;

import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.Preference;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.CompoundButton.OnCheckedChangeListener;

import org.codeaurora.bluetooth.R;
import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

/**
 * DunPermissionActivity shows dialogues for accepting incoming DUN request
 * with a remote Bluetooth device.
  */

public class BluetoothDunPermissionActivity extends AlertActivity implements
        DialogInterface.OnClickListener, Preference.OnPreferenceChangeListener, TextWatcher {
    private static final String TAG = "BluetoothDunPermissionActivity";

    private static final boolean V = BluetoothDunService.VERBOSE;

    private static final int DIALOG_YES_NO_CONNECT = 1;

    private static final String KEY_USER_TIMEOUT = "user_timeout";

    private View mView;

    private EditText mKeyView;

    private TextView messageView;

    private int mCurrentDialog;

    private Button mOkButton;

    private CheckBox mAlwaysAllowed;

    private boolean mTimeout = false;

    private boolean mAlwaysAllowedValue = false;

    private static final int DISMISS_TIMEOUT_DIALOG = 0;

    private static final int DISMISS_TIMEOUT_DIALOG_VALUE = 2000;

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (!BluetoothDunService.USER_CONFIRM_TIMEOUT_ACTION.equals(intent.getAction())) {
                return;
            }
            onTimeout();
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent i = getIntent();
        String action = i.getAction();
        if (V) Log.v(TAG , "action :" + action);
        if (action == null) return;
        if (BluetoothDunService.DUN_ACCESS_REQUEST_ACTION.equals(action)) {
            showDunDialog(DIALOG_YES_NO_CONNECT);
            mCurrentDialog = DIALOG_YES_NO_CONNECT;
        }
        else {
            Log.e(TAG, "Error: this activity may be started only with intent "
                    + "DUN_ACCESS_REQUEST");
            finish();
        }
        registerReceiver(mReceiver, new IntentFilter(
                BluetoothDunService.USER_CONFIRM_TIMEOUT_ACTION));
    }

    private void showDunDialog(int id) {
        final AlertController.AlertParams p = mAlertParams;
        switch (id) {
            case DIALOG_YES_NO_CONNECT:
                p.mIconId = android.R.drawable.ic_dialog_info;
                p.mTitle = getString(R.string.bluetooth_dun_request);
                p.mView = createView(DIALOG_YES_NO_CONNECT);
                p.mPositiveButtonText = getString(android.R.string.yes);
                p.mPositiveButtonListener = this;
                p.mNegativeButtonText = getString(android.R.string.no);
                p.mNegativeButtonListener = this;
                mOkButton = mAlert.getButton(DialogInterface.BUTTON_POSITIVE);
                setupAlert();
                break;
            default:
                break;
        }
    }

    private String getRemoteDeviceName() {
        String remoteDeviceName = null;
        Intent intent = getIntent();
        if (intent.hasExtra(BluetoothDunService.EXTRA_BLUETOOTH_DEVICE)) {
            BluetoothDevice device = intent.getParcelableExtra(BluetoothDunService.EXTRA_BLUETOOTH_DEVICE);
            if (device != null) {
                remoteDeviceName = device.getName();
            }
        }

        return (remoteDeviceName != null) ? remoteDeviceName : getString(R.string.defaultname);
    }

    private String createDisplayText(final int id) {
        String mRemoteName = getRemoteDeviceName();
        switch (id) {
        case DIALOG_YES_NO_CONNECT:
            String mMessage1 = getString(R.string.bluetooth_dun_acceptance_dialog_text, mRemoteName,
                    mRemoteName);
            return mMessage1;
        default:
            return null;
        }
    }

    private View createView(final int id) {
        switch (id) {
            case DIALOG_YES_NO_CONNECT:
                mView = getLayoutInflater().inflate(R.layout.bluetooth_access, null);
                messageView = (TextView)mView.findViewById(R.id.message);
                messageView.setText(createDisplayText(id));
                mAlwaysAllowed = (CheckBox)mView.findViewById(R.id.alwaysallowed);
                mAlwaysAllowed.setOnCheckedChangeListener(new OnCheckedChangeListener() {
                    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                        if (isChecked) {
                            mAlwaysAllowedValue = true;
                        } else {
                            mAlwaysAllowedValue = false;
                        }
                    }
                });
                return mView;
            default:
                return null;
        }
    }

    private void onPositive() {
        if (!mTimeout) {
            if (mCurrentDialog == DIALOG_YES_NO_CONNECT) {
                sendIntentToReceiver(BluetoothDunService.DUN_ACCESS_ALLOWED_ACTION,
                        BluetoothDunService.DUN_EXTRA_ALWAYS_ALLOWED, mAlwaysAllowedValue);
            }
        }
        mTimeout = false;
        finish();
    }

    private void onNegative() {
        if (mCurrentDialog == DIALOG_YES_NO_CONNECT) {
            sendIntentToReceiver(BluetoothDunService.DUN_ACCESS_DISALLOWED_ACTION, null, null);
        }
        finish();
    }

    private void sendIntentToReceiver(final String intentName, final String extraName,
            final String extraValue) {
        Intent intent = new Intent(intentName);
        intent.setClassName(BluetoothDunService.THIS_PACKAGE_NAME, BluetoothDunReceiver.class
                .getName());
        if (extraName != null) {
            intent.putExtra(extraName, extraValue);
        }
        intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
        sendBroadcast(intent);
    }

    private void sendIntentToReceiver(final String intentName, final String extraName,
            final boolean extraValue) {
        Intent intent = new Intent(intentName);
        intent.setClassName(BluetoothDunService.THIS_PACKAGE_NAME, BluetoothDunReceiver.class
                .getName());
        if (extraName != null) {
            intent.putExtra(extraName, extraValue);
        }
        Intent i = getIntent();
        if (i.hasExtra(BluetoothDunService.EXTRA_BLUETOOTH_DEVICE)) {
            intent.putExtra(BluetoothDunService.EXTRA_BLUETOOTH_DEVICE,
                (BluetoothDevice)i.getParcelableExtra(BluetoothDunService.EXTRA_BLUETOOTH_DEVICE));
        }
        i.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
        sendBroadcast(intent);
    }

    public void onClick(DialogInterface dialog, int which) {
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:
                onPositive();
                break;

            case DialogInterface.BUTTON_NEGATIVE:
                onNegative();
                break;
            default:
                break;
        }
    }

    private void onTimeout() {
        mTimeout = true;
        if (mCurrentDialog == DIALOG_YES_NO_CONNECT) {
            if(mView != null) {
                messageView.setText(getString(R.string.dun_acceptance_timeout_message,
                        getRemoteDeviceName()));
                mAlert.getButton(DialogInterface.BUTTON_NEGATIVE).setVisibility(View.GONE);
                mAlwaysAllowed.setVisibility(View.GONE);
                mAlwaysAllowed.clearFocus();
            }
        }

        mTimeoutHandler.sendMessageDelayed(mTimeoutHandler.obtainMessage(DISMISS_TIMEOUT_DIALOG),
                DISMISS_TIMEOUT_DIALOG_VALUE);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        mTimeout = savedInstanceState.getBoolean(KEY_USER_TIMEOUT);
        if (V) Log.v(TAG, "onRestoreInstanceState() mTimeout: " + mTimeout);

        if (mTimeout) {
            onTimeout();
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putBoolean(KEY_USER_TIMEOUT, mTimeout);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mReceiver);
    }

    public boolean onPreferenceChange(Preference preference, Object newValue) {
        return true;
    }

    public void beforeTextChanged(CharSequence s, int start, int before, int after) {
    }

    public void onTextChanged(CharSequence s, int start, int before, int count) {
    }

    public void afterTextChanged(android.text.Editable s) {
        if (s.length() > 0) {
            mOkButton.setEnabled(true);
        }
    }
    private final Handler mTimeoutHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case DISMISS_TIMEOUT_DIALOG:
                    if (V) Log.v(TAG, "Received DISMISS_TIMEOUT_DIALOG msg.");
                    finish();
                    break;
                default:
                    break;
            }
        }
    };
}
