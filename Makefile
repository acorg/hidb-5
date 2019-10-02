# -*- Makefile -*-
# ----------------------------------------------------------------------

TARGETS = \
  $(DIST)/hidb5-make \
  $(DIST)/hidb5-convert \
  $(DIST)/hidb5-stat \
  $(DIST)/hidb5-find \
  $(DIST)/hidb5-antigens-sera-of-chart \
  $(DIST)/hidb5-vaccines-of-chart \
  $(DIST)/hidb5-dates \
  $(DIST)/hidb5-first-table-date \
  $(DIST)/hidb5-reference-antigens-in-tables

HIDB_MAKE_SOURCES = hidb-maker.cc hidb-make.cc

HIDB_SOURCES = hidb.cc hidb-set.cc hidb-json.cc hidb-bin.cc vaccines.cc report.cc

HIDB_LIB_MAJOR = 5
HIDB_LIB_MINOR = 0
HIDB_LIB_NAME = libhidb
HIDB_LIB = $(DIST)/$(call shared_lib_name,$(HIDB_LIB_NAME),$(HIDB_LIB_MAJOR),$(HIDB_LIB_MINOR))

# ----------------------------------------------------------------------

all: install

include $(ACMACSD_ROOT)/share/Makefile.config

LDLIBS = \
  $(AD_LIB)/$(call shared_lib_name,libacmacsbase,1,0) \
  $(AD_LIB)/$(call shared_lib_name,liblocationdb,1,0) \
  $(AD_LIB)/$(call shared_lib_name,libacmacsvirus,1,0) \
  $(AD_LIB)/$(call shared_lib_name,libacmacschart,2,0) \
  $(XZ_LIBS) $(CXX_LIBS)

# ----------------------------------------------------------------------

install: install-headers $(TARGETS)
	$(call install_lib,$(HIDB_LIB))
	$(call symbolic_link_wildcard,$(DIST)/hidb5*,$(AD_BIN))

test: install
	test/test
.PHONY: test

# ----------------------------------------------------------------------

$(HIDB_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(HIDB_SOURCES)) | $(DIST)
	$(call echo_shared_lib,$@)
	$(call make_shared_lib,$(HIDB_LIB_NAME),$(HIDB_LIB_MAJOR),$(HIDB_LIB_MINOR)) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(DIST)/hidb5-make: $(patsubst %.cc,$(BUILD)/%.o,$(HIDB_MAKE_SOURCES)) | $(DIST)
	$(call echo_link_exe,$@)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS) $(AD_RPATH)

$(DIST)/%: $(BUILD)/%.o | $(HIDB_LIB)
	$(call echo_link_exe,$@)
	$(CXX) $(LDFLAGS) -o $@ $^ $(HIDB_LIB) $(LDLIBS) $(AD_RPATH)

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
