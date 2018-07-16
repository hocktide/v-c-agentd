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
    -L $(VCBLOCKCHAIN_HOST_CHECKED_LIB_DIR) -lvcblockchain
VCBLOCKCHAIN_HOST_RELEASE_LINK?= \
    -L $(VCBLOCKCHAIN_HOST_RELEASE_LIB_DIR) -lvcblockchain

#agentd source files
SRCDIR=$(CURDIR)/src
DIRS=$(SRCDIR) $(SRCDIR)/agentd
SOURCES=$(foreach d,$(DIRS),$(wildcard $(d)/*.c))
STRIPPED_SOURCES=$(patsubst $(SRCDIR)/%,%,$(SOURCES))

#platform options
HOST_RELEASE_BUILD_DIR=$(BUILD_DIR)/host/release
HOST_RELEASE_EXE=$(HOST_RELEASE_BUILD_DIR)/bin/$(EXE_NAME)
HOST_RELEASE_DIRS=$(filter-out $(SRCDIR), \
    $(patsubst $(SRCDIR)/%,$(HOST_RELEASE_BUILD_DIR)/%,$(DIRS)))
HOST_RELEASE_COBJECTS= \
    $(patsubst %.c,$(HOST_RELEASE_BUILD_DIR)/%.o,$(STRIPPED_SOURCES))
HOST_RELEASE_OBJECTS= \
    $(HOST_RELEASE_COBJECTS)

#toolchain location
TOOLCHAIN_DIR?=/opt/vctoolchain

#compilers
HOST_RELEASE_CC=$(TOOLCHAIN_DIR)/host/bin/gcc
HOST_RELEASE_CXX=$(TOOLCHAIN_DIR)/host/bin/g++
HOST_RELEASE_AR=$(AR)
HOST_RELEASE_RANLIB=$(RANLIB)

#platform compiler flags
COMMON_INCLUDES=$(MODEL_CHECK_INCLUDES) $(VCBLOCKCHAIN_CFLAGS) \
                -I $(PWD)/include
COMMON_CFLAGS=$(COMMON_INCLUDES) -Wall -Werror -Wextra
HOST_RELEASE_CFLAGS=$(COMMON_CFLAGS) -I $(HOST_RELEASE_BUILD_DIR) \
    -fPIC -O2
HOST_RELEASE_LEXCOMPAT_CFLAGS=$(COMMON_CFLAGS) -I $(HOST_RELEASE_BUILD_DIR) \
    -fPIC -O2 \
	-Wno-unused-function -Wno-unused-value

.PHONY: ALL clean vcblockchain-build vcblockchain-test vcblockchain-clean
.PHONY: agentd-build host.exe.release test

MODEL_MAKEFILES?= \
	$(foreach file,\
	    $(wildcard models/*.mk),$(notdir $(file)))

ALL: agentd-build

test: vcblockchain-test

agentd-build: vcblockchain-test host.exe.release

host.exe.release: $(HOST_RELEASE_DIRS) $(HOST_RELEASE_EXE)

#build missing directories
$(HOST_RELEASE_DIRS):
	mkdir -p $@

#Host release executable
$(HOST_RELEASE_EXE) : $(HOST_RELEASE_OBJECTS)
	mkdir -p $(dir $@)
	$(HOST_RELEASE_CC) -o $@ $(HOST_RELEASE_OBJECTS) \
	    $(VCBLOCKCHAIN_HOST_RELEASE_LINK)

#Host release build objects
$(HOST_RELEASE_BUILD_DIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(HOST_RELEASE_CC) $(HOST_RELEASE_CFLAGS) -c -o $@ $<

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
