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


#include "tree_basic.h"
#include "memory.h"
#include "free.h"
#include "dbug.h"
#include "str.h"


/** <!--******************************************************************-->
 *
 * @fn FREEattribString
 *
 * @brief Frees String attribute
 *
 * @param attr String node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
char *
FREEattribString (char *attr, node * parent)
{
  DBUG_ENTER ("FREEattribString");

  if (attr != NULL) {
    DBUG_PRINT ("FREE", ("Freeing string '%s' at " F_PTR, attr, attr));
    attr = MEMfree (attr);
  }

  DBUG_RETURN (attr);
}


/** <!--******************************************************************-->
 *
 * @fn FREEattribLink
 *
 * @brief Frees String attribute
 *
 * @param attr String node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node *
FREEattribLink (node *attr, node * parent)
{
  DBUG_ENTER ("FREEattribLink");

  DBUG_RETURN (NULL);
}

/********************************************************************
 *
 * @fn FREEattribLink
 *
 * @brief Frees String attribute
 *
 * @param attr String node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
void    *FREEattribVoid(void *attr, node * parent)
{
  DBUG_ENTER ("FREEattribVoid");

  DBUG_RETURN (NULL);
}

