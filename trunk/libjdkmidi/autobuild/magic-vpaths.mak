
##############################################################################################
#
# our vpaths
#

# all o files in $(OUTPUT_OBJ_DIR)
vpath %.o $(OUTPUT_OBJ_DIR)

# all source files in all of our src,tests,ecxamples,tools,gui dirs
vpath %.m $(LIB_SRC_DIR) $(LIB_TESTS_DIR) $(LIB_EXAMPLES_DIR) $(LIB_TOOLS_DIR) $(LIB_GUI_DIR)
vpath %.mm $(LIB_SRC_DIR) $(LIB_TESTS_DIR) $(LIB_EXAMPLES_DIR) $(LIB_TOOLS_DIR) $(LIB_GUI_DIR)
vpath %.cpp $(LIB_SRC_DIR) $(LIB_TESTS_DIR) $(LIB_EXAMPLES_DIR) $(LIB_TOOLS_DIR) $(LIB_GUI_DIR)
vpath %.cc $(LIB_SRC_DIR) $(LIB_TESTS_DIR) $(LIB_EXAMPLES_DIR) $(LIB_TOOLS_DIR) $(LIB_GUI_DIR)
vpath %.c $(LIB_SRC_DIR) $(LIB_TESTS_DIR) $(LIB_EXAMPLES_DIR) $(LIB_TOOLS_DIR) $(LIB_GUI_DIR)
vpath %.rc $(LIB_SRC_DIR) $(LIB_TESTS_DIR) $(LIB_EXAMPLES_DIR) $(LIB_TOOLS_DIR) $(LIB_GUI_DIR)

# all h files in our include dirs
vpath %.h .:$(LIB_INCLUDE_DIR)

# all libraries in our OUTPUT_LIB_DIR
vpath %.a $(OUTPUT_LIB_DIR)
vpath %.so $(OUTPUT_LIB_DIR)
vpath %.dylib $(OUTPUT_LIB_DIR)

# all testing shell scripts in our tests source dir
vpath %.sh $(LIB_TESTS_DIR)

# all object files are precious. Make should not delete them even if they are intermediate
.PRECIOUS : $(OUTPUT_OBJ_DIR)/%.o

# If we are cross compiling, then we must also look in our native dirs for objects and libraries.
ifeq ($(CROSS_COMPILING),1)
vpath %.o $(NATIVE_OUTPUT_OBJ_DIR)
vpath %.a $(NATIVE_OUTPUT_LIB_DIR)
.PRECIOUS : $(NATIVE_OUTPUT_OBJ_DIR)/%.o
endif


