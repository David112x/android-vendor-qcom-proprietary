RCS_DIR := $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS),true)
include $(RCS_DIR)/java/PresenceApp/Android.mk
include $(RCS_DIR)/java/ConnectionManagerTestApp/Android.mk
include $(RCS_DIR)/java/uceShimService2/Android.mk
else #device does not support new uce features, fallback
include $(RCS_DIR)/java/uceShimService/Android.mk
endif

#include $(call first-makefiles-under, $(RCS_DIR))
