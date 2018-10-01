/* imageProcessing.cpp : image processing for mean shift tracking
 * Bob Collins  Carnegie Mellon University  Aug 1, 2001
 */ 
//#include "stdafx.h"  
#include <stdio.h>
#include "globaldata.h"


//------------------------------------------------------------

void separatePlanes(unsigned char *src, 
		    unsigned char *dest, int nr, int nc)
{
  unsigned char *pr, *pg, *pb;
  const unsigned char *p = src;
  int size = nr*nc, i;
  pr = dest;
  pg = dest + size;
  pb = pg + size;
  for (i=0;i<size;i++)
    {
      *pr++ = *p++;
      *pg++ = *p++;
      *pb++ = *p++;
    }
}

void mergePlanes(unsigned char *src, unsigned char *dest, int nr, int nc)
{
  const unsigned char *pr, *pg, *pb;
  unsigned char *p = dest;
  int size = nr*nc, i;
  pr = src;
  pg = src + size;
  pb = pg + size;
  for (i=0;i<size;i++)
    {
      *p++ = *pr++;
      *p++ = *pg++;
      *p++ = *pb++;
    }
}

void crosshair(unsigned char *image, int nrows, int ncols, int numbands, int r, int c)
{
  int i,b;
  unsigned char *ptr;
  for (b=0; b < numbands; b++, image += ncols * nrows) {	
    ptr = image + r*ncols;
    for (i=0; i < ncols; i++, ptr++) {
      *(ptr-ncols) = 0; *ptr = 255; *(ptr+ncols) = 0;
    }
    ptr = image + (r-c/2)*ncols;
    for (i=0; i < ncols; i++, ptr++) {
      *(ptr-ncols) = 0; *ptr = 255; *(ptr+ncols) = 0;
    }
    ptr = image + (r+c/2)*ncols;
    for (i=0; i < ncols; i++, ptr++) {
      *(ptr-ncols) = 0; *ptr = 255; *(ptr+ncols) = 0;
    }
    
    ptr = image + c;
    for (i=0; i < nrows; i++, ptr+=ncols) {
      *(ptr-1) = 0; *ptr = 255; *(ptr+1) = 0;
    }
    ptr = image + c/2;
    for (i=0; i < nrows; i++, ptr+=ncols) {
      *(ptr-1) = 0; *ptr = 255; *(ptr+1) = 0;
    }
    ptr = image + c + c/2;
    for (i=0; i < nrows; i++, ptr+=ncols) {
      *(ptr-1) = 0; *ptr = 255; *(ptr+1) = 0;
    }
    
  }
  return;
}


void dbox(unsigned char *image, int nrows, int ncols, int numbands, 
	  int row, int col, int hrow, int hcol)
{
  unsigned char *ptr;
  int b,r,c,srow,erow,scol,ecol;

  srow = row - hrow; if (srow < 1) srow = 1;  
  scol = col - hcol; if (scol < 1) scol = 1;  
  erow = row + hrow; if (erow >= (nrows-1)) erow = nrows-2;  
  ecol = col + hcol; if (ecol >= (ncols-1)) ecol = ncols-2;    

  for (b=0; b < numbands; b++, image += ncols * nrows) {	
    ptr = image + (srow-1)*ncols + scol-1;
    for (c = scol-1; c <= ecol+1; c++, ptr++) *ptr = 0;
    ptr = image + (srow)*ncols + scol-1;
    for (c = scol-1; c <= ecol+1; c++, ptr++) *ptr = 255;
    ptr = image + (srow+1)*ncols + scol-1;
    for (c = scol-1; c <= ecol+1; c++, ptr++) *ptr = 0;

    ptr = image + (erow-1)*ncols + scol-1;
    for (c = scol-1; c <= ecol+1; c++, ptr++) *ptr = 0;
    ptr = image + (erow)*ncols + scol-1;
    for (c = scol-1; c <= ecol+1; c++, ptr++) *ptr = 255;
    ptr = image + (erow+1)*ncols + scol-1;
    for (c = scol-1; c <= ecol+1; c++, ptr++) *ptr = 0;

    ptr = image + (srow)*ncols + scol;
    for (r = srow; r <= erow; r++, ptr+=ncols) {
      *(ptr-1) = 0; *ptr = 255; *(ptr+1) = 0;
    }

    ptr = image + (srow)*ncols + ecol;
    for (r = srow; r <= erow; r++, ptr+=ncols) {
      *(ptr-1) = 0; *ptr = 255; *(ptr+1) = 0;
    }
  }
  return;
}


//======================================================================
// COMPUTING GRADIENTS USING SOBEL OPERATOR
// note: reduces width and height by 2 
// image is single plane (greyscale)
// Dx and Dy values are scaled by 16 

// Gaussian kernel as integer values, that is 
// filt = 6*d[n] + 4*(d[n-1]+d[n+1]) + 1*(d[n-2]+d[n+2]) 
#define G0  6
#define G1  4
#define G2  1
#define GSUM 16

// vertical Gaussian kernel assuming odd-row interpolation
// (to avoid interlace jaggedness).  That is, the above   
// formula with d[n-1] = (d[n]+d[n-2])/2                  
//          and d[n+1] = (d[n]+d[n+2])/2                  
// and thus       filt = 10*d(n) + 3*(d[n-2]+d[n+2])      
#define GI0 10
#define GI1  3

void sobel_grad_filter(unsigned char *image, int xsize, int ysize, int deinterlace,  
		       unsigned char *smoo, short int *Dx, short int *Dy, int *temp)
{
  unsigned char  *dptr, *dp;
  int H0,H1,H2,V0,V1,V2;
  int *tptr, *tp, *rptr, *rp;
  int *quad1, *quad2;
  int x,y,xsize2,ysize2, v, h, d1, d2;


  xsize2 = xsize / 2;
  ysize2 = ysize / 2;
  quad1 = temp;
  quad2 = temp + xsize2*ysize2;

  H0=G0; H1=G1; H2=G2;
  if (deinterlace) {
	  V0=GI0; V1=0; V2=GI1;}
  else {
	  V0=G0; V1=G1; V2=G2;}

  // Gaussian smoothing 
  dptr = image;
  tptr = quad2;
  for (y=0; y < ysize2; y++, dptr+=(xsize+xsize), tptr+=xsize2) {
	dp = dptr; 
	*tptr = GSUM * (*dp);  // first column
    for (x=2, tp=tptr+1, dp=dptr+2; x < xsize2; x++, tp++, dp+=2) {
      *tp = H0 * (*dp) +
            H1 * (*(dp-1) + *(dp+1)) +
            H2 * (*(dp-2) + *(dp+2));
      }

    *tp= GSUM * (*dp);    // last column
    }                  

  tptr = quad2;
  rptr = quad1;
  for (x=0, tp=tptr, rp=rptr; x < xsize2; x++, tp++, rp++)  //first row
	  *rp = *tp;  
  tptr += xsize2;
  rptr += xsize2;
  for (y=2; y < ysize2; y++, rptr+=xsize2, tptr+=xsize2) {
    for (x=0, tp=tptr, rp=rptr; x < xsize2; x++, tp++, rp++) {
       *rp = (V0 * (*tp) + 
		      V1 * (*(tp-xsize2) + *(tp+xsize2)) +
			  V2 * (*(tp-xsize) + *(tp+xsize))) >> 4;
       }
    }
  for (x=0, tp=tptr, rp=rptr; x < xsize2; x++, tp++, rp++) // last row
     *rp = *tp;  

  //copy to smoothed image
  tp = quad1;
  dp = smoo;
  for (x=0; x < xsize2*ysize2; x++)
	  *dp++ = (*tp++) >> 4;

  if ((Dx != NULL) && (Dy != NULL)) {
    //now compute directional derivatives
    tp = quad1;
    for (x=0; x < xsize2; x++) { // first row
	  *Dx++ = *Dy++ = 0;  
	  tp++;
	}
    for (y=2; y < ysize2; y++) {
	  *Dx++ = *Dy++ = 0; // first column
	  tp++;
	  for (x=2; x < xsize2; x++) {
		  h = *(tp+1) - *(tp-1);
		  v = *(tp+xsize2) - *(tp-xsize2);
		  d1 = *(tp-xsize2+1) - *(tp+xsize2-1);
		  d2 = *(tp+xsize2+1) - *(tp-xsize2-1);
		  // sobel operator
		  *Dx = (h + h + d1 + d2) >> 6;  //2^4 (grad) * 2^2 (sobel)
		  *Dy = (v + v - d1 + d2) >> 6;
		  tp++; Dx++; Dy++;
	  }
	  *Dx++ = *Dy++ = 0; // last column
	  tp++;
	}
    for (x=0; x < xsize2; x++) { // last row
	  *Dx++ = *Dy++ = 0;
	  tp++;
	}
  }
  return;
}


//======================================================================
// HISTOGRAM EQUALIZATION AND QUANTIZATION

// compute quantization map. step 1) intensity histogram, 
// step 2) cumulative intensity, step 3) quantization map.

void quantization_map (unsigned char *image, int nrows, int ncols, 
                       unsigned char *newvaltable, int numgreyvals)
{
  int i, numpix, accum;
  int histogram[256], cumhist[256];
  unsigned char *dptr;

  for (i=0; i < 256; i++) histogram[i] = 0;
  numpix = nrows*ncols;
  dptr = image;
  for (i=0; i < numpix; i++, dptr++)
     histogram[*dptr]++;

  //skip 0 and 255 values (black borders and saturated pixels)
  accum = 0;
  for (i=1; i < 255; i++)
    cumhist[i] = (accum += histogram[i]);

  newvaltable[0] = 0;
  newvaltable[255] = numgreyvals-1;
  for (i=1; i < 255; i++){
    newvaltable[i] = (unsigned char)((numgreyvals-1)*((float)cumhist[i]/(float)accum)+0.5);	
  }

  return;
}


// perform histogram equalization and quantization

void perform_quantization (unsigned char *image, int nrows, int ncols,
                           unsigned char *newvaltable)
{
  int i, numpix;
  unsigned char *dptr;

  numpix = nrows*ncols;
  dptr = image;
  for (i=0; i < numpix; i++, dptr++)
    *dptr = newvaltable[*dptr];
	
  return;
}

void gray_histeq (unsigned char *image, int nrows, int ncols, int numgreyvals)
{
  unsigned char newvaltable[256]; 
  quantization_map(image, nrows, ncols, newvaltable, numgreyvals);
  perform_quantization(image,nrows,ncols,newvaltable);
  return;
}


static unsigned char QUANTVALTABLE1[256];
static unsigned char QUANTVALTABLE2[256];
static unsigned char QUANTVALTABLE3[256];

void color_index_image (unsigned char *colorimage, int nrows, int ncols,
			unsigned char *indeximage, int reinitquant)
{
	unsigned char *red, *green, *blue, *index;
	unsigned char *tmpBG, *tmpGR, *tmpBGR, *r, *g, *b;
	int nsize, i;
	nsize = nrows*ncols;
	r = red = colorimage; 
	g = green = colorimage+nsize; 
	b = blue = colorimage+2*nsize;
	tmpBG = NGItemp;
	tmpGR = NGItemp+nsize;
	tmpBGR = NGItemp+2*nsize;
	for (i = 0; i < nsize; i++) {
		*tmpBG++ = (unsigned char)(((float)*b - (float)*g + 255.0)/2.0);
		*tmpGR++ = (unsigned char)(((float)*g - (float)*r + 255.0)/2.0);
		*tmpBGR++ = (unsigned char)(((float)*b + (float)*g + (double)*r)/3.0);
//		*tmpBG++ = *r;
//		*tmpGR++ = *g;
//		*tmpBGR++ = *b; 
		r++; g++; b++;
	}
	tmpBG = NGItemp;
	tmpGR = NGItemp+nsize;
	tmpBGR = NGItemp+2*nsize;
	if (reinitquant) {
		quantization_map(tmpBG, nrows, ncols, QUANTVALTABLE1, 8);
		quantization_map(tmpGR, nrows, ncols, QUANTVALTABLE2, 8);
		quantization_map(tmpBGR, nrows, ncols, QUANTVALTABLE3, 4);
	}

	//debug print out images
//	if(0)
//	{
//		FILE *file;
//		char *filename;
//		int row, col;
//		int offset;
//
//		filename = "result/tmpBGR.txt";
//		file = fopen(filename, "w");
//		for (row=0; row<nrows; row++){			
//			for (col=0; col<ncols; col++){
//				offset = row*ncols+col;
//				fprintf(file, "%3d ", *(tmpBGR+offset));
//			}
//			fprintf(file, "\n");
//		}
//		fclose(file);
//		filename = "result/tmpGR.txt";
//		file = fopen(filename, "w");
//		for (row=0; row<nrows; row++){			
//			for (col=0; col<ncols; col++){
//				offset = row*ncols+col;
//				fprintf(file, "%3d ", *(tmpGR+offset));
//			}
//			fprintf(file, "\n");
//		}
//		fclose(file);
//		filename = "result/tmpBG.txt";
//		file = fopen(filename, "w");
//		for (row=0; row<nrows; row++){			
//			for (col=0; col<ncols; col++){
//				offset = row*ncols+col;
//				fprintf(file, "%3d ", *(tmpBG+offset));
//			}
//			fprintf(file, "\n");
//		}
//		fclose(file);
//	}

	perform_quantization(tmpBG,nrows,ncols,QUANTVALTABLE1);
	perform_quantization(tmpGR,nrows,ncols,QUANTVALTABLE2);
	perform_quantization(tmpBGR,nrows,ncols,QUANTVALTABLE3);

//	//debug print out images
//	if(0)
//	{
//		FILE *file;
//		char *filename;
//		int row, col;
//		int offset;
//
//		filename = "result/tmpBGR.txt";
//		file = fopen(filename, "w");
//		for (row=0; row<nrows; row++){			
//			for (col=0; col<ncols; col++){
//				offset = row*ncols+col;
//				fprintf(file, "%3d ", *(tmpBGR+offset));
//			}
//			fprintf(file, "\n");
//		}
//		fclose(file);
//		filename = "result/tmpGR.txt";
//		file = fopen(filename, "w");
//		for (row=0; row<nrows; row++){			
//			for (col=0; col<ncols; col++){
//				offset = row*ncols+col;
//				fprintf(file, "%3d ", *(tmpGR+offset));
//			}
//			fprintf(file, "\n");
//		}
//		fclose(file);
//		filename = "result/tmpBG.txt";
//		file = fopen(filename, "w");
//		for (row=0; row<nrows; row++){			
//			for (col=0; col<ncols; col++){
//				offset = row*ncols+col;
//				fprintf(file, "%3d ", *(tmpBG+offset));
//			}
//			fprintf(file, "\n");
//		}
//		fclose(file);
//	}

	index = indeximage;
	for (i=0; i < nsize; i++){ 
		*index = (unsigned char)(((*tmpBGR++)*8 + (*tmpGR++))*8 + (*tmpBG++));		
		index ++;
	}

	//debug print out images
	if(0)
	{
		FILE *file;
		char *filename;
		int row, col;
		int offset;

		filename = "result/indeximage.txt";
		file = fopen(filename, "w");
		for (row=0; row<243; row++){			
			for (col=0; col<320; col++){
				offset = row*320+col;
				fprintf(file, "%3d ", *(indeximage+offset));
			}
			fprintf(file, "\n");
		}
	}

  return;
}



