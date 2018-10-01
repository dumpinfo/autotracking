/////////////////////////////////////////////////////////////////////////////// 
// File:	EMShift.c
// Desc:	EM-like meanshift Tracker
// Author:	Xuhui Zhou  @ Carnegie Mellon University
// Date:	11/23/2003
///////////////////////////////////////////////////////////////////////////////
#include "emshift.h"
#include "msfeature.h"
#include <float.h>

#define PTNUM 180
#define PI 3.14159265358979

///////////////////////////////////////////////////////////////////////////////
void ems_TrackInit(IplImage* imgInput, IplImage* imgObjMask, RECT inTargetBox)
{	
	//use PeakDiff feature tracker to initialzie
	//feat_TrackInit_PeakDiff(imgInput, imgObjMask, inTargetBox);
	feat_TrackInit(imgInput, imgObjMask, inTargetBox);
	//initialize global parameters V,
}

///////////////////////////////////////////////////////////////////////////////
void ems_TrackNextFrame(IplImage* inImg, RECT inStartBox, TkResult *outResult)
{		
	IplImage *imgWeight;
	double	eps;
	double	beta;
	int		loopcount;
	int		mx0, my0;
	double	v[4], inv[4], v2[4]; //2*2 covariance matrix
	CvMat	*matV, *matInv, *matEVec, *matEVal;
	int		boxWidth, boxHeight;
	int		borderext;
	int		xi, yi, dx, dy;
	RECT	bgBox;
	double  numerx,numery,denom, nterm;
	double  numerv[4];
	double  wi;
	int		mx2, my2;		
	double  dv, maxdt, maxdv;
	int		i, j;
	long	count;
	double  eigenVec[4], eigenVal[2];
	double  angle;
	int		axisA, axisB;

	//use PeakDiff feature tracker to get weight image
	//feat_TrackNextFrame_PeakDiff_Adapt(inImg, inStartBox, outResult);
	feat_TrackNextFrame_Adapt(inImg, inStartBox, outResult);

	//imgWeight = cvClone(outResult->FGImage);	
	imgWeight = outResult->FGImage;	
	//cvMul(imgWeight, imgWeight, imgWeight, 1);
	cvThreshold(imgWeight, imgWeight, 80, 0, CV_THRESH_TOZERO);
	//cvSaveImage("c:\\CTracker\\imgWeight.bmp", imgWeight);

	//get obj size
	mx0 = (int)((inStartBox.left + inStartBox.right)/2);
	my0 = (int)((inStartBox.top + inStartBox.bottom)/2);
	boxWidth = inStartBox.right - inStartBox.left;
	boxHeight = inStartBox.bottom - inStartBox.top;

	//get Bg neighboring box
	bgBox.left = inStartBox.left;
	bgBox.right = inStartBox.right;
	bgBox.top = inStartBox.top;
	bgBox.bottom = inStartBox.bottom;
	borderext = (int)max(boxWidth/2, boxHeight/2);
	utl_RectSizeIncrease(&bgBox, borderext, imgWeight->width, imgWeight->height);	

	//initialize parameters
	eps = 1;
	beta = 2;
	v[0] = boxWidth*boxWidth/16.0;
	v[1] = 0;
	v[2] = 0;
	v[3] = boxHeight*boxHeight/16.0;
	
	matV = cvCreateMat(2,2,CV_32FC1);
	matInv = cvCreateMat(2,2,CV_32FC1);
	matEVec = cvCreateMat(2,2,CV_32FC1);
	matEVal = cvCreateMat(2,1,CV_32FC1);
	cvSetReal2D(matV, 0,0, v[0]);
	cvSetReal2D(matV, 0,1, v[1]);
	cvSetReal2D(matV, 1,0, v[2]);
	cvSetReal2D(matV, 1,1, v[3]);

	//EM steps recursively
	loopcount = 0;
	while (loopcount<30)
	{		
		numerx = 0;
		numery = 0;
		numerv[0] = 0;
		numerv[1] = 0;
		numerv[2] = 0;
		numerv[3] = 0;
		denom = 0;
		cvInvert(matV, matInv, 0);
		inv[0] = cvGetReal2D(matInv,0,0);
		inv[1] = cvGetReal2D(matInv,0,1);
		inv[2] = cvGetReal2D(matInv,1,0);
		inv[3] = cvGetReal2D(matInv,1,1);

		//calulation for each non-zero points
		count = 0;
		for (xi=0;xi<imgWeight->width;xi++){
			for (yi=0;yi<imgWeight->height;yi++){				
				wi = cvGetReal2D(imgWeight, yi, xi);
				if (wi==0) continue;
				dx = xi-mx0;
				dy = yi-my0;
				nterm = exp(-1.0*(inv[0]*dx*dx + (inv[1]+inv[2])*dx*dy + inv[3]*dy*dy)/2);
				numerx = numerx + wi*nterm*xi;
				numery = numery + wi*nterm*yi;
				numerv[0] = numerv[0] + wi*nterm*dx*dx;
				numerv[1] = numerv[1] + wi*nterm*dx*dy;
				numerv[2] = numerv[1];
				numerv[3] = numerv[3] + wi*nterm*dy*dy;
				denom = denom + wi*nterm;
				count++;
			}
		}
		mx2 = (int)fn_Round(numerx/denom);
		my2 = (int)fn_Round(numery/denom);
		v2[0] = beta * numerv[0] /denom;
		v2[1] = beta * numerv[1] /denom;
		v2[2] = beta * numerv[2] /denom;
		v2[3] = beta * numerv[3] /denom;
        
		maxdt = max(mx2-mx0, my2-my0);
		maxdv = 0;
		for (i=0;i<4; i++){
			dv = fabs(v2[i] - v[i]);
			if (dv >maxdv) maxdv = dv;
		}

		//update obj pos and shape
		mx0 = mx2;
		my0 = my2;
		v[0] = v2[0];
		v[1] = v2[1];
		v[2] = v2[2];
		v[3] = v2[3];
		cvSetReal2D(matV, 0,0, v[0]);
		cvSetReal2D(matV, 0,1, v[1]);
		cvSetReal2D(matV, 1,0, v[2]);
		cvSetReal2D(matV, 1,1, v[3]);			
		loopcount++;

		//stop loop if converge
		if (maxdt<1 && maxdv <eps*eps) break;				
	}

	cvEigenVV(matV, matEVec, matEVal, DBL_EPSILON);

	for (i=0;i<2;i++){
		eigenVal[i] = cvGetReal2D(matEVal,i,0);
		for(j=0;j<2;j++){
			eigenVec[2*i+j] = cvGetReal2D(matEVec,i,j);			
		}
	}
	angle = eigenVec[1]/eigenVec[0];
	axisA = (int)fn_Round(2*sqrt(eigenVal[0]));
	axisB = (int)fn_Round(2*sqrt(eigenVal[1]));

	//angle = eigenVec[3]/eigenVec[2];
	//axisA = (int)fn_Round(2*sqrt(eigenVal[1]));
	//axisB = (int)fn_Round(2*sqrt(eigenVal[0]));

	//return shape result
	outResult->x0 = mx0;
	outResult->y0 = my0;
	outResult->axisA = axisA;
	outResult->axisB = axisB;
	outResult->angle = angle;	

	//ems_EllipseEdge(imgWeight, 0,mx0,my0, axisA,axisB,angle);
	//ems_EllipseEdge2(imgInput, 2, 150,100, 50,25,PI/4);
	//ems_EllipseRegion(imgInput, 1, 150,100, 20,10,PI/4);

	//cvSaveImage("c:\\CTracker\\imgWeight_ellipse.bmp", imgWeight);
	//cvReleaseImage(&imgWeight);
}

///////////////////////////////////////////////////////////////////////////////
void ems_EllipseEdge(IplImage *inImage, int channel, int x0, int y0, int a, int b, double angle)
//Label ellipse Edge on 3-channel 8-bit(BYTE) image
//label points every 2PI/PTNUM degree around the ellipse edge 
{
	double co, si;
	double theta;
	int x, y;
	CvScalar pixel;	
	int i;
	
	co = cos(angle);
	si = sin(angle);
	for (i=0;i<PTNUM;i++){
		theta = i*2*PI/PTNUM;
		x = (int)fn_Round(x0+a*co*cos(theta) - b*si*sin(theta));
		y = (int)fn_Round(y0+a*si*cos(theta) + b*co*sin(theta));	

		if (x<0 || x>=inImage->width || y<0 || y>=inImage->height) continue;
		pixel.val[0] = 0;
		pixel.val[1] = 0;
		pixel.val[2] = 0;
		pixel.val[channel] = 255;
		cvSet2D(inImage, y, x, pixel);				
	}
	//debug
	//cvSaveImage("c:\\CTracker\\inImage1.jpg", inImage);
}

void ems_EllipseEdge2(IplImage *inImage, int channel, int x0, int y0, int a, int b, double angle)
//Label ellipse edge on 3-channel 8-bit(BYTE) image
//Ellipse Equation (cos*x+sin*y)^2/a^2 + (-sin*x+cos*Y)^2/b^2 = 1, mark y1, y2 for every x
{
	double co, si;		
	CvScalar pixel;	
	double aa, bb, cc, deltasqrt; //ellipse equation parameters
	double xaa, xcc, xsqrt;   //parameters to estimate minx, maxx
	int x, y1, y2, yi, dy;
	int xmin, xmax, dx;	

	co = cos(angle);
	si = sin(angle);

	aa = si*si/(a*a) + co*co/(b*b);
	xcc = 4*aa;
	xaa = 4*aa*(co*co/(a*a) + si*si/(b*b)) - 4*co*co*si*si*pow(1.0/(a*a)-1.0/(b*b),2);
	xsqrt = sqrt(xcc/xaa);
	xsqrt = floor(xsqrt);
	xmin = x0-(int)xsqrt;
	xmax = x0+(int)xsqrt;

	for (x=xmin;x<=xmax;x++){
		//check boundary loc
		if (x<0 || x>inImage->width) continue;
		dx = x - x0;
		cc = dx*dx*(co*co/(a*a) + si*si/(b*b)) - 1;
		bb = dx*2*co*si*(1.0/(a*a) - 1.0/(b*b));
		deltasqrt = sqrt(bb*bb-4*aa*cc);
		y1 = y0 + (int)fn_Round((-bb - deltasqrt)/(2*aa));
		y2 = y0 + (int)fn_Round((-bb + deltasqrt)/(2*aa));
		
		//label pixel on image
		pixel.val[0] = 0;
		pixel.val[1] = 0;
		pixel.val[2] = 0;
		pixel.val[channel] = 255;
		for (yi=y1;yi<=y2;yi++){
			dy = min(abs(yi-y1), abs(yi-y2));
            if (yi<0 || yi>=inImage->height ||dy>=3)continue;
			cvSet2D(inImage, yi, x, pixel);			
		}
	}
	//debug
	//cvSaveImage("c:\\CTracker\\inImage2.jpg", inImage);
}

void ems_EllipseRegion(IplImage *inImage, int channel, int x0, int y0, int a, int b, double angle)
//Label ellipse mask on 3-channel 8-bit(BYTE) image
//Ellipse Equation (cos*x+sin*y)^2/a^2 + (-sin*x+cos*Y)^2/b^2 = 1, mark y1, y2 for every x
{
	double co, si;		
	CvScalar pixel;	
	double aa, bb, cc, deltasqrt; //ellipse equation parameters
	double xaa, xcc, xsqrt;   //parameters to estimate minx, maxx
	int x, y, y1, y2;
	int xmin, xmax, dx;	

	co = cos(angle);
	si = sin(angle);

	aa = si*si/(a*a) + co*co/(b*b);
	xcc = 4*aa;
	xaa = 4*aa*(co*co/(a*a) + si*si/(b*b)) - 4*co*co*si*si*pow(1.0/(a*a)-1.0/(b*b),2);
	xsqrt = sqrt(xcc/xaa);
	xsqrt = floor(xsqrt);
	xmin = x0-(int)xsqrt;
	xmax = x0+(int)xsqrt;

	for (x=xmin;x<=xmax;x++){
		//check boundary loc
		if (x<0 || x>inImage->width) continue;
		dx = x - x0;
		cc = dx*dx*(co*co/(a*a) + si*si/(b*b)) - 1;
		bb = dx*2*co*si*(1.0/(a*a) - 1.0/(b*b));
		deltasqrt = sqrt(bb*bb-4*aa*cc);
		y1 = y0 + (int)fn_Round((-bb - deltasqrt)/(2*aa));
		y2 = y0 + (int)fn_Round((-bb + deltasqrt)/(2*aa));
		
		//label pixel on image
		pixel.val[0] = 0;
		pixel.val[1] = 0;
		pixel.val[2] = 0;
		pixel.val[channel] = 255;
		for (y=y1;y<=y2;y++){
			if (y<0 || y>=inImage->height)continue;
			cvSet2D(inImage, y, x, pixel);			
		}
	}
	//debug
	//cvSaveImage("c:\\CTracker\\inImage_region.jpg", inImage);
}

