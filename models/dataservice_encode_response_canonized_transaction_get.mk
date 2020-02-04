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
	../src/inet/htonll.c \
    ../src/dataservice/dataservice_encode_response_canonized_transaction_get.c \
	dataservice_encode_response_canonized_transaction_get_main.c
