%{

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#include "types.h"
#include "tree_basic.h"
#include "str.h"
#include "dbug.h"
#include "ctinfo.h"
#include "free.h"
#include "globals.h"

static node *parseresult = NULL;
extern int yylex();
static int yyerror( char *errname);

%}

%union {
 nodetype            nodetype;
 char               *id;
 int                 cint;
 float               cflt;
 arithop             carithop;
 logicop             clogicop;
 relop               crelop;
 node               *node;
}

%right  LET
%left  OR
%left  AND
%left  EQ NE
%left  LE LT GE GT
%left  PLUS MINUS
%left  STAR SLASH PERCENT
%right NOT
%token IF ELSE DO WHILE FOR
%left  CURLY_L CURLY_R
%left  BRACKET_L BRACKET_R

%token EXTERN EXPORT RETURN
%token INT_TYPE FLOAT_TYPE BOOL_TYPE VOID
%token COMMA SEMICOLON
%token TRUEVAL FALSEVAL

%token <cint> NUM
%token <cflt> FLOAT
%token <id> ID

%type <node> program declarations declaration globaldec basictype

%start program

%%

program: declarations { parseresult = TBmakeProgram( $1); }

declarations: declaration declarations { $$ = TBmakeDeclarations( $1, $2); }
            | declaration              { $$ = TBmakeDeclarations( $1, NULL); }
            ;

declaration: globaldec { $$ = $1; }
           
globaldec: EXTERN basictype ID SEMICOLON   { $$ = TBmakeGlobaldec( $2, NULL, TBmakeId($3)); }

basictype: INT_TYPE   { $$ = TBmakeInt(); }
         | FLOAT_TYPE { $$ = TBmakeFloat(); }
         | BOOL_TYPE  { $$ = TBmakeBool(); }
         ;
		
%%

static int yyerror( char *error)
{
  CTIabort( "line %d, col %d\nError parsing source code: %s\n", 
            global.line + 1, global.col, error);

  return( 0);
}

node *YYparseTree( void)
{
  DBUG_ENTER("YYparseTree");

  yyparse();

  DBUG_RETURN( parseresult);
}

