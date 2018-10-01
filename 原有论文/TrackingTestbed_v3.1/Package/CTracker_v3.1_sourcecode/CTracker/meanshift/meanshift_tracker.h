/*
 * Functions for performing mean-shift tracking
 */
#include "windows.h"
#include <winuser.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

void phistogram(unsigned char *indeximage, int nrows, int ncols,
  		int row, int col, int hrow, int hcol, 
	        float *histogram, int histlen);

float bhatcoeff(float *phist, float *qhist, int histlen);


float meanshift(float *modelhist, int histlen,
		unsigned char *indeximage, int nrows, int ncols,
		int row, int col, int hrow, int hcol,
		float stepepsilon, int *bestrow, int *bestcol);

float newmeanshift(float *modelhist, int histlen,
		   unsigned char *indeximage, short int *gradDx, short int *gradDy,
		   int nrows, int ncols, int row, int col, int hrow, int hcol,
		   float stepepsilon, int *bestrow, int *bestcol);

float newmeanshift2(float *modelhist, int histlen,
		   unsigned char *indeximage, short int *gradDx, short int *gradDy,
		   int nrows, int ncols, int row, int col, int hrow, int hcol,
		   float stepepsilon, int *bestrow, int *bestcol);

void bhatimage(float *modelhist, int histlen,
	       unsigned char *indeximage, int nrows, int ncols,
	       int hrow, int hcol, unsigned char *bhatpix);

float ratmeanshift(float *modelhist, int *datahist, int histlen,
		   unsigned char *indeximage, short int *gradDx, short int *gradDy, 
		   int nrows, int ncols, int row, int col, int hrow, int hcol,
		   float stepepsilon, int *bestrow, int *bestcol);

void histblock(unsigned char *indexim, int nrows, int ncols, 
	       int srow, int erow, int scol, int ecol,
	       int *histogram, int histlen, int *histsum);

void ratblock(float *modelhist, int *datahist, int histlen,
	      unsigned char *indeximage, int nrows, int ncols,
	      int startrow, int endrow, int startcol, int endcol,
	      int hrow, int hcol, unsigned char *bhatpix, float *minval, float *maxval);

void meantrack_init(double *input_face_rect, unsigned char *image_RGB, float *modelhist, int *datahist);

void meantrack_run(double *input_face_rect, double *output_face_rect, unsigned char *image_RGB, float *modelhist, int *datahist, float *bhatt_coeff);
void meantrack_run_new(double *input_blob_rect, double *output_blob_rect, unsigned char *image_RGB, float *modelhist, int *datahist, float *bhatt_coeff, BOOL size_adapt);
void meantrack_run_display(double *input_blob_rect, double *output_blob_rect, unsigned char *image_RGB, float *modelhist, int *datahist, float *bhatt_coeff, unsigned char *image_weight);
void meantrack_run_scale2D(double *input_blob_rect, double *output_blob_rect, unsigned char *image_RGB, float *modelhist, int *datahist, float *bhatt_coeff, unsigned char *image_weight);

void setMeanshiftSize(int rows, int cols);