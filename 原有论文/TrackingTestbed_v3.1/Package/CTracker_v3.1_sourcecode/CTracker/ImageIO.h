   
// ImageIO.h : head file for imageio.c
//
/////////////////////////////////////////////////////////////////////////////

#include "Utility.h"

void	iio_Init(HWND hwnd);
void	iio_Exit();
int		iio_OpenFileByPrompt();
int		iio_ReadFileByName(char* filepath) ;
void	iio_ReadFileByIndex(int frameIndex);	
void	iio_ReadNextFrame(char* currentFile, int nextFrameIndex);
void	iio_CalcFrameNo(char* inFilename, int* outFrameNo, int* outFrameDigit);
void	iio_CalcFrameTitle(char* inFilePath, char* outFrameTitle);
void	iio_CalcFrameDir(char* inFilePath, char* outFrameDir);
char*	iio_GetFileTitle();
char*	iio_GetImageTitle();
char*	iio_GetFilePath();
int		iio_GetFrameNo();
IplImage* iio_GetInputImage();
IplImage* iio_GetMaskImage();
void	iio_RealeaseMaskImage();
int		iio_ImageMaxX();
int		iio_ImageMaxY();

int		iio_ReadTrackFile(char* filepath);
//POINT*	iio_GetPolyPoints();

//thread io functions
void	iio_ThreadWaitImageReady();
void	iio_ThreadBufferNextImage();
void	iio_ThreadReadNextImage(void *dummy);	

//Trk, Rpt file replay
int		iio_ReplayTrkFile();
int		iio_ReplayRptFile(int rptIndex);

//Btk file
int		iio_ReadBatchTrackFile(char* filepath);