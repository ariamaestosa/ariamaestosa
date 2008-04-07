
# include any dependencies created during the last make.
-include $(OUTPUT_OBJ_DIR)/*.d

ifeq ($(CROSS_COMPILING),1)
-include $(NATIVE_OUTPUT_OBJ_DIR)/*.d
endif
