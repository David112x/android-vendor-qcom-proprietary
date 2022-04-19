LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_SRC_FILES:= \
	test_diag.c \

commonSharedLibraries :=libdiag \
	
LOCAL_MODULE:= test_diag
LOCAL_SHARED_LIBRARIES := $(commonSharedLibraries)
LOCAL_SHARED_LIBRARIES += liblog

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)

