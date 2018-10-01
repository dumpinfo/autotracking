 /////////////////////////////////////////////////////////////////////////////

#include "Utility.h"


#ifndef EXTERN_C
	#ifdef __cplusplus
		#define EXTERN_C extern "C"        
	#else
		#define EXTERN_C        
	#endif
#endif

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
EXTERN_C void msw_TrackInit(IplImage* inImage, IplImage* inMask, RECT inTargetBox);
EXTERN_C void msw_TrackNextFrame(IplImage* inImg, RECT inStartBox, TkResult *outResult);
EXTERN_C void msw_TrackCleanUp();