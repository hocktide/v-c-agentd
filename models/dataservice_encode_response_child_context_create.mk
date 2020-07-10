CBMC_DIR?=/opt/cbmc
CBMC?=$(CBMC_DIR)/bin/cbmc
VCMODEL_DIR?=../subprojects/vcmodel
VCCRYPT_DIR?=../subprojects/vccrypt
LIBEVENT_DIR?=../subprojects/libevent
LIBEVENT_CONFIG_INCLUDE_DIR?=\
    $(MESON_BUILD_ROOT)/subprojects/libevent/__CMake_build/include
LMDB_DIR?=../subprojects/lmdb
VPR_DIR?=../subprojects/vpr
MODEL_CHECK_DIR?=../subprojects/vcmodel

include $(MODEL_CHECK_DIR)/model_check.mk

ALL:
	$(CBMC) --bounds-check --pointer-check --memory-leak-check \
	--div-by-zero-check --pointer-overflow-check --trace --stop-on-fail -DCBMC \
    --drop-unused-functions \
    --unwind 10 \
    --unwindset __builtin___memset_chk.0:60 \
	-I $(VCMODEL_DIR)/include -I ../include -I $(VPR_DIR)/include \
	-I $(VCCRYPT_DIR)/include -I $(LIBEVENT_DIR)/include \
	-I $(LIBEVENT_CONFIG_INCLUDE_DIR) \
	-I $(LMDB_DIR) \
	$(MODEL_CHECK_SOURCES) \
	$(VPR_DIR)/src/disposable/dispose.c \
	shadow/sys/htonl.c \
	../src/inet/htonll.c \
    ../src/dataservice/dataservice_encode_response_child_context_create.c \
	dataservice_encode_response_child_context_create_main.c
