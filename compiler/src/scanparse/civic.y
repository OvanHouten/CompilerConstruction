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

%token EXTERN EXPORT
%token INT_TYPE FLOAT_TYPE BOOL_TYPE VOID
%token COMMA SEMICOLON
%token TRUEVAL FALSEVAL

%token <cint> NUM
%token <cflt> FLOAT
%token <id> ID

%type <node> intval floatval boolval constant expr declarations declaration fundec
%type <node> stmts stmt assign declare if while do for program

%start program

%%

program: declarations { parseresult = TBmakeProgram( $1); }

declarations: declaration declarations { $$ = TBmakeDeclarations( $1, $2); }
            | declaration              { $$ = TBmakeDeclarations( $1, NULL); }
            ;

declaration: fundec { $$ = $1; }

fundec: INT_TYPE ID BRACKET_L BRACKET_R SEMICOLON { $$ = TBmakeFundec( TBmakeFunheader( TBmakeInt(), TBmakeId($2), NULL)); }

stmts: stmt stmts { $$ = TBmakeStatements( $1, $2); }
     | stmt       { $$ = TBmakeStatements( $1, NULL); }
     ;

 stmt:    declare { $$ = $1; }
        | assign  { $$ = $1; }
		| if      { $$ = $1; }
		| do      { $$ = $1; }
		| while   { $$ = $1; }
		| for     { $$ = $1; }
		;         

declare: INT_TYPE ID SEMICOLON   { $$ = TBmakeVardec( TBmakeInt(), NULL, TBmakeId( $2), NULL); }
       | FLOAT_TYPE ID SEMICOLON { $$ = TBmakeVardec( TBmakeFloat(), NULL, TBmakeId( $2), NULL); }
       | BOOL_TYPE ID SEMICOLON  { $$ = TBmakeVardec( TBmakeBool(), NULL, TBmakeId( $2), NULL); }
       ;

assign: INT_TYPE ID LET expr SEMICOLON   { $$ = TBmakeAssign( TBmakeVardec( TBmakeInt(), NULL, TBmakeId( $2), NULL), $4); }
      | FLOAT_TYPE ID LET expr SEMICOLON { $$ = TBmakeAssign( TBmakeVardec( TBmakeFloat(), NULL, TBmakeId( $2), NULL), $4); }
      | BOOL_TYPE ID LET expr SEMICOLON  { $$ = TBmakeAssign( TBmakeVardec( TBmakeBool(), NULL, TBmakeId( $2), NULL), $4); }
      | ID LET expr SEMICOLON            { $$ = TBmakeAssign( TBmakeId( $1), $3); }
      ;

if:		IF BRACKET_L expr BRACKET_R stmt { $$ = TBmakeIf( $3, $5, NULL ); }
      | IF BRACKET_L expr BRACKET_R CURLY_L stmts CURLY_R { $$ = TBmakeIf( $3, $6, NULL ); }
      | IF BRACKET_L expr BRACKET_R CURLY_L stmts CURLY_R ELSE CURLY_L stmts CURLY_R { $$ = TBmakeIf( $3, $6, $10 ); }
      ;

do:   DO CURLY_L stmts CURLY_R WHILE BRACKET_L expr BRACKET_R SEMICOLON { $$ = TBmakeDo($7, $3); }

while: WHILE BRACKET_L expr BRACKET_R CURLY_L stmts CURLY_R { $$ = TBmakeWhile($3, $6); }

for: FOR BRACKET_L INT_TYPE ID LET expr COMMA expr BRACKET_R stmt { $$ = TBmakeFor( TBmakeId( $4), $6, $8, NULL, $10); }
   | FOR BRACKET_L INT_TYPE ID LET expr COMMA expr BRACKET_R CURLY_L stmts CURLY_R { $$ = TBmakeFor( TBmakeId( $4), $6, $8, NULL, $11); }
   | FOR BRACKET_L INT_TYPE ID LET expr COMMA expr COMMA expr BRACKET_R stmt { $$ = TBmakeFor( TBmakeId( $4), $6, $8, $10, $12); }
   | FOR BRACKET_L INT_TYPE ID LET expr COMMA expr COMMA expr BRACKET_R CURLY_L stmts CURLY_R { $$ = TBmakeFor( TBmakeId( $4), $6, $8, $10, $13); }
   ;

expr: BRACKET_L expr BRACKET_R { $$ = $2; }
    | NOT expr          { $$ = TBmakeUnop( UO_not, $2); }
    | MINUS expr        { $$ = TBmakeUnop( UO_neg, $2); }
    | expr MINUS expr   { $$ = TBmakeArithop( AO_sub, $1, $3); }
    | expr PLUS expr    { $$ = TBmakeArithop( AO_add, $1, $3); }
    | expr STAR expr    { $$ = TBmakeArithop( AO_mul, $1, $3); }
    | expr SLASH expr   { $$ = TBmakeArithop( AO_div, $1, $3); }
    | expr PERCENT expr { $$ = TBmakeArithop( AO_mod, $1, $3); }
    | expr LT expr      { $$ = TBmakeRelop( RO_lt, $1, $3); }
    | expr LE expr      { $$ = TBmakeRelop( RO_le, $1, $3); }
    | expr EQ expr      { $$ = TBmakeRelop( RO_eq, $1, $3); }
    | expr NE expr      { $$ = TBmakeRelop( RO_ne, $1, $3); }
    | expr GE expr      { $$ = TBmakeRelop( RO_ge, $1, $3); }
    | expr GT expr      { $$ = TBmakeRelop( RO_gt, $1, $3); }
    | expr AND expr     { $$ = TBmakeLogicop( LO_and, $1, $3); }
    | expr OR expr      { $$ = TBmakeLogicop( LO_or, $1, $3); }
    | ID                { $$ = TBmakeId( STRcpy( $1)); }
    | constant          { $$ = $1; }
    ;

constant: floatval { $$ = $1; }
        | intval   { $$ = $1; }
        | boolval  { $$ = $1; }
        ;
 
floatval: FLOAT { $$ = TBmakeFloat( $1); }

intval:   NUM { $$ = TBmakeIntconst( $1, TBmakeInt()); }

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

