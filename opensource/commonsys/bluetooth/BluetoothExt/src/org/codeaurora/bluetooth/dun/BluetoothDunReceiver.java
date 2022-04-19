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

import android.app.ActivityManager;
import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Process;
import android.os.UserHandle;
import android.os.UserManager;
import android.content.pm.PackageManager;
import android.content.pm.UserInfo;
import android.util.Log;

public class BluetoothDunReceiver extends BroadcastReceiver {

    private static final String TAG = "BluetoothDunReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();

        Log.i(TAG, " action :" + action);

        if(action == null) return;

        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();

        if (adapter == null) {
            Log.w(TAG, " adapter null "); return;
        }

        Log.i(TAG, " State :" + adapter.getState());

        if (adapter.isEnabled()) {
            (new startDunService(context, intent)).start();
        }
    }

    private class startDunService extends Thread {

        Context mContext;
        Intent mIntent;

        startDunService(Context context, Intent intent){
            mContext = context;
            mIntent = intent;
        }

        @Override
        public void run() {
            try {
                if ((Binder.getCallingUid() != Process.SYSTEM_UID)
                    && (!checkIfCallerIsForegroundUser())) {
                    Log.w(TAG, "startDunService: not allowed for non-active and non system user");
                    return;
                } else {
                    Intent intentDun = new Intent(mContext, BluetoothDunService.class);
                    intentDun.setAction(mIntent.getAction());
                    intentDun.putExtras(mIntent);
                    mContext.startService(intentDun);
                }
            } catch (NullPointerException ex) {
              Log.w(TAG, "startDunService: Handled exception: " + ex.toString());
            }
         }

        private boolean checkIfCallerIsForegroundUser() {
            int foregroundUser;
            int callingUser = UserHandle.getCallingUserId();
            int callingUid = Binder.getCallingUid();
            long callingIdentity = Binder.clearCallingIdentity();
            UserManager um = (UserManager) mContext.getSystemService(Context.USER_SERVICE);
            UserInfo ui = um.getProfileParent(callingUser);
            int parentUser = (ui != null) ? ui.id : UserHandle.USER_NULL;
            int callingAppId = UserHandle.getAppId(callingUid);
            boolean valid = false;

            try {
                int systemUiUid = mContext.getPackageManager()
                            .getPackageUidAsUser("com.android.systemui",
                            PackageManager.MATCH_SYSTEM_ONLY, UserHandle.USER_SYSTEM);
                foregroundUser = ActivityManager.getCurrentUser();
                valid = (callingUser == foregroundUser) || parentUser == foregroundUser
                         || callingAppId == systemUiUid;
                if (!valid) {
                    Log.i(TAG, "checkIfCallerIsForegroundUser: valid=" + valid + " callingUser="
                            + callingUser + " parentUser=" + parentUser + " foregroundUser="
                            + foregroundUser);
                }
            } catch (PackageManager.NameNotFoundException ex) {
                Log.w(TAG, "checkIfCallerIsForegroundUser: Handled  exception: " + ex.toString());
            } finally {
                Binder.restoreCallingIdentity(callingIdentity);
            }
            return valid;
       }
   }

}
