ifeq ($(TARGET_USES_QMAA),true)
KMGK_USE_QTI_SERVICE := true
ENABLE_KM_4_0 := true
ifneq ($(TARGET_USES_QMAA_OVERRIDE_KMGK),true)
KMGK_USE_QTI_STUBS := true
endif #TARGET_USES_QMAA_OVERRIDE_KMGK
endif #TARGET_USES_QMAA

# kona/sm8250 specific defines
ifeq ($(TARGET_BOARD_PLATFORM),kona)
KMGK_USE_QTI_SERVICE := true
ENABLE_KM_4_0 := true
ENABLE_STRONGBOX_KM := true
ADOPTABLE_STORAGE_SUPPORTED := true
ENABLE_AUTH_SECRET := true
endif

# lito specific defines
ifeq ($(TARGET_BOARD_PLATFORM),lito)
KMGK_USE_QTI_SERVICE := true
ENABLE_KM_4_0 := true
endif

# atoll specific defines
ifeq ($(TARGET_BOARD_PLATFORM),atoll)
KMGK_USE_QTI_SERVICE := true
ENABLE_KM_4_0 := true
endif

# Bengal specific defines
ifeq ($(TARGET_BOARD_PLATFORM),bengal)
KMGK_USE_QTI_SERVICE := true
ENABLE_KM_4_0 := true
endif

# msmnile/sm8150 specific defines
ifeq ($(TARGET_BOARD_PLATFORM),msmnile)
KMGK_USE_QTI_SERVICE := true
ENABLE_KM_4_0 := true
ifneq ($(TARGET_HIBERNATION_ENABLE), true)
ENABLE_STRONGBOX_KM := true
else
ENABLE_STRONGBOX_KM := false
endif #TARGET_HIBERNATION_ENABLE
endif

# sdmshrike specific defines
ifeq ($(TARGET_BOARD_PLATFORM),sdmshrike)
KMGK_USE_QTI_SERVICE := true
ENABLE_KM_4_0 := true
ENABLE_STRONGBOX_KM := true
endif

# sdm845 specific defines
ifeq ($(TARGET_BOARD_PLATFORM),sdm845)
KMGK_USE_QTI_SERVICE := true
ENABLE_KM_4_0 := true
endif

# trinket specific defines
ifeq ($(TARGET_BOARD_PLATFORM),$(TRINKET))
KMGK_USE_QTI_SERVICE := true
ENABLE_KM_4_0 := true
endif

# talos specific defines
ifeq ($(TARGET_BOARD_PLATFORM),$(MSMSTEPPE))
KMGK_USE_QTI_SERVICE := true
ENABLE_KM_4_0 := true
endif

# sdm710 specific defines
ifeq ($(TARGET_BOARD_PLATFORM),sdm710)
KMGK_USE_QTI_SERVICE := true
ENABLE_KM_4_0 := true
endif

# qcs605 specific defines
ifeq ($(TARGET_BOARD_PLATFORM),qcs605)
KMGK_USE_QTI_SERVICE := true
ENABLE_KM_4_0 := true
endif

# sdm660 specific defines
ifeq ($(TARGET_BOARD_PLATFORM),sdm660)
ifeq ($(ENABLE_VENDOR_IMAGE),true)
KMGK_USE_QTI_SERVICE := true
endif
ENABLE_KM_4_0 := true
endif

# msm8998 specific defines
ifeq ($(TARGET_BOARD_PLATFORM),msm8998)
ifeq ($(ENABLE_VENDOR_IMAGE),true)
KMGK_USE_QTI_SERVICE := true
endif
ENABLE_KM_4_0 := true
endif

# msm8996 specific defines
ifeq ($(TARGET_BOARD_PLATFORM),msm8996)
ifeq ($(ENABLE_VENDOR_IMAGE),true)
KMGK_USE_QTI_SERVICE := true
endif
ifeq ($(strip $(TARGET_KERNEL_VERSION)), 4.4)
ENABLE_KM_4_0 := true
endif
endif

# msm8937 specific defines
ifeq ($(TARGET_BOARD_PLATFORM),msm8937)
ifeq ($(ENABLE_VENDOR_IMAGE),true)
KMGK_USE_QTI_SERVICE := true
endif
ifeq ($(strip $(TARGET_KERNEL_VERSION)), 4.9)
ENABLE_KM_4_0 := true
endif
endif

# msm8953 specific defines
ifeq ($(TARGET_BOARD_PLATFORM),msm8953)
ifeq ($(ENABLE_VENDOR_IMAGE),true)
KMGK_USE_QTI_SERVICE := true
endif
ifeq ($(strip $(TARGET_KERNEL_VERSION)), 4.9)
ENABLE_KM_4_0 := true
endif
endif

# msm8909 specific defines
ifeq ($(TARGET_BOARD_PLATFORM),msm8909)
ifeq ($(ENABLE_VENDOR_IMAGE),true)
KMGK_USE_QTI_SERVICE := true
endif
endif
