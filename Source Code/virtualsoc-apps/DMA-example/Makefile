############## Name of output executable file
ifdef EXENAME
EXE=o-optimize/$(EXENAME).exe
else
EXE=o-optimize/app.exe
endif

############## Default compiler and possible option sets
CC        = arm-elf-gcc
LD        = arm-elf-ld
AS        = arm-elf-as
OBJCOPY   = arm-elf-objcopy
OBJDUMP   = arm-elf-objdump

OPT       = -O2 -g
# DEFINES	  =-D${CASE}

ifndef VSOC_ROOT_DIR
  $(error Fatal error: Undefined VSOC_ROOT_DIR environment variable!)
endif

############## Initialization of global includes and libs
INCDIR       = -I. -I.. -I${VSOC_SRC_DIR}/core -I${VSOC_APP_DIR}/support/simulator
CFLAGS       = $(INCDIR) $(OPT) $(DEFINES) 
#CFLAGS      += -DVERBOSE
#CFLAGS      += -DULTRADEBUG
ASFLAGS      = -mfpu=softfpa
VPATH       += ${VSOC_APP_DIR}/support/simulator

############## Initialization of source and object files
CSRCS  = ${VSOC_APP_DIR}/support/simulator/appsupport.c access_tests.c
CSRCS  += ${VSOC_APP_DIR}/support/simulator/dmasupport.c
ASSRCS = ${VSOC_APP_DIR}/support/simulator/boot.s
COBJS  = $(CSRCS:.c=.o)
ASOBJS = $(ASSRCS:.s=.o)
OBJS   = $(COBJS) $(ASOBJS)

############## Actual build process
$(EXE): $(OBJS)
	test -d o-optimize || mkdir o-optimize
	$(LD) -T ${VSOC_APP_DIR}/support/simulator/vsoc.ld -o linked.o $(OBJS) $(LDFLAGS)
	$(OBJCOPY) -O binary linked.o $@
	ln -sf o-optimize/app.exe TargetMem_1.mem
	ln -sf o-optimize/app.exe TargetMem_2.mem
	ln -sf o-optimize/app.exe TargetMem_3.mem
	ln -sf o-optimize/app.exe TargetMem_4.mem

############## Support routines
.PHONY : clean disassemble

all: $(EXE)

run:
	vsoc.x --tb=32 -c4 --intc=c

clean:
	rm -f *.o *.dis *~ stats.txt stats_light.txt TargetMem_*
	rm -rf o-optimize/
	rm -f ${VSOC_APP_DIR}/support/simulator/appsupport*.o

disassemble: $(EXE)
	$(OBJDUMP) -SD linked.o > linked.dis
