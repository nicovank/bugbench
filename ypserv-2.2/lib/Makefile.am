#
# Copyright (c) 2000, 2001 Thorsten Kukuk <kukuk@suse.de>
#
# Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = 1.4 gnits

rpcsvcdir = ${prefix}/include/rpcsvc

noinst_LIBRARIES = libyp.a
noinst_HEADERS = log_msg.h yp.h ypserv_conf.h ypxfrd.h access.h yp_db.h

rpcsvc_HEADERS = ypxfrd.x

CFLAGS = @EXTRA_CFLAGS@ @CFLAGS@
DEFS = @DEFS@ -D_REENTRANT=1 -DCONFDIR=\"$(sysconfdir)\"
INCLUDES = -I$(top_srcdir) -I$(top_builddir) -I$(srcdir)

libyp_a_SOURCES = log_msg.c ypserv_conf.c ypxfrd_xdr.c \
		ypproc_match_2.c securenets.c access.c yp_db.c

CLEANFILES = *~

EXTRA_DIST = ${MANS}

man_MANS = ypserv.conf.5

