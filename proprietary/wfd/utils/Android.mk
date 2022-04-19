LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# ------------------------------------------------------------------------------
#            Common definitons
# ------------------------------------------------------------------------------

LOCAL_CFLAGS := -D_ANDROID_
ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_CFLAGS += -DWFD_ICS
endif

ifeq ($(call is-board-platform-in-list,msm8996),true)
LOCAL_CFLAGS += -DDISPLAY_DS_APIS_AVAILABLE
endif

# ------------------------------------------------------------------------------
#            WFD COMMON UTILS SRC
# ------------------------------------------------------------------------------

LOCAL_SRC_FILES := \
    src/wfd_cfg_parser.cpp \
    src/wfd_netutils.cpp \
    src/wfd_cfg_utils.cpp

# ------------------------------------------------------------------------------
#             WFD COMMON UTILS INC
# ------------------------------------------------------------------------------

LOCAL_C_INCLUDES := $(LOCAL_PATH)/./inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/omxcore
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qcom/display

LOCAL_HEADER_LIBRARIES := libmmosal_headers display_headers libwfd_headers

# ------------------------------------------------------------------------------
#            WFD COMMON UTILS SHARED LIB
# ------------------------------------------------------------------------------

LOCAL_SHARED_LIBRARIES := libmmosal
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libnl
LOCAL_SHARED_LIBRARIES += libwfdmminterface
LOCAL_LDLIBS += -llog

LOCAL_32_BIT_ONLY := true
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= libwfdconfigutils

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
include $(BUILD_SHARED_LIBRARY)
include $(call all-makefiles-under,$(LOCAL_PATH))

# ------------------------------------------------------------------------------
#            END
# ------------------------------------------------------------------------------
