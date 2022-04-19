#!/bin/sh

# Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

# call this from project root to update data make and bp files
QTI_IMS_HAL_COMPONENTS=(
    "vendor.qti.ims.callinfo@1.0"
)

QTI_IMS_HAL_SRC_FOLDER=vendor/qcom/proprietary/interfaces

for i in "${QTI_IMS_HAL_COMPONENTS[@]}"
do
    echo "Update $i Android.bp"
    hidl-gen -Landroidbp -r vendor.qti:$QTI_IMS_HAL_SRC_FOLDER \
        -randroid.hidl:system/libhidl/transport $i
done
