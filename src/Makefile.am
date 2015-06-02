## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"

AM_CFLAGS = \
	-O3
	-Wall\
	-g

lib_LTLIBRARIES = libbsp.la

libbsp_la_SOURCES = \
	bsp.h \
	core/bsp_debug.h \
	core/bsp_debug.c \
	core/bsp_misc.h \
	core/bsp_misc.c \
	core/bsp_tinyspin.h \
	core/bsp_tinyspin.c \
	core/bsp_mempool.h \
	core/bsp_mempool.c \
	core/bsp_event.h \
	core/bsp_event.c \
	core/bsp_thread.h \
	core/bsp_thread.c \
	core/bsp_bootstrap.h \
	core/bsp_bootstrap.c \
	ext/bsp_variable.h \
	ext/bsp_variable.c \
	ext/bsp_buffer.h \
	ext/bsp_buffer.c \
	ext/bsp_hash.h \
	ext/bsp_hash.c \
	utils/bsp_string.h \
	utils/bsp_string.c \
	utils/bsp_value.h \
	utils/bsp_value.c \
	utils/bsp_object.h \
	utils/bsp_object.c \
	utils/bsp_timer.c \
	utils/bsp_timer.h \
	net/bsp_socket.h \
	net/bsp_socket.c

libbsp_la_LDFLAGS = 

libbsp_la_LIBADD = 

nobase_pkginclude_HEADERS = \
	bsp.h \
	core/bsp_debug.h \
	core/bsp_misc.h \
	core/bsp_tinyspin.h \
	core/bsp_mempool.h \
	core/bsp_event.h \
	core/bsp_thread.h \
	core/bsp_bootstrap.h \
	ext/bsp_variable.h \
	ext/bsp_hash.h \
	ext/bsp_buffer.h \
	utils/bsp_string.h \
	utils/bsp_value.h \
	utils/bsp_object.h \
	utils/bsp_timer.h \
	net/bsp_socket.h

pkgconfigdir = $(datadir)/pkgconfig
pkgconfig_DATA = bsp.pc

EXTRA_DIST = \
	bsp.pc.in
