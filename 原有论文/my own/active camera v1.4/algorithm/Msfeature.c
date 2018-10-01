/////////////////////////////////////////////////////////////////////////////// 
// File:	Msfeature.c
// Desc:	Log(obj/bg) Feature MeanShift Tracker
// Author:	Xuhui Zhou  @ Carnegie Mellon University
// Date:	8/31/2003
///////////////////////////////////////////////////////////////////////////////
#include "Msfeature.h"
#include <search.h>
#include <time.h>
#define FEATNUM 49				//overall feature number
#define BINNUM  32				//histogram bin number

//global variable for initialization
int		gFeatlist[FEATNUM][5];		//store feature combination list
char	gFeatlabels[FEATNUM][10];	//string label of the feature list
double  gPobjFirst[FEATNUM][BINNUM];//Obj histogram on first frame
double  gMaxFeatRatio[3][BINNUM];	//selected 3 most ratio features
double	gMaxFeatScore[3];			//scores for max ratio features
int		gMaxFeatIdx[3];				//index for max ratio features
int		gSelectFreq=10;

//global vars for tracking
float	gBoxRatio = 1.0;			//ratio between forground and background bounding box	
int		gFrameCount=0;
int		gNumOfFeat=3;		//feat number used in tracking
int		gPeakDiff;			//peak difference

IplImage	*gRatioImg;			//logratio image for meanshift purpose
IplImage	*gWeightImg;			//weight image in meanshift process	
IplImage	*gImgFgMask;
IplImage	*gImgObjMask;

float	gMaskFgRatio;
extern FILE * fp;
int compare( const void *arg1, const void *arg2 ); //function used for qsort
///////////////////////////////////////////////////////////////////////////////
void feat_TrackInit(IplImage* imgInput, IplImage* imgObjMask, RECT inTargetBox)
//init tracker
{	
	RECT	bgBox;
	int		row,col;
	CvScalar pixel;
	int		boxHalfWid, boxHalfHgt;	
	int		dmax;	
	int		i,j;
	double  feat;
	int		featbin;
	long	histObj[FEATNUM][BINNUM], histBg[FEATNUM][BINNUM];
	double  Pobj[FEATNUM][BINNUM], Pbg[FEATNUM][BINNUM], Ptotal[FEATNUM][BINNUM];
	double  logRatio[FEATNUM][BINNUM]; 
	long	countObj, countBg;
	float	boxRatio = gBoxRatio;			//ratio between forground and background bounding box	

	//cvSaveImage("img0001.bmp", imgInput);	
	gFrameCount =0;

	gRatioImg = cvCreateImage(cvSize(imgInput->width, imgInput->height),BINNUM,1);
	gWeightImg = cvCreateImage(cvSize(imgInput->width, imgInput->height),8,1);
	gImgFgMask = cvCreateImage(cvGetSize(imgInput), 8, 1);
	gImgObjMask = cvCreateImage(cvGetSize(imgInput), 8, 3);

	//generate FEATNUM feature combination list
	gencolorfeatures();

	//get outter bg bounding box 
	boxHalfWid = (int)fn_Round((inTargetBox.right - inTargetBox.left)/2);
	boxHalfHgt = (int)fn_Round((inTargetBox.bottom - inTargetBox.top)/2);
	dmax = max(boxHalfWid, boxHalfHgt);

	bgBox.left = inTargetBox.left - (int)fn_Round(dmax*boxRatio);
	bgBox.right = inTargetBox.right + (int)fn_Round(dmax*boxRatio);
	bgBox.top = inTargetBox.top - (int)fn_Round(dmax*boxRatio);
	bgBox.bottom = inTargetBox.bottom + (int)fn_Round(dmax*boxRatio);
	utl_RectCheckBound(&bgBox,imgInput->width, imgInput->height);

	//compcolorfeatures for foreground and background
	countObj = 0;
	countBg = 0;
	for(i=0;i<FEATNUM;i++){
		for(j=0;j<BINNUM;j++){	
			histObj[i][j] = 0;
			histBg[i][j] = 0;
		}
	}

	for(col=bgBox.left+1;col<=bgBox.right;col++){
		for(row=bgBox.top+1;row<=bgBox.bottom;row++){
			pixel = cvGet2D(imgInput, row, col); //pixel value is in order of B,G,R
			//judge obj or background			
			//if (col>inTargetBox.left && col<=inTargetBox.right && row>inTargetBox.top && row<=inTargetBox.bottom){
			//	if (cvGetReal2D(imgObjMask, row,col)>0){
			if (col>inTargetBox.left && col<=inTargetBox.right && row>inTargetBox.top && row<=inTargetBox.bottom && cvGetReal2D(imgObjMask, row,col)>0)
			{
				//obj pixel
				countObj++;
				//calculate feature for feature list
				for(i=0;i<FEATNUM;i++){
					feat = (int)floor((gFeatlist[i][0]*pixel.val[2]+gFeatlist[i][1]*pixel.val[1]+gFeatlist[i][2]*pixel.val[0]+gFeatlist[i][3])/gFeatlist[i][4]);
					featbin = (int)floor(feat/8);
					//object histogram
					histObj[i][featbin]++;
				}			
			}
			else{
				//background pixel
				countBg++;
				//calculate feature for feature list
				for(i=0;i<FEATNUM;i++){
					feat = (gFeatlist[i][0]*pixel.val[2]+gFeatlist[i][1]*pixel.val[1]+gFeatlist[i][2]*pixel.val[0]+gFeatlist[i][3])/gFeatlist[i][4];
					featbin = (int)floor(feat/8);
					//background histogram
					histBg[i][featbin]++;
				}
			}			
		}
	}
	
	//get Fg pixel count
	gMaskFgRatio = (float)countObj/((inTargetBox.right-inTargetBox.left)*(inTargetBox.bottom-inTargetBox.top));

	//normalize histogram and calculate log ratio
	{
		double  x[FEATNUM][BINNUM]; //logRatio
		double  xx[FEATNUM][BINNUM]; //logRatio^2
		double  Ex_obj[FEATNUM], Exx_obj[FEATNUM];
		double  Ex_bg[FEATNUM], Exx_bg[FEATNUM];
		double  Ex_tot[FEATNUM], Exx_tot[FEATNUM];
		double  var_obj[FEATNUM], var_bg[FEATNUM], var_within[FEATNUM], var_between[FEATNUM];
		double  score[FEATNUM], score_sort[FEATNUM];
		double	maxscore, maxscore2, maxscore3;
		int		maxscoreIdx, maxscore2Idx, maxscore3Idx;	//index of max score feature		
		double	*tmp;
		int		num=FEATNUM;

		for (i=0;i<FEATNUM;i++){//for each feature
			Ex_obj[i] = 0;
			Exx_obj[i] = 0;
			Ex_bg[i] = 0;
			Exx_bg[i] = 0;
			Ex_tot[i] = 0;
			Exx_tot[i] = 0;						
			for(j=0;j<BINNUM;j++){//for each histogram bin
				Pobj[i][j] = (double)histObj[i][j]/countObj;
				gPobjFirst[i][j] = Pobj[i][j]; //record the histogram of first frame into global array
				Pbg[i][j] = (double)histBg[i][j]/countBg;
				Ptotal[i][j] = (Pobj[i][j] + Pbg[i][j])/2;				
				logRatio[i][j] =  max(-7, min(7,log((Pobj[i][j]+0.001)/(Pbg[i][j]+0.001))));
				x[i][j] = logRatio[i][j];
				xx[i][j] = x[i][j]*x[i][j];
				Ex_obj[i] = Ex_obj[i] + x[i][j]*Pobj[i][j];
				Exx_obj[i] = Exx_obj[i] + xx[i][j]*Pobj[i][j];
				Ex_bg[i] = Ex_bg[i] + x[i][j]*Pbg[i][j];
				Exx_bg[i] = Exx_bg[i] + xx[i][j]*Pbg[i][j];
				Ex_tot[i] = Ex_tot[i] + x[i][j]*Ptotal[i][j];
				Exx_tot[i] = Exx_tot[i] + xx[i][j]*Ptotal[i][j];
			}
		}

		//calculate variation score and find max feature index
		maxscoreIdx = 0;
		for (i=0;i<FEATNUM;i++){
			var_obj[i] = Exx_obj[i] - Ex_obj[i]*Ex_obj[i];
			var_bg[i] = Exx_bg[i] - Ex_bg[i]*Ex_bg[i];
			var_between[i] = Exx_tot[i] - Ex_tot[i]*Ex_tot[i];
			var_within[i] = (var_obj[i] + var_bg[i])/2;
			score[i] = var_between[i] / max(var_within[i], 1e-6);
			score_sort[i] = score[i];
			if (i==0){
				maxscore = score[i];
				maxscoreIdx = i;
			}			
			else{			
				if(score[i]>maxscore){
					maxscore = score[i];
					maxscoreIdx = i;
				}
			}
		}		

		//get the second max score
		qsort(score_sort, (size_t)FEATNUM, sizeof(double), compare);
		maxscore2 = score_sort[47];	
		maxscore3 = score_sort[46];
		tmp = _lfind(&maxscore2, score, &num, sizeof(double), compare);
		maxscore2Idx = tmp-score;	
		tmp = _lfind(&maxscore3, score, &num, sizeof(double), compare);
		maxscore3Idx = tmp-score;
	
		//record results into global variables for tracking purpose
		gMaxFeatIdx[0] = maxscoreIdx;
		gMaxFeatIdx[1] = maxscore2Idx;
		gMaxFeatIdx[2] = maxscore3Idx;
		gMaxFeatScore[0] = maxscore;
		gMaxFeatScore[1] = maxscore2;
		gMaxFeatScore[2] = maxscore3;

		//record the logratio of max score feature
		for(j=0;j<BINNUM;j++){//for each histogram bin
			gMaxFeatRatio[0][j] = logRatio[maxscoreIdx][j];
			gMaxFeatRatio[1][j] = logRatio[maxscore2Idx][j];
			gMaxFeatRatio[2][j] = logRatio[maxscore3Idx][j];
		}		
	}//end block

	//get weight image for display purpose
	{
		int featIdx;
		double maxRatio;
		double ratio;		

		featIdx  = gMaxFeatIdx[0];		//max feature index in featlist;
		maxRatio = gMaxFeatScore[0];	//max feature ratio score		
		
		//get ratio image
		cvSetZero(gWeightImg);
		for(col=bgBox.left;col<bgBox.right;col++){
			for(row=bgBox.top;row<bgBox.bottom;row++){
				pixel = cvGet2D(imgInput, row, col); //pixel value is in order of B,G,R
				//calculate feature 
				feat = (gFeatlist[featIdx][0]*pixel.val[2]+gFeatlist[featIdx][1]*pixel.val[1]+gFeatlist[featIdx][2]*pixel.val[0]+gFeatlist[featIdx][3])/gFeatlist[featIdx][4];
				featbin = (int)floor(feat/8);
				ratio = gMaxFeatRatio[0][featbin];
				//set weight image
				pixel.val[0] = (int)(255*ratio/maxRatio);
				cvSet2D(gWeightImg, row, col, pixel);
			}	
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void feat_TrackInit_PeakDiff(IplImage* imgInput, IplImage* imgObjMask, RECT inTargetBox)
//init tracker
{	
	RECT	bgBox;
	int		row,col;
	CvScalar pixel;
	int		boxHalfWid, boxHalfHgt;	
	int		dmax;	
	int		i,j;
	double  feat;
	int		featbin;
	long	histObj[FEATNUM][BINNUM], histBg[FEATNUM][BINNUM];
	double  Pobj[FEATNUM][BINNUM], Pbg[FEATNUM][BINNUM];
	//double	Ptotal[FEATNUM][BINNUM];
	double  logRatio[FEATNUM][BINNUM]; 
	long	countObj, countBg;
	float	boxRatio = gBoxRatio;			//ratio between forground and background bounding box		

	//cvSaveImage("img0001.bmp", imgInput);	
	gFrameCount =0;

	gRatioImg = cvCreateImage(cvSize(imgInput->width, imgInput->height),32,1); //float image
	gWeightImg = cvCreateImage(cvSize(imgInput->width, imgInput->height),8,1);
	gImgFgMask = cvCreateImage(cvGetSize(imgInput), 8, 1);
	gImgObjMask = cvCreateImage(cvGetSize(imgInput), 8, 3);

	//generate FEATNUM feature combination list
	gencolorfeatures();

	//get outter bg bounding box 
	boxHalfWid = (int)fn_Round((inTargetBox.right - inTargetBox.left)/2);
	boxHalfHgt = (int)fn_Round((inTargetBox.bottom - inTargetBox.top)/2);
	dmax = max(boxHalfWid, boxHalfHgt);

	bgBox.left = inTargetBox.left - (int)fn_Round(dmax*boxRatio);
	bgBox.right = inTargetBox.right + (int)fn_Round(dmax*boxRatio);
	bgBox.top = inTargetBox.top - (int)fn_Round(dmax*boxRatio);
	bgBox.bottom = inTargetBox.bottom + (int)fn_Round(dmax*boxRatio);
	utl_RectCheckBound(&bgBox,imgInput->width, imgInput->height);

	//compcolorfeatures for foreground and background
	countObj = 0;
	countBg = 0;
	for(i=0;i<FEATNUM;i++){
		for(j=0;j<BINNUM;j++){	
			histObj[i][j] = 0;
			histBg[i][j] = 0;
		}
	}

	for(col=bgBox.left+1;col<=bgBox.right;col++){
		for(row=bgBox.top+1;row<=bgBox.bottom;row++){
			pixel = cvGet2D(imgInput, row, col); //pixel value is in order of B,G,R
			//judge obj or background			
			//if (col>inTargetBox.left && col<=inTargetBox.right && row>inTargetBox.top && row<=inTargetBox.bottom){
			//	if (cvGetReal2D(imgObjMask, row,col)>0){
			if (col>inTargetBox.left && col<=inTargetBox.right && row>inTargetBox.top && row<=inTargetBox.bottom && cvGetReal2D(imgObjMask, row,col)>0)
			{
				//obj pixel
				countObj++;
				//calculate feature for feature list
				for(i=0;i<FEATNUM;i++){
					feat = (int)floor((double)(gFeatlist[i][0]*pixel.val[2]+gFeatlist[i][1]*pixel.val[1]+gFeatlist[i][2]*pixel.val[0]+gFeatlist[i][3])/gFeatlist[i][4]);
					featbin = (int)floor(feat/8);
					//object histogram
					histObj[i][featbin]++;
				}				
			}
			else{
				//background pixel
				countBg++;
				//calculate feature for feature list
				for(i=0;i<FEATNUM;i++){
					feat = (gFeatlist[i][0]*pixel.val[2]+gFeatlist[i][1]*pixel.val[1]+gFeatlist[i][2]*pixel.val[0]+gFeatlist[i][3])/gFeatlist[i][4];
					featbin = (int)floor(feat/8);
					//background histogram
					histBg[i][featbin]++;
				}
			}			
		}
	}

	//get Fg pixel count
	gMaskFgRatio = (float)countObj/((inTargetBox.right-inTargetBox.left)*(inTargetBox.bottom-inTargetBox.top));

	//normalize histogram and calculate log ratio	
	for (i=0;i<FEATNUM;i++){//for each feature
		for(j=0;j<BINNUM;j++){//for each histogram bin
			Pobj[i][j] = (double)histObj[i][j]/countObj;
			gPobjFirst[i][j] = Pobj[i][j]; //record the histogram of first frame into global array
			Pbg[i][j] = (double)histBg[i][j]/countBg;
			logRatio[i][j] =  max(-7, min(7,log((Pobj[i][j]+0.001)/(Pbg[i][j]+0.001))));
		}
	}	

	//get max peak diff index 
	{
		int featIdx;	
		int maxPeakDiffIdx;
		double ratio;
//		CvPoint	peak;
		CvPoint secondPeak;
		double maxVal, maxVal2;
		int		convWidth, convHeight;
		double minratio, maxratio;			
		double scale;
		double shift;
		CvRect roi;
		int	 peakDiff, maxPeakDiff;
		IplImage *imgWeight;
		IplImage *imgBuffer;
		POINT	objCenter;
		
		roi.x = bgBox.left;
		roi.y = bgBox.top;
		roi.width = bgBox.right-bgBox.left;
		roi.height = bgBox.bottom-bgBox.top;
		cvSetZero(gWeightImg);
		cvSetImageROI(imgInput, roi);
		cvSetImageROI(gRatioImg, roi);
		cvSetImageROI(gWeightImg, roi);
		cvSetImageROI(imgObjMask, roi);		
		cvNot(imgObjMask, imgObjMask);

		//get obj center
		objCenter.x = roi.width/2;
		objCenter.y = roi.height/2;

		//loop through featurelist to get maximum peak diff
		maxPeakDiff = 0;
		maxPeakDiffIdx = 0;
		convWidth = ((int)((inTargetBox.right - inTargetBox.left)/2))*2+1;
		convHeight = ((int)((inTargetBox.bottom - inTargetBox.top)/2))*2+1;

		imgWeight = cvClone(gWeightImg);
		imgBuffer = cvClone(gWeightImg);		
		for (featIdx=0;featIdx<FEATNUM;featIdx++){//for each feature
			//get ratio image
			cvSetZero(gRatioImg);
			for(col=0;col<roi.width;col++){
				for(row=0;row<roi.height;row++){
					pixel = cvGet2D(imgInput, row, col); //pixel value is in order of B,G,R
					//calculate feature 
					feat = (gFeatlist[featIdx][0]*pixel.val[2]+gFeatlist[featIdx][1]*pixel.val[1]+gFeatlist[featIdx][2]*pixel.val[0]+gFeatlist[featIdx][3])/gFeatlist[featIdx][4];
					featbin = (int)floor(feat/8);
					ratio = logRatio[featIdx][featbin];
					//set weight image					
					pixel.val[0] = ratio;
					cvSet2D(gRatioImg, row, col, pixel);
				}	
			}

			//get weight image by normalizing ratio image	
			cvSetZero(imgWeight);
			cvMinMaxLoc(gRatioImg, &minratio, &maxratio, NULL, NULL, 0);									
			shift = 0;
			if (maxratio<=minratio){
				scale = 1;
			}
			else{
				//scale = 255.0/(maxratio-minratio);
				//shift = -minratio*scale;
				scale = 255.0/(fn_Abs(maxratio));			
			}
			cvConvertScale(gRatioImg, imgWeight, scale, shift);
			//cvSaveImage("c:\\CTracker\\imgWeight.bmp", imgWeight);
			
			//mask out obj hole + Gaussian twice			
			//smooth with Gaussian			
			cvSmooth(imgWeight, imgBuffer, CV_GAUSSIAN, convWidth, convHeight,0,0);
			//cvSaveImage("c:\\CTracker\\imgBuffer_conv.bmp", imgBuffer);

			//get peak location;			
			//cvMinMaxLoc(imgBuffer, NULL, &maxVal, NULL, &peak, NULL);
			maxVal = cvGetReal2D(imgBuffer, objCenter.y, objCenter.x);
			//if (abs(peak.x-objCenter.x)>convWidth/2 || abs(peak.y-objCenter.y)>convHeight/2) continue;
			
			//mask out peak neighbor area		
			cvAnd(imgWeight, imgObjMask, imgBuffer, NULL);
			//cvSaveImage("c:\\CTracker\\imgBuffer_And.bmp", imgBuffer);

			cvSmooth(imgBuffer, imgBuffer, CV_GAUSSIAN, convWidth, convHeight,0,0);
			//cvSaveImage("c:\\CTracker\\imgBuffer_conv2.bmp", imgBuffer);

			//get second peak location;			
			cvMinMaxLoc(imgBuffer, NULL, &maxVal2, NULL, &secondPeak, imgObjMask);						

			peakDiff = (int)(maxVal - maxVal2);
			if (peakDiff > maxPeakDiff){
				maxPeakDiff = peakDiff;
				maxPeakDiffIdx = featIdx;
				//get weight image for display purpose
				cvCopy(imgWeight, gWeightImg,NULL);
				//cvSaveImage("c:\\CTracker\\gWeightImg_out.bmp", gWeightImg);
			}
		}//end for loop

		//record results into global variables for tracking purpose
		gMaxFeatIdx[0] = maxPeakDiffIdx;
		gPeakDiff = maxPeakDiff;

		//record the logratio of max score feature
		for(j=0;j<BINNUM;j++){//for each histogram bin
			gMaxFeatRatio[0][j] = logRatio[maxPeakDiffIdx][j];
		}

		//release memory
		cvRelease(&imgWeight);
		cvRelease(&imgBuffer);

		//reset Roi
		cvResetImageROI(imgInput);
		cvResetImageROI(imgObjMask);
		cvResetImageROI(gRatioImg);
		cvResetImageROI(gWeightImg);		
	}//end block of PeakDiff
}

///////////////////////////////////////////////////////////////////////////////
void feat_ReselectFeature(IplImage* imgInput, RECT inTargetBox, IplImage* imgFgMask)
//init tracker
{	
	RECT	bgBox;
	int		row,col;
	CvScalar pixel;
	int		boxHalfWid, boxHalfHgt;	
	int		dmax;	
	int		i,j;
	double  feat;
	int		featbin;
	long	histObj[FEATNUM][BINNUM], histBg[FEATNUM][BINNUM];
	double  Pobj[FEATNUM][BINNUM], Pbg[FEATNUM][BINNUM], Ptotal[FEATNUM][BINNUM];
	double  logRatio[FEATNUM][BINNUM]; 
	long	countObj, countBg;
	float	boxRatio = gBoxRatio;			//ratio between forground and background bounding box	

	//cvSaveImage("img0001.bmp", imgInput);	
	gFrameCount =0;

	//get outter bg bounding box 
	boxHalfWid = (int)fn_Round((inTargetBox.right - inTargetBox.left)/2);
	boxHalfHgt = (int)fn_Round((inTargetBox.bottom - inTargetBox.top)/2);
	dmax = max(boxHalfWid, boxHalfHgt);

	bgBox.left = inTargetBox.left - (int)fn_Round(dmax*boxRatio);
	bgBox.right = inTargetBox.right + (int)fn_Round(dmax*boxRatio);
	bgBox.top = inTargetBox.top - (int)fn_Round(dmax*boxRatio);
	bgBox.bottom = inTargetBox.bottom + (int)fn_Round(dmax*boxRatio);
	utl_RectCheckBound(&bgBox,imgInput->width, imgInput->height);

	//compcolorfeatures for foreground and background
	countObj = 0;
	countBg = 0;
	for(i=0;i<FEATNUM;i++){
		for(j=0;j<BINNUM;j++){	
			histObj[i][j] = 0;
			histBg[i][j] = 0;
		}
	}

	for(col=bgBox.left+1;col<=bgBox.right;col++){
		for(row=bgBox.top+1;row<=bgBox.bottom;row++){
			pixel = cvGet2D(imgInput, row, col); //pixel value is in order of B,G,R
			//judge obj or background			
			if (col>inTargetBox.left && col<=inTargetBox.right && row>inTargetBox.top && row<=inTargetBox.bottom){
				if (cvGetReal2D(imgFgMask, row,col)>20){
				//if(1){
					//obj pixel
					countObj++;
					//calculate feature for feature list
					for(i=0;i<FEATNUM;i++){
						feat = (int)floor((double)(gFeatlist[i][0]*pixel.val[2]+gFeatlist[i][1]*pixel.val[1]+gFeatlist[i][2]*pixel.val[0]+gFeatlist[i][3])/gFeatlist[i][4]);
						featbin = (int)floor(feat/8);
						//object histogram
						histObj[i][featbin]++;
					}
				}
			}
			else{
				//background pixel
				countBg++;
				//calculate feature for feature list
				for(i=0;i<FEATNUM;i++){
					feat = (gFeatlist[i][0]*pixel.val[2]+gFeatlist[i][1]*pixel.val[1]+gFeatlist[i][2]*pixel.val[0]+gFeatlist[i][3])/gFeatlist[i][4];
					featbin = (int)floor(feat/8);
					//background histogram
					histBg[i][featbin]++;
				}
			}			
		}
	}

	//normalize histogram and calculate log ratio
	{
		double  x[FEATNUM][BINNUM]; //logRatio
		double  xx[FEATNUM][BINNUM]; //logRatio^2
		double  Ex_obj[FEATNUM], Exx_obj[FEATNUM];
		double  Ex_bg[FEATNUM], Exx_bg[FEATNUM];
		double  Ex_tot[FEATNUM], Exx_tot[FEATNUM];
		double  var_obj[FEATNUM], var_bg[FEATNUM], var_within[FEATNUM], var_between[FEATNUM];
		double  score[FEATNUM], score_sort[FEATNUM];
		double	maxscore, maxscore2, maxscore3;
		int		maxscoreIdx, maxscore2Idx, maxscore3Idx;	//index of max score feature		
		double	*tmp;
		int		num=FEATNUM;

		for (i=0;i<FEATNUM;i++){//for each feature
			Ex_obj[i] = 0;
			Exx_obj[i] = 0;
			Ex_bg[i] = 0;
			Exx_bg[i] = 0;
			Ex_tot[i] = 0;
			Exx_tot[i] = 0;						
			for(j=0;j<BINNUM;j++){//for each histogram bin
				Pobj[i][j] = (double)histObj[i][j]/countObj;
				Pobj[i][j] = (gPobjFirst[i][j]+Pobj[i][j])/2;				
				Pbg[i][j] = (double)histBg[i][j]/countBg;
				Ptotal[i][j] = (Pobj[i][j] + Pbg[i][j])/2;
				logRatio[i][j] =  max(-7, min(7,log((Pobj[i][j]+0.001)/(Pbg[i][j]+0.001))));
				x[i][j] = logRatio[i][j];
				xx[i][j] = x[i][j]*x[i][j];
				Ex_obj[i] = Ex_obj[i] + x[i][j]*Pobj[i][j];
				Exx_obj[i] = Exx_obj[i] + xx[i][j]*Pobj[i][j];
				Ex_bg[i] = Ex_bg[i] + x[i][j]*Pbg[i][j];
				Exx_bg[i] = Exx_bg[i] + xx[i][j]*Pbg[i][j];
				Ex_tot[i] = Ex_tot[i] + x[i][j]*Ptotal[i][j];
				Exx_tot[i] = Exx_tot[i] + xx[i][j]*Ptotal[i][j];
			}
		}

		//calculate variation score and find max feature index
		maxscoreIdx = 0;
		for (i=0;i<FEATNUM;i++){
			var_obj[i] = Exx_obj[i] - Ex_obj[i]*Ex_obj[i];
			var_bg[i] = Exx_bg[i] - Ex_bg[i]*Ex_bg[i];
			var_between[i] = Exx_tot[i] - Ex_tot[i]*Ex_tot[i];
			var_within[i] = (var_obj[i] + var_bg[i])/2;
			score[i] = var_between[i] / max(var_within[i], 1e-6);
			score_sort[i] = score[i];
			if (i==0){
				maxscore = score[i];
				maxscoreIdx = i;
			}			
			else{			
				if(score[i]>maxscore){
					maxscore = score[i];
					maxscoreIdx = i;
				}
			}
		}		

		//get the second max score
		qsort(score_sort, (size_t)FEATNUM, sizeof(double), compare);
		maxscore2 = score_sort[47];	
		maxscore3 = score_sort[46];
		tmp = _lfind(&maxscore2, score, &num, sizeof(double), compare);
		maxscore2Idx = tmp-score;	
		tmp = _lfind(&maxscore3, score, &num, sizeof(double), compare);
		maxscore3Idx = tmp-score;
	
		//record results into global variables for tracking purpose
		gMaxFeatIdx[0] = maxscoreIdx;
		gMaxFeatIdx[1] = maxscore2Idx;
		gMaxFeatIdx[2] = maxscore3Idx;
		gMaxFeatScore[0] = maxscore;
		gMaxFeatScore[1] = maxscore2;
		gMaxFeatScore[2] = maxscore3;

		//record the logratio of max score feature
		for(j=0;j<BINNUM;j++){//for each histogram bin
			gMaxFeatRatio[0][j] = logRatio[maxscoreIdx][j];
			gMaxFeatRatio[1][j] = logRatio[maxscore2Idx][j];
			gMaxFeatRatio[2][j] = logRatio[maxscore3Idx][j];
		}		
	}//end block
}

///////////////////////////////////////////////////////////////////////////////
void feat_ReselectFeature_PeakDiff(IplImage* imgInput, RECT inTargetBox, IplImage* imgObjMask)
//init tracker
{	
	RECT	bgBox;
	int		row,col;
	CvScalar pixel;
	int		boxHalfWid, boxHalfHgt;	
	int		dmax;	
	int		i,j;
	double  feat;
	int		featbin;
	long	histObj[FEATNUM][BINNUM], histBg[FEATNUM][BINNUM];
	double  Pobj[FEATNUM][BINNUM], Pbg[FEATNUM][BINNUM];
	//double  Ptotal[FEATNUM][BINNUM];
	double  logRatio[FEATNUM][BINNUM]; 
	long	countObj, countBg;
	float	boxRatio = gBoxRatio;			//ratio between forground and background bounding box	

	//cvSaveImage("img0001.bmp", imgInput);	
	gFrameCount =0;

	//get outter bg bounding box 
	boxHalfWid = (int)fn_Round((inTargetBox.right - inTargetBox.left)/2);
	boxHalfHgt = (int)fn_Round((inTargetBox.bottom - inTargetBox.top)/2);
	dmax = max(boxHalfWid, boxHalfHgt);

	bgBox.left = inTargetBox.left - (int)fn_Round(dmax*boxRatio);
	bgBox.right = inTargetBox.right + (int)fn_Round(dmax*boxRatio);
	bgBox.top = inTargetBox.top - (int)fn_Round(dmax*boxRatio);
	bgBox.bottom = inTargetBox.bottom + (int)fn_Round(dmax*boxRatio);
	utl_RectCheckBound(&bgBox,imgInput->width, imgInput->height);

	//compcolorfeatures for foreground and background
	countObj = 0;
	countBg = 0;
	for(i=0;i<FEATNUM;i++){
		for(j=0;j<BINNUM;j++){	
			histObj[i][j] = 0;
			histBg[i][j] = 0;
		}
	}

	for(col=bgBox.left+1;col<=bgBox.right;col++){
		for(row=bgBox.top+1;row<=bgBox.bottom;row++){
			pixel = cvGet2D(imgInput, row, col); //pixel value is in order of B,G,R
			//judge obj or background			
			if (col>inTargetBox.left && col<=inTargetBox.right && row>inTargetBox.top && row<=inTargetBox.bottom){
				if (cvGetReal2D(imgObjMask, row,col)>20){				
					//obj pixel
					countObj++;
					//calculate feature for feature list
					for(i=0;i<FEATNUM;i++){
						feat = (int)floor((double)(gFeatlist[i][0]*pixel.val[2]+gFeatlist[i][1]*pixel.val[1]+gFeatlist[i][2]*pixel.val[0]+gFeatlist[i][3])/gFeatlist[i][4]);
						featbin = (int)floor(feat/8);
						//object histogram
						histObj[i][featbin]++;
					}
				}
			}
			else{
				//background pixel
				countBg++;
				//calculate feature for feature list
				for(i=0;i<FEATNUM;i++){
					feat = (gFeatlist[i][0]*pixel.val[2]+gFeatlist[i][1]*pixel.val[1]+gFeatlist[i][2]*pixel.val[0]+gFeatlist[i][3])/gFeatlist[i][4];
					featbin = (int)floor(feat/8);
					//background histogram
					histBg[i][featbin]++;
				}
			}			
		}
	}

	//normalize histogram and calculate log ratio	
	for (i=0;i<FEATNUM;i++){//for each feature
		for(j=0;j<BINNUM;j++){//for each histogram bin
			Pobj[i][j] = (double)histObj[i][j]/countObj;
			gPobjFirst[i][j] = Pobj[i][j]; //record the histogram of first frame into global array
			Pbg[i][j] = (double)histBg[i][j]/countBg;
			logRatio[i][j] =  max(-7, min(7,log((Pobj[i][j]+0.001)/(Pbg[i][j]+0.001))));
		}
	}	

	//get max peak diff index 
	{
		int featIdx;	
		int maxPeakDiffIdx;
		double ratio;
		CvPoint	peak, secondPeak;
		double maxVal, maxVal2;
		int		convWidth, convHeight;
		double minratio, maxratio;			
		double scale;
		double shift;
		CvRect roi;
		int	 peakDiff, maxPeakDiff;
		IplImage *imgWeight;
		IplImage *imgBuffer;
		IplImage *imgTmp;

		roi.x = bgBox.left;
		roi.y = bgBox.top;
		roi.width = bgBox.right-bgBox.left;
		roi.height = bgBox.bottom-bgBox.top;
		cvSetImageROI(imgInput, roi);
		cvSetImageROI(gRatioImg, roi);
		cvSetImageROI(gWeightImg, roi);
		cvSetImageROI(imgObjMask, roi);		
		cvNot(imgObjMask, imgObjMask);

		//loop through featurelist to get maximum peak diff
		maxPeakDiff = 0;
		maxPeakDiffIdx = 0;
		convWidth = ((int)((inTargetBox.right - inTargetBox.left)/2))*2+1;
		convHeight = ((int)((inTargetBox.bottom - inTargetBox.top)/2))*2+1;

		imgWeight = cvClone(gWeightImg);
		imgBuffer = cvClone(gWeightImg);	
		imgTmp	  = cvClone(gWeightImg);		

		for (featIdx=0;featIdx<FEATNUM;featIdx++){//for each feature			
			if (featIdx == gMaxFeatIdx[0]) continue;
			//get ratio image
			cvSetZero(gRatioImg);
			for(col=0;col<roi.width;col++){
				for(row=0;row<roi.height;row++){
					pixel = cvGet2D(imgInput, row, col); //pixel value is in order of B,G,R
					//calculate feature 
					feat = (gFeatlist[featIdx][0]*pixel.val[2]+gFeatlist[featIdx][1]*pixel.val[1]+gFeatlist[featIdx][2]*pixel.val[0]+gFeatlist[featIdx][3])/gFeatlist[featIdx][4];
					featbin = (int)floor(feat/8);
					ratio = logRatio[featIdx][featbin];
					//set weight image					
					pixel.val[0] = ratio;
					cvSet2D(gRatioImg, row, col, pixel);
				}	
			}

			//get weight image by normalizing ratio image	
			cvSetZero(imgWeight);
			cvMinMaxLoc(gRatioImg, &minratio, &maxratio, NULL, NULL, 0);									
			shift = 0;
			if (maxratio<=minratio){
				scale = 1;
			}
			else{
				//scale = 255.0/(maxratio-minratio);
				//shift = -minratio*scale;
				scale = 255.0/(fn_Abs(maxratio));			
			}
			cvConvertScale(gRatioImg, imgWeight, scale, shift);
			//cvSaveImage("c:\\CTracker\\code\\imgWeight.bmp", imgWeight);
			
			//mask out obj hole + Gaussian twice			
			//smooth with Gaussian			
			cvSmooth(imgWeight, imgBuffer, CV_GAUSSIAN, convWidth, convHeight,0,0);
			//cvSaveImage("c:\\CTracker\\code\\imgBuffer_conv.bmp", imgBuffer);

			//get peak location;			
			cvMinMaxLoc(imgBuffer, NULL, &maxVal, NULL, &peak, NULL);
			
			//mask out peak neighbor area		
			cvAnd(imgWeight, imgObjMask, imgBuffer, NULL);
			//cvSaveImage("c:\\CTracker\\code\\imgBuffer_And.bmp", imgBuffer);

			cvSmooth(imgBuffer, imgBuffer, CV_GAUSSIAN, convWidth, convHeight,0,0);
			//cvSaveImage("c:\\CTracker\\code\\imgBuffer_conv2.bmp", imgBuffer);

			//get second peak location;			
			cvMinMaxLoc(imgBuffer, NULL, &maxVal2, NULL, &secondPeak, imgObjMask);						

			peakDiff = (int)fn_Abs(maxVal - maxVal2);
			if (peakDiff > maxPeakDiff){
				maxPeakDiff = peakDiff;
				maxPeakDiffIdx = featIdx;
				//get weight image for display purpose
				cvCopy(imgWeight, imgTmp,NULL);
				//cvSaveImage("c:\\CTracker\\code\\gWeightImg_out.bmp", gWeightImg);
			}
		}//end for loop

		//record weight image
		cvCopy(imgTmp, gWeightImg, NULL);

		//record results into global variables for tracking purpose
		gMaxFeatIdx[0] = maxPeakDiffIdx;
		gPeakDiff = maxPeakDiff;

		//record the logratio of max score feature
		for(j=0;j<BINNUM;j++){//for each histogram bin
			gMaxFeatRatio[0][j] = logRatio[maxPeakDiffIdx][j];
		}

		//release memory
		cvRelease(&imgWeight);
		cvRelease(&imgBuffer);
		cvRelease(&imgTmp);

		//reset Roi
		cvResetImageROI(imgInput);
		cvResetImageROI(imgObjMask);
		cvResetImageROI(gRatioImg);
		cvResetImageROI(gWeightImg);		
	}//end block of PeakDiff
}

///////////////////////////////////////////////////////////////////////////////
void feat_TrackNextFrame(IplImage* inImg, RECT inStartBox, TkResult *outResult)
//track one frame
{
	int row, col;
	CvScalar pixel;
	double	ratio;
	double  feat;
	int		featbin;
	int		i;
	double	dnewx, dnewy; //new subpixel x,y after meanshift
	int		newx, newy;
	int		featIdx;
	int		ratioIdx;
	double  maxscore;
	int		xstart, ystart;
	int		halfwid, halfhgt;
	double	candx[3], candy[3];
	int		numOfFeat;		//feat number used in tracking
	//cvSaveImage("img0002.bmp", inImg);		
	RECT	bgBox;

	//get outter bg bounding box 
	{
		int boxHalfWid, boxHalfHgt;
		int dmax;
		float boxRatio = gBoxRatio;
		
		boxHalfWid = (int)fn_Round((inStartBox.right - inStartBox.left)/2);
		boxHalfHgt = (int)fn_Round((inStartBox.bottom - inStartBox.top)/2);
		dmax = max(boxHalfWid, boxHalfHgt);

		bgBox.left = inStartBox.left - (int)fn_Round(dmax*boxRatio);
		bgBox.right = inStartBox.right + (int)fn_Round(dmax*boxRatio);
		bgBox.top = inStartBox.top - (int)fn_Round(dmax*boxRatio);
		bgBox.bottom = inStartBox.bottom + (int)fn_Round(dmax*boxRatio);
		utl_RectCheckBound(&bgBox,inImg->width, inImg->height);
	}


	//loop over top 3 featus
	numOfFeat=gNumOfFeat;
	//numOfFeat=1;
	cvSetZero(gRatioImg);
	cvSetZero(gWeightImg);

	
	for(i=0;i<numOfFeat;i++){
//		i = 1; //second max
		featIdx  = gMaxFeatIdx[i];		//max feature index in featlist;
		maxscore = gMaxFeatScore[i];	//max feature ratio score
		ratioIdx = i;					//log ratio index;
		
		
		//get ratio image
		for(col=bgBox.left;col<bgBox.right;col++){
			for(row=bgBox.top;row<bgBox.bottom;row++){
				pixel = cvGet2D(inImg, row, col); //pixel value is in order of B,G,R
				//calculate feature 
				feat = (gFeatlist[featIdx][0]*pixel.val[2]+gFeatlist[featIdx][1]*pixel.val[1]+gFeatlist[featIdx][2]*pixel.val[0]+gFeatlist[featIdx][3])/gFeatlist[featIdx][4];
				featbin = (int)floor(feat/8);
				ratio = gMaxFeatRatio[ratioIdx][featbin];
				//set ratio image for meanshift purpose
				pixel.val[0] = ratio;
				cvSet2D(gRatioImg, row, col, pixel);								
			}	
		}
		
		
		//utl_WriteImage(gRatioImg, "ratioimg.txt");
		
		//meanshift ratio image		
		xstart = (int)fn_Round(((double)inStartBox.left+inStartBox.right)/2);
		ystart = (int)fn_Round(((double)inStartBox.top+inStartBox.bottom)/2);
		halfwid = (int)fn_Round(((double)inStartBox.right - inStartBox.left)/2);
		halfhgt = (int)fn_Round(((double)inStartBox.bottom - inStartBox.top)/2);
		
		meanshift(gRatioImg, xstart, ystart, halfwid, halfhgt, 1.0, &dnewx, &dnewy);		

		candx[i] = dnewx;
		candy[i] = dnewy;

		//get weight image by normalizing ratio image
		if (i==0)//first feature
		{
			double minratio, maxratio;			
			double scale, shift;
			CvRect roi;
			roi.x = bgBox.left;
			roi.y = bgBox.top;
			roi.width = bgBox.right-bgBox.left;
			roi.height = bgBox.bottom-bgBox.top;
			cvSetImageROI(gRatioImg, roi);
			cvSetImageROI(gWeightImg, roi);		
			cvMinMaxLoc(gRatioImg, &minratio, &maxratio, NULL, NULL, 0);			
			shift = 0;
			if (maxratio<=minratio){
				scale = 1;
			}
			else{
				//scale = 255.0/(maxratio-minratio);
				//shift = -minratio*scale;
				scale = 255.0/(fn_Abs(maxratio));			
			}
			cvConvertScale(gRatioImg, gWeightImg, scale, shift);
			cvResetImageROI(gRatioImg);
			cvResetImageROI(gWeightImg);
		}
	}
	

	//get median from candidate x,y
	newx = (int)fn_median(candx, numOfFeat);
	newy = (int)fn_median(candy, numOfFeat);

	//return tracking result
	{
		RECT targetBox;
		int  boxWidth, boxHeight;
		int  boxHalfWidth, boxHalfHeight;
		float score;
		float fgRatio;

		//get input box dimension
		boxWidth	= inStartBox.right-inStartBox.left;
		boxHeight	= inStartBox.bottom-inStartBox.top;
		boxHalfWidth	= boxWidth/2;
		boxHalfHeight	= boxHeight/2;

		//get target rect
		targetBox.left		= (long)(newx - boxHalfWidth);
		targetBox.right		= targetBox.left + boxWidth;
		targetBox.top		= (long)(newy - boxHalfHeight);
		targetBox.bottom	= targetBox.top + boxHeight;	
		
		targetBox.left		= max(targetBox.left,0);
		targetBox.top		= max(targetBox.top,0);
		targetBox.right		= max(targetBox.right,1);		
		targetBox.bottom	= max(targetBox.bottom,1);
		targetBox.right		= min(targetBox.right,inImg->width-1);
		targetBox.bottom	= min(targetBox.bottom,inImg->height-1);
		targetBox.left		= min(targetBox.left,inImg->width-2);
		targetBox.top		= min(targetBox.top,inImg->height-2);

		//get occlusion score
		fgRatio = feat_CountFGPixel(gWeightImg, targetBox);
		score	= fgRatio/ gMaskFgRatio;
		score	= min(1, score);

		//get FG object mask image
		{
			CvRect  roi;	
			cvSetZero(gImgFgMask);	
			cvSetZero(gImgObjMask);	
			roi.x = targetBox.left;
			roi.y = targetBox.top;
			roi.width = targetBox.right-targetBox.left;
			roi.height = targetBox.bottom-targetBox.top;
			cvSetImageROI(gWeightImg, roi);
			cvSetImageROI(gImgFgMask, roi);		
			cvSetImageROI(inImg, roi);
			cvSetImageROI(gImgObjMask, roi);		
			cvThreshold(gWeightImg, gImgFgMask, 0, 255, CV_THRESH_BINARY);
			cvCopy(inImg, gImgObjMask, gImgFgMask);
			cvResetImageROI(gWeightImg);
			cvResetImageROI(gImgFgMask);
			cvResetImageROI(inImg);		
			cvResetImageROI(gImgObjMask);	
		}	
		outResult->FGMask	= gImgFgMask;
		outResult->ObjMask	= gImgObjMask;
		
		//return tracker result
		outResult->targetBox	= targetBox;
		outResult->FGImage		= gWeightImg;
		outResult->score		= score;
		outResult->occlusion	= (score<0.6? TRUE:FALSE);	
	}
}

///////////////////////////////////////////////////////////////////////////////
void feat_TrackNextFrame_Adapt(IplImage* inImg, RECT inStartBox, TkResult *outResult)
//track one frame
{
	//search target on this frame
	feat_TrackNextFrame(inImg, inStartBox, outResult);

	//count number of frames tracked
	gFrameCount = (++gFrameCount)%gSelectFreq;

	//reselect features for adaptive tracking
	if (gFrameCount==0){
		feat_ReselectFeature(inImg, outResult->targetBox, gWeightImg);
	}
}

///////////////////////////////////////////////////////////////////////////////
void feat_TrackNextFrame_PeakDiff(IplImage* inImg, RECT inStartBox, TkResult *outResult)
//track one frame
{
	int row, col;
	CvScalar pixel;
	double	ratio;
	double  feat;
	int		featbin;
	int		i;
	double	dnewx, dnewy; //new subpixel x,y after meanshift
	int		newx, newy;
	int		featIdx;
	int		ratioIdx;
	double  maxscore;
	int		xstart, ystart;
	int		halfwid, halfhgt;
	double	candx[3], candy[3];
	int		numOfFeat;		//feat number used in tracking
	//cvSaveImage("img0002.bmp", inImg);		
	RECT	bgBox;

	//get outter bg bounding box 
	{
		int boxHalfWid, boxHalfHgt;
		int dmax;
		float boxRatio = gBoxRatio;
		
		boxHalfWid = (int)fn_Round((inStartBox.right - inStartBox.left)/2);
		boxHalfHgt = (int)fn_Round((inStartBox.bottom - inStartBox.top)/2);
		dmax = max(boxHalfWid, boxHalfHgt);

		bgBox.left = inStartBox.left - (int)fn_Round(dmax*boxRatio);
		bgBox.right = inStartBox.right + (int)fn_Round(dmax*boxRatio);
		bgBox.top = inStartBox.top - (int)fn_Round(dmax*boxRatio);
		bgBox.bottom = inStartBox.bottom + (int)fn_Round(dmax*boxRatio);
		utl_RectCheckBound(&bgBox,inImg->width, inImg->height);
	}


	//loop over max PeakDiff featus
	numOfFeat=1;
	cvSetZero(gRatioImg);
	cvSetZero(gWeightImg);
	for(i=0;i<numOfFeat;i++){
//		i = 1; //second max
		featIdx  = gMaxFeatIdx[i];		//max feature index in featlist;
		maxscore = gMaxFeatScore[i];	//max feature ratio score
		ratioIdx = i;					//log ratio index;
		
		//get ratio image
		for(col=bgBox.left;col<bgBox.right;col++){
			for(row=bgBox.top;row<bgBox.bottom;row++){
				pixel = cvGet2D(inImg, row, col); //pixel value is in order of B,G,R
				//calculate feature 
				feat = (gFeatlist[featIdx][0]*pixel.val[2]+gFeatlist[featIdx][1]*pixel.val[1]+gFeatlist[featIdx][2]*pixel.val[0]+gFeatlist[featIdx][3])/gFeatlist[featIdx][4];
				featbin = (int)floor(feat/8);
				ratio = gMaxFeatRatio[ratioIdx][featbin];
				//set ratio image for meanshift purpose
				pixel.val[0] = ratio;
				cvSet2D(gRatioImg, row, col, pixel);								
			}	
		}
		//utl_WriteImage(gRatioImg, "ratioimg.txt");

		//meanshift ratio image		
		xstart = (int)fn_Round(((double)inStartBox.left+inStartBox.right)/2);
		ystart = (int)fn_Round(((double)inStartBox.top+inStartBox.bottom)/2);
		halfwid = (int)fn_Round(((double)inStartBox.right - inStartBox.left)/2);
		halfhgt = (int)fn_Round(((double)inStartBox.bottom - inStartBox.top)/2);

		meanshift(gRatioImg, xstart, ystart, halfwid, halfhgt, 1.0, &dnewx, &dnewy);		
		candx[i] = dnewx;
		candy[i] = dnewy;

		//get weight image by normalizing ratio image
		if (i==0)//first feature
		{
			double minratio, maxratio;			
			double scale, shift;
			CvRect roi;
			roi.x = bgBox.left;
			roi.y = bgBox.top;
			roi.width = bgBox.right-bgBox.left;
			roi.height = bgBox.bottom-bgBox.top;
			cvSetImageROI(gRatioImg, roi);
			cvSetImageROI(gWeightImg, roi);		
			cvMinMaxLoc(gRatioImg, &minratio, &maxratio, NULL, NULL, 0);						
			shift = 0;
			if (maxratio<=minratio){
				scale = 1;
			}
			else{
				//scale = 255.0/(maxratio-minratio);
				//shift = -minratio*scale;
				scale = 255.0/(fn_Abs(maxratio));			
			}
			cvConvertScale(gRatioImg, gWeightImg, scale, shift);
			cvResetImageROI(gRatioImg);
			cvResetImageROI(gWeightImg);
		}
	}

	//get median from candidate x,y
	newx = (int)fn_median(candx, numOfFeat);
	newy = (int)fn_median(candy, numOfFeat);

	//return tracking result
	{
		RECT targetBox;
		int  boxWidth, boxHeight;
		int  boxHalfWidth, boxHalfHeight;
		float score;
		float fgRatio;

		//get input box dimension
		boxWidth	= inStartBox.right-inStartBox.left;
		boxHeight	= inStartBox.bottom-inStartBox.top;
		boxHalfWidth	= boxWidth/2;
		boxHalfHeight	= boxHeight/2;

		//get target rect
		targetBox.left		= (long)(newx - boxHalfWidth);
		targetBox.right		= targetBox.left + boxWidth;
		targetBox.top		= (long)(newy - boxHalfHeight);
		targetBox.bottom	= targetBox.top + boxHeight;	
		
		targetBox.left		= max(targetBox.left,0);
		targetBox.top		= max(targetBox.top,0);
		targetBox.right		= max(targetBox.right,1);		
		targetBox.bottom	= max(targetBox.bottom,1);
		targetBox.right		= min(targetBox.right,inImg->width-1);		
		targetBox.bottom	= min(targetBox.bottom,inImg->height-1);
		targetBox.left		= min(targetBox.left,inImg->width-2);
		targetBox.top		= min(targetBox.top,inImg->height-2);

		//get occlusion score
		fgRatio = feat_CountFGPixel(gWeightImg, targetBox);
		score	= fgRatio/ gMaskFgRatio;
		score	= min(1, score);

		//get FG object mask image
		{
			CvRect  roi;	
			cvSetZero(gImgFgMask);	
			cvSetZero(gImgObjMask);	
			roi.x = targetBox.left;
			roi.y = targetBox.top;
			roi.width = targetBox.right-targetBox.left;
			roi.height = targetBox.bottom-targetBox.top;
			cvSetImageROI(gWeightImg, roi);
			cvSetImageROI(gImgFgMask, roi);		
			cvSetImageROI(inImg, roi);
			cvSetImageROI(gImgObjMask, roi);		
			cvThreshold(gWeightImg, gImgFgMask, 0, 255, CV_THRESH_BINARY);
			cvCopy(inImg, gImgObjMask, gImgFgMask);
			cvResetImageROI(gWeightImg);
			cvResetImageROI(gImgFgMask);
			cvResetImageROI(inImg);		
			cvResetImageROI(gImgObjMask);	
		}	
		outResult->FGMask	= gImgFgMask;
		outResult->ObjMask	= gImgObjMask;

		//return tracker result
		outResult->targetBox	= targetBox;
		outResult->FGImage		= gWeightImg;
		outResult->score		= score;
		outResult->occlusion	= (score<0.6? TRUE:FALSE);	
	}
}

///////////////////////////////////////////////////////////////////////////////
void feat_TrackNextFrame_PeakDiff_Adapt(IplImage* inImg, RECT inStartBox, TkResult *outResult)
//track one frame with adaption
{
	//track one frame
	feat_TrackNextFrame_PeakDiff(inImg, inStartBox, outResult);	

	//count number of frames tracked
	gFrameCount = (++gFrameCount)%gSelectFreq;

	//reselect features for adaptive tracking
	if (gFrameCount==0)
	{
		feat_ReselectFeature_PeakDiff(inImg, outResult->targetBox, outResult->FGImage);
	}
}

///////////////////////////////////////////////////////////////////////////////
void meanshift(IplImage *imgInput, int xstart, int ystart, int hx, int hy, double eps, double *modex, double *modey)
//% example of mode-seeking using the mean-shift algorithm.
//% This simplified mean-shift routine works by sampling weights
//% within a rectangular window, computing the center of mass,
//% shifting the rectangle to that center of mass, and s on, 
//% until convergence (rectangle moves less than epsilon).  
//% Note, th algorithm converges to a local maximum.  It must 
//% be run multiple times all over the matrix to find all maxima, 
//% if the global maximum is needed.  Alternatively, you can find
//% clusters by keeping track of all the local maxima, and the
//% initialization points that led to those local maxima.
//%
//% function [modex, modey] = meanshift(im, xstart, ystart, hx, hy, eps)
//%   im - 2D array containing a distribution or weights
//%   xstart - col value of initial point
//%   ystart - row value of initial point
//%   hx - half width of rectanglar data window (determines your resolution)
//%   hy - half height of data window
//%   eps - less than this step size means convergence
//% output
//%   modex - subpixel col value of mode
//%   modey - subpixel row value of mode
{
	double	xcur = xstart;
	double	ycur = ystart;
	double	eps2 = eps*eps;
	double	dist2 = eps2+1;
	int		ntimes = 0;
	double	denom, sumx, sumy;
	int		dx, dy, col, row;
	double  incx, incy;
	double	pixelval;

	//loop until shift dist below eps or times greater than threshold
	while (dist2>eps2 && ntimes<10) {
		//loop around target candidate window
		denom = 0;
		sumx = 0;
		sumy = 0;
		for(dx=-hx;dx<=hx;dx++){
			for(dy=-hy;dy<=hy;dy++){
				col = (int)fn_Round(dx+xcur);
				row = (int)fn_Round(dy+ycur);
				//judge if pixel is out of boundary
				if (col<0 || col>=imgInput->width ||row<0||row>=imgInput->height){
					continue;
				}
				pixelval = cvGetReal2D(imgInput, row, col);
				denom = denom + fn_Abs(pixelval);
				sumx = sumx + dx*pixelval;
				sumy = sumy + dy*pixelval;
			}		
		}
		
		//if denom is zero, target out of boundary
		if (denom==0){
			*modex = xcur;
			*modey = ycur;
			return;
		}
		incx = sumx/denom;
		incy = sumy/denom;
		
		xcur = fn_Round(xcur+incx);
		ycur = fn_Round(ycur+incy);

		dist2 = incx*incx + incy*incy;
		ntimes++;
	}

	//return result
	*modex = xcur;
	*modey = ycur;
}

///////////////////////////////////////////////////////////////////////////////
void feat_TrackCleanUp()
//clean tracker memory
{
	cvReleaseImage(&gRatioImg);	
	cvReleaseImage(&gWeightImg);
	cvReleaseImage(&gImgFgMask);
	cvReleaseImage(&gImgObjMask);
}

///////////////////////////////////////////////////////////////////////////////
void gencolorfeatures()
//generate FEATNUM features list
//output:	gFeatlist[FEATNUM][5]	//global variable to store feature value
//			gFeatlabels[FEATNUM][10]	//glabal variable to store feature label
{
	int r, g, b;
	int featr, featg, featb;
	int tmpr, tmpg, tmpb;
	int	i, j, k, idx; //loop index
	int alphavals[5] = {0, 1, -1, 2, -2};
	int minval, maxval, sumval;
	int sumabs, sumneg;	
	int okflag;
	int featNum;	
//	double denum;
//	double uveclist[FEATNUM][3];	
	char strLabel[10];	
	char letters[3] = {'R','G', 'B'};
	int	 first;
	char strtmp[3];
	
	int	 featlist[FEATNUM][5];
	char  featlabels[FEATNUM][10];

	//get feature list
	featNum = 0;
	for(i=0;i<5;i++){
		for(j=0;j<5;j++){
			for(k=0;k<5;k++){
				r = alphavals[i];
				g = alphavals[j];
				b = alphavals[k];
				if (r*r+g*g+b*b>0){
					minval = utl_Min3(r,g,b);
					maxval = utl_Max3(r,g,b);
					sumval = r+g+b;
					if (abs(minval) > abs(maxval)){
						r = -r;
						g = -g;
						b = -b;
					}
					else if(sumval<0){
						r = -r;
						g = -g;
						b = -b;
					}
					okflag = 1;
					//test if this feature parallel with exist features
					for (idx=0;idx<featNum;idx++){
						featr = featlist[idx][0];
						featg = featlist[idx][1];
						featb = featlist[idx][2];
						tmpr = g*featb - b*featg;
						tmpg = b*featr - r*featb;
						tmpb = r*featg - g*featr;
						if (tmpr*tmpr+tmpg*tmpg+tmpb*tmpb==0){
							okflag = 0;
							break;
						}
					}
					if (okflag ==1){						
						sumabs = abs(r)+abs(g)+abs(b);
						sumneg = 0;
						if (r<0) sumneg = sumneg +r;
						if (g<0) sumneg = sumneg +g;
						if (b<0) sumneg = sumneg +b;						
	
						featlist[featNum][0] = r;
						featlist[featNum][1] = g;
						featlist[featNum][2] = b;
						featlist[featNum][3] = -256*sumneg;
						featlist[featNum][4] = sumabs;

//						denum = sqrt(r*r+g*g+b*b);
//						uveclist[featNum][0] = r/denum;
//						uveclist[featNum][1] = g/denum;
//						uveclist[featNum][2] = b/denum;
						featNum = featNum+1;
					}
				}
			}//end b
		}//end g
	}//end r

	for (i=0;i<featNum;i++){
		strcpy(strLabel,"");		
		first = 1;		
		for (j=0;j<3;j++){
			if (featlist[i][j]!=0){
				if (!first && featlist[i][j]>0){				
					strcat(strLabel, "+");
				}
				if (featlist[i][j]<0){				
					strcat(strLabel, "-");
				}
				
				if (abs(featlist[i][j])==1){
					wsprintf(strtmp, "%c", letters[j]);
					strcat(strLabel, strtmp);
				}
				else{		
					wsprintf(strtmp, "%d%c",  abs(featlist[i][j]),letters[j]);
					strcat(strLabel, strtmp);
				}
				first = 0;
			}
		}
		strcpy(featlabels[i],strLabel);
	}

	//return to global variable
	for (i=0;i<featNum;i++){
		for(j=0;j<5;j++){
			gFeatlist[i][j] = featlist[i][j];
		}
		strcpy(gFeatlabels[i], featlabels[i]);
	}
	
}

///////////////////////////////////////////////////////////////////////////////
int compare( const void *arg1, const void *arg2 )
//function used in qsort and _lfind
{
   /* Compare two args: */
   double num1 = *(double*)arg1;
   double num2 = *(double*)arg2;

   if(num1> num2){ return 1;}
   else{
	   if(num1<num2) return -1;
	   else return 0;   
   }
}

///////////////////////////////////////////////////////////////////////////////
char* feat_GetFeatLabel()
{
	return	gFeatlabels[gMaxFeatIdx[0]];	//string label of the max ratio feature
}

///////////////////////////////////////////////////////////////////////////////
IplImage* feat_GetWeightImage()
{
	return	gWeightImg;	//string label of the max ratio feature
}

///////////////////////////////////////////////////////////////////////////////
float feat_CountFGPixel(IplImage *inImg, RECT inRect)
//cout foreground pixel number to find score
{
	int pixCount;
	float score;
	CvRect crectRoi;	

	//count foreground points
	crectRoi.x = inRect.left;
	crectRoi.y = inRect.top;
	crectRoi.width = inRect.right - inRect.left;
	crectRoi.height = inRect.bottom - inRect.top;
	cvSetImageROI(inImg, crectRoi);
	pixCount = cvCountNonZero(inImg);
	inImg->roi = NULL;
	
	score = (float)pixCount/(crectRoi.width*crectRoi.height);			

	return score;
}

