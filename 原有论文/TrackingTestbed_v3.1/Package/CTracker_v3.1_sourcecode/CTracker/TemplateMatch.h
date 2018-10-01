#include "Utility.h"

//Tempate Match with Correlation Tracker
void temp_TrackInit_Corr(IplImage* inImage, IplImage* inMask, RECT inTargetBox);
void temp_TrackNextFrame_Corr(IplImage* inImg, RECT inStartBox, TkResult *outResult);

//Tempate Match with Sqr Difference Tracker
void temp_TrackInit_SqDiff(IplImage* inImage, IplImage* inMask, RECT inTargetBox);
void temp_TrackNextFrame_SqDiff(IplImage* inImg, RECT inStartBox, TkResult *outResult);

//Tempate Match with Coefficient Tracker
void temp_TrackInit_Coeff(IplImage* inImage, IplImage* inMask, RECT inTargetBox);
void temp_TrackNextFrame_Coeff(IplImage* inImg, RECT inStartBox, TkResult *outResult);
void temp_TrackCleanUp();