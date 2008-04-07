
MACOSX_DEFAULT_BUNDLE_ICON?=icon
MACOSX_DEFAULT_BUNDLE_IDENTIFIER?=exampleproject.$(PROJECT)
MACOSX_DEFAULT_BUNDLE_SIGNATURE?=????
MACOSX_DEFAULT_BUNDLE_VERSION?=$(PROJECT_VERSION)
MACOSX_DEFAULT_BUNDLE_COPYRIGHT?=$(PROJECT_COPYRIGHT)
LIB_GUI_EXE_FILES=$(addsuffix .app,$(addprefix $(OUTPUT_GUI_DIR)/,$(notdir $(LIB_GUI_O_FILES:.o=))))


#############################################
#
# Make a make os x application bundle
#
# parameters:
# 1 : Destination dir
# 2 : Application base name
# 3 : Source application executable
# 4	: Source Info.plist file
# 5 : Source Resources files and dirs

define make_macosx_app_bundle
	@echo MACOSX APP BUNDLE: $(2).app
	@rm -r -f $(1)/$(2).app
	@mkdir -p $(addprefix $(1)/$(2).app/Contents/,MacOS Frameworks Resources PlugIns SharedFrameworks SharedSupport)
	@$(CP) $(3) $(1)/$(2).app/Contents/MacOS/$(2)
	@$(CP) $(4) $(1)/$(2).app/Contents/Info.plist
	$(if $(wildcard $(5)/*),$(RSYNC) $(5)/* "$(1)/$(2).app/Contents/Resources/" )

endef


#############################################
#
# make a Info.plist file
# parameters:
#
# 1 : destination dir
# 2 : output file name
# 3 : application name
# 4 : icon file name
# 5 : bundle identifier
# 6 : bundle signature
# 7 : bundle version
# 8 : copyright

define make_macosx_info_plist_file
	@echo '<?xml version="1.0" encoding="UTF-8"?>' >$(1)/$(2)
	@echo '<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">' >>$(1)/$(2)
	@echo '<plist version="1.0">' >>$(1)/$(2)
	@echo '<dict>' >>$(1)/$(2)
	@echo '    <key>CFBundleDevelopmentRegion</key>' >>$(1)/$(2)
	@echo '    <string>English</string>' >>$(1)/$(2)
	@echo '    <key>CFBundleExecutable</key>' >>$(1)/$(2)
	@echo '    <string>$(3)</string>' >>$(1)/$(2)
	@echo '    <key>CFBundleIconFile</key>' >>$(1)/$(2)
	@echo '    <string>$(4)</string>' >>$(1)/$(2)
	@echo '    <key>CFBundleIdentifier</key>' >>$(1)/$(2)
	@echo '    <string>$(5)</string>' >>$(1)/$(2)
	@echo '    <key>CFBundleInfoDictionaryVersion</key>' >>$(1)/$(2)
	@echo '    <string>6.0</string>' >>$(1)/$(2)
	@echo '    <key>CFBundlePackageType</key>' >>$(1)/$(2)
	@echo '    <string>APPL</string>' >>$(1)/$(2)
	@echo '    <key>CFBundleSignature</key>' >>$(1)/$(2)
	@echo '    <string>$(6)</string>' >>$(1)/$(2)
	@echo '    <key>CFBundleVersion</key>' >>$(1)/$(2)
	@echo '    <string>$(7)</string>' >>$(1)/$(2)
	@echo '    <key>NSHumanReadableCopyright</key>' >>$(1)/$(2)
	@echo '    <string>$(8)</string>' >>$(1)/$(2)
	@echo '</dict>' >>$(1)/$(2)
	@echo '</plist>' >>$(1)/$(2)

endef


#######################################
#
# make a simple Info.plist file.
# 
# parameters:
# 
# 1 : output dir name
# 2 : output file name
# 3 : output application name
#

define make_default_macosx_info_plist_file
	$(call make_macosx_info_plist_file,$(1),$(2),$(3),$(basename $(MACOSX_DEFAULT_BUNDLE_ICON)),$(MACOSX_DEFAULT_BUNDLE_IDENTIFIER),$(MACOSX_DEFAULT_BUNDLE_SIGNATURE),$(MACOSX_DEFAULT_BUNDLE_VERSION),$(MACOSX_DEFAULT_BUNDLE_COPYRIGHT))

endef



$(OUTPUT_GUI_DIR)/%.app : $(OUTPUT_OBJ_DIR)/%.o
	@echo "LINKING MAC OS X gui: $(notdir $<)"
	@$(call make_default_macosx_info_plist_file,$(OUTPUT_OBJ_DIR),$*-Info.plist,$*)
	@$(LINK.cpp) $(LINK_FLAGS) $(LDFLAGS) $(LINK_FLAGS_GUI) -o $(OUTPUT_OBJ_DIR)/$*$(EXE) $< -L$(OUTPUT_LIB_DIR) $(PROJECT_LDLIB) $(LDLIBS) $(LDLIBS_PACKAGES) $(LDLIBS_GUI)
	@$(call make_macosx_app_bundle,$(OUTPUT_GUI_DIR),$*,$(OUTPUT_OBJ_DIR)/$*$(EXE),$(OUTPUT_OBJ_DIR)/$*-Info.plist,$(PROJECT_TOP_DIR)/gui/macosx/$*/resources/)

XCODEBUILD?=xcodebuild
XCODEBUILD_OPTS?=


define macosx_xcode_gui_program

LIB_GUI_EXE_FILES+=$$(OUTPUT_GUI_DIR)/$(1).app

$$(OUTPUT_GUI_DIR)/$(1).app : lib $$(PROJECT_TOP_DIR)/gui/macosx-xcode/$(1)/build/Release/$(1).app
	@echo COPYING MACOSX-XCODE gui app: $(1).app
	@$(RM) -r -f $$(OUTPUT_GUI_DIR)/$(1).app
	@$$(RSYNC) -aE $$(PROJECT_TOP_DIR)/gui/macosx-xcode/$(1)/build/Release/$(1).app $$(OUTPUT_GUI_DIR)/

$$(PROJECT_TOP_DIR)/gui/macosx-xcode/$(1)/build/Release/$(1).app :
	@echo BUILDING MACOSX-XCODE gui app: $(1).app
	@( cd $$(PROJECT_TOP_DIR)/gui/macosx-xcode/$(1) && $$(XCODEBUILD) $$(XCODEBUILD_OPTS) )

CLEAN_DIRS+=$$(PROJECT_TOP_DIR)/gui/macosx-xcode/$(1)/build $$(OUTPUT_GUI_DIR)/$(1).app

endef

$(foreach prog,$(call bare_subdirs_in_path,$(PROJECT_TOP_DIR)/gui/macosx-xcode),$(eval $(call macosx_xcode_gui_program,$(prog))))

define macosx_xcode_gui_sublib_program

LIB_GUI_EXE_FILES+=$$(OUTPUT_GUI_DIR)/$(1).app

$$(OUTPUT_GUI_DIR)/$(1).app : lib $$(PROJECT_TOP_DIR)/gui/$(2)/macosx-xcode/$(1)/build/Release/$(1).app
	@echo COPYING MACOSX-XCODE gui app: $(1).app
	@$(RM) -r -f $$(OUTPUT_GUI_DIR)/$(1).app
	@$$(RSYNC) -aE $$(PROJECT_TOP_DIR)/gui/$(2)/macosx-xcode/$(1)/build/Release/$(1).app $$(OUTPUT_GUI_DIR)/

$$(PROJECT_TOP_DIR)/gui/$(2)/macosx-xcode/$(1)/build/Release/$(1).app :
	@echo BUILDING MACOSX-XCODE gui app: $(1).app
	@( cd $$(PROJECT_TOP_DIR)/gui/$(2)/macosx-xcode/$(1) && $$(XCODEBUILD) $$(XCODEBUILD_OPTS) )

CLEAN_DIRS+=$$(PROJECT_TOP_DIR)/gui/$(2)/macosx-xcode/$(1)/build $$(OUTPUT_GUI_DIR)/$(1).app

endef


$(foreach sublib,$(SUBLIBS),$(foreach prog,$(call bare_subdirs_in_path,$(PROJECT_TOP_DIR)/gui/$(sublib)/macosx-code),$(eval $(call macosx_xcode_gui_sublib_program,$(prog),$(sublib)))))

