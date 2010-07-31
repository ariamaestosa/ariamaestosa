# Now we know all our target platform suffixes in SUFFIXES_TARGET_PLATFORM, so
# we eval the result of the calc_multi_target_options function. This effectively merges in
# the platform specific defines, compile flags, and link flags

$(eval $(call calc_multi_target_options,$(SUFFIXES_TARGET_PLATFORM)))

PACKAGE_BASENAME?=$(PROJECT)-$(PROJECT_VERSION)-$(TARGET_PLATFORM_NAME)$(PACKAGE_SUFFIX)
PACKAGEDEV_BASENAME?=$(PROJECT)-dev-$(PROJECT_VERSION)-$(TARGET_PLATFORM_NAME)$(PACKAGE_SUFFIX)
PACKAGEDOCSDEV_BASENAME?=$(PROJECT)-docs-dev-$(PROJECT_VERSION)-$(TARGET_PLATFORM_NAME)$(PACKAGE_SUFFIX)
PACKAGETESTRESULTS_BASENAME?=$(PROJECT)-testresults-$(PROJECT_VERSION)-$(TARGET_PLATFORM_NAME)$(PACKAGE_SUFFIX)

##############################################################################################
#
# Calculate all our source dirs for various things, for our target platform
#
LIB_INCLUDE_DIR+=$(call target_suffix_platform_dirs,include) 
LIB_SRC_DIR+=$(call target_suffix_platform_dirs,src) $(foreach i,$(SUBLIBS),$(call target_suffix_platform_dirs,src/$(i)))
LIB_TESTS_DIR+=$(call target_suffix_platform_dirs,tests) $(foreach i,$(SUBLIBS),$(call target_suffix_platform_dirs,tests/$(i)))
LIB_GUI_DIR+=$(call target_suffix_platform_dirs,gui) $(foreach i,$(SUBLIBS),$(call target_suffix_platform_dirs,gui/$(i)))
LIB_EXAMPLES_DIR+=$(call target_suffix_platform_dirs,examples) $(foreach i,$(SUBLIBS),$(call target_suffix_platform_dirs,examples/$(i)))
LIB_TOOLS_DIR+=$(call target_suffix_platform_dirs,tools) $(foreach i,$(SUBLIBS),$(call target_suffix_platform_dirs,tools/$(i)))
LIB_DOCS_DIR+=$(PROJECT_TOP_DIR)/docs/html
LIB_DOCS_DEV_DIR+=$(PROJECT_TOP_DIR)/docs

ALL_SOURCES_DIRS=$(strip $(LIB_SRC_DIR) $(LIB_TESTS_DIR) $(LIB_GUI_DIR) $(LIB_EXAMPLES_DIR) $(LIB_TOOLS_DIR))

# calculate our output directories for our target platform results
OUTPUT_DIR=$(BUILD_DIR)/build
OUTPUT_LIB_DIR?=$(OUTPUT_DIR)/lib
OUTPUT_TESTS_DIR?=$(OUTPUT_DIR)/tests
OUTPUT_DOCS_DEV_DIR?=$(OUTPUT_DIR)/docs
OUTPUT_DOCS_DIR?=$(OUTPUT_DIR)/docs/api
OUTPUT_TOOLS_DIR?=$(OUTPUT_DIR)/tools
OUTPUT_GUI_DIR?=$(OUTPUT_DIR)/gui
OUTPUT_EXAMPLES_DIR?=$(OUTPUT_DIR)/examples
OUTPUT_OBJ_DIR?=$(OUTPUT_DIR)/obj

# our output libraries name is simply our project name prefixed with lib in our output lib dir.

ifdef PREFIX
TARGET_INSTALL_DIR?=$(PREFIX)
else
TARGET_INSTALL_DIR?=/opt/local/$(PROJECT)-$(PROJECT_VERSION)
endif

TARGET_BIN_DIR?=bin
TARGET_LIB_DIR?=lib

ifdef TARGET_PLAIN_DIRS
TARGET_INCLUDE_DIR?=include
TARGET_DOCS_DIR?=share/doc/$(PROJECT)
TARGET_SHARE_DIR?=share/$(PROJECT)
TARGET_ETC_DIR?=etc/$(PROJECT)
else
TARGET_INCLUDE_DIR?=include/$(PROJECT)-$(PROJECT_VERSION)
TARGET_DOCS_DIR?=share/doc/$(PROJECT)-$(PROJECT_VERSION)
TARGET_SHARE_DIR?=share/$(PROJECT)-$(PROJECT_VERSION)
TARGET_ETC_DIR?=etc/$(PROJECT)-$(PROJECT_VERSION)
endif

TARGET_INSTALL_BIN_DIR?=$(TARGET_INSTALL_DIR)/$(TARGET_BIN_DIR)
TARGET_INSTALL_LIB_DIR?=$(TARGET_INSTALL_DIR)/$(TARGET_LIB_DIR)
TARGET_INSTALL_INCLUDE_DIR?=$(TARGET_INSTALL_DIR)/$(TARGET_INCLUDE_DIR)
TARGET_INSTALL_DOCS_DIR?=$(TARGET_INSTALL_DIR)/$(TARGET_DOCS_DIR)
TARGET_INSTALL_SHARE_DIR?=$(TARGET_INSTALL_DIR)/$(TARGET_SHARE_DIR)
TARGET_INSTALL_ETC_DIR?=$(TARGET_INSTALL_DIR)/$(TARGET_ETC_DIR)

LOCAL_INSTALL_DIR?=$(BUILD_DIR)/$(PROJECT)-$(PROJECT_VERSION)

LOCAL_INSTALL_BIN_DIR?=$(LOCAL_INSTALL_DIR)/$(TARGET_INSTALL_BIN_DIR)
LOCAL_INSTALL_LIB_DIR?=$(LOCAL_INSTALL_DIR)/$(TARGET_INSTALL_LIB_DIR)
LOCAL_INSTALL_INCLUDE_DIR?=$(LOCAL_INSTALL_DIR)/$(TARGET_INSTALL_INCLUDE_DIR)
LOCAL_INSTALL_DOCS_DIR?=$(LOCAL_INSTALL_DIR)/$(TARGET_INSTALL_DOCS_DIR)
LOCAL_INSTALL_SHARE_DIR?=$(LOCAL_INSTALL_DIR)/$(TARGET_INSTALL_SHARE_DIR)
LOCAL_INSTALL_ETC_DIR?=$(LOCAL_INSTALL_DIR)/$(TARGET_INSTALL_ETC_DIR)

TARGET_INSTALLDEV_BIN_DIR?=$(TARGET_INSTALL_DIR)/$(TARGET_BIN_DIR)
TARGET_INSTALLDEV_LIB_DIR?=$(TARGET_INSTALL_DIR)/$(TARGET_LIB_DIR)
TARGET_INSTALLDEV_INCLUDE_DIR?=$(TARGET_INSTALL_DIR)/$(TARGET_INCLUDE_DIR)
TARGET_INSTALLDEV_DOCS_DIR?=$(TARGET_INSTALL_DIR)/$(TARGET_DOCS_DIR)
TARGET_INSTALLDEV_SHARE_DIR?=$(TARGET_INSTALL_DIR)/$(TARGET_SHARE_DIR)
TARGET_INSTALLDEV_ETC_DIR?=$(TARGET_INSTALL_DIR)/$(TARGET_ETC_DIR)

LOCAL_INSTALLDEV_DIR?=$(BUILD_DIR)/$(PROJECT)-$(PROJECT_VERSION)-dev

LOCAL_INSTALLDEV_BIN_DIR?=$(LOCAL_INSTALLDEV_DIR)/$(TARGET_INSTALL_BIN_DIR)
LOCAL_INSTALLDEV_LIB_DIR?=$(LOCAL_INSTALLDEV_DIR)/$(TARGET_INSTALL_LIB_DIR)
LOCAL_INSTALLDEV_INCLUDE_DIR?=$(LOCAL_INSTALLDEV_DIR)/$(TARGET_INSTALL_INCLUDE_DIR)
LOCAL_INSTALLDEV_DOCS_DIR?=$(LOCAL_INSTALLDEV_DIR)/$(TARGET_INSTALL_DOCS_DIR)
LOCAL_INSTALLDEV_SHARE_DIR?=$(LOCAL_INSTALLDEV_DIR)/$(TARGET_INSTALL_SHARE_DIR)
LOCAL_INSTALLDEV_ETC_DIR?=$(LOCAL_INSTALLDEV_DIR)/$(TARGET_INSTALL_ETC_DIR)

LOCAL_INSTALLDOCSDEV_DIR?=$(BUILD_DIR)/$(PROJECT)-$(PROJECT_VERSION)-docs-dev
TARGET_INSTALLDOCSDEV_DOCS_DIR?=$(TARGET_INSTALL_DOCS_DIR)
LOCAL_INSTALLDOCSDEV_DOCS_DIR?=$(LOCAL_INSTALLDOCSDEV_DIR)/$(TARGET_INSTALLDOCSDEV_DOCS_DIR)

INSTALL_DIR?=$(TARGET_INSTALL_DIR)
INSTALL_BIN_DIR?=$(INSTALL_DIR)/$(TARGET_BIN_DIR)
INSTALL_LIB_DIR?=$(INSTALL_DIR)/$(TARGET_LIB_DIR)
INSTALL_INCLUDE_DIR?=$(INSTALL_DIR)/$(TARGET_INCLUDE_DIR)
INSTALL_DOCS_DIR?=$(INSTALL_DIR)/$(TARGET_DOCS_DIR)
INSTALL_SHARE_DIR?=$(INSTALL_DIR)/$(TARGET_SHARE_DIR)
INSTALL_ETC_DIR?=$(INSTALL_DIR)/$(TARGET_ETC_DIR)
INSTALLDEV_BIN_DIR?=$(INSTALL_DIR)/$(TARGET_BIN_DIR)
INSTALLDEV_LIB_DIR?=$(INSTALL_DIR)/$(TARGET_LIB_DIR)
INSTALLDEV_INCLUDE_DIR?=$(INSTALL_DIR)/$(TARGET_INCLUDE_DIR)
INSTALLDEV_DOCS_DIR?=$(INSTALL_DIR)/$(TARGET_DOCS_DIR)
INSTALLDEV_SHARE_DIR?=$(INSTALL_DIR)/$(TARGET_SHARE_DIR)
INSTALLDEV_ETC_DIR?=$(INSTALL_DIR)/$(TARGET_ETC_DIR)
INSTALLDOCSDEV_DOCS_DIR?=$(INSTALL_DIR)/$(TARGET_DOCS_DIR)

PACKAGES_DIR?="$(BUILD_DIR)/packages"

ALL_OUTPUT_DIRS+=$(OUTPUT_LIB_DIR) $(OUTPUT_TOOLS_DIR) $(OUTPUT_TESTS_DIR) $(OUTPUT_DOCS_DIR) $(OUTPUT_EXAMPLES_DIR) $(OUTPUT_OBJ_DIR) $(OUTPUT_GUI_DIR) $(PACKAGES_DIR)

NATIVE_USE_AR?=1
NATIVE_USE_MACOSX_LIBTOOL?=0
