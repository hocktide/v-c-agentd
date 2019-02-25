EXE_NAME=agentd
BUILD_DIR=$(CURDIR)/build

#vcblockchain options
VCBLOCKCHAIN_DIR?=lib/vcblockchain
VCBLOCKCHAIN_INCLUDE_PATH?=$(VCBLOCKCHAIN_DIR)/include
VCBLOCKCHAIN_VPR_INCLUDE_PATH?=$(VCBLOCKCHAIN_DIR)/lib/vpr/include
VCBLOCKCHAIN_VCCRYPT_INCLUDE_PATH?=$(VCBLOCKCHAIN_DIR)/lib/vccrypt/include
VCBLOCKCHAIN_VCCERT_INCLUDE_PATH?=$(VCBLOCKCHAIN_DIR)/lib/vccert/include
VCBLOCKCHAIN_LMDB_INCLUDE_PATH?=$(VCBLOCKCHAIN_DIR)/lib/lmdb
VCBLOCKCHAIN_VCMODEL_INCLUDE_PATH?=$(VCBLOCKCHAIN_DIR)/lib/vcmodel/include
VCBLOCKCHAIN_VCDB_INCLUDE_PATH?=$(VCBLOCKCHAIN_DIR)/lib/vcdb/include
VCBLOCKCHAIN_CFLAGS= \
    -I $(VCBLOCKCHAIN_INCLUDE_PATH) \
    -I $(VCBLOCKCHAIN_VPR_INCLUDE_PATH) \
    -I $(VCBLOCKCHAIN_VCCRYPT_INCLUDE_PATH) \
    -I $(VCBLOCKCHAIN_VCCERT_INCLUDE_PATH) \
    -I $(VCBLOCKCHAIN_LMDB_INCLUDE_PATH) \
    -I $(VCBLOCKCHAIN_VCMODEL_INCLUDE_PATH) \
    -I $(VCBLOCKCHAIN_VCDB_INCLUDE_PATH)
VCBLOCKCHAIN_HOST_CHECKED_LIB_DIR?=$(VCBLOCKCHAIN_DIR)/build/host/checked
VCBLOCKCHAIN_HOST_RELEASE_LIB_DIR?=$(VCBLOCKCHAIN_DIR)/build/host/release
VCBLOCKCHAIN_HOST_CHECKED_LINK?= \
    -L $(VCBLOCKCHAIN_HOST_CHECKED_LIB_DIR) -lvcblockchain -lpthread
VCBLOCKCHAIN_HOST_RELEASE_LINK?= \
    -L $(VCBLOCKCHAIN_HOST_RELEASE_LIB_DIR) -lvcblockchain -lpthread

#Google Test options
GTEST_DIR?=$(CURDIR)/lib/vcblockchain/lib/googletest/googletest
GTEST_OBJ=$(TEST_BUILD_DIR)/gtest-all.o

#Libevent options
LIBEVENT_DIR?=$(CURDIR)/lib/vcblockchain/lib/libevent
LIBEVENT_CFLAGS=-I $(LIBEVENT_DIR)/include

#agentd source files
SRCDIR=$(CURDIR)/src
DIRS=$(SRCDIR) $(SRCDIR)/agentd $(SRCDIR)/bootstrap_config \
    $(SRCDIR)/command $(SRCDIR)/commandline $(SRCDIR)/config \
    $(SRCDIR)/dataservice $(SRCDIR)/listenservice $(SRCDIR)/inet $(SRCDIR)/ipc \
    $(SRCDIR)/path $(SRCDIR)/privsep $(SRCDIR)/protocolservice \
    $(SRCDIR)/string $(SRCDIR)/supervisor
SOURCES=$(foreach d,$(DIRS),$(wildcard $(d)/*.c))
YACCSOURCES=$(foreach d,$(DIRS),$(wildcard $(d)/*.y))
LEXSOURCES=$(foreach d,$(DIRS),$(wildcard $(d)/*.l))
STRIPPED_SOURCES=$(patsubst $(SRCDIR)/%,%,$(SOURCES))
STRIPPED_YACCSOURCES=$(patsubst $(SRCDIR)/%,%,$(YACCSOURCES))
STRIPPED_LEXSOURCES=$(patsubst $(SRCDIR)/%,%,$(LEXSOURCES))

#agentd test files
TESTDIR=$(CURDIR)/test
TESTDIRS=$(TESTDIR) $(TESTDIR)/bitcap $(TESTDIR)/bootstrap_config \
    $(TESTDIR)/commandline $(TESTDIR)/config $(TESTDIR)/dataservice \
    $(TESTDIR)/ipc $(TESTDIR)/path $(TESTDIR)/status_codes $(TESTDIR)/string
TEST_BUILD_DIR=$(HOST_CHECKED_BUILD_DIR)/test
TEST_DIRS=$(filter-out $(TESTDIR), \
    $(patsubst $(TESTDIR)/%,$(TEST_BUILD_DIR)/%,$(TESTDIRS)))
TEST_SOURCES=$(foreach d,$(TESTDIRS),$(wildcard $(d)/*.cpp))
STRIPPED_TEST_SOURCES=$(patsubst $(TESTDIR)/%,%,$(TEST_SOURCES))
TEST_OBJECTS=$(patsubst %.cpp,$(TEST_BUILD_DIR)/%.o,$(STRIPPED_TEST_SOURCES))
TESTAGENTD=$(HOST_CHECKED_BUILD_DIR)/testagentd

#platform options
HOST_RELEASE_BUILD_DIR=$(BUILD_DIR)/host/release
HOST_CHECKED_BUILD_DIR=$(BUILD_DIR)/host/checked
HOST_RELEASE_EXE=$(HOST_RELEASE_BUILD_DIR)/bin/$(EXE_NAME)
HOST_CHECKED_EXE=$(HOST_CHECKED_BUILD_DIR)/bin/$(EXE_NAME)
HOST_CHECKED_DIRS=$(filter-out $(SRCDIR), \
    $(patsubst $(SRCDIR)/%,$(HOST_CHECKED_BUILD_DIR)/%,$(DIRS)))
HOST_CHECKED_COBJECTS= \
    $(filter-out $(HOST_CHECKED_BUILD_DIR)/agentd/%.o, \
        $(patsubst %.c,$(HOST_CHECKED_BUILD_DIR)/%.o,$(STRIPPED_SOURCES)))
HOST_CHECKED_YACCOBJECTS= \
    $(patsubst %.y,$(HOST_CHECKED_BUILD_DIR)/%.tab.o,$(STRIPPED_YACCSOURCES))
HOST_CHECKED_YACCHEADERS= \
    $(patsubst %.y,$(HOST_CHECKED_BUILD_DIR)/%.tab.h,$(STRIPPED_YACCSOURCES))
HOST_CHECKED_LEXOBJECTS= \
    $(patsubst %.l,$(HOST_CHECKED_BUILD_DIR)/%.yy.o,$(STRIPPED_LEXSOURCES))
HOST_CHECKED_OBJECTS= \
    $(HOST_CHECKED_COBJECTS) $(HOST_CHECKED_YACCOBJECTS) \
    $(HOST_CHECKED_LEXOBJECTS)
HOST_RELEASE_DIRS=$(filter-out $(SRCDIR), \
    $(patsubst $(SRCDIR)/%,$(HOST_RELEASE_BUILD_DIR)/%,$(DIRS)))
HOST_RELEASE_COBJECTS= \
    $(patsubst %.c,$(HOST_RELEASE_BUILD_DIR)/%.o,$(STRIPPED_SOURCES))
HOST_RELEASE_YACCOBJECTS= \
    $(patsubst %.y,$(HOST_RELEASE_BUILD_DIR)/%.tab.o,$(STRIPPED_YACCSOURCES))
HOST_RELEASE_YACCHEADERS= \
    $(patsubst %.y,$(HOST_RELEASE_BUILD_DIR)/%.tab.h,$(STRIPPED_YACCSOURCES))
HOST_RELEASE_LEXOBJECTS= \
    $(patsubst %.l,$(HOST_RELEASE_BUILD_DIR)/%.yy.o,$(STRIPPED_LEXSOURCES))
HOST_RELEASE_OBJECTS= \
    $(HOST_RELEASE_COBJECTS) $(HOST_RELEASE_YACCOBJECTS) \
    $(HOST_RELEASE_LEXOBJECTS)

#report files
COVERAGE_REPORT_DIR=$(TEST_BUILD_DIR)/coverage-report
REPORT_FILES=\
    $(patsubst %.c,$(COVERAGE_REPORT_DIR)/%.c.gcov,$(STRIPPED_SOURCES))

#toolchain location
TOOLCHAIN_DIR?=/opt/vctoolchain

#compilers
HOST_RELEASE_CC?=$(TOOLCHAIN_DIR)/host/bin/gcc
HOST_RELEASE_CXX=$(TOOLCHAIN_DIR)/host/bin/g++
HOST_RELEASE_GCOV?=$(TOOLCHAIN_DIR)/host/bin/gcov
HOST_RELEASE_AR=$(AR)
HOST_RELEASE_RANLIB=$(RANLIB)
HOST_RELEASE_LEX=$(TOOLCHAIN_DIR)/host/bin/flex
HOST_RELEASE_YACC=$(TOOLCHAIN_DIR)/host/bin/bison
HOST_CHECKED_CC?=$(TOOLCHAIN_DIR)/host/bin/gcc
HOST_CHECKED_CXX=$(TOOLCHAIN_DIR)/host/bin/g++
HOST_CHECKED_AR=$(AR)
HOST_CHECKED_RANLIB=$(RANLIB)
HOST_CHECKED_LEX=$(TOOLCHAIN_DIR)/host/bin/flex
HOST_CHECKED_YACC=$(TOOLCHAIN_DIR)/host/bin/bison

#platform compiler flags
COMMON_INCLUDES=$(MODEL_CHECK_INCLUDES) $(VCBLOCKCHAIN_CFLAGS) \
                $(LIBEVENT_CFLAGS) -I $(CURDIR)/include
COMMON_CFLAGS=$(COMMON_INCLUDES) -Wall -Werror -Wextra \
    -I $(TOOLCHAIN_DIR)/host/include
HOST_CHECKED_CFLAGS=$(COMMON_CFLAGS) -I $(HOST_CHECKED_BUILD_DIR) \
    -fPIC -O0 --coverage
HOST_CHECKED_LEXCOMPAT_CFLAGS=$(COMMON_CFLAGS) -I $(HOST_CHECKED_BUILD_DIR) \
    -fPIC -O0 --coverage \
	-Wno-unused-function -Wno-unused-value -Wno-unused-const-variable
HOST_RELEASE_CFLAGS=$(COMMON_CFLAGS) -I $(HOST_RELEASE_BUILD_DIR) \
    -fPIC -O2
HOST_RELEASE_LEXCOMPAT_CFLAGS=$(COMMON_CFLAGS) -I $(HOST_RELEASE_BUILD_DIR) \
    -fPIC -O2 \
	-Wno-unused-function -Wno-unused-value -Wno-unused-const-variable
COMMON_CXXFLAGS=-I $(PWD)/include $(VCBLOCKCHAIN_CFLAGS) -Wall -Werror -Wextra \
    -I $(TOOLCHAIN_DIR)/host/include
HOST_CHECKED_CXXFLAGS=-std=c++14 $(COMMON_CXXFLAGS) -O0 --coverage
HOST_RELEASE_CXXFLAGS=-std=c++14 $(COMMON_CXXFLAGS) -O2
TEST_CXXFLAGS=$(HOST_RELEASE_CXXFLAGS) $(COMMON_INCLUDES) -I $(GTEST_DIR) \
     -I $(GTEST_DIR)/include -I $(HOST_CHECKED_BUILD_DIR)

.PHONY: ALL clean vcblockchain-build vcblockchain-test vcblockchain-clean
.PHONY: agentd-build host.exe.release host.exe.checked test test.agentd
.PHONY: testreport.agentd
.PHONY: install agentd-install

MODEL_MAKEFILES?= \
	$(foreach file,\
	    $(wildcard models/*.mk),$(notdir $(file)))

ALL: agentd-build

test: vcblockchain-test test.agentd

install: agentd-install

agentd-install: ALL
	@if [ "${PREFIX}" == "" ]; then echo "PREFIX must be set for install."; exit 1; fi
	mkdir -p ${PREFIX}/bin ${PREFIX}/lib ${PREFIX}/etc ${PREFIX}/data
	install ${HOST_RELEASE_EXE} ${PREFIX}/bin
	ldd ${HOST_RELEASE_EXE} | egrep "[.]so" | grep -v ld.so | grep -v vdso.so \
	    | grep -v ld-linux-x86-64.so.2 \
	    | sed 's/(.*)//; s/\(.*\)=>/\1/' \
	    | awk '{ print $$NF }' \
	    | xargs -I instfile install instfile ${PREFIX}/lib
	if [ -f /usr/libexec/ld.so ]; then \
	    mkdir -p ${PREFIX}/usr/libexec; \
	    install /usr/libexec/ld.so ${PREFIX}/usr/libexec; \
	fi
	if [ -f /lib64/ld-linux-x86-64.so.2 ]; then \
	    mkdir -p ${PREFIX}/lib64; \
	    install /lib64/ld-linux-x86-64.so.2 ${PREFIX}/lib64; \
	fi

test.agentd: vcblockchain-build $(TEST_DIRS) host.exe.checked $(TESTAGENTD)
	rm -rf $(HOST_CHECKED_BUILD_DIR)/databases
	rm -rf $(BUILD_DIR)/test/isolation/databases
	find $(BUILD_DIR) -type f -name "*.gcda" -exec rm {} \; -print
	TEST_BIN=$(realpath $(shell which cat)) \
	LD_LIBRARY_PATH=$(TOOLCHAIN_DIR)/host/lib:$(TOOLCHAIN_DIR)/host/lib64:$(LD_LIBRARY_PATH) \
	$(TESTAGENTD) $(TEST_AGENTD_FILTER)

testreport.agentd: $(REPORT_FILES)

$(COVERAGE_REPORT_DIR)/%.c.gcov: $(SRCDIR)/%.c test.agentd
	mkdir -p $(dir $@)
	(cd $(dir $@) && \
	    $(HOST_RELEASE_GCOV) -o $(dir $(HOST_CHECKED_BUILD_DIR)/$*.o) $<)

agentd-build: host.exe.checked host.exe.release

host.exe.release: $(HOST_RELEASE_DIRS) $(HOST_RELEASE_EXE)

host.exe.checked: $(HOST_CHECKED_DIRS) $(HOST_CHECKED_EXE)

#build missing directories
$(HOST_RELEASE_DIRS) $(HOST_CHECKED_DIRS) $(TEST_DIRS):
	mkdir -p $@

#Host checked executable
$(HOST_CHECKED_EXE) : vcblockchain-build $(HOST_CHECKED_OBJECTS)
	mkdir -p $(dir $@)
	$(HOST_CHECKED_CC) -o $@ $(HOST_CHECKED_OBJECTS) \
	    $(VCBLOCKCHAIN_HOST_RELEASE_LINK) \
	    -L $(TOOLCHAIN_DIR)/host/lib -Wl,-Bstatic -lfl -ly -Wl,-Bdynamic \
	    --coverage

#Host release executable
$(HOST_RELEASE_EXE) : vcblockchain-build $(HOST_RELEASE_OBJECTS)
	mkdir -p $(dir $@)
	$(HOST_RELEASE_CC) -o $@ $(HOST_RELEASE_OBJECTS) \
	    $(VCBLOCKCHAIN_HOST_RELEASE_LINK) \
	    -L $(TOOLCHAIN_DIR)/host/lib -Wl,-Bstatic -lfl -ly -Wl,-Bdynamic

#Google Test object
$(GTEST_OBJ): $(GTEST_DIR)/src/gtest-all.cc
	mkdir -p $(dir $@)
	$(HOST_RELEASE_CXX) $(TEST_CXXFLAGS) -c -o $@ $<

#Test build objects
$(TEST_BUILD_DIR)/%.o: $(TESTDIR)/%.cpp
	mkdir -p $(dir $@)
	$(HOST_RELEASE_CXX) $(TEST_CXXFLAGS) -c -o $@ $<

#host checked build objects depend on lex and yacc objects
$(HOST_CHECKED_LEXOBJECTS): $(HOST_CHECKED_YACCOBJECTS)
$(HOST_CHECKED_COBJECTS): $(HOST_CHECKED_LEXOBJECTS)
$(HOST_CHECKED_COBJECTS): $(HOST_CHECKED_YACCOBJECTS)

#Host checked build objects
$(HOST_CHECKED_BUILD_DIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(HOST_CHECKED_CC) $(HOST_CHECKED_CFLAGS) -c -o $@ $<

#host release build objects depend on lex and yacc objects
$(HOST_RELEASE_LEXOBJECTS): $(HOST_RELEASE_YACCOBJECTS)
$(HOST_RELEASE_COBJECTS): $(HOST_RELEASE_LEXOBJECTS)
$(HOST_RELEASE_COBJECTS): $(HOST_RELEASE_YACCOBJECTS)

#Host release build objects
$(HOST_RELEASE_BUILD_DIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(HOST_RELEASE_CC) $(HOST_RELEASE_CFLAGS) -c -o $@ $<

HRBD=$(HOST_RELEASE_BUILD_DIR)
HCBD=$(HOST_CHECKED_BUILD_DIR)

#Host checked build yacc objects
$(HCBD)/%.tab.c $(HCBD)/%.tab.h : $(SRCDIR)/%.y
	mkdir -p $(dir $@)
	$(HOST_CHECKED_YACC) -d -b $(*F) -o $(HCBD)/$*.tab.c \
	    $(SRCDIR)/$*.y

#Host checked build lex objects
$(HCBD)/%.yy.c: $(SRCDIR)/%.l
	mkdir -p $(dir $@)
	$(HOST_CHECKED_LEX) -s --header-file=$(HCBD)/$*.yy.h -o $(HCBD)/$*.yy.c \
	    $(SRCDIR)/$*.l

#Host release build yacc objects
$(HRBD)/%.tab.c $(HRBD)/%.tab.h : $(SRCDIR)/%.y
	mkdir -p $(dir $@)
	$(HOST_RELEASE_YACC) -d -b $(*F) -o $(HRBD)/$*.tab.c \
	    $(SRCDIR)/$*.y

#Host release build lex objects
$(HRBD)/%.yy.c: $(SRCDIR)/%.l
	mkdir -p $(dir $@)
	$(HOST_RELEASE_LEX) -s --header-file=$(HRBD)/$*.yy.h -o $(HRBD)/$*.yy.c \
	    $(SRCDIR)/$*.l

#host checked lex objects depend on the availability of host checked yacc
#objects.
$(HCBD)/%.yy.o: $(HOST_CHECKED_YACCHEADERS)

#host release lex objects depend on the availability of host release yacc
#objects.
$(HRBD)/%.yy.o: $(HOST_RELEASE_YACCHEADERS)

$(HCBD)/%.yy.o: $(HCBD)/%.yy.c
	mkdir -p $(dir $@)
	$(HOST_CHECKED_CC) $(HOST_CHECKED_LEXCOMPAT_CFLAGS) -c -o $@ $<

$(HRBD)/%.yy.o: $(HRBD)/%.yy.c
	mkdir -p $(dir $@)
	$(HOST_RELEASE_CC) $(HOST_RELEASE_LEXCOMPAT_CFLAGS) -c -o $@ $<

$(HCBD)/%.tab.o: $(HCBD)/%.tab.c
	mkdir -p $(dir $@)
	$(HOST_CHECKED_CC) $(HOST_CHECKED_LEXCOMPAT_CFLAGS) -c -o $@ $<

$(HRBD)/%.tab.o: $(HRBD)/%.tab.c
	mkdir -p $(dir $@)
	$(HOST_RELEASE_CC) $(HOST_RELEASE_LEXCOMPAT_CFLAGS) -c -o $@ $<

$(TESTAGENTD): vcblockchain-build $(HOST_CHECKED_OBJECTS) $(TEST_OBJECTS) $(GTEST_OBJ)
	find $(TEST_BUILD_DIR) -name "*.gcda" -exec rm {} \; -print
	rm -f gtest-all.gcda
	$(HOST_RELEASE_CXX) $(TEST_CXXFLAGS) \
	    -o $@ $(TEST_OBJECTS) \
	    $(HOST_CHECKED_OBJECTS) $(GTEST_OBJ) -lpthread \
	    -L $(TOOLCHAIN_DIR)/host/lib64 -lstdc++ \
	    $(VCBLOCKCHAIN_HOST_RELEASE_LINK) \
	    -L $(TOOLCHAIN_DIR)/host/lib -Wl,-Bstatic -lfl -ly -Wl,-Bdynamic \
        --coverage

model-check:
	for n in $(MODEL_MAKEFILES); do \
	    (cd models && $(MAKE) -f $$n) \
	done

clean: vcblockchain-clean
	rm -rf $(BUILD_DIR)

vcblockchain-clean:
	(cd lib/vcblockchain && $(MAKE) clean)

vcblockchain-test: vcblockchain-build
	(cd lib/vcblockchain && $(MAKE) test)

vcblockchain-build:
	(cd lib/vcblockchain && $(MAKE))
