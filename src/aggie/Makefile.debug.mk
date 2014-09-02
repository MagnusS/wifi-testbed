#!make
CFLAGS  += -g
LDFLAGS += -g
DEFS    += DEBUG
OUTDIR   = debug

ifneq ($(IN_MAKEFILE),1)
	  include Makefile
endif
