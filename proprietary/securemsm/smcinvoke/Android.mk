ifeq ($(call is-board-platform-in-list,msmnile sdmshrike sdm845 sdm660 sdm710 $(MSMSTEPPE) kona lito atoll bengal $(TRINKET)),true)

include $(call all-subdir-makefiles)

endif
