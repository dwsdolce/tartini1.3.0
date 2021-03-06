/***************************************************************************
                          sound_stream.cpp  -  description
                             -------------------
    begin                : Sat Nov 27 2004
    copyright            : (C) 2004-2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/

#include "sound_stream.h"
#include <stdio.h>

const double v8=0x7F, v16=0x7FFF, v32=0x7FFFFFFF;

size_t SoundStream::writeFloats(float **channelData, size_t length, int ch)
{
    size_t written;
    int c;

    if(bits == 8) {
        uint8_t *temp = new uint8_t[length * channels];
        uint8_t *pos1 = temp;
        float *pos, *end;
        for(c=0; c<channels; c++) {
            end = channelData[c%ch] + length;
            pos1 = temp + c;
            for(pos = channelData[c%ch]; pos<end; pos++, pos1+=channels)
                *pos1 = (uint8_t)((*pos * v8) + v8);
        }
        written = write_bytes(temp, length * channels);
        delete[] temp;
        return written / channels;
    } else if(bits == 16) {
        uint16_t *temp = new uint16_t[length * channels * sizeof(uint16_t)];
        uint16_t*pos1 = temp;
        float *pos, *end;
        for(c=0; c<channels; c++) {
            end = channelData[c%ch] + length;
            pos1 = temp + c;
            for(pos = channelData[c%ch]; pos<end; pos++, pos1+=channels) {
                *pos1 = (uint16_t)(*pos * v16);
//#ifdef Q_OS_MACX //reverse endieness
#ifdef MACX //reverse endieness
                *pos1 = ((*pos1 & 0xFF00) >> 8) | ((*pos1 & 0x00FF) << 8);
#endif
            }
        }
        written = write_bytes(temp, length * channels * sizeof(uint16_t));
        delete[] temp;
        return written / channels / sizeof(short);
    }
    else if (bits == 32) {
        uint32_t *temp = new uint32_t[length * channels * sizeof(uint32_t)];
        uint32_t *pos1 = temp;
        float* pos, * end;
        for (c = 0; c < channels; c++) {
            end = channelData[c % ch] + length;
            pos1 = temp + c;
            for (pos = channelData[c % ch]; pos < end; pos++, pos1 += channels) {
                *pos1 = (uint32_t)(*pos * v32);
                //#ifdef Q_OS_MACX //reverse endieness
#ifdef MACX //reverse endieness
                * pos1 = ((*pos1 & 0xFF00) >> 8) | ((*pos1 & 0x00FF) << 8);
#endif
            }
        }
        written = write_bytes(temp, length * channels * sizeof(uint32_t));
        delete[] temp;
        return written / channels / sizeof(short);
    }
    fprintf(stderr, "%d bit data is not suported yet\n", bits);
    return 0;
}

size_t SoundStream::readFloats(float **channelData, size_t length, int ch)
{
    size_t read;
    int c;
    if(bits == 8) {
        unsigned char *temp = new unsigned char[length * channels];
        read = read_bytes(temp, length * channels);
        unsigned char *pos1 = temp;
        float *pos, *end;
        for(c=0; c<channels; c++) {
            end = channelData[c%ch] + length;
            pos1 = temp + c;
            for(pos = channelData[c%ch]; pos<end; pos++, pos1+=channels)
                *pos = float((double(*pos1) - v8) / v8);
        }
        delete[] temp;
        return read / channels;
    } else if(bits == 16) {
        short *temp = new short[length * channels * sizeof(short)];
        read = read_bytes(temp, length * channels * sizeof(short));
        short *pos1 = temp;
        float *pos, *end;
        for(c=0; c<channels; c++) {
            end = channelData[c%ch] + length;
            pos1 = temp + c;
            for(pos = channelData[c%ch]; pos<end; pos++, pos1+=channels) {
//#ifdef Q_OS_MACX //reverse endieness
#ifdef MACX //reverse endieness
                *pos1 = ((*pos1 & 0xFF00) >> 8) | ((*pos1 & 0x00FF) << 8);
#endif
                *pos = float(double(*pos1) / v16);
            }
        }
        delete[] temp;
        return read / channels / sizeof(short);
    } else if(bits == 32) {
        long *temp = new long[length * channels * sizeof(long)];
        read = read_bytes(temp, length * channels * sizeof(long));
        long *pos1 = temp;
        float *pos, *end;
        for(c=0; c<channels; c++) {
            end = channelData[c%ch] + length;
            pos1 = temp + c;
            for(pos = channelData[c%ch]; pos<end; pos++, pos1+=channels) {
//#ifdef Q_OS_MACX //reverse endieness
//                *pos1 = ((*pos1 & 0xFF00) >> 8) | ((*pos1 & 0x00FF) << 8);
//#endif
                *pos = float(double(*pos1) / v16);
            }
        }
        delete[] temp;
        return read / channels / sizeof(long);
    }
    //TODO: 24bit
    fprintf(stderr, "%d bit data is not suported yet\n", bits);
    return 0;
}

size_t SoundStream::writeReadFloats(float **outChannelData, int outCh, float **inChannelData, int inCh, size_t length)
{
  writeFloats(outChannelData, length, outCh);
  return readFloats(inChannelData, length, inCh);
}
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
