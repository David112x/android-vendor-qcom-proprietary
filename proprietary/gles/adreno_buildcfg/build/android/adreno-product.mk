#Adreno Product defs

#MM_GRAPHICS
MM_GRAPHICS += ProfilerPlaybackTools
MM_GRAPHICS += a300_pfp.fw
MM_GRAPHICS += a300_pm4.fw
MM_GRAPHICS += a330_pfp.fw
MM_GRAPHICS += a330_pm4.fw
MM_GRAPHICS += a506_zap.b00
MM_GRAPHICS += a506_zap.b01
MM_GRAPHICS += a506_zap.b02
MM_GRAPHICS += a506_zap.elf
MM_GRAPHICS += a506_zap.mdt
MM_GRAPHICS += a508_zap.b00
MM_GRAPHICS += a508_zap.b01
MM_GRAPHICS += a508_zap.b02
MM_GRAPHICS += a508_zap.elf
MM_GRAPHICS += a508_zap.mdt
MM_GRAPHICS += a512_zap.b00
MM_GRAPHICS += a512_zap.b01
MM_GRAPHICS += a512_zap.b02
MM_GRAPHICS += a512_zap.elf
MM_GRAPHICS += a512_zap.mdt
MM_GRAPHICS += a530_gpmu.fw2
MM_GRAPHICS += a530_pfp.fw
MM_GRAPHICS += a530_pm4.fw
MM_GRAPHICS += a530_zap.b00
MM_GRAPHICS += a530_zap.b01
MM_GRAPHICS += a530_zap.b02
MM_GRAPHICS += a530_zap.elf
MM_GRAPHICS += a530_zap.mdt
MM_GRAPHICS += a530v1_pfp.fw
MM_GRAPHICS += a530v1_pm4.fw
MM_GRAPHICS += a530v2_seq.fw2
MM_GRAPHICS += a530v3_gpmu.fw2
MM_GRAPHICS += a530v3_seq.fw2
MM_GRAPHICS += a540_gpmu.fw2
MM_GRAPHICS += a540_zap.b00
MM_GRAPHICS += a540_zap.b01
MM_GRAPHICS += a540_zap.b02
MM_GRAPHICS += a540_zap.elf
MM_GRAPHICS += a540_zap.mdt
MM_GRAPHICS += a608_rgmu.bin
MM_GRAPHICS += a608_rgmu.bin
MM_GRAPHICS += a608_zap.b00
MM_GRAPHICS += a608_zap.b00
MM_GRAPHICS += a608_zap.b01
MM_GRAPHICS += a608_zap.b01
MM_GRAPHICS += a608_zap.b02
MM_GRAPHICS += a608_zap.b02
MM_GRAPHICS += a608_zap.elf
MM_GRAPHICS += a608_zap.elf
MM_GRAPHICS += a608_zap.mdt
MM_GRAPHICS += a608_zap.mdt
MM_GRAPHICS += a610_zap.b00
MM_GRAPHICS += a610_zap.b01
MM_GRAPHICS += a610_zap.b02
MM_GRAPHICS += a610_zap.elf
MM_GRAPHICS += a610_zap.mdt
MM_GRAPHICS += a612_rgmu.bin
MM_GRAPHICS += a612_zap.b00
MM_GRAPHICS += a612_zap.b01
MM_GRAPHICS += a612_zap.b02
MM_GRAPHICS += a612_zap.elf
MM_GRAPHICS += a612_zap.mdt
MM_GRAPHICS += a615_zap.b00
MM_GRAPHICS += a615_zap.b00
MM_GRAPHICS += a615_zap.b01
MM_GRAPHICS += a615_zap.b01
MM_GRAPHICS += a615_zap.b02
MM_GRAPHICS += a615_zap.b02
MM_GRAPHICS += a615_zap.elf
MM_GRAPHICS += a615_zap.elf
MM_GRAPHICS += a615_zap.mdt
MM_GRAPHICS += a615_zap.mdt
MM_GRAPHICS += a618_gmu.bin
MM_GRAPHICS += a618_gmu.bin
MM_GRAPHICS += a619_gmu.bin
MM_GRAPHICS += a620_zap.b00
MM_GRAPHICS += a620_zap.b01
MM_GRAPHICS += a620_zap.b02
MM_GRAPHICS += a620_zap.elf
MM_GRAPHICS += a620_zap.mdt
MM_GRAPHICS += a630_gmu.bin
MM_GRAPHICS += a630_sqe.fw
MM_GRAPHICS += a630_zap.b00
MM_GRAPHICS += a630_zap.b01
MM_GRAPHICS += a630_zap.b02
MM_GRAPHICS += a630_zap.elf
MM_GRAPHICS += a630_zap.mdt
MM_GRAPHICS += a640_gmu.bin
MM_GRAPHICS += a640_zap.b00
MM_GRAPHICS += a640_zap.b01
MM_GRAPHICS += a640_zap.b02
MM_GRAPHICS += a640_zap.elf
MM_GRAPHICS += a640_zap.mdt
MM_GRAPHICS += a650_gmu.bin
MM_GRAPHICS += a650_sqe.fw
MM_GRAPHICS += a650_zap.b00
MM_GRAPHICS += a650_zap.b01
MM_GRAPHICS += a650_zap.b02
MM_GRAPHICS += a650_zap.elf
MM_GRAPHICS += a650_zap.mdt
MM_GRAPHICS += android.hardware.renderscript@1.0-impl
MM_GRAPHICS += eglSubDriverAndroid
MM_GRAPHICS += gpu_dcvsd
MM_GRAPHICS += libC2D2
MM_GRAPHICS += libCB
MM_GRAPHICS += libEGL_adreno
MM_GRAPHICS += libGLESv1_CM_adreno
MM_GRAPHICS += libGLESv2_adreno
MM_GRAPHICS += libOpenCL
MM_GRAPHICS += libOpenCL_system
MM_GRAPHICS += libQTapGLES
MM_GRAPHICS += libRSDriver_adreno
MM_GRAPHICS += libadreno_app_profiles
MM_GRAPHICS += libadreno_utils
MM_GRAPHICS += libbccQTI
MM_GRAPHICS += libc2d2_a3xx
MM_GRAPHICS += libc2d30
MM_GRAPHICS += libc2d30-a3xx
MM_GRAPHICS += libc2d30-a5xx
MM_GRAPHICS += libc2d30_bltlib
MM_GRAPHICS += libgsl
MM_GRAPHICS += libintrinsics_skel
MM_GRAPHICS += libkcl
MM_GRAPHICS += libkernelmanager
MM_GRAPHICS += libllvm-glnext
MM_GRAPHICS += libllvm-qcom
MM_GRAPHICS += libplayback_adreno
MM_GRAPHICS += libq3dtools_adreno
MM_GRAPHICS += libq3dtools_esx
MM_GRAPHICS += librs_adreno
MM_GRAPHICS += librs_adreno_sha1

#VULKAN
#List of targets where Vulkan is disabled
VULKAN_DISABLED_TARGETS_LIST  := msm8909 msm8909go msm8937_32go

#List of targets where Vulkan feature level is restricted to 0
VULKAN_FEATURE_LEVEL_0_TARGETS_LIST := msm8937 msm8937_32 msm8937_64 sdm660 sdm660_32 sdm660_64 msm8998 msm8998_32 msm8996 msm8953 msm8953_64 msm8953_32

#List of targets where Vulkan is enabled using swiftshader
VULKAN_SWIFTSHADER_TARGETS_LIST  := msm8937 msm8937_32 msm8937_64

ifeq ($(call is-board-platform-in-list,$(VULKAN_DISABLED_TARGETS_LIST)), )
MM_GRAPHICS += vulkan.$(TARGET_BOARD_PLATFORM)
MM_GRAPHICS += vulkan.adreno

# Targets listed in VULKAN_SWIFTSHADER_TARGETS_LIST have compilation support for swiftshader
ifeq ($(call is-board-platform-in-list,$(VULKAN_SWIFTSHADER_TARGETS_LIST)), true)
MM_GRAPHICS += vulkan.pastel
endif

PRODUCT_PROPERTY_OVERRIDES += ro.hardware.vulkan=adreno
PRODUCT_PROPERTY_OVERRIDES += ro.hardware.egl=adreno

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.vulkan.version-1_1.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.version-1_1.xml \
    frameworks/native/data/etc/android.hardware.vulkan.compute-0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.compute-0.xml

# Targets listed in VULKAN_FEATURE_LEVEL_0_TARGETS_LIST supports only vulkan feature level 0.
ifeq ($(call is-board-platform-in-list,$(VULKAN_FEATURE_LEVEL_0_TARGETS_LIST)), true)
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.vulkan.level-0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.level-0.xml
else
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.vulkan.level-1.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.level-1.xml
endif
endif

PRODUCT_PACKAGES += $(MM_GRAPHICS)

# include additional build utilities
-include device/qcom/common/utils.mk

#FEATURE_OPENGLES_EXTENSION_PACK support string config file
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.opengles.aep.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.opengles.aep.xml

# High performance VR feature
ifeq ($(filter $(TARGET_PRODUCT), sdm845 msm8996 msm8998),$(TARGET_PRODUCT))
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.vr.high_performance.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vr.high_performance.xml
endif

# system prop for opengles version
#
# 196608 is decimal for 0x30000 to report version 3
# 196609 is decimal for 0x30001 to report version 3.1
# 196610 is decimal for 0x30002 to report version 3.2
PRODUCT_PROPERTY_OVERRIDES  += \
    ro.opengles.version=196610

GRAPHICS_TESTS := libgpupostprocessing
PRODUCT_PACKAGES += $(GRAPHICS_TESTS)

#VR
ifeq ($(filter $(TARGET_PRODUCT), sdm845 msm8996 msm8998),$(TARGET_PRODUCT))
PRODUCT_PACKAGES += android.hardware.vr@1.0-impl \
                    android.hardware.vr@1.0-service
endif

#Android EGL implementation
PRODUCT_PACKAGES += libGLES_android

#ANGLE
ANGLE := libangle

PRODUCT_PACKAGES += $(ANGLE)
