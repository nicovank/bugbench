/*-------------------------------------------------------------------------
 *
 *	  Cyrillic and MULE_INTERNAL
 *
 * Portions Copyright (c) 1996-2003, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  $Header: /cvsroot/pgsql/src/backend/utils/mb/conversion_procs/cyrillic_and_mic/cyrillic_and_mic.c,v 1.6 2003/08/04 02:40:07 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "fmgr.h"
#include "mb/pg_wchar.h"

#define ENCODING_GROWTH_RATE 4

PG_FUNCTION_INFO_V1(koi8r_to_mic);
PG_FUNCTION_INFO_V1(mic_to_koi8r);
PG_FUNCTION_INFO_V1(iso_to_mic);
PG_FUNCTION_INFO_V1(mic_to_iso);
PG_FUNCTION_INFO_V1(win1251_to_mic);
PG_FUNCTION_INFO_V1(mic_to_win1251);
PG_FUNCTION_INFO_V1(alt_to_mic);
PG_FUNCTION_INFO_V1(mic_to_alt);
PG_FUNCTION_INFO_V1(koi8r_to_win1251);
PG_FUNCTION_INFO_V1(win1251_to_koi8r);
PG_FUNCTION_INFO_V1(koi8r_to_alt);
PG_FUNCTION_INFO_V1(alt_to_koi8r);
PG_FUNCTION_INFO_V1(alt_to_win1251);
PG_FUNCTION_INFO_V1(win1251_to_alt);
PG_FUNCTION_INFO_V1(iso_to_koi8r);
PG_FUNCTION_INFO_V1(koi8r_to_iso);
PG_FUNCTION_INFO_V1(iso_to_win1251);
PG_FUNCTION_INFO_V1(win1251_to_iso);
PG_FUNCTION_INFO_V1(iso_to_alt);
PG_FUNCTION_INFO_V1(alt_to_iso);

extern Datum koi8r_to_mic(PG_FUNCTION_ARGS);
extern Datum mic_to_koi8r(PG_FUNCTION_ARGS);
extern Datum iso_to_mic(PG_FUNCTION_ARGS);
extern Datum mic_to_iso(PG_FUNCTION_ARGS);
extern Datum win1251_to_mic(PG_FUNCTION_ARGS);
extern Datum mic_to_win1251(PG_FUNCTION_ARGS);
extern Datum alt_to_mic(PG_FUNCTION_ARGS);
extern Datum mic_to_alt(PG_FUNCTION_ARGS);
extern Datum koi8r_to_win1251(PG_FUNCTION_ARGS);
extern Datum win1251_to_koi8r(PG_FUNCTION_ARGS);
extern Datum koi8r_to_alt(PG_FUNCTION_ARGS);
extern Datum alt_to_koi8r(PG_FUNCTION_ARGS);
extern Datum alt_to_win1251(PG_FUNCTION_ARGS);
extern Datum win1251_to_alt(PG_FUNCTION_ARGS);
extern Datum iso_to_koi8r(PG_FUNCTION_ARGS);
extern Datum koi8r_to_iso(PG_FUNCTION_ARGS);
extern Datum iso_to_win1251(PG_FUNCTION_ARGS);
extern Datum win1251_to_iso(PG_FUNCTION_ARGS);
extern Datum iso_to_alt(PG_FUNCTION_ARGS);
extern Datum alt_to_iso(PG_FUNCTION_ARGS);

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

static void koi8r2mic(unsigned char *l, unsigned char *p, int len);
static void mic2koi8r(unsigned char *mic, unsigned char *p, int len);
static void iso2mic(unsigned char *l, unsigned char *p, int len);
static void mic2iso(unsigned char *mic, unsigned char *p, int len);
static void win12512mic(unsigned char *l, unsigned char *p, int len);
static void mic2win1251(unsigned char *mic, unsigned char *p, int len);
static void alt2mic(unsigned char *l, unsigned char *p, int len);
static void mic2alt(unsigned char *mic, unsigned char *p, int len);

Datum
koi8r_to_mic(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);

	Assert(PG_GETARG_INT32(0) == PG_KOI8R);
	Assert(PG_GETARG_INT32(1) == PG_MULE_INTERNAL);
	Assert(len >= 0);

	koi8r2mic(src, dest, len);

	PG_RETURN_VOID();
}

Datum
mic_to_koi8r(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);

	Assert(PG_GETARG_INT32(0) == PG_MULE_INTERNAL);
	Assert(PG_GETARG_INT32(1) == PG_KOI8R);
	Assert(len >= 0);

	mic2koi8r(src, dest, len);

	PG_RETURN_VOID();
}

Datum
iso_to_mic(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);

	Assert(PG_GETARG_INT32(0) == PG_ISO_8859_5);
	Assert(PG_GETARG_INT32(1) == PG_MULE_INTERNAL);
	Assert(len >= 0);

	iso2mic(src, dest, len);

	PG_RETURN_VOID();
}

Datum
mic_to_iso(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);

	Assert(PG_GETARG_INT32(0) == PG_MULE_INTERNAL);
	Assert(PG_GETARG_INT32(1) == PG_ISO_8859_5);
	Assert(len >= 0);

	mic2iso(src, dest, len);

	PG_RETURN_VOID();
}

Datum
win1251_to_mic(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);

	Assert(PG_GETARG_INT32(0) == PG_WIN1251);
	Assert(PG_GETARG_INT32(1) == PG_MULE_INTERNAL);
	Assert(len >= 0);

	win12512mic(src, dest, len);

	PG_RETURN_VOID();
}

Datum
mic_to_win1251(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);

	Assert(PG_GETARG_INT32(0) == PG_MULE_INTERNAL);
	Assert(PG_GETARG_INT32(1) == PG_WIN1251);
	Assert(len >= 0);

	mic2win1251(src, dest, len);

	PG_RETURN_VOID();
}

Datum
alt_to_mic(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);

	Assert(PG_GETARG_INT32(0) == PG_ALT);
	Assert(PG_GETARG_INT32(1) == PG_MULE_INTERNAL);
	Assert(len >= 0);

	alt2mic(src, dest, len);

	PG_RETURN_VOID();
}

Datum
mic_to_alt(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);

	Assert(PG_GETARG_INT32(0) == PG_MULE_INTERNAL);
	Assert(PG_GETARG_INT32(1) == PG_ALT);
	Assert(len >= 0);

	mic2alt(src, dest, len);

	PG_RETURN_VOID();
}

Datum
koi8r_to_win1251(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);
	unsigned char *buf;

	Assert(PG_GETARG_INT32(0) == PG_KOI8R);
	Assert(PG_GETARG_INT32(1) == PG_WIN1251);
	Assert(len >= 0);

	buf = palloc(len * ENCODING_GROWTH_RATE);
	koi8r2mic(src, buf, len);
	mic2win1251(buf, dest, strlen(buf));
	pfree(buf);

	PG_RETURN_VOID();
}

Datum
win1251_to_koi8r(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);
	unsigned char *buf;

	Assert(PG_GETARG_INT32(0) == PG_WIN1251);
	Assert(PG_GETARG_INT32(1) == PG_KOI8R);
	Assert(len >= 0);

	buf = palloc(len * ENCODING_GROWTH_RATE);
	win12512mic(src, buf, len);
	mic2koi8r(buf, dest, strlen(buf));
	pfree(buf);

	PG_RETURN_VOID();
}

Datum
koi8r_to_alt(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);
	unsigned char *buf;

	Assert(PG_GETARG_INT32(0) == PG_KOI8R);
	Assert(PG_GETARG_INT32(1) == PG_ALT);
	Assert(len >= 0);

	buf = palloc(len * ENCODING_GROWTH_RATE);
	koi8r2mic(src, buf, len);
	mic2alt(buf, dest, strlen(buf));
	pfree(buf);

	PG_RETURN_VOID();
}

Datum
alt_to_koi8r(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);
	unsigned char *buf;

	Assert(PG_GETARG_INT32(0) == PG_ALT);
	Assert(PG_GETARG_INT32(1) == PG_KOI8R);
	Assert(len >= 0);

	buf = palloc(len * ENCODING_GROWTH_RATE);
	alt2mic(src, buf, len);
	mic2koi8r(buf, dest, strlen(buf));
	pfree(buf);

	PG_RETURN_VOID();
}

Datum
alt_to_win1251(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);
	unsigned char *buf;

	Assert(PG_GETARG_INT32(0) == PG_ALT);
	Assert(PG_GETARG_INT32(1) == PG_WIN1251);
	Assert(len >= 0);

	buf = palloc(len * ENCODING_GROWTH_RATE);
	alt2mic(src, buf, len);
	mic2win1251(buf, dest, strlen(buf));
	pfree(buf);

	PG_RETURN_VOID();
}

Datum
win1251_to_alt(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);
	unsigned char *buf;

	Assert(PG_GETARG_INT32(0) == PG_WIN1251);
	Assert(PG_GETARG_INT32(1) == PG_ALT);
	Assert(len >= 0);

	buf = palloc(len * ENCODING_GROWTH_RATE);
	win12512mic(src, buf, len);
	mic2alt(buf, dest, strlen(buf));
	pfree(buf);

	PG_RETURN_VOID();
}

Datum
iso_to_koi8r(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);
	unsigned char *buf;

	Assert(PG_GETARG_INT32(0) == PG_ISO_8859_5);
	Assert(PG_GETARG_INT32(1) == PG_KOI8R);
	Assert(len >= 0);

	buf = palloc(len * ENCODING_GROWTH_RATE);
	iso2mic(src, buf, len);
	mic2koi8r(buf, dest, strlen(buf));
	pfree(buf);

	PG_RETURN_VOID();
}

Datum
koi8r_to_iso(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);
	unsigned char *buf;

	Assert(PG_GETARG_INT32(0) == PG_KOI8R);
	Assert(PG_GETARG_INT32(1) == PG_ISO_8859_5);
	Assert(len >= 0);

	buf = palloc(len * ENCODING_GROWTH_RATE);
	koi8r2mic(src, buf, len);
	mic2iso(buf, dest, strlen(buf));
	pfree(buf);

	PG_RETURN_VOID();
}

Datum
iso_to_win1251(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);
	unsigned char *buf;

	Assert(PG_GETARG_INT32(0) == PG_ISO_8859_5);
	Assert(PG_GETARG_INT32(1) == PG_WIN1251);
	Assert(len >= 0);

	buf = palloc(len * ENCODING_GROWTH_RATE);
	iso2mic(src, buf, len);
	mic2win1251(buf, dest, strlen(buf));
	pfree(buf);

	PG_RETURN_VOID();
}

Datum
win1251_to_iso(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);
	unsigned char *buf;

	Assert(PG_GETARG_INT32(0) == PG_WIN1251);
	Assert(PG_GETARG_INT32(1) == PG_ISO_8859_5);
	Assert(len >= 0);

	buf = palloc(len * ENCODING_GROWTH_RATE);
	win12512mic(src, buf, len);
	mic2win1251(buf, dest, strlen(buf));
	pfree(buf);

	PG_RETURN_VOID();
}

Datum
iso_to_alt(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);
	unsigned char *buf;

	Assert(PG_GETARG_INT32(0) == PG_ISO_8859_5);
	Assert(PG_GETARG_INT32(1) == PG_ALT);
	Assert(len >= 0);

	buf = palloc(len * ENCODING_GROWTH_RATE);
	iso2mic(src, buf, len);
	mic2alt(buf, dest, strlen(buf));
	pfree(buf);

	PG_RETURN_VOID();
}

Datum
alt_to_iso(PG_FUNCTION_ARGS)
{
	unsigned char *src = PG_GETARG_CSTRING(2);
	unsigned char *dest = PG_GETARG_CSTRING(3);
	int			len = PG_GETARG_INT32(4);
	unsigned char *buf;

	Assert(PG_GETARG_INT32(0) == PG_ALT);
	Assert(PG_GETARG_INT32(1) == PG_ISO_8859_5);
	Assert(len >= 0);

	buf = palloc(len * ENCODING_GROWTH_RATE);
	alt2mic(src, buf, len);
	mic2alt(buf, dest, strlen(buf));
	pfree(buf);

	PG_RETURN_VOID();
}

/*
 * Cyrillic support
 * currently supported Cyrillic encodings:
 *
 * KOI8-R (this is the charset for the mule internal code
 *		for Cyrillic)
 * ISO-8859-5
 * Microsoft's CP1251(windows-1251)
 * Alternativny Variant (MS-DOS CP866)
 */

/* koi8r2mic: KOI8-R to Mule internal code */
static void
koi8r2mic(unsigned char *l, unsigned char *p, int len)
{
	latin2mic(l, p, len, LC_KOI8_R);
}

/* mic2koi8r: Mule internal code to KOI8-R */
static void
mic2koi8r(unsigned char *mic, unsigned char *p, int len)
{
	mic2latin(mic, p, len, LC_KOI8_R);
}

/* iso2mic: ISO-8859-5 to Mule internal code */
static void
iso2mic(unsigned char *l, unsigned char *p, int len)
{
	static unsigned char iso2koi[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xe1, 0xe2, 0xf7, 0xe7, 0xe4, 0xe5, 0xf6, 0xfa,
		0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0,
		0xf2, 0xf3, 0xf4, 0xf5, 0xe6, 0xe8, 0xe3, 0xfe,
		0xfb, 0xfd, 0xff, 0xf9, 0xf8, 0xfc, 0xe0, 0xf1,
		0xc1, 0xc2, 0xd7, 0xc7, 0xc4, 0xc5, 0xd6, 0xda,
		0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0,
		0xd2, 0xd3, 0xd4, 0xd5, 0xc6, 0xc8, 0xc3, 0xde,
		0xdb, 0xdd, 0xdf, 0xd9, 0xd8, 0xdc, 0xc0, 0xd1,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	latin2mic_with_table(l, p, len, LC_KOI8_R, iso2koi);
}

/* mic2iso: Mule internal code to ISO8859-5 */
static void
mic2iso(unsigned char *mic, unsigned char *p, int len)
{
	static unsigned char koi2iso[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xee, 0xd0, 0xd1, 0xe6, 0xd4, 0xd5, 0xe4, 0xd3,
		0xe5, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde,
		0xdf, 0xef, 0xe0, 0xe1, 0xe2, 0xe3, 0xd6, 0xd2,
		0xec, 0xeb, 0xd7, 0xe8, 0xed, 0xe9, 0xe7, 0xea,
		0xce, 0xb0, 0xb1, 0xc6, 0xb4, 0xb5, 0xc4, 0xb3,
		0xc5, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe,
		0xbf, 0xcf, 0xc0, 0xc1, 0xc2, 0xc3, 0xb6, 0xb2,
		0xcc, 0xcb, 0xb7, 0xc8, 0xcd, 0xc9, 0xc7, 0xca
	};

	mic2latin_with_table(mic, p, len, LC_KOI8_R, koi2iso);
}

/* win2mic: CP1251 to Mule internal code */
static void
win12512mic(unsigned char *l, unsigned char *p, int len)
{
	static unsigned char win2koi[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0xbd, 0x00, 0x00,
		0xb3, 0x00, 0xb4, 0x00, 0x00, 0x00, 0x00, 0xb7,
		0x00, 0x00, 0xb6, 0xa6, 0xad, 0x00, 0x00, 0x00,
		0xa3, 0x00, 0xa4, 0x00, 0x00, 0x00, 0x00, 0xa7,
		0xe1, 0xe2, 0xf7, 0xe7, 0xe4, 0xe5, 0xf6, 0xfa,
		0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0,
		0xf2, 0xf3, 0xf4, 0xf5, 0xe6, 0xe8, 0xe3, 0xfe,
		0xfb, 0xfd, 0xff, 0xf9, 0xf8, 0xfc, 0xe0, 0xf1,
		0xc1, 0xc2, 0xd7, 0xc7, 0xc4, 0xc5, 0xd6, 0xda,
		0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0,
		0xd2, 0xd3, 0xd4, 0xd5, 0xc6, 0xc8, 0xc3, 0xde,
		0xdb, 0xdd, 0xdf, 0xd9, 0xd8, 0xdc, 0xc0, 0xd1
	};

	latin2mic_with_table(l, p, len, LC_KOI8_R, win2koi);
}

/* mic2win: Mule internal code to CP1251 */
static void
mic2win1251(unsigned char *mic, unsigned char *p, int len)
{
	static unsigned char koi2win[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xb8, 0xba, 0x00, 0xb3, 0xbf,
		0x00, 0x00, 0x00, 0x00, 0x00, 0xb4, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xa8, 0xaa, 0x00, 0xb2, 0xaf,
		0x00, 0x00, 0x00, 0x00, 0x00, 0xa5, 0x00, 0x00,
		0xfe, 0xe0, 0xe1, 0xf6, 0xe4, 0xe5, 0xf4, 0xe3,
		0xf5, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee,
		0xef, 0xff, 0xf0, 0xf1, 0xf2, 0xf3, 0xe6, 0xe2,
		0xfc, 0xfb, 0xe7, 0xf8, 0xfd, 0xf9, 0xf7, 0xfa,
		0xde, 0xc0, 0xc1, 0xd6, 0xc4, 0xc5, 0xd4, 0xc3,
		0xd5, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce,
		0xcf, 0xdf, 0xd0, 0xd1, 0xd2, 0xd3, 0xc6, 0xc2,
		0xdc, 0xdb, 0xc7, 0xd8, 0xdd, 0xd9, 0xd7, 0xda
	};

	mic2latin_with_table(mic, p, len, LC_KOI8_R, koi2win);
}

/* alt2mic: CP866 to Mule internal code */
static void
alt2mic(unsigned char *l, unsigned char *p, int len)
{
	static unsigned char alt2koi[] = {
		0xe1, 0xe2, 0xf7, 0xe7, 0xe4, 0xe5, 0xf6, 0xfa,
		0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0,
		0xf2, 0xf3, 0xf4, 0xf5, 0xe6, 0xe8, 0xe3, 0xfe,
		0xfb, 0xfd, 0xff, 0xf9, 0xf8, 0xfc, 0xe0, 0xf1,
		0xc1, 0xc2, 0xd7, 0xc7, 0xc4, 0xc5, 0xd6, 0xda,
		0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0xbd, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xd2, 0xd3, 0xd4, 0xd5, 0xc6, 0xc8, 0xc3, 0xde,
		0xdb, 0xdd, 0xdf, 0xd9, 0xd8, 0xdc, 0xc0, 0xd1,
		0xb3, 0xa3, 0xb4, 0xa4, 0xb7, 0xa7, 0x00, 0x00,
		0xb6, 0xa6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	latin2mic_with_table(l, p, len, LC_KOI8_R, alt2koi);
}

/* mic2alt: Mule internal code to CP866 */
static void
mic2alt(unsigned char *mic, unsigned char *p, int len)
{
	static unsigned char koi2alt[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xf1, 0xf3, 0x00, 0xf9, 0xf5,
		0x00, 0x00, 0x00, 0x00, 0x00, 0xad, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xf0, 0xf2, 0x00, 0xf8, 0xf4,
		0x00, 0x00, 0x00, 0x00, 0x00, 0xbd, 0x00, 0x00,
		0xee, 0xa0, 0xa1, 0xe6, 0xa4, 0xa5, 0xe4, 0xa3,
		0xe5, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae,
		0xaf, 0xef, 0xe0, 0xe1, 0xe2, 0xe3, 0xa6, 0xa2,
		0xec, 0xeb, 0xa7, 0xe8, 0xed, 0xe9, 0xe7, 0xea,
		0x9e, 0x80, 0x81, 0x96, 0x84, 0x85, 0x94, 0x83,
		0x95, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e,
		0x8f, 0x9f, 0x90, 0x91, 0x92, 0x93, 0x86, 0x82,
		0x9c, 0x9b, 0x87, 0x98, 0x9d, 0x99, 0x97, 0x9a
	};

	mic2latin_with_table(mic, p, len, LC_KOI8_R, koi2alt);
}
