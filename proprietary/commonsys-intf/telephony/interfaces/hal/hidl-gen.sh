#!/bin/sh

# Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

# call this from project root to update radio make and bp files

QTI_RADIO_HAL_COMPONENTS=(
    "vendor.qti.hardware.radio.am@1.0"
    "vendor.qti.hardware.radio.atcmdfwd@1.0"
    "vendor.qti.hardware.radio.ims@1.0"
    "vendor.qti.hardware.radio.ims@1.1"
    "vendor.qti.hardware.radio.ims@1.2"
    "vendor.qti.hardware.radio.ims@1.3"
    "vendor.qti.hardware.radio.ims@1.4"
    "vendor.qti.hardware.radio.ims@1.5"
    "vendor.qti.hardware.radio.ims@1.6"
    "vendor.qti.hardware.radio.qcrilhook@1.0"
    "vendor.qti.hardware.radio.qtiradio@1.0"
    "vendor.qti.hardware.radio.qtiradio@2.0"
    "vendor.qti.hardware.radio.qtiradio@2.1"
    "vendor.qti.hardware.radio.qtiradio@2.2"
    "vendor.qti.hardware.radio.qtiradio@2.3"
    "vendor.qti.hardware.radio.qtiradio@2.4"
    "vendor.qti.hardware.radio.uim_remote_client@1.0"
    "vendor.qti.hardware.radio.uim_remote_client@1.1"
    "vendor.qti.hardware.radio.uim_remote_client@1.2"
    "vendor.qti.hardware.radio.uim_remote_server@1.0"
    "vendor.qti.hardware.radio.lpa@1.0"
    "vendor.qti.hardware.radio.uim@1.0"
    "vendor.qti.hardware.radio.uim@1.1"
    "vendor.qti.hardware.radio.uim@1.2"
    "vendor.qti.hardware.radio.internal.deviceinfo@1.0"
)

QTI_RADIO_HAL_SRC_FOLDER=vendor/qcom/proprietary/commonsys-intf/telephony/interfaces/hal

for i in "${QTI_RADIO_HAL_COMPONENTS[@]}"
do
    echo "Update $i Android.bp"
    hidl-gen -Landroidbp -r vendor.qti.hardware.radio:$QTI_RADIO_HAL_SRC_FOLDER \
        -randroid.hidl:system/libhidl/transport $i
done
