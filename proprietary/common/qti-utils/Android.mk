LOCAL_PATH := $(call my-dir)

# =================================
# copy header
# =================================
include $(CLEAR_VARS)

LOCAL_MODULE := libqti-utils_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_HEADER_LIBRARY)

# =================================
# libqti-utils.a
# =================================

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    memsutils.c

LOCAL_HEADER_LIBRARIES := \
    libqti-utils_headers

LOCAL_MODULE := libqti-utils
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MULTILIB := both

include $(BUILD_STATIC_LIBRARY)

# =================================
# libqti-utils.so
# =================================

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    memsutils.c

LOCAL_HEADER_LIBRARIES := \
    libqti-utils_headers

LOCAL_MODULE := libqti-utils
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MULTILIB := both

include $(BUILD_SHARED_LIBRARY)
