##############################################################################################
#
# The magic.makefile Copyright 2004-2007 by Jeff Koftinoff <jeffk@jdkoftinoff.com>
# version 5.
#
# Simplifies the building of a c/c++ library, it's tests, tools, examples, and documentation.
#
# See http://opensource.jdkoftinoff.com/jdks/trac/wiki/MagicMakefileV5
# for more information, including license information (GPL). Note that this tool being GPL does
# NOT mean that it can only be used to build GPL projects.
#

MAGICMAKE_DIR=$(PROJECT_TOP_DIR)/autobuild

include $(MAGICMAKE_DIR)/magic-project.mak
include $(MAGICMAKE_DIR)/magic-tools.mak
ifeq ($(QMAKE_MODE),1)
include $(MAGICMAKE_DIR)/magic-qmake-mode.mak
endif
ifeq ($(WX_MODE),1)
include $(MAGICMAKE_DIR)/magic-wx-mode.mak
endif
include $(MAGICMAKE_DIR)/magic-utils.mak
include $(MAGICMAKE_DIR)/magic-options.mak
include $(MAGICMAKE_DIR)/magic-platform.mak
include $(MAGICMAKE_DIR)/magic-dirs.mak
ifeq ($(GTK_MODE),1)
include $(MAGICMAKE_DIR)/magic-gtk.mak
endif
include $(MAGICMAKE_DIR)/magic-vpaths.mak
include $(MAGICMAKE_DIR)/magic-rules.mak
include $(MAGICMAKE_DIR)/magic-firsttarget.mak
include $(MAGICMAKE_DIR)/magic-configtool.mak
include $(MAGICMAKE_DIR)/magic-native.mak
include $(MAGICMAKE_DIR)/magic-filelist.mak
ifeq ($(TARGET_PLATFORM_MACOSX),1)
include $(MAGICMAKE_DIR)/magic-macosx.mak
endif
ifeq ($(QT4_MODE),1)
include $(MAGICMAKE_DIR)/magic-qt4-mode.mak
endif
ifeq ($(TARGET_PLATFORM_WIN32),1)
include $(MAGICMAKE_DIR)/magic-win32gui.mak
endif
include $(MAGICMAKE_DIR)/magic-targets.mak
include $(MAGICMAKE_DIR)/magic-install.mak
include $(MAGICMAKE_DIR)/magic-package.mak
include $(MAGICMAKE_DIR)/magic-ship.mak
include $(MAGICMAKE_DIR)/magic-help.mak
include $(MAGICMAKE_DIR)/magic-custom.mak

ifeq ($(MAKECMDGOALS),clean)
GOALISCLEAN=1
endif
ifeq ($(MAKECMDGOALS),realclean)
GOALISCLEAN=1
endif
ifeq ($(MAKECMDGOALS),distclean)
GOALISCLEAN=1
endif

ifneq ($(GOALISCLEAN),1)
include $(MAGICMAKE_DIR)/magic-deps.mak
endif
