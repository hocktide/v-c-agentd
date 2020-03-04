CBMC_DIR?=/opt/cbmc
CBMC?=$(CBMC_DIR)/bin/cbmc
LIBEVENT_DIR?=../lib/vcblockchain/lib/libevent
VCCRYPT_DIR?=../lib/vcblockchain/lib/vccrypt
VCMODEL_DIR?=../lib/vcblockchain/lib/vcmodel
VPR_DIR?=../lib/vcblockchain/lib/vpr
MODEL_CHECK_DIR?=../lib/vcblockchain/lib/vcmodel

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
	$(MODEL_CHECK_SOURCES) \
	$(VPR_DIR)/src/disposable/dispose.c \
    ../src/ipc/ipc_socketpair.c \
    ../src/ipc/ipc_make_noblock.c \
	shadow/libevent/event_free.c \
	shadow/libevent/evbuffer_free.c \
	shadow/sys/close.c \
	shadow/sys/descriptor_hack.c \
	shadow/sys/fcntl.c \
	shadow/sys/socketpair.c \
	ipc_make_noblock_main.c