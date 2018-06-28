############## Name of output executable file
ifdef EXENAME
EXE=o-optimize/$(EXENAME).exe
else
EXE=o-optimize/app.exe
endif

#################### COMPILATION SETTINGS ##########################
## Default compiler and possible option sets
CC        = arm-elf-gcc
LD        = arm-elf-ld
AS        = arm-elf-as
OBJCOPY   = arm-elf-objcopy
OBJDUMP   = arm-elf-objdump

## Paths
#VSOC_APP_DIR	  = ${VSOC_DIR}/apps
VSOC_SRC_DIR	  = ${OWN_VIRTUALSOC_DIR}/src
LIBGOMP_PATH      = ${VSOC_APP_DIR}/libgomp
VSOC_CONFIG_PATH  = ${LIBGOMP_PATH}/config/vsoc
SUPPORT_PATH	  = ${VSOC_APP_DIR}/support/simulator

## Library FLAGS
# Enables safe memory allocation check
LIBGOMP_FLAGS	+= -DMEMCHECK_MALLOCS
LIBGOMP_FLAGS	+= -DSTACK_IN_SHARED
LIBGOMP_FLAGS	+= -DSINGLE_WS

## Other FLags
#Uncomment and replace for OpenMP
#INCDIR       	= -I. -I.. -I${SUPPORT_PATH} -I${LIBGOMP_PATH} -I${VSOC_CONFIG_PATH} -I${VSOC_SRC_DIR}/core 
INCDIR       	= -I. -I.. -I${SUPPORT_PATH} -I${VSOC_SRC_DIR}/core 
COMMON_CFLAGS 	= $(INCDIR) -msoft-float -mcpu=arm7 -Winline
CFLAGS	 	= -O2 $(COMMON_CFLAGS) $(LIBGOMP_FLAGS)
ASFLAGS    	= -mfpu=softfpa
LDFLAGS	    	= -L${VSOC_APP_DIR}/support/lib -lgcc

#################### FILES ##########################
CSRCS  += ${VSOC_APP_DIR}/support/simulator/appsupport.c 
#CSRCS  += ${VSOC_APP_DIR}/support/simulator/accsupport.c
#CSRCS  += ${VSOC_APP_DIR}/support/simulator/countersupport.c
CSRCS  += ${VSOC_APP_DIR}/support/simulator/outmsupport.c
OMP_CSRCS  = main.c

ifeq ($(openmp),yes)		
	CSRCS  += ${LIBGOMP_PATH}/root.c
	CFLAGS += -DOPENMP
	OMP_CFLAGS = -O1 ${COMMON_CFLAGS} -fopenmp $(LIBGOMP_FLAGS) -DOPENMP
else
	OMP_CFLAGS = $(CFLAGS)
endif

ASSRCS = ${SUPPORT_PATH}/boot.s


#################### OBJECTS ##########################
COBJS  		= $(CSRCS:.c=.o)
ASOBJS 		= $(ASSRCS:.s=.o)
OMP_COBJS  	= $(OMP_CSRCS:.c=.o)
OBJS   		= $(COBJS) $(ASOBJS) $(OMP_COBJS)


#################### BUILD ##########################
all: $(EXE)

$(EXE): $(OBJS)
	test -d o-optimize || mkdir o-optimize
	$(CC) $(OMP_CFLAGS) -c -o $(OMP_COBJS) $(OMP_CSRCS)
	$(LD) -T ${SUPPORT_PATH}/vsoc.ld -o linked.o $(OBJS) $(LDFLAGS)
	$(OBJCOPY) -O binary linked.o $@
	ln -sf o-optimize/app.exe TargetMem_1.mem

run:
	vsoc.x --tb=32 -c 1 --intc=c --ics=16

run_stat:
	vsoc.x --tb=32 -c 1 --intc=c --ics=16 -s

run_openmp:
	vsoc.x --tb=32 -c 8 --intc=c --ics=16

.PHONY : clean disassemble

clean:
	rm -f *.o *~
	rm -f ${LIBGOMP_PATH}/*.o
	rm -f ${SUPPORT_PATH}/*.o
	rm -rf o-optimize
	rm -f TargetMem* stats* results.pgm

disassemble: $(EXE)
	$(OBJDUMP) -SD linked.o > linked.dis

