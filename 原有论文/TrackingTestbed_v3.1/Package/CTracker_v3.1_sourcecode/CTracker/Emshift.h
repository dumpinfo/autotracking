 /////////////////////////////////////////////////////////////////////////////
#include "Utility.h"

//Tracker Interface Function
void ems_TrackInit(IplImage* imgInput, IplImage* imgObjMask, RECT inTargetBox);
void ems_TrackNextFrame(IplImage* inImg, RECT inStartBox, TkResult *outResult);

//ellipse shape routines
void ems_EllipseEdge(IplImage *inImage, int channel, int x0, int y0, int a, int b, double angle);
void ems_EllipseEdge2(IplImage *inImage, int channel, int x0, int y0, int a, int b, double angle);
void ems_EllipseRegion(IplImage *inImage, int channel, int x0, int y0, int a, int b, double angle);