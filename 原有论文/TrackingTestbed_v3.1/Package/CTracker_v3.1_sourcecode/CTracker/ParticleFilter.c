 /*****************************************************************************
 * File:	TemplateMatch.c
 * Desc:	functions of Template Match Tracking Algorithm
 * Author:	Xuhui Zhou  @ Carnegie Mellon University
 * Date:	05/20/2004
 *****************************************************************************/

#include "TemplateMatch.h"
#include "time.h"

IplImage	*gResultImg;		//particle filter results

#define NumOfParticles 100
double	gObjHist[3][16];
CvPoint2D64f gSamplePts[NumOfParticles];
//pure translational model (could get more fancy later by allowing bounding object rectangle to scale and rotate)
//dM is max displacement along rows, dN is max displacement along cols
double	gDY = 30; 
double	gDX = 30;
int		gTkFrameCount;

#define DEBUG 1

///////////////////////////////////////////////////////////////////////////////
void particle_TrackCleanUp()
//release memory
{
	//release image
	cvReleaseImage(&gResultImg);
}

///////////////////////////////////////////////////////////////////////////////
void particle_TrackInit(IplImage* inImage, IplImage* inMask, RECT inTargetBox)
//tracker init with box and mask
{	
	int		imgWidth, imgHeight;
	int		xi, yi;
	double  maskval;
	CvScalar scaPixel;
	int		binIdx;
	int		i,j;
	CvPoint2D64f	boxCenter;
	double	pixCount;
	double	histvals[36][36][3];
	char		filename[200];

	//check whether input is valid
	if (inImage==NULL) return;
	utl_RectCheckBound(&inTargetBox, inImage->width, inImage->height);

	imgWidth	= inImage->width;
	imgHeight	= inImage->height;

	//release memory in case re-initialization
	particle_TrackCleanUp();

	//init object histogram
	for (i=0;i<3; i++){
		for (j=0;j<16; j++){		
			gObjHist[i][j] = 0;
		}
	}
	//get object histogram
	pixCount = 0;
	for(xi=inTargetBox.left+1;xi<=inTargetBox.right;xi++){
		for(yi=inTargetBox.top+1;yi<=inTargetBox.bottom;yi++){
			//get mask value
			maskval = cvGetReal2D(inMask, yi, xi);
			if (maskval==0) continue;

			//get pixel value
			scaPixel = cvGet2D(inImage, yi, xi);			
			//loop through each channel
			for (i=0;i<3; i++){
				histvals[yi-(inTargetBox.top+1)][xi-(inTargetBox.left+1)][i] = scaPixel.val[i];
				binIdx = cvFloor(scaPixel.val[i]/16);
				gObjHist[i][binIdx]++;
			}
			//increase counter
			pixCount++;
		}
	}
	//normalize histogram
	for (i=0;i<3; i++){
		for (j=0;j<16; j++){		
			gObjHist[i][j] /= pixCount;
		}
	}

	//initialize particles around current location
	//this is a simple gaussian cluster around box center
	boxCenter.x = (inTargetBox.left + inTargetBox.right)/2.0;
	boxCenter.y = (inTargetBox.top + inTargetBox.bottom)/2.0;
	for (i=0;i<NumOfParticles; i++){
		gSamplePts[i].x = boxCenter.x + sqrt(gDX/2.5)*gaussrand();
		gSamplePts[i].y = boxCenter.y + sqrt(gDX/2.5)*gaussrand();
		////debug rand value
		//if(DEBUG>1){
		//	gSamplePts[i].x = boxCenter.x + sqrt(gDX/2.5)*0.1;
		//	gSamplePts[i].y = boxCenter.y + sqrt(gDX/2.5)*0.2;
		//}
	}

	//create result image buffer
	gResultImg = cvClone(inImage);

	//debug display
	utl_WriteBoxToImage(gResultImg, inTargetBox, cvScalar(0,255,0,0),2);	
	//for (j=0;j<NumOfParticles;j++){	
	//	utl_WriteBoxToImage_ctr(gResultImg, (int)gSamplePts[j].x, (int)gSamplePts[j].y, 1, cvScalar(0,255,0,0),1);
	//}
	//save images
	cvFlip (gResultImg, NULL,0);
	sprintf(filename, "../frame_0000.jpg",gTkFrameCount);
	cvSaveImage(filename, gResultImg);
	cvFlip (gResultImg, NULL,0);

	/* initialize random generator */
	srand ( time(NULL) );

	//set tracking frame count
	gTkFrameCount = 0;
}

//////////////////////////////////////////////////////////////////////////////
void particle_TrackNextFrame(IplImage* inImage, RECT inStartBox, TkResult *outResult)
//track one frame
{		
	int			boxWidth, boxHeight;
	int			boxHalfWidth, boxHalfHeight;	
	int			dispHalfWin;
	int			i, j, k;
	int			xi,yi;
	int			binIdx;
	//int			cumIdx;
	int			maxX, maxY;
	int			indices[NumOfParticles];	
	double		vx, vy;
	double		particleWeight[NumOfParticles];	
	double		cumWeightSum[NumOfParticles+1];
	double		sampleHist[3][16];	
	double		pixCount;
	double		bhattSsore;
	double		likelihood;
	double		sumWeight;	
	double		xprime[NumOfParticles];
	double		yprime[NumOfParticles];
	double		u1, uj;
	double		randval;	
	char		filename[200];
	RECT		sampleBox;
	RECT		targetBox;
	RECT		displayBox;
	CvScalar	scaPixel;
	CvPoint2D64f predictPts[NumOfParticles];

	//debug input
	//if(DEBUG>0){
	//	inStartBox.left		= 146-1;
	//	inStartBox.right	= 182-1;
	//	inStartBox.top		= 250-1;
	//	inStartBox.bottom	= 286-1;
	//}

	//check whether input is valid
	if (inImage==NULL){ return;}
	utl_RectCheckBound(&inStartBox, inImage->width, inImage->height);


	boxWidth	= inStartBox.right-inStartBox.left;
	boxHeight	= inStartBox.bottom-inStartBox.top;
	boxHalfWidth	= boxWidth/2;
	boxHalfHeight	= boxHeight/2;
		
	//predict particle locations by constant location assumption.. just apply gaussian noise
	sumWeight = 0;
	for (i=0;i<NumOfParticles;i++){	
		vx = sqrt(gDX)*gaussrand();
		vy = sqrt(gDY)*gaussrand();
		////debug rand value
		//if(DEBUG>1){
		//	vx = sqrt(gDX)*0.1;
		//	vy = sqrt(gDY)*0.2;
		//}
		//predict particles
		predictPts[i].x = gSamplePts[i].x + vx;
		predictPts[i].y = gSamplePts[i].y + vy;

		//get sample neighbor window
		sampleBox.left	= cvRound(predictPts[i].x - boxHalfWidth);
		sampleBox.right = cvRound(predictPts[i].x + boxHalfWidth);
		sampleBox.top	= cvRound(predictPts[i].y - boxHalfHeight);
		sampleBox.bottom = cvRound(predictPts[i].y + boxHalfHeight);
		utl_RectCheckBound(&sampleBox, inImage->width, inImage->height);

		//init object histogram
		for (j=0;j<3; j++){
			for (k=0;k<16; k++){		
				sampleHist[j][k] = 0;
			}
		}

		//Get sample histogram
		pixCount= 0;
		for(yi=sampleBox.top+1;yi<=sampleBox.bottom;yi++){
			for(xi=sampleBox.left+1;xi<=sampleBox.right;xi++){
				//get pixel value
				scaPixel = cvGet2D(inImage, yi, xi);
				//loop through each channel
				for (j=0;j<3; j++){
					binIdx = cvFloor(scaPixel.val[j]/16);
					sampleHist[j][binIdx]++;
				}
				//increase counter
				pixCount++;
			}
		}
		//normalize histogram
		for (j=0;j<3; j++){
			for (k=0;k<16; k++){		
				sampleHist[j][k] /= pixCount;
			}
		}
		//calculate weight based on likelihood function
		//which is based on bhattacharyya coeff between color distribution of init sample and current sample
		bhattSsore=0;
		for (j=0;j<3; j++){
			for (k=0;k<16; k++){		
				bhattSsore += sqrt(sampleHist[j][k]*gObjHist[j][k]);
			}
		}		
		bhattSsore /= 3;
		likelihood = exp(8*bhattSsore);
		particleWeight[i] = likelihood;
		sumWeight += particleWeight[i];
	}//end for (i=0;i<NumOfParticles;i++)

	//normalize weights	and get cummulative sum
	cumWeightSum[0] = 0;
	for (i=0;i<NumOfParticles;i++){	
		particleWeight[i] /= sumWeight;
		cumWeightSum[i+1] = cumWeightSum[i] + particleWeight[i];
	}

	//do weighted random resampling with replacement
	//likelihood of picking sample is proportional to its weight
	for (j=0;j<NumOfParticles;j++){	
		indices[j] = 0;
	}

	//more efficient version where one random sample seeds
	//a deterministically methodical sampling by 1/N
	randval = ((double)rand())/RAND_MAX;
	////debug rand value
	//if(DEBUG>1){
	//	randval = 0.1;
	//}
	i=1;
	u1 = randval/NumOfParticles;
	for (j=1;j<=NumOfParticles;j++){	
		uj = u1 + (j-1.0)/NumOfParticles;
		while (uj>cumWeightSum[i-1]){
			i=i+1;
		}
		indices[j-1] = (i-1)-1;
	}

	for (j=0;j<NumOfParticles;j++){	
		gSamplePts[j] = predictPts[indices[j]];
		xprime[j] = gSamplePts[j].x;
		yprime[j] = gSamplePts[j].y;
	}
	//get target location
	maxX = (int)fn_Round(fn_median(xprime, NumOfParticles));
	maxY = (int)fn_Round(fn_median(yprime, NumOfParticles));

	//get target rect
	targetBox.left		= (long)(maxX - boxHalfWidth);
	targetBox.right		= targetBox.left + boxWidth;
	targetBox.top		= (long)(maxY - boxHalfHeight);
	targetBox.bottom	= targetBox.top + boxHeight;		
	utl_RectCheckBound(&targetBox, inImage->width, inImage->height);	
	
	//return tracker result(half size for back compatible with Main.c)
	outResult->targetBox.left	= targetBox.left/2;
	outResult->targetBox.right	= targetBox.right/2;
	outResult->targetBox.top	= targetBox.top/2;
	outResult->targetBox.bottom	= targetBox.bottom/2;
	//get FG object mask image		
	dispHalfWin = max(boxHalfWidth, boxHalfHeight);
	displayBox.left		= (long)(maxX - 2*dispHalfWin);
	displayBox.right	= (long)(maxX + 2*dispHalfWin);
	displayBox.top		= (long)(maxY - 2*dispHalfWin);
	displayBox.bottom	= (long)(maxY + 2*dispHalfWin);
	utl_RectCheckBound(&displayBox, inImage->width, inImage->height);	

	cvCopy(inImage, gResultImg, NULL);
	utl_WriteBoxToImage(gResultImg, displayBox, cvScalar(0,255,0,0),2);
	for (j=0;j<NumOfParticles;j++){	
		utl_WriteBoxToImage_ctr(gResultImg, (int)gSamplePts[j].x, (int)gSamplePts[j].y, 1, cvScalar(0,255,0,0),1);
	}
	//outResult->FGImage		= gResultImg;
	outResult->FGMask		= NULL;

	//save images
	gTkFrameCount++;
	cvFlip (gResultImg, NULL,0);
	sprintf(filename, "../frame_%04d.jpg",gTkFrameCount);
	cvSaveImage(filename, gResultImg);
	cvFlip (gResultImg, NULL,0);

	
}

///////////////////////////////////////////////////////////////////////////////
IplImage* particle_GetParticleImg()
{
	return gResultImg;
}