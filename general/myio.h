/***************************************************************************
                          myio.h  -  low level io stuff
                             -------------------
    begin                : Unknown
    copyright            : Unknown
    email                : Unknown
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#include <stdio.h>
#include <stdint.h>

uint16_t igetw(FILE *f);  //Reads a 16 bit word (intel byte ordering)
uint32_t igetl(FILE *f); //Reads a 32 bit long (intel byte ordering)
int iputw(uint16_t w, FILE *f); //Writes a 16 bit int (intel byte ordering)
long iputl(uint32_t l, FILE *f); //Writes a 32 bit long (intel byte ordering)


int mgetw(FILE *f); //Reads a 16 bit int (motorola byte-ordering)
long mgetl(FILE *f); //Reads a 32 bit long (motorola byte-ordering)
int mputw(int w, FILE *f); //Writes a 16 bit int (motorola byte-ordering)
long mputl(long l, FILE *f); //Writes a 32 bit long (motorola byte-ordering)
