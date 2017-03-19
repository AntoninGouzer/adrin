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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sound.h"

#define BLOCK_SIZE 512

/*----------------------------------------------------------------------------*/
static void soundResize(Sound * S, long n)
{
  long size = sizeof(double) * S->nChannels * n;
  double * v = (double *) malloc(size);
  double ** w = (double **) malloc( sizeof(double *) * S->nChannels );
  long smin;
  long i;
  int k;

  for(k=0; k<S->nChannels; k++)
    w[k] = v + k * n;

  smin = (n < S->nSamples) ? n : S->nSamples;

  for(k=0; k<S->nChannels; k++)
    {
      for(i=0; i<smin; i++) w[k][i] = S->channel[k][i];
      for(; i<n; i++) w[k][i] = 0.0;
    }

  S->nSamples = n;
  if( S->channel )
    {
      free(S->channel[0]);
      free(S->channel);
    }
  S->channel = w;
}

/*----------------------------------------------------------------------------*/
void soundClean(Sound * S)
{
  if( S->channel )
    {
      free(S->channel[0]);
      free(S->channel);
    }
  free(S);
}

/*----------------------------------------------------------------------------*/
//test
Sound * audioReadFull(const char * infile)
{
  SNDFILE * f;
  double * buf;  /* for working */
  int k, m;
  long toRead,iSample;
  Sound * S = (Sound *) malloc( sizeof(Sound) );
  iSample=0;
  S->nChannels = 0;
  S->nSamples = 0L;
  S->channel = NULL;
  S->sampleRate = 0;
  S->Nbit=0;



  S->_sfinfo.format = 0;

  /* sf_open is a function of SNDFILE */
  f = sf_open(infile, SFM_READ, &(S->_sfinfo));
  S->sampleRate = S->_sfinfo.samplerate;
  S->nSamples = S->_sfinfo.frames;

  if( f )
    {
      S->nChannels = S->_sfinfo.channels;
      strcpy(S->errMsg, "");
    }
  else
    {
      strcpy(S->errMsg, "error (audioRead): Sound input file cannot be opened");
      return S;
    }



  buf = (double *) malloc( S->nChannels * S->nSamples * sizeof(double) );
    if (buf == NULL) {
		strcpy(S->errMsg, "error (audioRead): Buffer cannot be allocated");
		free(buf);
        sf_close(f);
		return S;
	}
long numFrames = sf_readf_double(f, buf, S->nSamples);
printf("numFrames 1 %ld",numFrames);
// Check correct number of samples loaded
	if (numFrames != S->nSamples) {
		strcpy(S->errMsg, "error (audioRead): Not all samples could be loaded");
		free(buf);
        sf_close(f);
		return S;
	}


// fill S with buffer

        soundResize(S, 2*S->nSamples);
        toRead = numFrames;
      for(m=0; m<numFrames; m++, iSample++)
        for(k=0; k<S->nChannels; k++)
          S->channel[k][iSample] = buf[k + S->nChannels * m];
      toRead -= numFrames;


  S->start = 1;
  free(buf);
  sf_close(f);

  return S;
}


/*----------------------------------------------------------------------------*/
Sound * audioRead(const char * infile, long n1, long n2)
{
  SNDFILE * f;
  double * buf;  /* for working */
  int k, m, readcount, blocksize;
  long toPass, toRead, nSamples;
  Sound * S = (Sound *) malloc( sizeof(Sound) );

  S->nChannels = 0;
  S->nSamples = 0L;
  S->channel = NULL;
  S->sampleRate = 0;
  S->Nbit=0;

  if( n2 == -1L ) n2 = 100000000L;

  if( n1 >= n2 && n1 != 0L && n2 != 0L )
    {
      strcpy(S->errMsg, "error (audioRead): n1 >= n2");
      return S;
    }

  S->_sfinfo.format = 0;

  /* sf_open is a function of SNDFILE */
  f = sf_open(infile, SFM_READ, &(S->_sfinfo));
  S->sampleRate = S->_sfinfo.samplerate;

  if( f )
    {
      S->nChannels = S->_sfinfo.channels;
      strcpy(S->errMsg, "");
    }
  else
    {
      strcpy(S->errMsg, "error (audioRead): Sound input file cannot be opened");
      return S;
    }

  //We only want to retrieve file infos
  if (n1==0L && n2==0L)
  {
      return S;
  }

  buf = (double *) malloc( S->nChannels * BLOCK_SIZE * sizeof(double) );

  nSamples = n2 - n1 + 1;

  soundResize(S, 1000L);

  /* The n1th file sample is the first signal sample,
     we pass the n1-1 first file samples */
  toPass = n1-1;
  blocksize = toPass > BLOCK_SIZE ? BLOCK_SIZE : toPass;

  while( (readcount = sf_readf_double(f, buf, blocksize)) > 0 )
    {
      toPass -= readcount;
      blocksize = toPass > BLOCK_SIZE ? BLOCK_SIZE : toPass;
    }

  toRead = nSamples;
  blocksize = toRead > BLOCK_SIZE ? BLOCK_SIZE : toRead;

  while( (readcount = sf_readf_double(f, buf, blocksize)) > 0 )
    {
      long iSample = nSamples - toRead;
      if( (iSample + readcount) > S->nSamples )
	{
        soundResize(S, 2*S->nSamples);

	}

      for(m=0; m<readcount; m++, iSample++)

        for(k=0; k<S->nChannels; k++)
          S->channel[k][iSample] = buf[k + S->nChannels * m];
      toRead -= readcount;
      blocksize = toRead > BLOCK_SIZE ? BLOCK_SIZE : toRead;
    }

  soundResize(S, nSamples - toRead);
  S->start = n1;
  free(buf);

  sf_close(f);

  return S;
}

/*----------------------------------------------------------------------------*/
void audioWrite(Sound * S, const char * outfile)
{
  SNDFILE * f;
  double * buf;
  int k, m, writecount, blocksize;
  long toWrite;
  long iSample;
  SF_INFO * p = &(S->_sfinfo);

  f = sf_open(outfile, SFM_WRITE, p);
  if( f )
    {
      strcpy(S->errMsg, "");
    }
  else
    {
      strcpy(S->errMsg, "error (audioWrite): "
                        "Sound output file cannot be opened");
      return;
    }

  buf = (double *) malloc(sizeof(double) * S->nChannels * BLOCK_SIZE);
  toWrite = S->nSamples;
  blocksize = toWrite > BLOCK_SIZE ? BLOCK_SIZE : toWrite;

  iSample = 0;
  while( toWrite > 0 )
    {
      for(m=0; m<blocksize; m++, iSample++)
        for(k=0; k<S->nChannels; k++)
          buf[k + S->nChannels * m] = S->channel[k][iSample];

      writecount = sf_writef_double (f, buf, blocksize);

      if( writecount < blocksize )
        {
          strcpy(S->errMsg, "error (audioWrite): Cannot write all samples");
          return;
        }
      toWrite -= writecount;
      blocksize = toWrite > BLOCK_SIZE ? BLOCK_SIZE : toWrite;
    }
  free(buf);
  sf_close(f);
}

/*----------------------------------------------------------------------------*/
