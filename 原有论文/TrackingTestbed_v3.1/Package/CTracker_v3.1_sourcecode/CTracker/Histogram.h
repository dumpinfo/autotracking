
#include "Utility.h"

void		his_PrintHistogram(CvHistogram *hist, char* filename);
CvHistogram* his_HistogramFromArray(float* modelhist);
CvHistogram* his_HistogramRatio(CvHistogram* hist1_model, CvHistogram *hist2_denom);
CvHistogram* his_HistogramAdd(CvHistogram* hist1, CvHistogram *hist2, float weight);
CvHistogram* his_CalcHistogram(IplImage* inImg);

void		his_CalcForeground2(IplImage *inImgHSV, CvHistogram *inHist, CvRect inRect, IplImage *outImgFg);
IplImage*	his_CalcBackProject(IplImage *inImgHSV, CvHistogram *inHist);
void		his_BackProject(BYTE *imgin, BYTE *imgout, CvHistogram *hist);
void		his_CheckBoundRect(CvRect *rect, int imgWidth, int imgHeight);

IplImage*	his_GetBackProjImage();

void		his_ReadHistBinsFromFile();

void		his_HistogramNormalize(CvHistogram* inHist); 

void		his_CheckBoxBound(RECT *inRect, int imgWidth, int imgHeight);

void		his_HistogramThresholdUp(CvHistogram* inHist, float thresholdVal);
void		his_CalcGravityCenter2(IplImage* inImg, RECT blobBox, POINT *outCenter);
void		his_CalcGravityCenter(IplImage* inImg, POINT *outCenter);
void		his_CountBoxHistogram(IplImage* inImg, RECT inRect, CvHistogram *outHist);
void		his_HistogramDiff(CvHistogram* hist1, CvHistogram *hist2, CvHistogram *histRst);
void		his_BlobCenterShift(IplImage* inImg, RECT *inRect, int range);

//mask image
void		his_TrackInit(IplImage *inImage, IplImage *inMask, RECT inRect);
void		his_TrackNextFrame(IplImage* inImg, RECT inRect, TkResult *outResult);
void		his_TrackInit_Bins(IplImage *inImg, IplImage *inMask, RECT inRect, int nbins1, int nbins2, int nbins3);
float		his_CountFGPixel(IplImage *inImg, RECT inRect);
CvHistogram* his_HistogramDivide(CvHistogram* hist1_model, CvHistogram *hist2_denom);

void		his_TrackCleanUp();