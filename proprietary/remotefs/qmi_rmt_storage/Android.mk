LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := rmt_storage

ifeq ($(TARGET_USES_QMAA), true)
LOCAL_INIT_RC := vendor.qti.rmt_storage_qmaa.rc
else
LOCAL_INIT_RC := vendor.qti.rmt_storage.rc
endif

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi-framework/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/libmdmdetect/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES  := remote_storage_v01.c rmt_storage_svc.c

LOCAL_SHARED_LIBRARIES := libqmi_csi libqmi_common_so libcutils liblog libmdmdetect
LOCAL_HEADER_LIBRARIES := libcutils_headers liblog_headers

LOCAL_MODULE_TAGS := optional
LOCAL_CLANG := true
LOCAL_CFLAGS := -D_LARGEFILE64_SOURCE
LOCAL_CFLAGS += -DLOG_NIDEBUG=0
LOCAL_SANITIZE :=integer_overflow

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

# Remove linking the lpthreads library. lpthreads is included by
# default and explicitly including can cause compiler errors.
#
# Uncomment line below if
# building on very old builds that require explicitly including the lib.
#
# LDLIBS += -lpthread

include $(BUILD_EXECUTABLE)

recovery_rmt_storage_binary := $(call intermediates-dir-for,EXECUTABLES,rmt_storage)/rmt_storage
