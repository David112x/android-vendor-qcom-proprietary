LOCAL_PATH:= $(call my-dir)

ifeq ($(call is-board-platform-in-list, kona msmnile sm6150 sdm845 trinket lito atoll ),true)
    include $(CLEAR_VARS)

    LOCAL_HEADER_LIBRARIES := libcutils_headers

    LOCAL_SRC_FILES := \
            io-prefetch/dblayer.cpp \
            io-prefetch/file_playback.cpp \
            io-prefetch/list_capture.cpp \
            io-prefetch/maintain_db.cpp \
            io-prefetch/uba.cpp \
            io-p.cpp

    LOCAL_SHARED_LIBRARIES := \
            libcutils \
            liblog \
            libutils \
            libxml2 \
            libqti-util \
            liblog \
            libhidlbase \
            libhidltransport \
            vendor.qti.hardware.iop@2.0 \
            libbase \
            libqti-iopd-client \
            libqti-perfd-client \
            libsqlite

    LOCAL_CFLAGS += \
            -DSERVER \
            -DCLIENT \
            -DIOPD \
            -g0 \
            -Wall \
            -Wno-unused-variable \
            -Wno-unused-parameter \
            -Wno-unused-function \
            -DVENDOR_DIR=\"/vendor/etc\" \

    LOCAL_C_INCLUDES := \
            vendor/qcom/proprietary/commonsys-intf/android-perf/perf-util \
            vendor/qcom/proprietary/commonsys-intf/android-perf/mp-ctl \
            vendor/qcom/proprietary/android-perf/mp-ctl \
            external/sqlite/dist

    LOCAL_MODULE := libqti-iopd
    LOCAL_PROPRIETARY_MODULE := true

    include $(BUILD_SHARED_LIBRARY)
endif
