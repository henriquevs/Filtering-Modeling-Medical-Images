
############## Name of output executable file
ifdef EXENAME
EXE=o-optimize/$(EXENAME).exe
else
EXE=o-optimize/app.exe
endif

PAR1 = 
PAR2 =
############## Default compiler and possible option sets
OMP_CC    = arm-elf-gcc
CC        = arm-elf-gcc
LD        = arm-elf-ld
AS        = arm-elf-as
OBJCOPY   = arm-elf-objcopy
OBJDUMP   = arm-elf-objdump

LIBGOMP_PATH        = ${VSOC_APP_DIR}/libgomp
VSOC_CONFIG_PATH	= ${LIBGOMP_PATH}/config/vsoc
SUPPORT_PATH		= ${VSOC_APP_DIR}/support/simulator


############## Initialization of global includes and libs
INCDIR       = -I. -I.. -I${SUPPORT_PATH} -I${LIBGOMP_PATH} -I${VSOC_CONFIG_PATH} -I${VSOC_SRC_DIR}/core 

####################### Library FLAGS #############################
## Enables safe memory allocation check
LIBGOMP_FLAGS	+= -DMEMCHECK_MALLOCS
LIBGOMP_FLAGS	+= -DSTACK_IN_SHARED
## Enable timers
# LIBGOMP_FLAGS += -DSTATS_ENABLE

####### Work-specific
## Enable support for work shares: #pragma omp single, sections and (dynamic) loops
## Only one global work share (1)
# LIBGOMP_FLAGS += -DGLOBAL_WS
## Only one work share/team (2)
LIBGOMP_FLAGS	+= -DSINGLE_WS
## Otherwise, full support for several ws and nowait (3)

############ HAL
## ASM implementation of gomp_hal_lock (faster is disabled: see lock.c)
# LIBGOMP_FLAGS	+= -DASM_LOCKS_FN
## ASM implementation of MSGBarrier_SlaveEnter
# LIBGOMP_FLAGS	+= -DASM_BAR_SLAVEENTER_FN

## Different barrier implementations:
## HW barrier (TODO selective HW barrier)
# LIBGOMP_FLAGS	+= -DMPARM_HW_BAR

############## Tasking-specific flags
# LIBGOMP_FLAGS += -DTASKING_ENABLED
# ....TODO add support for all of them...
# Locking strategies: Idle/wake (if enabled) vs libgomp busy-waiting locks
# LIBGOMP_FLAGS += -DTASK_LOCK_IDLE
# Busy-waiting strategies for thread dock: one of them MUST be defined!
# LIBGOMP_FLAGS += -DTASK_BARRIER_BUSY_WAITING
# LIBGOMP_FLAGS += -DTASK_BARRIER_WAKE_ONE
# LIBGOMP_FLAGS += -DTASK_BARRIER_WAKE_ALL
# LIBGOMP_FLAGS	+= -DMAX_TASKS=384

####################### Other FLAGS ##################################
COMMON_CFLAGS = $(INCDIR)  -msoft-float -mcpu=arm7 -Winline

CFLAGS	 	= -O2 -g $(COMMON_CFLAGS) $(LIBGOMP_FLAGS) ${PAR2}
ASFLAGS    	= -mfpu=softfpa

OMP_CFLAGS   	= -O2 -Wall ${COMMON_CFLAGS} ${PAR1} -fopenmp -DSIZE320

################### LIBGOMP/APPSUPPORT Sources ########################
CSRCS  += ${LIBGOMP_PATH}/root.c
CSRCS  += ${SUPPORT_PATH}/appsupport.c ${SUPPORT_PATH}/dmasupport.c 

LDFLAGS	    += -L${VSOC_APP_DIR}/support/lib -lgcc

## Boot
ASSRCS = ${SUPPORT_PATH}/boot.s

############################ OpenMP Sources ############################

OMP_CSRCS  += ctracking.c

############################ OBJECTS ###################################

COBJS  = $(CSRCS:.c=.o)
ASOBJS = $(ASSRCS:.s=.o)
OMP_COBJS  = $(OMP_CSRCS:.c=.o)

OBJS   = $(COBJS) $(ASOBJS)

############## Actual build process

all: $(EXE)

$(EXE): $(OBJS)
	test -d o-optimize || mkdir o-optimize
	$(OMP_CC) $(OMP_CFLAGS) -c -o $(OMP_COBJS) $(OMP_CSRCS)
	$(LD) -T ${SUPPORT_PATH}/vsoc.ld -o linked.o $(OBJS) $(OMP_COBJS) $(LDFLAGS)
	$(OBJCOPY) -O binary linked.o $@
	ln -sf o-optimize/app.exe TargetMem_1.mem
	ln -sf o-optimize/app.exe TargetMem_2.mem
	ln -sf o-optimize/app.exe TargetMem_3.mem
	ln -sf o-optimize/app.exe TargetMem_4.mem

run:
	vsoc.x --tb=32 -c16 --intc=c

############## Support routines
.PHONY : clean disassemble
clean:
	rm -f *.o *~
	rm -rf ${LIBGOMP_PATH}/*.o
	rm -rf ${SUPPORT_PATH}/*.o
	rm -rf o-optimize
	rm -f TargetMem*

disassemble: $(EXE)
	$(OBJDUMP) -D linked.o > linked.dis
