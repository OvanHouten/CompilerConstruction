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
 unop               cunop;
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
%left  BRACKET_R  
%left  BRACKET_L  

%token COMMA SEMICOLON
%token TRUEVAL FALSEVAL

%token <cint> NUM
%token <cflt> FLOAT
%token <id> ID

%type <node> intval floatval boolval constant expr unexpr binexpr
%type <node> stmts stmt assign varlet program
%type <cbinop> binop
%type <cunop> unop

%start program

%%

program: stmts 
         {
           parseresult = $1;
         }
         ;

stmts: stmts stmt
        {
          $$ = TBmakeStmts( $1, $2);
        }
      | stmt
        {
          $$ = TBmakeStmts( $1, NULL);
        }
        ;

stmt: assign
       {
         $$ = $1;
       }
       ;

assign: varlet LET expr SEMICOLON
        {
          $$ = TBmakeAssign( $1, $3);
        }
        ;

varlet: ID
        {
          $$ = TBmakeVarlet( STRcpy( $1));
        }
        ;


expr: unexpr
    | binexpr
    | ID
      {
        $$ = TBmakeVar( STRcpy( $1));
      }
    | constant
    {
      $$ = $1;
    }
    ;

unexpr: unop expr
      	  {
            $$ = TBmakeUnop( $1, $2);
    	  }


binexpr: expr binop expr
          {
            $$ = TBmakeBinop( $2, $1, $3);
          }

constant: floatval
          {
            $$ = $1;
          }
        | intval
          {
            $$ = $1;
          }
        | boolval
          {
            $$ = $1;
          }
        ;

floatval: FLOAT
           {
             $$ = TBmakeFloat( $1);
           }
         ;

intval: NUM
        {
          $$ = TBmakeNum( $1);
        }
      ;

boolval: TRUEVAL
         {
           $$ = TBmakeBool( TRUE);
         }
       | FALSEVAL
         {
           $$ = TBmakeBool( FALSE);
         }
       ;

 unop: NOT       { $$ = UO_not; }
	 | MINUS     { $$ = UO_neg; }
	 ;
	
binop: PLUS      { $$ = BO_add; }
     | MINUS     { $$ = BO_sub; }
     | STAR      { $$ = BO_mul; }
     | SLASH     { $$ = BO_div; }
     | PERCENT   { $$ = BO_mod; }
     | NE        { $$ = BO_ne; }
     | LE        { $$ = BO_le; }
     | LT        { $$ = BO_lt; }
     | GE        { $$ = BO_ge; }
     | GT        { $$ = BO_gt; }
     | EQ        { $$ = BO_eq; }
     | OR        { $$ = BO_or; }
     | AND       { $$ = BO_and; }
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

