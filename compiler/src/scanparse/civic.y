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
%token COMMA SEMICOLON CURLY_L CURLY_R
%token TRUEVAL FALSEVAL

%token <cint> NUM
%token <cflt> FLOAT
%token <id> ID

%type <node> program declarations declaration funheader type params param funbody statements statement expr 
%type <node> constant floatval intval boolval

%start program

%%

program: declarations { parseresult = TBmakeProgram( $1); }

declarations: declarations declaration { $$ = TBmakeDeclarations( $2, $1); }
            | declaration              { $$ = TBmakeDeclarations( $1, NULL); }
            ;

declaration: EXTERN type ID SEMICOLON          { $$ = TBmakeGlobaldec( $2, NULL, TBmakeId($3)); }
           | type ID SEMICOLON                 { $$ = TBmakeGlobalvardef( FALSE, $1, TBmakeId($2), NULL); }
           | type ID LET expr SEMICOLON        { $$ = TBmakeGlobalvardef( FALSE, $1, TBmakeId($2), $4); }
           | EXPORT type ID SEMICOLON          { $$ = TBmakeGlobalvardef( TRUE, $2, TBmakeId($3), NULL); }
           | EXPORT type ID LET expr SEMICOLON { $$ = TBmakeGlobalvardef( TRUE, $2, TBmakeId($3), $5); }
           | EXTERN funheader SEMICOLON        { $$ = TBmakeFundec( $2); }
           | funheader funbody                 { $$ = TBmakeFundef( FALSE, $1, $2); }
           | EXPORT funheader funbody          { $$ = TBmakeFundef( TRUE, $2, $3); }
           ;

funheader: type ID BRACKET_L BRACKET_R        { $$ = TBmakeFunheader( $1, TBmakeId($2), NULL); }
         | type ID BRACKET_L params BRACKET_R { $$ = TBmakeFunheader( $1, TBmakeId($2), $4); }
         ;
         
params: params COMMA param { $$ = TBmakeParams( $3, $1); }
      | param              { $$ = TBmakeParams( $1, NULL); }
      ;
      
param: type ID { $$ = TBmakeParam( $1, NULL, TBmakeId($2)); }

funbody: CURLY_L CURLY_R            { $$ = NULL; }
       | CURLY_L statements CURLY_R { $$ = $2; }

statements: statements statement { $$ = TBmakeStatements( $2, $1); }
          | statement            { $$ = TBmakeStatements( $1, NULL); }
          ;

statement: expr SEMICOLON { $$ = $1; }

expr: BRACKET_L expr BRACKET_R { $$ = $2; } 
    | constant                 { $$ = $1; }
    | ID                       { $$ = TBmakeId($1); }
    
constant: floatval { $$ = $1; }
        | intval   { $$ = $1; }
        | boolval  { $$ = $1; }
        ;
 
floatval: FLOAT { $$ = TBmakeFloatconst( $1, TBmakeFloat()); }

intval:   NUM { $$ = TBmakeIntconst( $1, TBmakeInt()); }

boolval:  TRUEVAL  { $$ = TBmakeBoolconst( TRUE, TBmakeBool()); }
       |  FALSEVAL { $$ = TBmakeBoolconst( FALSE, TBmakeBool()); }
       ;

type: INT_TYPE   { $$ = TBmakeInt(); }
    | FLOAT_TYPE { $$ = TBmakeFloat(); }
    | BOOL_TYPE  { $$ = TBmakeBool(); }
    | VOID       { $$ = TBmakeVoid(); }
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

