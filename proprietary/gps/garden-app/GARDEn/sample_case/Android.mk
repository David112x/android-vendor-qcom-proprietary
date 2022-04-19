LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libgardencasesample
LOCAL_SANITIZE += $(GNSS_SANITIZE)
# activate the following line for debug purposes only, comment out for production
#LOCAL_SANITIZE_DIAG += $(GNSS_SANITIZE_DIAG)
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES += \
    SampleCase.cpp \

LOCAL_CFLAGS += \
    -fno-short-enums \
    -fexceptions \

LOCAL_HEADER_LIBRARIES := \
    libgarden_headers \
    libgps.utils_headers \
    libloc_pla_headers

LOCAL_SHARED_LIBRARIES := \
    libgarden \
    liblog \
    libgps.utils

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)
