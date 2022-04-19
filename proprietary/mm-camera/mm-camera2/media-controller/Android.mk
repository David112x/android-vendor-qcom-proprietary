ifeq ($(call is-vendor-board-platform,QCOM),true)

#************* media-controller Makefile start ************#
LOCAL_PATH := $(call my-dir)
LOCAL_DIR_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS :=  -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags) \
  -DMSM_CAMERA_BIONIC

LOCAL_CFLAGS  += -D_ANDROID_
LOCAL_CFLAGS  += -Wall -Wno-unused-parameter -Wno-unused-function

ifeq ($(call is-board-platform-in-list, msm8610),true)
 FEATURE_GYRO := false
endif

#LOCAL_CFLAGS += -Werror

ifneq ($(call is-platform-sdk-version-at-least,17),true)
  LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/types.h
  LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/socket.h
  LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/in.h
  LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/un.h
endif

ifeq ($(strip $(TARGET_USES_ION)),true)
  LOCAL_CFLAGS += -DUSE_ION
  ifeq ($(MSM_VERSION),7x27A)
    LOCAL_CFLAGS += -DTARGET_7x27A
  endif
endif

ifeq ($(VFE_VERS),vfe40)
  LOCAL_CFLAGS += -DVFE_40
else ifeq ($(VFE_VERS),vfe32)
  LOCAL_CFLAGS += -DVFE_32
  ifeq ($(FEATURE_GYRO), true)
    LOCAL_CFLAGS += -DFEATURE_GYRO
  endif
else ifeq ($(VFE_VERS),vfe31)
  LOCAL_CFLAGS += -DVFE_31
  ifeq ($(MSM_VERSION), 7x30)
    LOCAL_CFLAGS += -DVFE_31_7x30
  else
    LOCAL_CFLAGS += -DVFE_31_8x60
  endif
else ifeq ($(VFE_VERS),vfe2x)
  LOCAL_CFLAGS += -DVFE_2X
endif

#ifeq ($(MM_DEBUG),true) # does not compile is not defined
LOCAL_CFLAGS+= -DIPL_DEBUG_STANDALONE
#endif

ifeq ($(TARGET_HAS_LOW_RAM),true)
    LOCAL_CFLAGS += -DCAMERA_FEATURE_LOW_RAM
endif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../server-imaging/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../server-tuning/tuning/

LOCAL_C_INCLUDES += $(LOCAL_PATH)/modules/includes/

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

MM_DEBUG := true

#************* mct shim layer lib start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/mct_shim_layer/Android.mk
#************* mct shim layer libs end ************#

#************* mct libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/mct/Android.mk
#************* mct libs end ************#

#************* sensor module libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/sensors/Android.mk
#************* sensor libs end ************#

#************* stats libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/stats/Android.mk
#************* stats libs end ************#

#************* isp libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/isp2/Android.mk
#************* isp libs end ************#

#************* iface libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/iface2/Android.mk
#************* iface libs end ************#

#************* pproc libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/pproc-new/Android.mk
#************* pproc libs end ************#

#************* imglib libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/imglib/Android.mk
#************* imglib libs end ************#

endif
#************* media-controller Makefile end ************#
