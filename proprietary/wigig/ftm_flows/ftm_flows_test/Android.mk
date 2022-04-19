LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ftm_flows_test
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti

LOCAL_CFLAGS := -Wno-unused-parameter

LOCAL_SHARED_LIBRARIES := libwigig_ftm_flows

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/libwigig_ftm_flows

LOCAL_SRC_FILES := $(shell find $(LOCAL_PATH) -type f \( -name "*.c" \) | sed s:^$(LOCAL_PATH)::g)

include $(BUILD_EXECUTABLE)
