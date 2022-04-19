# wrapper header location
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/external

# Add Enable extension, and specify prebuilt so name
#LOCAL_CFLAGS += -D_AWB_EXTENSION_
LOCAL_CFLAGS += -D_AWB_EXT_ALGO_FNAME='"dummy.so"'

