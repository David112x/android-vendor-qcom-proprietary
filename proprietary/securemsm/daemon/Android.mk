ifeq ($(call is-board-platform-in-list,msm8974 msm8960 msm8660 msm8226 msm8610 msm8916 msm8916_32 msm8916_32_k64 msm8916_64 apq8084 msm8939 msm8994 msm8909 msm8996 msm8992 msm8952 msm8937 msm8953 msm8998 apq8098_latv sdm660 sdm845 sdm710 qcs605 sdmshrike msmnile kona $(MSMSTEPPE) lito atoll bengal $(TRINKET)),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                    $(LOCAL_PATH)/../QSEEComAPI \
                    $(LOCAL_PATH)/../tzcommon/inc \
                    $(TARGET_OUT_HEADERS)/common/inc

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils \
        liblog \
        libdl \
        libQSEEComAPI \
        libdrmfs \

LOCAL_MODULE := qseecomd
ifeq ($(PRODUCT_VENDOR_MOVE_ENABLED),true)
LOCAL_PROPRIETARY_MODULE := true
endif

ifeq ($(TARGET_VB_NOT_ENABLED),true)
QSEECOM_CFLAGS += -DHLOS_LOAD_KEYMASTER
endif

#Disable virtualized listeners (rpmb & ssd) for guest OS
ifeq ($(ENABLE_HYP),true)
QSEECOM_CFLAGS += -DLISTENERS_VIRTUALIZED
endif

#Disable Secure UI listener for automotive builds
ifneq ($(TARGET_BOARD_AUTO),true)
QSEECOM_CFLAGS += -DSECURE_UI_LISTENER_ENABLED
endif

LOCAL_SRC_FILES := qseecomd.c
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := $(QSEECOM_CFLAGS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE       := qseecomd.rc
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
ifeq ($(call is-board-platform-in-list,sdm845 sdm710),true)
LOCAL_SRC_FILES    := postfs/$(LOCAL_MODULE)
else
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
endif
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)/init
include $(BUILD_PREBUILT)

ifeq ($(call is-board-platform-in-list,),true)

###############Compile QSEECOMFSD library #################################

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                    $(LOCAL_PATH)/../QSEEComAPI \
                    $(LOCAL_PATH)/../tzcommon/inc \
                    $(TARGET_OUT_HEADERS)/common/inc

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_LDLIBS := -llog
LOCAL_LDLIB +=- landroid
LOCAL_MODULE := qseecomfsd
ifeq ($(TARGET_VB_NOT_ENABLED),true)
QSEECOM_CFLAGS += -DHLOS_LOAD_KEYMASTER
endif
LOCAL_SRC_FILES := qseecomfsd.c
LOCAL_STATIC_LIBRARIES := libdrmMinimalfs \
                          libQSEEComAPIStatic \
                          libc \
                          liblog \
                          libcutils \

LOCAL_MODULE_PATH := $(TARGET_RECOVERY_OUT)
LOCAL_MODULE_TAGS := optional
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_CFLAGS := $(QSEECOM_CFLAGS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qti
include $(BUILD_EXECUTABLE)

##############Compile QSEECOMD_STATIC ####################################


include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                    $(LOCAL_PATH)/../QSEEComAPI \
                    $(LOCAL_PATH)/../tzcommon/inc \
                    $(TARGET_OUT_HEADERS)/common/inc

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_STATIC_LIBRARIES := \
        libc \
        libcutils \
        liblog \
        libssdStatic \
        librpmbStatic \
	libQSEEComAPIStatic

LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE := qseecomd_static
ifeq ($(TARGET_VB_NOT_ENABLED),true)
QSEECOM_CFLAGS += -DHLOS_LOAD_KEYMASTER
endif
LOCAL_SRC_FILES := qseecomd_static.c
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := $(QSEECOM_CFLAGS) -static
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qti
include $(BUILD_EXECUTABLE)

endif


##############GPFS path config file ######################################

include $(CLEAR_VARS)
LOCAL_MODULE := gpfspath_oem_config.xml
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := gpfspath_oem_config.xml
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_ETC)
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

endif # end filter
