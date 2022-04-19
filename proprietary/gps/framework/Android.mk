LOCAL_PATH := $(call my-dir)
include $(addsuffix /Android.mk, $(addprefix $(LOCAL_PATH)/, utils native))
