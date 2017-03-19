/*------------------------------------------------------------------------------

  ADRINAS: Automatic Detection and Removal of Impulsive Noise in Audio Signals

  This code is part of the following publication and was subject
  to peer review:

    "Automatic Detection and Removal of Impulsive Noise in Audio Signals"
    by Laurent Oudre, Image Processing On Line, 2015.
    DOI:10.5201/ipol.2015.XXXXX
    http://dx.doi.org/10.5201/ipol.2015.XXXXX

  Copyright (c) 2012-2015 Laurent Oudre <laurent.oudre@cmla.ens-cachan.fr>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.

------------------------------------------------------------------------------*/
#ifndef _SOUND_H_
#define _SOUND_H_

#include <sndfile.h>

typedef struct _Sound
{
  SF_INFO _sfinfo;
  double ** channel;
  long nChannels;
  long nSamples;
  long start;
  int sampleRate;
  int Nbit;
  char errMsg[1000];  /* static memory */
} Sound;

void soundClean(Sound * S);
Sound * audioReadFull(const char * infile);
Sound * audioRead(const char * infile, long n1, long n2);
void audioWrite(Sound * S, const char * outfile);

#endif
/*----------------------------------------------------------------------------*/
