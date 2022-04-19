#===========================================================================
# Copyright (c) 2019 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
#===========================================================================

ifeq ($(filter $(TARGET_PRODUCT), lito kona msmnile),$(TARGET_PRODUCT)

LOCAL_PATH:= $(call my-dir)

# ACVP.py build
include $(CLEAR_VARS)

LOCAL_MODULE := ACVP.py
LOCAL_SRC_FILES := ACVP.py
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_SUFFIX := #empty
LOCAL_IS_HOST_MODULE := true

include $(BUILD_PREBUILT)

endif
