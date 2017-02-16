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
 unop                cunop;
 node               *node;
}

%token BRACKET_L BRACKET_R CURLY_L CURLY_R COMMA SEMICOLON
%token MINUS PLUS STAR SLASH PERCENT LE LT GE GT EQ NE OR AND
%token NOT
%token IF ELSE WHILE
%token TRUEVAL FALSEVAL LET

%token <cint> NUM
%token <cflt> FLOAT
%token <id> ID

%type <node> intval floatval boolval constant expr
%type <node> stmts stmt assign if while varlet program
%type <cbinop> binop
%type <cunop> unop

%start program

%%

program: stmts 
         {
           parseresult = $1;
         }
         ;

stmts: stmt stmts
        {
          $$ = TBmakeStmts( $1, $2);
        }
      | stmt
        {
          $$ = TBmakeStmts( $1, NULL);
        }
        ;

stmt:  assign
       {
         $$ = $1;
       }
       | if { $$ = $1; }
       | while { $$ = $1; }
       ;

assign: varlet LET expr SEMICOLON
        {
          $$ = TBmakeAssign( $1, $3);
        }
        ;

if: 	IF BRACKET_L expr BRACKET_R CURLY_L stmts CURLY_R 
		{ 
	   	   $$ = TBmakeIf( $3, $6, NULL );
		}
		| IF BRACKET_L expr BRACKET_R CURLY_L stmts CURLY_R ELSE CURLY_L stmts CURLY_R
		{ 
	   	   $$ = TBmakeIf( $3, $6, $10 );
		}
		;
		
while:  WHILE BRACKET_L expr BRACKET_R CURLY_L stmts CURLY_R
		{
			$$ = TBmakeWhile($3, $6);
		}

varlet: ID
        {
          $$ = TBmakeVarlet( STRcpy( $1));
        }
        ;


expr: BRACKET_L expr BRACKET_R            { $$ = $2; }
    | expr binop expr                     { $$ = TBmakeBinop( $2, $1, $3); }
    | unop expr                           { $$ = TBmakeUnop( $1, $2); }
    | ID                                  { $$ = TBmakeVar( STRcpy( $1)); }
    | constant                            { $$ = $1; }
    ;

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
     | LE        { $$ = BO_le; }
     | LT        { $$ = BO_lt; }
     | GE        { $$ = BO_ge; }
     | GT        { $$ = BO_gt; }
     | EQ        { $$ = BO_eq; }
     | NE        { $$ = BO_ne; }
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

