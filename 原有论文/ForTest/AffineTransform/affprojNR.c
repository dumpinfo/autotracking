
/* affprojNR.c  - subroutine for fitting affine and projective transforms 
 * using numerical recipes in C routines for solving a set of linear equations.
 * Some care is taken to generate a well-conditioned set of equations.
 *
 * Bob Collins, CMU, September 30, 1998
 * for attempted airborne geolocation 
 */

#include <math.h>
#include "nrutil.h"
#include <STDIO.H>
#include <FLOAT.H>
/*----------------------------------------------------------------------*/

/* compute mean and spread of point data - used for better conditioning */
void prepare_points(int numpts, float *u, float *v, float *x, float *y, 
		     float *u0, float *us, float *v0, float *vs,
		     float *x0, float *xs, float *y0, float *ys, int debug)
{
  int i;
  *u0 = *v0 = *x0 = *y0 = 0.0;
  *us = *vs = *xs = *ys = 0.0;

  for (i=0; i < numpts; i++) {
    *u0 += u[i];
    *v0 += v[i];
    *x0 += x[i];
    *y0 += y[i];
  }
  *u0 /= (float)numpts;
  *v0 /= (float)numpts;
  *x0 /= (float)numpts;
  *y0 /= (float)numpts;
  for (i=0; i < numpts; i++) {
    *us += (float)fabs(u[i]-*u0);
    *vs += (float)fabs(v[i]-*v0);
    *xs += (float)fabs(x[i]-*x0);
    *ys += (float)fabs(y[i]-*y0);
  }
  *us /= (float)numpts;
  *vs /= (float)numpts;
  *xs /= (float)numpts;
  *ys /= (float)numpts;

  if (debug) {
    printf(" Umean %g Uscale %g\n",*u0,*us);
    printf(" Vmean %g Vscale %g\n",*v0,*vs);
    printf(" Xmean %g Xscale %g\n",*x0,*xs);
    printf(" Ymean %g Yscale %g\n",*y0,*ys);
  } 
  return;
}

/*----------------------------------------------------------------------*/

#define TOL 1.0e-5

void svdfitproj(int numpts, float *U, float *V, float *X, float *Y, float *p, int debug)
{
  void svbksb(float **u, float w[], float **v, int m, int n, float b[],
	      float x[]);
  void svdcmp(float **a, int m, int n, float w[], float **v);
	        
  float **Amat, *b, *w, **vmat, *xvec;
  float wmax, thresh; // tmp;
  int i, j; //*indx,
  float u,v,x,y;
  float u0,us,v0,vs,x0,xs,y0,ys,newp[9];

  printf("IS IT THIS ONE?\n");
  prepare_points(numpts, U, V, X, Y, 
		 &u0, &us, &v0, &vs,
		 &x0, &xs, &y0, &ys, debug);
  
  Amat = matrix(1,2*numpts,1,8);
  vmat = matrix(1,8,1,8);
  b = vector(1,2*numpts);
  w = vector(1,8);
  xvec = vector(1,8);

  j=1;
  for (i=0; i < numpts; i++) {
    /* make system of equations better conditioned */
    u = (U[i] - u0) / us;
    v = (V[i] - v0) / vs;
    x = (X[i] - x0) / xs;
    y = (Y[i] - y0) / ys;
  
    Amat[j][1] = u;  Amat[j][2] = v;  Amat[j][3] = 1;
    Amat[j][4] = 0;     Amat[j][5] = 0;     Amat[j][6] = 0;
    Amat[j][7] = - u*x;   Amat[j][8] = - v*x;
    b[j] = x;
    j++;
    Amat[j][1] = 0;     Amat[j][2] = 0;     Amat[j][3] = 0;
    Amat[j][4] = u;  Amat[j][5] = v;  Amat[j][6] = 1;
    Amat[j][7] = - u*y;   Amat[j][8] = - v*y;
    b[j] = y;
    j++;
  }

  if (debug) {
    printf("System of Equations\n");
    for (i=1; i <= 2*numpts; i++)
      printf(" %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f | %.3f\n",
	     Amat[i][1], Amat[i][2], Amat[i][3],
	     Amat[i][4], Amat[i][5], Amat[i][6],
	     Amat[i][7], Amat[i][8], b[i]);
  }

  svdcmp(Amat,2*numpts,8,w,vmat);
  printf("SINGULAR VALUES\n"); 
  for (j=1; j<=8; j++) printf(" %.8f",w[j]);
  printf("\n"); 
  wmax=0.0;
  for (j=1;j<=8;j++)
    if (w[j] > wmax) wmax=w[j];
  thresh=(float)TOL*wmax;
  for (j=1;j<=8;j++)
    if (w[j] < thresh) w[j]=0.0;
  svbksb(Amat,w,vmat,2*numpts,8,b,xvec);

  for (i=0; i < 8; i++)
    p[i] = xvec[i+1];
  p[8] = 1.0;

  free_vector(xvec,1,8);
  free_vector(w,1,8);
  free_vector(b,1,2*numpts);
  free_matrix(vmat,1,8,1,8);
  free_matrix(Amat,1,2*numpts,1,8);

  /* now massage proj mat to take into account what
     we did to the points to well-condition them */

  newp[0] = (p[0]*xs + p[6]*x0)/us;
  newp[1] = (p[1]*xs + p[7]*x0)/vs;
  newp[2] = p[2]*xs + p[8]*x0 - 
       (u0*(p[0]*xs + p[6]*x0))/us - (v0*(p[1]*xs + p[7]*x0))/vs;
  newp[3] = (p[3]*ys + p[6]*y0)/us;
  newp[4] = (p[4]*ys + p[7]*y0)/vs;
  newp[5] = p[5]*ys + p[8]*y0 - 
       (u0*(p[3]*ys + p[6]*y0))/us - (v0*(p[4]*ys + p[7]*y0))/vs;
  newp[6] = p[6]/us;
  newp[7] = p[7]/vs;
  newp[8] = p[8] - (p[6]*u0)/us - (p[7]*v0)/vs;

  for (i=0; i < 9; i++)
    p[i] = newp[i]/newp[8];

  return;
}

/*----------------------------------------------------------------------*/

void svdfitaff(int numpts, float *U, float *V, float *X, float *Y, float *p, int debug)
{
  void svbksb(float **u, float w[], float **v, int m, int n, float b[],
	      float x[]);
  void svdcmp(float **a, int m, int n, float w[], float **v);
	        
  float **Amat, *b, *w, **vmat, *xvec;
  float wmax, thresh; //tmp, 
  int  i, j; //*indx,
  float u,v,x,y;
  float u0,us,v0,vs,x0,xs,y0,ys,newp[6];

  prepare_points(numpts, U, V, X, Y, 
		 &u0, &us, &v0, &vs,
		 &x0, &xs, &y0, &ys, debug);
  
  Amat = matrix(1,2*numpts,1,6);
  vmat = matrix(1,6,1,6);
  b = vector(1,2*numpts);
  w = vector(1,6);
  xvec = vector(1,6);

  j=1;
  for (i=0; i < numpts; i++) {
    /* make system of equations better conditioned */
    u = (U[i] - u0) / us;
    v = (V[i] - v0) / vs;
    x = (X[i] - x0) / xs;
    y = (Y[i] - y0) / ys;
  
    Amat[j][1] = u;  Amat[j][2] = v;  Amat[j][3] = 1;
    Amat[j][4] = 0;     Amat[j][5] = 0;     Amat[j][6] = 0;
    b[j] = x;
    j++;
    Amat[j][1] = 0;     Amat[j][2] = 0;     Amat[j][3] = 0;
    Amat[j][4] = u;  Amat[j][5] = v;  Amat[j][6] = 1;
    b[j] = y;
    j++;
  }

  if (debug) {
    printf("System of Equations\n");
    for (i=1; i <= 2*numpts; i++)
      printf(" %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f | %.3f\n",
	     Amat[i][1], Amat[i][2], Amat[i][3],
	     Amat[i][4], Amat[i][5], Amat[i][6],
	     b[i]);
  }

  svdcmp(Amat,2*numpts,6,w,vmat);
  wmax=0.0;
  for (j=1;j<=6;j++)
    if (w[j] > wmax) wmax=w[j];
  thresh= (float)TOL*wmax;
  for (j=1;j<=6;j++)
    if (w[j] < thresh) w[j]=0.0;
  svbksb(Amat,w,vmat,2*numpts,6,b,xvec);

  for (i=0; i < 6; i++)
    p[i] = xvec[i+1];


  free_vector(xvec,1,6);
  free_vector(w,1,6);
  free_vector(b,1,2*numpts);
  free_matrix(vmat,1,6,1,6);
  free_matrix(Amat,1,2*numpts,1,6);

  /* now massage affine mat to take into account what
     we did to the points to well-condition them */

  newp[0] = (p[0]*xs)/us;
  newp[1] = (p[1]*xs)/vs;
  newp[2] = p[2]*xs - (p[0]*u0*xs)/us - (p[1]*v0*xs)/vs + x0;
  newp[3] = (p[3]*ys)/us;
  newp[4] = (p[4]*ys)/vs;
  newp[5] = p[5]*ys - (p[3]*u0*ys)/us - (p[4]*v0*ys)/vs + y0;

  for (i=0; i < 6; i++){
	  p[i] = newp[i];
	  //incase divide by zero
	  if (_isnan(newp[i])){
		  switch(i) {
		  case 0:
		  case 4:
			  p[i] = 1;
		  	break;
		  default:
			  p[i] = 0;
		  }		  
	  }
  }

  return;
}

