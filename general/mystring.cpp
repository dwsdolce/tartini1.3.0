/***************************************************************************
                          mystring.cpp  -  description
                             -------------------
    begin                : 2004
    copyright            : (C) 2004-2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#include <qglobal.h>
#include "mystring.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef WINDOWS
#include <string.h>
#else
#include <strings.h>
#endif

//returns a pointer to a copy of the string s
//allocating the memory required
char *copy_string(const char *s)
{
    if(!s) return NULL;
    char *t = (char *)malloc(strlen(s) + 1);
    strcpy(t, s);
    return t;
}
