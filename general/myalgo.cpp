/***************************************************************************
                          myalgo.cpp  -  Generic useful functions
                             -------------------
    begin                : 2006
    copyright            : (C) 2006 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/

#include "myalgo.h"

/** Decimates input into the output, using a fast pick every n'th value idea.
*/
void bresenham1d(const Array1d<float> &input, Array1d<float> &output)
{
    //Use Bresenham's algorithm in 1d to choose the points to draw
    size_t w = output.size();
    size_t intStep = size_t(input.size() / w);
    size_t remainderStep = input.size() - (intStep * w);
    size_t pos = 0;
    size_t remainder = 0;
    for(int j=0; j<w; j++, pos+=intStep, remainder+=remainderStep) {
      if(remainder >= w) {
        pos++;
        remainder -= w;
      }
      myassert(pos < input.size());
      output[j] = input[pos];
    }
}

/** Decimates input into the output, using a 'find the max of every n values' idea.
*/
void maxAbsDecimate1d(const Array1d<float> &input, Array1d<float> &output)
{
  if(input.size() > output.size()) {
    //Use Bresenham's algorithm in 1d to choose the points to draw
      size_t w = output.size();
    if (w <= 0) return;
    size_t intStep = size_t(input.size() / w);
    size_t remainderStep = input.size() - (intStep * w);
    size_t pos = 0;
    size_t remainder = 0;
    size_t prev = 0;
    size_t j=0;

    do {
      pos+=intStep;
      remainder+=remainderStep;
      if(remainder >= w) {
        pos++;
        remainder -= w;
      }
      myassert(pos <= input.size());
      output[j] = *std::max_element(input.begin() + prev, input.begin() + pos, absoluteLess<float>());
      prev = pos;
    } while(++j < w);
  } else if(input.size() == output.size()) {
    std::copy(input.begin(), input.end(), output.begin());
  } else {
    bresenham1d(input, output);
  }
}
