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

%type <node> program declarations declaration globaldec globaldef fundec fundef 
%type <node> funheader params param funbody vardecs vardec stmts stmt exprs expr
%type <node> assign if while do for typecast return funcall
%type <node> type constant floatval intval boolval

%start program

%%

program: declarations { parseresult = TBmakeProgram( $1); }

declarations: declarations declaration { $$ = TBmakeDeclarations( $2, $1); }
            | declaration              { $$ = TBmakeDeclarations( $1, NULL); }
            ;

declaration: globaldec { $$ = $1; }
           | globaldef { $$ = $1; }
           | fundec    { $$ = $1; }
           | fundef    { $$ = $1; }
           ;
           
globaldec: EXTERN type ID SEMICOLON { $$ = TBmakeGlobaldec( $2, NULL, TBmakeId($3)); }

globaldef: type ID SEMICOLON                 { $$ = TBmakeGlobalvardef( FALSE, $1, TBmakeId($2), NULL); }
         | type ID LET expr SEMICOLON        { $$ = TBmakeGlobalvardef( FALSE, $1, TBmakeId($2), $4); }
         | EXPORT type ID SEMICOLON          { $$ = TBmakeGlobalvardef( TRUE, $2, TBmakeId($3), NULL); }
         | EXPORT type ID LET expr SEMICOLON { $$ = TBmakeGlobalvardef( TRUE, $2, TBmakeId($3), $5); }
         ;

fundec: EXTERN funheader SEMICOLON { $$ = TBmakeFundec( $2); }

fundef: funheader funbody                 { $$ = TBmakeFundef( FALSE, $1, $2); }
      | EXPORT funheader funbody          { $$ = TBmakeFundef( TRUE, $2, $3); }
      ;

funheader: type ID BRACKET_L BRACKET_R        { $$ = TBmakeFunheader( $1, TBmakeId($2), NULL); }
         | type ID BRACKET_L params BRACKET_R { $$ = TBmakeFunheader( $1, TBmakeId($2), $4); }
         ;
         
params: params COMMA param { $$ = TBmakeParams( $3, $1); }
      | param              { $$ = TBmakeParams( $1, NULL); }
      ;
      
param: type ID { $$ = TBmakeParam( $1, NULL, TBmakeId($2)); }

funbody: CURLY_L vardecs stmts CURLY_R { $$ = TBmakeFunbody($2, NULL, $3); } 
       | CURLY_L vardecs CURLY_R       { $$ = TBmakeFunbody($2, NULL, NULL); } 
       | CURLY_L stmts CURLY_R         { $$ = TBmakeFunbody(NULL, NULL, $2); }
       | CURLY_L CURLY_R               { $$ = TBmakeFunbody(NULL, NULL, NULL); }
       ;

vardecs: vardecs vardec { $$ = TBmakeVardecs( $2, $1); }
       | vardec         { $$ = TBmakeVardecs( $1, NULL); }
       ;
       
vardec: type ID SEMICOLON            { $$ = TBmakeVardec( $1, NULL, TBmakeId( $2), NULL); }
      | type ID LET expr SEMICOLON   { $$ = TBmakeVardec( $1, NULL, TBmakeId( $2), $4); }
      ;

stmts: stmts stmt { $$ = TBmakeStatements( $2, $1); }
     | stmt       { $$ = TBmakeStatements( $1, NULL); }
     ;

stmt: assign  { $$ = $1; }  
    | if      { $$ = $1; }
    | do      { $$ = $1; }
    | while   { $$ = $1; }
    | for     { $$ = $1; }
    | return  { $$ = $1; }
    | funcall SEMICOLON { $$ = $1; }
    ;         

assign: ID LET expr SEMICOLON { $$ = TBmakeAssign( TBmakeId( $1), $3); }

if: IF BRACKET_L expr BRACKET_R stmt                                             { $$ = TBmakeIf( $3, $5, NULL ); }
  | IF BRACKET_L expr BRACKET_R CURLY_L stmts CURLY_R                            { $$ = TBmakeIf( $3, $6, NULL ); }
  | IF BRACKET_L expr BRACKET_R CURLY_L stmts CURLY_R ELSE CURLY_L stmts CURLY_R { $$ = TBmakeIf( $3, $6, $10 ); }
  ;

do:    DO CURLY_L stmts CURLY_R WHILE BRACKET_L expr BRACKET_R SEMICOLON { $$ = TBmakeDo($7, $3); }

while: WHILE BRACKET_L expr BRACKET_R CURLY_L stmts CURLY_R              { $$ = TBmakeWhile($3, $6); }

for: FOR BRACKET_L INT_TYPE ID LET expr COMMA expr BRACKET_R stmt                             { $$ = TBmakeFor( TBmakeId( $4), $6, $8, NULL, $10); }
   | FOR BRACKET_L INT_TYPE ID LET expr COMMA expr BRACKET_R CURLY_L stmts CURLY_R            { $$ = TBmakeFor( TBmakeId( $4), $6, $8, NULL, $11); }
   | FOR BRACKET_L INT_TYPE ID LET expr COMMA expr COMMA expr BRACKET_R stmt                  { $$ = TBmakeFor( TBmakeId( $4), $6, $8, $10, $12); }
   | FOR BRACKET_L INT_TYPE ID LET expr COMMA expr COMMA expr BRACKET_R CURLY_L stmts CURLY_R { $$ = TBmakeFor( TBmakeId( $4), $6, $8, $10, $13); }
   ;
   
funcall: ID BRACKET_L BRACKET_R       { $$ = TBmakeFuncall( TBmakeId($1), NULL); }
       | ID BRACKET_L exprs BRACKET_R { $$ = TBmakeFuncall( TBmakeId($1), $3); }

return: RETURN SEMICOLON      { $$ = TBmakeReturn(NULL); }
      | RETURN expr SEMICOLON { $$ = TBmakeReturn( $2); }
      
typecast: BRACKET_L type BRACKET_R expr { $$ = TBmakeTypecast( $2, $4); }

exprs: exprs COMMA expr { $$ = TBmakeExprs( $3, $1); }
     | expr             { $$ = TBmakeExprs( $1, NULL); }

expr: BRACKET_L expr BRACKET_R { $$ = $2; }
    | funcall                  { $$ = $1; }
    | typecast                 { $$ = $1; }
    | NOT expr                 { $$ = TBmakeUnop( UO_not, $2); }
    | MINUS expr               { $$ = TBmakeUnop( UO_neg, $2); }
    | expr MINUS expr          { $$ = TBmakeArithop( AO_sub, $1, $3); }
    | expr PLUS expr           { $$ = TBmakeArithop( AO_add, $1, $3); }
    | expr STAR expr           { $$ = TBmakeArithop( AO_mul, $1, $3); }
    | expr SLASH expr          { $$ = TBmakeArithop( AO_div, $1, $3); }
    | expr PERCENT expr        { $$ = TBmakeArithop( AO_mod, $1, $3); }
    | expr LT expr             { $$ = TBmakeRelop( RO_lt, $1, $3); }
    | expr LE expr             { $$ = TBmakeRelop( RO_le, $1, $3); }
    | expr EQ expr             { $$ = TBmakeRelop( RO_eq, $1, $3); }
    | expr NE expr             { $$ = TBmakeRelop( RO_ne, $1, $3); }
    | expr GE expr             { $$ = TBmakeRelop( RO_ge, $1, $3); }
    | expr GT expr             { $$ = TBmakeRelop( RO_gt, $1, $3); }
    | expr AND expr            { $$ = TBmakeLogicop( LO_and, $1, $3); }
    | expr OR expr             { $$ = TBmakeLogicop( LO_or, $1, $3); }
    | ID                       { $$ = TBmakeId( $1); }
    | constant                 { $$ = $1; }
    ;
    
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

