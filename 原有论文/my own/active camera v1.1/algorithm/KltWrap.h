#include "Utility.h"
#include "klt/klt.h"

#include "AffineTransform/affprojNR.h"



#ifndef EXTERN_C
	#ifdef __cplusplus
		#define EXTERN_C extern "C"        
	#else
		#define EXTERN_C        
	#endif
#endif

#include "cv.hpp"
#include "highgui.h"

typedef struct  _FloatPoint{
	float x;
	float y;
}
FloatPoint;

//functions about klt tracker
EXTERN_C void klt_TrackInit(IplImage *inImage);
EXTERN_C void klt_TrackNextFrame(IplImage *inImage, float *outAffineMtx);

IplImage* klt_GetFeatureImage();
void klt_ClustShift(KLT_FeatureTable ft, int frameFrom, int frameTo, float *outAffineMtx);

//operations for affine transformation
void klt_FitAffineTransform(FloatPoint *PtsFrom, FloatPoint *PtsTo, int PtCount, float *OutTransformMtx);
float* klt_GetAffineMtx();
float klt_CalcAffineX(float inX, float inY);
float klt_CalcAffineY(float inX, float inY);
void klt_Cleanup();