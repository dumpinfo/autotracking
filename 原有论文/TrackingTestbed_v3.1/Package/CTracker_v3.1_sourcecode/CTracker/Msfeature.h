 /////////////////////////////////////////////////////////////////////////////

#include "Utility.h"

//Tracker Interface Function
void feat_TrackInit(IplImage* inImage, IplImage* inMask, RECT inTargetBox);
void feat_TrackInit_PeakDiff(IplImage* inImage, IplImage* inMask, RECT inTargetBox);
void feat_TrackNextFrame(IplImage* inImg, RECT inStartBox, TkResult *outResult);
void feat_TrackNextFrame_Adapt(IplImage* inImg, RECT inStartBox, TkResult *outResult);
void feat_TrackNextFrame_PeakDiff(IplImage* inImg, RECT inStartBox, TkResult *outResult);
void feat_TrackNextFrame_PeakDiff_Adapt(IplImage* inImg, RECT inStartBox, TkResult *outResult);
void feat_TrackCleanUp();
void feat_ReselectFeature(IplImage* imgInput, RECT inTargetBox, IplImage* imgFgMask);
void feat_ReselectFeature_PeakDiff(IplImage* imgInput, RECT inTargetBox, IplImage* imgFgMask);
char* feat_GetFeatLabel();
float feat_CountFGPixel(IplImage *inImg, RECT inRect);

//aux functions
//void gencolorfeatures();
void gencolorfeatures();

//meanshift routine
void meanshift(IplImage *imgInput, int xstart, int ystart, int hx, int hy, double eps, double *modex, double *modey);

//get foreground image
IplImage* feat_GetWeightImage();

