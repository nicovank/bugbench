/*-------------------------------------------------------------------------
 *
 * name.c
 *	  Functions for the built-in type "name".
 *
 * name replaces char16 and is carefully implemented so that it
 * is a string of physical length NAMEDATALEN.
 * DO NOT use hard-coded constants anywhere
 * always use NAMEDATALEN as the symbolic constant!   - jolly 8/21/95
 *
 *
 * Portions Copyright (c) 1996-2003, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /cvsroot/pgsql/src/backend/utils/adt/name.c,v 1.49 2003/08/04 02:40:05 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "catalog/namespace.h"
#include "catalog/pg_type.h"
#include "libpq/pqformat.h"
#include "mb/pg_wchar.h"
#include "miscadmin.h"
#include "utils/array.h"
#include "utils/builtins.h"
#include "utils/lsyscache.h"


/*****************************************************************************
 *	 USER I/O ROUTINES (none)												 *
 *****************************************************************************/


/*
 *		namein	- converts "..." to internal representation
 *
 *		Note:
 *				[Old] Currently if strlen(s) < NAMEDATALEN, the extra chars are nulls
 *				Now, always NULL terminated
 */
Datum
namein(PG_FUNCTION_ARGS)
{
	char	   *s = PG_GETARG_CSTRING(0);
	NameData   *result;
	int			len;

	/* verify encoding */
	len = strlen(s);
	pg_verifymbstr(s, len, false);

	len = pg_mbcliplen(s, len, NAMEDATALEN - 1);

	result = (NameData *) palloc0(NAMEDATALEN);
	memcpy(NameStr(*result), s, len);

	PG_RETURN_NAME(result);
}

/*
 *		nameout - converts internal representation to "..."
 */
Datum
nameout(PG_FUNCTION_ARGS)
{
	Name		s = PG_GETARG_NAME(0);

	PG_RETURN_CSTRING(pstrdup(NameStr(*s)));
}

/*
 *		namerecv			- converts external binary format to name
 */
Datum
namerecv(PG_FUNCTION_ARGS)
{
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	Name		result;
	char	   *str;
	int			nbytes;

	str = pq_getmsgtext(buf, buf->len - buf->cursor, &nbytes);
	if (nbytes >= NAMEDATALEN)
		ereport(ERROR,
				(errcode(ERRCODE_NAME_TOO_LONG),
				 errmsg("identifier too long"),
				 errdetail("Identifier must be less than %d characters.",
						   NAMEDATALEN)));
	result = (NameData *) palloc0(NAMEDATALEN);
	memcpy(result, str, nbytes);
	pfree(str);
	PG_RETURN_NAME(result);
}

/*
 *		namesend			- converts name to binary format
 */
Datum
namesend(PG_FUNCTION_ARGS)
{
	Name		s = PG_GETARG_NAME(0);
	StringInfoData buf;

	pq_begintypsend(&buf);
	pq_sendtext(&buf, NameStr(*s), strlen(NameStr(*s)));
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}


/*****************************************************************************
 *	 PUBLIC ROUTINES														 *
 *****************************************************************************/

/*
 *		nameeq	- returns 1 iff arguments are equal
 *		namene	- returns 1 iff arguments are not equal
 *
 *		BUGS:
 *				Assumes that "xy\0\0a" should be equal to "xy\0b".
 *				If not, can do the comparison backwards for efficiency.
 *
 *		namelt	- returns 1 iff a < b
 *		namele	- returns 1 iff a <= b
 *		namegt	- returns 1 iff a < b
 *		namege	- returns 1 iff a <= b
 *
 */
Datum
nameeq(PG_FUNCTION_ARGS)
{
	Name		arg1 = PG_GETARG_NAME(0);
	Name		arg2 = PG_GETARG_NAME(1);

	PG_RETURN_BOOL(strncmp(NameStr(*arg1), NameStr(*arg2), NAMEDATALEN) == 0);
}

Datum
namene(PG_FUNCTION_ARGS)
{
	Name		arg1 = PG_GETARG_NAME(0);
	Name		arg2 = PG_GETARG_NAME(1);

	PG_RETURN_BOOL(strncmp(NameStr(*arg1), NameStr(*arg2), NAMEDATALEN) != 0);
}

Datum
namelt(PG_FUNCTION_ARGS)
{
	Name		arg1 = PG_GETARG_NAME(0);
	Name		arg2 = PG_GETARG_NAME(1);

	PG_RETURN_BOOL(strncmp(NameStr(*arg1), NameStr(*arg2), NAMEDATALEN) < 0);
}

Datum
namele(PG_FUNCTION_ARGS)
{
	Name		arg1 = PG_GETARG_NAME(0);
	Name		arg2 = PG_GETARG_NAME(1);

	PG_RETURN_BOOL(strncmp(NameStr(*arg1), NameStr(*arg2), NAMEDATALEN) <= 0);
}

Datum
namegt(PG_FUNCTION_ARGS)
{
	Name		arg1 = PG_GETARG_NAME(0);
	Name		arg2 = PG_GETARG_NAME(1);

	PG_RETURN_BOOL(strncmp(NameStr(*arg1), NameStr(*arg2), NAMEDATALEN) > 0);
}

Datum
namege(PG_FUNCTION_ARGS)
{
	Name		arg1 = PG_GETARG_NAME(0);
	Name		arg2 = PG_GETARG_NAME(1);

	PG_RETURN_BOOL(strncmp(NameStr(*arg1), NameStr(*arg2), NAMEDATALEN) >= 0);
}


/*
 * comparison routines for LIKE indexing support
 */

Datum
name_pattern_eq(PG_FUNCTION_ARGS)
{
	Name		arg1 = PG_GETARG_NAME(0);
	Name		arg2 = PG_GETARG_NAME(1);

	PG_RETURN_BOOL(memcmp(NameStr(*arg1), NameStr(*arg2), NAMEDATALEN) == 0);
}

Datum
name_pattern_ne(PG_FUNCTION_ARGS)
{
	Name		arg1 = PG_GETARG_NAME(0);
	Name		arg2 = PG_GETARG_NAME(1);

	PG_RETURN_BOOL(memcmp(NameStr(*arg1), NameStr(*arg2), NAMEDATALEN) != 0);
}

Datum
name_pattern_lt(PG_FUNCTION_ARGS)
{
	Name		arg1 = PG_GETARG_NAME(0);
	Name		arg2 = PG_GETARG_NAME(1);

	PG_RETURN_BOOL(memcmp(NameStr(*arg1), NameStr(*arg2), NAMEDATALEN) < 0);
}

Datum
name_pattern_le(PG_FUNCTION_ARGS)
{
	Name		arg1 = PG_GETARG_NAME(0);
	Name		arg2 = PG_GETARG_NAME(1);

	PG_RETURN_BOOL(memcmp(NameStr(*arg1), NameStr(*arg2), NAMEDATALEN) <= 0);
}

Datum
name_pattern_gt(PG_FUNCTION_ARGS)
{
	Name		arg1 = PG_GETARG_NAME(0);
	Name		arg2 = PG_GETARG_NAME(1);

	PG_RETURN_BOOL(memcmp(NameStr(*arg1), NameStr(*arg2), NAMEDATALEN) > 0);
}

Datum
name_pattern_ge(PG_FUNCTION_ARGS)
{
	Name		arg1 = PG_GETARG_NAME(0);
	Name		arg2 = PG_GETARG_NAME(1);

	PG_RETURN_BOOL(memcmp(NameStr(*arg1), NameStr(*arg2), NAMEDATALEN) >= 0);
}


/* (see char.c for comparison/operation routines) */

int
namecpy(Name n1, Name n2)
{
	if (!n1 || !n2)
		return -1;
	strncpy(NameStr(*n1), NameStr(*n2), NAMEDATALEN);
	return 0;
}

#ifdef NOT_USED
int
namecat(Name n1, Name n2)
{
	return namestrcat(n1, NameStr(*n2));		/* n2 can't be any longer
												 * than n1 */
}
#endif

#ifdef NOT_USED
int
namecmp(Name n1, Name n2)
{
	return strncmp(NameStr(*n1), NameStr(*n2), NAMEDATALEN);
}
#endif

int
namestrcpy(Name name, const char *str)
{
	if (!name || !str)
		return -1;
	StrNCpy(NameStr(*name), str, NAMEDATALEN);
	return 0;
}

#ifdef NOT_USED
int
namestrcat(Name name, const char *str)
{
	int			i;
	char	   *p,
			   *q;

	if (!name || !str)
		return -1;
	for (i = 0, p = NameStr(*name); i < NAMEDATALEN && *p; ++i, ++p)
		;
	for (q = str; i < NAMEDATALEN; ++i, ++p, ++q)
	{
		*p = *q;
		if (!*q)
			break;
	}
	return 0;
}
#endif

int
namestrcmp(Name name, const char *str)
{
	if (!name && !str)
		return 0;
	if (!name)
		return -1;				/* NULL < anything */
	if (!str)
		return 1;				/* NULL < anything */
	return strncmp(NameStr(*name), str, NAMEDATALEN);
}


/*
 * SQL-functions CURRENT_USER, SESSION_USER
 */
Datum
current_user(PG_FUNCTION_ARGS)
{
	PG_RETURN_DATUM(DirectFunctionCall1(namein, CStringGetDatum(GetUserNameFromId(GetUserId()))));
}

Datum
session_user(PG_FUNCTION_ARGS)
{
	PG_RETURN_DATUM(DirectFunctionCall1(namein, CStringGetDatum(GetUserNameFromId(GetSessionUserId()))));
}


/*
 * SQL-functions CURRENT_SCHEMA, CURRENT_SCHEMAS
 */
Datum
current_schema(PG_FUNCTION_ARGS)
{
	List	   *search_path = fetch_search_path(false);
	char	   *nspname;

	if (search_path == NIL)
		PG_RETURN_NULL();
	nspname = get_namespace_name(lfirsto(search_path));
	if (!nspname)
		PG_RETURN_NULL();		/* recently-deleted namespace? */
	PG_RETURN_DATUM(DirectFunctionCall1(namein, CStringGetDatum(nspname)));
}

Datum
current_schemas(PG_FUNCTION_ARGS)
{
	List	   *search_path = fetch_search_path(PG_GETARG_BOOL(0));
	Datum	   *names;
	int			i;
	ArrayType  *array;

	/* +1 here is just to avoid palloc(0) error */
	names = (Datum *) palloc((length(search_path) + 1) * sizeof(Datum));
	i = 0;
	while (search_path)
	{
		char	   *nspname;

		nspname = get_namespace_name(lfirsto(search_path));
		if (nspname)			/* watch out for deleted namespace */
		{
			names[i] = DirectFunctionCall1(namein, CStringGetDatum(nspname));
			i++;
		}
		search_path = lnext(search_path);
	}

	array = construct_array(names, i,
							NAMEOID,
							NAMEDATALEN,		/* sizeof(Name) */
							false,		/* Name is not by-val */
							'i');		/* alignment of Name */

	PG_RETURN_POINTER(array);
}
