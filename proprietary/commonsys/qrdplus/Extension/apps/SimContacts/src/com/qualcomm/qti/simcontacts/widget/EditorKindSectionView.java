/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts.widget;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.drawable.Drawable;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;

import com.qualcomm.qti.simcontacts.R;

public class EditorKindSectionView extends LinearLayout {
    private final Drawable mSectionDrawable;
    private final boolean mDeletable;
    private final String mEditorHint;
    private final int mEditorInputType;
    private EditText mEditText;
    private String mEditTextInitialValue = "";

    public EditorKindSectionView(Context context) {
        this(context, null);
    }

    public EditorKindSectionView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public EditorKindSectionView(Context context, AttributeSet attrs, int defStyleAttr) {
        this(context, attrs, defStyleAttr, 0);
    }

    public EditorKindSectionView(Context context, AttributeSet attrs, int defStyleAttr,
                                 int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        final TypedArray a = context.obtainStyledAttributes(
                attrs, R.styleable.EditorKindSectionView, defStyleAttr, defStyleRes);
        mSectionDrawable = a.getDrawable(R.styleable.EditorKindSectionView_sectionDrawable);
        mDeletable = a.getBoolean(R.styleable.EditorKindSectionView_deletable, true);
        mEditorHint = a.getString(R.styleable.EditorKindSectionView_editorHint);
        mEditorInputType = a.getInt(R.styleable.EditorKindSectionView_editorInputType, 1);
        a.recycle();
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        final LayoutInflater inflater = (LayoutInflater) getContext().getSystemService(
                Context.LAYOUT_INFLATER_SERVICE);
        inflater.inflate(R.layout.editor_kind_section_layout, this, true);
        ImageView kindIcon = findViewById(R.id.kind_icon);
        kindIcon.setImageDrawable(mSectionDrawable);
        mEditText = findViewById(R.id.kind_editor);
        mEditText.setHint(mEditorHint);
        mEditText.setInputType(mEditorInputType);
        final ImageView deleteButton = findViewById(R.id.delete_button);
        deleteButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mEditText.setText("");
            }
        });
        mEditText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {
                if (mDeletable){
                    deleteButton.setVisibility(!TextUtils.isEmpty(charSequence) ?
                            VISIBLE : INVISIBLE);
                }
            }

            @Override
            public void afterTextChanged(Editable editable) {

            }
        });
    }

    public void setInitialValue(String initialValue){
        if (!TextUtils.isEmpty(initialValue)) {
            mEditText.setText(initialValue);
            mEditTextInitialValue = initialValue;
        }
    }

    public String getEditorTextContent(){
        return mEditText.getText().toString();
    }

    public boolean hasPendingChange(){
        final String content = getEditorTextContent().trim();
        return !(content.equals(mEditTextInitialValue));
    }
}
