/*-------------------------------------------------------------------------
 *
 *	  WIN1256 and UTF-8
 *
 * Portions Copyright (c) 1996-2003, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  $Header: /cvsroot/pgsql/src/backend/utils/mb/conversion_procs/utf8_and_win1256/Attic/utf8_and_win1256.c,v 1.6 2003/08/04 02:40:08 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "fmgr.h"
#include "mb/pg_wchar.h"
#include "../../Unicode/utf8_to_win1256.map"
#include "../../Unicode/win1256_to_utf8.map"

PG_FUNCTION_INFO_V1(utf_to_win1256);
PG_FUNCTION_INFO_V1(win1256_to_utf);

extern Datum utf_to_win1256(PG_FUNCTION_ARGS);
extern Datum win1256_to_utf(PG_FUNCTION_ARGS);

/* ----------
 * conv_proc(
 *		INTEGER,	-- source encoding id
 *		INTEGER,	-- destination encoding id
 *		CSTRING,	-- source string (null terminated C string)
 *		CSTRING,	-- destination string (null terminated C string)
 *		INTEGER		-- source string length
 * ) returns VOID;
 * ----------
 */

Datum
utf_to_win1256(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);

	Assert(PG_GETARG_INT32(0) == PG_UTF8);
	Assert(PG_GETARG_INT32(1) == PG_WIN1256);
	Assert(len >= 0);

	UtfToLocal(src, dest, ULmapWIN1256,
			   sizeof(ULmapWIN1256) / sizeof(pg_utf_to_local), len);

	PG_RETURN_VOID();
}

Datum
win1256_to_utf(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);

	Assert(PG_GETARG_INT32(0) == PG_WIN1256);
	Assert(PG_GETARG_INT32(1) == PG_UTF8);
	Assert(len >= 0);

	LocalToUtf(src, dest, LUmapWIN1256,
		sizeof(LUmapWIN1256) / sizeof(pg_local_to_utf), PG_WIN1256, len);

	PG_RETURN_VOID();
}
