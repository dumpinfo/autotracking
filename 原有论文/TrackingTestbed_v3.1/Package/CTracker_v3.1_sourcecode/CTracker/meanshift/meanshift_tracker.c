             /*
 * meantrack.c - top level for mean-shift tracking
 * Original implementation by 
 * Bob Collins, Carnegie Mellon University, July 31, 2001
 * Matlab mex file written by
 * Raju Patil, Carnegie Mellon University, Nov 8, 2001
 */
//#include "stdafx.h"
#include "globaldata.h"
#include "imageProcessing.h"
#include "meanshift_tracker.h"
//#include "../Utility.h"

#define INIT_MODE 0
#define RUN_MODE  1

//#define NUMROWS 243
//#define NUMCOLS 320

//#define NUMROWS 480
//#define NUMCOLS 720
int NUMROWS=480;
int NUMCOLS=720;

#define SPEED_DELTA 10
#define FBD_DELTA 12
#define SIZE_DELTA  6

#define HISTLEN 8*8*4

#define PI 3.14159265358979


/*************************************/
/* Start of meanshift.c routines       */
/*************************************/
/*
 * Functions for performing mean-shift tracking
 * motivation is paper by Comanesciu, Ramesh and Meer in CVPR'00
 * Bob Collins  Carnegie Mellon University  Aug 1, 2001
 */
// acquire normalized weighted index histogram
// weights are according to Epanechnikov profile (4/(2 * pi)) * (1 - radiusq);
// since we are normalizing at end, we drop the constant 4/(2pi)

void phistogram(unsigned char *indeximage, int nrows, int ncols,
  		int row, int col, int hrow, int hcol, 
	        float *histogram, int histlen)
{
  unsigned char *dptr;
  float radiusq, accum, dr, dc;
  int i,r,c,srow,erow,scol,ecol;

  srow = row - hrow; if (srow < 0) srow = 0;  
  scol = col - hcol; if (scol < 0) scol = 0;  
  erow = row + hrow; if (erow >= nrows) erow = nrows-1;  
  ecol = col + hcol; if (ecol >= ncols) ecol = ncols-1;    
  for (i=0; i < histlen; i++) histogram[i] = 0.0;
  for (r = srow; r <= erow; r++) {
    dptr = indeximage + r*ncols + scol;
    dr = (float)(r - row)/(float)hrow;
    for (c = scol; c <= ecol; c++, dptr++) {
      dc = (float)(c - col)/(float)hcol;
      radiusq = dr*dr + dc*dc;
      if (radiusq < 1.0){      
		histogram[*dptr] += (float)((4/(2*PI))*(float)(1.0-radiusq));
	  }
	  //histogram[*dptr] ++;
    }
  }

  // Original code from Bob, to make norm(histogram) = 1 //
  accum=0.0;
  for  (i=0; i < histlen; i++)
    accum += (histogram[i] * histogram[i]);
  accum = (float)sqrt(accum);
  
  // orginal end//

  /* above code modified as follows by Raju Patil: 3 Oct 02, to make sum(histogram) = 1 */
//  accum=0.0;
//  for  (i=0; i < histlen; i++)
//  accum += histogram[i];
  /* code modification ends*/

  for  (i=0; i < histlen; i++)
    histogram[i] /= accum;
 
  return;
}


// Bhatacharyya coefficient for comparing two normalized histograms
// weights are according to Epanechnikov profile (4/(2 * pi)) * (1 - radiusq);
// since we are normalizing at end, we drop the constant 4/(2pi)

float bhatcoeff(float *modelhist, float *phist, int histlen)
{
  float accum;
  int i;
  accum = 0;
  for (i=0; i < histlen; i++, phist++, modelhist++) {
    accum += (float)sqrt((*phist) * (*modelhist));
  }

  return(accum);
}




//======================================================================
// FAST COMPUTATION OF BHATACHARYYA COEFF
// NOTE: NO CHECKING FOR OUT OF BOUND PIXELS IS DONE, BE CAREFUL!


//void histblock(unsigned char *indexim, int nrows, int ncols, 
//	       int srow, int erow, int scol, int ecol,
//	       int *histogram, int histlen, int *histsum);

void oldhistblock(unsigned char *indexim, int nrows, int ncols, 
	       int srow, int erow, int scol, int ecol,
	       int *histogram, int histlen, int *histsum)
{
  int i,r,c;
  unsigned char *dptr;
  for (i=0; i < histlen; i++) histogram[i] = 0;
  *histsum = 0;
  for (r = srow; r <= erow; r++) {
    dptr = indexim + r*ncols + scol;
    for (c = scol; c <= ecol; c++, dptr++) {
      histogram[*dptr]++;
      (*histsum)++;
    }
  }  
  return;
}

//update results based on shifting one column to right
void update_hist_col(unsigned char *indexim, int nrows, int ncols, 
		     int srow, int erow, int scol, int ecol,
		     int *histogram)
{
  int r;
  unsigned char *addptr, *subptr;
  subptr = indexim + srow*ncols + scol;
  addptr = indexim + srow*ncols + ecol + 1;
  for (r = srow; r <= erow; r++) {
    histogram[*subptr]--;
    histogram[*addptr]++;
    subptr += ncols;
    addptr += ncols;
  }
  return;
}

//update results based on shifting one row down
void update_hist_row(unsigned char *indexim, int nrows, int ncols, 
		     int srow, int erow, int scol, int ecol,
		     int *histogram)
{
  int c;
  unsigned char *addptr, *subptr;
  subptr = indexim + srow*ncols + scol;
  addptr = indexim + (erow+1)*ncols + scol;
  for (c = scol; c <= ecol; c++) {
    histogram[*subptr]--;
    histogram[*addptr]++;
    subptr++;
    addptr++;
  }
  return;
}


float fastbhat(float *modelhist, int histlen, int *histogram, int histsum)
{
  float accum;
  int i;

  for (i=0, accum=0.0; i < histlen; i++)
    if (modelhist[i])
      accum += (float)sqrt((float)modelhist[i] * (float)(histogram[i]));
  accum /= (float)sqrt((float)histsum);
  return(accum);
}


float fullbhat(float *modelhist, int histlen,
	       unsigned char *indeximage, int nrows, int ncols,
	       int row, int col, int hrow, int hcol)
{
  float score;
  int srow, scol, erow, ecol, histsum, histogram[256];

  srow = row - hrow; if (srow < 0) srow = 0;  
  scol = col - hcol; if (scol < 0) scol = 0;  
  erow = row + hrow; if (erow >= nrows) erow = nrows-1;  
  ecol = col + hcol; if (ecol >= ncols) ecol = ncols-1;    
  histblock(indeximage, nrows, ncols, srow, erow, scol, ecol, histogram, histlen, &histsum);
  score = fastbhat(modelhist, histlen, histogram, histsum);
  return(score);
}


void bhatimage(float *modelhist, int histlen,
	       unsigned char *indeximage, int nrows, int ncols,
	       int hrow, int hcol, unsigned char *bhatpix)

{
  float score, *fptr, minscore, maxscore;
  unsigned char *dptr;
  int i,r,c,srow,erow,scol,ecol, numpix, histsum;
  int Isrow, Ierow, Iscol, Iecol;
  int histogram[256];
  int firsttime;

  numpix = nrows*ncols;
  fptr = NGIftemp;
  for (i=0; i < numpix; i++)
    *fptr++ = 0;

  Isrow = hrow; Ierow = nrows-hrow;
  Iscol = hcol; Iecol = ncols-hcol;

  firsttime = 1;

  for (c=Iscol; c < Iecol; c++) {
    fptr = NGIftemp+Isrow*ncols+c;
    srow = Isrow - hrow; 
    erow = Isrow + hrow;
    scol = c - hcol;
    ecol = c + hcol;
    histblock(indeximage, nrows, ncols, srow, erow, scol, ecol, histogram, histlen, &histsum);
    *fptr = score = fastbhat(modelhist, histlen, histogram, histsum);
    fptr+=ncols;
    if (firsttime) {
      minscore = maxscore = score;
      firsttime = 0;
    }
    if (score < minscore) minscore = score;
    if (score > maxscore) maxscore = score;
    for (r=Isrow; r < Ierow; r++) {
      update_hist_row(indeximage, nrows, ncols, srow, erow, scol, ecol, histogram);
      *fptr = score = fastbhat(modelhist, histlen, histogram, histsum);
      fptr+=ncols;
      if (score < minscore) minscore = score;
      if (score > maxscore) maxscore = score;
      srow++; erow++;
    }
  }

  dptr = bhatpix;
  fptr = NGIftemp;
  for (i=0; i < numpix; i++)
    *dptr++ = (unsigned char)(((*fptr++ - minscore)/(maxscore-minscore))*255);

  return;
}

void histsubtract(int *hist1, int *hist2, int histlen, int *histout, int *histsum)
{
  int i, sum;
  for (i=0, sum=0; i < histlen; i++) {
    histout[i] = hist1[i] - hist2[i];
    sum += histout[i];
  }
  *histsum = sum;
  return;
}

void oldbhatblock(float *modelhist, int histlen,
	       unsigned char *indeximage, int nrows, int ncols,
	       int startrow, int endrow, int startcol, int endcol,
	       int hrow, int hcol, unsigned char *bhatpix, float *minval, float *maxval)

{
  float score, *fptr, minscore, maxscore, outerscore;
  unsigned char *dptr;
  int r,c,srow,erow,scol,ecol, histsum;
  int Isrow, Ierow, Iscol, Iecol, h;
  int histogram[256];
  int firsttime;

  h = hrow / 2;
  if (h > (hcol / 2)) h = hcol / 2;
  if (h < 2) h = 2;

  if (startrow < hrow+h) startrow = hrow+h;
  if (startcol < hcol+h) startcol = hcol+h;
  if (endrow > (nrows-hrow-h)) endrow = (nrows-hrow-h);
  if (endcol > (ncols-hcol-h)) endcol = (ncols-hcol-h);

  Isrow = startrow; Ierow = endrow;
  Iscol = startcol; Iecol = endcol;

  firsttime = 1;

  for (c=Iscol; c < Iecol; c++) {
    fptr = NGIftemp+Isrow*ncols+c;
    srow = Isrow - hrow; 
    erow = Isrow + hrow;
    scol = c - hcol;
    ecol = c + hcol;
    histblock(indeximage, nrows, ncols, srow, erow, scol, ecol, histogram, histlen, &histsum);
//    histblock(indeximage, nrows, ncols, srow-h, erow+h, scol-h, ecol+h, bighist, histlen, &bigsum);
//    histsubtract(bighist,histogram,histlen,outerhist,&outersum);

    score = fastbhat(modelhist, histlen, histogram, histsum);
//    outerscore = fastbhat(modelhist, histlen, outerhist, outersum);
    outerscore = 0;
    *fptr = score = score - outerscore;
    fptr+=ncols;
    if (firsttime) {
      minscore = maxscore = score;
      firsttime = 0;
    }
    if (score < minscore) minscore = score;
    if (score > maxscore) maxscore = score;
    for (r=Isrow; r < Ierow; r++) {
      update_hist_row(indeximage, nrows, ncols, srow, erow, scol, ecol, histogram);
//      update_hist_row(indeximage, nrows, ncols, srow-h, erow+h, scol-h, ecol+h, bighist);
//      histsubtract(bighist,histogram,histlen,outerhist,&outersum);
      score = fastbhat(modelhist, histlen, histogram, histsum);
//      outerscore = fastbhat(modelhist, histlen, outerhist, outersum);
      outerscore = 0;
      *fptr = score = score - outerscore;
      fptr+=ncols;
      if (score < minscore) minscore = score;
      if (score > maxscore) maxscore = score;
      srow++; erow++;
    }
  }

  dptr = bhatpix;
  fptr = NGIftemp;
  memset(bhatpix,0,nrows*ncols);
  for (r=Isrow; r < Ierow; r++) {
    fptr = NGIftemp+r*ncols+Iscol;
    dptr = bhatpix+r*ncols+Iscol;
    for (c=Iscol; c < Iecol; c++)
      *dptr++ = (unsigned char)(((*fptr++ - minscore)/(maxscore-minscore))*255);
  }
  *minval = minscore;
  *maxval = maxscore;
  return;
}

void bhatblock2(float *modelhist, int histlen,
	       unsigned char *indeximage, int nrows, int ncols,
	       int startrow, int endrow, int startcol, int endcol,
	       int hrow, int hcol, unsigned char *bhatpix, float *minval, float *maxval)

{
  float score, *fptr, minscore, maxscore, outerscore;
  unsigned char *dptr;
  int r,c,srow,erow,scol,ecol, histsum,bigsum,outersum;
  int Isrow, Ierow, Iscol, Iecol, h;
  int histogram[256],bighist[256],outerhist[256];
  int firsttime;

  h = hrow / 2;
  if (h > (hcol / 2)) h = hcol / 2;
  if (h < 2) h = 2;

  if (startrow < hrow+h) startrow = hrow+h;
  if (startcol < hcol+h) startcol = hcol+h;
  if (endrow > (nrows-hrow-h)) endrow = (nrows-hrow-h);
  if (endcol > (ncols-hcol-h)) endcol = (ncols-hcol-h);

  Isrow = startrow; Ierow = endrow;
  Iscol = startcol; Iecol = endcol;

  firsttime = 1;

  for (c=Iscol; c < Iecol; c++) {
    fptr = NGIftemp+Isrow*ncols+c;
    srow = Isrow - hrow; 
    erow = Isrow + hrow;
    scol = c - hcol;
    ecol = c + hcol;
    histblock(indeximage, nrows, ncols, srow, erow, scol, ecol, histogram, histlen, &histsum);
    histblock(indeximage, nrows, ncols, srow-h, erow+h, scol-h, ecol+h, bighist, histlen, &bigsum);
    histsubtract(bighist,histogram,histlen,outerhist,&outersum);

    score = fastbhat(modelhist, histlen, histogram, histsum);
    outerscore = fastbhat(modelhist, histlen, outerhist, outersum);
    *fptr = score = score - outerscore;
    fptr+=ncols;
    if (firsttime) {
      minscore = maxscore = score;
      firsttime = 0;
    }
    if (score < minscore) minscore = score;
    if (score > maxscore) maxscore = score;
    for (r=Isrow; r < Ierow; r++) {
      update_hist_row(indeximage, nrows, ncols, srow, erow, scol, ecol, histogram);
      update_hist_row(indeximage, nrows, ncols, srow-h, erow+h, scol-h, ecol+h, bighist);
      histsubtract(bighist,histogram,histlen,outerhist,&outersum);
      score = fastbhat(modelhist, histlen, histogram, histsum);
      outerscore = fastbhat(modelhist, histlen, outerhist, outersum);
      *fptr = score = score - outerscore;
      fptr+=ncols;
      if (score < minscore) minscore = score;
      if (score > maxscore) maxscore = score;
      srow++; erow++;
    }
  }

  dptr = bhatpix;
  fptr = NGIftemp;
  memset(bhatpix,0,nrows*ncols);
  for (r=Isrow; r < Ierow; r++) {
    fptr = NGIftemp+r*ncols+Iscol;
    dptr = bhatpix+r*ncols+Iscol;
    for (c=Iscol; c < Iecol; c++)
      *dptr++ = (unsigned char)(((*fptr++ - minscore)/(maxscore-minscore))*255);
  }
  *minval = minscore;
  *maxval = maxscore;
  return;
}



void edgeblock(short int *gradDx, short int *gradDy, int nrows, int ncols,
	       int startrow, int endrow, int startcol, int endcol,
	       int hrow, int hcol, unsigned char *edgepix, float *minval, float *maxval)

{
  short int *dx1, *dx2;
  int r,c,rr,cc,hnrows,hncols, srow, erow, scol, ecol, count;
  int firsttime;
  float *fptr, laccum, raccum, score, minscore, maxscore;
  unsigned char *dptr;

  hnrows = nrows/2;
  hncols = ncols/2;

  if (startrow < 0) startrow = 0;
  if (startcol < 0) startcol = 0;
  if (endrow >= nrows) endrow = (nrows-1);
  if (endcol >= ncols) endcol = (ncols-1);

  firsttime = 1;

  for (r=startrow; r <= endrow; r++) {
    fptr = NGIftemp+r*ncols+startcol;
    for (c=startcol; c <= endcol; c++) {

      /*note, must take into account that gradDx and gradDy are halfsize
	images, i.e. of size nrows/2 X ncols/2 */

      srow = (r - hrow)/2; if (srow < 0) srow = 0;
      erow = (r + hrow)/2; if (erow >= hnrows) erow = (hnrows-1);
      scol = (c - hcol)/2; if (scol < 0) scol = 0;
      ecol = (c + hcol)/2; if (ecol >= hncols) ecol = (hncols-1);

      laccum = 0; raccum = 0; count = 0;
      for (rr = srow; rr <= erow; rr++) {
	dx1 = gradDx + rr*hncols + scol;
	dx2 = gradDx + rr*hncols + ecol;

	for (cc = scol; cc <= ecol; cc++) {
	  laccum += ((*dx1) < 0? -(*dx1) : (*dx1));
	  raccum += ((*dx2) < 0? -(*dx2) : (*dx2));
	  count++;
	  dx1 += hncols;
	  dx2 += hncols;
	}
      }

//      accum = *(gradDx + (r/2)*hncols + (c/2)); 
//      if (accum < 0) accum = -accum;
//      count = 1;

     if (laccum < raccum)
       *fptr++ = score = laccum / (float)count;
     else
       *fptr++ = score = raccum / (float)count;

     if (firsttime) {
       minscore = maxscore = score;
       firsttime = 0;
     }
     if (score < minscore) minscore = score;
     if (score > maxscore) maxscore = score;
    }
  }
  
  memset(edgepix,0,nrows*ncols);
  for (r=startrow; r <= endrow; r++) {
    fptr = NGIftemp+r*ncols+startcol;
    dptr = edgepix+r*ncols+startcol;
    for (c=startcol; c <= endcol; c++) {
      *dptr++ = (unsigned char)(((*fptr++ - minscore)/(maxscore-minscore))*255);
    }
  }
  *minval = minscore;
  *maxval = maxscore;
  return;
}


void bhatblock(float *modelhist, int histlen,
	       unsigned char *indeximage, int nrows, int ncols,
	       int startrow, int endrow, int startcol, int endcol,
	       int hrow, int hcol, unsigned char *bhatpix, float *minval, float *maxval);
//========================================

void bhatblock(float *modelhist, int histlen,
	       unsigned char *indeximage, int nrows, int ncols,
	       int startrow, int endrow, int startcol, int endcol,
	       int hrow, int hcol, unsigned char *bhatpix, float *minval, float *maxval)
     
{
  int i, h, sor, sir, eor, eir, soc, sic, eoc, eic, midc, midr, c, r;
  int inhist[256], outhist[256], *hiptr, *hoptr, insqlen, outsqlen;
  float fscore, *fptr, *mptr, indotprod, outdotprod;
  double fminscore, fmaxscore;
  unsigned char *dptr, *dsor, *dsir, *deor, *deir, *dstart, *dend;
  h = ((hrow < hcol) ? hrow/2 : hcol/2);
  if (h < 2) h = 2;

  if (startrow < 0) startrow = 0;
  if (startcol < 0) startcol = 0;
  if (endrow > nrows) endrow = nrows;
  if (endcol > ncols) endcol = ncols;

  fminscore = 99999.9; fmaxscore = -99999.9;
  dstart = indeximage;
  dend = indeximage + nrows * ncols;

  for (midc=startcol; midc < endcol; midc++) {
    fptr = NGIftemp+startrow*ncols+midc;
    sic = midc - hcol;         if (sic < 0) sic = 0;
    soc = midc - hcol - h;     if (soc < 0) soc = 0;
    eic = midc + hcol + 1;     if (eic > ncols) eic = ncols;
    eoc = midc + hcol + h + 1; if (eoc > ncols) eoc = ncols;
    midr = startrow;
    sir = midr - hrow;         if (sir < 0) sir = 0;
    sor = midr - hrow - h;     if (sor < 0) sor = 0;
    eir = midr + hrow + 1;     if (eir > nrows) eir = nrows;
    eor = midr + hrow + h + 1; if (eor > nrows) eor = nrows;

    for (i=0, hiptr=inhist, hoptr=outhist; i < histlen; i++, hiptr++, hoptr++) 
      *hiptr = *hoptr = 0;

    dsor = indeximage + sor * ncols + soc;
    for (r=sor; r < sir; r++, dsor += ncols) {
      for (dptr = dsor, c = soc; c < eoc; c++, dptr++) { 
		outhist[*dptr]++;
      }
     }
    for (r=sir; r < eir; r++, dsor += ncols) {
      dptr = dsor;
      for (c = soc; c < sic; c++, dptr++) {
		outhist[*dptr]++;
      }
      for (c = sic; c < eic; c++, dptr++) { 
		inhist[*dptr]++;
      }
      for (c = eic; c < eoc; c++, dptr++) {
		outhist[*dptr]++;
      }
    }
    for (r=eir; r < eor; r++, dsor += ncols) {
      for (dptr = dsor, c = soc; c < eoc; c++, dptr++) {
		outhist[*dptr]++;
      }
    }
    
    indotprod = outdotprod = 0.0;
    insqlen = outsqlen = 0;
    for (i=0, hiptr=inhist, hoptr=outhist, mptr=modelhist; i < histlen; i++, hiptr++, hoptr++, mptr++) {
      if (*hiptr) {
		insqlen += (*hiptr) * (*hiptr);
		indotprod += (float)(*hiptr) * (*mptr);
      }
      if (*hoptr) {
		outsqlen += (*hoptr) * (*hoptr);
		outdotprod += (float)(*hoptr) * (*mptr);
      }
    }
    
    fscore = *fptr = (float)(indotprod/sqrt((float)insqlen) -  outdotprod/sqrt((float)outsqlen));
    if (fscore > fmaxscore) fmaxscore = fscore;
    if (fscore < fminscore) fminscore = fscore;
    fptr += ncols;

    midr = startrow;
    sir = midr - hrow;
    sor = midr - hrow - h;
    eir = midr + hrow + 1;
    eor = midr + hrow + h + 1;
    dsor = indeximage + sor * ncols + soc;
    deor = indeximage + eor * ncols + soc;
    dsir = indeximage + sir * ncols + sic;
    deir = indeximage + eir * ncols + sic;

    for (midr = startrow+1; midr < endrow; midr++) {
      if (dsor > dstart) {
		for (dptr=dsor, c=soc; c < eoc; c++, dptr++) {
		  outdotprod -= modelhist[*dptr];
		  outsqlen -= (2 * outhist[*dptr] - 1);
		  outhist[*dptr]--;
		}
      }
      if (dsir > dstart) {
		for (dptr=dsir, c=sic; c < eic; c++, dptr++) {
		  indotprod -= modelhist[*dptr];
		  insqlen -= (2 * inhist[*dptr] - 1);
		  inhist[*dptr]--;
		  outdotprod += modelhist[*dptr];
		  outsqlen += (2 * outhist[*dptr] + 1);
		  outhist[*dptr]++;
		}
      }
      if (deir < dend) {
		for (dptr=deir, c=sic; c < eic; c++, dptr++) {
		  indotprod += modelhist[*dptr];
		  insqlen += (2 * inhist[*dptr] + 1);
		  inhist[*dptr]++;
		  outdotprod -= modelhist[*dptr];
		  outsqlen -= (2 * outhist[*dptr] - 1);
		  outhist[*dptr]--;
		}
      }
      if (deor < dend) {
		for (dptr=deor, c=soc; c < eoc; c++, dptr++) {
		  outdotprod += modelhist[*dptr];
		  outsqlen += (2 * outhist[*dptr] + 1);
		  outhist[*dptr]++;
		}
      }

 	  fscore = *fptr = (float)(indotprod/sqrt((float)insqlen) -  outdotprod/sqrt((float)outsqlen));
	  if (fscore > fmaxscore) fmaxscore = fscore;
	  if (fscore < fminscore) fminscore = fscore;
	  fptr += ncols;
	  dsor += ncols; deor += ncols; 
	  dsir += ncols; deir += ncols;
    }
  }

  dptr = bhatpix;
  memset(bhatpix,0,nrows*ncols);
  for (r=startrow; r < endrow; r++) {
    fptr = NGIftemp+r*ncols+startcol;
    dptr = bhatpix+r*ncols+startcol;
    for (c=startcol; c < endcol; c++){
      //*dptr++ = (unsigned char)(((*fptr++ - fminscore)/(fmaxscore-fminscore))*255);
		*dptr = (unsigned char)(((*fptr++ - fminscore)/(fmaxscore-fminscore))*255);
		dptr++;
	}
  }
  *minval = (float)fminscore;
  *maxval = (float)fmaxscore;
  return;
}



void histblock(unsigned char *indexim, int nrows, int ncols, 
	       int srow, int erow, int scol, int ecol,
	       int *histogram, int histlen, int *histsum)
{
  int i,r,c;
  unsigned char *dptr;
  for (i=0; i < histlen; i++) histogram[i] = 0;
  *histsum = 0;
  if (srow < 0) srow = 0;
  if (scol < 0) scol = 0;
  if (erow > nrows) erow = nrows;
  if (ecol > ncols) ecol = ncols;
  for (r = srow; r < erow; r++) {
    dptr = indexim + r*ncols + scol;
    for (c = scol; c < ecol; c++, dptr++) {
      histogram[*dptr]++;
      (*histsum)++;
    }
  }  
  return;
}


void ratblock(float *modelhist, int *datahist, int histlen,
	      unsigned char *indeximage, int nrows, int ncols,
	      int startrow, int endrow, int startcol, int endcol,
	      int hrow, int hcol, unsigned char *bhatpix, float *minval, float *maxval)
     
{
  int i, h, c, r;
  float fscore, fminscore, fmaxscore, *fptr;
  float rathist[256], accum;
  unsigned char *dptr;
  h = ((hrow < hcol) ? hrow/2 : hcol/2);
  if (h < 2) h = 2;

  if (startrow < 0) startrow = 0;
  if (startcol < 0) startcol = 0;
  if (endrow > nrows) endrow = nrows;
  if (endcol > ncols) endcol = ncols;

  //make norm(datahist) = 1
  accum = 0;
  for (i=0; i < histlen; i++)
    accum += (float)(datahist[i]*datahist[i]);
  accum = (float)sqrt(accum);
//  printf("accum is %f\n",accum);

  fminscore = 0; fmaxscore = 0;
  for (i=0; i < histlen; i++){ 
    if (datahist[i] && modelhist[i]) {
      fscore = rathist[i] = (float)sqrt(modelhist[i] / (datahist[i] / accum));
//      fscore = rathist[i] = modelhist[i] / (datahist[i] / accum);
//      fscore = rathist[i] = modelhist[i] * (datahist[i] / accum);
//      fscore = rathist[i] = modelhist[i];
      if (fscore > fmaxscore) fmaxscore = fscore;
      if (fscore < fminscore) fminscore = fscore;
    }
	else
		rathist[i] = 0;
  }

  //for every entry in indexing, get histogram ratio (as weight)
  for (r=startrow; r < endrow; r++) {
    fptr = NGIftemp+r*ncols + startcol;
    dptr = indeximage+r*ncols + startcol;
    for (c=startcol; c < endcol; c++) {
      *fptr = rathist[*dptr];
      fptr++;
      dptr++;
    }
  }
//  printf("min %f max %f\n",fminscore,fmaxscore);

  //normalize NGIftemp to 0~255, output bhatpix
  dptr = bhatpix;
  memset(bhatpix,0,nrows*ncols);
  for (r=startrow; r < endrow; r++) {
    fptr = NGIftemp+r*ncols+startcol;
    dptr = bhatpix+r*ncols+startcol;
    for (c=startcol; c < endcol; c++){
      //*dptr++ = (unsigned char)(((*fptr++ - fminscore)/(fmaxscore-fminscore))*255);
		*dptr = (unsigned char)(((*fptr++ - fminscore)/(fmaxscore-fminscore))*255);
		dptr++;
	}
  }
  *minval = fminscore;
  *maxval = fmaxscore;
  return;
}



float ratmeanshift(float *modelhist, int *datahist, int histlen,
		   unsigned char *indeximage, short int *gradDx, short int *gradDy, 
		   int nrows, int ncols, int row, int col, int hrow, int hcol,
		   float stepepsilon, int *bestrow, int *bestcol)
{
	float score, scoreB, scoreE, w;
	unsigned char *bhatim, *edgeim, *dptr, *eptr;
	float accum, accumB, accumE, sumr, sumc, dr, dc, eps2, stepdist2;
	float bhatmin, bhatmax;
	int count,r,c,srow,erow,scol,ecol,deltarow,deltacol;
	int newrow,newcol,histsum,ntimes,border;

	eps2 = stepepsilon*stepepsilon;

	bhatim = NGItemp;
	border = ((hcol < hrow) ? hcol : hrow);

	histblock(indeximage, nrows, ncols, 
	  row-hrow, row+hrow, col-hcol, col+hcol,
	  datahist, histlen, &histsum);

	ratblock(modelhist, datahist, histlen, indeximage, nrows, ncols, 
	   row-hrow-border,row+hrow+border,col-hcol-border,col+hcol+border,
	   hrow, hcol, bhatim, &bhatmin, &bhatmax);

	memcpy(NGIbhatim, bhatim, nrows*ncols);
	edgeim = NGItemp+nrows*ncols;
	//edgeblock(gradDx, gradDy, nrows, ncols, 
	//row-2*hrow,row+2*hrow,col-2*hcol,col+2*hcol,
	//hrow, hcol, edgeim, &edgemin, &edgemax);

//	//debug print out images
//	if(0)
//	{
//		FILE *file;
//		char *filename;
//		int row, col;
//		int offset;
//	
//		filename = "result/bhatim0.txt";
//		file = fopen(filename, "w");
//		for (row=0; row<243; row++){			
//			for (col=0; col<320; col++){
//				offset = row*320+col;
//				fprintf(file, "%3d ", *(bhatim+offset));
//			}
//			fprintf(file, "\n");
//		}
//	}

	stepdist2 = eps2+1;
	ntimes = 0;
	while ((stepdist2 >= eps2)&&(ntimes < 10)) {
		accum = accumB = accumE = sumr = sumc = 0.0;  count = 0;
		srow = row - hrow; if (srow < 0) srow = 0;  
		scol = col - hcol; if (scol < 0) scol = 0;  
		erow = row + hrow; if (erow >= nrows) erow = nrows-1;  
		ecol = col + hcol; if (ecol >= ncols) ecol = ncols-1;    
		for (r = srow; r <= erow; r++) {
		  dptr = bhatim + r*ncols + scol;
		  eptr = edgeim + r*ncols + scol;
		  dr = (float)(r - row);
		  for (c = scol; c <= ecol; c++, dptr++, eptr++) {
			dc = (float)(c - col);
			//	w = *dptr + *eptr;
			w = *dptr;
			sumr += w*dr;
			sumc += w*dc;
			accum += w;
			accumB += *dptr;
			accumE += *eptr;
			count++;
		  }
		}
		scoreB = accumB / (float)count;
		scoreE = accumE / (float)count;
		newrow = (int)(row + floor(sumr/accum+0.5));
		newcol = (int)(col + floor(sumc/accum+0.5));    
		deltarow = newrow - row;
		deltacol = newcol - col;
	//    fprintf(stdout,"  step row %d col %d\n",newrow,newcol);

		stepdist2 = (float)(deltarow*deltarow + deltacol*deltacol);
		row = newrow; 
		col = newcol;


		histblock(indeximage, nrows, ncols, 
			  row-hrow, row+hrow, col-hcol, col+hcol,
			  datahist, histlen, &histsum);
		ratblock(modelhist, datahist, histlen, indeximage, nrows, ncols, 
			 row-hrow-border,row+hrow+border,col-hcol-border,col+hcol+border,
			 hrow, hcol, bhatim, &bhatmin, &bhatmax);
		memcpy(NGIbhatim, bhatim, nrows*ncols);
		ntimes++;
	}

	//  fprintf(stdout,"  final row %d col %d\n",row,col);
	*bestrow = row;
	*bestcol = col;

	histblock(indeximage, nrows, ncols, 
		row-hrow, row+hrow, col-hcol, col+hcol,
		datahist, histlen, &histsum);
	bhatblock(modelhist, histlen, indeximage, nrows, ncols,
		row, row+1, col, col+1,
		hrow, hcol, NGItemp, &bhatmin, &bhatmax);
	score = bhatmax;
	return score;
}

void meantrack_init(double *input_blob_rect, unsigned char *image_RGB, float *modelhist, int *datahist)
{
  float row, col, hrow, hcol, numpix;
  //int row, col, hrow, hcol, numpix;
  int n;

  NGIbbrow = row = (float)input_blob_rect[0];   /* center_row  */
  NGIbbcol = col = (float)input_blob_rect[1];   /* center_col  */
  NGIbbhrow = hrow = (float)input_blob_rect[2];  /* half_height */
  NGIbbhcol = hcol = (float)input_blob_rect[3];  /* half_width  */
  NGIbbscale = (float)input_blob_rect[4]; /* scale       */
  NGIbbox = 1;

  NGInumrows = NUMROWS;
  NGInumcols = NUMCOLS;
  NGInumbands = 3;
  numpix = (float)(NGInumrows*NGInumcols);

  separatePlanes(image_RGB, NGIimage, NGInumrows,NGInumcols);
  color_index_image(NGIimage, NGInumcols, NGInumrows, NGIindeximage,1);
  phistogram(NGIindeximage, NGInumrows, NGInumcols, (int)row, (int)col, (int)hrow, (int)hcol, modelhist, HISTLEN);
  for (n=0; n < HISTLEN; n++) {
	  datahist[n] = (int)(100000.0*modelhist[n]);
	  if (datahist[n] != 0)
		  datahist[n] = datahist[n];
  }
  
  //cv_Imwrite_C1(NGIindeximage, NGInumcols, NGInumrows, "result/NGIindeximage_rect.bmp");
 
}

void meantrack_run(double *input_blob_rect, double *output_blob_rect, unsigned char *image_RGB, float *modelhist, int *datahist, float *bhatt_coeff)
{
  float row, col, hrow, hcol, testrow, testcol;
  int newrow, newcol, dumrow, dumcol;
  int copydatahist[256], newdatahist[256], i;
  float scale, newscale, eps=1.0, score, score2;
  
  NGIbbrow = row = (float)input_blob_rect[0];   /* center_row  */
  NGIbbcol = col = (float)input_blob_rect[1];   /* center_col  */
  NGIbbhrow = hrow = (float)input_blob_rect[2];  /* half_height */
  NGIbbhcol = hcol = (float)input_blob_rect[3];  /* half_width  */
  NGIbbscale = scale = (float)input_blob_rect[4]; /* scale       */
  NGIbbox = 1;

/*
  printf("row = %d\n",row);
  printf("col = %d\n",col);
  printf("hrow = %d\n",hrow);
  printf("hcol = %d\n",hcol);
  printf("scale = %f\n",scale);
  */

    testrow = row; testcol = col;

    separatePlanes(image_RGB, NGIimage, NGInumrows,NGInumcols);

    color_index_image(NGIimage, NGInumcols, NGInumrows, NGIindeximage, 0);

	for (i=0; i < 256; i++){
	  newdatahist[i] = datahist[i];
	}

  newscale = scale;
  score = ratmeanshift(modelhist, newdatahist, HISTLEN,
                       NGIindeximage, NGIgradDX, NGIgradDY, NGInumrows, NGInumcols,
                       (int)row, (int)col, (int)(scale*hrow), (int)(scale*hcol),
                       eps, &newrow, &newcol);

  testrow = (float)newrow;
  testcol = (float)newcol;
  if (((0.9*scale*hrow) > 3)&&((0.9*scale*hcol) > 3)) {
    for (i=0; i < 256; i++) copydatahist[i] = datahist[i];
    score2 = ratmeanshift(modelhist, copydatahist, HISTLEN,
                          NGIindeximage, NGIgradDX, NGIgradDY, NGInumrows, NGInumcols,
                          (int)testrow, (int)testcol, (int)(0.9*scale*hrow), (int)(0.9*scale*hcol),
                          eps, &dumrow, &dumcol);

    if (score2 > score) {
      score = score2;
      newscale =  (float)(0.9*scale);
      newrow = dumrow;
      newcol = dumcol;
      for (i=0; i < 256; i++) newdatahist[i] = copydatahist[i];
    }
  }

  for (i=0; i < 256; i++) copydatahist[i] = datahist[i];
  score2 = ratmeanshift(modelhist, copydatahist, HISTLEN,
                        NGIindeximage, NGIgradDX, NGIgradDY, NGInumrows, NGInumcols,
                        (int)testrow, (int)testcol, (int)(1.1*scale*hrow), (int)(1.1*scale*hcol),
                        eps, &dumrow, &dumcol);

  if (score2 > score) {
    score = score2;
    newscale =  (float)(1.1*scale);
    newrow = dumrow;
    newcol = dumcol;
    for (i=0; i < 256; i++) newdatahist[i] = copydatahist[i];
  }

    scale = (float)(0.9*scale+0.1*newscale);
    fprintf(stdout,"score %f: newrow %d newcol %d: scale %f\n", score,newrow,newcol,scale);
  NGIbbrow = (float)newrow;
  NGIbbcol = (float)newcol;
  NGIbbscale = scale;
  for (i=0; i < 256; i++) datahist[i] = newdatahist[i];

    output_blob_rect[0] = newrow;
    output_blob_rect[1] = newcol;
    output_blob_rect[2] = hrow;;
    output_blob_rect[3] = hcol;;
    output_blob_rect[4] = scale;

	*bhatt_coeff = score;

/*
  printf("newrow = %d\n",newrow);
  printf("newcol = %d\n",newcol);
  printf("hrow = %d\n",hrow);
  printf("hcol = %d\n",hcol);
  printf("scale = %f\n",scale);
  */
    
}
//meantrack_run_new was added on 24 Jan 2003 by Raju Patil
//The only difference between meantrack_run_new and meantrack_run is that
//in meantrack_run_new, the size adaptation can be turned off or on
//by passing 1 or 0 to the parameter "size_adapt"
void meantrack_run_new(double *input_blob_rect, double *output_blob_rect, unsigned char *image_RGB, float *modelhist, int *datahist, float *bhatt_coeff, BOOL size_adapt)
{
  float row, col, hrow, hcol, testrow, testcol;
  int newrow, newcol, dumrow, dumcol;
  int copydatahist[256], newdatahist[256], i;
  float scale, newscale, eps=1.0, score, score2;
  
  NGIbbrow = row = (float)input_blob_rect[0];   /* center_row  */
  NGIbbcol = col = (float)input_blob_rect[1];   /* center_col  */
  NGIbbhrow = hrow = (float)input_blob_rect[2];  /* half_height */
  NGIbbhcol = hcol = (float)input_blob_rect[3];  /* half_width  */
  NGIbbscale = scale = (float)input_blob_rect[4]; /* scale       */
  NGIbbox = 1;


  /*
  printf("row = %d\n",row);
  printf("col = %d\n",col);
  printf("hrow = %d\n",hrow);
  printf("hcol = %d\n",hcol);
  printf("scale = %f\n",scale);
  */

    testrow = row; testcol = col;

    separatePlanes(image_RGB, NGIimage, NGInumrows,NGInumcols);

    color_index_image(NGIimage, NGInumcols, NGInumrows, NGIindeximage, 0);

  for (i=0; i < 256; i++) newdatahist[i] = datahist[i];

  newscale = scale;
  score = ratmeanshift(modelhist, newdatahist, HISTLEN,
                       NGIindeximage, NGIgradDX, NGIgradDY, NGInumrows, NGInumcols,
                       (int)row, (int)col, (int)(scale*hrow), (int)(scale*hcol),
                       eps, &newrow, &newcol);

  testrow = (float)newrow;
  testcol = (float)newcol;

  if(size_adapt){ //only if size_adapt is TRUE, we check for different values of scale
	  if (((0.9*scale*hrow) > 3)&&((0.9*scale*hcol) > 3)) {
		  for (i=0; i < 256; i++) copydatahist[i] = datahist[i];
		  score2 = ratmeanshift(modelhist, copydatahist, HISTLEN,
			  NGIindeximage, NGIgradDX, NGIgradDY, NGInumrows, NGInumcols,
			  (int)testrow, (int)testcol, (int)(0.9*scale*hrow), (int)(0.9*scale*hcol),
			  eps, &dumrow, &dumcol);
		  
		  if (score2 > score) {
			  score = score2;
			  newscale =  (float)(0.9*scale);
			  newrow = dumrow;
			  newcol = dumcol;
			  for (i=0; i < 256; i++) newdatahist[i] = copydatahist[i];
		  }
	  }
	  
	  for (i=0; i < 256; i++) copydatahist[i] = datahist[i];
	  score2 = ratmeanshift(modelhist, copydatahist, HISTLEN,
		  NGIindeximage, NGIgradDX, NGIgradDY, NGInumrows, NGInumcols,
		  (int)testrow, (int)testcol, (int)(1.1*scale*hrow), (int)(1.1*scale*hcol),
		  eps, &dumrow, &dumcol);
	  
	  if (score2 > score) {
		  score = score2;
		  newscale =  (float)(1.1*scale);
		  newrow = dumrow;
		  newcol = dumcol;
		  for (i=0; i < 256; i++) newdatahist[i] = copydatahist[i];
	  }
	  
	  scale = (float)(0.9*scale+0.1*newscale);
	  
  }

  fprintf(stdout,"score %f: newrow %d newcol %d: scale %f\n", score,newrow,newcol,scale);
  NGIbbrow = (float)newrow;
  NGIbbcol = (float)newcol;
  NGIbbscale = scale;
  for (i=0; i < 256; i++) datahist[i] = newdatahist[i];

	output_blob_rect[0] = newrow;
	output_blob_rect[1] = newcol;
	output_blob_rect[2] = hrow;;
	output_blob_rect[3] = hcol;;
	output_blob_rect[4] = scale;

	*bhatt_coeff = score;

  /*
  printf("newrow = %d\n",newrow);
  printf("newcol = %d\n",newcol);
  printf("hrow = %d\n",hrow);
  printf("hcol = %d\n",hcol);
  printf("scale = %f\n",scale);
  */
}

//meantrack_run_scale2D was added on Jul 21st, 2003 by Xuhui Zhou
//The only difference between meantrack_run_scale2D and meantrack_run is that 
//1. the scale is in X, Y independently.
//2. pass image_weight as a parameter to display the weight image.
void meantrack_run_scale2D(double *input_blob_rect, double *output_blob_rect, unsigned char *image_RGB, float *modelhist, int *datahist, float *bhatt_coeff, unsigned char *image_weight)
{
  float row, col, hrow, hcol, testrow, testcol;
  int newrow, newcol, dumrow, dumcol;
  int copydatahist[256], i, newdatahist[256];
  //int *newdatahist;
  float scale, newscale, eps=1.0, score, score2;
  double scaleR, scaleC, newscaleR, newscaleC; //make Row, Col scale independently

  NGIbbrow = row = (float)input_blob_rect[0];   /* center_row  */
  NGIbbcol = col = (float)input_blob_rect[1];   /* center_col  */
  NGIbbhrow = hrow = (float)input_blob_rect[2];  /* half_height */
  NGIbbhcol = hcol = (float)input_blob_rect[3];  /* half_width  */
  NGIbbscale = scale = (float)input_blob_rect[4]; /* scale       */
  NGIbbox = 1;

  scale = 1;	//initialize input scale as 1. 
  newscaleR = 1;
  newscaleC = 1;
  /*
  printf("row = %d\n",row);
  printf("col = %d\n",col);
  printf("hrow = %d\n",hrow);
  printf("hcol = %d\n",hcol);
  printf("scale = %f\n",scale);
  */

    testrow = row; testcol = col;

    separatePlanes(image_RGB, NGIimage, NGInumrows,NGInumcols);
    color_index_image(NGIimage, NGInumcols, NGInumrows, NGIindeximage, 0);	

	//cv_Imwrite_C1(NGIindeximage, NGInumcols, NGInumrows, "result/NGIindeximage.bmp");

  for (i=0; i < 256; i++) newdatahist[i] = datahist[i];
//newdatahist = datahist;

  newscale = scale;
  score = ratmeanshift(modelhist, newdatahist, HISTLEN,
                       NGIindeximage, NGIgradDX, NGIgradDY, NGInumrows, NGInumcols,
                       (int)row, (int)col, (int)(scale*hrow), (int)(scale*hcol),
                       eps, &newrow, &newcol);

  testrow = (float)newrow;
  testcol = (float)newcol;

  memcpy(image_weight, NGIbhatim, NGInumrows*NGInumcols);

  scaleR = 1;
  scaleC = 1;
  if(1){ 
	  //scale down 0.9*0.9
	  if (((0.9*scale*hrow) > 3)&&((0.9*scale*hcol) > 3)) {
		  for (i=0; i < 256; i++) copydatahist[i] = datahist[i];
		  score2 = ratmeanshift(modelhist, copydatahist, HISTLEN,
			  NGIindeximage, NGIgradDX, NGIgradDY, NGInumrows, NGInumcols,
			  (int)testrow, (int)testcol, (int)(0.9*scale*hrow), (int)(0.9*scale*hcol),
			  eps, &dumrow, &dumcol);
		  
		  if (score2 > score) {
			  score = score2;
			  newscaleR = 0.9;
			  newscaleC = 0.9;
			  newrow = dumrow;
			  newcol = dumcol;
			  for (i=0; i < 256; i++) newdatahist[i] = copydatahist[i];
		  }
	  }

//	  //scale down 0.9*1
//	  if (((0.9*scale*hrow) > 3)) {
//		  for (i=0; i < 256; i++) copydatahist[i] = datahist[i];
//		  score2 = ratmeanshift(modelhist, copydatahist, HISTLEN,
//			  NGIindeximage, NGIgradDX, NGIgradDY, NGInumrows, NGInumcols,
//			  (int)testrow, (int)testcol, (int)(0.9*scale*hrow), (int)(scale*hcol),
//			  eps, &dumrow, &dumcol);
//		  
//		  if (score2 > score) {
//			  score = score2;
//			  //newscaleR = 0.9;	
//			  //newscaleC = 1;
//			  newrow = dumrow;
//			  newcol = dumcol;
//			  for (i=0; i < 256; i++) newdatahist[i] = copydatahist[i];
//		  }
//	  }
//
//	  //scale down 1*0.9
//	  if (((0.9*scale*hcol) > 3)) {
//		  for (i=0; i < 256; i++) copydatahist[i] = datahist[i];
//		  score2 = ratmeanshift(modelhist, copydatahist, HISTLEN,
//			  NGIindeximage, NGIgradDX, NGIgradDY, NGInumrows, NGInumcols,
//			  (int)testrow, (int)testcol, (int)(scale*hrow), (int)(0.9*scale*hcol),
//			  eps, &dumrow, &dumcol);
//		  
//		  if (score2 > score) {
//			  score = score2;
//			  //newscaleR = 1;
//			  //newscaleC = 0.9;			  
//			  newrow = dumrow;
//			  newcol = dumcol;
//			  for (i=0; i < 256; i++) newdatahist[i] = copydatahist[i];
//		  }
//	  }
	  
	  //scale up 1.1*1.1
	  for (i=0; i < 256; i++) copydatahist[i] = datahist[i];
	  score2 = ratmeanshift(modelhist, copydatahist, HISTLEN,
		  NGIindeximage, NGIgradDX, NGIgradDY, NGInumrows, NGInumcols,
		  (int)testrow, (int)testcol, (int)(1.1*scale*hrow), (int)(1.1*scale*hcol),
		  eps, &dumrow, &dumcol);
	  
	  if (score2 > score) {
		  score = score2;
		  newscaleR = 1.1;
		  newscaleC = 1.1;
		  newrow = dumrow;
		  newcol = dumcol;
		  for (i=0; i < 256; i++) newdatahist[i] = copydatahist[i];
	  }
	  
//	  //scale up 1.1*1
//	  for (i=0; i < 256; i++) copydatahist[i] = datahist[i];
//	  score2 = ratmeanshift(modelhist, copydatahist, HISTLEN,
//		  NGIindeximage, NGIgradDX, NGIgradDY, NGInumrows, NGInumcols,
//		  (int)testrow, (int)testcol, (int)(1.1*scale*hrow), (int)(scale*hcol),
//		  eps, &dumrow, &dumcol);
//	  
//	  if (score2 > score) {
//		  score = score2;
//		  //newscaleR = 1.1;		  
//		  //newscaleC = 1;
//		  newrow = dumrow;
//		  newcol = dumcol;
//		  for (i=0; i < 256; i++) newdatahist[i] = copydatahist[i];
//	  }
//
//	  //scale up 1*1.1
//	  for (i=0; i < 256; i++) copydatahist[i] = datahist[i];
//	  score2 = ratmeanshift(modelhist, copydatahist, HISTLEN,
//		  NGIindeximage, NGIgradDX, NGIgradDY, NGInumrows, NGInumcols,
//		  (int)testrow, (int)testcol, (int)(scale*hrow), (int)(1.1*scale*hcol),
//		  eps, &dumrow, &dumcol);
//	  
//	  if (score2 > score) {
//		  score = score2;
//		  //newscaleR = 1;
//		  //newscaleC = 1.1;
//		  newrow = dumrow;
//		  newcol = dumcol;
//		  for (i=0; i < 256; i++) newdatahist[i] = copydatahist[i];
//	  }
	  
	  //adjust scale
	  scaleR = (float)(newscaleR*0.1+1*0.9);
	  scaleC = (float)(newscaleC*0.1+1*0.9);
  }

  fprintf(stdout,"score %f: newrow %d newcol %d: scale %f\n", score,newrow,newcol,scale);
  NGIbbrow = (float)newrow;
  NGIbbcol = (float)newcol;
  NGIbbscale = scale;
  for (i=0; i < 256; i++) datahist[i] = newdatahist[i];

	output_blob_rect[0] = newrow;
	output_blob_rect[1] = newcol;
	output_blob_rect[2] = hrow;;
	output_blob_rect[3] = hcol;;
	output_blob_rect[4] = scaleR;
	output_blob_rect[5] = scaleC;

	*bhatt_coeff = score;

  /*
  printf("newrow = %d\n",newrow);
  printf("newcol = %d\n",newcol);
  printf("hrow = %d\n",hrow);
  printf("hcol = %d\n",hcol);
  printf("scale = %f\n",scale);
  */
}

void setMeanshiftSize(int rows, int cols)
{
	NUMROWS = rows;
	NUMCOLS = cols;
	NGInumrows = NUMROWS;
	NGInumcols = NUMCOLS;
	NGInumbands = 3;
}
