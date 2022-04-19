ifeq ($(call is-vendor-board-platform,QCOM),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
#
# unit test executables
#

# Global flag and include definitions
TEST_CFLAGS := -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags) \
  -DMSM_CAMERA_BIONIC

TEST_CFLAGS  += -Werror

ifneq ($(call is-platform-sdk-version-at-least,17),true)
  TEST_CFLAGS += -include bionic/libc/kernel/common/linux/types.h
  TEST_CFLAGS += -include bionic/libc/kernel/common/linux/socket.h
  TEST_CFLAGS += -include bionic/libc/kernel/common/linux/in.h
  TEST_CFLAGS += -include bionic/libc/kernel/common/linux/un.h
endif

TEST_C_INCLUDES:= $(LOCAL_PATH)
TEST_C_INCLUDES+= $(LOCAL_PATH)/../includes/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/bus/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/controller/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/object/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/includes/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/tools/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/event/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/pipeline/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/stream/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/module/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/port/
TEST_C_INCLUDES+= \
 $(LOCAL_PATH)/../../../../../../hardware/qcom/camera/QCamera2/stack/common
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../system/core/libion/kernel-headers
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../ system/core/libion/include
endif
#
# test_pipeline
#

include $(LOCAL_PATH)/../../local_additional_dependency.mk

#LOCAL_LDFLAGS := $(mmcamera_debug_lflags)
#LOCAL_CFLAGS := $(TEST_CFLAGS)
#LOCAL_C_INCLUDES := $(TEST_C_INCLUDES)
#LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
#LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

#LOCAL_SRC_FILES:= test_pipeline.c

#LOCAL_SHARED_LIBRARIES:= libcutils libmmcamera2_mct.so

#LOCAL_MODULE:= test_pipeline
#LOCAL_MODULE_TAGS := optional

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

#include $(BUILD_EXECUTABLE)

#
# test_list
#
include $(CLEAR_VARS)

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := $(TEST_CFLAGS)
LOCAL_CFLAGS  += -D_ANDROID_

LOCAL_C_INCLUDES := $(TEST_C_INCLUDES)
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_SRC_FILES:= test_list.c

LOCAL_SHARED_LIBRARIES:= libcutils liblog libmmcamera2_mct libmmcamera_dbg
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES += libion
endif
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE:= test_list
LOCAL_MODULE_TAGS := optional

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)

# testgen_isp
include $(CLEAR_VARS)

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := $(TEST_CFLAGS)
LOCAL_C_INCLUDES := $(TEST_C_INCLUDES)
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += system/core/libion/include
LOCAL_C_INCLUDES += system/core/libion/kernel-headers/
endif
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
endif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/includes/

ifeq ($(VFE_VERS),vfe40)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/testgen_isp/include44
LOCAL_CFLAGS += -DVFE40
else ifeq ($(VFE_VERS),vfe44)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/testgen_isp/include44
LOCAL_CFLAGS += -DVFE44
else ifeq ($(VFE_VERS),vfe46)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/testgen_isp/include46
LOCAL_CFLAGS += -DVFE46
else ifeq ($(VFE_VERS),vfe47)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/testgen_isp/include47
LOCAL_CFLAGS += -DVFE47
else ifeq ($(VFE_VERS),vfe48)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/testgen_isp/include47
LOCAL_CFLAGS += -DVFE47
endif

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_SRC_FILES:= testgen_isp/testgen_isp.c

LOCAL_SHARED_LIBRARIES:= libcutils libmmcamera2_mct libmmcamera_dbg
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE:= testgen_isp_test
LOCAL_MODULE_TAGS := optional

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)

#
# test_sensor
#
include $(CLEAR_VARS)

ifeq ($(OEM_CHROMATIX_0308), 1)
CHROMATIX_VERSION := 0308E
else
CHROMATIX_VERSION := 0309
endif

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := $(TEST_CFLAGS)
LOCAL_CFLAGS  += -D_ANDROID_

LOCAL_C_INCLUDES := $(TEST_C_INCLUDES)
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
endif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/module/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/sensor/module/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../mm-camerasdk/sensor/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/actuator/module/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/chromatix/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/csid/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/csiphy/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/sensors/eeprom/module/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/core/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/hw/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/hw/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/hw/axi/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/hw/pix/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/hw/pix/pix40/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/hw/pix/modules/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/hw/pix/modules/src/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/isp/hw/pix/modules/scaler/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/iface/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/iface/src/

LOCAL_HEADER_LIBRARIES := libutils_headers

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_SRC_FILES:= test_sensor.c test_isp.c

LOCAL_PROPRIETARY_MODULE := true
LOCAL_SHARED_LIBRARIES:= libcutils liblog libmmcamera2_mct \
                                   libmmcamera2_isp_modules \
                                   libmmcamera2_sensor_modules \
                                   libmmcamera_dbg
LOCAL_MODULE:= test_sensor
LOCAL_MODULE_TAGS := optional

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)

#
# test_isp2
#
include $(CLEAR_VARS)
CHROMATIX_VERSION := $(CHROMATIX_VERSION)
LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := $(TEST_CFLAGS)
LOCAL_CFLAGS  += -D_ANDROID_

LOCAL_MMCAMERA_PATH := $(LOCAL_PATH)/../

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/server-tuning/tuning/

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

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/common/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common

LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_SRC_FILES:= test_isp2.c

LOCAL_PROPRIETARY_MODULE := true
LOCAL_SHARED_LIBRARIES:= libdl libcutils liblog libmmcamera2_mct \
                                   libmmcamera2_isp_modules \
                                   libmmcamera_dbg
LOCAL_MODULE:= test_isp2
LOCAL_MODULE_TAGS := optional

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif


include $(BUILD_EXECUTABLE)

#
# test_cpp
#
include $(CLEAR_VARS)

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := $(TEST_CFLAGS)
LOCAL_CFLAGS  += -D_ANDROID_
LOCAL_C_INCLUDES := $(TEST_C_INCLUDES)
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += system/core/libion/include
LOCAL_C_INCLUDES += system/core/libion/kernel-headers/
endif
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
endif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/modules/pproc-new/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../server-imaging/
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_SRC_FILES:= test_cpp.c

LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_SHARED_LIBRARIES:= libcutils liblog libmmcamera2_mct libmmcamera2_pproc_modules libdl libmmcamera_dbg

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE:= test_module_pproc
#LOCAL_CLANG := false
LOCAL_MODULE_TAGS := optional

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)

endif
