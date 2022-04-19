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
package com.android.contacts.database;

import android.content.ContentProviderResult;
import android.content.Context;
import android.content.OperationApplicationException;
import android.net.Uri;
import android.os.RemoteException;
import android.support.annotation.VisibleForTesting;

import com.android.contacts.model.SimCard;
import com.android.contacts.model.SimContact;
import com.android.contacts.model.account.AccountWithDataSet;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Provides data access methods for loading contacts from a SIM card and and migrating these
 * SIM contacts to a CP2 account.
 */
public abstract class SimContactDao {

    // Set to true for manual testing on an emulator or phone without a SIM card
    // DO NOT SUBMIT if set to true
    private static final boolean USE_FAKE_INSTANCE = false;
    private static SimContactDao sSimContactDao;

    private static SimContactDao createDebugInstance(Context context) {
        return new SimContactDaoImpl.DebugImpl(context)
                .addSimCard(new SimCard("fake-sim-id1", 0, 1, "Fake Carrier",
                        "Card 1", "15095550101", "us").withContacts(
                        new SimContact(1, "Sim One", "15095550111", null),
                        new SimContact(2, "Sim Two", "15095550112", null),
                        new SimContact(3, "Sim Three", "15095550113", null),
                        new SimContact(4, "Sim Four", "15095550114", null),
                        new SimContact(5, "411 & more", "411", null)
                ))
                .addSimCard(new SimCard("fake-sim-id2", 1, 2, "Carrier Two",
                        "Card 2", "15095550102", "us").withContacts(
                        new SimContact(1, "John Sim", "15095550121", null),
                        new SimContact(2, "Bob Sim", "15095550122", null),
                        new SimContact(3, "Mary Sim", "15095550123", null),
                        new SimContact(4, "Alice Sim", "15095550124", null),
                        new SimContact(5, "Sim Duplicate", "15095550121", null)
                ));
    }

    public static SimContactDao create(Context context) {
        if (sSimContactDao == null){
            synchronized (SimContactDao.class){
                if (sSimContactDao == null) {
                    sSimContactDao = USE_FAKE_INSTANCE ? createDebugInstance(context)
                            : new SimContactDaoImpl(context);
                }
            }
        }
        return sSimContactDao;
    }

    public abstract boolean canReadSimContacts();

    public abstract List<SimCard> getSimCards();

    public abstract ArrayList<SimContact> loadContactsForSim(SimCard sim);

    public abstract boolean deleteSimContact(SimContact simContact, int subscriptionId);

    public abstract Uri createNewSimContact(SimContact simContact, int subscriptionId);

    public abstract ContentProviderResult[] exportContacts(List<SimContact> contacts)
            throws RemoteException, OperationApplicationException;

    public abstract ContentProviderResult[] exportContacts(List<SimContact> contacts,
            AccountWithDataSet targetAccount)
            throws RemoteException, OperationApplicationException;

    public abstract int editExistingSimContact(SimContact oldSimContact,
                    SimContact newSimContact, int subscriptionId);

    public abstract void persistSimStates(List<SimCard> simCards);

    public abstract SimCard getSimBySubscriptionId(int subscriptionId);

    public void persistSimState(SimCard sim) {
        persistSimStates(Collections.singletonList(sim));
    }
}
