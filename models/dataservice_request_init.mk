CBMC_DIR?=/opt/cbmc
CBMC?=$(CBMC_DIR)/bin/cbmc
VCMODEL_DIR?=../subprojects/vcmodel
VPR_DIR?=../subprojects/vpr
MODEL_CHECK_DIR?=../subprojects/vcmodel

include $(MODEL_CHECK_DIR)/model_check.mk

ALL:
	$(CBMC) --bounds-check --pointer-check --memory-leak-check \
	--div-by-zero-check --signed-overflow-check --unsigned-overflow-check \
    --pointer-overflow-check --conversion-check \
	--conversion-check --trace --stop-on-fail -DCBMC \
    --drop-unused-functions \
    --unwind 10 \
    --unwindset __builtin___memset_chk.0:60 \
	-I $(VCMODEL_DIR)/include -I ../include -I $(VPR_DIR)/include \
	$(MODEL_CHECK_SOURCES) \
	$(VPR_DIR)/src/disposable/dispose.c \
    shadow/sys/ntohl.c \
    ../src/dataservice/dataservice_request_dispose.c \
    ../src/dataservice/dataservice_request_init.c \
	dataservice_request_init_main.c
