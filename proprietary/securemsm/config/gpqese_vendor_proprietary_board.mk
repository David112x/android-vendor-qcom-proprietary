#BENGAL specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),bengal)
TARGET_USES_GPQESE := true
endif

#RENNELL specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),atoll)
TARGET_USES_GPQESE := true
endif

#SAIPAN specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),lito)
TARGET_USES_GPQESE := true
endif

ifeq ($(TARGET_BOARD_PLATFORM),bengal)
TARGET_USES_GPQESE := true
endif

#KONA specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),kona)
TARGET_USES_GPQESE := true
endif

#HANA specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),msmnile)
TARGET_USES_GPQESE := true
endif

#POIPU specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),sdmshrike)
TARGET_USES_GPQESE := true
endif

#NAPALI specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),sdm845)
TARGET_USES_GPQESE := true
endif

#WARLOCK specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),sdm710)
TARGET_USES_GPQESE := true
endif

#WARLOCK specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),qcs605)
TARGET_USES_GPQESE := true
endif

#TALOS/MOOREA specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),$(MSMSTEPPE))
TARGET_USES_GPQESE := true
endif

#NICOBAR specific build flag
ifeq ($(TARGET_BOARD_PLATFORM),$(TRINKET))
TARGET_USES_GPQESE := true
endif
