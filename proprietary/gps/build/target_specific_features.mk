
#List of supported board platform targets
LOC_BOARD_PLATFORM_LIST := msm8960
LOC_BOARD_PLATFORM_LIST += msm8974
LOC_BOARD_PLATFORM_LIST += msm8226
LOC_BOARD_PLATFORM_LIST += msm8610
LOC_BOARD_PLATFORM_LIST += apq8084
LOC_BOARD_PLATFORM_LIST += msm8916
LOC_BOARD_PLATFORM_LIST += msm8994
LOC_BOARD_PLATFORM_LIST += msm8909
LOC_BOARD_PLATFORM_LIST += msm8952
LOC_BOARD_PLATFORM_LIST += msm8992
LOC_BOARD_PLATFORM_LIST += msm8996
LOC_BOARD_PLATFORM_LIST += msm8937
LOC_BOARD_PLATFORM_LIST += msm8953
LOC_BOARD_PLATFORM_LIST += msm8998
LOC_BOARD_PLATFORM_LIST += apq8098_latv
LOC_BOARD_PLATFORM_LIST += sdm660
LOC_BOARD_PLATFORM_LIST += sdm845
LOC_BOARD_PLATFORM_LIST += msmpeafowl
LOC_BOARD_PLATFORM_LIST += sdm710
LOC_BOARD_PLATFORM_LIST += qcs605
LOC_BOARD_PLATFORM_LIST += msmnile
LOC_BOARD_PLATFORM_LIST += sdmshrike
LOC_BOARD_PLATFORM_LIST += $(MSMSTEPPE)
LOC_BOARD_PLATFORM_LIST += $(TRINKET)
LOC_BOARD_PLATFORM_LIST += kona
LOC_BOARD_PLATFORM_LIST += atoll
LOC_BOARD_PLATFORM_LIST += lito
LOC_BOARD_PLATFORM_LIST += bengal

GNSS_CFLAGS := \
    -Werror \
    -Wno-error=unused-parameter \
    -Wno-error=macro-redefined \
    -Wno-error=reorder \
    -Wno-error=missing-braces \
    -Wno-error=self-assign \
    -Wno-error=enum-conversion \
    -Wno-error=logical-op-parentheses \
    -Wno-error=null-arithmetic \
    -Wno-error=null-conversion \
    -Wno-error=parentheses-equality \
    -Wno-error=undefined-bool-conversion \
    -Wno-error=tautological-compare \
    -Wno-error=switch \
    -Wno-error=date-time

ifneq (,$(filter $(LOC_BOARD_PLATFORM_LIST),$(TARGET_BOARD_PLATFORM)))
#default defines for all targets
GPS_LOC_API_V02_ENABLED := true
FEATURE_GNSS_BIT_API := true
GPS_FEATURES := \
        -D FEATURE_COARSE_POS_TIMESTAMP_AGE \
        -D FEATURE_LOC_MW_EFS_ACCESS \
        -D FEATURE_RPC_CHAR_ARRAY \
        -D FEATURE_GNSS_BIT_API

FEATURE_LOCTECH_SLIM_DSPS := false
FEATURE_LOCTECH_SLIM_VNS := false

ifeq ($(TARGET_BOARD_TYPE),auto)  #automotive target, enable VNS
FEATURE_LOCTECH_SLIM_VNS := true
endif

ifeq ($(TARGET_BOARD_PLATFORM),kona) # kona specific defines
FEATURE_MODEM_POWER_VOTE := false
endif

else #no supported target found
LOC_UNSUPPORTED_TARGET := true
endif #is-board-platform-in-list

ifeq ($(TARGET_DEVICE),apq8026_lw)
LW_FEATURE_SET := true
endif

ifeq ($(TARGET_SUPPORTS_ANDROID_WEAR),true)
LW_FEATURE_SET := true
FEATURE_PDMAPPER := false
endif

# Activate the following two lines for regression testing
#GNSS_SANITIZE := address cfi alignment bounds null unreachable integer
#GNSS_SANITIZE_DIAG := address cfi alignment bounds null unreachable integer
