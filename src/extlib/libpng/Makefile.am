# libpng: PNG library.

AUTOMAKE_OPTIONS = foreign subdir-objects

noinst_LTLIBRARIES = libpng.la

libpng_la_CFLAGS	= $(AM_CFLAGS) -O3 -funroll-loops -DPNG_PEDANTIC_WARNINGS
libpng_la_LDFLAGS	= $(AM_LDFLAGS)
libpng_la_LIBTOOLFLAGS	= $(AM_LIBTOOLFLAGS) --tag=disable-shared

if !GENS_X86_ASM
libpng_la_CFLAGS += -DPNG_NO_MMX_CODE
endif # !GENS_X86_ASM

if GENS_ZLIB_INTERNAL
INCLUDES = -I@srcdir@/../zlib/
endif # GENS_ZLIB_INTERNAL

libpng_la_SOURCES = \
		png.c \
		pngset.c \
		pngget.c \
		pngrutil.c \
		pngtrans.c \
		pngwutil.c \
		pngread.c \
		pngrio.c \
		pngwio.c \
		pngwrite.c \
		pngrtran.c \
		pngwtran.c \
		pngmem.c \
		pngerror.c \
		pngpread.c

noinst_HEADERS = \
		png.h \
		pngconf.h
