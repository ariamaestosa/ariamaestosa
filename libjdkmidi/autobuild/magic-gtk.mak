LIB_GUI_DIR+=$(call target_suffix_platform_dirs,gui/gtk) $(foreach i,$(SUBLIBS),$(call target_suffix_platform_dirs,gui/gtk$(i)))
