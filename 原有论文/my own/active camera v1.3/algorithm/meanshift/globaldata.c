// global data structures for mean shift tracking and image processing
// Bob Collins, Carnegie Mellon University, July 31, 2001
//#include "stdafx.h"
#include <stdio.h> 
#include "globaldata.h"

char NGIfilepre[256], NGIfilepost[10];
int NGIfilestartframe, NGIfilecurframe, NGIfilenumdigits;
int NGInumrows, NGInumcols, NGInumbands;

float NGIbbox, NGIbbrow, NGIbbcol, NGIbbhrow, NGIbbhcol;
float NGIbbscale;

unsigned char NGIimage[MAXPIX*MAXBANDS];
unsigned char NGIrawinput[MAXPIX*MAXBANDS];
unsigned char NGIbhatim[MAXPIX];
unsigned char NGIindeximage[MAXPIX];
unsigned char NGItemp[MAXPIX*MAXBANDS];
float NGIftemp[MAXPIX];
int NGIitemp[MAXPIX];
unsigned char NGIsmooimage[HALFPIX];
short int NGIgradDX[HALFPIX], NGIgradDY[HALFPIX];

/*============================================*/
char *NGIfilename(int framenum)
{
  static char fname[256];
  
  if (NGIfilenumdigits == 0) {
    sprintf(fname, "%s.%s", NGIfilepre, NGIfilepost);
  }
  if (NGIfilenumdigits == 1) {
    sprintf(fname, "%s%01d.%s", NGIfilepre, framenum, NGIfilepost);
  }
  if (NGIfilenumdigits == 2) {
    sprintf(fname, "%s%02d.%s", NGIfilepre, framenum, NGIfilepost);
  }
  if (NGIfilenumdigits == 3) {
    sprintf(fname, "%s%03d.%s", NGIfilepre, framenum, NGIfilepost);
  }
  if (NGIfilenumdigits == 4) {
    sprintf(fname, "%s%04d.%s", NGIfilepre, framenum, NGIfilepost);
  }
  if (NGIfilenumdigits == 5) {
    sprintf(fname, "%s%05d.%s", NGIfilepre, framenum, NGIfilepost);
  }
  if (NGIfilenumdigits == 6) {
    sprintf(fname, "%s%06d.%s", NGIfilepre, framenum, NGIfilepost);
  }
  if (NGIfilenumdigits == 7) {
    sprintf(fname, "%s%07d.%s", NGIfilepre, framenum, NGIfilepost);
  }
  if (NGIfilenumdigits == 8) {
    sprintf(fname, "%s%08d.%s", NGIfilepre, framenum, NGIfilepost);
  }
  if (NGIfilenumdigits == 9) {
    sprintf(fname, "%s%09d.%s", NGIfilepre, framenum, NGIfilepost);
  }
  if (NGIfilenumdigits == 10) {
    sprintf(fname, "%s%010d.%s", NGIfilepre, framenum, NGIfilepost);
  }
  if (NGIfilenumdigits  == 11) {
    sprintf(fname, "%s%011d.%s", NGIfilepre, framenum, NGIfilepost);
  }
  return fname;
};