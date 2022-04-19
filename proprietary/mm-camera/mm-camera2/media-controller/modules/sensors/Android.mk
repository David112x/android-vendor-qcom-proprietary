#======================================================================
#makefile for libmmcamera2_sensor_modules.so form mm-camera2
#======================================================================
ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(call is-board-platform-in-list,msm8974 msm8960 msm7627a msm8660 msm8916 msm8226 msm8952 msm8610 apq8084 msm8994 msm8996 msm8992 msm8937 msm8953 sdm660 msm8998 apq8098_latv $(TRINKET)),true)

LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS :=  -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags)

ifeq ($(call is-board-platform-in-list, apq8084),true)
LOCAL_CFLAGS  += -DAPQ8084_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8994),true)
LOCAL_CFLAGS  += -DMSM8994_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8992),true)
LOCAL_CFLAGS  += -DMSM8992_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8610),true)
LOCAL_CFLAGS  += -DMSM8610_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8226),true)
LOCAL_CFLAGS  += -DMSM8226_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8660),true)
LOCAL_CFLAGS  += -DMSM8660_SENSORS
else ifeq ($(call is-board-platform-in-list, msm7627a),true)
LOCAL_CFLAGS  += -DMSM7627A_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8960),true)
LOCAL_CFLAGS  += -DMSM8960_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8974),true)
LOCAL_CFLAGS  += -DMSM8974_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8916),true)
LOCAL_CFLAGS  += -DMSM8916_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8996),true)
LOCAL_CFLAGS  += -DMSM8996_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8952),true)
LOCAL_CFLAGS  += -DMSM8952_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8976),true)
LOCAL_CFLAGS  += -DMSM8976_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8937),true)
LOCAL_CFLAGS  += -DMSM8937_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8953),true)
LOCAL_CFLAGS  += -DMSM8953_SENSORS
else ifeq ($(call is-board-platform-in-list, sdm660 msm8998 apq8098_latv $(TRINKET)),true)
LOCAL_CFLAGS  += -DMSM8996_SENSORS
endif

LOCAL_CFLAGS += -Werror


LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_MMCAMERA_PATH  := $(LOCAL_PATH)/../../../../mm-camera2
LOCAL_SENSOR_PATH := $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/

LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/includes/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/bus/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/controller/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/event/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/object/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/pipeline/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/port/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/stream/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/tools/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/debug/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/debug/fdleak

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/server-tuning/tuning/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/sensor/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/sensor/libs/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/eebin/module/include/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/eebin/module/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/sensor/includes/
ifeq ($(OEM_CHROMATIX), true)
  LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camera-ext/mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
else
  LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
endif

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/ois/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/ois/libs/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/live_connect/
#LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/

#LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp/

LOCAL_C_INCLUDES   += external/zlib


LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += \
 $(LOCAL_PATH)/../../../../../../../../hardware/qcom/camera/QCamera2/stack/common
LOCAL_C_INCLUDES += $(SRC_XML_INCLUDE)
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common

include $(CLEAR_VARS)

LOCAL_CFLAGS  := -D_ANDROID_
#LOCAL_CFLAGS += -DCAMERA_FEATURE_THIRDPARTYEIS
LOCAL_CFLAGS  += -DCONFIG_XML_SYSTEM_PATH='"$(TARGET_COPY_OUT_VENDOR)/etc/camera/"'

ifeq ($(TARGET_KERNEL_VERSION), 4.9)
LOCAL_CFLAGS += -DUSE_4_9_DEFS
endif

ifeq ($(TARGET_KERNEL_VERSION), 4.14)
LOCAL_CFLAGS += -DUSE_4_9_DEFS
endif

ifeq ($(call is-board-platform-in-list, apq8084),true)
LOCAL_CFLAGS  += -DAPQ8084_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8994),true)
LOCAL_CFLAGS  += -DMSM8994_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8992),true)
LOCAL_CFLAGS  += -DMSM8992_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8610),true)
LOCAL_CFLAGS  += -DMSM8610_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8226),true)
LOCAL_CFLAGS  += -DMSM8226_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8660),true)
LOCAL_CFLAGS  += -DMSM8660_SENSORS
else ifeq ($(call is-board-platform-in-list, msm7627a),true)
LOCAL_CFLAGS  += -DMSM7627A_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8960),true)
LOCAL_CFLAGS  += -DMSM8960_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8974),true)
LOCAL_CFLAGS  += -DMSM8974_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8916),true)
LOCAL_CFLAGS  += -DMSM8916_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8996),true)
LOCAL_CFLAGS  += -DMSM8996_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8952),true)
LOCAL_CFLAGS  += -DMSM8952_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8976),true)
LOCAL_CFLAGS  += -DMSM8976_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8937),true)
LOCAL_CFLAGS  += -DMSM8937_SENSORS
else ifeq ($(call is-board-platform-in-list, msm8953),true)
LOCAL_CFLAGS  += -DMSM8953_SENSORS
else ifeq ($(call is-board-platform-in-list, sdm660 msm8998 apq8098_latv $(TRINKET)),true)
LOCAL_CFLAGS  += -DMSM8996_SENSORS
endif

LOCAL_CFLAGS += -Werror

LOCAL_MMCAMERA_PATH  := $(LOCAL_PATH)/../../../../mm-camera2

LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_SRC_DIR := $(LOCAL_PATH)/module \
                 $(LOCAL_PATH)/sensor/module \
                 $(LOCAL_PATH)/chromatix/module \
                 $(LOCAL_PATH)/actuator/module \
                 $(LOCAL_PATH)/ois/module \
                 $(LOCAL_PATH)/eeprom \
                 $(LOCAL_PATH)/flash/module \
                 $(LOCAL_PATH)/csiphy \
                 $(LOCAL_PATH)/csid \
                 $(LOCAL_PATH)/eebin \
                 $(LOCAL_PATH)/external \
                 $(LOCAL_PATH)/live_connect \
                 $(LOCAL_PATH)/ir_led \
                 $(LOCAL_PATH)/ir_cut \
                 $(LOCAL_PATH)/laser_led/module

LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_MMCAMERA_PATH  := $(LOCAL_PATH)/../../../../mm-camera2

LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/includes/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/bus/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/controller/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/event/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/object/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/pipeline/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/port/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/stream/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/tools/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/debug/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/server-tuning/tuning/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/sensor/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/sensor/libs/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/eebin/module/include/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/eebin/module/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/sensor/includes/
ifeq ($(OEM_CHROMATIX), true)
  LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camera-ext/mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
else
  LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
endif

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/ois/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/ois/libs/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/live_connect/


LOCAL_C_INCLUDES += external/zlib
LOCAL_C_INCLUDES += external/libxml2/include
LOCAL_C_INCLUDES += $(SRC_XML_INCLUDE)
LOCAL_C_INCLUDES += external/icu/icu4c/source/common

LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += \
                    $(LOCAL_PATH)/../../../../../../../../hardware/qcom/camera/QCamera2/stack/common

LOCAL_MODULE           := libmmcamera2_sensor_modules
include $(SDCLANG_COMMON_DEFS)

LOCAL_SHARED_LIBRARIES := libdl libcutils libmmcamera2_mct libz libxml2 libmmcamera_dbg liblog
LOCAL_SHARED_LIBRARIES += libmmcamera_pdaf libmmcamera_pdafcamif

LOCAL_LDFLAGS += -Wl,--wrap=malloc -Wl,--wrap=free -Wl,--wrap=realloc -Wl,--wrap=calloc
LOCAL_LDFLAGS += -Wl,--wrap=open -Wl,--wrap=close -Wl,--wrap=socket -Wl,--wrap=pipe -Wl,--wrap=mmap

LOCAL_MODULE_TAGS      := optional
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

ifeq ($(LOCAL_PROPRIETARY_MODULE), true)
LIB_PROPRIETARY_DIR ?=/system/vendor
  ifeq ($(LOCAL_32_BIT_ONLY), true)
    LOCAL_CFLAGS += -DMOD_SENSOR_LIB_PATH=\"$(LIB_PROPRIETARY_DIR)/lib\"
  else
    LOCAL_CFLAGS += -DMOD_SENSOR_LIB_PATH=\"$(LIB_PROPRIETARY_DIR)/lib64\"
  endif
else
LIB_PROPRIETARY_DIR ?=/system
 ifeq ($(LOCAL_32_BIT_ONLY), true)
  LOCAL_CFLAGS += -DMOD_SENSOR_LIB_PATH=\"$(LIB_PROPRIETARY_DIR)/lib\"
 else
  LOCAL_CFLAGS += -DMOD_SENSOR_LIB_PATH=\"$(LIB_PROPRIETARY_DIR)/lib64\"
 endif
endif

include $(BUILD_SHARED_LIBRARY)

#************* sensor libs start ************#
LOCAL_PATH := $(LOCAL_SENSOR_PATH)
include $(LOCAL_PATH)/sensor/libs/Android.mk
#************* sensor libs end ************#

#************* sensor libs start ************#
LOCAL_PATH := $(LOCAL_SENSOR_PATH)
include $(LOCAL_PATH)/eeprom/libs/Android.mk
#************* sensor libs end ************#

#************* actuator driver libs start ************#
LOCAL_PATH := $(LOCAL_SENSOR_PATH)
include $(LOCAL_SENSOR_PATH)/actuator/libs/Android.mk
#************* actuator driver libs end ************#

#************* eebin libs start ************#
LOCAL_PATH := $(LOCAL_SENSOR_PATH)
include $(LOCAL_PATH)/eebin/libs/Android.mk
#************* eebin libs end ************#

#************* ois driver libs start ************#
LOCAL_PATH := $(LOCAL_SENSOR_PATH)
include $(LOCAL_PATH)/ois/libs/Android.mk
#************* ois driver libs end ************#

#************* flash driver libs start ************#
LOCAL_PATH := $(LOCAL_SENSOR_PATH)
include $(LOCAL_PATH)/flash/libs/Android.mk
#************* flash driver libs end ************#

#************* chromatix libs start ************#
LOCAL_PATH := $(LOCAL_SENSOR_PATH)
CHROMATRIX_LOCAL_CFLAGS := -Werror
ifeq ($(OEM_CHROMATIX), true)
include $(LOCAL_MMCAMERA_PATH)/../../mm-camera-ext/mm-camera2/media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)/Android.mk
else
include $(LOCAL_PATH)/chromatix/$(CHROMATIX_VERSION)/Android.mk
endif
#************* chromatix libs end ************#

#************* configs start ************#
LOCAL_PATH := $(LOCAL_SENSOR_PATH)
include $(LOCAL_PATH)/configs/Android.mk
#************* configs end ************#
endif # if 8960
endif # is-vendor-board-platform,QCOM
