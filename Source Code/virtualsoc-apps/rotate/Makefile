ifndef VSOC_ROOT_DIR
    $(error VSOC_ROOT_DIR is undefined)
endif

#BUILD_TYPE ?= HWS_BARS
SRCS_OMP := rot.c

APP_FLAGS = -DCROP_IMAGE=${CROP_IMAGE} \
	    -DPRIVATIZE_WORK_COUNTER=${PRIVATIZE_WORK_COUNTER} \
	    -DCHUNK=${CHUNK$}
		
ifeq ($(DYNAMIC_SCHEDULER),1) 
APP_FLAGS += -DDYNAMIC_SCHEDULER
endif

CC        = arm-elf-gcc
LD        = arm-elf-ld
AS        = arm-elf-as
OBJCOPY   = arm-elf-objcopy
OBJDUMP   = arm-elf-objdump

##################################################################

LIBGOMP_PATH	= ${VSOC_APP_DIR}/libgomp
VSOC_CONFIG_PATH = ${LIBGOMP_PATH}/config/vsoc
APPSUPPORT_PATH = ${VSOC_APP_DIR}/support/simulator
INCLUDE_PATH = ${APPSUPPORT_PATH}
INCLUDE_PATH += -I${LIBGOMP_PATH}

CSRCS  += ${LIBGOMP_PATH}/root.c

ifeq ($(BUILD_TYPE),HWS_BARS)
CSRCS  += ${APPSUPPORT_PATH}/appsupport.c ${APPSUPPORT_PATH}/hws_support.c
CFLAGS += -O2 -I$(LIBGOMP_PATH) -I$(INCLUDE_PATH)  -I${VSOC_SRC_DIR}/core -I${VSOC_CONFIG_PATH} -DMPARM -DMPARM_HW_BAR
else
CSRCS  += ${APPSUPPORT_PATH}/appsupport.c
CFLAGS += -O2 -I$(LIBGOMP_PATH) -I$(INCLUDE_PATH)  -I${VSOC_SRC_DIR}/core -I${VSOC_CONFIG_PATH} -DMPARM
endif

ASSRCS = ${APPSUPPORT_PATH}/boot.s

COBJS  = $(CSRCS:.c=.o)
ASOBJS = $(ASSRCS:.s=.o)

CFLAGS += -O2 -I$(LIBGOMP_PATH) -I$(INCLUDE_PATH)  -I${VSOC_SRC_DIR}/core -I${VSOC_CONFIG_PATH} -DMPARM
ASFLAGS = -mfpu=softfpa
LDFLAGS = -T$(VSOC_ROOT_DIR)/apps/support/simulator/vsoc.ld 
OMP_CFLAGS = -fopenmp -O1 -g -c -I${APPSUPPORT_PATH} -I${VSOC_CONFIG_PATH} -I${VSOC_SRC_DIR}/core  -fdump-tree-ompexp

EXE=o-optimize/app.exe

all: $(COBJS) $(ASOBJS) app_compile $(EXE)


app_compile: 
	$(CC) $(OMP_CFLAGS) $(SRCS_OMP) -o app_obj.o


$(EXE): app_obj.o $(COBJS) $(ASOBJS)
	test -d o-optimize || mkdir o-optimize
	$(LD) $(LDFLAGS) app_obj.o $(COBJS) $(ASOBJS) -o  linked.o
	$(OBJCOPY) -O binary linked.o $@
	ln -sf o-optimize/app.exe TargetMem_1.mem
	ln -sf o-optimize/app.exe TargetMem_2.mem
	ln -sf o-optimize/app.exe TargetMem_3.mem
	ln -sf o-optimize/app.exe TargetMem_4.mem

run :
	vsoc.x --tb=32 -c 4
	
clean:	
	rm -rf *.o *.s *.ompexp *.dis o-optimize  TargetMem_*
	rm -rf ${APPSUPPORT_PATH}/*.o ${LIBGOMP_PATH}/*.o ${VSOC_CONFIG_PATH}/*.o ${LIBGOMP_PATH}/lib/${LIBRARY_NAME}

disassemble:
	$(OBJDUMP) -S linked.o > linked.dis
	
