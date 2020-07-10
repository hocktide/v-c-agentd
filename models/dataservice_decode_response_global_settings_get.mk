CBMC_DIR?=/opt/cbmc
CBMC?=$(CBMC_DIR)/bin/cbmc
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
	$(MODEL_CHECK_SOURCES) \
	$(VPR_DIR)/src/disposable/dispose.c \
	shadow/sys/ntohl.c \
    ../src/dataservice/dataservice_decode_response_memset_disposer.c \
    ../src/dataservice/dataservice_decode_response_global_settings_get.c \
	dataservice_decode_response_global_settings_get_main.c
