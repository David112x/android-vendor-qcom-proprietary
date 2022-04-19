LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO   := common/inc
LOCAL_COPY_HEADERS      := ../inc/armasm.h
LOCAL_COPY_HEADERS      += ../inc/comdef.h
LOCAL_COPY_HEADERS      += ../inc/common_log.h
LOCAL_COPY_HEADERS      += ../inc/customer.h
LOCAL_COPY_HEADERS      += ../inc/msm_ipc.h
LOCAL_COPY_HEADERS      += ../inc/rex.h
LOCAL_COPY_HEADERS      += ../inc/stringl.h
LOCAL_COPY_HEADERS      += ../inc/target.h
LOCAL_COPY_HEADERS      += ../inc/qsocket.h
LOCAL_COPY_HEADERS      += ../inc/qsocket_ipcr.h
LOCAL_VENDOR_MODULE:=true
include build/core/copy_headers.mk

#----------------------------------------------------------------------
# Copy public.libraries.txt to /system/vendor/etc
#----------------------------------------------------------------------
$(shell mkdir -p $(TARGET_OUT_VENDOR_ETC))
$(shell cat device/qcom/$(TARGET_PRODUCT)/public.libraries.vendor.txt > $(TARGET_OUT_VENDOR_ETC)/public.libraries.txt)
ifeq ($(TARGET_SUPPORTS_ANDROID_WEAR),true)
$(shell cat $(QC_PROP_ROOT)/common/config/public.libraries.wearable.txt >> $(TARGET_OUT_VENDOR_ETC)/public.libraries.txt)
else
$(shell cat $(QC_PROP_ROOT)/common/config/public.libraries.txt >> $(TARGET_OUT_VENDOR_ETC)/public.libraries.txt)
endif
#----------------------------------------------------------------------

#----------------------------------------------------------------------
# Copy public.libraries-qti.txt to /system/etc
#----------------------------------------------------------------------
$(shell cat device/qcom/$(TARGET_PRODUCT)/public.libraries-qti.txt > $(TARGET_OUT_ETC)/public.libraries-qti.txt)
$(shell cat $(QC_PROP_ROOT)/common/config/public.libraries-qti.txt >> $(TARGET_OUT_ETC)/public.libraries-qti.txt)
#----------------------------------------------------------------------

ifeq ($(ENABLE_EXTRA_VENDOR_LIBS),true)
ifeq ($(BOARD_VNDK_VERSION),)

EXTRA_VENDOR_LIBRARIES := \
    vendor.qti.hardware.slmadapter@1.0 \
    com.qualcomm.qti.ant@1.0 \
    com.qualcomm.qti.bluetooth_audio@1.0 \
    com.qualcomm.qti.dpm.api@1.0 \
    com.qualcomm.qti.imscmservice@1.0 \
    com.qualcomm.qti.imscmservice@2.0 \
    com.qualcomm.qti.imscmservice@2.1 \
    com.qualcomm.qti.imscmservice@2.2 \
    com.qualcomm.qti.uceservice@2.0 \
    com.qualcomm.qti.uceservice@2.1 \
    vendor.qti.hardware.bluetooth_audio@2.0 \
    vendor.display.color@1.0 \
    vendor.display.config@1.0 \
    vendor.display.postproc@1.0 \
    vendor.qti.data.factory@1.0 \
    vendor.qti.data.factory@2.0 \
    vendor.qti.data.factory@2.1 \
    vendor.qti.esepowermanager@1.0 \
    vendor.qti.gnss@1.0 \
    vendor.qti.hardware.alarm@1.0 \
    vendor.qti.hardware.btconfigstore@1.0 \
    vendor.qti.hardware.bluetooth_sar@1.0 \
    vendor.qti.hardware.bluetooth_dun@1.0 \
    vendor.qti.hardware.camera.device@1.0 \
    vendor.qti.hardware.cryptfshw@1.0 \
    vendor.qti.hardware.data.dynamicdds@1.0 \
    vendor.qti.hardware.cacert@1.0 \
    vendor.qti.hardware.capabilityconfigstore@1.0 \
    vendor.qti.hardware.data.cne.internal.api@1.0 \
    vendor.qti.hardware.data.cne.internal.constants@1.0 \
    vendor.qti.hardware.data.cne.internal.server@1.0 \
    vendor.qti.hardware.data.connection@1.0 \
    vendor.qti.hardware.data.connection@1.1 \
    vendor.qti.hardware.data.iwlan@1.0 \
    vendor.qti.hardware.data.latency@1.0 \
    vendor.qti.hardware.data.qmi@1.0 \
    vendor.qti.hardware.factory@1.0 \
    vendor.qti.hardware.factory@1.1 \
    vendor.qti.hardware.fingerprint@1.0 \
    vendor.qti.hardware.fm@1.0 \
    vendor.qti.hardware.iop@1.0 \
    vendor.nxp.hardware.nfc@1.0 \
    vendor.nxp.hardware.nfc@1.1 \
    vendor.qti.hardware.perf@1.0 \
    vendor.qti.hardware.qdutils_disp@1.0 \
    vendor.qti.hardware.qteeconnector@1.0 \
    vendor.qti.hardware.radio.am@1.0 \
    vendor.qti.hardware.radio.ims@1.0 \
    vendor.qti.hardware.radio.lpa@1.0 \
    vendor.qti.hardware.radio.qcrilhook@1.0 \
    vendor.qti.hardware.radio.qtiradio@1.0 \
    vendor.qti.hardware.radio.qtiradio@2.1 \
    vendor.qti.hardware.radio.qtiradio@2.2 \
    vendor.qti.hardware.radio.internal.deviceinfo@1.0 \
    vendor.qti.hardware.radio.uim@1.0 \
    vendor.qti.hardware.radio.uim_remote_client@1.0 \
    vendor.qti.hardware.radio.uim_remote_client@1.1 \
    vendor.qti.hardware.radio.uim_remote_client@1.2 \
    vendor.qti.hardware.radio.uim_remote_server@1.0 \
    vendor.qti.hardware.scve.objecttracker@1.0-adapter-helper \
    vendor.qti.hardware.scve.objecttracker@1.0 \
    vendor.qti.hardware.scve.panorama@1.0-adapter-helper \
    vendor.qti.hardware.scve.panorama@1.0 \
    vendor.qti.hardware.seccam@1.0 \
    vendor.qti.hardware.sigma_miracast@1.0-halimpl \
    vendor.qti.hardware.sigma_miracast@1.0-impl \
    vendor.qti.hardware.sigma_miracast@1.0 \
    vendor.qti.hardware.sensorscalibrate@1.0 \
    vendor.qti.hardware.soter@1.0 \
    vendor.qti.hardware.tui_comm@1.0 \
    vendor.qti.hardware.wifi.hostapd@1.0 \
    vendor.qti.hardware.wifi.hostapd@1.1 \
    vendor.qti.hardware.wifi.supplicant@2.0 \
    vendor.qti.hardware.wifi.supplicant@2.1 \
    vendor.qti.hardware.wigig.netperftuner@1.0 \
    vendor.qti.hardware.wigig.supptunnel@1.0 \
    vendor.qti.ims.callinfo@1.0 \
    vendor.qti.voiceprint@1.0 \
    vendor.display.color@1.1 \
    vendor.display.config@1.1 \
    vendor.qti.gnss@1.1 \
    vendor.qti.hardware.radio.ims@1.1 \
    vendor.qti.hardware.radio.uim@1.1 \
    vendor.qti.hardware.radio.uim@1.2 \
    vendor.qti.hardware.radio.atcmdfwd@1.0 \
    vendor.qti.hardware.vpp@1.1 \
    vendor.display.color@1.2 \
    vendor.display.config@1.2 \
    vendor.qti.gnss@1.2 \
    vendor.qti.hardware.radio.ims@1.2 \
    vendor.qti.hardware.radio.ims@1.3 \
    vendor.qti.hardware.radio.ims@1.4 \
    vendor.qti.hardware.radio.ims@1.5 \
    vendor.qti.hardware.radio.ims@1.6 \
    vendor.qti.gnss@2.0 \
    vendor.qti.hardware.perf@2.0 \
    vendor.qti.hardware.iop@2.0 \
    vendor.qti.hardware.radio.qtiradio@2.0 \
    vendor.qti.hardware.radio.qtiradio@2.3 \
    vendor.qti.gnss@2.1 \
    vendor.qti.gnss@3.0 \
    vendor.display.config@1.3 \
    vendor.display.config@1.4 \
    vendor.display.config@1.5 \
    vendor.display.config@1.6 \
    vendor.display.config@1.7 \
    vendor.display.config@1.8 \
    vendor.display.config@1.9 \
    vendor.qti.ims.rcsconfig@1.0 \
    vendor.qti.ims.rcsconfig@1.1 \
    vendor.qti.data.slm@1.0 \
    vendor.qti.imsrtpservice@2.0 \
    vendor.qti.imsrtpservice@2.1 \
    libvndfwk_detect_jni.qti \
    vendor.qti.latency@2.0 \
    libdrm \
    libeffectsconfig \
    libstagefright_softomx \
    libqti_vndfwk_detect \
    android.hardware.audio.common@5.0-util \
    android.hardware.audio.common@4.0-util \
    android.hardware.audio.effect@5.0 \
    android.hardware.audio.common-util \
    android.hidl.base@1.0

ifeq ($(call is-board-platform-in-list,msm8996),)
EXTRA_VENDOR_LIBRARIES += vendor.qti.hardware.improvetouch.touchcompanion@1.0
endif

define define-extra-vendor-lib
include $$(CLEAR_VARS)
LOCAL_MODULE := $1.extra-vendor-lib-gen
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PREBUILT_MODULE_FILE := $$(call intermediates-dir-for,SHARED_LIBRARIES,$1)/$1.so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := first
LOCAL_MODULE_TAGS := optional
LOCAL_INSTALLED_MODULE_STEM := $1.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_VENDOR_MODULE := true
include $$(BUILD_PREBUILT)

ifneq ($$(TARGET_2ND_ARCH),)
ifneq ($$(TARGET_TRANSLATE_2ND_ARCH),true)
include $$(CLEAR_VARS)
LOCAL_MODULE := $1.extra-vendor-lib-gen
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PREBUILT_MODULE_FILE := $$(call intermediates-dir-for,SHARED_LIBRARIES,$1,,,$(my_2nd_arch_prefix))/$1.so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_TAGS := optional
LOCAL_INSTALLED_MODULE_STEM := $1.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_VENDOR_MODULE := true
include $$(BUILD_PREBUILT)
endif # TARGET_TRANSLATE_2ND_ARCH is not true
endif # TARGET_2ND_ARCH is not empty
endef

$(foreach lib,$(EXTRA_VENDOR_LIBRARIES),\
    $(eval $(call define-extra-vendor-lib,$(lib))))

include $(CLEAR_VARS)
LOCAL_MODULE := vendor-extra-libs
LOCAL_MODULE_TAGS := optional
LOCAL_REQUIRED_MODULES := $(addsuffix .extra-vendor-lib-gen,$(EXTRA_VENDOR_LIBRARIES))
include $(BUILD_PHONY_PACKAGE)
endif
endif
