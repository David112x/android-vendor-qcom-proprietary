LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE:= false

# This variable determines the OpenGL ES API version to use:
# If set to true, OpenGL ES 1.1 is used, otherwise OpenGL ES 2.0.

USE_OPENGL_ES_1_1 := false

# Set OpenGL ES version-specific settings.

ifeq ($(USE_OPENGL_ES_1_1), true)
    OPENGLES_LIB  := libGLESv1_CM
    OPENGLES_DEF  := -DUSE_OPENGL_ES_1_1
else
    OPENGLES_LIB  := libGLESv2
    OPENGLES_DEF  := -DUSE_OPENGL_ES_2_0
endif

# An optional set of compiler flags that will be passed when building
# C ***AND*** C++ source files.
#
# NOTE: flag "-Wno-write-strings" removes warning about deprecated conversion
#       from string constant to ‘char*’

#LOCAL_CFLAGS := -std=gnu++0x -Wno-write-strings $(OPENGLES_DEF)
LOCAL_CFLAGS := -std=gnu++11 -Wno-write-strings $(OPENGLES_DEF)

# The list of additional linker flags to be used when building your
# module. This is useful to pass the name of specific system libraries
# with the "-l" prefix.

LOCAL_SHARED_LIBRARIES += liblog \
                          $(OPENGLES_LIB) \
                          libdl \
                          vendor.qti.hardware.scve.objecttracker@1.0 \
                          libhardware libhidlbase libhidltransport libhwbinder

LOCAL_MODULE    := libfastcvsample
LOCAL_CFLAGS    += -Werror
LOCAL_SRC_FILES := \
    FastCVSample.cpp \
    FPSCounter.cpp \
    CameraRendererRGB565GL2.cpp \
    CameraUtil.cpp \
    rpcmem_android.c \
    FastCVSampleRenderer.cpp

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_STATIC_JAVA_LIBRARIES :=
LOCAL_JNI_SHARED_LIBRARIES := libfastcvsample
LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_PACKAGE_NAME := FastCVSample

include $(BUILD_PACKAGE)

