# this Makefile is provided as just a handy, generic build tool. 
# It makes a dir $(BUILDDIR), does a generic configure in it, and 
# reflects the all, clean, test, docs, asm targets to it

BUILDDIR=tmp

all : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) 

clean : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) clean

realclean : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) realclean

test : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) test

tests : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) tests

check : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) check

packages : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) packages

package : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) package

package-dev : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) package-dev

package-docs-dev : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) package-docs-dev

install : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) install

install-dev : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) install-dev

install-docs : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) install-docs

install-docs-dev : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) install-docs-dev

docs : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) docs

docs-dev : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) docs-dev

asm : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) asm

disasm : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) disasm

$(BUILDDIR) :
	-mkdir $(BUILDDIR)

.PHONY : $(BUILDDIR)/GNUmakefile

$(BUILDDIR)/GNUmakefile : $(BUILDDIR)
	(cd $(BUILDDIR) && ../configure $(CONFIGURE_FLAGS) )

