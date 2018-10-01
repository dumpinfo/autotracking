#include "Utility.h"

#ifndef EXTERN_C
	#ifdef __cplusplus
		#define EXTERN_C extern "C"        
	#else
		#define EXTERN_C        
	#endif
#endif

//Tempate Match with Coefficient Tracker
EXTERN_C void graph_TrackInit(IplImage* inImage, IplImage* inMask, RECT inTargetBox);
EXTERN_C void graph_TrackNextFrame(IplImage* inImg, RECT inStartBox, TkResult *outResult);
EXTERN_C void graph_TrackCleanUp();
EXTERN_C IplImage* graph_GetFGImage();