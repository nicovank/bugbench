#include "_int.h"

#define GETENTRY(vec,pos) ((GISTTYPE *) DatumGetPointer(((GISTENTRY *) VARDATA(vec))[(pos)].key))
/*
** _intbig methods
*/
PG_FUNCTION_INFO_V1(g_intbig_consistent);
PG_FUNCTION_INFO_V1(g_intbig_compress);
PG_FUNCTION_INFO_V1(g_intbig_decompress);
PG_FUNCTION_INFO_V1(g_intbig_penalty);
PG_FUNCTION_INFO_V1(g_intbig_picksplit);
PG_FUNCTION_INFO_V1(g_intbig_union);
PG_FUNCTION_INFO_V1(g_intbig_same);

Datum		g_intbig_consistent(PG_FUNCTION_ARGS);
Datum		g_intbig_compress(PG_FUNCTION_ARGS);
Datum		g_intbig_decompress(PG_FUNCTION_ARGS);
Datum		g_intbig_penalty(PG_FUNCTION_ARGS);
Datum		g_intbig_picksplit(PG_FUNCTION_ARGS);
Datum		g_intbig_union(PG_FUNCTION_ARGS);
Datum		g_intbig_same(PG_FUNCTION_ARGS);

#define SUMBIT(val) (       \
        GETBITBYTE((val),0) + \
        GETBITBYTE((val),1) + \
        GETBITBYTE((val),2) + \
        GETBITBYTE((val),3) + \
        GETBITBYTE((val),4) + \
        GETBITBYTE((val),5) + \
        GETBITBYTE((val),6) + \
        GETBITBYTE((val),7)   \
)

PG_FUNCTION_INFO_V1(_intbig_in);
Datum           _intbig_in(PG_FUNCTION_ARGS);
                                                                                
PG_FUNCTION_INFO_V1(_intbig_out);
Datum           _intbig_out(PG_FUNCTION_ARGS);   
                        
        
Datum
_intbig_in(PG_FUNCTION_ARGS) {
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("_intbig_in() not implemented")));
        PG_RETURN_DATUM(0);
}
                
Datum
_intbig_out(PG_FUNCTION_ARGS) {
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("_intbig_out() not implemented")));
        PG_RETURN_DATUM(0);
}                                


/*********************************************************************
** intbig functions
*********************************************************************/
static bool
_intbig_overlap(GISTTYPE *a, ArrayType *b)
{
	int			num=ARRNELEMS(b);
	int4 *ptr=ARRPTR(b);

	while(num--) {
		if (GETBIT(GETSIGN(a),HASHVAL(*ptr)))
			return true;
		ptr++;
	}

	return false;
}

static bool
_intbig_contains(GISTTYPE *a, ArrayType *b)
{
	int			num=ARRNELEMS(b);
	int4 *ptr=ARRPTR(b);

	while(num--) {
		if (!GETBIT(GETSIGN(a),HASHVAL(*ptr)))
			return false;
		ptr++;
	}

	return true;
}

Datum
g_intbig_same(PG_FUNCTION_ARGS) {
	GISTTYPE   *a = (GISTTYPE *) PG_GETARG_POINTER(0);
	GISTTYPE   *b = (GISTTYPE *) PG_GETARG_POINTER(1);
	bool       *result = (bool *) PG_GETARG_POINTER(2);

	if (ISALLTRUE(a) && ISALLTRUE(b))
		*result = true;
	else if (ISALLTRUE(a))
		*result = false;
	else if (ISALLTRUE(b))
		*result = false;
	else {
		int4            i;
		BITVECP         sa = GETSIGN(a),
		sb = GETSIGN(b);
		*result = true;
		LOOPBYTE(
			if (sa[i] != sb[i]) {
				*result = false;
				break;
			}
		);
	}
	PG_RETURN_POINTER(result);
}

Datum
g_intbig_compress(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);

	if (entry->leafkey) {
		GISTENTRY  *retval;
		ArrayType *in = (ArrayType *) PG_DETOAST_DATUM(entry->key);
		int4 *ptr;
		int num;
		GISTTYPE *res=(GISTTYPE*)palloc(CALCGTSIZE(0));

		ARRISVOID(in);

		ptr=ARRPTR(in);
		num=ARRNELEMS(in);
		memset(res,0,CALCGTSIZE(0));
		res->len=CALCGTSIZE(0);

		while(num--) {
			HASH(GETSIGN(res),*ptr);
			ptr++;
		}

		retval = (GISTENTRY *) palloc(sizeof(GISTENTRY));
		gistentryinit(*retval, PointerGetDatum(res),
			entry->rel, entry->page,
			entry->offset, res->len, FALSE);
		
		if ( in!=(ArrayType *) PG_DETOAST_DATUM(entry->key) )
			pfree(in);

		PG_RETURN_POINTER(retval);
	} else if ( !ISALLTRUE(DatumGetPointer(entry->key)) ) {
		GISTENTRY  *retval;
		int	i;
		BITVECP         sign = GETSIGN(DatumGetPointer(entry->key));
		GISTTYPE   *res;

		LOOPBYTE(
			if ((sign[i] & 0xff) != 0xff)
				PG_RETURN_POINTER(entry);
		);

		res=(GISTTYPE*)palloc(CALCGTSIZE(ALLISTRUE));	
		res->len=CALCGTSIZE(ALLISTRUE);
		res->flag = ALLISTRUE;

		retval = (GISTENTRY *) palloc(sizeof(GISTENTRY));
		gistentryinit(*retval, PointerGetDatum(res),
			entry->rel, entry->page,
			entry->offset, res->len, FALSE);
		
		PG_RETURN_POINTER(retval);
	}
	
	PG_RETURN_POINTER(entry);
}


static int4
sizebitvec(BITVECP sign) {
	int4            size = 0, i;
	LOOPBYTE(
		size += SUMBIT(sign);
		sign = (BITVECP) (((char *) sign) + 1);
	);
        return size;
}

static int
hemdistsign(BITVECP  a, BITVECP b) {
        int i,dist=0;
        
        LOOPBIT(
                if ( GETBIT(a,i) != GETBIT(b,i) )
                        dist++;
        );
        return dist;
}

static int
hemdist(GISTTYPE   *a, GISTTYPE   *b) {
        if ( ISALLTRUE(a) ) {
                if (ISALLTRUE(b))
                        return 0;
                else
                        return SIGLENBIT-sizebitvec(GETSIGN(b));
        } else if (ISALLTRUE(b))
                return SIGLENBIT-sizebitvec(GETSIGN(a));
 
        return hemdistsign( GETSIGN(a), GETSIGN(b) );
}

Datum
g_intbig_decompress(PG_FUNCTION_ARGS)
{
	PG_RETURN_DATUM(PG_GETARG_DATUM(0));
}

static int4
unionkey(BITVECP sbase, GISTTYPE * add)
{
	int4            i;
	BITVECP         sadd = GETSIGN(add);

	if (ISALLTRUE(add))
		return 1;
	LOOPBYTE(
		sbase[i] |= sadd[i];
	);
	return 0;
}

Datum
g_intbig_union(PG_FUNCTION_ARGS) {
	bytea      *entryvec = (bytea *) PG_GETARG_POINTER(0);
	int                *size = (int *) PG_GETARG_POINTER(1);
	BITVEC          base;
	int4            len = (VARSIZE(entryvec) - VARHDRSZ) / sizeof(GISTENTRY);
	int4            i;
	int4            flag = 0;
	GISTTYPE   *result;

	MemSet((void *) base, 0, sizeof(BITVEC));
	for (i = 0; i < len; i++) {
		if (unionkey(base, GETENTRY(entryvec, i))) {
			flag = ALLISTRUE;
			break;
		}
	}

	len = CALCGTSIZE(flag);
	result = (GISTTYPE *) palloc(len);
	*size = result->len = len;
	result->flag = flag;
	if (!ISALLTRUE(result))
		memcpy((void *) GETSIGN(result), (void *) base, sizeof(BITVEC));
 
	PG_RETURN_POINTER(result);
}

Datum
g_intbig_penalty(PG_FUNCTION_ARGS) {
	GISTENTRY  *origentry = (GISTENTRY *) PG_GETARG_POINTER(0); /* always ISSIGNKEY */
	GISTENTRY  *newentry = (GISTENTRY *) PG_GETARG_POINTER(1);
	float      *penalty = (float *) PG_GETARG_POINTER(2);
	GISTTYPE   *origval = (GISTTYPE *) DatumGetPointer(origentry->key);
	GISTTYPE   *newval = (GISTTYPE *) DatumGetPointer(newentry->key);

	*penalty=hemdist(origval,newval);
	PG_RETURN_POINTER(penalty);
}


typedef struct {
	OffsetNumber pos;
	int4            cost;
} SPLITCOST;

static int
comparecost(const void *a, const void *b) {
	return ((SPLITCOST *) a)->cost - ((SPLITCOST *) b)->cost;
}


Datum
g_intbig_picksplit(PG_FUNCTION_ARGS) {
        bytea      *entryvec = (bytea *) PG_GETARG_POINTER(0);
        GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);
        OffsetNumber k,
                                j;
        GISTTYPE *datum_l,
                           *datum_r;
        BITVECP         union_l,
                                union_r;
        int4            size_alpha, size_beta;
        int4            size_waste,
                                waste = -1;
        int4            nbytes;
        OffsetNumber seed_1 = 0,
                                seed_2 = 0;
        OffsetNumber *left,
                           *right;
        OffsetNumber maxoff;
        BITVECP         ptr;
        int                     i;
        SPLITCOST  *costvector;
        GISTTYPE *_k,
                           *_j;

        maxoff = ((VARSIZE(entryvec) - VARHDRSZ) / sizeof(GISTENTRY)) - 2;
        nbytes = (maxoff + 2) * sizeof(OffsetNumber);
        v->spl_left = (OffsetNumber *) palloc(nbytes);
        v->spl_right = (OffsetNumber *) palloc(nbytes);

        for (k = FirstOffsetNumber; k < maxoff; k = OffsetNumberNext(k)) {
                _k = GETENTRY(entryvec, k);
                for (j = OffsetNumberNext(k); j <= maxoff; j = OffsetNumberNext(j)) {
                        size_waste=hemdist(_k, GETENTRY(entryvec, j));
                        if (size_waste > waste ) {
                                waste = size_waste;
                                seed_1 = k;
                                seed_2 = j;
                        }
                }
        }

        left = v->spl_left;
        v->spl_nleft = 0;
        right = v->spl_right;
        v->spl_nright = 0;

        if (seed_1 == 0 || seed_2 == 0)
        {
                seed_1 = 1;
                seed_2 = 2;
        }

        /* form initial .. */
        if (ISALLTRUE(GETENTRY(entryvec, seed_1)))
        {
                datum_l = (GISTTYPE *) palloc(GTHDRSIZE);
                datum_l->len = GTHDRSIZE;
                datum_l->flag = ALLISTRUE;
        }
        else
        {
                datum_l = (GISTTYPE *) palloc(GTHDRSIZE + SIGLEN);
                datum_l->len = GTHDRSIZE + SIGLEN;
                datum_l->flag = 0;
                memcpy((void *) GETSIGN(datum_l), (void *) GETSIGN(GETENTRY(entryvec, seed_1)), sizeof(BITVEC));
        }
        if (ISALLTRUE(GETENTRY(entryvec, seed_2)))
        {
                datum_r = (GISTTYPE *) palloc(GTHDRSIZE);
                datum_r->len = GTHDRSIZE;
                datum_r->flag = ALLISTRUE;
        }
        else
        {
                datum_r = (GISTTYPE *) palloc(GTHDRSIZE + SIGLEN);
                datum_r->len = GTHDRSIZE + SIGLEN;
                datum_r->flag = 0;
                memcpy((void *) GETSIGN(datum_r), (void *) GETSIGN(GETENTRY(entryvec, seed_2)), sizeof(BITVEC));
        }

        maxoff = OffsetNumberNext(maxoff);
        /* sort before ... */
        costvector = (SPLITCOST *) palloc(sizeof(SPLITCOST) * maxoff);
        for (j = FirstOffsetNumber; j <= maxoff; j = OffsetNumberNext(j))
        {
                costvector[j - 1].pos = j;
                _j = GETENTRY(entryvec, j);
                size_alpha = hemdist(datum_l,_j);
                size_beta  = hemdist(datum_r,_j);
                costvector[j - 1].cost = abs(size_alpha - size_beta);
        }
        qsort((void *) costvector, maxoff, sizeof(SPLITCOST), comparecost);

        union_l=GETSIGN(datum_l);
        union_r=GETSIGN(datum_r);

        for (k = 0; k < maxoff; k++)
        {
                j = costvector[k].pos;
                if (j == seed_1)
                {
                        *left++ = j;
                        v->spl_nleft++;
                        continue;
                }
                else if (j == seed_2)
                {
                        *right++ = j;
                        v->spl_nright++;
                        continue;
                }
                _j = GETENTRY(entryvec, j);
                size_alpha = hemdist(datum_l,_j);
                size_beta  = hemdist(datum_r,_j);

                if (size_alpha < size_beta  + WISH_F(v->spl_nleft, v->spl_nright, 0.00001))
                {
                        if (ISALLTRUE(datum_l) || ISALLTRUE(_j) ) {
                                if (!ISALLTRUE(datum_l))
                                        MemSet((void *) union_l, 0xff, sizeof(BITVEC));
                        } else {
                                ptr=GETSIGN(_j);
                                LOOPBYTE(
                                        union_l[i] |= ptr[i];
                                );
                        }
                        *left++ = j;
                        v->spl_nleft++;
                }
                else
                {
                        if (ISALLTRUE(datum_r) || ISALLTRUE(_j) ) {
                                if (!ISALLTRUE(datum_r))
                                        MemSet((void *) union_r, 0xff, sizeof(BITVEC));
                        } else {
                                ptr=GETSIGN(_j);
                                LOOPBYTE(
                                        union_r[i] |= ptr[i];
                                );
                        }
                        *right++ = j;
                        v->spl_nright++;
                }
        }

        *right = *left = FirstOffsetNumber;
        pfree(costvector);

        v->spl_ldatum = PointerGetDatum(datum_l);
        v->spl_rdatum = PointerGetDatum(datum_r);

        PG_RETURN_POINTER(v);
}

Datum
g_intbig_consistent(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	ArrayType  *query = (ArrayType *) PG_GETARG_POINTER(1);
	StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
	bool		retval;

	if ( ISALLTRUE(DatumGetPointer(entry->key)) )
		PG_RETURN_BOOL(true);
	
	if (strategy == BooleanSearchStrategy) {
		PG_RETURN_BOOL(signconsistent((QUERYTYPE *) query,
					   GETSIGN(DatumGetPointer(entry->key)),
									  false));
	}

	/* XXX what about toasted input? */
	if (ARRISVOID(query))
		return FALSE;

	switch (strategy)
	{
		case RTOverlapStrategyNumber:
			retval = _intbig_overlap((GISTTYPE *) DatumGetPointer(entry->key), query);
			break;
		case RTSameStrategyNumber:
			if (GIST_LEAF(entry)) {
				int i,num=ARRNELEMS(query);
				int4 *ptr=ARRPTR(query);
				BITVEC	qp;
				BITVECP dq, de;
				memset(qp,0,sizeof(BITVEC));

				while(num--) {
					HASH(qp, *ptr);
					ptr++;
				}

				de=GETSIGN((GISTTYPE *) DatumGetPointer(entry->key));
				dq=qp;
				retval=true;
				LOOPBYTE(
					if ( de[i] != dq[i] ) {
						retval=false;
						break;
					}
				);

			} else
				retval = _intbig_contains((GISTTYPE *) DatumGetPointer(entry->key), query);
			break;
		case RTContainsStrategyNumber:
			retval = _intbig_contains((GISTTYPE *) DatumGetPointer(entry->key), query);
			break;
		case RTContainedByStrategyNumber:
			if (GIST_LEAF(entry)) {
				int i,num=ARRNELEMS(query);
				int4 *ptr=ARRPTR(query);
				BITVEC	qp;
				BITVECP dq, de;
				memset(qp,0,sizeof(BITVEC));

				while(num--) {
					HASH(qp, *ptr);
					ptr++;
				}

				de=GETSIGN((GISTTYPE *) DatumGetPointer(entry->key));
				dq=qp;
				retval=true;
				LOOPBYTE(
					if ( de[i] & ~dq[i] ) {
						retval=false;
						break;
					}
				);

			} else
				retval = _intbig_overlap((GISTTYPE *) DatumGetPointer(entry->key), query);
			break;
		default:
			retval = FALSE;
	}
	PG_RETURN_BOOL(retval);
}


