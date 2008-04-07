# RSYNC is the name of our rsync program.
RSYNC?=rsync

# RSYNC_OPTIONS are the rsync options we use to copy a tree of files/directories to another dir.
# We want ot exclude backup files and subversion and CVS control directories.
RSYNC_OPTIONS?=-a --exclude='*~' --exclude='.svn' --exclude='CVS'

# When we build a config tool script, this is the file name we use. 
PROJECT_CONFIG_TOOL?=$(PROJECT)-config

OBJDUMP?=$(COMPILER_PREFIX)objdump
OBJDUMP_FLAGS?=-d -C
LIPO?=$(COMPILER_PREFIX)lipo
ZIP?=zip
GZIP?=gzip
BZIP2?=bzip2
DPKG?=dpkg
M4?=m4
RPMBUILD?=rpmbuild
EPM?=epm
MKEPMLIST?=mkepmlist
TAR?=tar
DOXYGEN?=doxygen
VALGRIND?=valgrind
SCP?=scp
SCP_OPTIONS?=-rp
SHIP_TO?=.
QMAKE?=qmake
MKDIR?=mkdir
MKDIRS?=mkdir -p
RMDIRS?=rm -r -f
RM?=rm -f
INSTALL=$(RSYNC) $(RSYNC_OPTIONS)
