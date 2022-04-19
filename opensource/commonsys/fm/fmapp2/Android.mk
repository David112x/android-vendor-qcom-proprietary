ifeq ($(BOARD_HAVE_QCOM_FM),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := src/com/caf/fmradio/CommaSeparatedFreqFileReader.java
LOCAL_SRC_FILES += src/com/caf/fmradio/FMAdapterApp.java
LOCAL_SRC_FILES += src/com/caf/fmradio/FMMediaButtonIntentReceiver.java
LOCAL_SRC_FILES += src/com/caf/fmradio/FMRadio.java
LOCAL_SRC_FILES += src/com/caf/fmradio/FMRadioService.java
LOCAL_SRC_FILES += src/com/caf/fmradio/FmSharedPreferences.java
LOCAL_SRC_FILES += src/com/caf/fmradio/FMStats.java
LOCAL_SRC_FILES += src/com/caf/fmradio/FmTags.java
LOCAL_SRC_FILES += src/com/caf/fmradio/GetNextFreqInterface.java
LOCAL_SRC_FILES += src/com/caf/fmradio/HorizontalNumberPicker.java
LOCAL_SRC_FILES += src/com/caf/fmradio/PresetList.java
LOCAL_SRC_FILES += src/com/caf/fmradio/PresetStation.java
LOCAL_SRC_FILES += src/com/caf/fmradio/Settings.java
LOCAL_SRC_FILES += src/com/caf/fmradio/StationListActivity.java
LOCAL_SRC_FILES += src/com/caf/fmradio/IFMRadioService.aidl
LOCAL_SRC_FILES += src/com/caf/fmradio/IFMRadioServiceCallbacks.aidl
LOCAL_SRC_FILES += src/com/caf/fmradio/IFMTransmitterServiceCallbacks.aidl

ifeq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) >= 11 ))" )))
LOCAL_SRC_FILES +=  $(call all-java-files-under, src/com/caf/hc_utils)
else
LOCAL_SRC_FILES +=  $(call all-java-files-under, src/com/caf/utils)
endif
LOCAL_PACKAGE_NAME := FM2
LOCAL_CERTIFICATE := platform
LOCAL_JNI_SHARED_LIBRARIES := libqcomfm_jni
LOCAL_JAVA_LIBRARIES := qcom.fmradio
LOCAL_PRIVATE_PLATFORM_APIS := true
include $(BUILD_PACKAGE)

endif
