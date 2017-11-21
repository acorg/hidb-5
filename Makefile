# -*- Makefile -*-
# Eugene Skepner 2017
# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

TARGETS = \
	$(DIST)/hidb-make \
	$(DIST)/hidb5-stat \
	$(DIST)/hidb5-find

HIDB_MAKE_SOURCES = hidb-maker.cc hidb-make.cc

HIDB_SOURCES = hidb.cc hidb-set.cc

HIDB_LIB_MAJOR = 5
HIDB_LIB_MINOR = 0
HIDB_LIB_NAME = libhidb
HIDB_LIB = $(DIST)/$(call shared_lib_name,$(HIDB_LIB_NAME),$(HIDB_LIB_MAJOR),$(HIDB_LIB_MINOR))

# ----------------------------------------------------------------------

include $(ACMACSD_ROOT)/share/makefiles/Makefile.g++
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.vars

CXXFLAGS = -MMD -g $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WARNINGS) -Icc -I$(AD_INCLUDE) $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)
LDLIBS = \
	$(AD_LIB)/$(call shared_lib_name,libacmacsbase,1,0) \
	$(AD_LIB)/$(call shared_lib_name,liblocationdb,1,0) \
	$(AD_LIB)/$(call shared_lib_name,libacmacschart,2,0) \
	$(shell pkg-config --libs liblzma) $(CXX_LIB)

PKG_INCLUDES = $(shell pkg-config --cflags liblzma)

# ----------------------------------------------------------------------

all: check-acmacsd-root $(TARGETS)

install: check-acmacsd-root install-headers $(TARGETS)
	$(call install_lib,$(HIDB_LIB))
	ln -sf $(abspath dist)/hidb* $(AD_BIN)

test: install
	test/test

# ----------------------------------------------------------------------

-include $(BUILD)/*.d
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.rules
include $(ACMACSD_ROOT)/share/makefiles/Makefile.rtags

# ----------------------------------------------------------------------

$(HIDB_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(HIDB_SOURCES)) | $(DIST)
	@printf "%-16s %s\n" "SHARED" $@
	@$(call make_shared,$(HIDB_LIB_NAME),$(HIDB_LIB_MAJOR),$(HIDB_LIB_MINOR)) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(DIST)/hidb-make: $(patsubst %.cc,$(BUILD)/%.o,$(HIDB_MAKE_SOURCES)) | $(DIST)
	@printf "%-16s %s\n" "LINK" $@
	@$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(DIST)/%: $(BUILD)/%.o | $(HIDB_LIB)
	@printf "%-16s %s\n" "LINK" $@
	@$(CXX) $(LDFLAGS) -o $@ $^ $(HIDB_LIB) $(LDLIBS)

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
