LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := init.qcom.data.test.rc

LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(OUT_DIR)/target/product/$(TARGET_DEVICE)/vendor/etc/init

LOCAL_SRC_FILES := $(LOCAL_MODULE)

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_PREBUILT)
