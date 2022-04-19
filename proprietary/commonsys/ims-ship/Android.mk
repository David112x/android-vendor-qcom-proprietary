IMS_DIR := $(call my-dir)
include $(CLEAR_VARS)
ifeq ($(TARGET_USES_IMS),true)
ifeq ($(call is-board-platform-in-list,msm8909 msm8953 msm8937 msm8952 msm8976 msm8996 msm8998 sdm660),true)
include $(IMS_DIR)/imscamera/Android.mk
endif
  include $(IMS_DIR)/vtext/Android.mk
  include $(IMS_DIR)/rcs/Android.mk
endif
