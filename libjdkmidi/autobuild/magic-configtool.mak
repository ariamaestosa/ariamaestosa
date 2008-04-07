# config tool generation options

# CONFIG_TOOL_FILE is the full path name of the generated config tool script
CONFIG_TOOL_FILE=$(OUTPUT_TOOLS_DIR)/$(PROJECT_CONFIG_TOOL)

# CONFIG_TOOL_PREPROCESS_FLAGS is the preprocessor flags that the config tool script 
# will output when given --cppflags
CONFIG_TOOL_PREPROCESS_FLAGS+=$(sort $(addprefix -I,$(TARGET_INSTALLDEV_INCLUDE_DIR)) $(addprefix -D,$(DEFINES)))

# CONFIG_TOOL_COMPILE_FLAGS is the full compile flags that the config tool script 
# will output when given --cflags or --cxxflags
CONFIG_TOOL_COMPILE_FLAGS+=$(sort $(WARNINGS) $(OPTIMIZE_FLAGS) $(CONFIG_TOOL_PREPROCESS_FLAGS) $(COMPILE_FLAGS))


PREPROCESS_FLAGS+=-DCONFIG_INSTALL_DIR=\"$(TARGET_INSTALL_DIR)\" \
	-DCONFIG_BIN_DIR=\"$(TARGET_BIN_DIR)\" \
	-DCONFIG_LIB_DIR=\"$(TARGET_LIB_DIR)\" \
	-DCONFIG_DOCS_DIR=\"$(TARGET_DOCS_DIR)\" \
	-DCONFIG_SHARE_DIR=\"$(TARGET_SHARE_DIR)\" \
	-DCONFIG_ETC_DIR=\"$(TARGET_ETC_DIR)\" 

ifeq ($(CROSS_COMPILING),1)
NATIVE_PREPROCESS_FLAGS+=-DCONFIG_INSTALL_DIR=\"$(NATIVE_OUTPUT_DIR)\" \
	-DCONFIG_BIN_DIR=\"bin\" \
	-DCONFIG_LIB_DIR=\"lib\" \
	-DCONFIG_DOCS_DIR=\"docs\" \
	-DCONFIG_SHARE_DIR=\"share\" \
	-DCONFIG_ETC_DIR=\"etc\" 
endif

.PHONY : config-tool

config-tool : $(CONFIG_TOOL_FILE)

$(CONFIG_TOOL_FILE) :
	@-rm -f $(CONFIG_TOOL_FILE)
	@echo '#!/bin/bash' >$(CONFIG_TOOL_FILE)
	@echo 'for i in $$*; do' >>$(CONFIG_TOOL_FILE)
	@echo ' case $$i in ' >>$(CONFIG_TOOL_FILE)
	@echo '  (--help) echo "Usage: --help --version --prefix --compiler-prefix --includes --defines --libs --cflags --cxxflags --cppflags --mflags --mmflags --cc --cxx --ldflags " ;;' >>$(CONFIG_TOOL_FILE)
	@echo '  (--version) echo "$(PROJECT_NAME) version $(PROJECT_VERSION) config tool. " ;;' >>$(CONFIG_TOOL_FILE)
	@echo '  (--prefix) echo -n "$(TARGET_INSTALL_DIR) " ;;' >>$(CONFIG_TOOL_FILE)	
	@echo '  (--libs) echo -n "$(sort -L$(TARGET_INSTALLDEV_LIB_DIR) $(LDLIBS) $(PROJECT_LDLIB) $(LDLIBS_PACKAGES)) ";;' >>$(CONFIG_TOOL_FILE)
	@echo '  (--defines) echo -n "$(addprefix -D,$(DEFINES)) ";;' >>$(CONFIG_TOOL_FILE)
	@echo '  (--includes) echo -n "$(addprefix -I,$(TARGET_INSTALLDEV_INCLUDE_DIR)) ";;' >>$(CONFIG_TOOL_FILE)
	@echo '  (--cflags) echo -n "$(CFLAGS) $(CONFIG_TOOL_COMPILE_FLAGS) ";;' >>$(CONFIG_TOOL_FILE)
	@echo '  (--cxxflags) echo -n "$(CXXFLAGS) $(CONFIG_TOOL_COMPILE_FLAGS) ";;' >>$(CONFIG_TOOL_FILE)
	@echo '  (--cppflags) echo -n "$(CONFIG_TOOL_PREPROCESS_FLAGS) ";;' >>$(CONFIG_TOOL_FILE)
	@echo '  (--mflags) echo -n "$(MFLAGS) $(CONFIG_TOOL_COMPILE_FLAGS) ";;' >>$(CONFIG_TOOL_FILE)
	@echo '  (--mmflags) echo -n "$(MMFLAGS) $(CONFIG_TOOL_COMPILE_FLAGS) ";;' >>$(CONFIG_TOOL_FILE)
	@echo '  (--cc) echo -n "$(CC) ";;' >>$(CONFIG_TOOL_FILE)
	@echo '  (--cxx) echo -n "$(CXX) ";;' >>$(CONFIG_TOOL_FILE)
	@echo '  (--ldflags) echo -n "$(sort $(LDFLAGS)) ";;' >>$(CONFIG_TOOL_FILE)
	@echo '  (--compiler-prefix) echo -n "$(COMPILER_PREFIX)";;' >>$(CONFIG_TOOL_FILE)
	@echo ' esac' >>$(CONFIG_TOOL_FILE)
	@echo 'done' >>$(CONFIG_TOOL_FILE)
	@echo 'echo' >>$(CONFIG_TOOL_FILE)
	@chmod +x $(CONFIG_TOOL_FILE)

