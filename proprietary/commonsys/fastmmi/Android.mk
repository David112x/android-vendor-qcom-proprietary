ifeq ($(call is-board-platform-in-list,msm8909 msm8916 msm8952 msm8996 msm8937 msm8953 msm8998 sdm630 sdm660 sdm710 sdm845 msmnile $(MSMSTEPPE) kona lito bengal atoll $(TRINKET)),true)
include $(call all-subdir-makefiles)
endif
