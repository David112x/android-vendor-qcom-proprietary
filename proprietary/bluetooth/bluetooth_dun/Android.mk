ifeq ($(BOARD_HAVE_BLUETOOTH_QCOM),true)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
include $(LOCAL_PATH)/1.0/default/Android.mk
endif # BOARD_HAVE_BLUETOOTH_QCOM
