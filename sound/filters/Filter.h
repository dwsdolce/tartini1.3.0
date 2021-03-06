/***************************************************************************
                          Filter.h  -  description
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
#ifndef FILTER_H
#define FILTER_H

#include "array1d.h"

typedef unsigned int uint;

class Filter
{
public:
  virtual void filter(const float *input, float *output, size_t n) = 0;
  virtual void reset() = 0;
  virtual size_t delay() { return 0; }
  virtual ~Filter() { }
};

#endif
