/*-------------------------------------------------------------------------
 *
 * datetime.h
 *	  Definitions for the date/time and other date/time support code.
 *	  The support code is shared with other date data types,
 *	   including abstime, reltime, date, and time.
 *
 *
 * Portions Copyright (c) 1996-2003, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $Id: datetime.h,v 1.45 2003/08/27 23:29:29 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef DATETIME_H
#define DATETIME_H

#include <limits.h>
#include <math.h>
#include <time.h>

#include "utils/timestamp.h"


/* ----------------------------------------------------------------
 *				time types + support macros
 *
 * String definitions for standard time quantities.
 *
 * These strings are the defaults used to form output time strings.
 * Other alternate forms are hardcoded into token tables in datetime.c.
 * ----------------------------------------------------------------
 */

#define DAGO			"ago"
#define DCURRENT		"current"
#define EPOCH			"epoch"
#define INVALID			"invalid"
#define EARLY			"-infinity"
#define LATE			"infinity"
#define NOW				"now"
#define TODAY			"today"
#define TOMORROW		"tomorrow"
#define YESTERDAY		"yesterday"
#define ZULU			"zulu"

#define DMICROSEC		"usecond"
#define DMILLISEC		"msecond"
#define DSECOND			"second"
#define DMINUTE			"minute"
#define DHOUR			"hour"
#define DDAY			"day"
#define DWEEK			"week"
#define DMONTH			"month"
#define DQUARTER		"quarter"
#define DYEAR			"year"
#define DDECADE			"decade"
#define DCENTURY		"century"
#define DMILLENNIUM		"millennium"
#define DA_D			"ad"
#define DB_C			"bc"
#define DTIMEZONE		"timezone"

/*
 * Fundamental time field definitions for parsing.
 *
 *	Meridian:  am, pm, or 24-hour style.
 *	Millennium: ad, bc
 */

#define AM		0
#define PM		1
#define HR24	2

#define AD		0
#define BC		1

/*
 * Fields for time decoding.
 *
 * Can't have more of these than there are bits in an unsigned int
 * since these are turned into bit masks during parsing and decoding.
 *
 * Furthermore, the values for YEAR, MONTH, DAY, HOUR, MINUTE, SECOND
 * must be in the range 0..14 so that the associated bitmasks can fit
 * into the left half of an INTERVAL's typmod value.
 */

#define RESERV	0
#define MONTH	1
#define YEAR	2
#define DAY		3
#define JULIAN	4
#define TZ		5
#define DTZ		6
#define DTZMOD	7
#define IGNORE_DTF	8
#define AMPM	9
#define HOUR	10
#define MINUTE	11
#define SECOND	12
#define DOY		13
#define DOW		14
#define UNITS	15
#define ADBC	16
/* these are only for relative dates */
#define AGO		17
#define ABS_BEFORE		18
#define ABS_AFTER		19
/* generic fields to help with parsing */
#define ISODATE 20
#define ISOTIME 21
/* reserved for unrecognized string values */
#define UNKNOWN_FIELD	31

/*
 * Token field definitions for time parsing and decoding.
 * These need to fit into the datetkn table type.
 * At the moment, that means keep them within [-127,127].
 * These are also used for bit masks in DecodeDateDelta()
 *	so actually restrict them to within [0,31] for now.
 * - thomas 97/06/19
 * Not all of these fields are used for masks in DecodeDateDelta
 *	so allow some larger than 31. - thomas 1997-11-17
 */

#define DTK_NUMBER		0
#define DTK_STRING		1

#define DTK_DATE		2
#define DTK_TIME		3
#define DTK_TZ			4
#define DTK_AGO			5

#define DTK_SPECIAL		6
#define DTK_INVALID		7
#define DTK_CURRENT		8
#define DTK_EARLY		9
#define DTK_LATE		10
#define DTK_EPOCH		11
#define DTK_NOW			12
#define DTK_YESTERDAY	13
#define DTK_TODAY		14
#define DTK_TOMORROW	15
#define DTK_ZULU		16

#define DTK_DELTA		17
#define DTK_SECOND		18
#define DTK_MINUTE		19
#define DTK_HOUR		20
#define DTK_DAY			21
#define DTK_WEEK		22
#define DTK_MONTH		23
#define DTK_QUARTER		24
#define DTK_YEAR		25
#define DTK_DECADE		26
#define DTK_CENTURY		27
#define DTK_MILLENNIUM	28
#define DTK_MILLISEC	29
#define DTK_MICROSEC	30
#define DTK_JULIAN		31

#define DTK_DOW			32
#define DTK_DOY			33
#define DTK_TZ_HOUR		34
#define DTK_TZ_MINUTE	35


/*
 * Bit mask definitions for time parsing.
 */

#define DTK_M(t)		(0x01 << (t))

#define DTK_DATE_M		(DTK_M(YEAR) | DTK_M(MONTH) | DTK_M(DAY))
#define DTK_TIME_M		(DTK_M(HOUR) | DTK_M(MINUTE) | DTK_M(SECOND))

#define MAXDATELEN		51		/* maximum possible length of an input
								 * date string (not counting tr. null) */
#define MAXDATEFIELDS	25		/* maximum possible number of fields in a
								 * date string */
#define TOKMAXLEN		10		/* only this many chars are stored in
								 * datetktbl */

/* keep this struct small; it gets used a lot */
typedef struct
{
#if defined(_AIX)
	char	   *token;
#else
	char		token[TOKMAXLEN];
#endif   /* _AIX */
	char		type;
	char		value;			/* this may be unsigned, alas */
} datetkn;


/* FMODULO()
 * Macro to replace modf(), which is broken on some platforms.
 * t = input and remainder
 * q = integer part
 * u = divisor
 */
#define FMODULO(t,q,u) \
do { \
	q = ((t < 0) ? ceil(t / u) : floor(t / u)); \
	if (q != 0) t -= rint(q * u); \
} while(0)

/* TMODULO()
 * Like FMODULO(), but work on the timestamp datatype (either int64 or float8).
 * We assume that int64 follows the C99 semantics for division (negative
 * quotients truncate towards zero).
 */
#ifdef HAVE_INT64_TIMESTAMP
#define TMODULO(t,q,u) \
do { \
	q = (t / u); \
	if (q != 0) t -= (q * u); \
} while(0)
#else
#define TMODULO(t,q,u) \
do { \
	q = ((t < 0) ? ceil(t / u) : floor(t / u)); \
	if (q != 0) t -= rint(q * u); \
} while(0)
#endif

/*
 * Date/time validation
 * Include check for leap year.
 */

extern int	day_tab[2][13];

#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

/* Julian date support for date2j() and j2date()
 *
 * IS_VALID_JULIAN checks the minimum date exactly, but is a bit sloppy
 * about the maximum, since it's far enough out to not be especially
 * interesting.
 */

#define JULIAN_MINYEAR (-4713)
#define JULIAN_MINMONTH (11)
#define JULIAN_MINDAY (24)
#define JULIAN_MAXYEAR (5874898)

#define IS_VALID_JULIAN(y,m,d) ((((y) > JULIAN_MINYEAR) \
  || (((y) == JULIAN_MINYEAR) && (((m) > JULIAN_MINMONTH) \
  || (((m) == JULIAN_MINMONTH) && ((d) >= JULIAN_MINDAY))))) \
 && ((y) < JULIAN_MAXYEAR))

/* Julian-date equivalents of Day 0 in Unix and Postgres reckoning */
#define UNIX_EPOCH_JDATE		2440588 /* == date2j(1970, 1, 1) */
#define POSTGRES_EPOCH_JDATE	2451545 /* == date2j(2000, 1, 1) */

/*
 * Info about limits of the Unix time_t data type.  We assume that time_t
 * is a signed int32 with origin 1970-01-01.  Note this is only relevant
 * when we use the C library's time routines for timezone processing.
 */
#define UTIME_MINYEAR (1901)
#define UTIME_MINMONTH (12)
#define UTIME_MINDAY (14)
#define UTIME_MAXYEAR (2038)
#define UTIME_MAXMONTH (01)
#define UTIME_MAXDAY (18)

#define IS_VALID_UTIME(y,m,d) ((((y) > UTIME_MINYEAR) \
 || (((y) == UTIME_MINYEAR) && (((m) > UTIME_MINMONTH) \
  || (((m) == UTIME_MINMONTH) && ((d) >= UTIME_MINDAY))))) \
 && (((y) < UTIME_MAXYEAR) \
 || (((y) == UTIME_MAXYEAR) && (((m) < UTIME_MAXMONTH) \
  || (((m) == UTIME_MAXMONTH) && ((d) <= UTIME_MAXDAY))))))

/*
 * Datetime input parsing routines (ParseDateTime, DecodeDateTime, etc)
 * return zero or a positive value on success.  On failure, they return
 * one of these negative code values.  DateTimeParseError may be used to
 * produce a correct ereport.
 */
#define DTERR_BAD_FORMAT		(-1)
#define DTERR_FIELD_OVERFLOW	(-2)
#define DTERR_MD_FIELD_OVERFLOW	(-3)	/* triggers hint about DateStyle */
#define DTERR_INTERVAL_OVERFLOW	(-4)
#define DTERR_TZDISP_OVERFLOW	(-5)


extern void GetCurrentDateTime(struct tm * tm);
extern void GetCurrentTimeUsec(struct tm * tm, fsec_t *fsec, int *tzp);
extern void j2date(int jd, int *year, int *month, int *day);
extern int	date2j(int year, int month, int day);

extern int ParseDateTime(const char *timestr, char *lowstr,
			  char **field, int *ftype,
			  int maxfields, int *numfields);
extern int DecodeDateTime(char **field, int *ftype,
			   int nf, int *dtype,
			   struct tm * tm, fsec_t *fsec, int *tzp);
extern int DecodeTimeOnly(char **field, int *ftype,
			   int nf, int *dtype,
			   struct tm * tm, fsec_t *fsec, int *tzp);
extern int DecodeInterval(char **field, int *ftype,
			   int nf, int *dtype,
			   struct tm * tm, fsec_t *fsec);
extern void DateTimeParseError(int dterr, const char *str,
							   const char *datatype);

extern int	DetermineLocalTimeZone(struct tm * tm);

extern int	EncodeDateOnly(struct tm * tm, int style, char *str);
extern int	EncodeTimeOnly(struct tm * tm, fsec_t fsec, int *tzp, int style, char *str);
extern int	EncodeDateTime(struct tm * tm, fsec_t fsec, int *tzp, char **tzn, int style, char *str);
extern int	EncodeInterval(struct tm * tm, fsec_t fsec, int style, char *str);

extern int	DecodeSpecial(int field, char *lowtoken, int *val);
extern int	DecodeUnits(int field, char *lowtoken, int *val);
extern bool ClearDateCache(bool newval, bool doit, bool interactive);

extern int	j2day(int jd);

extern int	DecodePosixTimezone(char *str, int *tzp);

extern bool CheckDateTokenTables(void);

#endif   /* DATETIME_H */
