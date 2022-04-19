#======================================================================
#makefile for libmmcamera2_isp_modules.so form mm-camera2
#======================================================================
ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(call is-board-platform-in-list,msm8994 msm8992 apq8084 msm8974 msm8226 msm8916 msm8952 msm8937 msm8996 msm8953 sdm660 msm8998 apq8098_latv $(TRINKET)),true)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS :=  -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags)

LOCAL_CFLAGS  += -Werror

LOCAL_MMCAMERA_PATH := $(LOCAL_PATH)/../../../../

ifeq ($(OEM_CHROMATIX), true)
  LOCAL_C_INCLUDES += $(LOCAL_EXTEN_ISP_INCLUDES)
  LOCAL_CFLAGS += -DOVERRIDE_FUNC=1
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)
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
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/debug/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/debug/fdleak

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/includes/
LOCAL_C_INCLUDES += $(LOCAL_CHROMATIX_PATH)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camera-lib/tintless/be/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/sensor/includes/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/common/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/module_algo/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/module_algo/la_algo/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/module_algo/ltm_algo/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/module_algo/gtm_algo/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/module_algo/tintless_algo/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common

LOCAL_HEADER_LIBRARIES := libutils_headers

ifeq ($(call is-board-platform-in-list,msm8994),true)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/isp46
else ifeq ($(call is-board-platform-in-list,sdm660 msm8998 apq8098_latv $(TRINKET)),true)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/isp48
else ifeq ($(call is-board-platform-in-list,msm8996),true)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/isp47
else ifeq ($(call is-board-platform-in-list,apq8084 msm8992),true)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/isp44
else ifeq ($(call is-board-platform-in-list,msm8916 msm8974),true)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/isp40
else ifeq ($(call is-board-platform-in-list,msm8952),true)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/isp41
else ifeq ($(call is-board-platform-in-list,msm8937 msm8953),true)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/isp42
endif


LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_SRC_FILES        := isp_module.c \
                          isp_port.c \
                          isp_handler.c \
                          isp_resource.c \
                          isp_trigger_thread.c \
                          isp_hw_update_thread.c \
                          isp_hw_update_util.c \
                          isp_parser_thread.c \
                          isp48/isp_pipeline48.c \
                          isp48/isp_pipeline48_parser.c \
                          isp47/isp_pipeline47.c \
                          isp47/isp_pipeline47_parser.c \
                          isp46/isp_pipeline46.c \
                          isp46/isp_pipeline46_parser.c \
                          isp44/isp_pipeline44.c \
                          isp40/isp_pipeline40.c \
                          isp40/isp_pipeline40_parser.c \
                          isp41/isp_pipeline41.c \
                          isp41/isp_pipeline41_parser.c \
                          isp42/isp_pipeline42.c \
                          isp42/isp_pipeline42_parser.c \
                          isp32/isp_pipeline32.c \
                          isp44/isp_pipeline44_parser.c \
                          module_algo/isp_algo.c\
                          module_algo/la_algo/la40_algo.c\
                          module_algo/ltm_algo/ltm44_algo.c\
                          module_algo/ltm_algo/autoltm.c \
                          module_algo/gtm_algo/gtm46_algo.c\
                          module_algo/tintless_algo/tintless40_algo.c\
                          isp_util.c\
                          isp_stats_buf_mgr.c

LOCAL_MODULE           := libmmcamera2_isp_modules
include $(SDCLANG_COMMON_DEFS)
LOCAL_SHARED_LIBRARIES := libdl libcutils libmmcamera2_mct libmmcamera_dbg
LOCAL_LDFLAGS += -Wl,--wrap=malloc -Wl,--wrap=free -Wl,--wrap=realloc -Wl,--wrap=calloc
LOCAL_LDFLAGS += -Wl,--wrap=open -Wl,--wrap=close -Wl,--wrap=socket -Wl,--wrap=pipe -Wl,--wrap=mmap

#ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog
#endif

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)

endif # is-vendor-board-platform,QCOM
endif
