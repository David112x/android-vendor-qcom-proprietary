LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := qcril.db
LOCAL_MODULE_CLASS := ETC
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/radio/qcril_database

$(foreach script,$(call all-named-files-under,*.sql,upgrade), \
$(eval LOCAL_REQUIRED_MODULES += $(subst upgrade/,,$(script))) \
)

include $(BUILD_SYSTEM)/base_rules.mk

DB_UPGRADE_PATH := $(LOCAL_PATH)/upgrade
DB_UPGRADES := $(sort $(addprefix $(LOCAL_PATH)/,$(call all-named-files-under,*.sql,upgrade)))

$(LOCAL_BUILT_MODULE): $(DB_UPGRADES) $(HOST_OUT_EXECUTABLES)/sqlite3
	@rm -f $@
	$(hide) cat $(DB_UPGRADES) | $(HOST_OUT_EXECUTABLES)/sqlite3 $@

include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := debug

include $(BUILD_HOST_PREBUILT)


$(foreach script,$(call all-named-files-under,*.sql,upgrade), \
$(eval include $(CLEAR_VARS)) \
$(eval LOCAL_MODULE := $(subst upgrade/,,$(script))) \
$(eval LOCAL_PROPRIETARY_MODULE := true) \
$(eval LOCAL_MODULE_OWNER := qti) \
$(eval LOCAL_MODULE_CLASS := SCRIPT) \
$(eval LOCAL_MODULE_TAGS := optional) \
$(eval LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/radio/qcril_database/upgrade) \
$(eval LOCAL_SRC_FILES := $(script)) \
$(eval include $(BUILD_PREBUILT)) \
)
