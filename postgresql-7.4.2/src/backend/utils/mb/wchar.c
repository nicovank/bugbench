/*
 * conversion functions between pg_wchar and multibyte streams.
 * Tatsuo Ishii
 * $Id: wchar.c,v 1.34 2003/09/25 06:58:05 petere Exp $
 *
 * WIN1250 client encoding updated by Pavel Behal
 *
 */
/* can be used in either frontend or backend */
#ifdef FRONTEND
#include "postgres_fe.h"
#define Assert(condition)
#else
#include "postgres.h"
#endif

#include "mb/pg_wchar.h"


/*
 * conversion to pg_wchar is done by "table driven."
 * to add an encoding support, define mb2wchar_with_len(), mblen()
 * for the particular encoding. Note that if the encoding is only
 * supported in the client, you don't need to define
 * mb2wchar_with_len() function (SJIS is the case).
 */

/*
 * SQL/ASCII
 */
static int	pg_ascii2wchar_with_len
			(const unsigned char *from, pg_wchar *to, int len)
{
	int			cnt = 0;

	while (len > 0 && *from)
	{
		*to++ = *from++;
		len--;
		cnt++;
	}
	*to = 0;
	return (cnt);
}

static int
pg_ascii_mblen(const unsigned char *s)
{
	return (1);
}

/*
 * EUC
 */

static int	pg_euc2wchar_with_len
			(const unsigned char *from, pg_wchar *to, int len)
{
	int			cnt = 0;

	while (len > 0 && *from)
	{
		if (*from == SS2 && len >= 2)
		{
			from++;
			*to = 0xff & *from++;
			len -= 2;
		}
		else if (*from == SS3 && len >= 3)
		{
			from++;
			*to = *from++ << 8;
			*to |= 0x3f & *from++;
			len -= 3;
		}
		else if ((*from & 0x80) && len >= 2)
		{
			*to = *from++ << 8;
			*to |= *from++;
			len -= 2;
		}
		else
		{
			*to = *from++;
			len--;
		}
		to++;
		cnt++;
	}
	*to = 0;
	return (cnt);
}

static int
pg_euc_mblen(const unsigned char *s)
{
	int			len;

	if (*s == SS2)
		len = 2;
	else if (*s == SS3)
		len = 3;
	else if (*s & 0x80)
		len = 2;
	else
		len = 1;
	return (len);
}

/*
 * EUC_JP
 */
static int	pg_eucjp2wchar_with_len
			(const unsigned char *from, pg_wchar *to, int len)
{
	return (pg_euc2wchar_with_len(from, to, len));
}

static int
pg_eucjp_mblen(const unsigned char *s)
{
	return (pg_euc_mblen(s));
}

/*
 * EUC_KR
 */
static int	pg_euckr2wchar_with_len
			(const unsigned char *from, pg_wchar *to, int len)
{
	return (pg_euc2wchar_with_len(from, to, len));
}

static int
pg_euckr_mblen(const unsigned char *s)
{
	return (pg_euc_mblen(s));
}

/*
 * EUC_CN
 */
static int	pg_euccn2wchar_with_len
			(const unsigned char *from, pg_wchar *to, int len)
{
	int			cnt = 0;

	while (len > 0 && *from)
	{
		if (*from == SS2 && len >= 3)
		{
			from++;
			*to = 0x3f00 & (*from++ << 8);
			*to = *from++;
			len -= 3;
		}
		else if (*from == SS3 && len >= 3)
		{
			from++;
			*to = *from++ << 8;
			*to |= 0x3f & *from++;
			len -= 3;
		}
		else if ((*from & 0x80) && len >= 2)
		{
			*to = *from++ << 8;
			*to |= *from++;
			len -= 2;
		}
		else
		{
			*to = *from++;
			len--;
		}
		to++;
		cnt++;
	}
	*to = 0;
	return (cnt);
}

static int
pg_euccn_mblen(const unsigned char *s)
{
	int			len;

	if (*s & 0x80)
		len = 2;
	else
		len = 1;
	return (len);
}

/*
 * EUC_TW
 */
static int	pg_euctw2wchar_with_len
			(const unsigned char *from, pg_wchar *to, int len)
{
	int			cnt = 0;

	while (len > 0 && *from)
	{
		if (*from == SS2 && len >= 4)
		{
			from++;
			*to = *from++ << 16;
			*to |= *from++ << 8;
			*to |= *from++;
			len -= 4;
		}
		else if (*from == SS3 && len >= 3)
		{
			from++;
			*to = *from++ << 8;
			*to |= 0x3f & *from++;
			len -= 3;
		}
		else if ((*from & 0x80) && len >= 2)
		{
			*to = *from++ << 8;
			*to |= *from++;
			len -= 2;
		}
		else
		{
			*to = *from++;
			len--;
		}
		to++;
		cnt++;
	}
	*to = 0;
	return (cnt);
}

static int
pg_euctw_mblen(const unsigned char *s)
{
	int			len;

	if (*s == SS2)
		len = 4;
	else if (*s == SS3)
		len = 3;
	else if (*s & 0x80)
		len = 2;
	else
		len = 1;
	return (len);
}

/*
 * JOHAB
 */
static int
pg_johab2wchar_with_len(const unsigned char *from, pg_wchar *to, int len)
{
	return (pg_euc2wchar_with_len(from, to, len));
}

static int
pg_johab_mblen(const unsigned char *s)
{
	return (pg_euc_mblen(s));
}

/*
 * convert UTF-8 string to pg_wchar (UCS-2)
 * caller should allocate enough space for "to"
 * len: length of from.
 * "from" not necessarily null terminated.
 */
static int
pg_utf2wchar_with_len(const unsigned char *from, pg_wchar *to, int len)
{
	unsigned char c1,
				c2,
				c3;
	int			cnt = 0;

	while (len > 0 && *from)
	{
		if ((*from & 0x80) == 0)
		{
			*to = *from++;
			len--;
		}
		else if ((*from & 0xe0) == 0xc0 && len >= 2)
		{
			c1 = *from++ & 0x1f;
			c2 = *from++ & 0x3f;
			*to = c1 << 6;
			*to |= c2;
			len -= 2;
		}
		else if ((*from & 0xe0) == 0xe0 && len >= 3)
		{
			c1 = *from++ & 0x0f;
			c2 = *from++ & 0x3f;
			c3 = *from++ & 0x3f;
			*to = c1 << 12;
			*to |= c2 << 6;
			*to |= c3;
			len -= 3;
		}
		else
		{
			*to = *from++;
			len--;
		}
		to++;
		cnt++;
	}
	*to = 0;
	return (cnt);
}

/*
 * returns the byte length of a UTF-8 word pointed to by s
 */
int
pg_utf_mblen(const unsigned char *s)
{
	int			len = 1;

	if ((*s & 0x80) == 0)
		len = 1;
	else if ((*s & 0xe0) == 0xc0)
		len = 2;
	else if ((*s & 0xe0) == 0xe0)
		len = 3;
	return (len);
}

/*
 * convert mule internal code to pg_wchar
 * caller should allocate enough space for "to"
 * len: length of from.
 * "from" not necessarily null terminated.
 */
static int
pg_mule2wchar_with_len(const unsigned char *from, pg_wchar *to, int len)
{
	int			cnt = 0;

	while (len > 0 && *from)
	{
		if (IS_LC1(*from) && len >= 2)
		{
			*to = *from++ << 16;
			*to |= *from++;
			len -= 2;
		}
		else if (IS_LCPRV1(*from) && len >= 3)
		{
			from++;
			*to = *from++ << 16;
			*to |= *from++;
			len -= 3;
		}
		else if (IS_LC2(*from) && len >= 3)
		{
			*to = *from++ << 16;
			*to |= *from++ << 8;
			*to |= *from++;
			len -= 3;
		}
		else if (IS_LCPRV2(*from) && len >= 4)
		{
			from++;
			*to = *from++ << 16;
			*to |= *from++ << 8;
			*to |= *from++;
			len -= 4;
		}
		else
		{						/* assume ASCII */
			*to = (unsigned char) *from++;
			len--;
		}
		to++;
		cnt++;
	}
	*to = 0;
	return (cnt);
}

int
pg_mule_mblen(const unsigned char *s)
{
	int			len;

	if (IS_LC1(*s))
		len = 2;
	else if (IS_LCPRV1(*s))
		len = 3;
	else if (IS_LC2(*s))
		len = 3;
	else if (IS_LCPRV2(*s))
		len = 4;
	else
	{							/* assume ASCII */
		len = 1;
	}
	return (len);
}

/*
 * ISO8859-1
 */
static int
pg_latin12wchar_with_len(const unsigned char *from, pg_wchar *to, int len)
{
	int			cnt = 0;

	while (len > 0 && *from)
	{
		*to++ = *from++;
		len--;
		cnt++;
	}
	*to = 0;
	return (cnt);
}

static int
pg_latin1_mblen(const unsigned char *s)
{
	return (1);
}

/*
 * SJIS
 */
static int
pg_sjis_mblen(const unsigned char *s)
{
	int			len;

	if (*s >= 0xa1 && *s <= 0xdf)
	{							/* 1 byte kana? */
		len = 1;
	}
	else if (*s > 0x7f)
	{							/* kanji? */
		len = 2;
	}
	else
	{							/* should be ASCII */
		len = 1;
	}
	return (len);
}

/*
 * Big5
 */
static int
pg_big5_mblen(const unsigned char *s)
{
	int			len;

	if (*s > 0x7f)
	{							/* kanji? */
		len = 2;
	}
	else
	{							/* should be ASCII */
		len = 1;
	}
	return (len);
}

/*
 * GBK
 */
static int
pg_gbk_mblen(const unsigned char *s)
{
	int			len;

	if (*s > 0x7f)
	{							/* kanji? */
		len = 2;
	}
	else
	{							/* should be ASCII */
		len = 1;
	}
	return (len);
}

/*
 * UHC
 */
static int
pg_uhc_mblen(const unsigned char *s)
{
	int			len;

	if (*s > 0x7f)
	{							/* 2byte? */
		len = 2;
	}
	else
	{							/* should be ASCII */
		len = 1;
	}
	return (len);
}

/*
 *	* GB18030
 *	 * Added by Bill Huang <bhuang@redhat.com>,<bill_huanghb@ybb.ne.jp>
 *	  */
static int
pg_gb18030_mblen(const unsigned char *s)
{
	int			len;

	if (*s <= 0x7f)
	{							/* ASCII */
		len = 1;
	}
	else
	{
		if ((*(s + 1) >= 0x40 && *(s + 1) <= 0x7e) || (*(s + 1) >= 0x80 && *(s + 1) <= 0xfe))
			len = 2;
		else if (*(s + 1) >= 0x30 && *(s + 1) <= 0x39)
			len = 4;
		else
			len = 2;
	}
	return (len);
}


pg_wchar_tbl pg_wchar_table[] = {
	{pg_ascii2wchar_with_len, pg_ascii_mblen, 1},		/* 0; PG_SQL_ASCII	*/
	{pg_eucjp2wchar_with_len, pg_eucjp_mblen, 3},		/* 1; PG_EUC_JP */
	{pg_euccn2wchar_with_len, pg_euccn_mblen, 3},		/* 2; PG_EUC_CN */
	{pg_euckr2wchar_with_len, pg_euckr_mblen, 3},		/* 3; PG_EUC_KR */
	{pg_euctw2wchar_with_len, pg_euctw_mblen, 3},		/* 4; PG_EUC_TW */
	{pg_johab2wchar_with_len, pg_johab_mblen, 3},		/* 5; PG_JOHAB */
	{pg_utf2wchar_with_len, pg_utf_mblen, 3},	/* 6; PG_UNICODE */
	{pg_mule2wchar_with_len, pg_mule_mblen, 3}, /* 7; PG_MULE_INTERNAL */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 8; PG_LATIN1 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 9; PG_LATIN2 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 10; PG_LATIN3 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 11; PG_LATIN4 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 12; PG_LATIN5 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 13; PG_LATIN6 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 14; PG_LATIN7 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 15; PG_LATIN8 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 16; PG_LATIN9 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 17; PG_LATIN10 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 18; PG_WIN1256 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 19; PG_TCVN */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 20; PG_WIN874 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 21; PG_KOI8 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 22; PG_WIN1251 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 23; PG_ALT */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 24; ISO-8859-5 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 25; ISO-8859-6 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 26; ISO-8859-7 */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 27; ISO-8859-8 */
	{0, pg_sjis_mblen, 2},		/* 28; PG_SJIS */
	{0, pg_big5_mblen, 2},		/* 29; PG_BIG5 */
	{0, pg_gbk_mblen, 2},		/* 30; PG_GBK */
	{0, pg_uhc_mblen, 2},		/* 31; PG_UHC */
	{pg_latin12wchar_with_len, pg_latin1_mblen, 1},		/* 32; PG_WIN1250 */
	{0, pg_gb18030_mblen, 2}	/* 33; PG_GB18030 */
};

/* returns the byte length of a word for mule internal code */
int
pg_mic_mblen(const unsigned char *mbstr)
{
	return (pg_mule_mblen(mbstr));
}

/*
 * Returns the byte length of a multibyte word.
 */
int
pg_encoding_mblen(int encoding, const unsigned char *mbstr)
{
	Assert(PG_VALID_ENCODING(encoding));

	return ((encoding >= 0 &&
			 encoding < sizeof(pg_wchar_table) / sizeof(pg_wchar_tbl)) ?
			((*pg_wchar_table[encoding].mblen) (mbstr)) :
			((*pg_wchar_table[PG_SQL_ASCII].mblen) (mbstr)));
}

/*
 * fetch maximum length of a char encoding
 */
int
pg_encoding_max_length(int encoding)
{
	Assert(PG_VALID_ENCODING(encoding));

	return pg_wchar_table[encoding].maxmblen;
}

#ifndef FRONTEND

/*
 * Verify mbstr to make sure that it has a valid character sequence.
 * mbstr is not necessarily NULL terminated; length of mbstr is
 * specified by len.
 *
 * If OK, return TRUE.	If a problem is found, return FALSE when noError is
 * true; when noError is false, ereport() a descriptive message.
 */
bool
pg_verifymbstr(const unsigned char *mbstr, int len, bool noError)
{
	int			l;
	int			i;
	int			encoding;

	/* we do not need any check in single-byte encodings */
	if (pg_database_encoding_max_length() <= 1)
		return true;

	encoding = GetDatabaseEncoding();

	while (len > 0 && *mbstr)
	{
		/* special UTF-8 check */
		if (encoding == PG_UTF8 && (*mbstr & 0xf8) == 0xf0)
		{
			if (noError)
				return false;
			ereport(ERROR,
					(errcode(ERRCODE_CHARACTER_NOT_IN_REPERTOIRE),
			 errmsg("Unicode characters greater than or equal to 0x10000 are not supported")));
		}

		l = pg_mblen(mbstr);

		for (i = 1; i < l; i++)
		{
			/*
			 * we expect that every multibyte char consists of bytes
			 * having the 8th bit set
			 */
			if (i >= len || (mbstr[i] & 0x80) == 0)
			{
				char		buf[8 * 2 + 1];
				char	   *p = buf;
				int			j,
							jlimit;

				if (noError)
					return false;

				jlimit = Min(l, len);
				jlimit = Min(jlimit, 8);		/* prevent buffer overrun */

				for (j = 0; j < jlimit; j++)
					p += sprintf(p, "%02x", mbstr[j]);

				ereport(ERROR,
						(errcode(ERRCODE_CHARACTER_NOT_IN_REPERTOIRE),
						 errmsg("invalid byte sequence for encoding \"%s\": 0x%s",
								GetDatabaseEncodingName(), buf)));
			}
		}

		len -= l;
		mbstr += l;
	}

	return true;
}

/*
 * fetch maximum length of a char encoding for the current database
 */
int
pg_database_encoding_max_length(void)
{
	return pg_wchar_table[GetDatabaseEncoding()].maxmblen;
}

#endif



