/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts.util;

import android.text.TextUtils;
import android.os.Bundle;

import com.android.contacts.model.SimContact;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class SimContactsSortAlgorithm {

    public static final String EXTRA_CONTACTS_INDEX_TITLES = "index_titles";
    public static final String EXTRA_CONTACTS_INDEX_COUNTS = "index_counts";

    public static Bundle defaultSort(ArrayList<SimContact> simContacts){
        ArrayList<SimContact> otherAlphabet = new ArrayList<>();
        ArrayList<SimContact> englishAlphabet = new ArrayList<>();
        ArrayList<SimContact> digit = new ArrayList<>();
        ArrayList<SimContact> symbol = new ArrayList<>();
        Map<String, Integer> maps = new TreeMap<>();
        String displayName;
        for (SimContact simContact : simContacts){
            displayName  = simContact.getItemLabel().trim().toLowerCase();
            if (!TextUtils.isEmpty(displayName)) {
                char name = displayName.charAt(0);
                if (Character.isLetter(name)) {
                    if (isEnglishAlphabet(name)) {
                        if (maps.containsKey("" + name)) {
                            int currentValue = maps.get("" + name).intValue();
                            maps.put("" + name, ++currentValue);
                        } else {
                            maps.put("" + name, 1);
                        }
                        englishAlphabet.add(simContact);
                    } else {
                        otherAlphabet.add(simContact);
                    }
                } else if (Character.isDigit(name)) {
                    digit.add(simContact);
                } else if (isSymbol(name)) {
                    symbol.add(simContact);
                }
            } else {
                symbol.add(simContact);
            }
        }

        simContacts.clear();
        List<String> indexTitles = new ArrayList<>();
        List<Integer> indexCounts = new ArrayList<>();

        if (symbol.size() > 0){
            indexTitles.add("...");
            indexCounts.add(symbol.size());
            simContacts.addAll(symbol);
        }

        if (englishAlphabet.size() > 0){
            Set<Map.Entry<String, Integer>> sets = maps.entrySet();
            for (Map.Entry<String, Integer> entry : sets){
                indexTitles.add(entry.getKey());
                indexCounts.add(entry.getValue());
            }

            simContacts.addAll(englishAlphabet);
        }

        if (digit.size() > 0){
            indexTitles.add("#");
            indexCounts.add(digit.size());
            simContacts.addAll(digit);
        }

        if (otherAlphabet.size() > 0){
            indexTitles.add("...");
            indexCounts.add(otherAlphabet.size());
            simContacts.addAll(otherAlphabet);
        }

        int[] newIndexCounts = new int[indexCounts.size()];
        for (int i=0; i < indexCounts.size(); i++){
            newIndexCounts[i] = indexCounts.get(i);
        }
        final Bundle extra = new Bundle();
        extra.putStringArray(EXTRA_CONTACTS_INDEX_TITLES,
                indexTitles.toArray(new String[indexTitles.size()]));

        extra.putIntArray(EXTRA_CONTACTS_INDEX_COUNTS, newIndexCounts);

        return extra;
    }

    private static boolean isSymbol(char name){
        String regEx="[`~!@#$%^&*()+=|{}':;',\\[\\].<>/?~！@#￥%……&*（）——+|{}【】‘；：”“’。，、？]";
        Pattern p= Pattern.compile(regEx);
        Matcher m=p.matcher(""+name);
        return m.find();
    }

    private static boolean isEnglishAlphabet(char name){
        return (name>='A' && name <='Z') || (name>='a' && name <='z');
    }
}
