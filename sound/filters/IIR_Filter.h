/***************************************************************************
                          IIR_Filter.h  -  description
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
#ifndef IIR_FILTER_H
#define IIR_FILTER_H

#include "Filter.h"

class FilterState;

/** Infinite Impulse Response filter
  */
class IIR_Filter : public Filter
{
  Array1d<double> bufx, bufy; //tempery buffer storage
  Array1d<double> _a, _b; //The filter coefficient's
  Array1d<double> _x, _y; //The current filter state (last n states of input and output)

public:

  IIR_Filter() { }
  IIR_Filter(double *b, double *a, size_t n, size_t m);
  IIR_Filter(double* b, double* a, size_t n);
  ~IIR_Filter() { }
  void init(double *b, double *a, size_t n, size_t m);
  void init(double* b, double* a, size_t n) { init(b, a, n, n); };
  void print();
  void filter(const float *input, float *output, size_t n);
  void reset();
  void getState(FilterState *filterState) const;
  void setState(const FilterState *filterState);
};

class FilterState
{
public:
  Array1d<double> _x, _y;
};

#endif
