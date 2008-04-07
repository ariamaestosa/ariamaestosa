
# get the list of library source files from the src directory
LIB_CPP_FILES+=$(call get_src_file_list,cpp)
LIB_CC_FILES+=$(call get_src_file_list,cc)
LIB_C_FILES+=$(call get_src_file_list,c)
LIB_M_FILES+=$(call get_src_file_list,m)
LIB_MM_FILES+=$(call get_src_file_list,mm)
LIB_RC_FILES+=$(call get_src_file_list,rc)
LIB_O_FILES+=$(call get_cpp_o_files,$(LIB_CPP_FILES)) \
	$(call get_cc_o_files,$(LIB_CC_FILES))  \
	$(call get_c_o_files,$(LIB_C_FILES)) \
	$(call get_m_o_files,$(LIB_M_FILES)) \
	$(call get_mm_o_files,$(LIB_MM_FILES)) \
	$(call get_rc_o_files,$(LIB_RC_FILES)) 

LIB_DISASM_FILES+=$(LIB_O_FILES:.o=.disasm)
LIB_ASM_FILES+=$(LIB_O_FILES:.o=.asm)


ifeq ($(CROSS_COMPILING),1)
NATIVE_LIB_CPP_FILES+=$(call get_native_src_file_list,cpp)
NATIVE_LIB_CC_FILES+=$(call get_native_src_file_list,cc)
NATIVE_LIB_C_FILE+=$(call get_native_src_file_list,c)
NATIVE_LIB_M_FILES+=$(call get_native_src_file_list,m)
NATIVE_LIB_MM_FILES+=$(call get_native_src_file_list,mm)
NATIVE_LIB_RC_FILES+=$(call get_native_src_file_list,rc)

NATIVE_LIB_O_FILES+=$(call get_native_cpp_o_files,$(NATIVE_LIB_CPP_FILES)) \
	$(call get_native_cc_o_files,$(NATIVE_LIB_CC_FILES)) \
	$(call get_native_c_o_files,$(NATIVE_LIB_C_FILES)) \
	$(call get_native_m_o_files,$(NATIVE_LIB_M_FILES)) \
	$(call get_native_mm_o_files,$(NATIVE_LIB_MM_FILES)) \
	$(call get_native_rc_o_files,$(NATIVE_LIB_RC_FILES)) 
endif

LIB_INCLUDE_FILES+=$(wildcard $(LIB_INCLUDE_DIR)/*.h) 


# get the list of tool program source files from the tools directories

$(eval $(call search_program_group,TOOLS))

# get the list of test program source files from the tests directories

$(eval $(call search_program_group,TESTS))

# get the list of example program source files from the examples directories

$(eval $(call search_program_group,EXAMPLES))

# get the list of example program source files from the gui directories

$(eval $(call search_program_group,GUI))


# if there are no O files to build, do not build or link a lib!

ifeq ($(strip $(LIB_O_FILES)),)
DO_NOT_BUILD_LIB=1
else
DO_NOT_BUILD_LIB=0
endif

ifeq ($(strip $(NATIVE_LIB_O_FILES)),)
NATIVE_DO_NOT_BUILD_LIB=1
else
NATIVE_DO_NOT_BUILD_LIB=0
endif


ifeq ($(DO_NOT_BUILD_LIB),1)
PROJECT_LDLIB=
else
PROJECT_LDLIB=-l$(PROJECT)
endif

ifeq ($(NATIVE_DO_NOT_BUILD_LIB),1)
NATIVE_PROJECT_LDLIB=
else
NATIVE_PROJECT_LDLIB=-l$(PROJECT)
endif

ifeq ($(DO_NOT_BUILD_LIB),1)
OUTPUT_LIB=
else
OUTPUT_LIB?=$(OUTPUT_LIB_DIR)/lib$(PROJECT).a
endif

ifeq ($(DO_NOT_BUILD_LIB),1)
NATIVE_OUTPUT_LIB=
else
NATIVE_OUTPUT_LIB?=$(NATIVE_OUTPUT_LIB_DIR)/lib$(PROJECT).a
endif





