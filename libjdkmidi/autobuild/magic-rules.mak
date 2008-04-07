##############################################################################################
#
# Make's rules.


# Remove make's built-in rules that we do not want

(%.o) : %.o

%.o : %.cpp

%.o : %.cc

%.o : %.rc

%.o : %.c

%.o : %.m

%.o : %.mm

%$(EXE) : %.cpp

%$(EXE) : %.cc

%$(EXE) : %.c

%$(EXE) : %.m

%$(EXE) : %.mm

COMPILE.cpp=$(CXX) $(CXXFLAGS) -c
COMPILE.cc=$(CXX) $(CXXFLAGS) -c
COMPILE.c=$(CC) $(CFLAGS) -c
COMPILE.mm=$(CXX) $(MMFLAGS) -c
COMPILE.m=$(CC) $(MFLAGS) -c
COMPILE.rc=$(WINDRES) 

LINK.cpp=$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) $(LDLIBS_PACKAGES)
LINK.cc=$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) $(LDLIBS_PACKAGES)
LINK.c=$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $(LDLIBS_PACKAGES)
LINK.mm=$(CXX) $(MMFLAGS) $(LDFLAGS) $(LDLIBS) $(LDLIBS_PACKAGES)
LINK.m=$(CC) $(MFLAGS) $(LDFLAGS) $(LDLIBS) $(LDLIBS_PACKAGES)

NATIVE_COMPILE.cpp=$(NATIVE_CXX) $(NATIVE_CXXFLAGS) -c
NATIVE_COMPILE.cc=$(NATIVE_CX) $(NATIVE_CXXFLAGS) -c
NATIVE_COMPILE.c=$(NATIVE_CC) $(NATIVE_CFLAGS) -c
NATIVE_COMPILE.mm=$(NATIVE_CXX) $(NATIVE_MMFLAGS) -c
NATIVE_COMPILE.m=$(NATIVE_CC) $(NATIVE_MFLAGS) -c
NATIVE_COMPILE.rc=$(NATIVE_WINDRES)

NATIVE_LINK.cpp=$(NATIVE_CXX) $(NATIVE_CXXFLAGS) $(NATIVE_LDFLAGS) $(NATIVE_LDLIBS)
NATIVE_LINK.cc=$(NATIVE_CXX) $(NATIVE_CXXFLAGS) $(NATIVE_LDFLAGS) $(NATIVE_LDLIBS)
NATIVE_LINK.c=$(NATIVE_CC) $(NATIVE_CFLAGS) $(NATIVE_LDFLAGS) $(NATIVE_LDLIBS)
NATIVE_LINK.mm=$(NATIVE_CXX) $(NATIVE_MMFLAGS) $(NATIVE_LDFLAGS) $(NATIVE_LDLIBS)
NATIVE_LINK.m=$(NATIVE_CC) $(NATIVE_MFLAGS) $(NATIVE_LDFLAGS) $(NATIVE_LDLIBS)


##############################################################################################
# 
# Replace make's original rules  with our special rules, which place output objects 
# directly in $(OUTPUT_OBJ_DIR), and also perform include file dependancy file creation.

# For Objective C++:
$(OUTPUT_OBJ_DIR)/%.o $(OUTPUT_OBJ_DIR)/%.d : %.mm
	@echo "CXX mm : $(notdir $<)"
	@$(CXX) $(SORTED_PREPROCESS_FLAGS) $(MMFLAGS) $(DEPENDENCY_OPTIONS) -MT $(OUTPUT_OBJ_DIR)/$*.o -MF $(OUTPUT_OBJ_DIR)/$*.d $< &&	$(COMPILE.cpp) $(SORTED_PREPROCESS_FLAGS) $(COMPILE_FLAGS) -o $(OUTPUT_OBJ_DIR)/$*.o $< 

# For Objective C:
$(OUTPUT_OBJ_DIR)/%.o $(OUTPUT_OBJ_DIR)/%.d : %.m
	@echo "CC  m  : $(notdir $<)"
	@$(CC) $(SORTED_PREPROCESS_FLAGS) $(MFLAGS) $(DEPENDENCY_OPTIONS) -MT  $(OUTPUT_OBJ_DIR)/$*.o -MF $(OUTPUT_OBJ_DIR)/$*.d $< &&	$(COMPILE.c) $(SORTED_PREPROCESS_FLAGS) $(COMPILE_FLAGS) -o $(OUTPUT_OBJ_DIR)/$*.o $<

# For C++: (cpp)
$(OUTPUT_OBJ_DIR)/%.o $(OUTPUT_OBJ_DIR)/%.d : %.cpp
	@echo "CXX    : $(notdir $<)"
	@$(CXX) $(SORTED_PREPROCESS_FLAGS)  $(DEPENDENCY_OPTIONS) -MT $(OUTPUT_OBJ_DIR)/$*.o -MF $(OUTPUT_OBJ_DIR)/$*.d $< && $(COMPILE.cpp) $(SORTED_PREPROCESS_FLAGS) $(COMPILE_FLAGS) -o $(OUTPUT_OBJ_DIR)/$*.o $<

# Asm For C++: (cpp)
$(OUTPUT_OBJ_DIR)/%.asm : %.cpp
	@echo "CXX asm  : $(notdir $<)"
	@$(COMPILE.cc) $(SORTED_PREPROCESS_FLAGS) $(COMPILE_FLAGS) -S -o $(OUTPUT_OBJ_DIR)/$*.asm $<

# For C++: (cc)
$(OUTPUT_OBJ_DIR)/%.o $(OUTPUT_OBJ_DIR)/%.d : %.cc
	@echo "CXX    : $(notdir $<)"
	@$(CXX) $(SORTED_PREPROCESS_FLAGS)  $(DEPENDENCY_OPTIONS) -MT  $(OUTPUT_OBJ_DIR)/$*.o -MF $(OUTPUT_OBJ_DIR)/$*.d $< && $(COMPILE.cc) $(SORTED_PREPROCESS_FLAGS) $(COMPILE_FLAGS) -o $(OUTPUT_OBJ_DIR)/$*.o $<

# Asm For C++: (cc)
$(OUTPUT_OBJ_DIR)/%.asm : %.cc
	@echo "CXX asm  : $(notdir $<)"
	@$(COMPILE.cc) $(SORTED_PREPROCESS_FLAGS) $(COMPILE_FLAGS) -S -o $(OUTPUT_OBJ_DIR)/$*.asm $<

# For C:
$(OUTPUT_OBJ_DIR)/%.o $(OUTPUT_OBJ_DIR)/%.d : %.c
	@echo "CC     : $(notdir $<)"
	@$(CC) $(SORTED_PREPROCESS_FLAGS)  $(DEPENDENCY_OPTIONS) -MT  $(OUTPUT_OBJ_DIR)/$*.o -MF $(OUTPUT_OBJ_DIR)/$*.d $< && $(COMPILE.c) $(SORTED_PREPROCESS_FLAGS) $(COMPILE_FLAGS) -o $(OUTPUT_OBJ_DIR)/$*.o $<

# Asm For C:
$(OUTPUT_OBJ_DIR)/%.asm : %.c
	@echo "C asm  : $(notdir $<)"
	@$(COMPILE.c) $(SORTED_PREPROCESS_FLAGS) $(COMPILE_FLAGS) -S -o $(OUTPUT_OBJ_DIR)/$*.asm $<

# For RC (windows):
$(OUTPUT_OBJ_DIR)/%.o : %.rc
	@echo "WINDRES     : $(notdir $<)"
	@$(COMPILE.rc) -I$(dir $<) $(SORTED_PREPROCESS_FLAGS) $< $@

ifeq ($(TARGET_PLATFORM_MACOSX_UNIVERSAL),1)
# For disassembly of object files for mac 'fat' binaries, i386 and ppc archs
$(OUTPUT_OBJ_DIR)/%.disasm : $(OUTPUT_OBJ_DIR)/%.o
	@echo "DISASM (2)  : $(notdir $<)"
	@echo '# disassembly of ' $(notdir $<) >$(OUTPUT_OBJ_DIR)/$*.disasm
	@for i in $(MACOSX_UNIVERSAL_ARCHS); do \
		echo '#---------- ' $$i >>$(OUTPUT_OBJ_DIR)/$*.disasm; \
		$(LIPO) -thin $$i $< -o $(OUTPUT_OBJ_DIR)/$*.$$i.o && \
		$(DISASM) $(DISASM_FLAGS) $(OUTPUT_OBJ_DIR)/$*.$$i.o >>$(OUTPUT_OBJ_DIR)/$*.disasm; \
		$(RM) $(OUTPUT_OBJ_DIR)/$*.$$i.o; \
	done
else
# For disassembly of object files
$(OUTPUT_OBJ_DIR)/%.disasm : $(OUTPUT_OBJ_DIR)/%.o
	@echo "DISASM      : $(notdir $<)"
	@$(OBJDUMP) $(OBJDUMP_FLAGS) $< >$(OUTPUT_OBJ_DIR)/$*.disasm
endif


ifeq ($(CROSS_COMPILING),1)

$(NATIVE_OUTPUT_OBJ_DIR)/%.o $(NATIVE_OUTPUT_OBJ_DIR)/%.d : %.mm
	@echo "NATIVE_CXX mm : $(notdir $<)"
	@$(NATIVE_CXX) $(SORTED_NATIVE_PREPROCESS_FLAGS) $(DEPENDENCY_OPTIONS) $(NATIVE_OUTPUT_OBJ_DIR)/$*.o -MF $(NATIVE_OUTPUT_OBJ_DIR)/$*.d $< && $(NATIVE_COMPILE.mm) $(SORTED_NATIVE_PREPROCESS_FLAGS) $(NATIVE_COMPILE_FLAGS) -o $(NATIVE_OUTPUT_OBJ_DIR)/$*.o $<

$(NATIVE_OUTPUT_OBJ_DIR)/%.o $(NATIVE_OUTPUT_OBJ_DIR)/%.d : %.m
	@echo "NATIVE_CC  m  : $(notdir $<)"
	@$(NATIVE_CC) $(SORTED_NATIVE_PREPROCESS_FLAGS) $(DEPENDENCY_OPTIONS) -MT $(NATIVE_OUTPUT_OBJ_DIR)/$*.o -MF $(NATIVE_OUTPUT_OBJ_DIR)/$*.d $< && $(NATIVE_COMPILE.m) $(SORTED_NATIVE_PREPROCESS_FLAGS) $(NATIVE_COMPILE_FLAGS) -o $(NATIVE_OUTPUT_OBJ_DIR)/$*.o $<)

$(NATIVE_OUTPUT_OBJ_DIR)/%.o $(NATIVE_OUTPUT_OBJ_DIR)/%.d : %.cpp
	@echo "NATIVE_CXX    : $(notdir $<)"
	@$(NATIVE_CXX) $(SORTED_NATIVE_PREPROCESS_FLAGS)  $(DEPENDENCY_OPTIONS) -MT $(NATIVE_OUTPUT_OBJ_DIR)/$*.o -MF $(NATIVE_OUTPUT_OBJ_DIR)/$*.d $< && $(NATIVE_COMPILE.cpp) $(SORTED_NATIVE_PREPROCESS_FLAGS) $(NATIVE_COMPILE_FLAGS) -o $(NATIVE_OUTPUT_OBJ_DIR)/$*.o $<

$(NATIVE_OUTPUT_OBJ_DIR)/%.o $(NATIVE_OUTPUT_OBJ_DIR)/%.d : %.cc
	@echo "NATIVE_CXX    : $(notdir $<)"
	@$(NATIVE_CXX) $(SORTED_NATIVE_PREPROCESS_FLAGS)  $(DEPENDENCY_OPTIONS) -MT  $(NATIVE_OUTPUT_OBJ_DIR)/$*.o -MF $(NATIVE_OUTPUT_OBJ_DIR)/$*.d $< && $(NATIVE_COMPILE.cpp) $(SORTED_NATIVE_PREPROCESS_FLAGS) $(NATIVE_COMPILE_FLAGS) -o $(NATIVE_OUTPUT_OBJ_DIR)/$*.o $<

$(NATIVE_OUTPUT_OBJ_DIR)/%.o $(NATIVE_OUTPUT_OBJ_DIR)/%.d : %.c
	@echo "NATIVE_CC     : $(notdir $<)"
	@$(NATIVE_CC) $(SORTED_NATIVE_PREPROCESS_FLAGS) $(DEPENDENCY_OPTIONS) -MT $(NATIVE_OUTPUT_OBJ_DIR)/$*.o -MF $(NATIVE_OUTPUT_OBJ_DIR)/$*.d $< && $(NATIVE_COMPILE.c)  $(SORTED_NATIVE_PREPROCESS_FLAGS) $(NATIVE_COMPILE_FLAGS) -o $(NATIVE_OUTPUT_OBJ_DIR)/$*.o $<

$(NATIVE_OUTPUT_OBJ_DIR)/%.o : %.rc
	@echo "NATIVE_WINDRES     : $(notdir $<)"
	@$(NATIVE_COMPILE.rc) -I$(dir $<) $(SORTED_NATIVE_PREPROCESS_FLAGS) $< $@
endif


$(OUTPUT_TOOLS_DIR)/%$(EXE) : $(OUTPUT_OBJ_DIR)/%.o
	@echo "LINKING tool: $(notdir $<)"
	@$(LINK.cpp) $(LINK_FLAGS) $(LDFLAGS) -o $(OUTPUT_TOOLS_DIR)/$*$(EXE) $< -L$(OUTPUT_LIB_DIR) $(PROJECT_LDLIB) $(LDLIBS) $(LDLIBS_PACKAGES)


$(OUTPUT_GUI_DIR)/%$(EXE) : $(OUTPUT_OBJ_DIR)/%.o
	@echo "LINKING gui: $(notdir $<)"
	@$(LINK.cpp) $(LINK_FLAGS) $(LDFLAGS) $(LINK_FLAGS_GUI) -o $(OUTPUT_GUI_DIR)/$*$(EXE) $< -L$(OUTPUT_LIB_DIR) $(PROJECT_LDLIB) $(LDLIBS) $(LDLIBS_PACKAGES) $(LDLIBS_GUI)


$(OUTPUT_EXAMPLES_DIR)/%$(EXE) : $(OUTPUT_OBJ_DIR)/%.o
	@echo "LINKING example: $(notdir $<)"
	@$(LINK.cpp) $(LINK_FLAGS) $(LDFLAGS) -o $(OUTPUT_EXAMPLES_DIR)/$*$(EXE) $< -L$(OUTPUT_LIB_DIR) $(PROJECT_LDLIB) $(LDLIBS) $(LDLIBS_PACKAGES)

$(OUTPUT_TESTS_DIR)/%$(EXE) : $(OUTPUT_OBJ_DIR)/%.o
	@echo "LINKING test: $(notdir $<)"
	@$(LINK.cpp) $(LINK_FLAGS) $(LDFLAGS) -o $(OUTPUT_TESTS_DIR)/$*$(EXE) $< -L$(OUTPUT_LIB_DIR) $(PROJECT_LDLIB) $(LDLIBS) $(LDLIBS_PACKAGES)


ifeq ($(CROSS_COMPILING),1)

$(NATIVE_OUTPUT_TOOLS_DIR)/%$(NATIVE_EXE) : $(NATIVE_OUTPUT_OBJ_DIR)/%.o
	@echo "NATIVE_LINKING tool: $(notdir $<)"
	@$(NATIVE_LINK.cpp) $(NATIVE_LINK_FLAGS) $(NATIVE_LDFLAGS) -o $(NATIVE_OUTPUT_TOOLS_DIR)/$*$(NATIVE_EXE) $< -L$(NATIVE_OUTPUT_LIB_DIR) $(NATIVE_PROJECT_LDLIB) $(NATIVE_LDLIBS)

$(NATIVE_OUTPUT_EXAMPLES_DIR)/%$(NATIVE_EXE) : $(NATIVE_OUTPUT_OBJ_DIR)/%.o
	@echo "NATIVE_LINKING example: $(notdir $<)"
	@$(NATIVE_LINK.cpp) $(NATIVE_LINK_FLAGS) $(NATIVE_LDFLAGS) -o $(NATIVE_OUTPUT_EXAMPLES_DIR)/$*$(NATIVE_EXE) $< -L$(NATIVE_OUTPUT_LIB_DIR) $(NATIVE_PROJECT_LDLIB) $(NATIVE_LDLIBS)

$(NATIVE_OUTPUT_TESTS_DIR)/%$(NATIVE_EXE) : $(NATIVE_OUTPUT_OBJ_DIR)/%.o
	@echo "NATIVE_LINKING test: $(notdir $<)"
	@$(NATIVE_LINK.cpp) $(NATIVE_LINK_FLAGS) $(NATIVE_LDFLAGS) -o $(NATIVE_OUTPUT_TESTS_DIR)/$*$(NATIVE_EXE) $< -L$(NATIVE_OUTPUT_LIB_DIR) $(NATIVE_PROJECT_LDLIB) $(NATIVE_LDLIBS)
endif


