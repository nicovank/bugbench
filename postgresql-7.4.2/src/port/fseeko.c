/*-------------------------------------------------------------------------
 *
 * fseeko.c
 *	  64-bit versions of fseeko/ftello()
 *
 * Portions Copyright (c) 1996-2003, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /cvsroot/pgsql/src/port/fseeko.c,v 1.12 2003/08/04 02:40:20 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */

/*
 * We have to use the native defines here because configure hasn't
 * completed yet.
 */
#if defined(__bsdi__) || defined(__NetBSD__)

#include "c.h"

#ifdef bsdi
#include <pthread.h>
#endif
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

/*
 *	On BSD/OS and NetBSD, off_t and fpos_t are the same.  Standards
 *	say off_t is an arithmetic type, but not necessarily integral,
 *	while fpos_t might be neither.
 *
 *	This is thread-safe on BSD/OS using flockfile/funlockfile.
 */

int
fseeko(FILE *stream, off_t offset, int whence)
{
	off_t		floc;
	struct stat filestat;

	switch (whence)
	{
		case SEEK_CUR:
#ifdef bsdi
			flockfile(stream);
#endif
			if (fgetpos(stream, &floc) != 0)
				goto failure;
			floc += offset;
			if (fsetpos(stream, &floc) != 0)
				goto failure;
#ifdef bsdi
			funlockfile(stream);
#endif
			return 0;
			break;
		case SEEK_SET:
			if (fsetpos(stream, &offset) != 0)
				return -1;
			return 0;
			break;
		case SEEK_END:
#ifdef bsdi
			flockfile(stream);
#endif
			if (fstat(fileno(stream), &filestat) != 0)
				goto failure;
			floc = filestat.st_size;
			if (fsetpos(stream, &floc) != 0)
				goto failure;
#ifdef bsdi
			funlockfile(stream);
#endif
			return 0;
			break;
		default:
			errno = EINVAL;
			return -1;
	}

failure:
#ifdef bsdi
	funlockfile(stream);
#endif
	return -1;
}


off_t
ftello(FILE *stream)
{
	off_t		floc;

	if (fgetpos(stream, &floc) != 0)
		return -1;
	return floc;
}

#endif
