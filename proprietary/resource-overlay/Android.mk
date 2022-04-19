BASE_PATH := $(call my-dir)

include $(BASE_PATH)/common/Android.mk
-include $(BASE_PATH)/$(TARGET_BOARD_PLATFORM)/Android.mk
