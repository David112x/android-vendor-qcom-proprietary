ifneq ($(AUDIO_USE_STUB_HAL), true)
ifeq ($(call is-board-platform-in-list, msm8996 msm8909),true)

ifneq ($(AUDIO_FEATURE_ENABLED_BG_KEYWORD_DETECTION),true)
ifneq ($(AUDIO_FEATURE_DISABLED_SOUND_TRIGGER_LEGACY_HAL),true)
    USE_SOUND_TRIGGER_LEGACY_HAL := true
endif # is platform MSM8909
endif # is platform MSM8909 BG
endif

ifeq ($(call is-board-platform-in-list,msm8909 msm8996 msm8937 msm8953 msm8998 apq8098_latv sdm660 sdm845 sdm710 qcs605 msmnile $(MSMSTEPPE) $(TRINKET) kona lito atoll),true)
include $(call all-subdir-makefiles)
endif
endif
