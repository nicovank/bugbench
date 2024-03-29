/*
 * interface functions to parser
 * Teodor Sigaev <teodor@sigaev.ru>
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "postgres.h"
#include "fmgr.h"
#include "utils/array.h"
#include "catalog/pg_type.h"
#include "executor/spi.h"
#include "funcapi.h"

#include "wparser.h"
#include "ts_cfg.h"
#include "snmap.h"
#include "common.h"

/*********top interface**********/

static void *plan_getparser = NULL;
static Oid	current_parser_id = InvalidOid;

void
init_prs(Oid id, WParserInfo * prs)
{
	Oid			arg[1] = {OIDOID};
	bool		isnull;
	Datum		pars[1] = {ObjectIdGetDatum(id)};
	int			stat;

	memset(prs, 0, sizeof(WParserInfo));
	SPI_connect();
	if (!plan_getparser)
	{
		plan_getparser = SPI_saveplan(SPI_prepare("select prs_start, prs_nexttoken, prs_end, prs_lextype, prs_headline from pg_ts_parser where oid = $1", 1, arg));
		if (!plan_getparser)
			ts_error(ERROR, "SPI_prepare() failed");
	}

	stat = SPI_execp(plan_getparser, pars, " ", 1);
	if (stat < 0)
		ts_error(ERROR, "SPI_execp return %d", stat);
	if (SPI_processed > 0)
	{
		Oid			oid = InvalidOid;

		oid = DatumGetObjectId(SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 1, &isnull));
		fmgr_info_cxt(oid, &(prs->start_info), TopMemoryContext);
		oid = DatumGetObjectId(SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 2, &isnull));
		fmgr_info_cxt(oid, &(prs->getlexeme_info), TopMemoryContext);
		oid = DatumGetObjectId(SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 3, &isnull));
		fmgr_info_cxt(oid, &(prs->end_info), TopMemoryContext);
		prs->lextype = DatumGetObjectId(SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 4, &isnull));
		oid = DatumGetObjectId(SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 5, &isnull));
		fmgr_info_cxt(oid, &(prs->headline_info), TopMemoryContext);
		prs->prs_id = id;
	}
	else
		ts_error(ERROR, "No parser with id %d", id);
	SPI_finish();
}

typedef struct
{
	WParserInfo *last_prs;
	int			len;
	int			reallen;
	WParserInfo *list;
	SNMap		name2id_map;
}	PrsList;

static PrsList PList = {NULL, 0, 0, NULL, {0, 0, NULL}};

void
reset_prs(void)
{
	freeSNMap(&(PList.name2id_map));
	if (PList.list)
		free(PList.list);
	memset(&PList, 0, sizeof(PrsList));
}

static int
compareprs(const void *a, const void *b)
{
	return ((WParserInfo *) a)->prs_id - ((WParserInfo *) b)->prs_id;
}

WParserInfo *
findprs(Oid id)
{
	/* last used prs */
	if (PList.last_prs && PList.last_prs->prs_id == id)
		return PList.last_prs;

	/* already used prs */
	if (PList.len != 0)
	{
		WParserInfo key;

		key.prs_id = id;
		PList.last_prs = bsearch(&key, PList.list, PList.len, sizeof(WParserInfo), compareprs);
		if (PList.last_prs != NULL)
			return PList.last_prs;
	}

	/* last chance */
	if (PList.len == PList.reallen)
	{
		WParserInfo *tmp;
		int			reallen = (PList.reallen) ? 2 * PList.reallen : 16;

		tmp = (WParserInfo *) realloc(PList.list, sizeof(WParserInfo) * reallen);
		if (!tmp)
			ts_error(ERROR, "No memory");
		PList.reallen = reallen;
		PList.list = tmp;
	}
	PList.last_prs = &(PList.list[PList.len]);
	init_prs(id, PList.last_prs);
	PList.len++;
	qsort(PList.list, PList.len, sizeof(WParserInfo), compareprs);
	return findprs(id); /* qsort changed order!! */ ;
}

static void *plan_name2id = NULL;

Oid
name2id_prs(text *name)
{
	Oid			arg[1] = {TEXTOID};
	bool		isnull;
	Datum		pars[1] = {PointerGetDatum(name)};
	int			stat;
	Oid			id = findSNMap_t(&(PList.name2id_map), name);

	if (id)
		return id;


	SPI_connect();
	if (!plan_name2id)
	{
		plan_name2id = SPI_saveplan(SPI_prepare("select oid from pg_ts_parser where prs_name = $1", 1, arg));
		if (!plan_name2id)
			ts_error(ERROR, "SPI_prepare() failed");
	}

	stat = SPI_execp(plan_name2id, pars, " ", 1);
	if (stat < 0)
		ts_error(ERROR, "SPI_execp return %d", stat);
	if (SPI_processed > 0)
		id = DatumGetObjectId(SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 1, &isnull));
	else
		ts_error(ERROR, "No parser '%s'", text2char(name));
	SPI_finish();
	addSNMap_t(&(PList.name2id_map), name, id);
	return id;
}


/******sql-level interface******/
typedef struct
{
	int			cur;
	LexDescr   *list;
}	TypeStorage;

static void
setup_firstcall(FuncCallContext *funcctx, Oid prsid)
{
	TupleDesc	tupdesc;
	MemoryContext oldcontext;
	TypeStorage *st;
	WParserInfo *prs = findprs(prsid);

	oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

	st = (TypeStorage *) palloc(sizeof(TypeStorage));
	st->cur = 0;
	st->list = (LexDescr *) DatumGetPointer(
				OidFunctionCall1(prs->lextype, PointerGetDatum(prs->prs))
		);
	funcctx->user_fctx = (void *) st;
	tupdesc = RelationNameGetTupleDesc("tokentype");
	funcctx->slot = TupleDescGetSlot(tupdesc);
	funcctx->attinmeta = TupleDescGetAttInMetadata(tupdesc);
	MemoryContextSwitchTo(oldcontext);
}

static Datum
process_call(FuncCallContext *funcctx)
{
	TypeStorage *st;

	st = (TypeStorage *) funcctx->user_fctx;
	if (st->list && st->list[st->cur].lexid)
	{
		Datum		result;
		char	   *values[3];
		char		txtid[16];
		HeapTuple	tuple;

		values[0] = txtid;
		sprintf(txtid, "%d", st->list[st->cur].lexid);
		values[1] = st->list[st->cur].alias;
		values[2] = st->list[st->cur].descr;

		tuple = BuildTupleFromCStrings(funcctx->attinmeta, values);
		result = TupleGetDatum(funcctx->slot, tuple);

		pfree(values[1]);
		pfree(values[2]);
		st->cur++;
		return result;
	}
	else
	{
		if (st->list)
			pfree(st->list);
		pfree(st);
	}
	return (Datum) 0;
}

PG_FUNCTION_INFO_V1(token_type);
Datum		token_type(PG_FUNCTION_ARGS);

Datum
token_type(PG_FUNCTION_ARGS)
{
	FuncCallContext *funcctx;
	Datum		result;

	if (SRF_IS_FIRSTCALL())
	{
		funcctx = SRF_FIRSTCALL_INIT();
		setup_firstcall(funcctx, PG_GETARG_OID(0));
	}

	funcctx = SRF_PERCALL_SETUP();

	if ((result = process_call(funcctx)) != (Datum) 0)
		SRF_RETURN_NEXT(funcctx, result);
	SRF_RETURN_DONE(funcctx);
}

PG_FUNCTION_INFO_V1(token_type_byname);
Datum		token_type_byname(PG_FUNCTION_ARGS);
Datum
token_type_byname(PG_FUNCTION_ARGS)
{
	FuncCallContext *funcctx;
	Datum		result;

	if (SRF_IS_FIRSTCALL())
	{
		text	   *name = PG_GETARG_TEXT_P(0);

		funcctx = SRF_FIRSTCALL_INIT();
		setup_firstcall(funcctx, name2id_prs(name));
		PG_FREE_IF_COPY(name, 0);
	}

	funcctx = SRF_PERCALL_SETUP();

	if ((result = process_call(funcctx)) != (Datum) 0)
		SRF_RETURN_NEXT(funcctx, result);
	SRF_RETURN_DONE(funcctx);
}

PG_FUNCTION_INFO_V1(token_type_current);
Datum		token_type_current(PG_FUNCTION_ARGS);
Datum
token_type_current(PG_FUNCTION_ARGS)
{
	FuncCallContext *funcctx;
	Datum		result;

	if (SRF_IS_FIRSTCALL())
	{
		funcctx = SRF_FIRSTCALL_INIT();
		if (current_parser_id == InvalidOid)
			current_parser_id = name2id_prs(char2text("default"));
		setup_firstcall(funcctx, current_parser_id);
	}

	funcctx = SRF_PERCALL_SETUP();

	if ((result = process_call(funcctx)) != (Datum) 0)
		SRF_RETURN_NEXT(funcctx, result);
	SRF_RETURN_DONE(funcctx);
}


PG_FUNCTION_INFO_V1(set_curprs);
Datum		set_curprs(PG_FUNCTION_ARGS);
Datum
set_curprs(PG_FUNCTION_ARGS)
{
	findprs(PG_GETARG_OID(0));
	current_parser_id = PG_GETARG_OID(0);
	PG_RETURN_VOID();
}

PG_FUNCTION_INFO_V1(set_curprs_byname);
Datum		set_curprs_byname(PG_FUNCTION_ARGS);
Datum
set_curprs_byname(PG_FUNCTION_ARGS)
{
	text	   *name = PG_GETARG_TEXT_P(0);

	DirectFunctionCall1(
						set_curprs,
						ObjectIdGetDatum(name2id_prs(name))
		);
	PG_FREE_IF_COPY(name, 0);
	PG_RETURN_VOID();
}

typedef struct
{
	int			type;
	char	   *lexem;
}	LexemEntry;

typedef struct
{
	int			cur;
	int			len;
	LexemEntry *list;
}	PrsStorage;


static void
prs_setup_firstcall(FuncCallContext *funcctx, int prsid, text *txt)
{
	TupleDesc	tupdesc;
	MemoryContext oldcontext;
	PrsStorage *st;
	WParserInfo *prs = findprs(prsid);
	char	   *lex = NULL;
	int			llen = 0,
				type = 0;

	oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

	st = (PrsStorage *) palloc(sizeof(PrsStorage));
	st->cur = 0;
	st->len = 16;
	st->list = (LexemEntry *) palloc(sizeof(LexemEntry) * st->len);

	prs->prs = (void *) DatumGetPointer(
										FunctionCall2(
													  &(prs->start_info),
										   PointerGetDatum(VARDATA(txt)),
								   Int32GetDatum(VARSIZE(txt) - VARHDRSZ)
													  )
		);

	while ((type = DatumGetInt32(FunctionCall3(
											   &(prs->getlexeme_info),
											   PointerGetDatum(prs->prs),
											   PointerGetDatum(&lex),
										  PointerGetDatum(&llen)))) != 0)
	{

		if (st->cur >= st->len)
		{
			st->len = 2 * st->len;
			st->list = (LexemEntry *) repalloc(st->list, sizeof(LexemEntry) * st->len);
		}
		st->list[st->cur].lexem = palloc(llen + 1);
		memcpy(st->list[st->cur].lexem, lex, llen);
		st->list[st->cur].lexem[llen] = '\0';
		st->list[st->cur].type = type;
		st->cur++;
	}

	FunctionCall1(
				  &(prs->end_info),
				  PointerGetDatum(prs->prs)
		);

	st->len = st->cur;
	st->cur = 0;

	funcctx->user_fctx = (void *) st;
	tupdesc = RelationNameGetTupleDesc("tokenout");
	funcctx->slot = TupleDescGetSlot(tupdesc);
	funcctx->attinmeta = TupleDescGetAttInMetadata(tupdesc);
	MemoryContextSwitchTo(oldcontext);
}

static Datum
prs_process_call(FuncCallContext *funcctx)
{
	PrsStorage *st;

	st = (PrsStorage *) funcctx->user_fctx;
	if (st->cur < st->len)
	{
		Datum		result;
		char	   *values[2];
		char		tid[16];
		HeapTuple	tuple;

		values[0] = tid;
		sprintf(tid, "%d", st->list[st->cur].type);
		values[1] = st->list[st->cur].lexem;
		tuple = BuildTupleFromCStrings(funcctx->attinmeta, values);
		result = TupleGetDatum(funcctx->slot, tuple);

		pfree(values[1]);
		st->cur++;
		return result;
	}
	else
	{
		if (st->list)
			pfree(st->list);
		pfree(st);
	}
	return (Datum) 0;
}



PG_FUNCTION_INFO_V1(parse);
Datum		parse(PG_FUNCTION_ARGS);
Datum
parse(PG_FUNCTION_ARGS)
{
	FuncCallContext *funcctx;
	Datum		result;

	if (SRF_IS_FIRSTCALL())
	{
		text	   *txt = PG_GETARG_TEXT_P(1);

		funcctx = SRF_FIRSTCALL_INIT();
		prs_setup_firstcall(funcctx, PG_GETARG_OID(0), txt);
		PG_FREE_IF_COPY(txt, 1);
	}

	funcctx = SRF_PERCALL_SETUP();

	if ((result = prs_process_call(funcctx)) != (Datum) 0)
		SRF_RETURN_NEXT(funcctx, result);
	SRF_RETURN_DONE(funcctx);
}

PG_FUNCTION_INFO_V1(parse_byname);
Datum		parse_byname(PG_FUNCTION_ARGS);
Datum
parse_byname(PG_FUNCTION_ARGS)
{
	FuncCallContext *funcctx;
	Datum		result;

	if (SRF_IS_FIRSTCALL())
	{
		text	   *name = PG_GETARG_TEXT_P(0);
		text	   *txt = PG_GETARG_TEXT_P(1);

		funcctx = SRF_FIRSTCALL_INIT();
		prs_setup_firstcall(funcctx, name2id_prs(name), txt);
		PG_FREE_IF_COPY(name, 0);
		PG_FREE_IF_COPY(txt, 1);
	}

	funcctx = SRF_PERCALL_SETUP();

	if ((result = prs_process_call(funcctx)) != (Datum) 0)
		SRF_RETURN_NEXT(funcctx, result);
	SRF_RETURN_DONE(funcctx);
}


PG_FUNCTION_INFO_V1(parse_current);
Datum		parse_current(PG_FUNCTION_ARGS);
Datum
parse_current(PG_FUNCTION_ARGS)
{
	FuncCallContext *funcctx;
	Datum		result;

	if (SRF_IS_FIRSTCALL())
	{
		text	   *txt = PG_GETARG_TEXT_P(0);

		funcctx = SRF_FIRSTCALL_INIT();
		if (current_parser_id == InvalidOid)
			current_parser_id = name2id_prs(char2text("default"));
		prs_setup_firstcall(funcctx, current_parser_id, txt);
		PG_FREE_IF_COPY(txt, 0);
	}

	funcctx = SRF_PERCALL_SETUP();

	if ((result = prs_process_call(funcctx)) != (Datum) 0)
		SRF_RETURN_NEXT(funcctx, result);
	SRF_RETURN_DONE(funcctx);
}

PG_FUNCTION_INFO_V1(headline);
Datum		headline(PG_FUNCTION_ARGS);
Datum
headline(PG_FUNCTION_ARGS)
{
	TSCfgInfo  *cfg = findcfg(PG_GETARG_OID(0));
	text	   *in = PG_GETARG_TEXT_P(1);
	QUERYTYPE  *query = (QUERYTYPE *) DatumGetPointer(PG_DETOAST_DATUM(PG_GETARG_DATUM(2)));
	text	   *opt = (PG_NARGS() > 3 && PG_GETARG_POINTER(3)) ? PG_GETARG_TEXT_P(3) : NULL;
	HLPRSTEXT	prs;
	text	   *out;
	WParserInfo *prsobj = findprs(cfg->prs_id);

	memset(&prs, 0, sizeof(HLPRSTEXT));
	prs.lenwords = 32;
	prs.words = (HLWORD *) palloc(sizeof(HLWORD) * prs.lenwords);
	hlparsetext(cfg, &prs, query, VARDATA(in), VARSIZE(in) - VARHDRSZ);


	FunctionCall3(
				  &(prsobj->headline_info),
				  PointerGetDatum(&prs),
				  PointerGetDatum(opt),
				  PointerGetDatum(query)
		);

	out = genhl(&prs);

	PG_FREE_IF_COPY(in, 1);
	PG_FREE_IF_COPY(query, 2);
	if (opt)
		PG_FREE_IF_COPY(opt, 3);
	pfree(prs.words);
	pfree(prs.startsel);
	pfree(prs.stopsel);

	PG_RETURN_POINTER(out);
}


PG_FUNCTION_INFO_V1(headline_byname);
Datum		headline_byname(PG_FUNCTION_ARGS);
Datum
headline_byname(PG_FUNCTION_ARGS)
{
	text	   *cfg = PG_GETARG_TEXT_P(0);

	Datum		out = DirectFunctionCall4(
										  headline,
									  ObjectIdGetDatum(name2id_cfg(cfg)),
										  PG_GETARG_DATUM(1),
										  PG_GETARG_DATUM(2),
			(PG_NARGS() > 3) ? PG_GETARG_DATUM(3) : PointerGetDatum(NULL)
	);

	PG_FREE_IF_COPY(cfg, 0);
	PG_RETURN_DATUM(out);
}

PG_FUNCTION_INFO_V1(headline_current);
Datum		headline_current(PG_FUNCTION_ARGS);
Datum
headline_current(PG_FUNCTION_ARGS)
{
	PG_RETURN_DATUM(DirectFunctionCall4(
										headline,
										ObjectIdGetDatum(get_currcfg()),
										PG_GETARG_DATUM(0),
										PG_GETARG_DATUM(1),
			(PG_NARGS() > 2) ? PG_GETARG_DATUM(2) : PointerGetDatum(NULL)
										));
}
