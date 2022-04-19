#
# Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#

ifneq ($(TARGET_NO_TELEPHONY), true)
ifneq ($(TARGET_HAS_LOW_RAM),true)

ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS), true)
ifneq (, $(filter aarch64, $(TARGET_ARCH)))
    $(info TODOAArch64: $(LOCAL_PATH)/Android.mk: Enable build support for 64 bit)
else
ifeq ($(call is-vendor-board-platform,QCOM),true)
LOCAL_PATH:= $(call my-dir)

ifneq ($(TARGET_HAS_LOW_RAM),true)

ifeq ($(TARGET_USES_QCOM_BSP_ATEL), true)

# Disable SimContacts because new solution doesn't need SIM accounts and
# data synchronization
ifeq (0, 1)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := SimContacts
LOCAL_CERTIFICATE := platform
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PRODUCT_MODULE := true

include $(BUILD_PACKAGE)
endif
endif
endif
endif
endif
endif
endif
endif # TARGET_NO_TELEPHONY

