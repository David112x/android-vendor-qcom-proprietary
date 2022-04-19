# signing tool
SECIMAGE_BASE_PATH := android_compat/common/scripts/SecImage
SECIMAGE_PATH := $(WORKSPACE)/$(SECIMAGE_BASE_PATH)
HASH_SEG_ALGO := sha384

# 9650 IPA FWs configs (IPAv3.0)
if IS_MDM9650
DPS_ADDR := 0x07B5e000
HPS_ADDR := 0x07B5e080
GSI_ADDR := 0x07B08000
GSI_FW_FILE := fw_mhi_ipa_v3.bin
GSI_MEM_SIZE = 0x4000
TARGET_NAME := mdm9650
SOC_HW_VERSION := 0x20080100
FW_OPERATIONS := create_elf
SECIMAGE_CONFIG_FILE := config/integration/secimage.xml
endif # 9650 check

# sdx20 IPA FWs configs (IPAv3.5)
if IS_SDX20
DPS_ADDR := 0x07B5e000
HPS_ADDR := 0x07B5e080
GSI_ADDR := 0x07B08000
GSI_FW_FILE := fw_mhi_ipa_v3.5.bin
GSI_MEM_SIZE = 0x4000
TARGET_NAME := sdx20
SOC_HW_VERSION := 0x200E0000
FW_OPERATIONS := sign_and_split
SECIMAGE_CONFIG_FILE := config/integration/secimage.xml
endif # sdx20 check

# sdxpoorwills IPA FWs configs (IPAv4.0)
if IS_SDXPOORWILLS
DPS_ADDR := 0x01E5E000
HPS_ADDR := 0x01E5E080
GSI_ADDR := 0x01E08000
GSI_FW_FILE := fw_mhi_ipa_v4.0.bin
GSI_MEM_SIZE = 0x6000
TARGET_NAME := sdxpoorwills
SOC_HW_VERSION := 0x60020100
FW_OPERATIONS := sign_and_split_v6
SECIMAGE_CONFIG_FILE := config/integration/secimagev3.xml
SOC_VERS := 0x6002
endif # sdxpoorwills check

CURRENT_DIR := $(shell pwd)
$(info Current dir is $(CURRENT_DIR))
$(info Workspace dir is $(WORKSPACE))
$(info FW operations are $(FW_OPERATIONS))

# split image tool
SPLIT_BASE_PATH := android_compat/common/scripts/
SPLIT_PATH := $(WORKSPACE)/$(SPLIT_BASE_PATH)

BINS := $(CURRENT_DIR)/$(GSI_FW_FILE) $(CURRENT_DIR)/hps.bin $(CURRENT_DIR)/dps.bin

create_elf: $(BINS) $(CURRENT_DIR)/elf_creator.py
	@echo IPA FW creation for $(TARGET_NAME)
	if [ ! -e $(CURRENT_DIR)/$(GSI_FW_FILE) ]; then \
		echo FW file missing at $(CURRENT_DIR)/$(GSI_FW_FILE) ;\
		exit -1; \
	fi;
	python elf_creator.py \
	--gsi_fw $(GSI_FW_FILE) \
	--gsi_fw_address $(GSI_ADDR) \
	--gsi_fw_mem_size $(GSI_MEM_SIZE) \
	--hps hps.bin \
	--hps_addr $(HPS_ADDR) \
	--dps dps.bin \
	--dps_addr $(DPS_ADDR) \
	--outfile ipa_fws.elf

sign_and_split: create_elf $(SECIMAGE_PATH)/sectools_builder.py $(SPLIT_PATH)/pil-splitter.py
	@echo Sign with $(SECIMAGE_BASE_PATH)/sectools_builder.py
	if [ ! -x $(SECIMAGE_PATH)/sectools_builder.py ]; then \
		echo No Executable $(SECIMAGE_PATH)/sectools_builder.py ;\
		exit -1; \
	fi;
	if [ ! -e $(SECIMAGE_PATH)/$(SECIMAGE_CONFIG_FILE) ]; then \
		echo No signing config file $(SECIMAGE_PATH)/$(SECIMAGE_CONFIG_FILE) ;\
		exit -1; \
	fi;
	SECIMAGE_LOCAL_DIR=$(SECIMAGE_PATH) USES_SEC_POLICY_DEFAULT_SIGN_UPDATE_UNSIGN=1 \
	python $(SECIMAGE_PATH)/sectools_builder.py \
	-i $(CURRENT_DIR)/ipa_fws.elf \
	-g ipa_fw \
	-t $(CURRENT_DIR) \
	--install_base_dir=$(CURRENT_DIR) \
	--config $(SECIMAGE_PATH)/$(SECIMAGE_CONFIG_FILE) \
	>$(CURRENT_DIR)/secimage.log 2>&1
	@echo Split with $SPLIT_BASE_PATH/pil-splitter.py
	if [ ! -x $(SPLIT_PATH)/pil-splitter.py ]; then \
		echo No Executable $(SPLIT_PATH)/pil-splitter.py ;\
		exit -1; \
	fi;
	python $(SPLIT_PATH)/pil-splitter.py \
	$(CURRENT_DIR)/ipa_fws.elf \
	$(CURRENT_DIR)/ipa_fws

sign_and_split_v6: create_elf $(SECIMAGE_PATH)/sectools_builder.py $(SPLIT_PATH)/pil-splitter.py
	@echo Sign with $(SECIMAGE_BASE_PATH)/sectools_builder.py
	if [ ! -x $(SECIMAGE_PATH)/sectools_builder.py ]; then \
		echo No Executable $(SECIMAGE_PATH)/sectools_builder.py ;\
		exit -1; \
	fi;
	if [ ! -e $(SECIMAGE_PATH)/$(SECIMAGE_CONFIG_FILE) ]; then \
		echo No signing config file $(SECIMAGE_PATH)/$(SECIMAGE_CONFIG_FILE) ;\
		exit -1; \
	fi;
	SECIMAGE_LOCAL_DIR=$(SECIMAGE_PATH) USES_SEC_POLICY_DEFAULT_SIGN_UPDATE_UNSIGN=1 \
	python $(SECIMAGE_PATH)/sectools_builder.py \
	-i $(CURRENT_DIR)/ipa_fws.elf \
	-g ipa_fw \
	-t $(CURRENT_DIR) \
	--install_base_dir=$(CURRENT_DIR) \
	--hash_table_algo=$(HASH_SEG_ALGO) \
	--config $(SECIMAGE_PATH)/$(SECIMAGE_CONFIG_FILE) \
	--soc_vers $(SOC_VERS) \
	--soc_hw_version $(SOC_HW_VERSION) \
	>$(CURRENT_DIR)/secimage.log 2>&1
	@echo Split with $SPLIT_BASE_PATH/pil-splitter.py
	if [ ! -x $(SPLIT_PATH)/pil-splitter.py ]; then \
		echo No Executable $(SPLIT_PATH)/pil-splitter.py ;\
		exit -1; \
	fi;
	python $(SPLIT_PATH)/pil-splitter.py \
	$(CURRENT_DIR)/ipa_fws.elf \
	$(CURRENT_DIR)/ipa_fws

all: $(FW_OPERATIONS)

clean:
	rm -rf ipa_fws.* ipa_fws-signed.*
	rm -rf secimage.log Sectools_Builder_ipa_fw_log.txt
	rm -rf sign sign_and_encrypt
	rm -rf signed signed_encrypted
	rm -rf unsigned integrity_check
