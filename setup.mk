# this file sets up variables further, based on the values from config.mak
-include config.mak
ifeq ($(TARGET_OS),SunOS)
    LDFLAGS= -G -Wl,-h,lib$(SHORTLIB).so
else
# This works for linux, dunno about others
	LDFLAGS= -shared -Wl,-soname,lib$(SHORTLIB).so
endif


CFLAGS += -ftabstop=4 -Wall -Wbad-function-cast -Wcast-align -Wwrite-strings
CFLAGS += -Wnonnull -Wno-attributes
#CFLAGS += -Wunreachable-code

override CPUFLAGS =
ifeq ($(TARGET_MMX),yes)
    override CPUFLAGS +=-mmmx
endif
ifeq ($(TARGET_SSE2),yes)
    override CPUFLAGS +=-msse2 -mfpmath=sse,387
else 
	ifeq ($(TARGET_SSE),yes)
    	override CPUFLAGS +=-msse -mfpmath=sse,387 
    endif
endif

APPSTATMSG=
DEFFLAGS=

APPSTATMSG += Compiler Debug messages are
ifeq ($(DEBUG_COMP),yes)
	DEFFLAGS +=-DDEBUG_COMP
	APPSTATMSG += ON\\n
else
	APPSTATMSG += OFF\\n
endif

APPSTATMSG += Interpreter execution tracing is
ifeq ($(TRACE_INTERP),yes)
	DEFFLAGS += -D_COMPSL_TRACE
	APPSTATMSG += ON\\n
else
	APPSTATMSG += OFF\\n
endif

#cause the optimizer to output code dumps as it runs each stage
APPSTATMSG += Tracing bytecode optimizer is
ifeq ($(TRACE_OPTIM),yes)
	DEFFLAGS += -DCOMPSL_TRACE_OPTIM
	APPSTATMSG += ON\\n
else
	APPSTATMSG += OFF\\n
endif

# make the compiler output the line number it's currently compiling 
# to stdout
APPSTATMSG += Tracing compile is
ifeq ($(TRACE_COMPILE),yes)
	DEFFLAGS += -DCOMPSL_TRACE_COMPILE
	APPSTATMSG += ON\\n
else
	APPSTATMSG += OFF\\n
endif

APPSTATMSG += Compile time bytecode stack bounds checking is
ifeq ($(STACK_CHECK),yes)
	DEFFLAGS += -DCOMP_STACKCHECK
	APPSTATMSG += ON\\n
else
	APPSTATMSG += OFF\\n
endif

APPSTATMSG += Mudflap pointer debuging is
ifeq ($(MUDFLAP_ENABLE),yes)
	MUDFLAP_FLAGS += -fmudflap
	PLATLIBS += -lmudflap 
	APPSTATMSG += ON\\n
else
	APPSTATMSG += OFF\\n
endif

ifdef DEBUG
	# -fmudflap is some pointer debuging stuff
	# -fno-builtins since we can't set breakpoints on calls to builtin functions
	# -fstack-protector checks for stack overruns
	CFLAGS += -ggdb3 -fno-builtin -fstack-protector
else
	CFLAGS += -fbuiltin
	DEFFLAGS += -DNDEBUG
endif

ifeq ($(OPTIMIZE),FULL)
	# don't use O3 since we really probably don't need it and we don't
	# know if it really procuces better code for us
	OPTFLAGS += -O2 -fmerge-all-constants -fmodulo-sched -fgcse-after-reload
	#these are what -O3 turns on
	#-finline-functions -funswitch-loops -fgcse-after-reload
	
	#OPTFLAGS += -funroll-loops -fsee -fipa-pta
	#OPTFLAGS += -fsched-spec-load -maccumulate-outgoing-args
	OPTFLAGS += -minline-all-stringops 
	OPTFLAGS += -fno-stack-limit
	
	#OPTFLAGS +=-fdata-sections -ffunction-sections
	# TODO: make sure none of these breaks the library for linking....
	OPTFLAGS += -fstrict-aliasing -Wstrict-aliasing=2 
	
	#OPTFLAGS += -fgcse-sm -fgcse-las
	#OPTFLAGS += -ftree-vectorize -ftree-loop-linear -ftree-loop-im -fivopts -ftree-loop-ivcanon
	#according to gentoo wiki tracer may not work... 
	#OPTFLAGS += -ftracer -fsched2-use-traces
	#OPTFLAGS += -fsplit-ivs-in-unroller -fvariable-expansion-in-unroller
	OPTFLAGS += -freorder-blocks-and-partition
	#OPTFLAGS += -fprefetch-loop-arrays
	
	OPTFLAGS += -fbranch-target-load-optimize 
	#OPTFLAGS += -fbranch-target-load-optimize2
	#OPTFLAGS += -floop-optimize2 -fmove-all-movables
	
	###################################################
	# potentially bad optimizations
	###################################################
	# from gcc manual
	# This option is experimental, as not all machine descriptions used by GCC 
	# model the CPU closely enough to avoid unreliable results from the algorithm.
	#OPTFLAGS += -fsched2-use-superblocks
	
	#can we use this?
	#-fmerge-all-constants Attempt to merge identical constants and identical variables.
	#  This option implies -fmerge-constants. In addition to -fmerge-constants this considers 
	#  e.g. even constant initialized arrays or initialized constant variables with integral or 
	#  floating point types. Languages like C or C++ require each non-automatic variable to have 
	#  distinct location, so using this option will result in non-conforming behavior.
else
	OPTFLAGS += -O$(OPTIMIZE)
endif

MATH_FLAGS =-fsingle-precision-constant -fno-math-errno
MATH_FLAGS += -fno-trapping-math -ffast-math 

APPSTATMSG += IEEE compliant floating point is
ifeq ($(IEEE_FLOATS),yes)
	APPSTATMSG += ON\\n
else
	MATH_FLAGS += -ffinite-math-only -mno-ieee-fp -funsafe-math-optimizations
	APPSTATMSG += OFF\\n
endif

# for shared library
ifneq ($(TARGET_WIN32),yes)
	CFLAGS += -fvisibility=hidden -fpic
else
	DEFFLAGS += -DWINDOWS
endif

DEFFLAGS +=-D_GNU_SOURCE -DBUILDING_COMPSL

#PP_FLAGS = -I $(abspath src/include)
PP_FLAGS = -I "$(shell pwd)/src/include"

ALL_CFLAGS = -std=gnu99 $(CPUFLAGS) $(OPTFLAGS) $(MATH_FLAGS) $(CFLAGS) $(DEFFLAGS)

override ALL_CFLAGS := $(shell CC=$(CC) ./gcc-optioncheck $(ALL_CFLAGS)) $(PP_FLAGS) $(MUDFLAP_FLAGS)

STATMSG  = Compiling with $(CC) version $(shell $(CC) -dumpversion)
STATMSG += targeting $(TARGET_CPU) on $(TARGET_OS) for
ifdef DEBUG
	STATMSG += Debugging\\n
else
	STATMSG += Release\\n
endif
ifeq ($(OPTIMIZE),0)
	STATMSG += Optimization is OFF\\n
else
	STATMSG += Optimization is ON at $(OPTIMIZE)\\n
endif
ifeq ($(CPUGUESS),yes)
    STATMSG += Guessed CPUTYPE of $(GUESSED_CPU)\\n
    STATMSG +=    Adding CPU flags $(CPUFLAGS)\\n
else
    STATMSG += Compiling for $(TARGET_CPU) with $(CPUFLAGS)\\n
endif

ifdef PLATLIBS
	STATMSG += Platform specifice librarys to link $(patsubst -l%,lib%,$(PLATLIBS))\\n
endif
STATMSG += $(APPSTATMSG)

STATMSG += \\nTools\\n

STATMSG +=BISON           = $(BISON)\\n
STATMSG +=FLEX            = $(FLEX)\\n
STATMSG +=LIBTOOL         = $(LIBTOOL)\\n
STATMSG +=INSTALL         = $(INSTALL)\\n
STATMSG +=INSTALL_PROGRAM = $(INSTALL_PROGRAM)\\n
STATMSG +=INSTALL_DATA    = $(INSTALL_DATA)\\n
STATMSG +=LDCONFIG        = $(LDCONFIG)\\n
STATMSG +=RANLIB          = $(RANLIB)\\n
STATMSG +=DOXYGEN         = $(DOXYGEN)\\n

STATMSG +=\\nDirectories\\n
STATMSG +=prefix      = $(prefix)\\n
STATMSG +=exec_prefix = $(prefix)\\n
STATMSG +=bindir      = $(bindir)\\n
STATMSG +=datadir     = $(datadir)\\n
STATMSG +=sysconfdir  = $(sysconfdir)\\n
STATMSG +=includedir  = $(includedir)\\n
STATMSG +=docdir      = $(docdir)\\n
STATMSG +=libdir      = $(libdir)\\n
STATMSG +=mandir      = $(mandir)\\n

STATMSG +=\\n