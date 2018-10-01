

/* affprojNR.h  -  fitting affine and projective transforms using singular
 * value decomposition (SVD) routines from numerical recipes in C for solving
 * a set of linear equations.  Some care is taken to generate a well-conditioned
 * set of equations.  See affprojsvd.c
 *
 */


void svdfitproj(int numpts, float *U, float *V, float *X, float *Y, float *p, int debug);

void svdfitaff(int numpts, float *U, float *V, float *X, float *Y, float *p, int debug);

