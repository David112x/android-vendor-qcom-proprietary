LOCAL_PATH := $(call my-dir)

daemon_src_files := main.cpp
daemon_src_files += settingsD.cpp
daemon_src_files += file_mapper.cpp

console_src_files := console_main.cpp

common_src_files :=
common_src_files += MainLoop.cpp
common_src_files += commands/Command.cpp
common_src_files += commands/CommandSource.cpp
common_src_files += commands/ConsoleSource.cpp
common_src_files += Logger.cpp
common_src_files += AndroidLogger.cpp
#common_src_files += variant.cpp
common_src_files += commands/commands.proto
common_src_files += commands/ProtoSource.cpp

target_src_files :=
target_daemon_src_files :=

## Build header library
include $(CLEAR_VARS)
LOCAL_MODULE               := libril-qc-settingsd-headers
LOCAL_VENDOR_MODULE        := true
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)/commands
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE               := libril-qc-settingsd-headers
LOCAL_VENDOR_MODULE        := true
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)/commands
LOCAL_IS_HOST_MODULE       := true
include $(BUILD_HEADER_LIBRARY)

##
## Build shared library
##
include $(CLEAR_VARS)

LOCAL_PROTOC_OPTIMIZE_TYPE := full
LOCAL_CFLAGS               += -Wall -Werror -Wno-error=unused-parameter -DTAG=\"libsettings\"
LOCAL_CXXFLAGS             += -std=c++11
LOCAL_SRC_FILES            += $(common_src_files) $(target_src_files) DiagLogger.cpp

LOCAL_MODULE               := libsettings
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS          := optional
LOCAL_C_INCLUDES           += $(LOCAL_PATH) $(LOCAL_PATH)/commands
LOCAL_C_INCLUDES           += $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES           += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_SHARED_LIBRARIES     += liblog
LOCAL_SHARED_LIBRARIES     += libcutils
LOCAL_SHARED_LIBRARIES     += libc++
LOCAL_SHARED_LIBRARIES     += libdiag
LOCAL_HEADER_LIBRARIES     += libril-qc-settingsd-headers
#LOCAL_SHARED_LIBRARIES     += libprotobuf-cpp-lite
#LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_VENDOR)/CarrierConfig/rild.prop
include $(BUILD_SHARED_LIBRARY)


##
## Build host shared library
##
include $(CLEAR_VARS)

LOCAL_PROTOC_OPTIMIZE_TYPE := full
LOCAL_CFLAGS               += -Wall -Werror -Wno-error=unused-parameter -DTAG=\"libsettings\" -DDEBUG_LOGGER_STDERR
LOCAL_CXXFLAGS             += -std=c++11
LOCAL_SRC_FILES            += $(common_src_files)

LOCAL_MODULE               := libsettings
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS          := optional
LOCAL_C_INCLUDES           += $(LOCAL_PATH) $(LOCAL_PATH)/commands
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_SHARED_LIBRARIES     += liblog
LOCAL_SHARED_LIBRARIES     += liblog
LOCAL_SHARED_LIBRARIES     += libcutils
LOCAL_SHARED_LIBRARIES     += libc++

## TODO: uncomment these local copy headers once below error is fixed in build system.
##"libsettings: LOCAL_COPY_HEADERS may not be used with host modules"
##LOCAL_COPY_HEADERS += Logger.h
##LOCAL_COPY_HEADERS += AndroidLogger.h
##LOCAL_COPY_HEADERS += MainLoop.h
##LOCAL_COPY_HEADERS += commands/Command.h
##LOCAL_COPY_HEADERS += commands/ClientSocket.h
##LOCAL_COPY_HEADERS += commands/ServiceSocket.h
##LOCAL_COPY_HEADERS += commands/ProtoSource.h
##LOCAL_COPY_HEADERS += commands/ConsoleSource.h
##LOCAL_COPY_HEADERS += commands/CommandSource.h
##LOCAL_COPY_HEADERS += commands/CommandReader.h
##LOCAL_COPY_HEADERS += commands/EndpointInfo.h


include $(BUILD_HOST_SHARED_LIBRARY)

##
## Build target executable
##

include $(CLEAR_VARS)

LOCAL_PROTOC_OPTIMIZE_TYPE := full
LOCAL_CFLAGS += -Wall -Werror -Wno-error=unused-parameter -DTAG=\"settingsd\"
LOCAL_CXXFLAGS += -std=c++11
LOCAL_SRC_FILES += $(daemon_src_files) $(target_daemon_src_files)

LOCAL_MODULE := settingsd
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES += $(LOCAL_PATH) $(LOCAL_PATH)/commands
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_SHARED_LIBRARIES += libsettings
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libc++
LOCAL_SHARED_LIBRARIES += libprotobuf-cpp-full

include $(BUILD_EXECUTABLE)

##
## Build host executable
##
include $(CLEAR_VARS)

LOCAL_CFLAGS += -Wall -Werror -Wno-error=unused-parameter -DTAG=\"settingsd\"
LOCAL_CXXFLAGS += -std=c++11
LOCAL_SRC_FILES += ${daemon_src_files}
LOCAL_SRC_FILES += properties.c


LOCAL_MODULE := settingsd
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES += $(LOCAL_PATH) $(LOCAL_PATH)/commands
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_SHARED_LIBRARIES += libsettings
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libc++

include $(BUILD_HOST_EXECUTABLE)

##
## Build target executable
##

include $(CLEAR_VARS)

LOCAL_CFLAGS += -Wall -Werror -Wno-error=unused-parameter -DTAG=\"settingscli\"
LOCAL_CXXFLAGS += -std=c++11
LOCAL_SRC_FILES += $(console_src_files)

LOCAL_MODULE := settingscli
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES += $(LOCAL_PATH) $(LOCAL_PATH)/commands
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_SHARED_LIBRARIES += libsettings
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libc++
LOCAL_SHARED_LIBRARIES += libprotobuf-cpp-full

include $(BUILD_EXECUTABLE)

##
## Build host executable
##
include $(CLEAR_VARS)

LOCAL_CFLAGS += -Wall -Werror -Wno-error=unused-parameter -DTAG=\"settingscli\"
LOCAL_CXXFLAGS += -std=c++11
LOCAL_SRC_FILES += ${console_src_files}
LOCAL_SRC_FILES += properties.c


LOCAL_MODULE := settingscli
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES += $(LOCAL_PATH) $(LOCAL_PATH)/commands
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_SHARED_LIBRARIES += libsettings
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libc++
LOCAL_SHARED_LIBRARIES += libprotobuf-cpp-full

include $(BUILD_HOST_EXECUTABLE)

