%{
/*-------------------------------------------------------------------------
 *
 * pgc.l
 *	  lexical scanner for ecpg
 *
 * This is a modified version of src/backend/parser/scan.l
 *
 *
 * Portions Copyright (c) 1996-2003, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/interfaces/ecpg/preproc/pgc.l,v 1.122.2.2 2004/02/15 13:50:02 meskes Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres_fe.h"

#include <ctype.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>

#include "extern.h"

extern YYSTYPE yylval;

static int		xcdepth = 0;	/* depth of nesting in slash-star comments */

/*
 * literalbuf is used to accumulate literal values when multiple rules
 * are needed to parse a single literal.  Call startlit to reset buffer
 * to empty, addlit to add text.  Note that the buffer is permanently
 * malloc'd to the largest size needed so far in the current run.
 */
static char    *literalbuf = NULL;		/* expandable buffer */
static int		literallen;				/* actual current length */
static int		literalalloc;			/* current allocated buffer size */

#define startlit()	(literalbuf[0] = '\0', literallen = 0)
static void addlit(char *ytext, int yleng);
static void addlitchar (unsigned char);
static void parse_include (void);

char *token_start;
int state_before;

struct _yy_buffer 
{ 
	YY_BUFFER_STATE		buffer;
	long				lineno;
	char		  		*filename;
	struct _yy_buffer 	*next;
} *yy_buffer = NULL;

static char *old;

#define MAX_NESTED_IF 128
static short preproc_tos;
static short ifcond;
static struct _if_value 
{
	short condition;
	short else_branch;
} stacked_if_value[MAX_NESTED_IF];

%}

%option 8bit
%option never-interactive
%option noyywrap

%option yylineno

%s C SQL incl def def_ident

/*
 * OK, here is a short description of lex/flex rules behavior.
 * The longest pattern which matches an input string is always chosen.
 * For equal-length patterns, the first occurring in the rules list is chosen.
 * INITIAL is the starting state, to which all non-conditional rules apply.
 * Exclusive states change parsing rules while the state is active.  When in
 * an exclusive state, only those rules defined for that state apply.
 *
 * We use exclusive states for quoted strings, extended comments,
 * and to eliminate parsing troubles for numeric strings.
 * Exclusive states:
 *	<xb> bit string literal
 *	<xc> extended C-style comments - thomas 1997-07-12
 *	<xd> delimited identifiers (double-quoted identifiers) - thomas 1997-10-27
 *	<xh> hexadecimal numeric string - thomas 1997-11-16
 *	<xq> quoted strings - thomas 1997-07-30
 */

%x xb
%x xc
%x xd
%x xdc
%x xh
%x xq
%x xpre
%x xcond
%x xskip

/* Bit string
 */
xbstart			[bB]{quote}
xbstop			{quote}
xbinside		[^']*
xbcat			{quote}{whitespace_with_newline}{quote}

/* Hexadecimal number
 */
xhstart			[xX]{quote}
xhstop			{quote}
xhinside		[^']*
xhcat			{quote}{whitespace_with_newline}{quote}

/* National character
 */
xnstart                        [nN]{quote}

/* C version of hex number
 */
xch			0[xX][0-9A-Fa-f]*

/* Extended quote
 * xqdouble implements SQL92 embedded quote
 * xqcat allows strings to cross input lines
 */
quote			'
xqstart			{quote}
xqstop			{quote}
xqdouble		{quote}{quote}
xqinside		[^\\']+
xqescape                [\\][^0-7]
xqoctesc                [\\][0-7]{1,3}
xqcat			{quote}{whitespace_with_newline}{quote}

/* Double quote
 * Allows embedded spaces and other special characters into identifiers.
 */
dquote			\"
xdstart			{dquote}
xdstop			{dquote}
xddouble				{dquote}{dquote}
xdinside		[^"]+

/* special stuff for C strings */
xdcqq			\\\\
xdcqdq			\\\"
xdcother		[^"]
xdcinside		({xdcqq}|{xdcqdq}|{xdcother})

/* C-style comments
 *
 * The "extended comment" syntax closely resembles allowable operator syntax.
 * The tricky part here is to get lex to recognize a string starting with
 * slash-star as a comment, when interpreting it as an operator would produce
 * a longer match --- remember lex will prefer a longer match!	Also, if we
 * have something like plus-slash-star, lex will think this is a 3-character
 * operator whereas we want to see it as a + operator and a comment start.
 * The solution is two-fold:
 * 1. append {op_chars}* to xcstart so that it matches as much text as
 *	  {operator} would. Then the tie-breaker (first matching rule of same
 *	  length) ensures xcstart wins.  We put back the extra stuff with yyless()
 *	  in case it contains a star-slash that should terminate the comment.
 * 2. In the operator rule, check for slash-star within the operator, and
 *	  if found throw it back with yyless().  This handles the plus-slash-star
 *	  problem.
 * SQL92-style comments, which start with dash-dash, have similar interactions
 * with the operator rule.
 */
xcstart			\/\*{op_chars}*
xcstop			\*+\/
xcinside		[^*/]+

digit			[0-9]
ident_start		[A-Za-z\200-\377_]
ident_cont		[A-Za-z\200-\377_0-9\$]

identifier		{ident_start}{ident_cont}*

array			({ident_cont}|{whitespace}|[\[\]\+\-\*\%\/\(\)])*
typecast		"::"

/*
 * "self" is the set of chars that should be returned as single-character
 * tokens.	"op_chars" is the set of chars that can make up "Op" tokens,
 * which can be one or more characters long (but if a single-char token
 * appears in the "self" set, it is not to be returned as an Op).  Note
 * that the sets overlap, but each has some chars that are not in the other.
 *
 * If you change either set, adjust the character lists appearing in the
 * rule for "operator"!
 */
self			[,()\[\].;\:\+\-\*\/\%\^\<\>\=]
op_chars		[\~\!\@\#\^\&\|\`\?\+\-\*\/\%\<\>\=]
operator		{op_chars}+

/* we no longer allow unary minus in numbers.
 * instead we pass it separately to parser. there it gets
 * coerced via doNegate() -- Leon aug 20 1999
 */

integer			{digit}+
decimal			(({digit}*\.{digit}+)|({digit}+\.{digit}*))
real			((({digit}*\.{digit}+)|({digit}+\.{digit}*)|({digit}+))([Ee][-+]?{digit}+))

param			\${integer}

/*
 * In order to make the world safe for Windows and Mac clients as well as
 * Unix ones, we accept either \n or \r as a newline.  A DOS-style \r\n
 * sequence will be seen as two successive newlines, but that doesn't cause
 * any problems.  SQL92-style comments, which start with -- and extend to the
 * next newline, are treated as equivalent to a single whitespace character.
 *
 * NOTE a fine point: if there is no newline following --, we will absorb
 * everything to the end of the input as a comment.  This is correct.  Older
 * versions of Postgres failed to recognize -- as a comment if the input
 * did not end with a newline.
 *
 * XXX perhaps \f (formfeed) should be treated as a newline as well?
 */

ccomment		"//".*\n

space			[ \t\n\r\f]
horiz_space		[ \t\f]
newline			[\n\r]
non_newline		[^\n\r]

comment			("--"{non_newline}*)

whitespace		({space}+|{comment})

/*
 * SQL92 requires at least one newline in the whitespace separating
 * string literals that are to be concatenated.  Silly, but who are we
 * to argue?  Note that {whitespace_with_newline} should not have * after
 * it, whereas {whitespace} should generally have a * after it...
 */

horiz_whitespace	({horiz_space}|{comment})
whitespace_with_newline ({horiz_whitespace}*{newline}{whitespace}*)

/* special characters for other dbms */
/* we have to react differently in compat mode */
informix_special	[\$]

other			.

/* some stuff needed for ecpg */
exec	[eE][xX][eE][cC]
sql		[sS][qQ][lL]
define	[dD][eE][fF][iI][nN][eE]
include [iI][nN][cC][lL][uU][dD][eE]

ifdef	[iI][fF][dD][eE][fF]
ifndef	[iI][fF][nN][dD][eE][fF]
else	[eE][lL][sS][eE]
elif	[eE][lL][iI][fF]
endif	[eE][nN][dD][iI][fF]

struct	[sS][tT][rR][uU][cC][tT]

exec_sql		{exec}{space}*{sql}{space}*
ipdigit			({digit}|{digit}{digit}|{digit}{digit}{digit})
ip			{ipdigit}\.{ipdigit}\.{ipdigit}\.{ipdigit}

/* we might want to parse all cpp include files */
cppinclude 		{space}*#{include}{space}*

/* Take care of cpp continuation lines */
cppline			{space}*#(.*\\{space})+.*

/*
 * Quoted strings must allow some special characters such as single-quote
 *	and newline.
 * Embedded single-quotes are implemented both in the SQL92-standard
 *	style of two adjacent single quotes "''" and in the Postgres/Java style
 *	of escaped-quote "\'".
 * Other embedded escaped characters are matched explicitly and the leading
 *	backslash is dropped from the string. - thomas 1997-09-24
 * Note that xcstart must appear before operator, as explained above!
 *	Also whitespace (comment) must appear before operator.
 */

%%

%{
                                       /* code to execute during start of each call of yylex() */
                                       token_start = NULL;
%}

<SQL>{whitespace}	{ /* ignore */ }

{xcstart}			{
						token_start = yytext;
						state_before = YYSTATE;
						xcdepth = 0;
						BEGIN(xc);
						/* Put back any characters past slash-star; see above */
						yyless(2);
						fputs("/*", yyout);
					}
<xc>{xcstart}		{
						xcdepth++;
				 		/* Put back any characters past slash-star; see above */
				 		yyless(2);
						fputs("/*", yyout);
					}

<xc>{xcstop}		{
						ECHO;
						if (xcdepth <= 0)
						{
							BEGIN(state_before);
							token_start = NULL;
						}
						else
							xcdepth--;
					}

<xc>{xcinside}		{ ECHO; }
<xc>{op_chars}		{ ECHO; }

<xc><<EOF>>		{ mmerror(PARSE_ERROR, ET_FATAL, "Unterminated /* comment"); }

<SQL>{xbstart}	{
						token_start = yytext;
						BEGIN(xb);
						startlit();
						addlitchar('b');
					}
<xb>{xbstop}	{
						BEGIN(SQL);
						if (literalbuf[strspn(literalbuf, "01") + 1] != '\0')
							mmerror(PARSE_ERROR, ET_ERROR, "invalid bit string input.");
						yylval.str = literalbuf;
						return BCONST;
					}

<xh>{xhinside}	|
<xb>{xbinside}	{ addlit(yytext, yyleng); }
<xh>{xhcat}		|
<xb>{xbcat}		{ /* ignore */ }
<xb><<EOF>>		{ mmerror(PARSE_ERROR, ET_FATAL, "Unterminated bit string"); }

<SQL>{xhstart}		{
						token_start = yytext;
						BEGIN(xh);
						startlit();
						addlitchar('x');
					}
<xh>{xhstop}		{
						yylval.str = literalbuf;
						return XCONST;
					}

<xh><<EOF>>		{ mmerror(PARSE_ERROR, ET_FATAL, "Unterminated hexadecimal integer"); }
<SQL>{xnstart}              {
				/* National character.
				 * Need to remember type info to flow it forward into the parser.
		                 * Not yet implemented. - thomas 2002-06-17
		                 */
			        token_start = yytext;
				BEGIN(xq);
				startlit();
			}
<C,SQL>{xqstart}	{
						token_start = yytext;
						state_before = YYSTATE;
						BEGIN(xq);
						startlit();
					}
<xq>{xqstop}		{
						BEGIN(state_before);
						yylval.str = mm_strdup(literalbuf);
						return SCONST;
					}
<xq>{xqdouble}		{ addlitchar('\''); }
<xq>{xqinside}		{ addlit(yytext, yyleng); }
<xq>{xqescape}  	{ addlit(yytext, yyleng); }
<xq>{xqoctesc}          { addlit(yytext, yyleng); }
<xq>{xqcat}		{ /* ignore */ }

<xq><<EOF>>		{ mmerror(PARSE_ERROR, ET_FATAL, "Unterminated quoted string"); }

<SQL>{xdstart}		{
						state_before = YYSTATE;
						BEGIN(xd);
						startlit();
					}
<xd>{xdstop}		{
						BEGIN(state_before);
						if (literallen == 0)
							mmerror(PARSE_ERROR, ET_ERROR, "zero-length delimited identifier");
						yylval.str = mm_strdup(literalbuf);
						return CSTRING;
					}
<xdc>{xdstop}		{
						BEGIN(state_before);
						yylval.str = mm_strdup(literalbuf);
						return CSTRING;
					}
<xd>{xddouble}		{ addlitchar('"'); }
<xd>{xdinside}		{ addlit(yytext, yyleng); }
<xd,xdc><<EOF>>		{ mmerror(PARSE_ERROR, ET_FATAL, "Unterminated quoted identifier"); }
<C,SQL>{xdstart}	{
						state_before = YYSTATE;
						BEGIN(xdc);
						startlit();
					}
<xdc>{xdcinside}	{ addlit(yytext, yyleng); }
<SQL>{typecast}		{ return TYPECAST; }
<SQL>{informix_special}	{
			  /* are we simulating Informix? */
                          if (INFORMIX_MODE)
			  {
			  	unput(':');
			  }
			  else
				return yytext[0];
			}
<SQL>{self}			{ /*
					   * We may find a ';' inside a structure
					   * definition in a TYPE or VAR statement.
					   * This is not an EOL marker.
					   */
					  if (yytext[0] == ';' && struct_level == 0)
						 BEGIN C;
					  return yytext[0];
					}
<SQL>{operator}		{
						/*
						 * Check for embedded slash-star or dash-dash; those
						 * are comment starts, so operator must stop there.
						 * Note that slash-star or dash-dash at the first
						 * character will match a prior rule, not this one.
						 */
						int		nchars = yyleng;
						char   *slashstar = strstr(yytext, "/*");
						char   *dashdash = strstr(yytext, "--");

						if (slashstar && dashdash)
						{
							/* if both appear, take the first one */
							if (slashstar > dashdash)
								slashstar = dashdash;
						}
						else if (!slashstar)
							slashstar = dashdash;
						if (slashstar)
							nchars = slashstar - yytext;

						/*
						 * For SQL92 compatibility, '+' and '-' cannot be the
						 * last char of a multi-char operator unless the operator
						 * contains chars that are not in SQL92 operators.
						 * The idea is to lex '=-' as two operators, but not
						 * to forbid operator names like '?-' that could not be
						 * sequences of SQL92 operators.
						 */
						while (nchars > 1 &&
							   (yytext[nchars-1] == '+' ||
								yytext[nchars-1] == '-'))
						{
							int		ic;

							for (ic = nchars-2; ic >= 0; ic--)
							{
								if (strchr("~!@#^&|`?%", yytext[ic]))
									break;
							}
							if (ic >= 0)
								break; /* found a char that makes it OK */
							nchars--; /* else remove the +/-, and check again */
						}

						if (nchars < yyleng)
						{
							/* Strip the unwanted chars from the token */
							yyless(nchars);
							/*
							 * If what we have left is only one char, and it's
							 * one of the characters matching "self", then
							 * return it as a character token the same way
							 * that the "self" rule would have.
							 */
							if (nchars == 1 &&
								strchr(",()[].;:+-*/%^<>=", yytext[0]))
								return yytext[0];
						}

						/* Convert "!=" operator to "<>" for compatibility */
						if (strcmp(yytext, "!=") == 0)
							yylval.str = mm_strdup("<>");
						else
							yylval.str = mm_strdup(yytext);
						return Op;
					}
<SQL>{param}		{
						yylval.ival = atol(yytext+1);
						return PARAM;
					}
<C,SQL>{integer}	{
						long val;
						char* endptr;

						errno = 0;
						val = strtol((char *)yytext, &endptr,10);
						if (*endptr != '\0' || errno == ERANGE
#ifdef HAVE_LONG_INT_64
							/* if long > 32 bits, check for overflow of int4 */
							|| val != (long) ((int32) val)
#endif
							)
						{
							errno = 0;
							yylval.str = mm_strdup(yytext);
							return FCONST;
						}
						yylval.ival = val;
						return ICONST;
					}
<SQL>{ip}			{
						yylval.str = mm_strdup(yytext);
						return IP;
					}
{decimal}			{
						yylval.str = mm_strdup(yytext);
						return FCONST;
					}
<C,SQL>{real}		{
						yylval.str = mm_strdup(yytext);
						return FCONST;
					}
<SQL>:{identifier}((("->"|\.){identifier})|(\[{array}\]))*	{
						yylval.str = mm_strdup(yytext+1);
						return(CVARIABLE);
					}
<SQL>{identifier}	{
						ScanKeyword    *keyword;
						struct _defines *ptr;

						/* How about a DEFINE? */
						for (ptr = defines; ptr; ptr = ptr->next)
						{
							if (strcmp(yytext, ptr->old) == 0)
							{
								struct _yy_buffer *yb;

								yb = mm_alloc(sizeof(struct _yy_buffer));

								yb->buffer =  YY_CURRENT_BUFFER;
								yb->lineno = yylineno;
								yb->filename = mm_strdup(input_filename);
								yb->next = yy_buffer;

								yy_buffer = yb;

								yy_scan_string(ptr->new);
								break;
							}
						}

						if (ptr == NULL)
						{
							/* Is it an SQL keyword? */
							keyword = ScanKeywordLookup(yytext);
							if (keyword != NULL)
								return keyword->value;

							/* Is it an ECPG keyword? */
							keyword = ScanECPGKeywordLookup( yytext);
							if (keyword != NULL)
								return keyword->value;

							/* Is it a C keyword? */
							keyword = ScanCKeywordLookup(yytext);
							if (keyword != NULL)
								return keyword->value;

							/*
							 * None of the above.  Return it as an identifier.
							 *
							 * The backend would attempt to truncate and case-fold
							 * the identifier, but I see no good reason for ecpg
							 * to do so; that's just another way that ecpg could get
							 * out of step with the backend.
							 */
							if (ptr == NULL)
							{
								yylval.str = mm_strdup(yytext);
								return IDENT;
							}
						}
					}
<SQL>{other}		{ return yytext[0]; }
<C>{exec_sql}		{ BEGIN SQL; return SQL_START; }
<C>{informix_special}	{ 
			  /* are we simulating Informix? */
			  if (INFORMIX_MODE)
			  {
			  	BEGIN SQL;
				return SQL_START;
			  }
			  else
			  	return S_ANYTHING;
			 }
<C>{ccomment}		 { ECHO; }
<C>{xch}			{
						char* endptr;

						errno = 0;
						yylval.ival = strtoul((char *)yytext,&endptr,16);
						if (*endptr != '\0' || errno == ERANGE)
						{
							errno = 0;
							yylval.str = mm_strdup(yytext);
							return SCONST;
						}
						return ICONST;
					}
<C>{cppinclude}		{
					  if (system_includes)
					  {
					  	BEGIN(incl);
					  }
					  else
					  {
						yylval.str = mm_strdup(yytext);
						return(CPP_LINE);
					  }
			}
<C>{cppline}		{
						yylval.str = mm_strdup(yytext);
						return(CPP_LINE);
					}
<C>{identifier} 	{
						ScanKeyword		*keyword;
						struct _defines *ptr;

						/* is it a define? */
						for (ptr = defines; ptr; ptr = ptr->next)
						{
							if (strcmp(yytext, ptr->old) == 0)
							{
								struct _yy_buffer *yb;

								yb = mm_alloc(sizeof(struct _yy_buffer));

								yb->buffer =  YY_CURRENT_BUFFER;
								yb->lineno = yylineno;
								yb->filename = mm_strdup(input_filename);
								yb->next = yy_buffer;

								yy_buffer = yb;

								yy_scan_string(ptr->new);
								break;
							}
						}

						if (ptr == NULL)
						{
							keyword = ScanCKeywordLookup(yytext);
							if (keyword != NULL)
								return keyword->value;

							else {
								yylval.str = mm_strdup(yytext);
								return IDENT;
							}
						}
					}
<C>";"				{ return(';'); }
<C>","				{ return(','); }
<C>"*"				{ return('*'); }
<C>"%"				{ return('%'); }
<C>"/"				{ return('/'); }
<C>"+"				{ return('+'); }
<C>"-"				{ return('-'); }
<C>"("				{ return('('); }
<C>")"				{ return(')'); }
<C>{space}			{ ECHO; }
<C>\{				{ return('{'); }
<C>\}				{ return('}'); }
<C>\[				{ return('['); }
<C>\]				{ return(']'); }
<C>\=				{ return('='); }
<C>"->"				{ return(S_MEMBER); }
<C>">>"				{ return(S_RSHIFT); }
<C>"<<"				{ return(S_LSHIFT); }
<C>"||"				{ return(S_OR); }
<C>"&&"				{ return(S_AND); }
<C>"++"				{ return(S_INC); }
<C>"--"				{ return(S_DEC); }
<C>"=="				{ return(S_EQUAL); }
<C>"!="				{ return(S_NEQUAL); }
<C>"+="				{ return(S_ADD); }
<C>"-="				{ return(S_SUB); }
<C>"*="				{ return(S_MUL); }
<C>"/="				{ return(S_DIV); }
<C>"%="				{ return(S_MOD); }
<C>"->*"			{ return(S_MEMPOINT); }
<C>".*"				{ return(S_DOTPOINT); }
<C>{other}			{ return S_ANYTHING; }

<C>{exec_sql}{define}{space}*	{ BEGIN(def_ident); }
<C>{informix_special}{define}{space}*	{
						/* are we simulating Informix? */
						if (INFORMIX_MODE)
						{
							BEGIN(def_ident);
						}
						else
						{
							yyless(1);
							return (S_ANYTHING);
						}
					}
<C>{exec_sql}{include}{space}*	{ BEGIN(incl); }
<C>{informix_special}{include}{space}* { 
					  /* are we simulating Informix? */
					  if (INFORMIX_MODE)
					  {
						  BEGIN(incl);
					  }
					  else
					  {
						  yyless(1);
						  return (S_ANYTHING);
					  }
					}
<C,xskip>{exec_sql}{ifdef}{space}*	{ ifcond = TRUE; BEGIN(xcond); }
<C,xskip>{informix_special}{ifdef}{space}* { 
					  /* are we simulating Informix? */
					  if (INFORMIX_MODE)
					  {
						  ifcond = TRUE;
						  BEGIN(xcond);
					  }
					  else
					  {
						  yyless(1);
						  return (S_ANYTHING);
					  }
					}
<C,xskip>{exec_sql}{ifndef}{space}* { ifcond = FALSE; BEGIN(xcond); }
<C,xskip>{informix_special}{ifndef}{space}* { 
					  /* are we simulating Informix? */
					  if (INFORMIX_MODE)
					  {
						  ifcond = FALSE;
						  BEGIN(xcond);
					  }
					  else
					  {
						  yyless(1);
						  return (S_ANYTHING);
					  }
					}
<C,xskip>{exec_sql}{elif}{space}*	{	/* pop stack */
						if ( preproc_tos == 0 ) {
							mmerror(PARSE_ERROR, ET_FATAL, "Missing matching 'EXEC SQL IFDEF / EXEC SQL IFNDEF'");
						}
						else if ( stacked_if_value[preproc_tos].else_branch )
							mmerror(PARSE_ERROR, ET_FATAL, "Missing 'EXEC SQL ENDIF;'");
						else
							preproc_tos--;

						ifcond = TRUE; BEGIN(xcond);
					}
<C,xskip>{informix_special}{elif}{space}* { 
					  /* are we simulating Informix? */
					  if (INFORMIX_MODE)
					  {
						if ( preproc_tos == 0 ) {
							mmerror(PARSE_ERROR, ET_FATAL, "Missing matching 'EXEC SQL IFDEF / EXEC SQL IFNDEF'");
						}
						else if ( stacked_if_value[preproc_tos].else_branch )
							mmerror(PARSE_ERROR, ET_FATAL, "Missing 'EXEC SQL ENDIF;'");
						else
							preproc_tos--;

						ifcond = TRUE; BEGIN(xcond);
					  }
					  else
					  {
						  yyless(1);
						  return (S_ANYTHING);
					  }
					}

<C,xskip>{exec_sql}{else}{space}*";" {	/* only exec sql endif pops the stack, so take care of duplicated 'else' */
						if ( stacked_if_value[preproc_tos].else_branch ) {
							mmerror(PARSE_ERROR, ET_FATAL, "Duplicated 'EXEC SQL ELSE;'");
						}
						else {
							stacked_if_value[preproc_tos].else_branch = TRUE;
							stacked_if_value[preproc_tos].condition =
							(stacked_if_value[preproc_tos-1].condition &&
							 ! stacked_if_value[preproc_tos].condition);

							if ( stacked_if_value[preproc_tos].condition )
								BEGIN(C);
							else
								BEGIN(xskip);
						}
					}
<C,xskip>{informix_special}{else}{space}*	{
					  /* are we simulating Informix? */
					  if (INFORMIX_MODE)
					  {
						if ( stacked_if_value[preproc_tos].else_branch ) {
							mmerror(PARSE_ERROR, ET_FATAL, "Duplicated 'EXEC SQL ELSE;'");
						}
						else {
							stacked_if_value[preproc_tos].else_branch = TRUE;
							stacked_if_value[preproc_tos].condition =
							(stacked_if_value[preproc_tos-1].condition &&
							 ! stacked_if_value[preproc_tos].condition);

							if ( stacked_if_value[preproc_tos].condition )
								BEGIN(C);
							else
								BEGIN(xskip);
						}
					  }
					  else
					  {
						  yyless(1);
						  return (S_ANYTHING);
					  }
					}
<C,xskip>{exec_sql}{endif}{space}*";" {
						if ( preproc_tos == 0 )
							mmerror(PARSE_ERROR, ET_FATAL, "Unmatched 'EXEC SQL ENDIF;'");
						else
							preproc_tos--;

						if ( stacked_if_value[preproc_tos].condition )
						   BEGIN(C);
						else
						   BEGIN(xskip);
					}
<C,xskip>{informix_special}{endif}{space}*";"	{
					  /* are we simulating Informix? */
					  if (INFORMIX_MODE)
					  {
						if ( preproc_tos == 0 )
							mmerror(PARSE_ERROR, ET_FATAL, "Unmatched 'EXEC SQL ENDIF;'");
						else
							preproc_tos--;

						if ( stacked_if_value[preproc_tos].condition )
						   BEGIN(C);
						else
						   BEGIN(xskip);
					  }
					  else
					  {
						  yyless(1);
						  return (S_ANYTHING);
					  }
					}

<xskip>{other}		{ /* ignore */ }

<xcond>{identifier}{space}*";" {
						if ( preproc_tos >= MAX_NESTED_IF-1 ) {
							mmerror(PARSE_ERROR, ET_FATAL, "Too many nested 'EXEC SQL IFDEF' conditions");
						}
						else 
						{
							struct _defines *defptr;
							unsigned int i;

							/* skip the ";" and trailing whitespace. Note that yytext contains
							   at least one non-space character plus the ";" */
							for ( i = strlen(yytext)-2;
								  i > 0 && isspace((unsigned char) yytext[i]);
								  i-- )
							{}
							yytext[i+1] = '\0';

							for ( defptr = defines; defptr != NULL &&
								  ( strcmp(yytext, defptr->old) != 0 ); defptr = defptr->next );

							preproc_tos++;
							stacked_if_value[preproc_tos].else_branch = FALSE;
							stacked_if_value[preproc_tos].condition =
							( (defptr ? ifcond : !ifcond) && stacked_if_value[preproc_tos-1].condition );
						}

						if ( stacked_if_value[preproc_tos].condition )
						   BEGIN C;
						else
						   BEGIN(xskip);
					}

<def_ident>{identifier} {
				old = mm_strdup(yytext);
				BEGIN(def);
				startlit();
			}
<def>{space}*";"	{
						struct _defines *ptr, *this;

						for (ptr = defines; ptr != NULL; ptr = ptr->next)
						{
							 if (strcmp(old, ptr->old) == 0)
							 {
								free(ptr->new);
								/* ptr->new = mm_strdup(scanstr(literalbuf));*/
								ptr->new = mm_strdup(literalbuf);
							 }
						}
						if (ptr == NULL)
						{
												this = (struct _defines *) mm_alloc(sizeof(struct _defines));

												/* initial definition */
												this->old = old;
												this->new = mm_strdup(literalbuf);
							this->next = defines;
							defines = this;
						}

						BEGIN(C);
					}
<def>[^;]			{ addlit(yytext, yyleng); }

<incl>\<[^\>]+\>{space}*";"?		{	parse_include(); }
<incl>{dquote}{xdinside}{dquote}{space}*";"?	{	parse_include(); }
<incl>[^;\<\>\"]+";"		{ parse_include(); }

<<EOF>>				{
				  		if (yy_buffer == NULL) {
				  		if ( preproc_tos > 0 ) 
						{
					  		preproc_tos = 0;
							mmerror(PARSE_ERROR, ET_FATAL, "Missing 'EXEC SQL ENDIF;'");
				  		}
							yyterminate();
							}
				  		else
				  		{
							struct _yy_buffer *yb = yy_buffer;
							int i;

							if (yyin != NULL)
								fclose(yyin);

							yy_delete_buffer( YY_CURRENT_BUFFER );
							yy_switch_to_buffer(yy_buffer->buffer);

							yylineno = yy_buffer->lineno;

							/* We have to output the filename only if we change files here */
							i = strcmp(input_filename, yy_buffer->filename);

							free(input_filename);
							input_filename = yy_buffer->filename;

							yy_buffer = yy_buffer->next;
							free(yb);

							if (i != 0)
								output_line_number();
				  		}
					}
%%
void
lex_init(void)
{
	braces_open = 0;

	preproc_tos = 0;
	yylineno = 1;
	ifcond = TRUE;
	stacked_if_value[preproc_tos].condition = ifcond;
	stacked_if_value[preproc_tos].else_branch = FALSE;

	/* initialize literal buffer to a reasonable but expansible size */
	if (literalbuf == NULL)
	{
		literalalloc = 128;
		literalbuf = (char *) malloc(literalalloc);
	}
	startlit();

	BEGIN C;
}

static void
addlit(char *ytext, int yleng)
{
	/* enlarge buffer if needed */
	if ((literallen+yleng) >= literalalloc)
	{
		do 
			literalalloc *= 2;
		while ((literallen+yleng) >= literalalloc);
		literalbuf = (char *) realloc(literalbuf, literalalloc);
	}
	/* append new data, add trailing null */
	memcpy(literalbuf+literallen, ytext, yleng);
	literallen += yleng;
	literalbuf[literallen] = '\0';
}

static void
addlitchar(unsigned char ychar)
{
	/* enlarge buffer if needed */
        if ((literallen+1) >= literalalloc)
        {
                literalalloc *= 2;
                literalbuf = (char *) realloc(literalbuf, literalalloc);
        }
	/* append new data, add trailing null */
	literalbuf[literallen] = ychar;
	literallen += 1;
	literalbuf[literallen] = '\0';
}

static void
parse_include(void)
{
	/* got the include file name */
	struct _yy_buffer *yb;
  	struct _include_path *ip;
  	char inc_file[MAXPGPATH];
  	unsigned int i;

  	yb = mm_alloc(sizeof(struct _yy_buffer));

  	yb->buffer =	YY_CURRENT_BUFFER;
  	yb->lineno = yylineno;
  	yb->filename = input_filename;
  	yb->next = yy_buffer;

  	yy_buffer = yb;

  	/*
	 * skip the ";" if there is one and trailing whitespace. Note that
	 * yytext contains at least one non-space character plus the ";" 
	 */
  	for ( i = strlen(yytext)-2;
		i > 0 && isspace((unsigned char) yytext[i]);
		i-- )
  		{}

	if (yytext[i] == ';')
		i--;

	yytext[i+1] = '\0';
	
	yyin = NULL;

	/* If file name is enclosed in '"' remove these and look only in '.' */
	/* Informix does look into all include paths though, except filename starts with '/' */
	if ((yytext[0] == '"' && yytext[i] == '"') &&
	    ((compat != ECPG_COMPAT_INFORMIX && compat != ECPG_COMPAT_INFORMIX_SE) || yytext[1] == '/'))
	{
		yytext[i] = '\0';
		memmove(yytext, yytext+1, strlen(yytext));
	
		strncpy(inc_file, yytext, sizeof(inc_file));
		yyin = fopen(inc_file, "r");
		if (!yyin)
		{
			if (strcmp(inc_file + strlen(inc_file) - 2, ".h"))
			{
				strcat(inc_file, ".h");
				yyin = fopen(inc_file, "r");
			}
		}
		
	}
	else
	{
		if ((yytext[0] == '"' && yytext[i] == '"') || (yytext[0] == '<' && yytext[i] == '>'))
		{
			yytext[i] = '\0';
			memmove(yytext, yytext+1, strlen(yytext));
		}
		
	  	for (ip = include_paths; yyin == NULL && ip != NULL; ip = ip->next)
	  	{
			if (strlen(ip->path) + strlen(yytext) + 3 > MAXPGPATH)
			{
				fprintf(stderr, "Error: Path %s/%s is too long in line %d, skipping.\n", ip->path, yytext, yylineno);
				continue;
			}
			snprintf (inc_file, sizeof(inc_file), "%s/%s", ip->path, yytext);
			yyin = fopen(inc_file, "r");
			if (!yyin)
			{
				if (strcmp(inc_file + strlen(inc_file) - 2, ".h"))
				{
					strcat(inc_file, ".h");
					yyin = fopen( inc_file, "r" );
				}
			}
		}
	}
	if (!yyin)
	{
		snprintf(errortext, sizeof(errortext), "Cannot open include file %s in line %d\n", yytext, yylineno);
		mmerror(NO_INCLUDE_FILE, ET_FATAL, errortext);
  	}

	input_filename = mm_strdup(inc_file);
	yy_switch_to_buffer(yy_create_buffer(yyin,YY_BUF_SIZE ));
	yylineno = 1;
	output_line_number();

  	BEGIN C;
}
