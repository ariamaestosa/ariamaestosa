
.PHONY : ship-all

ship-all : ship ship-dev ship-docs-dev ship-testresults

.PHONY : ship

ship : package
	@echo SHIPPING: package to $(SHIP_TO)
	$(SCP) $(SCP_OPTIONS) $(PACKAGE_PATH) $(SHIP_TO)

.PHONY : ship-dev

ship-dev : package-dev 
	@echo SHIPPING: package-dev to $(SHIP_TO)
	$(SCP) $(SCP_OPTIONS) $(PACKAGE_DEV_PATH) $(SHIP_TO)

.PHONY : ship-docs-dev

ship-docs-dev : package-docs-dev 
	@echo SHIPPING: package-docs-dev to $(SHIP_TO)
	$(SCP) $(SCP_OPTIONS) $(PACKAGE_DOCSDEV_PATH) $(SHIP_TO)


.PHONY : check

check : test

.PHONY : test

.PHONY : ship-testresults

ship-testresults : 

.PHONY : ship-raw-testresults

ship-raw-testresults : 

.PHONY : testresults

testresults : test


ifeq ($(CROSS_COMPILING),1)
test : native-test
else

CLEAN_FILES+=$(foreach f,$(LIB_TESTS_EXE_FILES),$(basename $(f)).err $(basename $(f)).out ) $(foreach f,$(notdir $(LIB_TESTS_SH_FILES)),$(OUTPUT_TESTS_DIR)/$(basename $(f)).sh.err $(OUTPUT_TESTS_DIR)/$(basename $(f)).sh.out )


test: lib tools $(LIB_TESTS_SH_FILES) $(LIB_TESTS_EXE_FILES)
	@cd "$(OUTPUT_TESTS_DIR)"; \
	for i in $(LIB_TESTS_EXE_FILES); \
	do \
	  n=$$(basename $$i); \
	  if $(VALGRIND) $(VALGRIND_OPTIONS) "./$${n}" >"$${n}.out" 2>"$${n}.err"; then echo SUCCESS:$${n}; else echo  FAIL:$${n}; fi;\
	done; \
	for i in $(LIB_TESTS_SH_FILES); \
	do \
	  n=$$(basename $$i); \
	  if /bin/sh "$${i}" >"$${n}.out" 2>"$${n}.err"; then echo SUCCESS:$${n}; else echo  FAIL:$${n}; fi;\
	done

ship-testresults : package-testresults 
	@echo SHIPPING: package-testresults to $(SHIP_TO)
	$(SCP) $(SCP_OPTIONS) $(PACKAGE_TESTRESULTS_PATH) $(SHIP_TO)
endif

