
# compile options

ifeq ($(RELEASE),1)
OPTIMIZE?=1
endif

# having OPTIMIZE set to 1 means we compile with -O3
ifeq ($(OPTIMIZE),1)
OPTIMIZE_FLAGS+=-O3
endif

# having DEBUG set to 1 means we compile with -g and define DEBUG=1

ifeq ($(DEBUG),1)
COMPILE_FLAGS+=-g
DEFINES+=DEBUG=1
endif

# having PROFILE set to 1 means we compile with -pg 

ifeq ($(PROFILE),1)
COMPILE_FLAGS+=-pg
endif

#COMPILE_FLAGS_PACKAGES:=$(foreach pkg,$(PKGCONFIG_PACKAGES),$(shell pkg-config $(pkg) --cflags)) $(foreach pkg,$(CONFIG_TOOLS),$(shell $(pkg) $(CONFIG_TOOLS_OPTIONS) --cflags))

INCLUDES_PACKAGES:=$(foreach pkg,$(PKGCONFIG_PACKAGES),$(shell pkg-config $(pkg) --cflags)) $(foreach pkg,$(CONFIG_TOOLS),$(shell $(pkg) $(CONFIG_TOOLS_OPTIONS) --cflags))

COMPILE_FLAGS+=$(COMPILE_FLAGS_PACKAGES)

# all our */include directories
INCLUDES+=$(LIB_INCLUDE_DIR)

# and any destination prefix directories afterwards

ifdef PREFIX
INCLUDES+="$(PREFIX)/include"
endif

ifdef TARGET_INSTALL_DIR
INCLUDES+="$(TARGET_INSTALL_DIR)/include"
endif


# our compiler defines
DEFINES?=

# The preprocessor flags is initially comprised of -I option for each include dir, 
# and the -D option for each define
PREPROCESS_FLAGS+=$(addprefix -I,$(INCLUDES)) $(addprefix -D,$(DEFINES)) $(INCLUDES_PACKAGES) $(COMPILE_FLAGS_PACKAGES)

SORTED_PREPROCESS_FLAGS=$(sort $(PREPROCESS_FLAGS))

# The compiler flag settings for optimization and warnings
COMPILE_FLAGS+=$(OPTIMIZE_FLAGS) $(WARNINGS)

# the additional linker flags:
LDFLAGS+=

# the addition linker libraries:

LDLIBS_PACKAGES:=$(foreach pkg,$(PKGCONFIG_PACKAGES),$(shell pkg-config $(pkg) --libs)) $(foreach pkg,$(CONFIG_TOOLS),$(shell $(pkg) $(COINFIG_TOOLS_OPTIONS) --libs))

LDLIBS+=

# The preprocessor flags needed to generate dependency information:
DEPENDENCY_OPTIONS?=-MM 

ifeq ($(NATIVE_OPTIMIZE),1)
NATIVE_OPTIMIZE_FLAGS+=-O3
endif

# native platform debug flags: having NATIVE_DEBUG set to 1 means we compile native platform
# code with -g and define DEBUG=1
ifeq ($(NATIVE_DEBUG),1)
NATIVE_COMPILE_FLAGS+=-g
NATIVE_DEFINES+=DEBUG=1
endif

# native platform profile flag: having NATIVE_PROFILE set to 1 means we compile native platform
# code with -pg 
ifeq ($(NATIVE_PROFILE),1)
NATIVE_COMPILE_FLAGS+=-pg
endif


# The native platform preprocessor flags is initially comprised of -I option for each include dir, 
# and the -D option for each define
NATIVE_PREPROCESS_FLAGS=$(addprefix -I,$(INCLUDES)) $(addprefix -D,$(NATIVE_DEFINES)) 
SORTED_NATIVE_PREPROCESS_FLAGS=$(sort $(NATIVE_PREPROCESS_FLAGS))

# The native platform compiler flags settings for optimization and warnings
NATIVE_COMPILE_FLAGS+=$(NATIVE_OPTIMIZE_FLAGS) $(NATIVE_WARNINGS)

# the additional linker flags:
NATIVE_LDFLAGS+=

# the addition linker libraries:
NATIVE_LDLIBS+=



