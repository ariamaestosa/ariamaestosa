.PHONY : packages

packages : package package-dev package-docs-dev package-testresults

.PHONY : package

package : preinstall package-$(PACKAGER)

.PHONY : package-dev

package-dev : package preinstall-dev package-dev-$(PACKAGER)

package-docs-dev : package-dev preinstall-docs-dev package-docs-dev-$(PACKAGER)

.PHONY : package-testresults



PACKAGE_EXT=
ifeq ($(PACKAGER),dpkg)
PACKAGE_EXT=deb
endif
ifeq ($(PACKAGER),rpm)
PACKAGE_EXT=rpm
endif
ifeq ($(PACKAGER),macosx)
PACKAGE_EXT=pkg
endif
ifeq ($(PACKAGER),nsis)
PACKAGE_EXT=exe
endif
ifeq ($(PACKAGER),tgz)
PACKAGE_EXT=tgz
endif
ifeq ($(PACKAGER),zip)
PACKAGE_EXT=zip
endif

PACKAGE_PATH=$(PACKAGES_DIR)/$(PACKAGE_BASENAME).$(PACKAGE_EXT)
PACKAGE_DEV_PATH=$(PACKAGES_DIR)/$(PACKAGEDEV_BASENAME).$(PACKAGE_EXT)
PACKAGE_DOCSDEV_PATH=$(PACKAGES_DIR)/$(PACKAGEDOCSDEV_BASENAME).$(PACKAGE_EXT)
PACKAGE_TESTRESULTS_PATH=$(PACKAGES_DIR)/$(PACKAGETESTRESULTS_BASENAME).zip

CLEAN_FILES += $(PACKAGE_PATH) $(PACKAGE_DEV_PATH) $(PACKAGE_DOCSDEV_PATH) $(PACKAGE_TESTRESULTS_PATH)

.PHONY : ship-raw-testresults

ifeq ($(CROSS_COMPILING),1)

package-testresults :

ship-raw-testresults :

else

package-testresults : $(PACKAGE_TESTRESULTS_PATH)

$(PACKAGE_TESTRESULTS_PATH) : test 
	@-( cd $(OUTPUT_TESTS_DIR) && $(ZIP) -r $(PACKAGE_TESTRESULTS_PATH) . >/dev/null )

$(PACKAGES_DIR)/$(PACKAGETESTRESULTS_BASENAME).tgz : test 
	@-( cd $(OUTPUT_TESTS_DIR) && $(TAR) cf - . | $(GZIP) >$(PACKAGES_DIR)/$(PACKAGETESTRESULTS_BASENAME).zip )

ship-raw-testresults : test
	@-( cd $(OUTPUT_TESTS_DIR) && $(SCP) $(SCP_OPTIONS) $(OUTPUT_TESTS_DIR) $(SHIP_TO) )

endif

.PHONY : ship-raw-docs-dev

ship-raw-docs-dev : docs-dev
	@-( cd $(OUTPUT_DOCS_DIR) && $(SCP) $(SCP_OPTIONS) $(OUTPUT_DOCS_DIR) $(SHIP_TO) )

PACKAGE_DEB_M4_DEFINES="-DPROJECT_NAME=$(PROJECT)" \
  "-DPROJECT_LONG_NAME=$(PROJECT_NAME)" \
	"-DPROJECT_VERSION=$(PROJECT_VERSION)" \
	"-DPROJECT_COPYRIGHT=$(PROJECT_COPYRIGHT)" \
	"-DPROJECT_COMPANY=$(PROJECT_COMPANY)" \
	"-DPROJECT_ARCHITECTURE=$(PROJECT_ARCHITECTURE)" \
	"-DPROJECT_MAINTAINER=$(PROJECT_MAINTAINER)" \
	"-DPROJECT_DESCRIPTION=$(PROJECT_DESCRIPTION)" \
	"-DPROJECT_LICENSE_FILE=$(PROJECT_LICENSE_FILE)" \
	"-DPROJECT_WEBSITE=$(PROJECT_WEBSITE)" \
	"-DPROJECT_README_FILE=$(PROJECT_README_FILE)"  

PACKAGEDEV_DEB_M4_DEFINES="-DPROJECT_NAME=$(PROJECT)-dev" \
  "-DPROJECT_LONG_NAME=$(PROJECT_NAME)" \
	"-DPROJECT_VERSION=$(PROJECT_VERSION)" \
	"-DPROJECT_COPYRIGHT=$(PROJECT_COPYRIGHT)" \
	"-DPROJECT_COMPANY=$(PROJECT_COMPANY)" \
	"-DPROJECT_ARCHITECTURE=$(PROJECT_ARCHITECTURE)" \
	"-DPROJECT_MAINTAINER=$(PROJECT_MAINTAINER)" \
	"-DPROJECT_DESCRIPTION=$(PROJECT_DESCRIPTION)" \
	"-DPROJECT_LICENSE_FILE=$(PROJECT_LICENSE_FILE)" \
	"-DPROJECT_WEBSITE=$(PROJECT_WEBSITE)" \
	"-DPROJECT_README_FILE=$(PROJECT_DEV_README_FILE)"  


PACKAGEDOCSDEV_DEB_M4_DEFINES="-DPROJECT_NAME=$(PROJECT)-docs-dev" \
  "-DPROJECT_LONG_NAME=$(PROJECT_NAME)" \
	"-DPROJECT_VERSION=$(PROJECT_VERSION)" \
	"-DPROJECT_COPYRIGHT=$(PROJECT_COPYRIGHT)" \
	"-DPROJECT_COMPANY=$(PROJECT_COMPANY)" \
	"-DPROJECT_ARCHITECTURE=$(PROJECT_ARCHITECTURE)" \
	"-DPROJECT_MAINTAINER=$(PROJECT_MAINTAINER)" \
	"-DPROJECT_DESCRIPTION=$(PROJECT_DESCRIPTION)" \
	"-DPROJECT_LICENSE_FILE=$(PROJECT_LICENSE_FILE)" \
	"-DPROJECT_WEBSITE=$(PROJECT_WEBSITE)" \
	"-DPROJECT_README_FILE=$(PROJECT_DOCS_DEV_README_FILE)"  


PACKAGE_M4_DEFINES="-DPROJECT_NAME=$(PROJECT)" \
  "-DPROJECT_LONG_NAME=$(PROJECT_NAME)" \
	"-DPROJECT_VERSION=$(PROJECT_VERSION)" \
	"-DPROJECT_COPYRIGHT=$(PROJECT_COPYRIGHT)" \
	"-DPROJECT_COMPANY=$(PROJECT_COMPANY)" \
	"-DPROJECT_ARCHITECTURE=$(PROJECT_ARCHITECTURE)" \
	"-DPROJECT_MAINTAINER=$(PROJECT_MAINTAINER)" \
	"-DPROJECT_DESCRIPTION=$(PROJECT_DESCRIPTION)" \
	"-DPROJECT_LICENSE_FILE=$(PROJECT_LICENSE_FILE)" \
	"-DPROJECT_WEBSITE=$(PROJECT_WEBSITE)" \
	"-DPROJECT_README_FILE=$(PROJECT_README_FILE)"  

PACKAGEDEV_M4_DEFINES="-DPROJECT_NAME=$(PROJECT)dev" \
  "-DPROJECT_LONG_NAME=$(PROJECT_NAME)" \
	"-DPROJECT_VERSION=$(PROJECT_VERSION)" \
	"-DPROJECT_COPYRIGHT=$(PROJECT_COPYRIGHT)" \
	"-DPROJECT_COMPANY=$(PROJECT_COMPANY)" \
	"-DPROJECT_ARCHITECTURE=$(PROJECT_ARCHITECTURE)" \
	"-DPROJECT_MAINTAINER=$(PROJECT_MAINTAINER)" \
	"-DPROJECT_DESCRIPTION=$(PROJECT_DESCRIPTION)" \
	"-DPROJECT_LICENSE_FILE=$(PROJECT_LICENSE_FILE)" \
	"-DPROJECT_WEBSITE=$(PROJECT_WEBSITE)" \
	"-DPROJECT_README_FILE=$(PROJECT_DEV_README_FILE)"  


PACKAGEDOCSDEV_M4_DEFINES="-DPROJECT_NAME=$(PROJECT)docsdev" \
  "-DPROJECT_LONG_NAME=$(PROJECT_NAME)" \
	"-DPROJECT_VERSION=$(PROJECT_VERSION)" \
	"-DPROJECT_COPYRIGHT=$(PROJECT_COPYRIGHT)" \
	"-DPROJECT_COMPANY=$(PROJECT_COMPANY)" \
	"-DPROJECT_ARCHITECTURE=$(PROJECT_ARCHITECTURE)" \
	"-DPROJECT_MAINTAINER=$(PROJECT_MAINTAINER)" \
	"-DPROJECT_DESCRIPTION=$(PROJECT_DESCRIPTION)" \
	"-DPROJECT_LICENSE_FILE=$(PROJECT_LICENSE_FILE)" \
	"-DPROJECT_WEBSITE=$(PROJECT_WEBSITE)" \
	"-DPROJECT_README_FILE=$(PROJECT_DOCS_DEV_README_FILE)"  

.PHONY : package-dpkg package-dev-dpkg package-docs-dev-dpkg \
	package-rpm package-dev-rpm package-docs-dev-rpm \
	package-epm package-dev-epm package-docs-dev-epm \
	package-macosx package-dev-macosx package-docs-dev-macosx \
	package-nsis package-dev-nsis package-docs-dev-nsis \
	package-tgz package-dev-tgz package-docs-dev-tgz \
	package-zip package-dev-zip package-docs-dev-zip \
	package-none package-dev-none package-docs-dev-none \
	package- package-dev- package-docs-dev-

package- :
	@echo "no packager set"

package-dev- :
	@echo "no packager set"

package-docs-dev- :
	@echo "no packager set"

package-none :

package-dev-none :

package-docs-dev-none :

package-dpkg : $(PACKAGES_DIR)/$(PACKAGE_BASENAME).deb

$(PACKAGES_DIR)/$(PACKAGE_BASENAME).deb : preinstall
	@echo "PACKAGE-DPKG"
	@$(MKDIR) -p $(LOCAL_INSTALL_DIR)/DEBIAN
	@( for i in control preinst postinst prerm postrm; do $(M4) -P $(PACKAGE_DEB_M4_DEFINES) "$(PROJECT_TOP_DIR)/package/dpkg/$$i" >"$(LOCAL_INSTALL_DIR)/DEBIAN/$$i";  done )
	@( for i in preinst postinst prerm postrm; do chmod +x "$(LOCAL_INSTALL_DIR)/DEBIAN/$$i";  done )
	@$(DPKG) --build $(LOCAL_INSTALL_DIR) $(PACKAGES_DIR)/$(PACKAGE_BASENAME).deb

package-dev-dpkg : $(PACKAGES_DIR)/$(PACKAGEDEV_BASENAME).deb

$(PACKAGES_DIR)/$(PACKAGEDEV_BASENAME).deb : preinstall-dev
	@echo "PACKAGE-DEV-DPKG"
	@$(MKDIR) -p $(LOCAL_INSTALLDEV_DIR)/DEBIAN
	@( for i in control preinst postinst prerm postrm; do $(M4) -P $(PACKAGEDEV_DEB_M4_DEFINES) "$(PROJECT_TOP_DIR)/package/dpkg/$$i" >"$(LOCAL_INSTALLDEV_DIR)/DEBIAN/$$i";  done )
	@( for i in preinst postinst prerm postrm; do chmod +x "$(LOCAL_INSTALLDEV_DIR)/DEBIAN/$$i";  done )
	@$(DPKG) --build $(LOCAL_INSTALLDEV_DIR) $(PACKAGES_DIR)/$(PACKAGEDEV_BASENAME).deb

package-docs-dev-dpkg : $(PACKAGES_DIR)/$(PACKAGEDOCSDEV_BASENAME).deb

$(PACKAGES_DIR)/$(PACKAGEDOCSDEV_BASENAME).deb : preinstall-docs-dev
	@echo "PACKAGE-DOCSDEV-DPKG"
	@$(MKDIR) -p $(LOCAL_INSTALLDOCSDEV_DIR)/DEBIAN
	@( for i in control preinst postinst prerm postrm; do $(M4) -P $(PACKAGEDOCSDEV_DEB_M4_DEFINES) "$(PROJECT_TOP_DIR)/package/dpkg/$$i" >"$(LOCAL_INSTALLDOCSDEV_DIR)/DEBIAN/$$i";  done )
	@( for i in preinst postinst prerm postrm; do chmod +x "$(LOCAL_INSTALLDOCSDEV_DIR)/DEBIAN/$$i";  done )
	@$(DPKG) --build $(LOCAL_INSTALLDOCSDEV_DIR) $(PACKAGES_DIR)/$(PACKAGEDOCSDEV_BASENAME).deb

package-rpm : $(PACKAGES_DIR)/$(PACKAGE_BASENAME).rpm

$(PACKAGES_DIR)/$(PACKAGE_BASENAME).rpm :
	@echo "PACKAGE-RPM not working yet"

package-dev-rpm : $(PACKAGES_DIR)/$(PACKAGEDEV_BASENAME).rpm

$(PACKAGES_DIR)/$(PACKAGEDEV_BASENAME).rpm :
	@echo "PACKAGE-DEV-RPM not working yet"

package-docs-dev-rpm : $(PACKAGES_DIR)/$(PACKAGEDOCSDEV_BASENAME).rpm

$(PACKAGES_DIR)/$(PACKAGEDOCSDEV_BASENAME).rpm :
	@echo "PACKAGE-DOCS-DEV-RPM not working yet"

EPM_FORMAT ?= native

package-epm : preinstall
	@echo "PACKAGE-EPM"
	@$(M4) -P $(PACKAGE_M4_DEFINES) "$(PROJECT_TOP_DIR)/package/epm/epm.txt" >"$(PACKAGE_BASENAME).epmlist"
	@$(MKEPMLIST) -g root -u root --prefix / $(LOCAL_INSTALL_DIR) >>"$(PACKAGE_BASENAME).epmlist"
	@$(EPM) -f $(EPM_FORMAT) -ns $(PROJECT_NAME) "$(PACKAGE_BASENAME).epmlist"

package-dev-epm : preinstall-dev
	@echo "PACKAGEDEV-EPM"
	@$(M4) -P $(PACKAGEDEV_M4_DEFINES) "$(PROJECT_TOP_DIR)/package/epm/epm.txt" >"$(PACKAGEDEV_BASENAME).epmlist"
	@$(MKEPMLIST) -g root -u root --prefix / $(LOCAL_INSTALLDEV_DIR) >>"$(PACKAGEDEV_BASENAME).epmlist"
	@$(EPM) -f $(EPM_FORMAT) -ns $(PROJECT_NAME)dev "$(PACKAGEDEV_BASENAME).epmlist"

package-docs-dev-epm : preinstall-docs-dev
	@echo "PACKAGEDOCSDEV-EPM"
	@$(M4) -P $(PACKAGEDOCSDEV_M4_DEFINES) "$(PROJECT_TOP_DIR)/package/epm/epm.txt" >"$(PACKAGEDOCSDEV_BASENAME).epmlist"
	@$(MKEPMLIST) -g root -u root --prefix / $(LOCAL_INSTALLDOCSDEV_DIR) >>"$(PACKAGEDOCSDEV_BASENAME).epmlist"
	@$(EPM) -f $(EPM_FORMAT) -ns $(PROJECT_NAME)docsdev "$(PACKAGEDOCSDEV_BASENAME).epmlist"

package-macosx : $(PACKAGES_DIR)/$(PACKAGE_BASENAME).pkg

$(PACKAGES_DIR)/$(PACKAGE_BASENAME).pkg :
	@echo "PACKAGE-MACOSX not working yet"

package-dev-macosx :
	@echo "PACKAGE-DEV-MACOSX not working yet"

package-docs-dev-macosx :
	@echo "PACKAGE-DOCS-DEV-MACOSX not working yet"

MAKENSIS=makensis
NSIS_SCRIPT_BASE_NAME=simple
NSIS_SCRIPT=$(PROJECT_TOP_DIR)/package/nsis/$(NSIS_SCRIPT_BASE_NAME).nsi
NSIS_DEV_SCRIPT=$(PROJECT_TOP_DIR)/package/nsis/$(NSIS_SCRIPT_BASE_NAME)-dev.nsi
NSIS_DOCSDEV_SCRIPT=$(PROJECT_TOP_DIR)/package/nsis/$(NSIS_SCRIPT_BASE_NAME)-docsdev.nsi

package-nsis :
	@echo "PACKAGE-NSIS"
	@$(M4) -P $(PACKAGE_M4_DEFINES) $(NSIS_SCRIPT) >"$(PACKAGE_BASENAME).nsi"
	@echo "File /r $(LOCAL_INSTALL_DIR)/*" >nsis-files.txt
	$(MAKENSIS) $(PACKAGE_BASENAME).nsi

package-dev-nsis :
	@echo "PACKAGE-DEV-NSIS"
	@$(M4) -P $(PACKAGEDEV_M4_DEFINES) $(NSIS_DEV_SCRIPT) >"$(PACKAGEDEV_BASENAME).nsi"
	@echo "File /r $(LOCAL_INSTALLDEV_DIR)/*" >nsis-dev-files.txt
	$(MAKENSIS) $(PACKAGEDEV_BASENAME).nsi

package-docs-dev-nsis :
	@echo "PACKAGE-DOCS-DEV-NSIS"
	@$(M4) -P $(PACKAGEDOCSDEV_M4_DEFINES) $(NSIS_DOCSDEV_SCRIPT) >"$(PACKAGEDOCSDEV_BASENAME).nsi"
	@echo "File /r $(LOCAL_INSTALLDOCSDEV_DIR)/*" >nsis-docsdev-files.txt
	$(MAKENSIS) $(PACKAGEDOCSDEV_BASENAME).nsi


package-tgz : $(PACKAGES_DIR)/$(PACKAGE_BASENAME).tgz

$(PACKAGES_DIR)/$(PACKAGE_BASENAME).tgz :
	@echo "PACKAGE-TGZ"
	@( cd $(LOCAL_INSTALL_DIR) && tar cvf - . | gzip >$(PACKAGES_DIR)/$(PACKAGE_BASENAME).tgz )

package-dev-tgz : $(PACKAGES_DIR)/$(PACKAGEDEV_BASENAME).tgz

$(PACKAGES_DIR)/$(PACKAGEDEV_BASENAME).tgz :
	@echo "PACKAGE-DEV-TGZ"
	@( cd $(LOCAL_INSTALLDEV_DIR) && tar cvf - . | gzip >$(PACKAGES_DIR)/$(PACKAGEDEV_BASENAME).tgz )

package-docs-dev-tgz : $(PACKAGES_DIR)/$(PACKAGEDOCSDEV_BASENAME).tgz

$(PACKAGES_DIR)/$(PACKAGEDOCSDEV_BASENAME).tgz :
	@echo "PACKAGE-DOCS-DEV-TGZ"
	@( cd $(LOCAL_INSTALLDOCSDEV_DIR) && tar cvf - . | gzip >$(PACKAGES_DIR)/$(PACKAGEDOCSDEV_BASENAME).tgz )

package-zip : $(PACKAGES_DIR)/$(PACKAGE_BASENAME).zip

$(PACKAGES_DIR)/$(PACKAGE_BASENAME).zip :
	@echo "PACKAGE-ZIP"
	@( cd $(LOCAL_INSTALL_DIR) && zip -r $(PACKAGES_DIR)/$(PACKAGE_BASENAME).zip . )

package-dev-zip : $(PACKAGES_DIR)/$(PACKAGEDEV_BASENAME).zip

$(PACKAGES_DIR)/$(PACKAGEDEV_BASENAME).zip :
	@echo "PACKAGE-DEV-ZIP"
	@( cd $(LOCAL_INSTALLDEV_DIR) && zip -r $(PACKAGES_DIR)/$(PACKAGEDEV_BASENAME).zip . )

package-docs-dev-zip : $(PACKAGES_DIR)/$(PACKAGEDOCSDEV_BASENAME).zip

$(PACKAGES_DIR)/$(PACKAGEDOCSDEV_BASENAME).zip :
	@echo "PACKAGE-DOCS-DEV-ZIP"
	@( cd $(LOCAL_INSTALLDOCSDEV_DIR) && zip -r $(PACKAGES_DIR)/$(PACKAGEDOCSDEV_BASENAME).zip . )

