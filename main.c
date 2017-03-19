/*------------------------------------------------------------------------------

 AudioCorruptDetect
 
  V2.0:  add analyse only support
  V2.1: add Sample Hold detect support
  V2.2: Large files

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
#include <stdio.h>
#include <stdlib.h>
#include "sound.h"
#include "AudioCorruptDetect.h"

/*----------------------------------------------------------------------------*/
int main(int argc, char ** argv)
{
    char  *progname, *infilename, *outfilename ;
    SNDFILE		*infile = NULL ;
    FILE		*outfile = NULL ;
    SF_INFO		sfinfo ;


  double K = 2.0;    /* First threshold */
  int b = 20;        /* Second threshold */
  int p = 3*100+2;   /* Order of the model */
  int Nw = 8*p;      /* Window length */
  int T = 0; /*Test Mode, Wav output generated*/
  int Aud= 0; /*Audacity Mode,  output Audacity marker Std Out*/
  int SampleHold= 0; /*Detect consecutive same sample Values*/
  int sampleHoldTresh= 3; /* Number of  consecutive same sample Values*/
  int mergeTimeHoldsFeMultiplier=1;
  int mergeTimeHolds=0;
  long nLongBuffer=100000;
  long readCount=0;
  Sound * signal = NULL;
  Sound * soundInfo = NULL;
  int * burst1 = NULL;
  int * burst2 = NULL;
  int nSamples = 0;
  long numAudioSamples = 0;
  int i,c;

  /* read input input */
  if( argc < 3 )
    {
      fprintf(stderr,"usage: AudioCorruptDetect in.wav out.wav [K b p Nw]\n"
                     "optional parameters:\n"
                     "       K: threshold parameter    (default value %0.2f)\n"
                     "       b: burst fusion parameter (default value %d)\n"
                     "       p: order of AR model      (default value %d)\n"
                     "       Nw: window size           (default value %d)\n"
                     "       T: Test Mode , no output file (default value %d)\n"
                     "       Aud: Audacity Mode ,  output Audacity Marker Std Out (default value %d)\n"
		     "       SampleHold: Detect consecutive same sample Values , (default value %d)\n"
		     "       sampleHoldTresh:Number of  consecutive same sample Values , (default value %d)\n" ,
                     K,b,p,Nw,T,Aud,SampleHold,sampleHoldTresh);
      exit(EXIT_FAILURE);
    }

    soundInfo = audioRead(argv[1],0,0);
    numAudioSamples = soundInfo->_sfinfo.frames;
    printf("nchannels: %d\n",soundInfo->_sfinfo.channels);
  if(Aud==0)
        {
        printf("numAudioSamples: %ld\n",numAudioSamples);
        }



  if( argc >= 4 ) K  = atof(argv[3]);
  if( argc >= 5 ) b  = atoi(argv[4]);
  if( argc >= 6 ) p  = atoi(argv[5]);
  if( argc >= 7 ) Nw = atoi(argv[6]);
  if( argc >= 8 ) T = atoi(argv[7]);
  if( argc >= 9 ) Aud = atoi(argv[8]);
  if( argc >= 10) SampleHold = atoi(argv[9]);
  if( argc >= 11) sampleHoldTresh = atoi(argv[10]);

 if(Aud==0)
{
  printf("input signal: %ld channel, %ld samples\n",
         soundInfo->nChannels,soundInfo->nSamples);
  printf("parameters: K=%.2f b=%d p=%d Nw=%d T=%d Aud=%d\n\n",K,b,p,Nw,T,Aud);
}

/*Click Detect Mode*/
if (SampleHold==0)
{
	  /* process each channel */
	  for(c=0; c<signal->nChannels; c++)
	    {
	  if(Aud==0)
	{
	printf("processing channel #%d:\n",c);
	}
	      /* iteration 1 */
	      burst1 = adrinas(signal->channel[0],signal->nSamples,p,K,b,Nw);
	      /* iteration 2 */
	      burst2 = adrinas(signal->channel[0],signal->nSamples,p,K,b,Nw);

	      /* compute number of samples detected as noise */
	      for(i=0; i<(signal->nSamples); i++)
	        if( (burst1[i]==1) || (burst2[i]==1) )
		{
		++nSamples;
		printf("%0.5f	%0.5f	Digital Click Channel: %d\n",(float)i / signal->sampleRate,(float)i / signal->sampleRate,c);
		}
	        if(Aud==0)
	       {
	      printf("  samples detected as noise: %d (%0.2f%%)\n",
	             nSamples, 100.0 * nSamples / signal->nSamples);
	      }
	    }

	if(T==0)
	{
	  /* write output */
	  audioWrite(signal,argv[2]);
	}
}

/*Sample Hold Detect  Mode*/
if (SampleHold==1)
{
//integration time of events
mergeTimeHolds=mergeTimeHoldsFeMultiplier *  soundInfo->sampleRate;
 /* process each channel */


        while(readCount < numAudioSamples)
        {
            //printf("readcount: %ld",readCount);
            signal = audioRead(argv[1],readCount,readCount+nLongBuffer);
	      /* iteration 1 */

            for(c=0; c<soundInfo->nChannels; c++)
            {

              if(Aud==0)
                {
                printf("processing channel #%d:\n",c);
                }
              burst1 = sampleHoldDetect(signal->channel[c],signal->nSamples,sampleHoldTresh, mergeTimeHolds);
              /* compute number of samples detected as SampleHold */

              for(i=0; i<(signal->nSamples); i++)
                if( (burst1[i]==1) )
                {
                ++nSamples;
                printf("%0.5f	%0.5f	Suspicious Sample Hold of  %d samples, channel %d \n",(float)(readCount+i) / signal->sampleRate,(float)(readCount+i) / signal->sampleRate,sampleHoldTresh,c);
                }

                readCount+=nLongBuffer+1;
                free(burst1);
            }
            free(signal);
        }

    if(Aud==0)
    {
        printf("  samples detected as samplehold: %d (%0.2f%%)\n",
        nSamples, 100.0 * nSamples / signal->nSamples);
    }

}
  /* free memory */
  soundClean(signal);
  soundClean(soundInfo);

  free(burst1);
  free(burst2);

  return 0;
}
/*----------------------------------------------------------------------------*/
