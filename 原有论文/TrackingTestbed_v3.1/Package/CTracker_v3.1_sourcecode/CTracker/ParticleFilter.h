#ifndef particlefilter_h
#define particlefilter_h

#include "Utility.h"


//Tempate Match with Coefficient Tracker
void particle_TrackInit(IplImage* inImage, IplImage* inMask, RECT inTargetBox);
void particle_TrackNextFrame(IplImage* inImg, RECT inStartBox, TkResult *outResult);
void particle_TrackCleanUp();
IplImage* particle_GetParticleImg();

#endif