%{

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "types.h"
#include "node_basic.h"
#include "str.h"
#include "free.h"
#include "civic.tab.h"
#include "globals.h"
#include "ctinfo.h"


#define FILTER(token) \
  global.col += yyleng;  \
  return( token);

#define LINEBUF_SIZE 256
#define YY_NO_UNPUT
#define YY_NO_INPUT


%}

%p 6000 /* set positions to 6000 */
%e 2000 /* set nodes to 1100 */
%option noyywrap
%option nounput


%%

"if"                       { FILTER( IF); }
"else"                     { FILTER( ELSE); }
"do"                       { FILTER( DO); }
"while"                    { FILTER( WHILE); }
"for"                      { FILTER( FOR); }
"extern"                   { FILTER( EXTERN); }
"export"                   { FILTER( EXPORT); }
"return"                   { FILTER( RETURN); }

"void"                     { FILTER( VOID); }
"int"                      { FILTER( INT_TYPE); }
"float"                    { FILTER( FLOAT_TYPE) }
"bool"                     { FILTER( BOOL_TYPE) }

"("                        { FILTER( BRACKET_L); }
")"                        { FILTER( BRACKET_R); }
"{"                        { FILTER( CURLY_L); }
"}"                        { FILTER( CURLY_R); }
"["                        { FILTER( SQUARE_L); }
"]"                        { FILTER( SQUARE_R); }
","                        { FILTER( COMMA); }
";"                        { FILTER( SEMICOLON); }

"-"                        { FILTER( MINUS); }
"+"                        { FILTER( PLUS);  }
"*"                        { FILTER( STAR);  } 
"/"                        { FILTER( SLASH);  } 
"%"                        { FILTER( PERCENT);  } 

"!"                        { FILTER( NOT);  } 

"<="                       { FILTER( LE);  }
"<"                        { FILTER( LT);  } 
">="                       { FILTER( GE);  }
">"                        { FILTER( GT);  }
"=="                       { FILTER( EQ);  }
"!="                       { FILTER( NE);  }
"&&"                       { FILTER( AND); }
"||"                       { FILTER( OR);  }

"="                        { FILTER( LET);    }

"true"                     { FILTER( TRUEVAL); }
"false"                    { FILTER( FALSEVAL); }

[A-Za-z][A-Za-z0-9_]*      { yylval.id = STRcpy(yytext);
                             FILTER( ID);
                           }

#.*$                         { /* empty */ }
"//".*$                      { /* empty */ }
"/*"([^*]|(\*+[^*/]))*\*+"/" { char* eol = yytext; 
                               while (*eol != '\0') { 
                            	   if (*eol == '\n') {
                            	 	  global.line += 1; 
                            	 	  global.col = 0;
                            	   };
                            	   eol++;
                               }; 
                             }

[0-9]+                     { yylval.cint=STRcpy(yytext);
                             FILTER( NUM);
                           }

[0-9]+\.[0-9]+             { yylval.cflt=STRcpy(yytext);
                             FILTER( FLOAT);
                           }

\n.*                       { global.line += 1;
                             global.col = 0;
                             yyless(1);
                           }

[ \t]                      { global.col += yyleng;
                           } 

