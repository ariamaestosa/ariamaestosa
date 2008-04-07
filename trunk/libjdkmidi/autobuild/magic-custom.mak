# read any custom mak files in the top dir of each library dir / project

CUSTOM_MAK_FILES := $(foreach dir,$(LIB_DIRS),$(dir)/$(dir).mak)

ifneq ($(CUSTOM_MAK_FILES),)
-include $(CUSTOM_MAK_FILES)
endif

