# Copyright (C) 2000 MySQL AB & MySQL Finland AB & TCX DataKonsult AB
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

INCLUDES =		@MT_INCLUDES@ -I$(srcdir)/../include -I../include
pkglib_LIBRARIES =	libmyisammrg.a
noinst_HEADERS =	mymrgdef.h
libmyisammrg_a_SOURCES = myrg_open.c myrg_extra.c myrg_info.c myrg_locking.c \
			myrg_rrnd.c myrg_update.c myrg_delete.c myrg_rsame.c \
			myrg_panic.c myrg_close.c myrg_create.c myrg_static.c \
			myrg_rkey.c myrg_rfirst.c myrg_rlast.c myrg_rnext.c \
			myrg_rprev.c myrg_queue.c

# Don't update the files from bitkeeper
%::SCCS/s.%
