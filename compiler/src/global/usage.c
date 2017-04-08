
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "dbug.h"
#include "usage.h"
#include "phase_options.h"


static
void PrintGeneralInfo(void)
{
  DBUG_ENTER("PrintGeneralInfo");
  
  printf( "\n\n"
          "      civicc  --  Programming C,  the civilised way\n"
          "    ------------------------------------------------\n");
          
  printf( "\n\nSPECIAL OPTIONS:\n\n"
          
          "    -h              Display this helptext.\n");
  
  printf( "\n\nGENERAL OPTIONS:\n\n"
          
          "    <filename>      Name of program file to compile.\n\n"
          "    -kppf           Keep the pre-processed file. (The pre-processor uses a hidden file for storing its output.\n"
          "                    This file is removed by default, but this flag can be used to disable this removal.)\n\n"
          "    -I <dirame>     Name of the directory containing the CiviC system header files.\n"
          "                    The compiler uses the environment variable 'C_INCLUDE_PATH' or '.' if it is not set.\n\n"
          "    -o <filename>   Name of output file.\n\n"
          "    -noopt          Disable the optimisations.\n\n"
          "    -eap            Enable printing of the AST in C-style.\n\n"
          "    -dstp           Disable printing of the symbol table information.\n\n"
          "    -pvd            Prints variable details from the symbol table at variable usage.\n\n"
          "    -tc             Apply syntax tree consistency checks.\n\n"
          "    -#d,<id>        Print debugging information for tag <id>.\n"
          "                    Supported tags are:\n\n"
          
          "                    MAKE - prints debug information of tree constructors.\n"
          "                    SA   - prints debug information of scope analysis.\n"
          "                    SV   - prints debug information of Split VarDef transformation.\n"
          "                    GI   - prints debug information of Global Init transformation.\n"
          "                    OP   - prints debug information of optimisations.\n"
          "                    UTIL - prints debug information of miscellaneous utils.\n"
          "                    FREE - prints debug information of tree destructors.\n",
          global.verbosity);

  DBUG_VOID_RETURN;
}

static
void PrintAuthorInfo(void)
{
  DBUG_ENTER("PrintAuthorInfo");
  
  printf( "\n\nAUTHORS:\n\n"

          "SAC Development Team for the Compiler Framework.\n\n"
          "Nico Tromp, Olaf van Houten for the CiviC compiler.\n\n");
  

  DBUG_VOID_RETURN;
}

static
void PrintBreakOptions( void)
{
  DBUG_ENTER("PrintBreakOptions");
  
  printf( "\n\nBREAK OPTIONS:\n\n"

          "    Break options allow you to stop the compilation process\n"
          "    after a particular phase, subphase or cycle optimisation.\n"
          "    By default the intermediate programm will be printed, \n"
          "    but this behaviour may be influenced by the following\n"
          "    compiler options:\n"
          "\n"
          "    -b<spec>        Break after the compilation stage given\n"
          "                    by <spec>, where <spec> follows the pattern\n"
          "                    <phase>:<subphase>:<cyclephase>:<pass>.\n"
          "                    The first three are from the list of\n"
          "                    encodings below. The last one is a natural\n"
          "                    number. Alternatively, a number can be used\n"
          "                    for the phase, as well.\n"
          );

  DBUG_VOID_RETURN;
}


static
void PrintBreakoptionSpecifier( void)
{
  DBUG_ENTER("PrintBreakoptionSpecifier");
  
  printf( "\n\nBREAK OPTION SPECIFIERS:\n");

  PHOprintPhases();

  DBUG_VOID_RETURN;
}


void USGprintUsage()
{
  DBUG_ENTER("USGprintUsage");

  PrintGeneralInfo();
  PrintBreakOptions();
  PrintBreakoptionSpecifier();
  PrintAuthorInfo();
  
  DBUG_VOID_RETURN;
}
