ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(call is-board-platform-in-list,msm8084),true)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
#
# unit test executables
#

# Global flag and include definitions
TEST_CFLAGS :=   -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags) \
  -DMSM_CAMERA_BIONIC

TEST_C_INCLUDES:= $(LOCAL_PATH)
TEST_C_INCLUDES+= $(LOCAL_PATH)/../../../../../includes/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../../../../../media-controller/mct/bus/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../../../../../media-controller/mct/controller/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../../../../../media-controller/mct/object/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../../../../../media-controller/includes/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../../../../../media-controller/mct/tools/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../../../../../media-controller/mct/event/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../../../../../media-controller/mct/pipeline/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../../../../../media-controller/mct/stream/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../../../../../media-controller/mct/module/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../../../../../media-controller/mct/port/
TEST_C_INCLUDES+= \
 $(LOCAL_PATH)/../../../../../../../../../../hardware/qcom/camera/QCamera2/stack/common

#
# test_bet_8084
#
include $(CLEAR_VARS)
CHROMATIX_VERSION := 0304
LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := $(TEST_CFLAGS)
LOCAL_CFLAGS  += -D_ANDROID_

LOCAL_MMCAMERA_PATH := $(LOCAL_PATH)/../../../../../

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/server-tuning/tuning/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/tools/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/bus/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/controller/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/event/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/object/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/pipeline/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/port/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/stream/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/includes/
LOCAL_C_INCLUDES += $(LOCAL_CHROMATIX_PATH)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp/test/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/chromatix-wrapper/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/isp44
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/linearization/linearization40/include40
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/bpc/bpc44/include44
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/bcc/bcc44/include44
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/chroma_enhan/chroma_enhan40/include40
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/chroma_suppress/chroma_suppress40/include40
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/mce/mce40/include40
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/rolloff/mesh_rolloff44/include44
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/gamma/gamma44/include44
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/demosaic/demosaic44/include44
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/abf/abf44/include44
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/gamma/gamma44/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/clf/clf44/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/sce/sce40/include40/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/ltm/ltm44/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/ltm/ltm44/include44
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/common/
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common

LOCAL_SRC_FILES:= vfe_test_vector.c \
               ../test_bet.c \
               ../test_linearization44.c \
               ../test_bcc44.c \
               ../test_bpc44.c \
               ../test_abf44.c \
               ../test_chroma_enhan44.c \
               ../test_colorcorrect44.c \
               ../test_demosaic44.c \
               ../test_luma_adaptation44.c \
               ../test_gamma44.c \
               ../test_clf44.c \
               ../test_chroma_suppress44.c \
               ../test_mce44.c \
               ../test_mesh_rolloff44.c \
               ../test_wb44.c \
               ../test_sce44.c \
               ../test_ltm44.c \

LOCAL_SHARED_LIBRARIES:= libdl libcutils libmmcamera2_mct libmmcamera2_isp_modules libmmcamera_dbg
LOCAL_MODULE:= test_bet_8084
LOCAL_MODULE_TAGS := optional

# copy data files
$(shell cp -R $(LOCAL_PATH)/data/gain $(TARGET_OUT_ETC))
$(shell cp -R $(LOCAL_PATH)/data/lux $(TARGET_OUT_ETC))

include $(BUILD_EXECUTABLE)
#END
endif
endif
