 /////////////////////////////////////////////////////////////////////////////

#include "Utility.h"

//full size
void msw_Init(IplImage *inImage, RECT inRect);	
void msw_Track(IplImage *inImage, RECT *ioRect);

//reSize
void msw_InitSize(IplImage* inImage, RECT inRect);
void msw_TrackSize(IplImage *inImage, RECT *ioRect);

IplImage* msw_GetWeightImg();
IplImage* msw_GetTrackImg();
void msw_ClearWeightImg();

void msw_InitRegion(IplImage* inImage, HRGN hRegion);
void msw_PrintHistogram(char* filename);

//Tracker Interface Function
void msw_TrackInit(IplImage* inImage, IplImage* inMask, RECT inTargetBox);
void msw_TrackNextFrame(IplImage* inImg, RECT inStartBox, TkResult *outResult);
void msw_TrackCleanUp();