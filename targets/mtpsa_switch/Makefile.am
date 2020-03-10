SUBDIRS = .

THRIFT_IDL = $(srcdir)/thrift/mtpsa_switch.thrift

noinst_LTLIBRARIES = libmtpsaswitch.la

libmtpsaswitch_la_SOURCES = \
mtpsa_switch.cpp \
mtpsa_switch.h \
primitives.cpp \
externs/mtpsa_counter.h \
externs/mtpsa_counter.cpp \
externs/mtpsa_meter.h \
externs/mtpsa_meter.cpp

libmtpsaswitch_la_LIBADD = \
$(top_builddir)/src/bm_sim/libbmsim.la \
$(top_builddir)/src/bf_lpm_trie/libbflpmtrie.la \
$(top_builddir)/src/BMI/libbmi.la \
$(top_builddir)/third_party/jsoncpp/libjson.la \
-lboost_system $(THRIFT_LIB) -lboost_program_options -lboost_filesystem

if COND_THRIFT

mtpsa_switch_thrift_py_files = \
gen-py/mtpswitch_runtime/constants.py \
gen-py/mtpswitch_runtime/__init__.py \
gen-py/mtpswitch_runtime/MtPsaSwitch.py \
gen-py/mtpswitch_runtime/ttypes.py
mtpsa_switch_thrift_files = \
gen-cpp/bm/mtpsa_switch_constants.cpp \
gen-cpp/bm/mtpsa_switch_constants.h \
gen-cpp/bm/MtPsaSwitch.cpp \
gen-cpp/bm/MtPsaSwitch.h \
gen-cpp/bm/mtpsa_switch_types.cpp \
gen-cpp/bm/mtpsa_switch_types.h

BUILT_SOURCES = $(mtpsa_switch_thrift_files) $(mtpsa_switch_thrift_py_files)

pswitchpydir = $(pythondir)/mtpswitch_runtime
nodist_pswitchpy_PYTHON = $(mtpsa_switch_thrift_py_files)

python_PYTHON = mtpswitch_CLI.py

# See
# http://www.gnu.org/software/autoconf/manual/autoconf-2.69/html_node/Installation-Directory-Variables.html
edit = sed \
	-e 's|@pythondir[@]|$(pythondir)|g'

mtpsa_switch_CLI: Makefile
	rm -f $@ $@.tmp
	$(edit) $(srcdir)/$@.in >$@.tmp
	chmod +x $@.tmp
	chmod a-w $@.tmp
	mv $@.tmp $@

mtpsa_switch_CLI: mtpsa_switch_CLI.in

EXTRA_DIST = \
mtpsa_switch_CLI.in \
pswitch_CLI

bin_SCRIPTS = \
mtpsa_switch_CLI

AM_CPPFLAGS += \
-I$(srcdir)/thrift/src/ \
-I$(builddir)/gen-cpp

libmtpsaswitch_la_SOURCES += thrift/src/MtPsaSwitch_server.cpp

bin_PROGRAMS = mtpsa_switch

mtpsa_switch_LDADD = \
$(top_builddir)/src/bm_runtime/libbmruntime.la \
$(top_builddir)/thrift_src/libruntimestubs.la \
libmtpsaswitch.la \
libmtpsaswitch_thrift.la

mtpsa_switch_SOURCES = main.cpp

lib_LTLIBRARIES = libmtpsaswitch_thrift.la

nodist_libmtpsaswitch_thrift_la_SOURCES = \
$(mtpsa_switch_thrift_files)

# See http://www.gnu.org/software/automake/manual/html_node/Multiple-Outputs.html

EXTRA_DIST += $(THRIFT_IDL)

# We copy the generated Python code to srcdir/ if we have permissions (i.e. not
# for 'make distcheck'). This is to ensure we can run the CLI easily. Maybe I
# will think of a better solution later.
thrift_files.ts: $(THRIFT_IDL)
	@rm -f thrift_files.tmp
	@touch thrift_files.tmp
	@mkdir -p $(builddir)/gen-cpp/bm
	$(THRIFT) -out $(builddir)/gen-cpp/bm --gen cpp -r $(THRIFT_IDL)
	$(THRIFT) -o $(builddir) --gen py -r $(THRIFT_IDL)
	if mkdir $(srcdir)/mtpswitch_runtime.test 2>/dev/null; then \
	  rm -rf $(srcdir)/mtpswitch_runtime/; \
	  cp -r $(builddir)/gen-py/mtpswitch_runtime/ $(srcdir)/; \
	  rm -rf $(srcdir)/mtpswitch_runtime.test; else :; \
        fi
	@mv -f thrift_files.tmp $@

$(BUILT_SOURCES): thrift_files.ts
## Recover from the removal of $@
	@if test -f $@; then :; else \
	  trap 'rm -rf thrift_files.lock thrift_files.ts' 1 2 13 15; \
## mkdir is a simple test-and-set
	if mkdir thrift_files.lock 2>/dev/null; then \
## This code is being executed by the first process.
	  rm -f thrift_files.ts; \
	  $(MAKE) $(AM_MAKEFLAGS) thrift_files.ts; \
	  result=$$?; rm -rf thrift_files.lock; exit $$result; \
	else \
## This code is being executed by the follower processes.
## Wait until the first process is done.
	  while test -d thrift_files.lock; do sleep 1; done; \
## Succeed if and only if the first process succeeded.
	    test -f thrift_files.ts; \
	  fi; \
	fi

nodist_pkginclude_HEADERS = \
gen-cpp/bm/mtpsa_switch_constants.h \
gen-cpp/bm/MtPsaSwitch.h \
gen-cpp/bm/mtpsa_switch_types.h

CLEANFILES = $(BUILT_SOURCES) \
thrift_files.ts \
gen-cpp/bm/MtPsaSwitch_server.skeleton.cpp \
$(bin_SCRIPTS)

# I used to do a rm on gen-cpp, but it was removing a .deps directory, instead I
# am adding the skeleton file to CLEANFILES
clean-local:
	rm -rf gen-py

endif  # COND_THRIFT
