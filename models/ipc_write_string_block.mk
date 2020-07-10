CBMC_DIR?=/opt/cbmc
CBMC?=$(CBMC_DIR)/bin/cbmc
LIBEVENT_DIR?=../subprojects/libevent
LIBEVENT_CONFIG_INCLUDE_DIR?=\
    $(MESON_BUILD_ROOT)/subprojects/libevent/__CMake_build/include
VCCRYPT_DIR?=../subprojects/vccrypt
VCMODEL_DIR?=../subprojects/vcmodel
VPR_DIR?=../subprojects/vpr
MODEL_CHECK_DIR?=../subprojects/vcmodel

include $(MODEL_CHECK_DIR)/model_check.mk

ALL:
	$(CBMC) --bounds-check --pointer-check --memory-leak-check \
	--div-by-zero-check \
    --pointer-overflow-check --trace --stop-on-fail -DCBMC \
    --drop-unused-functions \
    --unwind 10 \
    --unwindset __builtin___memset_chk.0:60 \
	-I $(VCMODEL_DIR)/include -I ../include -I $(VPR_DIR)/include \
    -I $(VCCRYPT_DIR)/include -I $(LIBEVENT_DIR)/include \
	-I $(LIBEVENT_CONFIG_INCLUDE_DIR) \
	$(MODEL_CHECK_SOURCES) \
	$(VPR_DIR)/src/disposable/dispose.c \
    ../src/ipc/ipc_socketpair.c \
    ../src/ipc/ipc_write_string_block.c \
	shadow/sys/close.c \
	shadow/sys/descriptor_hack.c \
	shadow/sys/htonl.c \
	shadow/sys/socketpair.c \
	shadow/sys/write.c \
	ipc_write_string_block_main.c
