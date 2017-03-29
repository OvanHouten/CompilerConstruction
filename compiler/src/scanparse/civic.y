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
#include "numbers.h"

static node *parseresult = NULL;
extern int yylex();
static int yyerror( char *errname);

%}

%union {
 nodetype            nodetype;
 char               *id;
 char               *cint;
 char               *cflt;
 type                type;
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

%token IF
%nonassoc THEN
%nonassoc ELSE
%token DO WHILE FOR

%token EXTERN EXPORT RETURN
%token INT_TYPE FLOAT_TYPE BOOL_TYPE VOID
%token COMMA SEMICOLON CURLY_L CURLY_R
%token TRUEVAL FALSEVAL
%left  BRACKET_L BRACKET_R

%token <cint> NUM
%token <cflt> FLOAT
%token <id> ID

%type <type> type

%type <node> program declarations declaration globaldec globaldef fundec fundef
%type <node> funheader params param funbody vardecs vardec block stmts stmt exprs expr
%type <node> localfundefs
%type <node> assign if while do for typecast return funcall
%type <node> constant floatval intval boolval

%start program

%%

program: declarations { parseresult = TBmakeProgram( $1, TBmakeSymboltable(NULL)); }

declarations: declarations declaration { $$ = TBmakeDeclarations( $2, $1); }
            | declaration              { $$ = TBmakeDeclarations( $1, NULL); }
            ;

declaration: globaldec { $$ = $1; }
           | globaldef { $$ = $1; }
           | fundec    { $$ = $1; }
           | fundef    { $$ = $1; }
           ;
           
globaldec: EXTERN type ID SEMICOLON { $$ = TBmakeVardef( TRUE, FALSE, $3, $2, NULL, NULL, NULL); }

globaldef: type ID SEMICOLON                 { $$ = TBmakeVardef( FALSE, FALSE, $2, $1, NULL, NULL, NULL); }
         | type ID LET expr SEMICOLON        { $$ = TBmakeVardef( FALSE, FALSE, $2, $1, $4, NULL, NULL); }
         | EXPORT type ID SEMICOLON          { $$ = TBmakeVardef( FALSE, TRUE, $3, $2, NULL, NULL, NULL); }
         | EXPORT type ID LET expr SEMICOLON { $$ = TBmakeVardef( FALSE, TRUE, $3, $2, $5, NULL, NULL); }
         ;

fundec: EXTERN funheader SEMICOLON { $$ = TBmakeFundef(TRUE, FALSE, $2, NULL, NULL); }

fundef: funheader funbody                 { $$ = TBmakeFundef( FALSE, FALSE, $1, $2, NULL); }
      | EXPORT funheader funbody          { $$ = TBmakeFundef( FALSE, TRUE, $2, $3, NULL); }
      ;

funheader: type ID BRACKET_L BRACKET_R        { $$ = TBmakeFunheader( $1, $2, NULL); }
         | type ID BRACKET_L params BRACKET_R { $$ = TBmakeFunheader( $1, $2, $4); }
         ;
         
params: params COMMA param { $$ = TBmakeParams( $3, $1); }
      | param              { $$ = TBmakeParams( $1, NULL); }
      ;
      
param: type ID { $$ = TBmakeVardef( FALSE, FALSE, $2, $1, NULL, NULL, NULL); }

funbody: CURLY_L vardecs localfundefs stmts CURLY_R { $$ = TBmakeFunbody($2, $3, $4); }
       | CURLY_L vardecs localfundefs CURLY_R       { $$ = TBmakeFunbody($2, $3, NULL); }
       | CURLY_L vardecs stmts CURLY_R              { $$ = TBmakeFunbody($2, NULL, $3); } 
       | CURLY_L localfundefs stmts CURLY_R         { $$ = TBmakeFunbody(NULL, $2, $3); } 
       | CURLY_L vardecs CURLY_R                    { $$ = TBmakeFunbody($2, NULL, NULL); } 
       | CURLY_L localfundefs CURLY_R               { $$ = TBmakeFunbody(NULL, $2, NULL); } 
       | CURLY_L stmts CURLY_R                      { $$ = TBmakeFunbody(NULL, NULL, $2); }
       | CURLY_L CURLY_R                            { $$ = TBmakeFunbody(NULL, NULL, NULL); }
       ;

vardecs: vardecs vardec { $$ = TBmakeVardecs( $2, $1); }
       | vardec         { $$ = TBmakeVardecs( $1, NULL); }
       ;
       
vardec: type ID SEMICOLON            { $$ = TBmakeVardef( FALSE, FALSE, $2, $1, NULL, NULL, NULL); }
      | type ID LET expr SEMICOLON   { $$ = TBmakeVardef( FALSE, FALSE, $2, $1, $4, NULL, NULL); }
      ;

localfundefs: localfundefs fundef { $$ = TBmakeLocalfundefs( $2, $1); }
            | fundef              { $$ = TBmakeLocalfundefs( $1, NULL); }
            ;
       
block: CURLY_L stmts CURLY_R { $$ = $2; }
     | CURLY_L CURLY_R       { $$ = NULL; }
     | stmt                  { $$ = TBmakeStatements($1, NULL); }
     ;

stmts: stmts stmt { $$ = TBmakeStatements( $2, $1); }
     | stmt       { $$ = TBmakeStatements( $1, NULL); }
     ;
     
stmt: assign            { $$ = $1; }  
    | if                { $$ = $1; }
    | do                { $$ = $1; }
    | while             { $$ = $1; }
    | for               { $$ = $1; }
    | return            { $$ = $1; }
    | funcall SEMICOLON { $$ = $1; FUNCALL_PROCEDURECALL($$) = TRUE;}
    ;         

assign: ID LET expr SEMICOLON { $$ = TBmakeAssign( TBmakeId( $1), $3); }

if: IF BRACKET_L expr BRACKET_R block %prec THEN { $$ = TBmakeIf( $3, $5, NULL ); }
  | IF BRACKET_L expr BRACKET_R block ELSE block { $$ = TBmakeIf( $3, $5, $7 ); }
  ;

do: DO block WHILE BRACKET_L expr BRACKET_R SEMICOLON { $$ = TBmakeDo($5, $2); }

while: WHILE BRACKET_L expr BRACKET_R block { $$ = TBmakeWhile($3, $5); }

for: FOR BRACKET_L INT_TYPE ID LET expr COMMA expr BRACKET_R block             { $$ = TBmakeFor( TBmakeVardef( FALSE, FALSE, $4, TY_int, $6, NULL, NULL), $8, TBmakeIntconst(TY_int, 1), $10); }
   | FOR BRACKET_L INT_TYPE ID LET expr COMMA expr COMMA expr BRACKET_R block  { $$ = TBmakeFor( TBmakeVardef( FALSE, FALSE, $4, TY_int, $6, NULL, NULL), $8, $10, $12); }
   ;
   
funcall: ID BRACKET_L BRACKET_R       { $$ = TBmakeFuncall( $1, NULL); }
       | ID BRACKET_L exprs BRACKET_R { $$ = TBmakeFuncall( $1, $3); }

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
    | expr MINUS expr          { $$ = TBmakeBinop( BO_sub, $1, $3); }
    | expr PLUS expr           { $$ = TBmakeBinop( BO_add, $1, $3); }
    | expr STAR expr           { $$ = TBmakeBinop( BO_mul, $1, $3); }
    | expr SLASH expr          { $$ = TBmakeBinop( BO_div, $1, $3); }
    | expr PERCENT expr        { $$ = TBmakeBinop( BO_mod, $1, $3); }
    | expr LT expr             { $$ = TBmakeBinop( BO_lt, $1, $3); }
    | expr LE expr             { $$ = TBmakeBinop( BO_le, $1, $3); }
    | expr EQ expr             { $$ = TBmakeBinop( BO_eq, $1, $3); }
    | expr NE expr             { $$ = TBmakeBinop( BO_ne, $1, $3); }
    | expr GE expr             { $$ = TBmakeBinop( BO_ge, $1, $3); }
    | expr GT expr             { $$ = TBmakeBinop( BO_gt, $1, $3); }
    | expr AND expr            { $$ = TBmakeBinop( BO_and, $1, $3); }
    | expr OR expr             { $$ = TBmakeBinop( BO_or, $1, $3); }
    | ID                       { $$ = TBmakeId( $1); }
    | constant                 { $$ = $1; }
    ;
    
constant: floatval { $$ = $1; }
        | intval   { $$ = $1; }
        | boolval  { $$ = $1; }
        ;
 
floatval: FLOAT { float* value = strToFloat($1);
                  MEMfree($1);
                  if (value) {
                      $$ = TBmakeFloatconst( TY_float, *value);
                      free(value);
                  } else {
                      yyerror("Float value out of range.");
                  }
                }

intval:   NUM { int* value = strToInt($1);
                MEMfree($1);
                if (value) {
                    $$ = TBmakeIntconst( TY_int, *value);
                    free(value);
                } else {
                    yyerror("Integer value out of range.");
                }
              }

boolval:  TRUEVAL  { $$ = TBmakeBoolconst( TY_bool, TRUE); }
       |  FALSEVAL { $$ = TBmakeBoolconst( TY_bool, FALSE); }
       ;

type: INT_TYPE   { $$ = TY_int; }
    | FLOAT_TYPE { $$ = TY_float; }
    | BOOL_TYPE  { $$ = TY_bool; }
    | VOID       { $$ = TY_void; }
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

