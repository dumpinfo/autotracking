              /*****************************************************************************
 * File:	KltWrap.c
 * Desc:	Wrapping function of KLT Tracker
 * Author:	Xuhui Zhou  @ Carnegie Mellon University
 * Date:	12/01/2003
 *****************************************************************************/

#include "KltWrap.h"

#include "Utility.h"
#include "AffineTransform/affprojNR.h"

//define klt track variables
#define		NFeatures 300
#define 	NFrames 2	
KLT_TrackingContext tc;
KLT_FeatureList		fl;
KLT_FeatureTable	ft;
KLT_FeatureTable	ftBg;

IplImage*	kltImages[NFrames]; //track images
IplImage*	kltImgFeature;		//feature image
IplImage*	kltImgFG;		//foreground features
IplImage*	kltImgBG;		//foreground features

int			kltWidth, kltHeight;
int			kltFrameIdx; //frame index
int			kltTrackCount=0;
float		gAffineMtx[6];

///////////////////////////////////////////////////////////////////////////////
void klt_TrackInit(IplImage *inImage){
	int i;

	//image dimension
	kltWidth	= inImage->width;
	kltHeight	= inImage->height;

	//release memeory in case re-init
	klt_Cleanup();

	//initialize klt track variables	
	tc = KLTCreateTrackingContext();
	tc->mindist = 10;
	tc->nSkippedPixels = 2;
	tc->sequentialMode = TRUE;
	tc->window_width = 7;
	tc->window_height = 7;
	tc->max_iterations = 5;
	KLTChangeTCPyramid(tc, 4);
	KLTUpdateTCBorder(tc);

	fl = KLTCreateFeatureList(NFeatures);
	ft = KLTCreateFeatureTable(NFrames, NFeatures);
	ftBg = KLTCreateFeatureTable(NFrames, NFeatures);
		
	//create klt images
	for(i=0;i<NFrames;i++){
		kltImages[i]	= cvCreateImage(cvSize(kltWidth, kltHeight), 8, 1);
	}
	kltImgFeature = cvCreateImage(cvSize(kltWidth, kltHeight), 8, 3);
	kltImgFG = cvCreateImage(cvSize(kltWidth, kltHeight), 8, 1);
	kltImgBG = cvCreateImage(cvSize(kltWidth, kltHeight), 8, 1);

	//get Green Channel
	kltFrameIdx = 0;
	cvCvtPixToPlane(inImage, NULL, kltImages[kltFrameIdx], NULL, NULL);
		
	//select features
	KLTSelectGoodFeatures(tc, kltImages[kltFrameIdx]->imageData, kltWidth, kltHeight, fl);
	KLTStoreFeatureList(fl, ft, kltFrameIdx);
}
///////////////////////////////////////////////////////////////////////////////
void klt_TrackNextFrame(IplImage *inImage, float *outAffineMtx)
{
	int		iFrameIdxPrev;

	//increase frame index
	iFrameIdxPrev = kltFrameIdx;
	kltFrameIdx = (kltFrameIdx+1)%NFrames;

	//get Green Channel
	cvCvtPixToPlane(inImage, NULL, kltImages[kltFrameIdx], NULL, NULL);	

	//track features	
	KLTTrackFeatures(tc,kltImages[iFrameIdxPrev]->imageData,kltImages[kltFrameIdx]->imageData,kltWidth,kltHeight,fl);
	KLTStoreFeatureList(fl, ft, kltFrameIdx);

	//calculate translation flow
	klt_ClustShift(ft, iFrameIdxPrev, kltFrameIdx, outAffineMtx);

	//replace features
	kltTrackCount++;

	//replace features 
	kltTrackCount=0;
	KLTReplaceLostFeatures(tc, kltImages[kltFrameIdx]->imageData,kltWidth,kltHeight, fl);
	KLTStoreFeatureList(fl, ft, kltFrameIdx);

	//dilate feature points	
	cvDilate(kltImgFG, kltImgFG, NULL, 1);
	cvDilate(kltImgBG, kltImgBG, NULL, 1);
	
	//copy to feature image
	cvCvtPlaneToPix(kltImgFG, kltImages[kltFrameIdx], kltImgBG, NULL, kltImgFeature);	
}

///////////////////////////////////////////////////////////////////////////////
void klt_ClustShift(KLT_FeatureTable ft, int frameFrom, int frameTo, float *outAffineMtx)
{
	//calculate translation flow
	int validCount;
	int feat, i,j;
	long offset;		
	FloatPoint Diff[NFeatures], p1[NFeatures], p2[NFeatures];
	FloatPoint BgPtsFrom[NFeatures], BgPtsTo[NFeatures];
	double sqrDist[NFeatures];	
	int ival;
	double med, lmed;
	int   fgCount;
	float dx=0, dy=0;
	float avgDx=0, avgDy=0;
	double threshold;
	double dist2;
	int bgCount;
	float sumx, sumy;

	//get valid feature list tracked and their translation
	validCount=0;
	for (feat = 0 ; feat < ft->nFeatures ; feat++)
	{
		//if (ft->feature[feat][1]->val == 0 && ft->feature[feat][0]->val == 0) {	
		if (ft->feature[feat][kltFrameIdx]->val == 0) {				
				
				p1[validCount].x = ft->feature[feat][frameFrom]->x;
				p1[validCount].y = ft->feature[feat][frameFrom]->y;
				p2[validCount].x = ft->feature[feat][frameTo]->x;
				p2[validCount].y = ft->feature[feat][frameTo]->y;
				
				Diff[validCount].x = p2[validCount].x  - p1[validCount].x;
				Diff[validCount].y = p2[validCount].y  - p1[validCount].y;
				validCount++;
		}
	}

	//use least median of sqares to find the largest cluster of traslations
	ival = -1;
	for (i=0;i<validCount;i+=10)
	{
		for(j=0;j<validCount;j++){
			sqrDist[j] = (Diff[j].x-Diff[i].x)*(Diff[j].x- Diff[i].x) + (Diff[j].y-Diff[i].y)*(Diff[j].y-Diff[i].y);
		}
		med = fn_median(sqrDist, validCount);
		if (i==0 || med<lmed ){
			lmed = med;
			ival = i;
		}
	}

	dx = Diff[ival].x; //background translation
	dy = Diff[ival].y; 

	//get background points and average translation
	//background are pixels not consistent with most "popular" translation  r= +/- 3 sigma -> r^2 = 4 sigma, which is roughly 16*lmed			
	//threshold = lmed;
	threshold = 16*lmed;	
	bgCount = 0;
	fgCount = 0;
	sumx = 0;
	sumy = 0;

	cvSetZero(kltImgFG);
	cvSetZero(kltImgBG);
	for (i=0;i<validCount;i++){
		dist2 = (Diff[i].x-dx)*(Diff[i].x-dx) + (Diff[i].y-dy)*(Diff[i].y-dy);
		if (dist2 <= threshold)
		{//background
			//summary for the calculation of average
			sumx = sumx + Diff[i].x; 
			sumy = sumy + Diff[i].y;		
			
			//update background image
			offset = kltWidth*(int)fn_Round(p2[i].y)+(int)fn_Round(p2[i].x);
			*(kltImgBG->imageDataOrigin+offset) = (BYTE)255;			

			//update background points array
			BgPtsFrom[bgCount].x = p1[i].x;
			BgPtsFrom[bgCount].y = p1[i].y;
			BgPtsTo[bgCount].x = p2[i].x;
			BgPtsTo[bgCount].y = p2[i].y;			
			
			//record this point in global variable			
			ftBg->feature[bgCount][0]->x   = p1[i].x;
			ftBg->feature[bgCount][0]->y   = p1[i].y;
			ftBg->feature[bgCount][1]->x   = p2[i].x;
			ftBg->feature[bgCount][1]->y   = p2[i].y;

			//increase counter
			bgCount++;
		}
		//else{//foreground
			//update feature image								
			//offset = kltWidth*(int)fn_Round(p2[i].y)+(int)fn_Round(p2[i].x);
 			//*(kltImgFG->imageDataOrigin+offset) = (BYTE)255;			
		//}
	}
	ftBg->nFeatures = bgCount;

	//get average movement
	if (bgCount!=0){
		avgDx = sumx/bgCount;
		avgDy = sumy/bgCount;			
	}		

	//Fit affine transform
	klt_FitAffineTransform(BgPtsFrom, BgPtsTo, bgCount, outAffineMtx);
}

///////////////////////////////////////////////////////////////////////////////
IplImage* klt_GetFeatureImage()
{
	return kltImgFeature;
}

/////////////////////////////////////////////////////////////////////////////
void klt_FitAffineTransform(FloatPoint *PtsFrom, FloatPoint *PtsTo, int PtCount, float *OutAffineMtx)
{
	float u[NFeatures]; //array to hold from_x;
	float v[NFeatures]; //array to hold from_y;
	float x[NFeatures]; //array to hold to_x;
	float y[NFeatures]; //array to hold to_y;
	int	  debug = 0;
	int		i;

	for(i=0;i<PtCount;i++){
		u[i] = PtsFrom[i].x;
		v[i] = PtsFrom[i].y;
		x[i] = PtsTo[i].x;
		y[i] = PtsTo[i].y;
	}

	/* affine fit */
	svdfitaff(PtCount, u, v, x, y, OutAffineMtx, debug);  
	
	//update global affine matrix for Calculate pixel(X,Y) transformation
	for (i=0;i<6;i++){	
		gAffineMtx[i] = OutAffineMtx[i];
	}
}

///////////////////////////////////////////////////////////////////////////////
float* klt_GetAffineMtx()
{
	return gAffineMtx;
}

///////////////////////////////////////////////////////////////////////////////
float klt_CalcAffineX(float inX, float inY)
{
	return (inX * gAffineMtx[0] + inY * gAffineMtx[1] + gAffineMtx[2]);
	//return (inX * gAffineMtx[0] + gAffineMtx[2]);
}

///////////////////////////////////////////////////////////////////////////////
float klt_CalcAffineY(float inX, float inY)
{	
	return (inX * gAffineMtx[3] + inY * gAffineMtx[4] + gAffineMtx[5]);
	//return (inY * gAffineMtx[4] + gAffineMtx[5]);
}

void klt_Cleanup()
//release memeory
{
	int i;
		
	for (i=0;i<NFrames;i++){
		cvReleaseImage(&kltImages[i]);	
	}
	cvReleaseImage(&kltImgFeature);
	cvReleaseImage(&kltImgFG);
	cvReleaseImage(&kltImgBG);

	KLTFreeFeatureTable(ft);
	KLTFreeFeatureTable(ftBg);
	KLTFreeFeatureList(fl);	
	KLTFreeTrackingContext(tc);
}