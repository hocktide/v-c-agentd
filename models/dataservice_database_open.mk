CBMC_DIR?=/opt/cbmc
CBMC?=$(CBMC_DIR)/bin/cbmc
VCMODEL_DIR?=../lib/vcblockchain/lib/vcmodel
VCCRYPT_DIR?=../lib/vcblockchain/lib/vccrypt
LIBEVENT_DIR?=../lib/vcblockchain/lib/libevent
LMDB_DIR?=../lib/vcblockchain/lib/lmdb
VPR_DIR?=../lib/vcblockchain/lib/vpr
MODEL_CHECK_DIR?=../lib/vcblockchain/lib/vcmodel

include $(MODEL_CHECK_DIR)/model_check.mk

ALL:
	$(CBMC) --bounds-check --pointer-check --memory-leak-check \
	--div-by-zero-check --pointer-overflow-check --trace --stop-on-fail -DCBMC \
    --object-bits 16 --drop-unused-functions \
    --unwind 10 \
    --unwindset __builtin___memset_chk.0:60 \
	-I $(VCMODEL_DIR)/include -I ../include -I $(VPR_DIR)/include \
	-I $(VCCRYPT_DIR)/include -I $(LIBEVENT_DIR)/include \
	-I $(LMDB_DIR) \
	$(MODEL_CHECK_SOURCES) \
	$(VPR_DIR)/src/disposable/dispose.c \
	../models/shadow/lmdb/*.c \
    ../src/dataservice/dataservice_database_open.c \
    ../src/dataservice/dataservice_database_close.c \
	dataservice_database_open_main.c
