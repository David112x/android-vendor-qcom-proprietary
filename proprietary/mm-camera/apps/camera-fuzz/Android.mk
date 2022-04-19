
# -------------------------------------------------------
# Camera HAL : Fuzzing wrapper module
# -------------------------------------------------------
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng

LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/..
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../..
LOCAL_C_INCLUDES+= \
        $(TARGET_OUT_HEADERS)/mm-camera \
        $(TARGET_OUT_HEADERS)/mm-camera/common \
        $(TARGET_OUT_HEADERS)/mm-still \
        $(TARGET_OUT_HEADERS)/mm-still/jpeg \
        $(TARGET_OUT_HEADERS)/mm-camera-interface \

      ifeq ($(V4L2_BASED_LIBCAM),true)
        LOCAL_C_INCLUDES+= $(TARGET_OUT_HEADERS)/mm-core/omxcore
        LOCAL_C_INCLUDES+= $(TARGET_OUT_HEADERS)/mm-still/mm-omx
      endif
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += hardware/qcom/display/libgralloc \
        hardware/qcom/display/libgenlock \
        hardware/qcom/media/libstagefrighthw \
        hardware/qcom/camera 

LOCAL_HEADER_LIBRARIES := libcutils_headers
LOCAL_HEADER_LIBRARIES += liblog_headers
LOCAL_HEADER_LIBRARIES += libhardware_headers

LOCAL_SRC_FILES  := QCamera2HAL.cpp
LOCAL_SRC_FILES  += QCamera2HALFuzzLib.cpp
LOCAL_SRC_FILES  += QCamera2HALFuzzLibHelper.cpp
LOCAL_SRC_FILES  += FuzzerUtil.cpp
LOCAL_MODULE     := libFuzzQCamera2HALFuzzLib
include $(SDCLANG_COMMON_DEFS)

LOCAL_CFLAGS     := -DANDROID_BUILD
LOCAL_CFLAGS     += -fexceptions
LOCAL_CFLAGS     += -error-unresolved-symbols

LOCAL_SHARED_LIBRARIES := libcutils libdl libhardware liblog

LOCAL_PRELINK_MODULE   := FALSE

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)


