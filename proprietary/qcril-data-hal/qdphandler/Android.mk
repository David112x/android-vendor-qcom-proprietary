LOCAL_PATH := $(call my-dir)

include $(call all-subdir-makefiles)

DATAMODULE_DIR := ${LOCAL_PATH}/..
DATA_HAL_DIR := ${LOCAL_PATH}/../..



###################### Target ########################################
include $(CLEAR_VARS)

LOCAL_MODULE               := qdphandler
LOCAL_SANITIZE:=integer_overflow
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS          := optional

LOCAL_CFLAGS               += -Wall -Werror -Wno-macro-redefined
LOCAL_CXXFLAGS             += -std=c++14
LOCAL_SRC_FILES            += $(call all-cpp-files-under, legacy)
LOCAL_SRC_FILES            += $(call all-cpp-files-under, src)


LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/data/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi-framework/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

# Local
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/legacy

# LOCAL_SHARED_LIBRARIES += liblqe
LOCAL_SHARED_LIBRARIES += libqdp
# LOCAL_SHARED_LIBRARIES += libdsi_netctrl
LOCAL_SHARED_LIBRARIES += libqmi

$(info Done building qdphandler for target...)

# Currently being built in main module library to avoid some linkage issues
# TODO re-enable lib
#include $(BUILD_STATIC_LIBRARY)
