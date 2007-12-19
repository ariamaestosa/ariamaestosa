# this Makefile is provided as just a handy, generic build tool. 
# It makes a dir $(BUILDDIR), does a generic configure in it, and 
# reflects the all, clean, test, docs, asm targets to it

BUILDDIR=tmp

all : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) 

clean : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) clean

test : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) test

docs : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) docs

asm : $(BUILDDIR)/GNUmakefile
	cd $(BUILDDIR) && $(MAKE) && $(MAKE) asm

$(BUILDDIR) :
	-mkdir $(BUILDDIR)

.PHONY : $(BUILDDIR)/GNUmakefile

$(BUILDDIR)/GNUmakefile : $(BUILDDIR)
	(cd $(BUILDDIR) && ../configure $(CONFIGURE_FLAGS) )

