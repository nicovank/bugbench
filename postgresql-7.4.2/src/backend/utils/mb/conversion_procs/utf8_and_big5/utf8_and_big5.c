/*-------------------------------------------------------------------------
 *
 *	  BIG5 <--> UTF-8
 *
 * Portions Copyright (c) 1996-2003, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  $Header: /cvsroot/pgsql/src/backend/utils/mb/conversion_procs/utf8_and_big5/utf8_and_big5.c,v 1.6 2003/08/04 02:40:07 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "fmgr.h"
#include "mb/pg_wchar.h"
#include "../../Unicode/big5_to_utf8.map"
#include "../../Unicode/utf8_to_big5.map"

PG_FUNCTION_INFO_V1(big5_to_utf8);
PG_FUNCTION_INFO_V1(utf8_to_big5);

extern Datum big5_to_utf8(PG_FUNCTION_ARGS);
extern Datum utf8_to_big5(PG_FUNCTION_ARGS);

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
big5_to_utf8(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);

	Assert(PG_GETARG_INT32(0) == PG_BIG5);
	Assert(PG_GETARG_INT32(1) == PG_UTF8);
	Assert(len >= 0);

	LocalToUtf(src, dest, LUmapBIG5,
			   sizeof(LUmapBIG5) / sizeof(pg_local_to_utf), PG_BIG5, len);

	PG_RETURN_VOID();
}

Datum
utf8_to_big5(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);

	Assert(PG_GETARG_INT32(0) == PG_UTF8);
	Assert(PG_GETARG_INT32(1) == PG_BIG5);
	Assert(len >= 0);

	UtfToLocal(src, dest, ULmapBIG5,
			   sizeof(ULmapBIG5) / sizeof(pg_utf_to_local), len);

	PG_RETURN_VOID();
}
