TARGET_LIST := msm8998 sdm845 sdm710 msmnile $(MSMSTEPPE) $(TRINKET) kona lito atoll bengal lahaina

GEN_IMG_DIR := $(TARGET_OUT_VENDOR)/firmware/
$(info $(shell mkdir -p $(GEN_IMG_DIR)))
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

PILSPLITTER_BASE := vendor/qcom/proprietary/common/scripts
HASH_SEG_ALGO := sha384

ifeq ($(call is-board-platform-in-list,$(TARGET_LIST)),true)

$(info IPA_FWS: Checking $(TARGET_BOARD_PLATFORM) target)

# Firmware hardware addresses and bin files
# If these differ across devices, define them accordingly.
DPS_ADDR := 0x01E5E000
HPS_ADDR := 0x01E5E080
GSI_ADDR := 0x01E08000
HPS_BIN := hps.bin

GEN_IPA_UC := false

# msm8998 IPA FWs configs (IPAv3.1)
ifeq ($(call is-board-platform-in-list,msm8998),true)
$(info IPA_FWS creation for msm8998)
GSI_FW_FILE := fw_mhi_ipa_v3.bin
GSI_MEM_SIZE := 0x4000
SOC_HW_VERSION := 0x30020000
# For 8997 HW version
SOC_VERS := 0x3005
ifeq ("$(wildcard vendor/qcom/proprietary/sectools)","")
SECIMAGE_BASE := vendor/qcom/proprietary/common/scripts/SecImage
else
SECIMAGE_BASE := vendor/qcom/proprietary/sectools
endif
SECIMAGE_CONFIG_FILE := config/integration/secimagev2.xml
endif #msm8998 check

# sdm845 IPA FWs configs (IPAv3.5.1)
ifeq ($(call is-board-platform-in-list,sdm845),true)
$(info IPA_FWS creation for sdm845)
GSI_FW_FILE := fw_mhi_ipa_v3.5.bin
GSI_MEM_SIZE := 0x4000
SOC_HW_VERSION := 0x60000000
SOC_VERS := 0x6001
SECIMAGE_CONFIG_FILE := config/integration/secimagev2.xml
SECIMAGE_BASE := vendor/qcom/proprietary/common/scripts/SecImage
endif #sdm845 check

# sdm710 IPA FWs configs (IPAv3.5.1)
ifeq ($(call is-board-platform-in-list,sdm710),true)
$(info IPA_FWS creation for sdm710)
GSI_FW_FILE := fw_mhi_ipa_v3.5.bin
GSI_MEM_SIZE := 0x4000
SOC_HW_VERSION := 0x60040000
SOC_VERS := 0x6005 0x6009 0x600A
SECIMAGE_CONFIG_FILE := config/integration/secimagev2.xml
SECIMAGE_BASE := vendor/qcom/proprietary/common/scripts/SecImage
endif #sdm710 check

# msmnile IPA FWs configs (IPAv4.1)
ifeq ($(call is-board-platform-in-list,msmnile),true)
$(info IPA_FWS creation for msmnile)
GSI_FW_FILE := fw_mhi_ipa_v4.0.bin
GSI_MEM_SIZE := 0x6000
SOC_HW_VERSION := 0x60030100
SOC_VERS := 0x6003
SECIMAGE_CONFIG_FILE := config/integration/secimagev3.xml
ifeq ("$(wildcard vendor/qcom/proprietary/sectools)","")
SECIMAGE_BASE := vendor/qcom/proprietary/tools/sectools
else
SECIMAGE_BASE := vendor/qcom/proprietary/sectools
endif
GEN_IPA_UC := true
UC_FW_FILE := ipa_uc_v4.1.bin
UC_ADDR := 0x1E60000
UC_MEM_SIZE := 0x10000
endif #msmnile check

# MSMSTEPPE IPA FWs configs (IPAv4.2)
ifeq ($(call is-board-platform-in-list,$(MSMSTEPPE)),true)
$(info IPA_FWS creation for $(MSMSTEPPE))
GSI_FW_FILE := fw_mhi_ipa_v4.2.bin
GSI_MEM_SIZE := 0x4000
SOC_HW_VERSION := 0x60070000
SOC_VERS := "0x6007 0x600c"
SECIMAGE_CONFIG_FILE := config/integration/secimagev3.xml
ifeq ("$(wildcard vendor/qcom/proprietary/sectools)","")
SECIMAGE_BASE := vendor/qcom/proprietary/tools/sectools
else
SECIMAGE_BASE := vendor/qcom/proprietary/sectools
endif
endif #MSMSTEPPE check

# atoll IPA FWs configs (IPAv4.2)
ifeq ($(call is-board-platform-in-list,atoll),true)
$(info IPA_FWS creation for atoll)
GSI_FW_FILE := fw_mhi_ipa_v4.2.bin
GSI_MEM_SIZE := 0x4000
SOC_HW_VERSION := 0x600E0100
SOC_VERS := "0x600E"
SECIMAGE_CONFIG_FILE := config/integration/secimagev3.xml
ifeq ("$(wildcard vendor/qcom/proprietary/sectools)","")
SECIMAGE_BASE := vendor/qcom/proprietary/tools/sectools
else
SECIMAGE_BASE := vendor/qcom/proprietary/sectools
endif
endif #atoll check

# TRINKET IPA FWs configs (IPAv4.2)
ifeq ($(call is-board-platform-in-list,$(TRINKET)),true)
$(info IPA_FWS creation for $(TRINKET))
DPS_ADDR := 0x0585E000
HPS_ADDR := 0x0585E080
GSI_ADDR := 0x05808000
GSI_FW_FILE := fw_mhi_ipa_v4.2.bin
GSI_MEM_SIZE := 0x4000
SOC_HW_VERSION := 0x90010100
SOC_VERS := 0x9001
SECIMAGE_CONFIG_FILE := config/integration/secimagev3.xml
ifeq ("$(wildcard vendor/qcom/proprietary/sectools)","")
SECIMAGE_BASE := vendor/qcom/proprietary/tools/sectools
else
SECIMAGE_BASE := vendor/qcom/proprietary/sectools
endif
endif #TRINKET check

# kona IPA FWs configs (IPAv4.5)
ifeq ($(call is-board-platform-in-list,kona),true)
$(info IPA_FWS creation for kona)
DPS_ADDR := 0x01E81000
HPS_ADDR := 0x01E81080
GSI_ADDR := 0x01E1F000
HPS_BIN := hps_v4.5.bin
GSI_FW_FILE := fw_mhi_ipa_v4.5.bin
GSI_MEM_SIZE := 0x8000
SOC_HW_VERSION := 0x60080200
SOC_VERS := 0x6008
SECIMAGE_CONFIG_FILE := config/integration/secimagev3.xml
ifeq ("$(wildcard vendor/qcom/proprietary/sectools)","")
SECIMAGE_BASE := vendor/qcom/proprietary/tools/sectools
else
SECIMAGE_BASE := vendor/qcom/proprietary/sectools
endif
GEN_IPA_UC := true
UC_FW_FILE := ipa_uc_v4.5.bin
UC_ADDR := 0x1EA0000
UC_MEM_SIZE := 0x10000
endif #kona check

# lito IPA FWs configs (IPAv4.7)
ifeq ($(call is-board-platform-in-list,lito),true)
$(info IPA_FWS creation for lito)
DPS_ADDR := 0x01E81000
HPS_ADDR := 0x01E81080
GSI_ADDR := 0x01E1F000
HPS_BIN := hps_v4.5.bin
GSI_FW_FILE := fw_mhi_ipa_v4.7.bin
GSI_MEM_SIZE := 0x5000
SOC_HW_VERSION := 0x600D0100
SOC_VERS := 0x600D 0x6012
SECIMAGE_CONFIG_FILE := config/integration/secimagev3.xml
ifeq ("$(wildcard vendor/qcom/proprietary/sectools)","")
SECIMAGE_BASE := vendor/qcom/proprietary/tools/sectools
else
SECIMAGE_BASE := vendor/qcom/proprietary/sectools
endif
endif #lito check

# Bengal IPA FWs configs (IPAv4.2)
ifeq ($(call is-board-platform-in-list,bengal),true)
$(info IPA_FWS creation for bengal)
DPS_ADDR := 0x0585E000
HPS_ADDR := 0x0585E080
GSI_ADDR := 0x05808000
HPS_BIN := hps.bin
GSI_FW_FILE := fw_mhi_ipa_v4.2.bin
GSI_MEM_SIZE := 0x4000
SOC_HW_VERSION := 0x90020100
SOC_VERS := 0x9002
SECIMAGE_CONFIG_FILE := config/integration/secimagev3.xml
ifeq ("$(wildcard vendor/qcom/proprietary/sectools)","")
SECIMAGE_BASE := vendor/qcom/proprietary/tools/sectools
else
SECIMAGE_BASE := vendor/qcom/proprietary/sectools
endif
endif #Bengalcheck

# lahaina IPA FWs configs (IPAv4.9)
ifeq ($(call is-board-platform-in-list,lahaina),true)
$(info IPA_FWS creation for lahaina)
DPS_ADDR := 0x01E81000
HPS_ADDR := 0x01E81080
GSI_ADDR := 0x01E1F000
HPS_BIN := hps_v4.5.bin
GSI_FW_FILE := fw_mhi_ipa_v4.9.bin
GSI_MEM_SIZE := 0x6000
SOC_HW_VERSION := 0x600F0100
SOC_VERS := 0x600F
SECIMAGE_CONFIG_FILE := config/integration/secimagev3.xml
ifeq ("$(wildcard vendor/qcom/proprietary/sectools)","")
SECIMAGE_BASE := vendor/qcom/proprietary/tools/sectools
else
SECIMAGE_BASE := vendor/qcom/proprietary/sectools
endif
endif #lahaina check

# Check GSI FW file existence
ifeq ("$(wildcard $(LOCAL_PATH)/$(GSI_FW_FILE))","")
$(info IPA_FWS: $(LOCAL_PATH)/$(GSI_FW_FILE) does not exist!)
else
BINS_GSI := $(LOCAL_PATH)/$(GSI_FW_FILE) $(LOCAL_PATH)/$(HPS_BIN) $(LOCAL_PATH)/dps.bin
TOOLS_GSI := $(LOCAL_PATH)/elf_creator.py $(SECIMAGE_BASE)/sectools_builder.py $(PILSPLITTER_BASE)/pil-splitter.py
OUT_GSI_FW := $(OUT_DIR)/ipa_tmp_gsi

GEN_IMG_GSI := ipa_fws.elf
ifeq ($(USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN),1)
GEN_IMG_GSI_OTHERS := ipa_fws.b00 ipa_fws.b01 ipa_fws.b02 ipa_fws.b03 ipa_fws.b04 ipa_fws.mdt
GEN_IMG_GSI_LAST := ipa_fws.b04
else
GEN_IMG_GSI_OTHERS := ipa_fws.b00 ipa_fws.b01 ipa_fws.b02 ipa_fws.mdt
GEN_IMG_GSI_LAST := ipa_fws.b02
endif
$(info GEN_IMG_GSI_OTHERS=$(GEN_IMG_GSI_OTHERS) GEN_IMG_GSI_LAST=$(GEN_IMG_GSI_LAST))

include $(CLEAR_VARS)
LOCAL_MODULE := ipa_fws
LOCAL_SANITIZE:=integer_overflow
LOCAL_INIT_RC := ipa_fws.rc
LOCAL_MODULE_PATH := $(OUT_GSI_FW)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_REQUIRED_MODULES := $(GEN_IMG_GSI) $(GEN_IMG_GSI_OTHERS)
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE := ipa_fws.b00
LOCAL_SANITIZE:=integer_overflow
LOCAL_MODULE_CLASS := FAKE
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := ipa
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_MODULE_PATH := $(GEN_IMG_DIR)
include $(BUILD_SYSTEM)/base_rules.mk
$(LOCAL_BUILT_MODULE): PRIVATE_FILE := $(LOCAL_MODULE)
$(LOCAL_BUILT_MODULE): $(GEN_IMG_DIR)/$(GEN_IMG_GSI)
	echo "IPA_FWS staging $(PRIVATE_FILE)"
	if [ -d $(OUT_GSI_FW) ]; then \
		cp -f $(OUT_GSI_FW)/$(PRIVATE_FILE) $@; \
	else \
		touch $@; \
	fi

include $(CLEAR_VARS)
LOCAL_MODULE := ipa_fws.b01
LOCAL_SANITIZE:=integer_overflow
LOCAL_MODULE_CLASS := FAKE
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := ipa
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_MODULE_PATH := $(GEN_IMG_DIR)
include $(BUILD_SYSTEM)/base_rules.mk
$(LOCAL_BUILT_MODULE): PRIVATE_FILE := $(LOCAL_MODULE)
$(LOCAL_BUILT_MODULE): $(GEN_IMG_DIR)/ipa_fws.b00
	echo "IPA_FWS staging $(PRIVATE_FILE)"
	if [ -d $(OUT_GSI_FW) ]; then \
		cp -f $(OUT_GSI_FW)/$(PRIVATE_FILE) $@; \
	else \
		touch $@; \
	fi

include $(CLEAR_VARS)
LOCAL_MODULE := ipa_fws.b02
LOCAL_SANITIZE:=integer_overflow
LOCAL_MODULE_CLASS := FAKE
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := ipa
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_MODULE_PATH := $(GEN_IMG_DIR)
include $(BUILD_SYSTEM)/base_rules.mk
$(LOCAL_BUILT_MODULE): PRIVATE_FILE := $(LOCAL_MODULE)
$(LOCAL_BUILT_MODULE): $(GEN_IMG_DIR)/ipa_fws.b01
	echo "IPA_FWS staging $(PRIVATE_FILE)"
	if [ -d $(OUT_GSI_FW) ]; then \
		cp -f $(OUT_GSI_FW)/$(PRIVATE_FILE) $@; \
	else \
		touch $@; \
	fi

ifeq ($(USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN),1)
include $(CLEAR_VARS)
LOCAL_MODULE := ipa_fws.b03
LOCAL_SANITIZE:=integer_overflow
LOCAL_MODULE_CLASS := FAKE
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := ipa
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_MODULE_PATH := $(GEN_IMG_DIR)
include $(BUILD_SYSTEM)/base_rules.mk
$(LOCAL_BUILT_MODULE): PRIVATE_FILE := $(LOCAL_MODULE)
$(LOCAL_BUILT_MODULE): $(GEN_IMG_DIR)/ipa_fws.b02
	echo "IPA_FWS staging $(PRIVATE_FILE)"
	if [ -d $(OUT_GSI_FW) ]; then \
		cp -f $(OUT_GSI_FW)/$(PRIVATE_FILE) $@; \
	else \
		touch $@; \
	fi

include $(CLEAR_VARS)
LOCAL_MODULE := ipa_fws.b04
LOCAL_SANITIZE:=integer_overflow
LOCAL_MODULE_CLASS := FAKE
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := ipa
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_MODULE_PATH := $(GEN_IMG_DIR)
include $(BUILD_SYSTEM)/base_rules.mk
$(LOCAL_BUILT_MODULE): PRIVATE_FILE := $(LOCAL_MODULE)
$(LOCAL_BUILT_MODULE): $(GEN_IMG_DIR)/ipa_fws.b03
	echo "IPA_FWS staging $(PRIVATE_FILE)"
	if [ -d $(OUT_GSI_FW) ]; then \
		cp -f $(OUT_GSI_FW)/$(PRIVATE_FILE) $@; \
	else \
		touch $@; \
	fi
endif

include $(CLEAR_VARS)
LOCAL_MODULE := ipa_fws.mdt
LOCAL_SANITIZE:=integer_overflow
LOCAL_MODULE_CLASS := FAKE
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := ipa
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_MODULE_PATH := $(GEN_IMG_DIR)
include $(BUILD_SYSTEM)/base_rules.mk
$(LOCAL_BUILT_MODULE): PRIVATE_FILE := $(LOCAL_MODULE)
$(LOCAL_BUILT_MODULE): $(GEN_IMG_DIR)/$(GEN_IMG_GSI) $(GEN_IMG_DIR)/$(GEN_IMG_GSI_LAST)
	echo "IPA_FWS staging $(PRIVATE_FILE)"
	if [ -d $(OUT_GSI_FW) ]; then \
		cp -f $(OUT_GSI_FW)/$(PRIVATE_FILE) $@; \
		rm -rf $(OUT_GSI_FW); \
	else \
		touch $@; \
	fi

include $(CLEAR_VARS)
LOCAL_MODULE := $(GEN_IMG_GSI)
LOCAL_SANITIZE:=integer_overflow
LOCAL_MODULE_CLASS := FAKE
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := ipa
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_MODULE_PATH := $(GEN_IMG_DIR)
include $(BUILD_SYSTEM)/base_rules.mk
$(LOCAL_BUILT_MODULE): PRIVATE_PATH := $(LOCAL_PATH)
$(LOCAL_BUILT_MODULE): $(BINS_GSI) $(TOOLS_GSI)
	mkdir -p $(OUT_GSI_FW)
	# Assemble binaries/firmwares to a single ELF file
	python $(PRIVATE_PATH)/elf_creator.py \
		--gsi_fw $(PRIVATE_PATH)/$(GSI_FW_FILE) \
		--gsi_fw_address $(GSI_ADDR) \
		--gsi_fw_mem_size $(GSI_MEM_SIZE) \
		--hps $(PRIVATE_PATH)/$(HPS_BIN) \
		--hps_addr $(HPS_ADDR) \
		--dps $(PRIVATE_PATH)/dps.bin \
		--dps_addr $(DPS_ADDR) \
		--outfile $(OUT_GSI_FW)/$(GEN_IMG_GSI)
	echo IPA_FWS: Creating ipa ELF image OUT folders
	mkdir -p $(PRODUCT_OUT)/ipa
	mkdir -p $(PRODUCT_OUT)/ipa/signed
	mkdir -p $(PRODUCT_OUT)/ipa/unsigned
	echo IPA_FWS: install unsigned $(GEN_IMG_GSI) at $(PRODUCT_OUT)/ipa/unsigned
	cp -f $(OUT_GSI_FW)/$(GEN_IMG_GSI) $(PRODUCT_OUT)/ipa/unsigned
	# Sign the ELF file using SecImage tool
	SECIMAGE_LOCAL_DIR=$(SECIMAGE_BASE) USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN=$(USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN) \
	python $(SECIMAGE_BASE)/sectools_builder.py \
		-i $(OUT_GSI_FW)/$(GEN_IMG_GSI) \
		-g ipa_fw \
		-t $(OUT_GSI_FW) \
		--install_base_dir=$(OUT_GSI_FW) \
		--config $(SECIMAGE_BASE)/$(SECIMAGE_CONFIG_FILE) \
		--soc_hw_version=$(SOC_HW_VERSION) \
		--soc_vers=$(SOC_VERS) \
		> $(OUT_GSI_FW)/secimage.log 2>&1
	echo IPA_FWS: install signed $(GEN_IMG_GSI) at $(PRODUCT_OUT)/ipa/signed
	cp -f $(OUT_GSI_FW)/$(GEN_IMG_GSI) $(PRODUCT_OUT)/ipa/signed
	# PIL split the output of the SecImage tool
	python $(PILSPLITTER_BASE)/pil-splitter.py \
		$(OUT_GSI_FW)/$(GEN_IMG_GSI) \
		$(OUT_GSI_FW)/ipa_fws
	cp -f $(OUT_GSI_FW)/ipa_fws.elf $@
endif # GSI FW file existence check

# uC image creation
ifeq ($(GEN_IPA_UC),true)
$(info IPA_FWS: Create uC image files)

# Check uC FW file existence
ifeq ("$(wildcard $(LOCAL_PATH)/$(UC_FW_FILE))","")
$(info IPA_FWS: $(LOCAL_PATH)/$(UC_FW_FILE) does not exist!)
else # uC FW file existence check
BINS_UC := $(LOCAL_PATH)/$(UC_FW_FILE)
TOOLS_UC := $(LOCAL_PATH)/elf_creator_uc.py $(SECIMAGE_BASE)/sectools_builder.py $(PILSPLITTER_BASE)/pil-splitter.py
OUT_UC_FW := $(OUT_DIR)/ipa_tmp_uc

GEN_IMG_UC := ipa_uc.elf
ifeq ($(USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN),1)
GEN_IMG_UC_OTHERS := ipa_uc.b00 ipa_uc.b01 ipa_uc.b02 ipa_uc.mdt
GEN_IMG_UC_LAST := ipa_uc.b02
else
GEN_IMG_UC_OTHERS := ipa_uc.b00 ipa_uc.mdt
GEN_IMG_GSI_LAST := ipa_uc.b00
endif
$(info GEN_IMG_UC_OTHERS=$(GEN_IMG_UC_OTHERS) GEN_IMG_UC_LAST=$(GEN_IMG_UC_LAST))

include $(CLEAR_VARS)
LOCAL_MODULE := ipa_uc
LOCAL_SANITIZE:=integer_overflow
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH := $(OUT_UC_FW)
LOCAL_REQUIRED_MODULES := $(GEN_IMG_UC) $(GEN_IMG_UC_OTHERS)
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE := ipa_uc.b00
LOCAL_SANITIZE:=integer_overflow
LOCAL_MODULE_CLASS := FAKE
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := ipa
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_MODULE_PATH := $(GEN_IMG_DIR)
include $(BUILD_SYSTEM)/base_rules.mk
$(LOCAL_BUILT_MODULE): PRIVATE_FILE := $(LOCAL_MODULE)
$(LOCAL_BUILT_MODULE): $(GEN_IMG_DIR)/$(GEN_IMG_UC)
	echo "IPA_FWS staging $(PRIVATE_FILE)"
	if [ -d $(OUT_UC_FW) ]; then \
		cp -f $(OUT_UC_FW)/$(PRIVATE_FILE) $@; \
	else \
		touch $@; \
	fi

ifeq ($(USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN),1)
include $(CLEAR_VARS)
LOCAL_MODULE := ipa_uc.b01
LOCAL_SANITIZE:=integer_overflow
LOCAL_MODULE_CLASS := FAKE
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := ipa
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_MODULE_PATH := $(GEN_IMG_DIR)
include $(BUILD_SYSTEM)/base_rules.mk
$(LOCAL_BUILT_MODULE): PRIVATE_FILE := $(LOCAL_MODULE)
$(LOCAL_BUILT_MODULE): $(GEN_IMG_DIR)/ipa_uc.b00
	echo "IPA_FWS staging $(PRIVATE_FILE)"
	if [ -d $(OUT_UC_FW) ]; then \
		cp -f $(OUT_UC_FW)/$(PRIVATE_FILE) $@; \
	else \
		touch $@; \
	fi

include $(CLEAR_VARS)
LOCAL_MODULE := ipa_uc.b02
LOCAL_SANITIZE:=integer_overflow
LOCAL_MODULE_CLASS := FAKE
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := ipa
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_MODULE_PATH := $(GEN_IMG_DIR)
include $(BUILD_SYSTEM)/base_rules.mk
$(LOCAL_BUILT_MODULE): PRIVATE_FILE := $(LOCAL_MODULE)
$(LOCAL_BUILT_MODULE): $(GEN_IMG_DIR)/ipa_uc.b01
	echo "IPA_FWS staging $(PRIVATE_FILE)"
	if [ -d $(OUT_UC_FW) ]; then \
		cp -f $(OUT_UC_FW)/$(PRIVATE_FILE) $@; \
	else \
		touch $@; \
	fi
endif

include $(CLEAR_VARS)
LOCAL_MODULE := ipa_uc.mdt
LOCAL_SANITIZE:=integer_overflow
LOCAL_MODULE_CLASS := FAKE
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := ipa
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_MODULE_PATH := $(GEN_IMG_DIR)
include $(BUILD_SYSTEM)/base_rules.mk
$(LOCAL_BUILT_MODULE): PRIVATE_FILE := $(LOCAL_MODULE)
$(LOCAL_BUILT_MODULE): $(GEN_IMG_DIR)/$(GEN_IMG_UC) $(GEN_IMG_DIR)/$(GEN_IMG_UC_LAST)
	echo "IPA_FWS staging $(PRIVATE_FILE)"
	if [ -d $(OUT_UC_FW) ]; then \
		cp -f $(OUT_UC_FW)/$(PRIVATE_FILE) $@; \
		rm -rf $(OUT_UC_FW); \
	else \
		touch $@; \
	fi

include $(CLEAR_VARS)
LOCAL_MODULE := $(GEN_IMG_UC)
LOCAL_SANITIZE:=integer_overflow
LOCAL_MODULE_CLASS := FAKE
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := ipa
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_MODULE_PATH := $(GEN_IMG_DIR)
include $(BUILD_SYSTEM)/base_rules.mk
$(LOCAL_BUILT_MODULE): PRIVATE_PATH := $(LOCAL_PATH)
$(LOCAL_BUILT_MODULE): $(BINS_UC) $(TOOLS_UC)
	mkdir -p $(OUT_UC_FW)
	# Assemble binaries/firmwares to a single ELF file
	python $(PRIVATE_PATH)/elf_creator_uc.py \
		--uc_fw $(PRIVATE_PATH)/$(UC_FW_FILE) \
		--uc_fw_address $(UC_ADDR) \
		--uc_fw_mem_size $(UC_MEM_SIZE) \
		--outfile $(OUT_UC_FW)/$(GEN_IMG_UC)
	echo IPA_FWS: Creating ipa ELF image OUT folders
	mkdir -p $(PRODUCT_OUT)/ipa
	mkdir -p $(PRODUCT_OUT)/ipa/signed
	mkdir -p $(PRODUCT_OUT)/ipa/unsigned
	echo IPA_FWS: install unsigned $(GEN_IMG_UC) at $(PRODUCT_OUT)/ipa/unsigned
	cp -f $(OUT_UC_FW)/$(GEN_IMG_UC) $(PRODUCT_OUT)/ipa/unsigned
	# Sign the ELF file using SecImage tool
	SECIMAGE_LOCAL_DIR=$(SECIMAGE_BASE) USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN=$(USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN) \
	python $(SECIMAGE_BASE)/sectools_builder.py \
                -i $(OUT_UC_FW)/$(GEN_IMG_UC) \
                -g ipa_uc \
                -t $(OUT_UC_FW) \
                --install_base_dir=$(OUT_UC_FW) \
                --config $(SECIMAGE_BASE)/$(SECIMAGE_CONFIG_FILE) \
                --soc_hw_version=$(SOC_HW_VERSION) \
                --soc_vers=$(SOC_VERS) \
                > $(OUT_UC_FW)/secimage.log 2>&1
	echo IPA_FWS: install signed $(GEN_IMG_UC) at $(PRODUCT_OUT)/ipa/signed
	cp -f $(OUT_UC_FW)/$(GEN_IMG_UC) $(PRODUCT_OUT)/ipa/signed
	# PIL split the output of the SecImage tool
	python $(PILSPLITTER_BASE)/pil-splitter.py \
		$(OUT_UC_FW)/$(GEN_IMG_UC) \
		$(OUT_UC_FW)/ipa_uc
	cp -f $(OUT_UC_FW)/ipa_uc.elf $@
endif # uC FW file existence check

else # uC image creation
$(info IPA_FWS: Do not create uC image files)
endif # uC image creation

else #Target check
$(info IPA_FWS: unknow target $(TARGET_BOARD_PLATFORM). skipping)
endif #Target check
