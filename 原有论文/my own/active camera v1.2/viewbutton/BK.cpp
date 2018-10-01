// BK.cpp: implementation of the BK class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BK.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Define global constants


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BK::BK()
{
/*
  weight = 1/K;
  for (register int i=0; i<3; i++) mean[i] = 128;
  std = STD;
*/
	label = 0;
}

BK::~BK()
{

}
// Find least probable gaussian and replace
void BK::least(int rgb[3]) { 
  // ptr is a pointer to the first of K gaussians on the heap
  BK * ptr = this;
  // use pointer p to point to the next gaussian iteratively
  BK * p = this;
  double least = p->weight/p->std;
  for (register int i=1; i<K; i++) {
    p++;
    // if w/s of the next gaussian is smaller, then pointer ptr to it
    if ( p->weight/p->std < least )
      ptr = p;
  }

  double sum=1+WEIGHT-ptr->weight;

  // ptr now points to the lowest w/s gaussian.  Replace gaussian values
  for (i=0; i<3; i++) ptr->mean[i] = rgb[i];
  ptr->std = STD;
  ptr->weight = WEIGHT;

  //normalize weights
  for(i=0,p=this;i<K;i++,p++)p->weight=p->weight/sum;
}

// Adjust weights of gaussians
void BK::adjWeights( BK * pMtch) {
  // Pointer p points to first of K gaussians
  BK * p = this;
  // For each K gaussian, update weight according to wether it matches
  for (register int i=0; i<K; i++) {
    if ( p == pMtch ) {
      p->weight = (1 - ALPHA)*p->weight + ALPHA;
    } else {
      p->weight = (1 - ALPHA)*p->weight;
    }
    p++;
  }
}

// Calculate PDF and modify matched gaussian
  void BK::adjMeanStd(int *rgb) {
  // array to hold vector RGB value
  double v[3];
  // find the distance between the rgb vector and the mean vector
  for (register int i=0; i<3; i++) v[i] = rgb[i] - mean[i];
  double dist =  (v[0]*v[0] + v[1]*v[1] + v[2]*v[2])/3;
 
   //original GMM 
 /* double mah = dist/std;
  // (2*pi)^3/2 * covariance
  double d = 15.7496 * sqrt(pow(std*std,3));
  double e = exp(-.5 * mah);
  // alpha * PDF
  double p = ALPHA * e / d;
*/
  double p = ALPHA;  //my GMM
  double pm = 1 - p;
  // calculate the new mean for each rgb of the matched gaussian
  for (i=0; i<3; i++) mean[i] = pm * mean[i] + p * rgb[i];
  // calculate the standard deviation of gaussian which is assumed the same for each RGB
  std = sqrt(pm * (std*std) + p * dist);
  std = std >= 6 ? std : 6;
}

// Sort gaussians and determine if guassian is background
bool BK::bgd(BK * pMtch) {


  BK * p1 = this;
  bool match = 0;
  double sum = 0;

  // after sorting gaussians, determine if matched gaussian is background
  // loop through each gaussian until sum > T, if matched gaussian is found then 
  // it is a bacgkround gaussian.
  for (register int i=0; i<K; i++) {
    if (p1 == pMtch) match=1;
    sum = sum + p1->weight;
    if ( sum > T && match==1) return 1;
    if ( sum > T && match==0) return 0;
    p1++;
  }
  return 0;
}

 void BK::sortGauss(void)
  {
	  // Sort gaussians: use p1 and p2 as pointers to manipulate gaussians
	  BK * p1 = this;
	  BK * p2;
	  BK ptmp;
	  
	  for (register int j=0; j<K-1; j++) {
		  p2=p1;
		  for (register int i=1; i<K-j; i++) {
			  p2++;
			  if ( p1->weight/p1->std < p2->weight/p2->std ) {
				  ptmp = *p1;
				  *p1 = *p2;
				  *p2 = ptmp;
			  }
		  }
		  p1++;
	  }
	  
  }
