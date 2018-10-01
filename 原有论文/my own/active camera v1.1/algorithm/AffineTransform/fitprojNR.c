 
/* fitprojNR.c - main routine for fitting affine and projective transforms 
 * using numerical recipes in C routines for solving a set of linear equations.
 * File containing the actual fitting code is affprojNR.c
 *
 * Bob Collins, CMU, September 30, 1998
 * for attempted airborne geolocation 
 */

#include <stdio.h>
//#include "affprojsvd.h"
#include "affprojNR.h"

#define LINESIZE 256
#define MAXPOINTS 100

/*----------------------------------------------------------------------*/

//#define INFILE argv[1]
//#define AFFINE argv[2]
#define		INFILE "simple.txt"
#define		AFFINE 1 // input flag : 0 projective, otherwise affine 
#define		DEBUG 0

int main(int argc, char *argv[])
{
  FILE *in;
  int numpts, i, affine;
  float x[MAXPOINTS];
  float y[MAXPOINTS];
  float u[MAXPOINTS];
  float v[MAXPOINTS];
  char line[LINESIZE];
  float p[9];
  float xnew, ynew;
  float tmp;

  /* input flag : 0 projective, otherwise affine */
  //sscanf(AFFINE,"%d",&affine);  	
  affine = AFFINE;

  if ((in=fopen(INFILE,"rt"))==NULL) {
    printf("error opening file %s for input\n",INFILE);
    //exit(-1);
	return 0;
  }

  /* read in point correspondences u,v,x,y */
  numpts = 0;
  while(fgets(line,LINESIZE,in) != NULL) {
    sscanf(line,"%f %f %f %f",
	   u+numpts,v+numpts,
	   x+numpts,y+numpts);
    numpts++;
  }
  fclose(in);

  printf("Correspondences:\n");
  for (i=0; i < numpts; i++)
    printf(" %d: %g %g -> %g %g\n",i,u[i],v[i],x[i],y[i]);

  //fit transfrom projection
  if (affine) {
    svdfitaff(numpts, u, v, x, y, p, DEBUG);     /* affine fit */

    printf("Affine transform:\n");
    for (i=0; i < 6; i++)
      printf(" %g",p[i]);
    printf("\n");

    printf("Transformed points\n");
    for (i=0; i < numpts; i++) {
      xnew = (u[i] * p[0] + v[i] * p[1] + p[2]); 
      ynew = (u[i] * p[3] + v[i] * p[4] + p[5]); 
      printf(" %d: %g %g -> %g %g [%g %g]\n",i,
	     u[i],v[i],xnew,ynew,xnew-x[i],ynew-y[i]);
    }
  }
  else {
    svdfitproj(numpts, u, v, x, y, p, DEBUG);   /* projective fit */

    printf("Projective transform:\n");
    for (i=0; i < 9; i++)
      printf(" %g",p[i]);
    printf("\n");

    printf("Transformed points\n");
    for (i=0; i < numpts; i++) {
      tmp = u[i] * p[6] + v[i] * p[7] + p[8];
      xnew = (u[i] * p[0] + v[i] * p[1] + p[2]) / tmp; 
      ynew = (u[i] * p[3] + v[i] * p[4] + p[5]) / tmp; 
      printf(" %d: %g %g -> %g %g [%g %g]\n",i,
	     u[i],v[i],xnew,ynew,xnew-x[i],ynew-y[i]);
    }
  }
  return 1;
}

