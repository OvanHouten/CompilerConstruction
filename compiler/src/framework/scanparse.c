/* ---------------------------------------------------------------------------
 * 
 * SAC Compiler Construction Framework
 * 
 * ---------------------------------------------------------------------------
 * 
 * SAC COPYRIGHT NOTICE, LICENSE, AND DISCLAIMER
 * 
 * (c) Copyright 1994 - 2011 by
 * 
 *   SAC Development Team
 *   SAC Research Foundation
 * 
 *   http://www.sac-home.org
 *   email:info@sac-home.org
 * 
 *   All rights reserved
 * 
 * ---------------------------------------------------------------------------
 * 
 * The SAC compiler construction framework, all accompanying 
 * software and documentation (in the following named this software)
 * is developed by the SAC Development Team (in the following named
 * the developer) which reserves all rights on this software.
 * 
 * Permission to use this software is hereby granted free of charge
 * exclusively for the duration and purpose of the course 
 *   "Compilers and Operating Systems" 
 * of the MSc programme Grid Computing at the University of Amsterdam.
 * Redistribution of the software or any parts thereof as well as any
 * alteration  of the software or any parts thereof other than those 
 * required to use the compiler construction framework for the purpose
 * of the above mentioned course are not permitted.
 * 
 * The developer disclaims all warranties with regard to this software,
 * including all implied warranties of merchantability and fitness.  In no
 * event shall the developer be liable for any special, indirect or
 * consequential damages or any damages whatsoever resulting from loss of
 * use, data, or profits, whether in an action of contract, negligence, or
 * other tortuous action, arising out of or in connection with the use or
 * performance of this software. The entire risk as to the quality and
 * performance of this software is with you. Should this software prove
 * defective, you assume the cost of all servicing, repair, or correction.
 * 
 * ---------------------------------------------------------------------------
 */ 



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scanparse.h"
#include "dbug.h"
#include "globals.h"
#include "ctinfo.h"
#include "str.h"
#include "memory.h"
#include "myglobals.h"


/*
 * file handle for parsing
 */
FILE *yyin;

/*
 * external parser function from fun.y
 */
extern node *YYparseTree();

// File used to store the result from the preprocessor
char *tempFileName;

// Finds the last occurrence of a separator character in a string.
// If it is not found the original string is returned.
char *findLast(char *text, char sep) {
    char *lastOccurence = text;
    char *occurence = strchr(text, sep);
    while (occurence) {
        lastOccurence = occurence;
        occurence = strchr(occurence + 1, sep);
    }
    return lastOccurence;
}

char *createTempFileName(char *fileName) {
    char *tempFileName = MEMmalloc(STRlen(fileName) + 2);
    strcpy(tempFileName, fileName);
    // Assume we are runnning on a unix like file system!
    char *lastSlash = findLast(tempFileName, '/');
    // Move everything from the last occurrence until the end one position to the right
    // e.g. make room for the 'hidden' file indicator
    memmove((lastSlash + 1), lastSlash, strlen(lastSlash));
    // Place the 'hidden' file indicator
    if (lastSlash != tempFileName) {
        lastSlash++;
    }
    *lastSlash = '.';
    return tempFileName;
}

node *SPdoScanParse( node *syntax_tree)
{
  node *result = NULL;
  char *filename;
  
  DBUG_ENTER("SPdoScanParse");

  DBUG_ASSERT( syntax_tree == NULL, 
               "SPdoScanParse() called with existing syntax tree.");
  
  if (myglobal.preprocessor_enabled) {
    filename = tempFileName;
  }
  else {
    filename = STRcpy( global.infile);
  }
  
  yyin = fopen( filename, "r");
  
  if (yyin == NULL) {
    CTIabort( "Cannot open file '%s'.", filename);
  }

  result = YYparseTree();


  if (myglobal.preprocessor_enabled && myglobal.remove_preprocessor_file) {
      remove(tempFileName);
  }

  MEMfree(filename);

  DBUG_RETURN( result);
}


node *SPdoRunPreProcessor( node *syntax_tree)
{
  int  err;
  char *cppcallstr;
  
  DBUG_ENTER("SPdoRunPreProcessor");

  if (myglobal.preprocessor_enabled) {
      DBUG_PRINT("SP", ("Enabling then pre-processor."));
      if (myglobal.includedir) {
          printf("Using '%s' as include folder.\n", myglobal.includedir);
          setenv("C_INCLUDE_PATH", myglobal.includedir, 1);
      }

      // Mac OS-X does not like (e.g. supports) the 'popen' function properly so we create a 'hidden' version of the file.
      tempFileName = createTempFileName(global.infile);
      cppcallstr = STRcatn( 4, "gcc -E -x c ", global.infile, " > ", tempFileName);

      err = system( cppcallstr);

      cppcallstr = MEMfree( cppcallstr);

      if (err) {
        CTIabort( "Unable to run C preprocessor, did you use the '-I' switch correctly?");
      }

  } else {
      DBUG_PRINT("SP", ("Pre-processor is disabled."));
  }
  
  DBUG_RETURN( syntax_tree);
}

