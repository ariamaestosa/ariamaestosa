
QT4_LDLIBS = -L$(QTDIR)/lib $(addprefix -l,$(QTLIBS))

ifeq ($(TARGET_PLATFORM_MACOSX),1)
QT4_FRAMEWORKDIR?=/Library/Frameworks
COMPILE_FLAGS+=
INCLUDES+=$(foreach lib,$(QTLIBS),$(QT4_FRAMEWORKDIR)/$(lib).framework/Headers)
QT4_LDLIBS= $(addprefix -framework ,$(QTLIBS)) #-F $(QT4_FRAMEWORKDIR)
endif


QTLIBS+=QtCore

MOC?=$(QTDIR)/bin/moc
UIC?=$(QTDIR)/bin/uic
RCC?=$(QTDIR)/bin/rcc

QT4_INCLUDE_DIR?=$(QTDIR)/include
INCLUDES += $(OUTPUT_OBJ_DIR) $(QT4_INCLUDE_DIR) $(addprefix $(QT4_INCLUDE_DIR)/,$(QTLIBS))
LDLIBS+=$(QT4_LDLIBS)


vpath moc_h_%.cpp : $(OUTPUT_OBJ_DIR)
vpath moc_cpp_%.cpp : $(OUTPUT_OBJ_DIR)
vpath qrc_%.cpp : $(OUTPUT_OBJ_DIR)
vpath ui_%.h : $(OUTPUT_OBJ_DIR)
vpath %.ui : $(LIB_SRC_DIR) $(LIB_TESTS_DIR) $(LIB_EXAMPLES_DIR) $(LIB_TOOLS_DIR) $(LIB_GUI_DIR)
vpath %.qrc : $(LIB_SRC_DIR) $(LIB_TESTS_DIR) $(LIB_EXAMPLES_DIR) $(LIB_TOOLS_DIR) $(LIB_GUI_DIR)
vpath %.h : $(call subdirs_in_path,$(LIB_INCLUDE_DIR)) $(LIB_INCLUDE_DIR)

$(OUTPUT_OBJ_DIR)/moc_h_%.cpp: %.h
	@echo MOC: $<
	@$(MOC) $(addprefix -D,$(DEFINES)) $(addprefix -I,$(INCLUDES)) $< -o $@

$(OUTPUT_OBJ_DIR)/moc_cpp_%.cpp: %.cpp
	@echo MOC: $<
	@$(MOC) $(addprefix -D,$(DEFINES)) $(addprefix -I,$(INCLUDES)) $< -o $@

$(OUTPUT_OBJ_DIR)/qrc_%.cpp: %.qrc
	@echo RCC: $<
	@$(RCC) -o $@ $< 

$(OUTPUT_OBJ_DIR)/ui_%.h: %.ui
	@echo UIC: $<
	@$(UIC) $< -o $@

LIB_H_FILES+=$(call get_include_file_list,h)
LIB_H_FILES_WITH_CPP_SUFFIX=$(notdir $(LIB_H_FILES:.h=.cpp))
LIB_MOC_CPP_FILES+=$(addprefix $(OUTPUT_OBJ_DIR)/moc_h_,$(LIB_H_FILES_WITH_CPP_SUFFIX)) $(addprefix $(OUTPUT_OBJ_DIR)/moc_cpp_,$(LIB_CPP_FILES))
LIB_MOC_O_FILES+=$(LIB_MOC_CPP_FILES:.cpp=.o)

LIB_UI_FILES+=$(notdir $(call get_src_file_list,ui))
LIB_UIC_H_FILES+=$(addprefix $(OUTPUT_OBJ_DIR)/ui_,$(LIB_UI_FILES:.ui=.h))

LIB_QRC_FILES+=$(notdir $(call get_src_file_list,qrc))
LIB_QRC_CPP_FILES+=$(addprefix $(OUTPUT_OBJ_DIR)/qrc_,$(LIB_QRC_FILES:.qrc=.cpp))
LIB_QRC_O_FILES+=$(LIB_QRC_CPP_FILES:.cpp=.o)

LIB_SRC_FILES+=$(LIB_QRC_CPP_FILES) $(LIB_MOC_CPP_FILES)

dump-qt4-info: 
	@echo LIB_H_FILES: $(LIB_H_FILES)
	@echo LIB_UI_FILES: $(LIB_UI_FILES)
	@echo LIB_UIC_H_FILES: $(LIB_UIC_H_FILES)
	@echo LIB_QRC_FILES: $(LIB_QRC_FILES)
	@echo LIB_QRC_CPP_FILES: $(LIB_QRC_CPP_FILES)
	@echo LIB_MOC_CPP_FILES: $(LIB_MOC_CPP_FILES)
	@echo LDLIBS: $(LDLIBS)
	@echo INCLUDES: $(INCLUDES)
	@echo LIB_MOC_O_FILES $(LIB_MOC_O_FILES)
	@echo LIB_QRC_O_FILES $(LIB_QRC_O_FILES)
	@echo LIB_O_FILES: $(LIB_O_FILES)

.PHONY : prelib

prelib :  $(LIB_UIC_H_FILES) $(LIB_MOC_CPP_FILES) $(LIB_QRC_CPP_FILES) 

LIB_O_FILES += $(LIB_MOC_O_FILES) $(LIB_QRC_O_FILES)

