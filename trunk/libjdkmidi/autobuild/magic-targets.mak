.PHONY : all

ifdef ALL
all : $(ALL)
else
ifeq ($(CROSS_COMPILING)$(BUILD_NATIVE),11)
all : native-dirs dirs native-lib native-tools native-examples native-tests dirs lib tools tests examples gui 
else
all : dirs lib tools tests examples gui
endif
endif

.PHONY : dirs

dirs :
	-@$(MKDIR) -p $(ALL_OUTPUT_DIRS)

.PHONY : prelib

prelib :

.PHONY : lib

ifeq ($(DO_NOT_BUILD_LIB),0)
lib : dirs prelib $(OUTPUT_LIB)

$(OUTPUT_LIB) : $(LIB_O_FILES) 
ifeq ($(TARGET_USE_AR),1)
	@echo "AR     : $(notdir $@)($(notdir $?))"
	@echo "$(AR) $(ARFLAGS) $@ $?"
	@$(AR) $(ARFLAGS) $@ $?
	@$(RANLIB) $@
endif
ifeq ($(TARGET_USE_MACOSX_LIBTOOL),1)
	@echo "LIBTOOL: $(notdir $@)($(notdir $?))"
#	@echo $(MACOSX_LIBTOOL) $(MACOSX_LIBTOOLFLAGS) -o $@ $^
	@$(MACOSX_LIBTOOL) $(MACOSX_LIBTOOLFLAGS) -o $@ $^
endif
else
lib : dirs
endif


.PHONY : tools

tools : lib $(LIB_TOOLS_EXE_FILES) config-tool

$(LIB_TOOLS_EXE_FILES) : $(OUTPUT_LIB)

.PHONY : examples

examples: lib $(LIB_EXAMPLES_EXE_FILES)

$(LIB_EXAMPLES_EXE_FILES) : $(OUTPUT_LIB)

.PHONY : tests

tests: lib $(LIB_TESTS_EXE_FILES)

$(LIB_TESTS_EXE_FILES) : $(OUTPUT_LIB)

.PHONY : gui

gui: lib $(LIB_GUI_EXE_FILES)

$(LIB_GUI_EXE_FILES) : $(OUTPUT_LIB)


.PHONY : docs

docs : lib 
	@echo "DOCS : in $(LIB_DOCS_DIR)"

.PHONY : docs-dev

docs-dev : $(LIB_DOCS_DEV_DIR)/Doxyfile
	@echo "DOCS-DEV :"
	@( export TOP="$(PROJECT_TOP_DIR)"; export PROJECT="$(PROJECT)"; export PROJECT_VERSION=$(PROJECT_VERSION); cd "$(OUTPUT_DOCS_DEV_DIR)" && for i in $(LIB_DOCS_DEV_DIR); do $(DOXYGEN) $$i/Doxyfile; done )


.PHONY : clean

clean :
	-@$(RM) -r -f $(LIB_TESTS_O_FILES) $(LIB_EXAMPLES_O_FILES) $(LIB_TOOLS_O_FILES) $(LIB_O_FILES) $(OUTPUT_OBJ_DIR)/*.d 2>/dev/null
	-@$(RM) -r -f $(LIB_TESTS_EXE_FILES) $(LIB_EXAMPLES_EXE_FILES) $(LIB_TOOLS_EXE_FILES) $(LIB_GUI_EXE_FILES) 2>/dev/null
	-@$(RM) -r -f $(OUTPUT_LIB) $(CLEAN_DIRS) $(CLEAN_FILES) 2>/dev/null
ifeq ($(CROSS_COMPILING),1)
	-@$(RM) -r -f $(NATIVE_LIB_TESTS_O_FILES) $(NATIVE_LIB_EXAMPLES_O_FILES) $(NATIVE_LIB_TOOLS_O_FILES) $(NATIVE_LIB_O_FILES) $(NATIVE_OUTPUT_OBJ_DIR)/*.d 2>/dev/null
	-@$(RM) -r -f $(NATIVE_LIB_TESTS_EXE_FILES) $(NATIVE_LIB_EXAMPLES_EXE_FILES) $(NATIVE_LIB_TOOLS_EXE_FILES) 2>/dev/null
	-@$(RM) -r -f $(NATIVE_OUTPUT_LIB) $(NATIVE_CLEAN_DIRS) $(NATIVE_CLEAN_FILES) 2>/dev/null
endif

.PHONY : realclean

realclean : distclean

.PHONY : distclean

distclean : clean
	-@$(RM) $(OUTPUT_LIB) 2>/dev/null
ifeq ($(CROSS_COMPILING),1)
	-@$(RM) $(NATIVE_OUTPUT_LIB) 2>/dev/null
endif

TAGS : $(LIB_CPP_FILES) $(LIB_CC_FILES) $(LIB_C_FILES) $(LIB_H_FILES)
	@echo "TAGS:"
	@etags --language c++ $(shell find $(LIB_INCLUDE_DIR) $(LIB_SRC_DIR) $(LIB_TESTS_DIR) $(LIB_GUI_DIR) $(LIB_EXAMPLES_DIR) $(LIB_TOOLS_DIR) \( -name "*.cpp" -or -name "*.cc" -or -name "*.h" \) )

tags : $(LIB_CPP_FILES) $(LIB_CC_FILES) $(LIB_C_FILES) $(LIB_H_FILES)
	@echo "tags:"
	@ctags --language c++ $(shell find $(LIB_INCLUDE_DIR) $(LIB_SRC_DIR) $(LIB_TESTS_DIR) $(LIB_GUI_DIR) $(LIB_EXAMPLES_DIR) $(LIB_TOOLS_DIR) \( -name "*.cpp" -or -name "*.cc" -or -name "*.h" \) )

ifeq ($(CROSS_COMPILING),1)
.PHONY : native-dirs

native-dirs :
	-@$(MKDIR) -p $(NATIVE_ALL_OUTPUT_DIRS)

.PHONY : native-lib

ifeq ($(NATIVE_DO_NOT_BUILD_LIB),0)
native-lib : native-dirs $(NATIVE_OUTPUT_LIB)


$(NATIVE_OUTPUT_LIB) : $(NATIVE_LIB_O_FILES) 
ifeq ($(NATIVE_USE_AR),1)
	@echo "NATIVE_AR     : $(notdir $@)($(notdir $?))"
	@$(NATIVE_AR) $(NATIVE_ARFLAGS) $@ $? >/dev/null
	@$(NATIVE_RANLIB) $@
endif
ifeq ($(NATIVE_USE_MACOSX_LIBTOOL),1)
	@echo "NATIVE_LIBTOOL: $(notdir $@)($(notdir $?))"
#	@echo $(NATIVE_MACOSX_LIBTOOL) $(NATIVE_MACOSX_LIBTOOLFLAGS) -o $@ $^
	@$(NATIVE_MACOSX_LIBTOOL) $(NATIVE_MACOSX_LIBTOOLFLAGS) -o $@ $^
endif
endif

.PHONY : native-tools

native-tools : native-lib $(NATIVE_LIB_TOOLS_EXE_FILES)

$(NATIVE_LIB_TOOLS_EXE_FILES) : $(NATIVE_OUTPUT_LIB)

.PHONY : native-examples

native-examples: native-lib $(NATIVE_LIB_EXAMPLES_EXE_FILES)

$(NATIVE_LIB_EXAMPLES_EXE_FILES) : $(NATIVE_OUTPUT_LIB)

.PHONY : native-tests

native-tests: native-lib $(NATIVE_LIB_TESTS_EXE_FILES)

$(NATIVE_LIB_TESTS_EXE_FILES) : $(NATIVE_OUTPUT_LIB)

.PHONY : native-test

native-test: native-lib native-tools $(LIB_TESTS_SH_FILES) $(NATIVE_LIB_TESTS_EXE_FILES)
	@cd "$(NATIVE_OUTPUT_TESTS_DIR)"; \
	for i in $(NATIVE_LIB_TESTS_EXE_FILES); \
	do \
	  n=$$(basename $$i); \
	  if $(VALGRIND) $(VALGRIND_OPTIONS) "./$${n}" >"$${n}.out" 2>"$${n}.err"; then echo SUCCESS:$${n}; else echo  FAIL:$${n}; fi;\
	done; \
	for i in $(LIB_TESTS_SH_FILES); \
	do \
	  n=$$(basename $$i); \
	  if bash "$${i}" >"$${n}.out" 2>"$${n}.err"; then echo SUCCESS:$${n}; else echo  FAIL:$${n}; fi;\
	done

endif

.PHONY : disasm

disasm :  $(LIB_DISASM_FILES) $(LIB_EXAMPLES_DISASM_FILES) $(LIB_TOOLS_DISASM_FILES) $(LIB_TESTS_DISASM_FILES)

.PHONY : asm

asm :  $(LIB_ASM_FILES) 



