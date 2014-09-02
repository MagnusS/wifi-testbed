#!/usr/bin/make

TARGET      = aggie

OBJECTS     = main aggie messagelist cmdline stringutils vout config \
              ipsocket jsoncpp timetools wclient

HEADERS     = platform.h aggie.hpp messagelist.hpp resulthandler.hpp \
              stringutils.hpp vout.hpp cmdline.hpp config.hpp ipsocket.hpp \
              color_streams.h threadable.hpp timetools.hpp wclient.hpp


PREDEPEND   = jsoncpp.cpp json/json.h

WINLIBS     = 
LINUXLIBS   = pthread

DEFS        += 
UNDEFS      += 

OBJS        = $(foreach i,$(OBJECTS),$(OUTPATH)/$(i).o)
LIBS        = 
LDIR        = ./lib $(CCROOT)/lib
IDIR        = ./ $(CCROOT)/include $(CCROOT)

CFLAGS     += 

IN_MAKEFILE = 1

ifeq ($(PLATFORM),)
	ifneq ($(OS),)
		PLATFORM = WINDOWS
	else
		PLATFORM = LINUX
	endif
endif

ifeq ($(PLATFORM),WINDOWS)
	CCROOT   = mingw
	CCCMD    = g++
	OUTFILE  = $(TARGET).exe
	LIBS    += $(WINLIBS)
	DEFS    += PLATFORM=WINDOWS
	RM       = del
	RMR      = rmdir /q /s
	RD       = rmdir
	MD       = mkdir
	CP       = copy
	CPR      = xcopy
	PATHSEP  = \\
	PLATFORMDIR = w32
	DEVNULL  = nul
	SHELL    = cmd.exe
else 
ifeq ($(PLATFORM),LINUX)
	CCROOT   = /usr
	CCCMD    = g++
	OUTFILE  = $(TARGET)
	LIBS    += $(LINUXLIBS)
	DEFS    += PLATFORM=LINUX
	RM       = rm -f
	RMR      = rm -rf
	RD       = rmdir
	MD       = mkdir
	CP       = cp
	CPR      = cp -r
	PATHSEP  = /
	PLATFORMDIR = x86
	DEVNULL  = /dev/null
	CFLAGS  += 
else
	ERROR    = error_invalidplatform
endif
endif

ifneq ($(IDIR),)
	INCLUDEDIRS = $(foreach i,$(IDIR),-I $(i))
endif

ifneq ($(LDIR),)
	LIBRARYDIRS = $(foreach i,$(LDIR),-L $(i))
endif

ifneq ($(LIBS),)
	LIBRARIES = $(foreach i,$(LIBS),-l$(i))
endif

ifneq ($(DEFS),)
	DEFINES = $(foreach i,$(DEFS),-D$(i))
endif

ifneq ($(UNDEFS),)
	DEFINES += $(foreach i,$(UNDEFS),-U$(i))
endif

ifeq ($(OUTDIR),)
	include Makefile.debug.mk    # standard target
endif

OUTMAIN  = .
OUTPATH  = $(OUTMAIN)$(PATHSEP)$(OUTDIR)$(PATHSEP)$(PLATFORMDIR)
CC       = $(CCCMD)
CFLAGS	+= $(DEFINES)
LDFLAGS += 

ABORTMAKE	= false

.PHONY:	all release debug clean help doxygen docs directories

.SILENT:	directories

all:	$(ERROR) directories $(PREDEPEND) $(OUTPATH)/$(OUTFILE);

debug:	all;

release:	
	$(MAKE) -f Makefile.release.mk

directories:	$(OUTMAIN)$(PATHSEP)$(OUTDIR)$(PATHSEP)$(PLATFORMDIR)

jsoncpp.cpp:	json
	$(CP) jsoncpp/dist/jsoncpp.cpp .

json/json.h: json
	$(CPR) jsoncpp/dist/json .

json:	
	cd jsoncpp && python amalgamate.py

$(OUTMAIN)$(PATHSEP)$(OUTDIR)$(PATHSEP)$(PLATFORMDIR):	
	-$(MD) $(OUTMAIN)
	-$(MD) $(OUTMAIN)$(PATHSEP)$(OUTDIR)
	-$(MD) $(OUTMAIN)$(PATHSEP)$(OUTDIR)$(PATHSEP)$(PLATFORMDIR)

help:	printhelp_target printhelp_platform;

echo print_platform:
	@echo $(PLATFORM)

printhelp_target:	
	@echo Specify wich target you wish to make \(e.g. 'debug' or 'release'\)
	@echo like this: 'make debug' or 'make release'. 'make' on its own is
	@echo equivalent to 'make debug'.

printerror_target:	
	@echo Error: No target specified.

error_notarget:	printerror_target printhelp_target
	@$(ABORTMAKE)

printhelp_platform:	
	@echo "Override platform detection (e.g. 'windows' or 'linux') by specifying"
	@echo "PLATFORM=WINDOWS or PLATFORM=LINUX (all uppercase) on the command line."

printerror_platform:	
	@echo Error: Platform could not be detected, or an invalid platform was specified.
	@echo Platform was set to '$(PLATFORM)'.

error_invalidplatform:	printerror_platform printhelp_platform
	@$(ABORTMAKE)

$(OUTPATH)/$(OUTFILE):	$(OBJS) $(HEADERS)
	$(CC) $(LDFLAGS) -o $@ $(INCLUDEDIRS) $(LIBRARYDIRS) $(LIBRARIES) $(OBJS)
	-cp $(OUTPATH)/$(OUTFILE) .

-include $(OBJS:.o=.d)

$(OUTPATH)/%.o	:	%.cpp
	$(CC) -c $(CFLAGS) -o $@ $(INCLUDEDIRS) $<
	$(CC) -MM -MT $(OUTPATH)/$*.o $(CFLAGS) $(INCLUDEDIRS) $*.cpp > $(OUTPATH)/$*.d

clean:	;-$(RM) $(OBJS) $(OBJS:.o=.d) $(OUTPATH)/$(OUTFILE)
	-$(RMR) json jsoncpp.cpp
	-$(RM) aggie

cleanbak:	;-$(RM) *~ *.bak

doxygen:	Doxyfile
	doxygen
