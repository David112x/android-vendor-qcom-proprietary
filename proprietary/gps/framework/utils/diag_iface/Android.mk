ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH := $(call my-dir)

# ---------------------------------------------------------------------------------
# Diag interface Library
# ---------------------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := liblocdiagiface
LOCAL_SANITIZE += $(GNSS_SANITIZE)
# activate the following line for debug purposes only, comment out for production
#LOCAL_SANITIZE_DIAG += $(GNSS_SANITIZE_DIAG)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_CFLAGS := $(COMMON_CFLAGS)
LOCAL_CFLAGS += -fno-short-enums
ifeq (, $(filter aarch64 arm64, $(TARGET_ARCH)))
LOCAL_CFLAGS += -DANDROID_32_BIT_PTR_SUPPORT
endif

LOCAL_SHARED_LIBRARIES := \
    libgps.utils \
    libcutils \
    libdiag \
    liblog \
    libutils

LOCAL_C_INCLUDES += \
  $(LOCAL_PATH) \
  $(TARGET_OUT_HEADERS)/common/inc \
  $(TARGET_OUT_HEADERS)/diag/include

LOCAL_HEADER_LIBRARIES := \
    libgps.utils_headers \
    libloc_pla_headers

LOCAL_SRC_FILES:= \
    LocDiagIface.cpp

LOCAL_CFLAGS += $(QC_LOC_FW_LOCAL_C_FLAGS) \
    $(GNSS_CFLAGS)

LOCAL_MODULE_OWNER := qti

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libloc_diag_iface_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := \
    $(LOCAL_PATH)/
include $(BUILD_HEADER_LIBRARY)

endif # not BUILD_TINY_ANDROID
