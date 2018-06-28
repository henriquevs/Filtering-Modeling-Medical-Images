include Makefile.defs

#MUST BE UPDATED
#OWN_VIRTUALSOC_DIR = /home/v/ventroux/perso/virtualsoc

#DO NOT TOUCH
VSOC_BUILD_DIR = ${OWN_VIRTUALSOC_DIR}/build
VSOC_ROOT_DIR = /opt/ensta/pack/virtualsoc/VirtualSoc
VSOC_SRC_DIR = ${OWN_VIRTUALSOC_DIR}/src
VSOC_3RD_PARTY_DIR = ${VSOC_ROOT_DIR}/3rd_party
LIB_SIMSOC = ${VSOC_3RD_PARTY_DIR}/src/LIBSIMSOC
SYSTEMC = /opt/ensta/pack/virtualsoc/systemc-2.2.0-patched
TLM_HOME = /opt/ensta/pack/virtualsoc/TLM-2.0.1
CC = g++
SYSC_TARGET_ARCH = linux64
VSOC_BIN_DIR = ${OWN_VIRTUALSOC_DIR}/bin

############## TESTS #############
ifndef VSOC_SRC_DIR
  $(error Fatal error: Undefined VSOC_SRC_DIR environment variable!)
endif
ifndef LIB_SIMSOC
  $(error Fatal error: Undefined LIB_SIMSOC environment variable!)
endif
ifndef TLM_HOME
  $(error Fatal error: Undefined TLM_HOME environment variable!)
endif

############## Initialization of core includes, libs, sources
INCDIR   += -I. -I.. -I$(VSOC_SRC_DIR)/core -I$(SYSTEMC)/include \
            -I${TLM_HOME}/include/tlm -I$(LIB_SIMSOC)/include/libsimsoc \
			-I$(LIB_SIMSOC)/include -I$(VSOC_3RD_PARTY_DIR)/src/DRAMSim2
LIBDIR   += -L$(SYSTEMC)/lib-$(SYSC_TARGET_ARCH) -L$(LIB_SIMSOC)/lib -L/usr/lib \
			-L$(VSOC_3RD_PARTY_DIR)/src/DRAMSim2 -L/usr/lib/x86_64-linux-gnu
LIBS     += -lsystemc -lsimsoc -ldramsim -lpthread -lmpfr -ldramsim

CPPSRCS  += main.cpp parser.cpp globals.cpp core_signal.cpp sim_support.cpp \
            mem_class.cpp power.cpp cache_RPU.cpp cache_storagetypes.cpp \
            cache_address.cpp cache_module.cpp address.cpp stats.cpp

CSRCS    +=
CPPOBJS   = $(CPPSRCS:.cpp=.o)
COBJS     = $(CSRCS:.c=.o)
OBJS      = $(CPPOBJS) $(COBJS)
VPATH    += $(VSOC_SRC_DIR)/core 
EXE       = $(VSOC_BIN_DIR)/$(MODULE).x


############## Inclusion of relevant architectural modules

ifeq ($(findstring cluster,$(MAKECMDGOALS)),cluster)
  include Makefile.cluster
endif

#ifeq ($(findstring multicluster,$(MAKECMDGOALS)),multicluster)
  #include Makefile.noc
  #include Makefile.noc2
#endif

ifeq ($(findstring all,$(MAKECMDGOALS)),all)
  #include Makefile.noc
  #include Makefile.noc2
  include Makefile.cluster
endif


############## Choice of compiler options to use 
ifeq ($(OPTIONSET),QUIET)
  CFLAGS    += ${OPT} ${INCDIR} ${LIBDIR} ${DEFAULTFLAGS} ${PLATFLAGS}
  CPPFLAGS  += ${CFLAGS}
endif
ifeq ($(OPTIONSET),VERBOSE)
  CFLAGS    += ${DEBUGOPTS} ${INCDIR} ${LIBDIR} ${DEFAULTFLAGS} ${PLATFLAGS}
  CPPFLAGS  += ${CFLAGS}
endif
ifdef REDUCED
  CFLAGS += -DREDUCED_RELEASE
endif


###### Parse Command Line For Verbose Flag
QUIET = @
ECHO  =
ifeq ($(BUILDOPTIONS),VERBOSE)
    QUIET =
    ECHO  = > /dev/null
endif


############## Actual build process: executable, objects, dependencies
all: cluster 

$(EXE): $(OBJS)
	$(QUIET)test -d "$(VSOC_BIN_DIR)" || mkdir "$(VSOC_BIN_DIR)" 
	$(QUIET)$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) 2>&1 | c++filt
	@echo $(ECHO)
	@echo "  [$(LINK)LINK$(DEFAULTCOLOR)]  :  $@"$(ECHO)
	@echo $(ECHO)
        
%.o:    %.cpp
	$(QUIET)$(CC) $(CFLAGS) -c $<
	@echo "  [$(COMPILE)CC$(DEFAULTCOLOR)]    :  $@"$(ECHO)

%.dpp:  %.cpp
	@echo "  [$(DEPENDENCY)DEP$(DEFAULTCOLOR)]   :  $@"$(ECHO)
	$(QUIET)set -e; $(CC) $(CFLAGS) -MM $< | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; [ -s $@ ] || rm -f $@
        
%.d:    %.c
	@echo "  [$(DEPENDENCY)DEP$(DEFAULTCOLOR)]   :  $@"$(ECHO)
	$(QUIET)set -e; $(CC) $(CFLAGS) -MM $< | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; [ -s $@ ] || rm -f $@




############## Cleanup routines
.PHONY : clean
.PHONY : cleanexe
.PHONY : cleanmain
.PHONY : printmessage
clean:
	@echo
	rm -f $(VSOC_BUILD_DIR)/*.o $(VSOC_BUILD_DIR)/*.dpp $(VSOC_BUILD_DIR)/*.d $(EXE)
	@echo
cleanexe:
	@echo
	rm -f $(EXE)
	@echo
cleanmain:
	@echo
	$(VSOC_BUILD_DIR)/main.*
	@echo
printmessage:
	@echo
	@echo Building platform with: $(MSGSTRING)
	@echo
	@echo Compilation line is: $(ECHO)
	@echo $(ECHO)
	@echo $(CC) $(CFLAGS) $(ECHO)
	@echo $(ECHO)



############## Inclusion of dependencies. Beware: set to discard warnings, so it won't notice errors...
ifneq ($(MAKECMDGOALS),cleanmain)
  ifneq ($(MAKECMDGOALS),clean)
    ifneq ($(MAKECMDGOALS),cleanexe)
      ifeq ($(findstring cpp,$(CPPSRCS)),cpp)
        -include $(CPPSRCS:.cpp=.dpp)
      endif
      ifeq ($(findstring c,$(CSRCS)),c)
        -include $(CSRCS:.c=.d)
      endif
    endif
  endif
endif




############## Color Codes
RED          = [01;31m
GREEN        = [01;32m
YELLOW       = [01;33m
BLUE         = [01;34m
VIOLET       = [01;35m
CYAN         = [01;36m
WHITE        = [01;37m
DEFAULTCOLOR = [00m
PLAIN        =
