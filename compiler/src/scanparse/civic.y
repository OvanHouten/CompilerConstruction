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
 binop               cbinop;
 node               *node;
}

%left  LET
%left  OR
%left  AND
%left  EQ NE
%left  LE LT GE GT
%left  PLUS MINUS
%left  STAR SLASH PERCENT
%right NOT
%token IF ELSE WHILE FOR
%right DO
%left  CURLY_L CURLY_R
%left  BRACKET_L BRACKET_R

%token INT_TYPE FLOAT_TYPE BOOL_TYPE
%token COMMA SEMICOLON
%token TRUEVAL FALSEVAL

%token <cint> NUM
%token <cflt> FLOAT
%token <id> ID

%type <node> intval floatval boolval constant expr
%type <node> stmts stmt assign declare if while do for program

%start program

%%

program: stmts { parseresult = TBmakeModule( $1); }

stmts: stmt stmts { $$ = TBmakeStmts( $1, $2); }
     | stmt       { $$ = TBmakeStmts( $1, NULL); }
     ;

 stmt:    declare { $$ = $1; }
        | assign  { $$ = $1; }
		| if      { $$ = $1; }
		| do      { $$ = $1; }
		| while   { $$ = $1; }
		| for     { $$ = $1; }
		;         

declare: INT_TYPE ID SEMICOLON   { $$ = TBmakeVardeclare( STRcpy( $2), TY_int); }
       | FLOAT_TYPE ID SEMICOLON { $$ = TBmakeVardeclare( STRcpy( $2), TY_float); }
       | BOOL_TYPE ID SEMICOLON  { $$ = TBmakeVardeclare( STRcpy( $2), TY_bool); }
       ;

assign: INT_TYPE ID LET expr SEMICOLON   { $$ = TBmakeAssign( TBmakeVardeclare( STRcpy( $2), TY_int), $4); }
      | FLOAT_TYPE ID LET expr SEMICOLON { $$ = TBmakeAssign( TBmakeVardeclare( STRcpy( $2), TY_float), $4); }
      | BOOL_TYPE ID LET expr SEMICOLON  { $$ = TBmakeAssign( TBmakeVardeclare( STRcpy( $2), TY_bool), $4); }
      | ID LET expr SEMICOLON            { $$ = TBmakeAssign( TBmakeVar( $1), $3); }
      ;

if:		IF BRACKET_L expr BRACKET_R stmt { $$ = TBmakeIf( $3, $5, NULL ); }
      | IF BRACKET_L expr BRACKET_R CURLY_L stmts CURLY_R { $$ = TBmakeIf( $3, $6, NULL ); }
      | IF BRACKET_L expr BRACKET_R CURLY_L stmts CURLY_R ELSE CURLY_L stmts CURLY_R { $$ = TBmakeIf( $3, $6, $10 ); }
      ;

do:   DO CURLY_L stmts CURLY_R WHILE BRACKET_L expr BRACKET_R SEMICOLON { $$ = TBmakeDo($7, $3); }

while: WHILE BRACKET_L expr BRACKET_R CURLY_L stmts CURLY_R { $$ = TBmakeWhile($3, $6); }

for: FOR BRACKET_L INT_TYPE ID LET expr COMMA expr BRACKET_R stmt { $$ = TBmakeFor( TBmakeVar( $4), $6, $8, NULL, $10); }
   | FOR BRACKET_L INT_TYPE ID LET expr COMMA expr BRACKET_R CURLY_L stmts CURLY_R { $$ = TBmakeFor( TBmakeVar( $4), $6, $8, NULL, $11); }
   | FOR BRACKET_L INT_TYPE ID LET expr COMMA expr COMMA expr BRACKET_R stmt { $$ = TBmakeFor( TBmakeVar( $4), $6, $8, $10, $12); }
   | FOR BRACKET_L INT_TYPE ID LET expr COMMA expr COMMA expr BRACKET_R CURLY_L stmts CURLY_R { $$ = TBmakeFor( TBmakeVar( $4), $6, $8, $10, $13); }
   ;

expr: BRACKET_L expr BRACKET_R { $$ = $2; }
    | NOT expr          { $$ = TBmakeUnop( UO_not, $2); }
    | MINUS expr        { $$ = TBmakeUnop( UO_neg, $2); }
    | expr MINUS expr   { $$ = TBmakeBinop( BO_sub, $1, $3); }
    | expr PLUS expr    { $$ = TBmakeBinop( BO_add, $1, $3); }
    | expr STAR expr    { $$ = TBmakeBinop( BO_mul, $1, $3); }
    | expr SLASH expr   { $$ = TBmakeBinop( BO_div, $1, $3); }
    | expr PERCENT expr { $$ = TBmakeBinop( BO_mod, $1, $3); }
    | expr LT expr      { $$ = TBmakeBinop( BO_lt, $1, $3); }
    | expr LE expr      { $$ = TBmakeBinop( BO_le, $1, $3); }
    | expr EQ expr      { $$ = TBmakeBinop( BO_eq, $1, $3); }
    | expr NE expr      { $$ = TBmakeBinop( BO_ne, $1, $3); }
    | expr GE expr      { $$ = TBmakeBinop( BO_ge, $1, $3); }
    | expr GT expr      { $$ = TBmakeBinop( BO_gt, $1, $3); }
    | expr AND expr     { $$ = TBmakeBinop( BO_and, $1, $3); }
    | expr OR expr      { $$ = TBmakeBinop( BO_or, $1, $3); }
    | ID                { $$ = TBmakeVar( STRcpy( $1)); }
    | constant          { $$ = $1; }
    ;

constant: floatval { $$ = $1; }
        | intval   { $$ = $1; }
        | boolval  { $$ = $1; }
        ;
 
floatval: FLOAT { $$ = TBmakeFloat( $1); }

intval:   NUM { $$ = TBmakeNum( $1); }

boolval:  TRUEVAL  { $$ = TBmakeBool( TRUE); }
       |  FALSEVAL { $$ = TBmakeBool( FALSE); }
       ;
		
%%

static int yyerror( char *error)
{
  CTIabort( "line %d, col %d\nError parsing source code: %s\n", 
            global.line, global.col, error);

  return( 0);
}

node *YYparseTree( void)
{
  DBUG_ENTER("YYparseTree");

  yyparse();

  DBUG_RETURN( parseresult);
}

