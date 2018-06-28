#
# OpenMP MPARM Makefile
#
##############################################################################

LIBRARY_NAME = libgomp_vsoc_single.a
LIBGOMP_PATH ?= ${VSOC_APP_DIR}/libgomp

# include $(GEPOP_FABRIC_HOME)/src/gepop.mk
# include $(GEPOP_FABRIC_HOME)/src/gepop_custom.mk
# include $(LIBGOMP_PATH)/config/p2012/gepop_custom.mk

VSOC_CONFIG_PATH = ${LIBGOMP_PATH}/config/vsoc
APPSUPPORT_PATH = ${LIBGOMP_PATH}/config/vsoc
INCLUDE_PATH = ${APPSUPPORT_PATH}
INCLUDE_PATH += -I${LIBGOMP_PATH}

CSRCS  += ${LIBGOMP_PATH}/root.c
CSRCS  += ${APPSUPPORT_PATH}/appsupport.c

COBJS  = $(CSRCS:.c=.o)

CFLAGS += -O2 -I. -I$(INCLUDE_PATH)
# CFLAGS += -DP2012_HW_BAR
# CFLAGS += -DSTATS_ENABLE

all: $(COBJS) link

link:
	$(AR) rcs $(LIBRARY_NAME)  $(GEPOP_FABRIC_HOME)/src/cluster0.ld ${LIBGOMP_PATH}/root.o ${APPSUPPORT_PATH}/appsupport.o
	mv $(LIBRARY_NAME) ${LIBGOMP_PATH}/lib/

clean:
	rm -r -f *.o

	rm -r -f ${APPSUPPORT_PATH}/*.o

	rm -r -f ${LIBGOMP_PATH}/*.o

	rm -r -f ${VSOC_CONFIG_PATH}/*.o

	rm -r -f ${LIBGOMP_PATH}/lib/${LIBRARY_NAME}

	rm -r -f *.s
