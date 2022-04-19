LOCAL_PATH := $(call my-dir)
PREBUILT_DIR_PATH := $(LOCAL_PATH)

ifeq ($(call is-board-platform,sdm660),true)
ifeq ($(strip $(TARGET_BOARD_SUFFIX)),_64)
  -include $(LOCAL_PATH)/target/product/sdm660_64/Android.mk
endif
endif

-include $(sort $(wildcard $(PREBUILT_DIR_PATH)/*/*/Android.mk))
